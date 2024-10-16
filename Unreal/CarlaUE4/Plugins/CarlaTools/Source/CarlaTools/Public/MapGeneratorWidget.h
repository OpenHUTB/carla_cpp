// 版权所有(c) 2022年巴塞罗那自治大学（UAB）计算机视觉中心（CVC）。本作品在麻省理工学院许可条款下获得许可。查看副本，参见<https://opensource.org/licenses/MIT>。

#pragma once 

#include "CoreMinimal.h"

#include "EditorUtilityWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ProceduralFoliageSpawner.h"
#include "RegionOfInterest.h"
#include "UnrealString.h"

#include "MapGeneratorWidget.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCarlaToolsMapGenerator, Log, All);

struct FSoilTerramechanicsProperties;

/// 结构体，用作基本地图信息的容器
USTRUCT(BlueprintType)
struct CARLATOOLS_API FMapGeneratorMetaInfo
{
  GENERATED_USTRUCT_BODY();

  UPROPERTY(BlueprintReadWrite)
  FString DestinationPath;

  UPROPERTY(BlueprintReadWrite)
  FString MapName;

  UPROPERTY(BlueprintReadWrite)   
  int SizeX;

  UPROPERTY(BlueprintReadWrite)
  int SizeY;

  UPROPERTY(BlueprintReadWrite)
  TArray<UProceduralFoliageSpawner*> FoliageSpawners;

  UPROPERTY(BlueprintReadWrite)
  FSoilTerramechanicsProperties GeneralSoilType;

  UPROPERTY(BlueprintReadWrite)
  UTextureRenderTarget2D* GlobalHeightmap;

  UPROPERTY(BlueprintReadWrite)
  TMap<FRoiTile, FTerrainROI> TerrainRoisMap;

  UPROPERTY(BlueprintReadWrite)
  TMap<FRoiTile, FVegetationROI> VegetationRoisMap;

  UPROPERTY(BlueprintReadWrite)
  TMap<FRoiTile, FSoilTypeROI> SoilTypeRoisMap;

  UPROPERTY(BlueprintReadWrite)
  TMap<FRoiTile, FMiscSpecificLocationActorsROI> MiscSpecificLocationActorsRoisMap;

  UPROPERTY(BlueprintReadWrite)
  TMap<FRoiTile, FMiscSpreadedActorsROI> MiscSpreadedActorsRoisMap;

  UPROPERTY(BlueprintReadWrite)
  TSubclassOf<AActor> VegetationManagerBpClass;

  UPROPERTY(BlueprintReadWrite)
  TSubclassOf<AActor> TerramechanicsBpClass;

  UPROPERTY(BlueprintReadWrite)
  float RiverChanceFactor;

  UPROPERTY(BlueprintReadWrite)
  float RiverFlateningFactor;
};

/// 结构体，用作基本瓦片信息的容器
USTRUCT(BlueprintType)
struct CARLATOOLS_API FMapGeneratorTileMetaInfo
{
  GENERATED_USTRUCT_BODY();

  UPROPERTY(BlueprintReadWrite)
  FMapGeneratorMetaInfo MapMetaInfo;

  UPROPERTY(BlueprintReadWrite)   
  bool bIsTiled = true;

  UPROPERTY(BlueprintReadWrite)   
  int IndexX;

  UPROPERTY(BlueprintReadWrite)
  int IndexY;

  UPROPERTY(BlueprintReadWrite)
  bool ContainsRiver;

  UPROPERTY(BlueprintReadWrite)
  FString RiverPreset;
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FMapGeneratorWidgetState
{
  GENERATED_USTRUCT_BODY();

  //综合类
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  bool IsPersistentState;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  FString MapName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  FString WorkingPath;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  FString ActiveTabName;

  // 地形
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainGeneralSize;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainGeneralSlope;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainGeneralHeight;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainGeneralMinHeight;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainGeneralMaxHeight;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainGeneralInvert;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainOverallSeed;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainOverallScale;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainOverallSlope;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainOverallHeight;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainOverallMinHeight;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainOverallMaxHeight;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainOverallInvert;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainDetailedSeed;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainDetailedScale;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainDetailedSlope;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainDetailedHeight;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainDetailedMinHeight;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainDetailedMaxHeight;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  float TerrainDetailedInvert;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary")
  bool bShowDetailedNoise;
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FMapGeneratorPreset
{
  GENERATED_USTRUCT_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|TerrainPresets")
  FString PresetName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|TerrainPresets")
  FMapGeneratorWidgetState WidgetState;
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FTileBoundariesInfo
{
  GENERATED_USTRUCT_BODY()

  UPROPERTY()
  TArray<uint16> RightHeightData;

  UPROPERTY()
  TArray<uint16> BottomHeightData;
};

UENUM(BlueprintType)
enum EMapGeneratorToolMode
{
  NEW_MAP,
  MISC_EDITOR
};

/// 类UMapGeneratorWidget扩展了
///UEditorUtilityWidget的功能，
///能够生成和管理地图和大型地图块，用于过程地图生成
UCLASS(BlueprintType)
class CARLATOOLS_API UMapGeneratorWidget : public UEditorUtilityWidget
{
  GENERATED_BODY()

public:
  /// 设置指定的材质为@a Landscape
  UFUNCTION(BlueprintImplementableEvent)
  void AssignLandscapeMaterial(ALandscape* Landscape);

  UFUNCTION(BlueprintImplementableEvent)
  void InstantiateRiverSublevel(UWorld* World, const FMapGeneratorTileMetaInfo TileMetaInfo);

  ///临时执照
  UFUNCTION(BlueprintImplementableEvent)
  void UpdateTileRT(const FMapGeneratorTileMetaInfo& TileMetaInfo);

  UFUNCTION(BlueprintImplementableEvent)
  void UpdateTileRoiRT(const FMapGeneratorTileMetaInfo& TileMetaInfo, UMaterialInstanceDynamic* RoiMeterialInstance);

  UFUNCTION(BlueprintImplementableEvent)
  void UpdateRiverActorSplinesEvent(AActor* RiverActor);

  /// 由Widget Blueprint调用的函数，
  ///该函数生成map @a mapName的所有tile，
  ///并将它们保存在@a destinationPath中。
  ///返回一个空字符串表示成功，如果进程失败则返回错误消息
  UFUNCTION(Category="Map Generator",BlueprintCallable)
  void GenerateMapFiles(const FMapGeneratorMetaInfo& MetaInfo);

  /// Widget Blueprint调用的函数，
  ///用于启动@a MetaInfo中定义的地图的整个植被过程
  UFUNCTION(Category="Map Generator|Vegetation",BlueprintCallable)
  void CookVegetation(const FMapGeneratorMetaInfo& MetaInfo);

  UFUNCTION(Category="Map Generator|Soil Terramechanics", BlueprintCallable)
  void CookSoilTypeToMaps(const FMapGeneratorMetaInfo& MetaInfo);

  UFUNCTION(Category="Map Generator|Miscellaneous", BlueprintCallable)
  void CookMiscSpreadedInformationToTiles(const FMapGeneratorMetaInfo& MetaInfo);

  UFUNCTION(Category="Map Generator|Miscellaneous", BlueprintCallable)
  void CookMiscSpecificLocationInformationToTiles(const FMapGeneratorMetaInfo& MetaInfo);

  UFUNCTION(Category="Map Generator|Miscellaneous", BlueprintCallable)
  void DeleteAllSpreadedActors(const FMapGeneratorMetaInfo& MetaInfo);

  ///由小部件调用的函数，
  ///该小部件只在编辑器中打开的世界中处理 
  ///@a FoliageSpawners中定义的植被
  UFUNCTION(Category="Map Generator", BlueprintCallable)
  void CookVegetationToCurrentTile(const TArray<UProceduralFoliageSpawner*> FoliageSpawners);

  /// 重新计算加载地图的碰撞
  UFUNCTION(Category="Map Generator", BlueprintCallable)
  bool RecalculateCollision();

  /// 遍历@a MetaInfo中所有的贴图并重新计算
  ///它们的碰撞。可以从蓝图中调用
  UFUNCTION(Category = "MapGenerator", BlueprintCallable)
  void CookTilesCollisions(const FMapGeneratorMetaInfo& MetaInfo);

  ///Utils函数格式化@a InDirectory，
  ///使其以虚幻可以访问该目录的格式进行清理，
  ///删除不必要的字符，如最后的‘/’或‘\’
  UFUNCTION(Category="Map Generator", BlueprintCallable)
  FString SanitizeDirectory(FString InDirectory);

  ///用于查找@a InDirectory是否包含映射。
  ///如果确实返回true并且@a OutMapSize
  ///和@OutFoundMapName存储找到的地图的信息
  UFUNCTION(Category="Map Generator", BlueprintCallable)
  bool LoadMapInfoFromPath(FString InDirectory, int& OutMapSize, FString& OutFoundMapName);

  /// 生成@a RiverClass类型的河流
  UFUNCTION(Category="MapGenerator|Water", BlueprintCallable)
  AActor* GenerateWater(TSubclassOf<class AActor> RiverClass);

  UFUNCTION(Category="MapGenerator|Water", BlueprintCallable)
  bool GenerateWaterFromWorld(UWorld* RiversWorld, TSubclassOf<class AActor> RiverClass);

  UFUNCTION(Category="MapGenerator", BlueprintCallable)
  UWorld* DuplicateWorld(const FString BaseWorldPath, const FString TargetWorldPath, const FString NewWorldName);

  /// 添加@a WeatherActorClass类型的天气actor，
  ///并将@a SelectedWeather设置为@a MetaInfo中指定的映射。
  ///如果角色已经存在于地图上，则返回它，
  ///因此在每个地图中只生成一个天气角色
  UFUNCTION(Category="MapGenerator|Weather", BlueprintCallable)
  AActor* AddWeatherToExistingMap(TSubclassOf<class AActor> WeatherActorClass, 
      const FMapGeneratorMetaInfo& MetaInfo, const FString SelectedWeather);

  UFUNCTION(Category="MapGenerator|ROIs", BlueprintCallable)
  TMap<FRoiTile, FVegetationROI> CreateVegetationRoisMap(TArray<FVegetationROI> VegetationRoisArray);

  UFUNCTION(Category="MapGenerator|ROIs", BlueprintCallable)
  TMap<FRoiTile, FTerrainROI> CreateTerrainRoisMap(TArray<FTerrainROI> TerrainRoisArray);

  UFUNCTION(Category="MapGenerator|ROIs", BlueprintCallable)
  TMap<FRoiTile, FMiscSpreadedActorsROI> CreateMiscSpreadedActorsRoisMap(TArray<FMiscSpreadedActorsROI> SpreadedActorsRoisArray);

  UFUNCTION(Category="MapGenerator|ROIs", BlueprintCallable)
  TMap<FRoiTile, FMiscSpecificLocationActorsROI> CreateMiscSpecificLocationActorsRoisMap(TArray<FMiscSpecificLocationActorsROI> SpecificLocationActorsRoisArray);

  UFUNCTION(Category="MapGenerator|ROIs", BlueprintCallable)
  TMap<FRoiTile, FSoilTypeROI> CreateSoilTypeRoisMap(TArray<FSoilTypeROI> SoilTypeRoisArray);

  UFUNCTION(Category="MapGenerator|Vegetation", BlueprintCallable)
  bool DeleteAllVegetationInMap(const FString Path, const FString MapName);

  UFUNCTION(Category="MapGenerator|JsonLibrary", BlueprintCallable)
  bool GenerateWidgetStateFileFromStruct(FMapGeneratorWidgetState WidgetState, const FString JsonPath);

  UFUNCTION(Category="MapGenerator|JsonLibrary", BlueprintCallable)
  FMapGeneratorWidgetState LoadWidgetStateStructFromFile(const FString JsonPath);

  UFUNCTION(Category="MapGenerator|JsonLibrary|Misc", BlueprintCallable)
  bool GenerateMiscStateFileFromStruct(FMiscWidgetState MiscState, const FString JsonPath);

  UFUNCTION(Category="MapGenerator|JsonLibrary|Misc", BlueprintCallable)
  FMiscWidgetState LoadMiscStateStructFromFile(const FString JsonPath);

  UFUNCTION(Category="MapGenerator|TerrainPresets", BlueprintCallable)
  bool GenerateTerrainPresetFileFromStruct(FMapGeneratorPreset Preset, const FString JsonPath);

  UFUNCTION(Category="MapGenerator|TerrainPresets", BlueprintCallable)
  FMapGeneratorPreset LoadTerrainPresetStructFromFile(const FString JsonPath);

private:  
  UPROPERTY()
  TMap<FRoiTile, FTileBoundariesInfo> BoundariesInfo;

  ///加载一堆位于@a BaseMapPath中的世界对象，
  ///并在@a WorldAssetsData中返回它们。
  ///函数成功返回true，否则返回false
  UFUNCTION()
  bool LoadWorlds(TArray<FAssetData>& WorldAssetsData, const FString& BaseMapPath, bool bRecursive = true);

  /// 保存@a worldtobessaved。
  ///它还保存所有脏包，以确保正确管理景观
  UFUNCTION()
  bool SaveWorld(UWorld* WorldToBeSaved);

  //包裹功能
  //bool保存世界包

  /// 从@a MetaInfo获取地图的名称并创建
  ///主地图包括大型地图系统所需的所有角色
  UFUNCTION()
  bool CreateMainLargeMap(const FMapGeneratorMetaInfo& MetaInfo);

  /// 为@a MetaInfo中指定的已创建的地图创建一个开放驱动文件
  UFUNCTION()
  bool CreateOpenDriveFile(const FMapGeneratorMetaInfo& MetaInfo);

  /// 以@a MetaInfo作为输入，
  ///并根据为地图指定的尺寸生成所有的贴图。
  ///函数返回true表示成功，否则返回false
  UFUNCTION()
  bool CreateTilesMaps(const FMapGeneratorMetaInfo& MetaInfo);

  /// 在@a MetaInfo的指定路径中搜索指定的地图，
  ///并为每个贴图启动植被烹饪过程。
  ///重要提示：只有带有“Tile”标签的地图才会被处理，
  ///因为植被只会被应用到贴图上。
  ///如果成功，函数返回true，否则返回false
  UFUNCTION()
  bool CookVegetationToTiles(const FMapGeneratorMetaInfo& MetaInfo);

  ///为@a FoliageSpawners 中的每个元素实例化一个过程叶子体积，
  ///并将相应的植被烹饪到@a World 
  ///如果成功返回true，否则返回false
  UFUNCTION()
  bool CookVegetationToWorld(UWorld* World, const TArray<UProceduralFoliageSpawner*> FoliageSpawners);

  UFUNCTION()
  bool CookMiscSpreadedActors(const FMapGeneratorMetaInfo& MetaInfo);

  ///返回@a WorldAssetData中的世界对象
  UFUNCTION()
  UWorld* GetWorldFromAssetData(FAssetData& WorldAssetData);

  /// 返回@a World中位于@a x和@a y位置的风景的Z坐标。
  UFUNCTION()
  float GetLandscapeSurfaceHeight(UWorld* World, float x, float y, bool bDrawDebugLines);

  UFUNCTION()
  float GetLandscapeSurfaceHeightFromRayCast(UWorld* World, float x, float y, bool bDrawDebugLines);

  UFUNCTION()
  void ExtractCoordinatedFromMapName(const FString MapName, int& X, int& Y);

  UFUNCTION()
  void SmoothHeightmap(TArray<uint16> HeightData, TArray<uint16>& OutHeightData);

  UFUNCTION()
  void SewUpperAndLeftTiles(TArray<uint16> HeightData, TArray<uint16>& OutHeightData, int IndexX, int IndexY);

  // 将2D坐标转换为1D坐标
  UFUNCTION()
  FORCEINLINE int Convert2DTo1DCoord(int IndexX, int IndexY, int TileSize)
  {
    return (IndexX * TileSize) + IndexY;
  }
};
