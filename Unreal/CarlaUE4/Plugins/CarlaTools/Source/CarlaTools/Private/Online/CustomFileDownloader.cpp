// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#undef CreateDirectory  // 取消对CreateDirectory的宏定义，防止与文件操作相关的重定义冲突

#include "Online/CustomFileDownloader.h"
#include "OpenDriveToMap.h"  
#include "HttpModule.h"  
#include "Http.h"  
#include "Misc/FileHelper.h"  

#include <OSM2ODR.h>  // 引入OpenStreetMap到OpenDrive格式转换的库

// 用于将OSM文件转换为OpenDrive格式
void UCustomFileDownloader::ConvertOSMInOpenDrive(FString FilePath, float Lat_0, float Lon_0)
{
  IPlatformFile &FileManager = FPlatformFileManager::Get().GetPlatformFile();  // 获取平台文件管理器

  FString FileContent;
  // 检查文件是否存在
  if (FileManager.FileExists(*FilePath))
  {
    // 加载文件内容到FileContent字符串
    if (FFileHelper::LoadFileToString(FileContent, *FilePath, FFileHelper::EHashOptions::None))
    {
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FileManipulation: Text From File: %s"), *FilePath);  // 打印文件读取成功的日志
    }
    else
    {
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FileManipulation: Did not load text from file"));  // 打印加载文件失败的日志
    }
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("File: %s does not exist"), *FilePath);  // 如果文件不存在，打印警告
    return;
  }

  std::string OsmFile = std::string(TCHAR_TO_UTF8(*FileContent));  // 将FString转换为std::string

  // 设置OSM到OpenDrive转换的参数
  osm2odr::OSM2ODRSettings Settings;
  Settings.proj_string += " +lat_0=" + std::to_string(Lat_0) + " +lon_0=" + std::to_string(Lon_0);  // 设置投影原点的经纬度
  Settings.center_map = false;  // 不居中地图
  std::string OpenDriveFile = osm2odr::ConvertOSMToOpenDRIVE(OsmFile, Settings);  // 转换OSM文件为OpenDrive格式

  // 修改文件路径，将文件后缀从".osm"更改为".xodr"
  FilePath.RemoveFromEnd(".osm", ESearchCase::Type::IgnoreCase);
  FilePath += ".xodr";

  // 将转换后的OpenDrive文件内容保存到指定路径
  if (FFileHelper::SaveStringToFile(FString(OpenDriveFile.c_str()), *FilePath))
  {
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FileManipulation: Sucsesfuly Written: \"%s\" to the text file"), *FilePath);  // 打印文件写入成功的日志
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FileManipulation: Failed to write FString to file."));  // 打印文件写入失败的日志
  }
}

// UCustomFileDownloader类的StartDownload函数：用于启动文件下载
void UCustomFileDownloader::StartDownload()
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("FHttpDownloader CREATED"));  // 打印下载器创建的日志
  UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("Map Name Is %s"), *ResultFileName);  // 打印地图名称
  FHttpDownloader *Download = new FHttpDownloader("GET", Url, ResultFileName, DownloadDelegate);  // 创建下载器对象
  Download->Run();  // 启动下载
}

// FHttpDownloader构造函数：初始化HTTP下载器
FHttpDownloader::FHttpDownloader(const FString &InVerb, const FString &InUrl, const FString &InFilename, FDownloadComplete &Delegate)
    : Verb(InVerb), Url(InUrl), Filename(InFilename), DelegateToCall(Delegate)  // 初始化HTTP请求的相关参数
{
}

// FHttpDownloader默认构造函数
FHttpDownloader::FHttpDownloader()
{
}

// Run函数：启动HTTP请求
void FHttpDownloader::Run(void)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("Starting download [%s] Url=[%s]"), *Verb, *Url);  // 打印启动下载日志
  TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();  // 创建HTTP请求对象
  UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("Map Name Is %s"), *Filename);  // 打印文件名
  Request->OnProcessRequestComplete().BindRaw(this, &FHttpDownloader::RequestComplete);  // 绑定请求完成后的回调函数
  Request->SetURL(Url);  // 设置请求的URL
  Request->SetVerb(Verb);  // 设置HTTP请求方法（GET、POST等）
  Request->ProcessRequest();  // 发起请求
}

// 函数：处理下载请求完成后的回调
void FHttpDownloader::RequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
  if (!HttpResponse.IsValid())
  {
    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("Download failed. NULL response"));  // 如果响应为空，打印下载失败的日志
  }
  else
  {
    // 如果响应码不在200到299之间，表示下载失败
    if (HttpResponse->GetResponseCode() < 200 || 300 <= HttpResponse->GetResponseCode())
    {
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Error during download [%s] Url=[%s] Response=[%d]"),
             *HttpRequest->GetVerb(),
             *HttpRequest->GetURL(),
             HttpResponse->GetResponseCode());  // 打印错误日志
      return;
    }

    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("Completed download [%s] Url=[%s] Response=[%d]"),
           *HttpRequest->GetVerb(),
           *HttpRequest->GetURL(),
           HttpResponse->GetResponseCode());  // 打印下载成功的日志

    // 设置下载文件的存储路径
    FString CurrentFile = FPaths::ProjectContentDir() + "CustomMaps/" + (Filename) + "/OpenDrive/";
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FHttpDownloader::RequestComplete CurrentFile %s."), *CurrentFile);

    // 获取平台文件管理器，处理文件操作
    IPlatformFile &FileManager = FPlatformFileManager::Get().GetPlatformFile();
    if (!FileManager.DirectoryExists(*CurrentFile))  // 如果目标文件夹不存在
    {
      FileManager.CreateDirectory(*CurrentFile);  // 创建目标文件夹
    }
    CurrentFile += Filename + ".osm";  // 设置文件路径，确保文件名正确

    FString StringToWrite = HttpResponse->GetContentAsString();  // 获取下载内容为字符串

    // 保存下载的内容到文件
    if (FFileHelper::SaveStringToFile(StringToWrite, *CurrentFile, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FileManipulation: Sucsesfuly Written "));  // 打印成功写入文件的日志
      DelegateToCall.ExecuteIfBound();  // 调用下载完成的回调函数
    }
    else
    {
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FileManipulation: Failed to write FString to file."));  // 打印文件写入失败的日志
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FileManipulation: CurrentFile %s."), *CurrentFile);  // 打印当前文件路径
    }
  }

  delete this;  // 删除下载对象，防止内存泄漏
}

