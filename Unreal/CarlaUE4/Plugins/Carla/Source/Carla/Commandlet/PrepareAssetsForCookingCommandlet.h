// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/OpenDrive/OpenDriveActor.h"
#include "Commandlets/Commandlet.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "PrepareAssetsForCookingCommandlet.generated.h"

// 取消定义此API以避免与UE 4.26发生冲突
// (see UE_4.26\Engine\Source\Runtime\Core\Public\Windows\HideWindowsPlatformAtomics.h)
#undef InterlockedCompareExchange
#undef _InterlockedCompareExchange

/// 包含带有 @a Name 的 Package 和 @a bOnlyPrepareMaps 标志的结构体，用于
/// 在不同阶段分离地图和道具的烹饪过程（地图将在 make import 命令期间导入，而道具将在 make package 命令期间导入）
USTRUCT()
struct CARLA_API FPackageParams
{
  GENERATED_USTRUCT_BODY()

  FString Name;

  bool bOnlyPrepareMaps;
};

/// 包含从 .Package.json 文件中读取的地图数据的结构体
USTRUCT()
struct CARLA_API FMapData
{
  GENERATED_USTRUCT_BODY()

  FString Name;

  FString Path;

  bool bUseCarlaMapMaterials;
};

/// 包含从 .Package.json 文件中读取的所有资产数据的结构
USTRUCT()
struct CARLA_API FAssetsPaths
{
  GENERATED_USTRUCT_BODY()

  TArray<FMapData> MapsPaths;

  TArray<FString> PropsPaths;
};

UCLASS()
class CARLA_API UPrepareAssetsForCookingCommandlet
  : public UCommandlet
{
  GENERATED_BODY()

public:

  /// 默认构造函数
  UPrepareAssetsForCookingCommandlet();
#if WITH_EDITORONLY_DATA

  ///解析通过 @a InParams 提供的命令行参数
  FPackageParams ParseParams(const FString &InParams) const;

  /// 将包含在 Carla BaseMap 中的 UWorld 对象加载到 @a AssetData 数据结构中
  void LoadWorld(FAssetData &AssetData);

  /// 将包含在 Carla BaseTile 中的 UWorld 对象加载到 @a AssetData 数据结构中
  void LoadWorldTile(FAssetData &AssetData);

  void LoadLargeMapWorld(FAssetData &AssetData);

  ///在世界中生成位于 @a AssetsPaths 中的所有静态网格物体
  /// 可以通过设置 @a bUseCarlaMaterials 来使用 Carla 材质
  /// 否则它将使用RoadRunner材料
  /// 如果网格被添加到PropsMap，请将@a bIsPropMap设置为true.
  ///
  ///@pre 世界预计已预先加载
  TArray<AStaticMeshActor *> SpawnMeshesToWorld(
      const TArray<FString> &AssetsPaths,
      bool bUseCarlaMaterials,
      int i = -1,
      int j = -1);

  /// 将当前包含在 @a AssetData 中的世界保存到 @a DestPath
  /// 由@a PackageName 组成，并以@a WorldName 命名
  bool SaveWorld(
      FAssetData &AssetData,
      const FString &PackageName,
      const FString &DestPath,
      const FString &WorldName,
      bool bGenerateSpawnPoints = true);

  ///销毁之前在 @a SpawnedActors 中存储的所有已生成的角色
  void DestroySpawnedActorsInWorld(TArray<AStaticMeshActor *> &SpawnedActors);

  ///获取与名称 @a PackageName 相关联的包中包含的所有资源的 Path
  FAssetsPaths GetAssetsPathFromPackage(const FString &PackageName) const;

  /// 生成提供的 @a AssetsPaths 和 @a PropsMapPath 的 MapPaths 文件
  void GenerateMapPathsFile(const FAssetsPaths &AssetsPaths, const FString &PropsMapPath);

  ///生成包含带有 @a PackageName 的包路径的 PackagePat 文件
  void GeneratePackagePathFile(const FString &PackageName);

  /// 对于@MapsPaths中包含的每个Map数据，它创建一个World，在世界中生成其actors，并将其以.umap格式保存到由@a PackageName构建的目标路径中。
  void PrepareMapsForCooking(const FString &PackageName, const TArray<FMapData> &MapsPaths);

  /// 对于@a PropsPaths中的所有道具，它创建一个单一的世界，在世界中生成所有道具，并将其以.umap格式保存到由@a PackageName和@a MapDestPath构建的目标路径中
  void PreparePropsForCooking(FString &PackageName, const TArray<FString> &PropsPaths, FString &MapDestPath);

  /// 如果资产之间有任何瓦片，则返回
  bool IsMapInTiles(const TArray<FString> &AssetsPaths);

public:

  /// 命令let的主方法和入口，接受输入参数@a Params
  virtual int32 Main(const FString &Params) override;

#endif // WITH_EDITORONLY_DATA

private:

  /// 从任何对象库加载的资源
  UPROPERTY()
  TArray<FAssetData> AssetDatas;

  /// 从任何对象库加载地图内容
  UPROPERTY()
  TArray<FAssetData> MapContents;

  /// 用于加载对象库中的地图。加载的数据存储在AssetDatas中
  UPROPERTY()
  UObjectLibrary *MapObjectLibrary;

  /// 用于在对象库中加载资源。加载的数据存储在 AssetDatas 中
  UPROPERTY()
  UObjectLibrary *AssetsObjectLibrary;

  ///基础地图世界从Carla内容中加载
  UPROPERTY()
  UWorld *World;

  /// 道路节点网格的替代材料
  UPROPERTY()
  UMaterialInstance *RoadNodeMaterial;

  /// 用于道路路缘的材料
  UPROPERTY()
  UMaterialInstance *CurbNodeMaterialInstance;

  /// 用于道路排水沟的材料
  UPROPERTY()
  UMaterialInstance *GutterNodeMaterialInstance;

  ///中心车道标线的替代材料
  UPROPERTY()
  UMaterialInstance *MarkingNodeYellow;

  /// 用于外部车道的标线材料
  UPROPERTY()
  UMaterialInstance *MarkingNodeWhite;

  /// 地形节点的替代材料
  UPROPERTY()
  UMaterialInstance *TerrainNodeMaterialInstance;

  /// 用于SidewalkNodes的解决方法材料
  UPROPERTY()
  UMaterialInstance *SidewalkNodeMaterialInstance;

  /// 将@a Package 保存为 .umap 格式的文件，路径为 Unreal 内容文件夹内的 @a PackagePath
  bool SavePackage(const FString &PackagePath, UPackage *Package) const;

  ///获取在虚幻内容目录中找到的第一个 .Package.json 文件，文件名为
  /// @a PackageName
  FString GetFirstPackagePath(const FString &PackageName) const;

};
