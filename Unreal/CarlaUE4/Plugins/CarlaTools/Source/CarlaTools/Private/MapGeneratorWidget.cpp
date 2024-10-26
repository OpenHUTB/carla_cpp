// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MapGeneratorWidget.h"

#include "ActorFactories/ActorFactory.h"
#include "AssetRegistryModule.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "Carla/MapGen/SoilTypeManager.h"
#include "Carla/Weather/Weather.h"
#include "Carla/Vehicle/CustomTerrainPhysicsComponent.h"
#include "Components/SplineComponent.h"
#include "Editor/FoliageEdit/Public/FoliageEdMode.h"
#include "EditorLevelLibrary.h"
#include "FileHelpers.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFilemanager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "Misc/FileHelper.h"
#include "Misc/CString.h"
#include "ProceduralFoliageComponent.h"
#include "ProceduralFoliageVolume.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"

#include "EditorAssetLibrary.h"
#include "EngineUtils.h"
#include "ObjectEditorUtils.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/ObjectMacros.h"

#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"

#define CUR_CLASS_FUNC (FString(__FUNCTION__))
#define CUR_LINE  (FString::FromInt(__LINE__))
#define CUR_CLASS_FUNC_LINE (CUR_CLASS_FUNC + "::" + CUR_LINE)

#define TAG_SPREADED FName("Spreaded Actor")
#define TAG_SPECIFIC_LOCATION FName("Specific Location Actor")

#define TILESIZE 1009

#undef CreateDirectory
#undef CopyFile

DEFINE_LOG_CATEGORY(LogCarlaToolsMapGenerator);

void UMapGeneratorWidget::GenerateMapFiles(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Starting Map Generation %s %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.DestinationPath, *MetaInfo.MapName);

  // // 1. 创建瓦片地形。
  bool bTilesSuccess = CreateTilesMaps(MetaInfo);
  if(!bTilesSuccess)
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Creating Tile for %s"), 
        *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);

  // 2.创建主大型地图。
  bool bLargeMapSuccess = CreateMainLargeMap(MetaInfo);
  if(!bLargeMapSuccess)
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error creating Main Large Map for %s"), 
        *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);

  // 3. 开放驱动辅助文件
  bool bOpenDriveCopySuccess = CreateOpenDriveFile(MetaInfo);
  if(!bOpenDriveCopySuccess)
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error creating OpenDrive file for %s"), 
        *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);

}

void UMapGeneratorWidget::CookVegetation(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Starting Cooking Vegetation to Tiles in %s %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.DestinationPath, *MetaInfo.MapName);
  // 3. 给瓦片添加植被。
  bool bVegetationSuccess = CookVegetationToTiles(MetaInfo);
  if(!bVegetationSuccess){
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Cooking vegetation for %s"), 
        *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: SUCCESS Cooking Vegetation to Tiles in %s %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.DestinationPath, *MetaInfo.MapName);
  }
}

void UMapGeneratorWidget::CookSoilTypeToMaps(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Starting Cooking Soil Type to Tiles in %s %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.DestinationPath, *MetaInfo.MapName);

  // 检查地图是否有效。
  const FString MapCompletePath = MetaInfo.DestinationPath + "/" + MetaInfo.MapName;
  const FString MapPackageFileName = FPackageName::LongPackageNameToFilename(
      MapCompletePath, 
      FPackageName::GetMapPackageExtension());

  if(!FPaths::FileExists(*MapPackageFileName))
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Soil Terramechanics cannot be applied to a non existing map"), 
        *CUR_CLASS_FUNC_LINE);
    return;
  }

  //在主地图中实例化天气角色。
  const FString WorldToLoadPath = MapCompletePath + "." + MetaInfo.MapName;
  UWorld* World = LoadObject<UWorld>(nullptr, *WorldToLoadPath);

  ASoilTypeManager* SoilTypeManagerActor = (ASoilTypeManager*) UGameplayStatics::GetActorOfClass(World, ASoilTypeManager::StaticClass());

  SoilTypeManagerActor->ClearTerrainPropertiesMap();

  //设置常规设置。
  SoilTypeManagerActor->SetGeneralTerrainProperties(MetaInfo.GeneralSoilType);

  for(TPair<FRoiTile, FSoilTypeROI> SoilROIPair : MetaInfo.SoilTypeRoisMap)
  {
    FRoiTile SoilROITile = SoilROIPair.Key;
    FSoilTypeROI SoilROI = SoilROIPair.Value;
    SoilTypeManagerActor->AddTerrainPropertiesToTile(SoilROITile.X, SoilROITile.Y, SoilROI.SoilProperties);
  }

  World->GetPackage()->SetDirtyFlag(true);
  SaveWorld(World);
}

void UMapGeneratorWidget::CookMiscSpreadedInformationToTiles(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Starting Cooking Miscellaneous Info to Tiles in %s %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.DestinationPath, *MetaInfo.MapName);

  // 分散的角色（感兴趣区域）。
  bool bSpreadedSuccess = CookMiscSpreadedActors(MetaInfo);
  if(!bSpreadedSuccess)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Miscellaneous Spreaded Actor cooking was not successful..."), 
        *CUR_CLASS_FUNC_LINE);
  }
}

void UMapGeneratorWidget::CookMiscSpecificLocationInformationToTiles(const FMapGeneratorMetaInfo& MetaInfo)
{
  UWorld* World = GEditor->GetEditorWorldContext().World();

  //一次仅支持一个感兴趣区域。
  TArray<FMiscSpecificLocationActorsROI> ActorROIArray;
  MetaInfo.MiscSpecificLocationActorsRoisMap.GenerateValueArray(ActorROIArray);

  if(ActorROIArray.Num() > 0)
  {
    FMiscSpecificLocationActorsROI ActorROI = ActorROIArray[0];

    FRotator Rotation(0, FMath::RandRange(ActorROI.MinRotationRange, ActorROI.MaxRotationRange), 0);
    FActorSpawnParameters SpawnInfo;

    AActor* SpawnedActor =  World->SpawnActor<AActor>(
        ActorROI.ActorClass, 
        ActorROI.ActorLocation, 
        Rotation, 
        SpawnInfo);
    SpawnedActor->Tags.Add(TAG_SPECIFIC_LOCATION);

    SaveWorld(World);
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Miscellaneous Specific Location Actor cooking was not successful..."), 
        *CUR_CLASS_FUNC_LINE);
  }

}

void UMapGeneratorWidget::DeleteAllSpreadedActors(const FMapGeneratorMetaInfo& MetaInfo)
{
  TArray<FAssetData> AssetsData;
  const FString TilesPath = MetaInfo.DestinationPath;
  bool success = LoadWorlds(AssetsData, TilesPath);

  for(FAssetData AssetData : AssetsData)
  {
    UWorld* World = GetWorldFromAssetData(AssetData);

    // 检查它是否不是一块（瓷砖/图块）。
    if(!World->GetMapName().Contains("_Tile_"))
    {
      UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: %s Skipped as it is not a tile"), 
          *CUR_CLASS_FUNC_LINE, *World->GetMapName());
      continue;
    }

    TArray<AActor*> TaggedActors; 
    UGameplayStatics::GetAllActorsWithTag(World, TAG_SPREADED, TaggedActors);

    for(AActor* Actor : TaggedActors)
    {
      Actor->Destroy();
    }

    SaveWorld(World);
  }
}

void UMapGeneratorWidget::CookVegetationToCurrentTile(const TArray<UProceduralFoliageSpawner*> FoliageSpawners)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, 
      TEXT("%s: Cooking vegetation to current tile. Vegetation type num: %d"), 
      *CUR_CLASS_FUNC_LINE, FoliageSpawners.Num());

  bool result = CookVegetationToWorld(GEditor->GetEditorWorldContext().World(), FoliageSpawners);
  if(!result)
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Cooking vegetation for Current Tile"), 
        *CUR_CLASS_FUNC_LINE);
}

bool UMapGeneratorWidget::RecalculateCollision()
{
  UWorld* World = GEditor->GetEditorWorldContext().World();
  if(World == nullptr)
    return false;
    
  ALandscape* Landscape = (ALandscape*) UGameplayStatics::GetActorOfClass(
      World, 
      ALandscape::StaticClass());

  Landscape->RecreateCollisionComponents();
  Landscape->ReregisterAllComponents();

  SaveWorld(World);

  return true;
}

void UMapGeneratorWidget::CookTilesCollisions(const FMapGeneratorMetaInfo& MetaInfo)
{
  for (int i = 0; i < MetaInfo.SizeX; i++)
  {
    for (int j = 0; j < MetaInfo.SizeY; j++)
    {
      const FString MapName =
          MetaInfo.MapName + "_Tile_" + FString::FromInt(i) + "_" + FString::FromInt(j);
      const FString MapNameToLoad = MetaInfo.DestinationPath + "/" + MapName + "." + MapName;

      bool bLoadedSuccess = FEditorFileUtils::LoadMap(*MapNameToLoad, false, true);

      if (!bLoadedSuccess)
        UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Error loading to %s tiles"),
            *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);

      UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();

      bool bRecalculateResult = RecalculateCollision();

      if (!bRecalculateResult)
        UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error recalculating to %s tiles"),
            *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);
    }
  }
}

FString UMapGeneratorWidget::SanitizeDirectory(FString InDirectory)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Sanitazing directory: %s"), 
        *CUR_CLASS_FUNC_LINE, *InDirectory);
  // 检查 metaInfo 中的目录不能以“/”结尾，如果是，则删除它。
  while(InDirectory.EndsWith("/") || InDirectory.EndsWith("\\"))
  {
    if(InDirectory.EndsWith("/"))
    {
      InDirectory.RemoveFromEnd("/");
    }
    else if(InDirectory.EndsWith("\\"))
    {
      InDirectory.RemoveFromEnd("\\");
    }
  }
  return InDirectory;
}

bool UMapGeneratorWidget::LoadMapInfoFromPath(FString InDirectory, int& OutMapSize, FString& OutFoundMapName)
{
  TArray<FAssetData> DirectoryAssets;
  bool bLoaded = LoadWorlds(DirectoryAssets, InDirectory, false);

  FString MainMapName;
  FString MapNameInTiles;

  if(bLoaded && DirectoryAssets.Num() > 0)
  {
    for(FAssetData AssetData : DirectoryAssets) // Find name in tiles
    {
      const FString AssetName = AssetData.AssetName.ToString();
      FString Name, Coordinates;
      if(AssetName.Split(TEXT("_Tile_"), &Name, &Coordinates))
      {
        MapNameInTiles = Name;
        break;
      }
    }

    if(MapNameInTiles.IsEmpty()) // No tiles found
    {
      return false;
    }

    for(FAssetData AssetData : DirectoryAssets)
    {
      const FString AssetName = AssetData.AssetName.ToString();
      if(!AssetName.Contains("_Tile_") && AssetName == MapNameInTiles) // Find Main Map
      {
        MainMapName = AssetName;
        break;
      }
    }

    if(MainMapName.IsEmpty()) // No main map found
    {
      return false;
    }

    int NumberOfTiles = DirectoryAssets.Num() - 1;
    OutFoundMapName = MainMapName;
    OutMapSize = (int)UKismetMathLibrary::Sqrt((float)NumberOfTiles);
  }
  else
  {
    // 未找到地图。
    return false;
  }

  return true;
}

AActor* UMapGeneratorWidget::GenerateWater(TSubclassOf<class AActor> RiverClass)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Starting Generating Waterbodies"), 
        *CUR_CLASS_FUNC_LINE);

  UWorld* World = GetWorld();

  float XCoord = 2000;
  float YCoord = 2000;

  float ZRot = 50;

  float ActorZCoord = GetLandscapeSurfaceHeight(World, XCoord, YCoord, false);
  FVector Location(XCoord, YCoord, ActorZCoord+5); // Auxiliar values for x and y coords
  FRotator Rotation(0, ZRot, 0);
  FActorSpawnParameters SpawnInfo;
  
  
  AActor* RiverActor =  World->SpawnActor<AActor>(
      RiverClass, 
      Location, 
      Rotation, 
      SpawnInfo);

  USplineComponent* RiverSpline = RiverActor->FindComponentByClass<USplineComponent>();

  int NumberOfPoints = RiverSpline->GetNumberOfSplinePoints();

  for(int i = 0; i < NumberOfPoints; i++)
  {
    FVector PointPosition = RiverSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
    PointPosition.Z = GetLandscapeSurfaceHeight(World, PointPosition.X, PointPosition.Y, false);
    RiverSpline->SetLocationAtSplinePoint(i, PointPosition, ESplineCoordinateSpace::World, true);
  }

  return RiverActor;
}

bool UMapGeneratorWidget::GenerateWaterFromWorld(UWorld* RiversWorld, TSubclassOf<class AActor> RiverClass)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Starting Generating Rivers from world %s"), 
        *CUR_CLASS_FUNC_LINE, *RiversWorld->GetMapName());

  TArray<AActor*> RiversActors;
  UGameplayStatics::GetAllActorsOfClass(RiversWorld, RiverClass, RiversActors);

  if(RiversActors.Num() <= 0)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: No Rivers Found in %s"), 
        *CUR_CLASS_FUNC_LINE, *RiversWorld->GetMapName());
    return false;
  }

  float RiverSurfaceDisplacement = 100.0f;
  for(AActor* RiverActor : RiversActors)
  {
    USplineComponent* RiverSpline = dynamic_cast<USplineComponent*>(RiverActor->GetComponentByClass(USplineComponent::StaticClass()));
    for(int i = 0; i < RiverSpline->GetNumberOfSplinePoints(); i++)
    {
      FVector SplinePosition = RiverSpline->GetWorldLocationAtSplinePoint(i);
      SplinePosition.Z = GetLandscapeSurfaceHeight(RiversWorld, SplinePosition.X, SplinePosition.Y, false) + RiverSurfaceDisplacement;
      RiverSpline->SetWorldLocationAtSplinePoint(i, SplinePosition);
    }
    UpdateRiverActorSplinesEvent(RiverActor);
  }
  return true;
}

UWorld* UMapGeneratorWidget::DuplicateWorld(FString BaseWorldPath, FString TargetWorldPath, const FString NewWorldName)
{
  UWorld* DuplicateWorld;

  UWorld* BaseWorld = LoadObject<UWorld>(nullptr, *BaseWorldPath);
  if(BaseWorld == nullptr)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: No World Found in %s"), 
        *CUR_CLASS_FUNC_LINE, *BaseWorldPath);
    return nullptr;
  }

  const FString PackageName = TargetWorldPath + "/" + NewWorldName;
  UPackage* WorldPackage = CreatePackage(*PackageName);

  FObjectDuplicationParameters Parameters(BaseWorld, WorldPackage);
  Parameters.DestName = FName(*NewWorldName);
  Parameters.DestClass = BaseWorld->GetClass();
  Parameters.DuplicateMode = EDuplicateMode::World;
  Parameters.PortFlags = PPF_Duplicate;

  DuplicateWorld = CastChecked<UWorld>(StaticDuplicateObjectEx(Parameters));

  const FString PackageFileName = FPackageName::LongPackageNameToFilename(
          PackageName, 
          FPackageName::GetMapPackageExtension());
      UPackage::SavePackage(WorldPackage, DuplicateWorld, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
          *PackageFileName, GError, nullptr, true, true, SAVE_NoError);

  return DuplicateWorld;
}

AActor* UMapGeneratorWidget::AddWeatherToExistingMap(TSubclassOf<class AActor> WeatherActorClass, 
    const FMapGeneratorMetaInfo& MetaInfo, const FString SelectedWeather)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Generating Weather to %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);

  //检查地图是否有效。
  const FString MapCompletePath = MetaInfo.DestinationPath + "/" + MetaInfo.MapName;
  const FString MapPackageFileName = FPackageName::LongPackageNameToFilename(
      MapCompletePath, 
      FPackageName::GetMapPackageExtension());
    
  if(!FPaths::FileExists(*MapPackageFileName))
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Weather cannot be applied to a non existing map"), 
        *CUR_CLASS_FUNC_LINE);
    return nullptr;
  }

  //在主地图中实例化天气角色。
  const FString WorldToLoadPath = MapCompletePath + "." + MetaInfo.MapName;
  UWorld* World = LoadObject<UWorld>(nullptr, *WorldToLoadPath);

  AActor* WeatherActor = UGameplayStatics::GetActorOfClass(World, AWeather::StaticClass());

  if(WeatherActor == nullptr)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Creating a new weather actor to world"), 
        *CUR_CLASS_FUNC_LINE);

    WeatherActor = World->SpawnActor<AActor>(WeatherActorClass);
  }

  World->GetPackage()->SetDirtyFlag(true);

  return WeatherActor;  
}

TMap<FRoiTile, FVegetationROI> UMapGeneratorWidget::CreateVegetationRoisMap(TArray<FVegetationROI> VegetationRoisArray)
{
  TMap<FRoiTile, FVegetationROI> ResultMap;
  for(FVegetationROI VegetationRoi : VegetationRoisArray)
  {
    for(FRoiTile VegetationRoiTile : VegetationRoi.TilesList)
    {
      ResultMap.Add(VegetationRoiTile, VegetationRoi);
    }
  }
  return ResultMap;
}

TMap<FRoiTile, FTerrainROI> UMapGeneratorWidget::CreateTerrainRoisMap(TArray<FTerrainROI> TerrainRoisArray)
{
  TMap<FRoiTile, FTerrainROI> ResultMap;
  for(FTerrainROI TerrainRoi : TerrainRoisArray)
  {
    for(FRoiTile TerrainRoiTile : TerrainRoi.TilesList)
    {
      if(ResultMap.Contains(TerrainRoiTile))
      {
        ResultMap[TerrainRoiTile] = TerrainRoi;
      }
      else
      {
        ResultMap.Add(TerrainRoiTile, TerrainRoi);
      }
    }
  }
  return ResultMap;
}

TMap<FRoiTile, FMiscSpreadedActorsROI> UMapGeneratorWidget::CreateMiscSpreadedActorsRoisMap(TArray<FMiscSpreadedActorsROI> SpreadedActorsRoisArray)
{
  TMap<FRoiTile, FMiscSpreadedActorsROI> ResultMap;
  for(FMiscSpreadedActorsROI SpreadedRoi : SpreadedActorsRoisArray)
  {
    for(FRoiTile SpreadedRoiTile : SpreadedRoi.TilesList)
    {
      ResultMap.Add(SpreadedRoiTile, SpreadedRoi);
    }
  }
  return ResultMap;
}

TMap<FRoiTile, FMiscSpecificLocationActorsROI> UMapGeneratorWidget::CreateMiscSpecificLocationActorsRoisMap(TArray<FMiscSpecificLocationActorsROI> SpecificLocationActorsRoisArray)
{
  TMap<FRoiTile, FMiscSpecificLocationActorsROI> ResultMap;
  for(FMiscSpecificLocationActorsROI SpecificLocationRoi : SpecificLocationActorsRoisArray)
  {
    for(FRoiTile SpecificLocationRoiTile : SpecificLocationRoi.TilesList)
    {
      ResultMap.Add(SpecificLocationRoiTile, SpecificLocationRoi);
    }
  }
  return ResultMap;
}

TMap<FRoiTile, FSoilTypeROI> UMapGeneratorWidget::CreateSoilTypeRoisMap(TArray<FSoilTypeROI> SoilTypeRoisArray)
{
  TMap<FRoiTile, FSoilTypeROI> ResultMap;
  for(FSoilTypeROI SoilTypeRoi : SoilTypeRoisArray)
  {
    for(FRoiTile SoilTypeRoiTile : SoilTypeRoi.TilesList)
    {
      ResultMap.Add(SoilTypeRoiTile, SoilTypeRoi);
    }
  }
  return ResultMap;
}

bool UMapGeneratorWidget::DeleteAllVegetationInMap(const FString Path, const FString MapName)
{
  TArray<FAssetData> AssetsData;
  const FString TilesPath = Path;
  bool success = LoadWorlds(AssetsData, TilesPath);
  if(!success || AssetsData.Num() <= 0)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("No Tiles found in %s. Vegetation cooking Aborted!"), *TilesPath);
    return false;
  }

  // 为每一张地图烘焙植被。
  for(FAssetData AssetData : AssetsData)
  {
    UWorld* World = GetWorldFromAssetData(AssetData);
    TArray<AActor*> FoliageVolumeActors;
    UGameplayStatics::GetAllActorsOfClass(World, AProceduralFoliageVolume::StaticClass(), FoliageVolumeActors);
    for(AActor* FoliageActor : FoliageVolumeActors)
    {
      FoliageActor->Destroy();
    }

    SaveWorld(World);
  }

  return true;
}

bool UMapGeneratorWidget::GenerateWidgetStateFileFromStruct(FMapGeneratorWidgetState WidgetState, const FString JsonPath)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Creating Widget State JSON"), 
      *CUR_CLASS_FUNC_LINE);

  TSharedRef<FJsonObject> OutJsonObject = MakeShareable(new FJsonObject());
  FJsonObjectConverter::UStructToJsonObject(FMapGeneratorWidgetState::StaticStruct(), &WidgetState, OutJsonObject, 0, 0);

  FString OutputJsonString;
  TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutputJsonString);
  FJsonSerializer::Serialize(OutJsonObject, JsonWriter);

  FFileHelper::SaveStringToFile(OutputJsonString, *JsonPath);

  return true;
}

FMapGeneratorWidgetState UMapGeneratorWidget::LoadWidgetStateStructFromFile(const FString JsonPath)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Creating Widget State Struct from JSON"), 
      *CUR_CLASS_FUNC_LINE);

  FMapGeneratorWidgetState WidgetState;
  FString File;
  FFileHelper::LoadFileToString(File, *JsonPath);

  TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(*File);
  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
  bool bDeserializeSuccess = FJsonSerializer::Deserialize(JsonReader, JsonObject, FJsonSerializer::EFlags::None);

  if (bDeserializeSuccess && JsonObject.IsValid())
	{
    FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), FMapGeneratorWidgetState::StaticStruct(), &WidgetState, 1, 0);
  }

  return WidgetState;
}

bool UMapGeneratorWidget::GenerateMiscStateFileFromStruct(FMiscWidgetState MiscState, const FString JsonPath)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Creating Miscellaneous State JSON"), 
      *CUR_CLASS_FUNC_LINE);

  TSharedRef<FJsonObject> OutJsonObject = MakeShareable(new FJsonObject());
  FJsonObjectConverter::UStructToJsonObject(FMiscWidgetState::StaticStruct(), &MiscState, OutJsonObject, 0, 0);

  FString OutputJsonString;
  TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutputJsonString);
  FJsonSerializer::Serialize(OutJsonObject, JsonWriter);

  FFileHelper::SaveStringToFile(OutputJsonString, *JsonPath);

  return true;
}

FMiscWidgetState UMapGeneratorWidget::LoadMiscStateStructFromFile(const FString JsonPath)
{
  FMiscWidgetState MiscState;
  FString File;
  FFileHelper::LoadFileToString(File, *JsonPath);

  TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(*File);
  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
  bool bDeserializeSuccess = FJsonSerializer::Deserialize(JsonReader, JsonObject, FJsonSerializer::EFlags::None);

  if(bDeserializeSuccess && JsonObject.IsValid())
  {
    FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), FMiscWidgetState::StaticStruct(), &MiscState, 1, 0);
  }

  return MiscState;
}

bool UMapGeneratorWidget::GenerateTerrainPresetFileFromStruct(FMapGeneratorPreset Preset, const FString JsonPath)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Creating Terrain Preset State JSON"), 
      *CUR_CLASS_FUNC_LINE);

  TSharedRef<FJsonObject> OutJsonObject = MakeShareable(new FJsonObject());
  FJsonObjectConverter::UStructToJsonObject(FMapGeneratorPreset::StaticStruct(), &Preset, OutJsonObject, 0, 0);

  FString OutputJsonString;
  TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutputJsonString);
  FJsonSerializer::Serialize(OutJsonObject, JsonWriter);

  FFileHelper::SaveStringToFile(OutputJsonString, *JsonPath);

  return true;
}

FMapGeneratorPreset UMapGeneratorWidget::LoadTerrainPresetStructFromFile(const FString JsonPath)
{
  FMapGeneratorPreset Preset;
  FString File;
  FFileHelper::LoadFileToString(File, *JsonPath);

  TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(*File);
  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
  bool bDeserializeSuccess = FJsonSerializer::Deserialize(JsonReader, JsonObject, FJsonSerializer::EFlags::None);

  if(bDeserializeSuccess && JsonObject.IsValid())
  {
    FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), FMapGeneratorPreset::StaticStruct(), &Preset, 1, 0);
  }

  return Preset;
}

bool UMapGeneratorWidget::LoadWorlds(TArray<FAssetData>& WorldAssetsData, const FString& BaseMapPath, bool bRecursive)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Loading Worlds from %s"), 
      *CUR_CLASS_FUNC_LINE, *BaseMapPath);

  TArray<FAssetData> AssetsData;
  UObjectLibrary *MapObjectLibrary;

  //使用对象库从文件夹中加载地图。
  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  MapObjectLibrary->bRecursivePaths = bRecursive;
  MapObjectLibrary->AddToRoot();
  MapObjectLibrary->LoadAssetDataFromPath(*BaseMapPath);
  MapObjectLibrary->LoadAssetsFromAssetData();
  MapObjectLibrary->GetAssetDataList(AssetsData);

  if (AssetsData.Num() > 0)
  {
    // 返回在目录中找到的所有世界资产的完整列表。
    WorldAssetsData = AssetsData;
    return true;
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: No Worlds found in %s"), 
      *CUR_CLASS_FUNC_LINE, *BaseMapPath);
    return false;
  }
}

bool UMapGeneratorWidget::SaveWorld(UWorld* WorldToBeSaved)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Saving World to %s"), 
      *CUR_CLASS_FUNC_LINE, *WorldToBeSaved->GetPathName());

  bool bSaveAssetSuccess = UEditorAssetLibrary::SaveAsset(WorldToBeSaved->GetPathName(), true);
  bool bSaveDirtySuccess = FEditorFileUtils::SaveDirtyPackages(true, true, true, true, true, true);
  return bSaveAssetSuccess && bSaveDirtySuccess;
}

bool UMapGeneratorWidget::CreateMainLargeMap(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Creating %s main large map in %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName, *MetaInfo.DestinationPath);
  FAssetData WorldAssetData;

  const FString BaseWorldSearchPath = TEXT("/CarlaTools/MapGenerator/BaseMap/MainLargeMap/MapGeneratorBaseLargeMap.MapGeneratorBaseLargeMap");
  UWorld* BaseWorld = LoadObject<UWorld>(nullptr, *BaseWorldSearchPath);
  const FString PackageName = MetaInfo.DestinationPath + "/" + MetaInfo.MapName;

  UPackage* BaseMapPackage = CreatePackage(*PackageName);
  FObjectDuplicationParameters Parameters(BaseWorld, BaseMapPackage);
  Parameters.DestName = FName(*MetaInfo.MapName);
  Parameters.DestClass = BaseWorld->GetClass();
  Parameters.DuplicateMode = EDuplicateMode::World;
  Parameters.PortFlags = PPF_Duplicate;

  UWorld* World = CastChecked<UWorld>(StaticDuplicateObjectEx(Parameters));

  const FString PackageFileName = FPackageName::LongPackageNameToFilename(
      PackageName, 
      FPackageName::GetMapPackageExtension());
  UPackage::SavePackage(BaseMapPackage, World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName, GError, nullptr, true, true, SAVE_NoError);


  bool bLoadedSuccess = FEditorFileUtils::LoadMap(*PackageName, false, true);
    if(!bLoadedSuccess){
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Loading %s"),
          *CUR_CLASS_FUNC_LINE, *PackageName);
      return false;
    }

  AActor* LargeMapManagerActor = UGameplayStatics::GetActorOfClass(World, ALargeMapManager::StaticClass());
  if(LargeMapManagerActor == nullptr)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, 
        TEXT("%s: Could not find LargeMapManager Actor in %s."),
        *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName); 
    return false;
  }

  ALargeMapManager* LargeMapManager = StaticCast<ALargeMapManager*>(LargeMapManagerActor);
  if(LargeMapManager == nullptr)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, 
        TEXT("%s: Failed to cast Large Map Actor in %s."),
        *MetaInfo.MapName); 
    return false;
  }

  LargeMapManager->LargeMapTilePath = MetaInfo.DestinationPath;
  LargeMapManager->LargeMapName = MetaInfo.MapName;

  // 将 Tile0Offset 设置为 0 以烘焙图块信息。
  FVector OriginalTile0Offset = LargeMapManager->GetTile0Offset();
  LargeMapManager->SetTile0Offset(FVector(0.0f, 0.0f, 0.0f));

  LargeMapManager->GenerateMap_Editor();

  //将 Tile0Offset 重置为原始的图块中间位置以用于运行时操作。
  LargeMapManager->SetTile0Offset(OriginalTile0Offset);

  // 如果已定义，则生成植被管理器。
  if(MetaInfo.VegetationManagerBpClass)
  {
    AActor* VegetationManagerActor = World->SpawnActor(MetaInfo.VegetationManagerBpClass);
  }

  // 如果已定义，则生成地形力学管理器。
  if(MetaInfo.TerramechanicsBpClass)
  {
    AActor* TerramechanicsActor = World->SpawnActor(MetaInfo.TerramechanicsBpClass);
    UActorComponent* ActorComponent = TerramechanicsActor->FindComponentByClass(UCustomTerrainPhysicsComponent::StaticClass());
    UCustomTerrainPhysicsComponent* TerramechanicsActorComponent = StaticCast<UCustomTerrainPhysicsComponent*>(ActorComponent);
    TerramechanicsActorComponent->WorldSize = FVector(100800,100800,0);
    TerramechanicsActorComponent->DrawDebugInfo = false;
    TerramechanicsActorComponent->bUpdateParticles = true;

  }

  UPackage::SavePackage(BaseMapPackage, World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName, GError, nullptr, true, true, SAVE_NoError);

  return true;
}

bool UMapGeneratorWidget::CreateOpenDriveFile(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Warning,
      TEXT("%s: Creating OpenDrive file for %s. This is needed but deprecated for offroad maps."), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);
  
  FString PrunedPath = MetaInfo.DestinationPath.Replace(TEXT("/Game/"), TEXT(""));
  const FString DestinationPath = FPaths::ConvertRelativePathToFull(
      FPaths::ProjectContentDir().Append(PrunedPath + "/OpenDrive/"));
  const FString DestinationPathWithFile = FPaths::ConvertRelativePathToFull(
      FPaths::ProjectContentDir().Append(PrunedPath + "/OpenDrive/" + MetaInfo.MapName + ".xodr"));
  const FString SourcePath = FPaths::ProjectPluginsDir()
      .Append(TEXT("CarlaTools/Content/MapGenerator/Misc/OpenDrive/TemplateOpenDrive.xodr"));
  ;

  IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

  return FileManager.CreateDirectory(*DestinationPath)
      && FileManager.CopyFile(*DestinationPathWithFile,*SourcePath, EPlatformFileRead::None, EPlatformFileWrite::None);
}

bool UMapGeneratorWidget::CreateTilesMaps(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Creating %s tiles maps in %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName, *MetaInfo.DestinationPath);

   const FString BaseWorldSearchPath = TEXT("/CarlaTools/MapGenerator/BaseMap/TilesEmpty/BaseTileEmpty.BaseTileEmpty");

  for(int i = 0; i < MetaInfo.SizeX; i++)
  {
    for(int j = 0; j < MetaInfo.SizeY; j++)
    {
      UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Creating tile map %s (%d_%d)"), 
          *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName, i, j);

      UWorld* BaseWorld = LoadObject<UWorld>(nullptr, *BaseWorldSearchPath);

      const FString MapName = 
          MetaInfo.MapName + "_Tile_" + FString::FromInt(i) + "_" + FString::FromInt(j);  
      const FString PackageName = MetaInfo.DestinationPath + "/" + MapName;

      UPackage* TilePackage = CreatePackage(*PackageName);
      FObjectDuplicationParameters Parameters(BaseWorld, TilePackage);
      Parameters.DestName = FName(*MapName);
      Parameters.DestClass = BaseWorld->GetClass();
      Parameters.DuplicateMode = EDuplicateMode::World;
      Parameters.PortFlags = PPF_Duplicate;

      UWorld* World = CastChecked<UWorld>(StaticDuplicateObjectEx(Parameters));

      if (World == nullptr)
        UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error. No world found in tile %s %d_%d"), 
            *CUR_CLASS_FUNC_LINE, *World->GetName(), i, j);

      // 2017 x 2017
      ALandscape* Landscape = World->SpawnActor<ALandscape>();
      Landscape->ComponentSizeQuads = 126; // Component Size
      Landscape->SubsectionSizeQuads = 63; // Quads / Section
      Landscape->NumSubsections = 2;       // (1 for 1x1 , 2 for 2x2)
      Landscape->SetLandscapeGuid(FGuid::NewGuid());

      // 高度渲染目标。
      UTextureRenderTarget2D* HeightRT = MetaInfo.GlobalHeightmap;
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: Heightmap detected with dimensions %dx%d"), 
            *CUR_CLASS_FUNC_LINE, HeightRT->SizeX, HeightRT->SizeY);
      TArray<uint16> HeightData;
      
      FMapGeneratorTileMetaInfo TileMetaInfo;
      TileMetaInfo.IndexX = i;
      TileMetaInfo.IndexY = MetaInfo.SizeY-j-1;
      TileMetaInfo.MapMetaInfo = MetaInfo;
      

      //河流管理。
      if(FMath::RandRange(0.0f, 100.0f) < MetaInfo.RiverChanceFactor)
      {
        TileMetaInfo.ContainsRiver = true;
      }
      else
      {
        TileMetaInfo.ContainsRiver = false;
      }

      // 从纹理中更新并获取高度图。
      UpdateTileRT(TileMetaInfo);

      FTextureRenderTargetResource* RenderTargetResource = HeightRT->GameThread_GetRenderTargetResource();
      FIntRect Rect = FIntRect(0, 0, HeightRT->SizeX, HeightRT->SizeY);
      TArray<FLinearColor> HeightmapColor;

      HeightmapColor.Reserve(Rect.Width() * Rect.Height());
      RenderTargetResource->ReadLinearColorPixels(HeightmapColor, FReadSurfaceDataFlags(RCM_MinMax, CubeFace_MAX), Rect);
      HeightData.Reserve(HeightmapColor.Num());

      for(FLinearColor LinearColor : HeightmapColor)
      {
        HeightData.Add((uint16)(LinearColor.R * 255 * 255 + LinearColor.G * 255));
      }

      // 地形感兴趣区域。
      FRoiTile ThisTileIndex(i, j);
      if(FCarlaRegionOfInterest::IsTileInRegionsSet(ThisTileIndex, MetaInfo.TerrainRoisMap))
      {
        FTerrainROI TileRegion = MetaInfo.TerrainRoisMap[ThisTileIndex];

        //使用 ROI 材质更新 ROI 渲染目标（ROI Render Target）。
        UpdateTileRoiRT(TileMetaInfo, TileRegion.RoiMaterialInstance);

        UTextureRenderTarget2D* RoiHeightRT = TileRegion.RoiHeightmapRenderTarget;
        TArray<uint16> RoiHeightData;
        FTextureRenderTargetResource* RoiRenderTargetResource = RoiHeightRT->GameThread_GetRenderTargetResource();
        FIntRect RoiRect = FIntRect(0, 0, RoiHeightRT->SizeX, RoiHeightRT->SizeY);
        TArray<FLinearColor> RoiHeightmapColor;
        RoiHeightmapColor.Reserve(RoiRect.Width() * RoiRect.Height());
        RoiRenderTargetResource->ReadLinearColorPixels(RoiHeightmapColor, FReadSurfaceDataFlags(RCM_MinMax, CubeFace_MAX), RoiRect);
        RoiHeightData.Reserve(RoiHeightmapColor.Num());

        for(FLinearColor RoiLinearColor : RoiHeightmapColor)
        {
          RoiHeightData.Add((uint16)(RoiLinearColor.R * 255 * 255 + RoiLinearColor.G * 255));
        }


        //int FlateningMargin = 30;   //未被平整化。
        int FlateningMargin = 0;   // 未被平整化。
        int FlateningFalloff = 100; // 从实际值和平整值进行过渡。
        int TileSize = TILESIZE; // 应该通过 HeightData 的数量的平方根（sqrt(HeightData.Num())）来计算。

        bool IsThereTileUp, IsThereTileDown, IsThereTileLeft, IsThereTileRight;

        /* Blending the height data of the ROI with the height data of the tile. */
        if(FTerrainROI::IsTileInRoiBoundary(ThisTileIndex, MetaInfo.TerrainRoisMap, IsThereTileUp, IsThereTileRight, IsThereTileDown, IsThereTileLeft))
        {
          /* Blending the height data of the ROI with the height data of the tile. */
          for(int X = FlateningMargin; X < (TileSize); X++)
          {
            for(int Y = FlateningMargin; Y < (TileSize); Y++)
            {
              float TransitionFactor = 1.0f;

              if(!IsThereTileLeft && X < (FlateningMargin + FlateningFalloff))
              {
                TransitionFactor *= (X - FlateningMargin) / (float) FlateningFalloff;
              }
              if(!IsThereTileUp && Y < (FlateningMargin + FlateningFalloff))
              {
                TransitionFactor *= (Y - FlateningMargin) / (float) FlateningFalloff;
              }
              if(!IsThereTileRight && X > (TileSize - FlateningMargin - FlateningFalloff))
              {
                TransitionFactor *= 1 - ((X - (TileSize - FlateningMargin - FlateningFalloff)) / (float) FlateningFalloff);
              }
              if(!IsThereTileDown && Y > (TileSize - FlateningMargin - FlateningFalloff))
              {
                TransitionFactor *= 1 - ((Y - (TileSize - FlateningMargin - FlateningFalloff)) / (float) FlateningFalloff);
              }
              HeightData[(X * TileSize) + Y] = (RoiHeightData[(X * TileSize) + Y]) * TransitionFactor +  HeightData[(X * TileSize) + Y] * (1-TransitionFactor);
            }
          }
        }
        else
        {
          HeightData = RoiHeightData;
        }
      }

      //如果包含河流则进行平整化。
      //待办事项：将这个 if 判断移到一个函数中。
      //待办事项：检查并修复平整化算法。
      if(TileMetaInfo.ContainsRiver)
      {
        int TileSize = TILESIZE; // Should be calculated by sqrt(HeightData.Num())
        int FlateningMargin = 30;   // 未被平整化。
        int FlateningFalloff = 100; // 从实际值向平整值过渡。

        for(int X = FlateningMargin; X < (TileSize - FlateningMargin); X++)
        {
          for(int Y = FlateningMargin; Y < (TileSize - FlateningMargin); Y++)
          {
            float TransitionFactor = 1.0f;

            if(X < (FlateningMargin + FlateningFalloff))
            {
              TransitionFactor *= (X - FlateningMargin) / (float) FlateningFalloff;
            }
            if(Y < (FlateningMargin + FlateningFalloff))
            {
              TransitionFactor *= (Y - FlateningMargin) / (float) FlateningFalloff;
            }
            if(X > (TileSize - FlateningMargin - FlateningFalloff))
            {
              TransitionFactor *= 1 - ((X - (TileSize - FlateningMargin - FlateningFalloff)) / (float) FlateningFalloff);
            }
            if(Y > (TileSize - FlateningMargin - FlateningFalloff))
            {
              TransitionFactor *= 1 - ((Y - (TileSize - FlateningMargin - FlateningFalloff)) / (float) FlateningFalloff);
            }
            
            // 高度数据数组 HeightData 中索引为（X 乘以瓦片尺寸 TileSize 再加上 Y）的元素值等
            //（该元素原本的值乘以元信息 MetaInfo 中的河流平整化因子 RiverFlateningFactor）再乘以
            //过渡因子 TransitionFactor，加上该元素原本的值乘以（1 减去过渡因子）”
          }
        }
        DuplicateWorld("/CarlaTools/MapGenerator/Rivers/RiverPresets/River01/RiverPreset01.RiverPreset01",
            MetaInfo.DestinationPath + "/Rivers", MapName + "_River");
      }

      // 平滑处理
      TArray<uint16> SmoothedData;
      SmoothHeightmap(HeightData, SmoothedData);
      HeightData = SmoothedData;

      //缝合上边界和左边界。
      if(BoundariesInfo.Num() != 0) // 为了避免在第一个图块上崩溃。
      {
        TArray<uint16> SewedData;
        SewUpperAndLeftTiles(HeightData, SewedData, ThisTileIndex.X, ThisTileIndex.Y);
        HeightData = SewedData;
      }

      // 存储边界信息。
      FTileBoundariesInfo ThisTileBoundariesInfo;
      TArray<uint16> RightHeightData;
      TArray<uint16> BottomHeightData;

      for(int DataIndex = 0; DataIndex < TILESIZE - 1; DataIndex++)
      {
        // 右边
        RightHeightData.Add(HeightData[ Convert2DTo1DCoord(0, DataIndex, TILESIZE) ]);
        // 底部
        BottomHeightData.Add(HeightData[ Convert2DTo1DCoord(DataIndex, TILESIZE - 1, TILESIZE) ]);
      }

      ThisTileBoundariesInfo.RightHeightData = RightHeightData;
      ThisTileBoundariesInfo.BottomHeightData = BottomHeightData;
      BoundariesInfo.Add(ThisTileIndex, ThisTileBoundariesInfo);


      FVector LandscapeScaleVector(100.0f, 100.0f, 100.0f);
      Landscape->CreateLandscapeInfo();
      Landscape->SetActorTransform(FTransform(FQuat::Identity, FVector(), LandscapeScaleVector));

      TMap<FGuid, TArray<uint16>> HeightmapDataPerLayers;
      TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayer;

      HeightmapDataPerLayers.Add(FGuid(), HeightData);
      MaterialLayerDataPerLayer.Add(FGuid(), TArray<FLandscapeImportLayerInfo>());
      Landscape->Import(Landscape->GetLandscapeGuid(), 0, 0, HeightRT->SizeX-1, HeightRT->SizeY-1, Landscape->NumSubsections, Landscape->SubsectionSizeQuads,
          HeightmapDataPerLayers, TEXT("NONE"), MaterialLayerDataPerLayer, ELandscapeImportAlphamapType::Layered);

      Landscape->ReregisterAllComponents();
      Landscape->CreateLandscapeInfo();
      Landscape->SetActorLabel(TEXT("Landscape_"+FString::FromInt(i) + "_" + FString::FromInt(j)));
      //“景观（Landscape）对象设置行为体标签为‘Landscape’”。
      // 应用材质。
      AssignLandscapeMaterial(Landscape);

      Landscape->RecreateCollisionComponents();

      // 创建地面力学高度图。
      FString HeightmapPath = MetaInfo.DestinationPath + "/HeightMaps/";
      UCustomTerrainPhysicsComponent::BuildLandscapeHeightMapDataAasset(Landscape, 1009, FVector(100800,100800,0),
          HeightmapPath, MapName);

      const FString PackageFileName = FPackageName::LongPackageNameToFilename(
          PackageName, 
          FPackageName::GetMapPackageExtension());
      UPackage::SavePackage(TilePackage, World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
          *PackageFileName, GError, nullptr, true, true, SAVE_NoError);

      // TODO PROV
      FText ErrorUnloadingStr;
      bool bUnload = FEditorFileUtils::AttemptUnloadInactiveWorldPackage(BaseWorld->GetPackage(),ErrorUnloadingStr);
      if(!bUnload){
        UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: Error unloading Base map: %s"), 
            *CUR_CLASS_FUNC_LINE, *ErrorUnloadingStr.ToString());
        return false;
      }
    }
  }

  // 清空边界数据数组。
  BoundariesInfo.Empty();

  CookTilesCollisions(MetaInfo);

  return true;
}

bool UMapGeneratorWidget::CookVegetationToTiles(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Cooking vegetation to %s tiles"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);

  //首先检查元信息（MetaInfo）中的植被生成器（FoliageSpawners）中是否有元素。
  if(MetaInfo.FoliageSpawners.Num() == 0)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s Vegetation cooking skipped. No foliage spawners selected."), 
        *MetaInfo.MapName);
        return true;
  }

  // 加载所有新生成的地图。
  TArray<FAssetData> AssetsData;
  const FString TilesPath = MetaInfo.DestinationPath;
  bool success = LoadWorlds(AssetsData, TilesPath);
  if(!success || AssetsData.Num() <= 0)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("No Tiles found in %s. Vegetation cooking Aborted!"), *TilesPath);
    return false;
  }

  // 为每张地图烘焙植被。
  for(FAssetData AssetData : AssetsData)
  {
    UWorld* World = GetWorldFromAssetData(AssetData);

    // 检查它是否不是一个图块。
    if(!World->GetMapName().Contains("_Tile_"))
    {
      UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: %s Skipped as it is not a tile"), 
          *CUR_CLASS_FUNC_LINE, *World->GetMapName());
      continue;
    }

    const FString MapNameToLoad = TilesPath + "/" + World->GetMapName() + "." + World->GetMapName();

    // 将地图加载到编辑器中。这是生成模拟的程序性植被所必需的。
    bool bLoadedSuccess = FEditorFileUtils::LoadMap(*MapNameToLoad, false, true);
    if(!bLoadedSuccess){
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Loading %s"),
          *CUR_CLASS_FUNC_LINE, *MapNameToLoad);
      return false;
    }

    // 感兴趣区域检查
    int TileIndexX, TileIndexY;
    ExtractCoordinatedFromMapName(World->GetMapName(), TileIndexX, TileIndexY);

    FRoiTile ThisTileIndex(TileIndexX, TileIndexY);
    TArray<UProceduralFoliageSpawner*> FoliageSpawnersToCook;
    if(FCarlaRegionOfInterest::IsTileInRegionsSet(ThisTileIndex, MetaInfo.VegetationRoisMap))
    {
      FVegetationROI TileRegion = MetaInfo.VegetationRoisMap[ThisTileIndex];
      FoliageSpawnersToCook = TileRegion.GetFoliageSpawners();
    }
    else
    {
      FoliageSpawnersToCook = MetaInfo.FoliageSpawners;
    }

    //为世界烘焙植被。
    bool bVegetationSuccess = CookVegetationToWorld(World, FoliageSpawnersToCook);
    if(!bVegetationSuccess){
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Cooking Vegetation in %s"),
          *CUR_CLASS_FUNC_LINE, *MapNameToLoad);
      return false;
    }
    SaveWorld(World);
  }

  return true;
}

bool UMapGeneratorWidget::CookVegetationToWorld(
  UWorld* World, 
  const TArray<UProceduralFoliageSpawner*> FoliageSpawners)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Cooking vegetation to %s"), 
      *CUR_CLASS_FUNC_LINE, *World->GetMapName());

  // 对于每个生成器，创建一个程序性植被体积并模拟植被。
  for(auto Spawner : FoliageSpawners)
  {
    ULevel* Level = World->GetCurrentLevel();

    VectorRegister	Rotation{ 0,0,0 };
    VectorRegister	Translation{ 200000.0, 200000.0, 0.0 };
    VectorRegister Scale3D{ 2500,2500,900 };
    EObjectFlags InObjectFlags = RF_Transactional;
    FName InName = NAME_None;

    FTransform Transform{ Rotation,Translation,Scale3D };

    UActorFactory* ActorFactory = GEditor->FindActorFactoryForActorClass(AProceduralFoliageVolume::StaticClass());
    AProceduralFoliageVolume* FoliageVolumeActor = (AProceduralFoliageVolume*) ActorFactory->CreateActor(
        AProceduralFoliageVolume::StaticClass(), Level, Transform, InObjectFlags, InName);

    UProceduralFoliageComponent* FoliageComponent = FoliageVolumeActor->ProceduralComponent;
    FoliageComponent->FoliageSpawner = Spawner;

    TArray<FDesiredFoliageInstance> FoliageInstances;
    bool result = FoliageComponent->GenerateProceduralContent(FoliageInstances);

    if(result && FoliageInstances.Num() > 0)
    {
      FoliageComponent->RemoveProceduralContent(false);

      FFoliagePaintingGeometryFilter OverrideGeometryFilter;
      OverrideGeometryFilter.bAllowStaticMesh = FoliageComponent->bAllowStaticMesh;
      OverrideGeometryFilter.bAllowBSP = FoliageComponent->bAllowBSP;
      OverrideGeometryFilter.bAllowLandscape = FoliageComponent->bAllowLandscape;
      OverrideGeometryFilter.bAllowFoliage = FoliageComponent->bAllowFoliage;
      OverrideGeometryFilter.bAllowTranslucent = FoliageComponent->bAllowTranslucent;

      FEdModeFoliage::AddInstances(World, FoliageInstances, OverrideGeometryFilter, true);
    }
    else
    {
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Procedural content generation failed!"), 
          *CUR_CLASS_FUNC_LINE);
      return false;
    }
  }
  return true;
}

bool UMapGeneratorWidget::CookMiscSpreadedActors(const FMapGeneratorMetaInfo& MetaInfo)
{
  bool bSuccess = true;

  TArray<FRoiTile> ListOfTiles;
  MetaInfo.MiscSpreadedActorsRoisMap.GetKeys(ListOfTiles);

  for(FRoiTile ThisTile : ListOfTiles)
  {
    //检查地图是否有效。
    const FString MapCompletePath = MetaInfo.DestinationPath + "/" + MetaInfo.MapName +
        "_Tile_" + FString::FromInt(ThisTile.X) + "_" + FString::FromInt(ThisTile.Y);

    //在主地图中实例化天气行为体。
    const FString WorldToLoadPath = MapCompletePath + "." + MetaInfo.MapName + 
        "_Tile_" + FString::FromInt(ThisTile.X) + "_" + FString::FromInt(ThisTile.Y);
    //创建一个指向虚幻世界（UWorld）类型的指针 World，
    //通过加载对象函数（LoadObject）以空指针（nullptr）和待加载世界的路径（WorldToLoadPath）作为参数，
    //尝试加载一个虚幻世界对象。);
    bool bLoadedSuccess = FEditorFileUtils::LoadMap(*WorldToLoadPath, false, true);
    if(!bLoadedSuccess){
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Loading %s"),
          *CUR_CLASS_FUNC_LINE, *WorldToLoadPath);
      return false;
    }
    UWorld* World = GEditor->GetEditorWorldContext().World();

    if(!IsValid(World))
    {
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error loading world %s"), 
          *CUR_CLASS_FUNC_LINE, *WorldToLoadPath);
      return false;
    }

    FMiscSpreadedActorsROI ActorROI = MetaInfo.MiscSpreadedActorsRoisMap[ThisTile];

    int NumOfTilesForSpreadedActorsGrid;
    switch(ActorROI.ActorsDensity)
    {
      case ESpreadedActorsDensity::LOW:
        NumOfTilesForSpreadedActorsGrid = 50;
        break;
      case ESpreadedActorsDensity::MEDIUM:
        NumOfTilesForSpreadedActorsGrid = 100;
        break;
      case ESpreadedActorsDensity::HIGH:
        NumOfTilesForSpreadedActorsGrid = 200;
        break;
    }

    float TotalMapTileSize = 100800.0f; // In cm
    float MaxTileDisplacement = 0.5f * TotalMapTileSize / NumOfTilesForSpreadedActorsGrid;
    for(int i = 1; i < NumOfTilesForSpreadedActorsGrid - 1; i++)
    {
      for(int j = 1; j < NumOfTilesForSpreadedActorsGrid - 1; j++)
      {
        bool bIsGridTileEligible = FMath::RandRange(0.0f,100.0f) <= ActorROI.Probability;
        float ActorXCoord = (i * TotalMapTileSize / NumOfTilesForSpreadedActorsGrid) + FMath::RandRange(-MaxTileDisplacement,MaxTileDisplacement);
        float ActorYCoord = (j * TotalMapTileSize / NumOfTilesForSpreadedActorsGrid) + FMath::RandRange(-MaxTileDisplacement,MaxTileDisplacement);
        if(bIsGridTileEligible)
        {
          float ActorZCoord = GetLandscapeSurfaceHeight(World, ActorXCoord, ActorYCoord, false);
          FVector Location(ActorXCoord, ActorYCoord, ActorZCoord);
          // 待办事项：随机添加旋转吗？
          FRotator Rotation(0, 0, 0);
          FActorSpawnParameters SpawnInfo;

          AActor* SpreadedActor =  World->SpawnActor<AActor>(
              ActorROI.ActorClass, 
              Location, 
              Rotation, 
              SpawnInfo);
          SpreadedActor->Tags.Add(TAG_SPREADED);
        }
      }
    }

    //保存地图。
    SaveWorld(World); 
  }

  return bSuccess;
}

UWorld* UMapGeneratorWidget::GetWorldFromAssetData(FAssetData& WorldAssetData)
{
  UWorld* World;
  UObjectRedirector *BaseMapRedirector = 
    Cast<UObjectRedirector>(WorldAssetData.GetAsset());
  if(BaseMapRedirector != nullptr)
    World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
  else
    World = CastChecked<UWorld>(WorldAssetData.GetAsset());

  return World;
}

float UMapGeneratorWidget::GetLandscapeSurfaceHeight(UWorld* World, float x, float y, bool bDrawDebugLines)
{
  if(World)
  {
    ALandscape* Landscape = (ALandscape*) UGameplayStatics::GetActorOfClass(
        World, 
        ALandscape::StaticClass());
    
    FVector Location(x, y, 0);
    TOptional<float> Height = Landscape->GetHeightAtLocation(Location);
    // 待办事项：将函数的返回类型更改为可选浮点型（TOptional<float>）。
    return Height.IsSet() ? Height.GetValue() : GetLandscapeSurfaceHeightFromRayCast(World, x, y, bDrawDebugLines);
  }
  return 0.0f;
}

float UMapGeneratorWidget::GetLandscapeSurfaceHeightFromRayCast(UWorld* World, float x, float y, bool bDrawDebugLines)
{
  if(World)
  {
    FVector RayStartingPoint(x, y, 9999999);
    FVector RayEndPoint(x, y, -9999999);

    // 光线追踪。
    FHitResult HitResult;
    World->LineTraceSingleByObjectType(
        OUT HitResult,
        RayStartingPoint,
        RayEndPoint,
        FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldStatic),
        FCollisionQueryParams());

    //绘制调试线.
    if (bDrawDebugLines)
    {
      FColor LineColor;

      if (HitResult.GetActor()) LineColor = FColor::Red;
      else LineColor = FColor::Green;

      DrawDebugLine(
          World,
          RayStartingPoint,
          RayEndPoint,
          LineColor,
          true,
          5.f,
          0.f,
          10.f);
    }

    // 返回 Z 轴位置。
    if (HitResult.GetActor()) 
      return HitResult.ImpactPoint.Z;

  }
  return 0.0f;
}

void UMapGeneratorWidget::ExtractCoordinatedFromMapName(const FString MapName, int& X, int& Y)
{
  FString Name, Coordinates;
  MapName.Split(TEXT("_Tile_"), &Name, &Coordinates);

  FString XStr, YStr;
  Coordinates.Split(TEXT("_"), &XStr, &YStr);

  X = FCString::Atoi(*XStr);
  Y = FCString::Atoi(*YStr);
}

void UMapGeneratorWidget::SmoothHeightmap(TArray<uint16> HeightData, TArray<uint16>& OutHeightData)
{
  TArray<uint16> SmoothedData(HeightData);

  // 准备高斯核。
  int KernelSize = 5;
  int KernelWeight = 273; 
  float Kernel[] = {1,  4,  7,  4,  1,
                    4, 16, 26, 16,  4,
                    7, 26, 41, 26,  7,
                    4, 16, 26, 16,  4,
                    1,  4,  7,  4,  1};

  TArray<float> SmoothKernel;
  for(int i = 0; i < KernelSize*KernelSize; i++)
  {
    SmoothKernel.Add(Kernel[i] / KernelWeight);
  }

  // 将核应用于高度数据。
  int TileMargin = 2;

  /* Applying a smoothing kernel to the height data */
  for(int X = 0; X < (TILESIZE); X++)
  {
    for(int Y = 0; Y < (TILESIZE); Y++)
    {
      int Value = 0;

      for(int i = -2; i <= 2; i++)
      {
        for(int j = -2; j <=2; j++)
        {
          float KernelValue = SmoothKernel[(i+2)*2 + (j+2)];

          int IndexX = X+i;
          int IndexY = Y+j;

          /* Checking if the index is out of bounds. If it is, it sets the index to the current X or Y. */
          if(IndexX < 0 || IndexX >= TILESIZE)
              IndexX = X;
          if(IndexY < 0 || IndexY >= TILESIZE)
              IndexY = Y;

          int HeightValue = HeightData[ Convert2DTo1DCoord(IndexX, IndexY, TILESIZE) ];

          Value += (int) ( KernelValue * HeightValue );
        }
      }

      SmoothedData[ Convert2DTo1DCoord(X, Y, TILESIZE) ] = Value;
    }
  }

  OutHeightData = SmoothedData;
}

void UMapGeneratorWidget::SewUpperAndLeftTiles(TArray<uint16> HeightData, TArray<uint16>& OutHeightData, int IndexX, int IndexY)
{
  TArray<uint16> SewedData(HeightData);

  FRoiTile ThisTile(IndexX, IndexY);
  FRoiTile UpTile = ThisTile.Up();
  FRoiTile LeftTile = ThisTile.Right(); // Right because of the coord system used

  // Up
  if(BoundariesInfo.Contains(UpTile))
  {
    for(int DataIndex = 0; DataIndex < TILESIZE - 1; DataIndex++)
    {
      TArray<uint16> BottomInfo = BoundariesInfo[UpTile].BottomHeightData;
      SewedData[ Convert2DTo1DCoord(DataIndex, 0, TILESIZE) ] = BottomInfo[DataIndex];
    }
  }

  // Left
  if(BoundariesInfo.Contains(LeftTile))
  {
    for(int DataIndex = 0; DataIndex < TILESIZE - 1; DataIndex++)
    {
      TArray<uint16> RightInfo = BoundariesInfo[LeftTile].RightHeightData;
      SewedData[ Convert2DTo1DCoord(TILESIZE - 1, DataIndex, TILESIZE) ] = RightInfo[DataIndex];
    }
  }

  OutHeightData = SewedData;
}
