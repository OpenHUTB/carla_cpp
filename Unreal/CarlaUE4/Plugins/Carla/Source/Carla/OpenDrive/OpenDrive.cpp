/**
 * This function finds the path to the XODR file based on the map name.
 * @param InMapName The name of the map to search for.
 * @return The path to the XODR file.
 */
FString UOpenDrive::FindPathToXODRFile(const FString &InMapName) {
    // function implementation
}
// 版权所有 (c) 2019 巴塞罗那自治大学 (UAB) 计算机视觉中心 (CVC)。
// 
// 本作品依据MIT许可证授权。
// 许可证副本见 <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/OpenDrive/OpenDrive.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Misc/FileHelper.h"
#include "Carla/Game/CarlaStatics.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"
#include "Misc/FileHelper.h"

// 函数：查找指定地图的.xodr文件路径
FString UOpenDrive::FindPathToXODRFile(const FString &InMapName){
  
  // 获取地图名称
  FString MapName = InMapName;

#if WITH_EDITOR
    // 如果在编辑器中运行，地图名称会有一个额外的前缀，移除该前缀
    FString CorrectedMapName = MapName;
    constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
    CorrectedMapName.RemoveFromStart(PIEPrefix);
    MapName = CorrectedMapName;
#endif // WITH_EDITOR

  // 生成.xodr文件的路径
  MapName += TEXT(".xodr");

  // 默认文件路径
  const FString DefaultFilePath =
      FPaths::ProjectContentDir() +
      TEXT("Carla/Maps/OpenDrive/") +
      MapName;

  auto &FileManager = IFileManager::Get();

  // 检查文件是否存在
  if (FileManager.FileExists(*DefaultFilePath))
  {
    return DefaultFilePath;
  }

  // 如果默认路径没有找到文件，则递归搜索项目目录
  TArray<FString> FilesFound;
  FileManager.FindFilesRecursive(
      FilesFound,
      *FPaths::ProjectContentDir(),
      *MapName,
      true,
      false,
      false);

  // 如果找到了文件，返回第一个文件的路径，否则返回空
  return FilesFound.Num() > 0 ? FilesFound[0u] : FString{};
}

// 函数：获取当前世界中的OpenDrive文件内容
FString UOpenDrive::GetXODR(const UWorld *World)
{
  // 获取地图名称
  auto MapName = World->GetMapName();

  // 如果在编辑器中，移除前缀
#if WITH_EDITOR
  {
    FString CorrectedMapName = MapName;
    constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
    CorrectedMapName.RemoveFromStart(PIEPrefix);
    MapName = CorrectedMapName;
  }
#endif // WITH_EDITOR

  // 获取当前游戏模式并获取地图路径
  ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(World);
  auto MapDir = GameMode->GetFullMapPath();
  const auto FolderDir = MapDir + "/OpenDrive/";
  const auto FileName = MapDir.EndsWith(MapName) ? "*" : MapName;

  // 查找所有与该地图相关的.xodr文件
  TArray<FString> Files;
  IFileManager::Get().FindFilesRecursive(Files, *FolderDir, *FString(FileName + ".xodr"), true, false, false);

  FString Content;

  // 如果没有找到文件，记录错误；否则尝试加载文件
  if (!Files.Num())
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *MapName);
  }
  else if (FFileHelper::LoadFileToString(Content, *Files[0]))
  {
    UE_LOG(LogTemp, Log, TEXT("Loaded OpenDrive file '%s'"), *Files[0]);
  }
  else
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to load OpenDrive file '%s'"), *Files[0]);
  }

  return Content;
}

// 函数：根据地图名称加载OpenDrive文件
FString UOpenDrive::LoadXODR(const FString &MapName)
{
  // 查找.xodr文件路径
  const auto FilePath = FindPathToXODRFile(MapName);

  FString Content;

  // 如果路径为空，记录错误；否则加载文件内容
  if (FilePath.IsEmpty())
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *MapName);
  }
  else if (FFileHelper::LoadFileToString(Content, *FilePath))
  {
    UE_LOG(LogTemp, Log, TEXT("Loaded OpenDrive file '%s'"), *FilePath);
  }
  else
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to load OpenDrive file '%s'"), *FilePath);
  }

  return Content;
}

// 函数：根据指定的路径和地图名称加载OpenDrive文件
FString UOpenDrive::GetXODRByPath(FString XODRPath, FString MapName){
  
  // 如果在编辑器中，移除前缀
#if WITH_EDITOR
  {
    FString CorrectedMapName = MapName;
    constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
    CorrectedMapName.RemoveFromStart(PIEPrefix);
    MapName = CorrectedMapName;
  }
#endif // WITH_EDITOR

  // 如果路径已包含地图名称，使用“*”通配符匹配文件
  FString FileName = XODRPath.EndsWith(MapName) ? "*" : MapName;
  FString FolderDir = XODRPath;
  FolderDir.RemoveFromEnd(MapName + ".xodr");

  // 查找指定路径下的所有.xodr文件
  TArray<FString> Files;
  IFileManager::Get().FindFilesRecursive(Files, *FolderDir, *FString(FileName + ".xodr"), true, false, false);

  FString Content;

  // 如果未找到文件，记录错误
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

// 函数：根据地图名称加载OpenDrive地图对象
UOpenDriveMap *UOpenDrive::LoadOpenDriveMap(const FString &MapName)
{
  UOpenDriveMap *Map = nullptr;
  
  // 加载.xodr文件内容
  auto XODRContent = LoadXODR(MapName);
  if (!XODRContent.IsEmpty())
  {
    // 创建OpenDrive地图对象并加载内容
    Map = NewObject<UOpenDriveMap>();
    Map->Load(XODRContent);
  }
  return Map;
}

// 函数：加载当前世界的OpenDrive地图对象
UOpenDriveMap *UOpenDrive::LoadCurrentOpenDriveMap(const UObject *WorldContextObject)
{
  // 获取当前世界并加载OpenDrive地图
  UWorld *World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
  return World != nullptr ?
      LoadOpenDriveMap(World->GetMapName()) :
      nullptr;
}
