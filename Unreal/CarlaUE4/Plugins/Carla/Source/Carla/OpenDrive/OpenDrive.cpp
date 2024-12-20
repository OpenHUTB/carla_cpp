// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/OpenDrive/OpenDrive.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Misc/FileHelper.h"
#include "Carla/Game/CarlaStatics.h"
#include "GenericPlatform/GenericPlatformProcess.h"

#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"
#include "Misc/FileHelper.h"

FString UOpenDrive::FindPathToXODRFile(const FString &InMapName){

  FString MapName = InMapName;

#if WITH_EDITOR
    {
      // 在编辑器中游玩时，地图名称会多出一个前缀，这里我们将其移除。
      FString CorrectedMapName = MapName;
      constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
      CorrectedMapName.RemoveFromStart(PIEPrefix);
      MapName = CorrectedMapName;
    }
#endif // WITH_EDITOR

  // 向地图名称添加.xodr扩展名
  MapName += TEXT(".xodr");

  // 定义默认文件路径，位于项目的Content目录下的Carla/Maps/OpenDrive子目录中
  const FString DefaultFilePath =
      FPaths::ProjectContentDir() +
      TEXT("Carla/Maps/OpenDrive/") +
      MapName;

  // 获取文件管理器的引用
  auto &FileManager = IFileManager::Get();

  // 检查文件管理器是否存在指定的默认文件路径
  if (FileManager.FileExists(*DefaultFilePath))
  {
    // 如果文件存在，则返回该路径
    return DefaultFilePath;
  }

  // 创建一个字符串数组，用于存储查找到的文件
  TArray<FString> FilesFound;
  // 递归查找项目内容目录下所有匹配MapName的文件
  FileManager.FindFilesRecursive(
      FilesFound,
      *FPaths::ProjectContentDir(),
      *MapName,
      true,// 搜索子目录
      false,// 不递归查找子目录中的子目录
      false);// 只查找文件，不查找目录

  // 返回找到的第一个文件的路径，如果没有找到任何文件，则返回空字符串
  return FilesFound.Num() > 0 ? FilesFound[0u] : FString{};
}

// UOpenDrive类的成员函数，用于获取当前世界对应的OpenDrive文件路径
FString UOpenDrive::GetXODR(const UWorld *World)
{
  // 获取世界对象的地图名称
  auto MapName = World->GetMapName();

  // 在编辑器中游玩时，地图名称会多出一个前缀，这里我们将其移除。
  #if WITH_EDITOR
  {
    FString CorrectedMapName = MapName;
    constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
    CorrectedMapName.RemoveFromStart(PIEPrefix);
    MapName = CorrectedMapName;
  }
  #endif // WITH_EDITOR

  ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(World);

  auto MapDir = GameMode->GetFullMapPath();
  const auto FolderDir = MapDir + "/OpenDrive/";
  const auto FileName = MapDir.EndsWith(MapName) ? "*" : MapName;

  // 查找地图中所有.xodr和.bin文件。
  TArray<FString> Files;
  // 递归查找指定文件夹下所有.xodr文件
  IFileManager::Get().FindFilesRecursive(Files, *FolderDir, *FString(FileName + ".xodr"), true, false, false);

  FString Content;

  // 如果没有找到文件
  if (!Files.Num())
  {
    // 记录错误日志，表示没有找到OpenDrive文件
    UE_LOG(LogTemp, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *MapName);
  }
    // 如果成功加载了文件
  else if (FFileHelper::LoadFileToString(Content, *Files[0]))
  {
    // 记录日志，表示成功加载了OpenDrive文件
    UE_LOG(LogTemp, Log, TEXT("Loaded OpenDrive file '%s'"), *Files[0]);
  }
  // 如果加载文件失败
  else
  {
    // 记录错误日志，表示加载OpenDrive文件失败
    UE_LOG(LogTemp, Error, TEXT("Failed to load OpenDrive file '%s'"), *Files[0]);
  }

  // 返回文件内容
  return Content;
}

// UOpenDrive类的成员函数，用于加载指定地图的OpenDrive文件
FString UOpenDrive::LoadXODR(const FString &MapName)
{
  // 调用FindPathToXODRFile函数查找OpenDrive文件的路径
  const auto FilePath = FindPathToXODRFile(MapName);

  FString Content;

   // 如果文件路径为空
  if (FilePath.IsEmpty())
  {
    // 记录错误日志，表示没有找到OpenDrive文件
    UE_LOG(LogTemp, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *MapName);
  }
    // 如果成功加载了文件
  else if (FFileHelper::LoadFileToString(Content, *FilePath))
  {
    // 记录日志，表示成功加载了OpenDrive文件
    UE_LOG(LogTemp, Log, TEXT("Loaded OpenDrive file '%s'"), *FilePath);
  }
    // 如果加载文件失败
  else
  {
    // 记录错误日志，表示加载OpenDrive文件失败
    UE_LOG(LogTemp, Error, TEXT("Failed to load OpenDrive file '%s'"), *FilePath);
  }

  // 返回文件内容
  return Content;
}

// UOpenDrive类的成员函数，用于根据给定的路径和地图名称获取OpenDrive文件内容
FString UOpenDrive::GetXODRByPath(FString XODRPath, FString MapName){

  // 在编辑器中游玩时，地图名称会多出一个前缀，这里我们将其移除。
  #if WITH_EDITOR
  {
    FString CorrectedMapName = MapName;
    constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
    CorrectedMapName.RemoveFromStart(PIEPrefix);
    MapName = CorrectedMapName;
  }
  #endif // WITH_EDITOR

  FString FileName = XODRPath.EndsWith(MapName) ? "*" : MapName;
  FString FolderDir = XODRPath;
  FolderDir.RemoveFromEnd(MapName + ".xodr");

  // 查找地图中所有.xodr和.bin文件。
  TArray<FString> Files;
  IFileManager::Get().FindFilesRecursive(Files, *FolderDir, *FString(FileName + ".xodr"), true, false, false);

  FString Content;

  if (!Files.Num())
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *MapName);
  }
  else if (FFileHelper::LoadFileToString(Content, *Files[0]))
  {
    UE_LOG(LogTemp, Log, TEXT("Loaded OpenDrive file '%s'"), *Files[0]);
  }

  return Content;
}

UOpenDriveMap *UOpenDrive::LoadOpenDriveMap(const FString &MapName)
{
  UOpenDriveMap *Map = nullptr;
  auto XODRContent = LoadXODR(MapName);
  if (!XODRContent.IsEmpty())
  {
    Map = NewObject<UOpenDriveMap>();
    Map->Load(XODRContent);
  }
  return Map;
}

UOpenDriveMap *UOpenDrive::LoadCurrentOpenDriveMap(const UObject *WorldContextObject)
{
  UWorld *World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
  return World != nullptr ?
      LoadOpenDriveMap(World->GetMapName()) :
      nullptr;
}
