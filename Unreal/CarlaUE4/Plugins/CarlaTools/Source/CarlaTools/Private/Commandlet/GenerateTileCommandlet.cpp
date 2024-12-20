// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "GenerateTileCommandlet.h"

#include <iostream>
#include <fstream>

#if WITH_EDITOR
#include "FileHelpers.h"
#endif
#include "UObject/ConstructorHelpers.h"
// 定义日志类别
DEFINE_LOG_CATEGORY(LogCarlaToolsMapGenerateTileCommandlet);


UGenerateTileCommandlet::UGenerateTileCommandlet()
{
#if WITH_EDITOR
  // 使用构造函数辅助工具查找 UOpenDriveToMap 类
  ConstructorHelpers::FClassFinder<UOpenDriveToMap> OpenDrivelassFinder(TEXT("/CarlaTools/OnroadMapGenerator/BP_OpenDriveToMap"));
  // 将查找到的类存储在 OpenDriveClass 成员变量中
  OpenDriveClass = OpenDrivelassFinder.Class;
#endif
}

UGenerateTileCommandlet::UGenerateTileCommandlet(const FObjectInitializer& Initializer)
  : Super(Initializer)
{
#if WITH_EDITOR
   // 再次使用构造函数辅助工具查找 UOpenDriveToMap 类
  ConstructorHelpers::FClassFinder<UOpenDriveToMap> OpenDrivelassFinder(TEXT("/CarlaTools/OnroadMapGenerator/BP_OpenDriveToMap"));
   // 将查找到的类存储在 OpenDriveClass 成员变量中
  OpenDriveClass = OpenDrivelassFinder.Class;
#endif
}

#if WITH_EDITORONLY_DATA

int32 UGenerateTileCommandlet::Main(const FString &Params)
{
  // 输出日志信息，包含传入的参数
  UE_LOG(LogCarlaToolsMapGenerateTileCommandlet, Log, TEXT("UGenerateTileCommandlet::Main Arguments %s"), *Params);
  // 存储解析后的命令行标记和开关的数组
  TArray<FString> Tokens;
  TArray<FString> Switches;
  // 存储解析后的命令行参数映射
  TMap<FString,FString> ParamsMap;
  // 解析命令行，将结果存储在 Tokens、Switches 和 ParamsMap 中
  ParseCommandLine(*Params, Tokens, Switches, ParamsMap );
  // 将命令行参数转换为 std::string 并存储在 logstr 中
  std::string logstr = std::string(TCHAR_TO_UTF8(*Params)); 
  // 以追加模式打开文件 CommandletParameters.txt
  std::ofstream file("CommandletParameters.txt", std::ios::app);
  // 将 logstr 写入文件
  file << logstr << std::endl;
  // 遍历 Tokens 数组，将每个元素写入文件
  for( auto fstr : Tokens )
  {
    logstr = std::string(TCHAR_TO_UTF8(*fstr));
    file << " Tokens " << logstr << std::endl;
  }
  // 遍历 Switches 数组，将每个元素写入文件
  for( auto fstr : Switches )
  {
    logstr = std::string(TCHAR_TO_UTF8(*fstr));
    file << " SWITCHES " << logstr << std::endl;
  }
  // 遍历 ParamsMap，将键值对写入文件
  for( auto PairMap : ParamsMap )
  {
    std::string key = std::string(TCHAR_TO_UTF8(*PairMap.Key));
    std::string value = std::string(TCHAR_TO_UTF8(*PairMap.Value));

    file << "Key: " << key << " Value: " << value << std::endl;
  }


  // 使用查找到的类创建一个新的 UOpenDriveToMap 对象
  OpenDriveMap = NewObject<UOpenDriveToMap>(this, OpenDriveClass);
  // 从 ParamsMap 中设置文件路径
  OpenDriveMap->FilePath = ParamsMap["FilePath"];
  // 从 ParamsMap 中设置基础级别名称
  OpenDriveMap->BaseLevelName = ParamsMap["BaseLevelName"];
  // 从 ParamsMap 中设置原始地理坐标，将字符串转换为浮点数
  OpenDriveMap->OriginGeoCoordinates = FVector2D(FCString::Atof(*ParamsMap["GeoCoordsX"]),FCString::Atof(*ParamsMap["GeoCoordsY"]));
   // 从 ParamsMap 中设置当前瓦片的 XY 坐标，将字符串转换为整数
  OpenDriveMap->CurrentTilesInXY = FIntVector(FCString::Atof(*ParamsMap["CTileX"]),FCString::Atof(*ParamsMap["CTileY"]), 0);
  // 解析参数
  // 调用生成瓦片的函数
  OpenDriveMap->GenerateTile();
  return 0;
}

#endif
