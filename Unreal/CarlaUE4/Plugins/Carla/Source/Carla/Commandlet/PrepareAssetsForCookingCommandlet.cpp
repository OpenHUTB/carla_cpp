// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "PrepareAssetsForCookingCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "SSTags.h"

#if WITH_EDITOR
#include "FileHelpers.h"
#endif
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/PlatformFilemanager.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Carla/MapGen/LargeMapManager.h"

// 静态函数，用于验证静态网格是否符合某些条件
static bool ValidateStaticMesh(UStaticMesh *Mesh)
{
  // 获取网格资源的名称
  const FString AssetName = Mesh->GetName();

  // 检查网格名称是否包含“light”或“sign”，忽略大小写
  if (AssetName.Contains(TEXT("light"), ESearchCase::IgnoreCase) ||
      AssetName.Contains(TEXT("sign"), ESearchCase::IgnoreCase))
  {
    // 如果包含，则返回false，表示验证不通过
    return false;
  }

  // 遍历网格的所有静态材质
  for (int i = 0; i < Mesh->StaticMaterials.Num(); i++)
  {
    // 获取当前索引处的材质
    UMaterialInterface *Material = Mesh->GetMaterial(i);
    if (!Material) {
      // 如果材质为空，则使用默认的表面材质
      Material = UMaterial::GetDefaultMaterial(MD_Surface);
    }
    // 获取材质的名称
    const FString MaterialName = Material->GetName();

    // 检查材质名称是否包含“light”或“sign”，忽略大小写
    if (MaterialName.Contains(TEXT("light"), ESearchCase::IgnoreCase) ||
        MaterialName.Contains(TEXT("sign"), ESearchCase::IgnoreCase))
    {
      // 如果包含，则返回false，表示验证不通过
      return false;
    }
  }

  // 如果所有检查都通过，则返回true，表示验证通过
  return true;
}

UPrepareAssetsForCookingCommandlet::UPrepareAssetsForCookingCommandlet()
{
  // 设置必要的标志以运行命令行工具
  IsClient = false;
  IsEditor = true;
  IsServer = false;
  LogToConsole = true;

#if WITH_EDITORONLY_DATA
  // 获取 Carla 默认材质，
  //这些材质将用于需要使用 Carla 材质的地图
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MarkingNodeYellowMaterial(TEXT(
    "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LargeMaps/M_Road_03_Tiled_V3.M_Road_03_Tiled_V3'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MarkingNodeWhiteMaterial(TEXT(
    "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/M_Road_03_LMW.M_Road_03_LMW'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> RoadNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LargeMaps/M_Road_03_Tiled_V2.M_Road_03_Tiled_V2'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TerrainNodeMaterial(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/00_MastersOpt/Large_Maps/materials/MI_LargeLandscape_Grass.MI_LargeLandscape_Grass'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> CurbNodeMaterial(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/LargeMap_materials/largeM_curb/MI_largeM_curb01.MI_largeM_curb01'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> GutterNodeMaterial(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/LargeMap_materials/largeM_gutter/MI_largeM_gutter01.MI_largeM_gutter01'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> SidewalkNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/LargeMap_materials/largeM_sidewalk/tile01/MI_largeM_tile02.MI_largeM_tile02'"));

  GutterNodeMaterialInstance = (UMaterialInstance *) GutterNodeMaterial.Object;
  CurbNodeMaterialInstance = (UMaterialInstance *) CurbNodeMaterial.Object;
  TerrainNodeMaterialInstance = (UMaterialInstance *) TerrainNodeMaterial.Object;
  MarkingNodeYellow = (UMaterialInstance *)MarkingNodeYellowMaterial.Object;
  MarkingNodeWhite = (UMaterialInstance *)MarkingNodeWhiteMaterial.Object;
  RoadNodeMaterial = (UMaterialInstance *) RoadNode.Object;
  SidewalkNodeMaterialInstance = (UMaterialInstance *) SidewalkNode.Object;
#endif
}
#if WITH_EDITORONLY_DATA

FPackageParams UPrepareAssetsForCookingCommandlet::ParseParams(const FString &InParams) const
{
  TArray<FString> Tokens;
  TArray<FString> Params;
  TMap<FString, FString> ParamVals;

  ParseCommandLine(*InParams, Tokens, Params);

  FPackageParams PackageParams;

  // 解析并存储包名称
  FParse::Value(*InParams, TEXT("PackageName="), PackageParams.Name);

  // 解析并存储仅准备地图的标志
  FParse::Bool(*InParams, TEXT("OnlyPrepareMaps="), PackageParams.bOnlyPrepareMaps);
  return PackageParams;
}

void UPrepareAssetsForCookingCommandlet::LoadWorld(FAssetData &AssetData)
{
  // Carla 中的基础地图路径
  const FString BaseMap = TEXT("/Game/Carla/Maps/BaseMap");

  // 使用对象库加载地图文件夹
  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  MapObjectLibrary->AddToRoot();
  MapObjectLibrary->LoadAssetDataFromPath(*BaseMap);
  MapObjectLibrary->LoadAssetsFromAssetData();
  MapObjectLibrary->GetAssetDataList(AssetDatas);

  if (AssetDatas.Num() > 0)
  {
    // 提取在文件夹路径中找到的第一个资产（即基础地图）
    AssetData = AssetDatas.Pop();
  }
}

void UPrepareAssetsForCookingCommandlet::LoadWorldTile(FAssetData &AssetData)
{
  //Carla 中的基础瓦片路径
  const FString BaseTile = TEXT("/Game/Carla/Maps/TestMaps");

  // 使用对象库加载地图文件夹。
  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  MapObjectLibrary->AddToRoot();
  MapObjectLibrary->LoadAssetDataFromPath(*BaseTile);
  MapObjectLibrary->LoadAssetsFromAssetData();
  MapObjectLibrary->GetAssetDataList(AssetDatas);

  if (AssetDatas.Num() > 0)
  {
    // 提取在文件夹路径中找到的第一个资产（即基础瓦片）
    AssetData = AssetDatas.Pop();
  }
}

void UPrepareAssetsForCookingCommandlet::LoadLargeMapWorld(FAssetData &AssetData)
{
  // Carla 中的基础地图路径
  const FString BaseMap = TEXT("/Game/Carla/Maps/BaseLargeMap");

  //使用对象库加载地图文件夹。
  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  MapObjectLibrary->AddToRoot();
  MapObjectLibrary->LoadAssetDataFromPath(*BaseMap);
  MapObjectLibrary->LoadAssetsFromAssetData();
  MapObjectLibrary->GetAssetDataList(AssetDatas);

  if (AssetDatas.Num() > 0)
  {
    // 提取在文件夹路径中找到的第一个资产（即基础地图）。
    AssetData = AssetDatas.Pop();
  }
}

TArray<AStaticMeshActor *> UPrepareAssetsForCookingCommandlet::SpawnMeshesToWorld(
    const TArray<FString> &AssetsPaths,
    bool bUseCarlaMaterials,
    int i,
    int j)
{
  TArray<AStaticMeshActor *> SpawnedMeshes;

  // 为所有要生成的资产创建默认变换。
  const FTransform ZeroTransform = FTransform();

  // 使用对象库加载由 “AssetsPaths” 中指定的资产，
  //用于构建地图世界。
  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
  AssetsObjectLibrary->AddToRoot();
  AssetsObjectLibrary->LoadAssetDataFromPaths(AssetsPaths);
  AssetsObjectLibrary->LoadAssetsFromAssetData();
  MapContents.Empty();
  AssetsObjectLibrary->GetAssetDataList(MapContents);

  UStaticMesh *MeshAsset;
  AStaticMeshActor *MeshActor;

  //要烹饪的当前瓦片的名称。
  FString TileName;
  if (i != -1)
  {
    TileName = FString::Printf(TEXT("_Tile_%d_%d"), i, j);
  }

  // try to get the name of the map that precedes all assets name
  FString AssetName;
  for (auto MapAsset : MapContents)
  {
    // 生成静态网格体
    MeshAsset = Cast<UStaticMesh>(MapAsset.GetAsset());
    if (MeshAsset && ValidateStaticMesh(MeshAsset))
    {
      // 获取资产名称。
      MapAsset.AssetName.ToString(AssetName);

      //检查以忽略来自其他瓦片的网格体。
      if (i == -1 || (i != -1 && (AssetName.EndsWith(TileName) || AssetName.Contains(TileName + "_"))))
      {
        MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), ZeroTransform);
        UStaticMeshComponent *MeshComponent = MeshActor->GetStaticMeshComponent();
        MeshComponent->SetStaticMesh(CastChecked<UStaticMesh>(MeshAsset));
        MeshActor->SetActorLabel(AssetName, true);

        // 在资产中将复杂碰撞设置为简单碰撞。
        UBodySetup *BodySetup = MeshAsset->BodySetup;
        if (BodySetup)
        {
          BodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
          MeshAsset->MarkPackageDirty();
        }

        SpawnedMeshes.Add(MeshActor);

        if (bUseCarlaMaterials)
        {
          // 根据 RoadRunner 的语义分割标签设置 
          //Carla 材质。
          if (AssetName.Contains(SSTags::R_MARKING1) || AssetName.Contains(SSTags::R_MARKING2))
          {
            for (int32 i = 0; i < MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials.Num(); ++i)
            {
              if (MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials[i].ImportedMaterialSlotName.ToString().Contains("Yellow"))
              {
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, MarkingNodeYellow);
              }
              else
              {
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, MarkingNodeWhite);
              }
            }
          }
          else if (AssetName.Contains(SSTags::R_ROAD1) || AssetName.Contains(SSTags::R_ROAD2))
          {
            MeshActor->GetStaticMeshComponent()->SetMaterial(0, RoadNodeMaterial);
          }
          else if (AssetName.Contains(SSTags::R_TERRAIN))
          {
            MeshActor->GetStaticMeshComponent()->SetMaterial(0, TerrainNodeMaterialInstance);
            MeshActor->GetStaticMeshComponent()->bReceivesDecals = false;
          }
          else if (AssetName.Contains(SSTags::R_SIDEWALK1) || AssetName.Contains(SSTags::R_SIDEWALK2))
          {
            MeshActor->GetStaticMeshComponent()->SetMaterial(0, SidewalkNodeMaterialInstance);
            MeshActor->GetStaticMeshComponent()->bReceivesDecals = false;
          }
          else if (AssetName.Contains(SSTags::R_CURB1) || AssetName.Contains(SSTags::R_CURB2)) {

            MeshActor->GetStaticMeshComponent()->SetMaterial(0, CurbNodeMaterialInstance);
            MeshActor->GetStaticMeshComponent()->bReceivesDecals = false;
          }
          else if (AssetName.Contains(SSTags::R_GUTTER1) || AssetName.Contains(SSTags::R_GUTTER2)) {

            MeshActor->GetStaticMeshComponent()->SetMaterial(0, GutterNodeMaterialInstance);
            MeshActor->GetStaticMeshComponent()->bReceivesDecals = false;
          }
        }
      }
    }
  }

  // 清除库中已加载的资产。
  AssetsObjectLibrary->ClearLoaded();

  // 将包标记为已修改。
  World->MarkPackageDirty();

  return SpawnedMeshes;
}

bool UPrepareAssetsForCookingCommandlet::IsMapInTiles(const TArray<FString> &AssetsPaths)
{
  //使用对象库加载 “AssetsPaths” 中指定的资产，
  //以构建地图世界
  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
  AssetsObjectLibrary->AddToRoot();
  AssetsObjectLibrary->LoadAssetDataFromPaths(AssetsPaths);
  AssetsObjectLibrary->LoadAssetsFromAssetData();
  MapContents.Empty();
  AssetsObjectLibrary->GetAssetDataList(MapContents);

  UStaticMesh *MeshAsset;

  FString AssetName;
  bool Found = false;
  for (auto MapAsset : MapContents)
  {
    // 生成静态网格。
    MeshAsset = Cast<UStaticMesh>(MapAsset.GetAsset());
    if (MeshAsset && ValidateStaticMesh(MeshAsset))
    {
      // 获取资产名称。
      MapAsset.AssetName.ToString(AssetName);

      // 检查该资产是否是一个瓦片。
      if (AssetName.Contains("_Tile_"))
      {
        Found = true;
        break;
      }
    }
  }

  // 清除库中已加载的资产。
  AssetsObjectLibrary->ClearLoaded();

  return Found;
}

void UPrepareAssetsForCookingCommandlet::DestroySpawnedActorsInWorld(
    TArray<AStaticMeshActor *> &SpawnedActors)
{
  // 销毁所有已生成的演员（角色）。
  for (auto Actor : SpawnedActors)
  {
    Actor->Destroy();
  }

  // 将包标记为已修改（脏）状态。
  World->MarkPackageDirty();
}

bool UPrepareAssetsForCookingCommandlet::SaveWorld(
    FAssetData &AssetData,
    const FString &PackageName,
    const FString &DestPath,
    const FString &WorldName,
    bool bGenerateSpawnPoints)
{
  // 创建包以进行保存。
  UPackage *Package = AssetData.GetPackage();
  Package->SetFolderName(*WorldName);
  Package->FullyLoad();
  Package->MarkPackageDirty();
  FAssetRegistryModule::AssetCreated(World);

  // 重命名地图。
  World->Rename(*WorldName, World->GetOuter());
  const FString PackagePath = DestPath + "/" + WorldName;
  FAssetRegistryModule::AssetRenamed(World, *PackagePath);
  World->MarkPackageDirty();
  World->GetOuter()->MarkPackageDirty();

  // 检查 OpenDrive 文件是否存在。
  const FString PathXODR = FPaths::ProjectContentDir() + PackageName + TEXT("/Maps/") +
      WorldName + TEXT("/OpenDrive/") + WorldName + TEXT(".xodr");

  bool bPackageSaved = false;
  if (FPaths::FileExists(PathXODR) && bGenerateSpawnPoints)
  {
    // 在保存地图之前，我们需要生成 OpenDrive 资产。
    AOpenDriveActor *OpenWorldActor = CastChecked<AOpenDriveActor>(
        World->SpawnActor(AOpenDriveActor::StaticClass(),
        new FVector(),
        NULL));

    OpenWorldActor->BuildRoutes(WorldName);
    OpenWorldActor->AddSpawners();

    bPackageSaved = SavePackage(PackagePath, Package);

    // 一旦保存了地图，我们就需要销毁 OpenDrive 资产。
    OpenWorldActor->RemoveRoutes();
    OpenWorldActor->RemoveSpawners();
    OpenWorldActor->Destroy();
  }
  else
  {
    bPackageSaved = SavePackage(PackagePath, Package);
  }

  return bPackageSaved;
}

FString UPrepareAssetsForCookingCommandlet::GetFirstPackagePath(const FString &PackageName) const
{
  // 获取所有包的名称。
  TArray<FString> PackageList;
  IFileManager::Get().FindFilesRecursive(PackageList, *(FPaths::ProjectContentDir()),
      *(PackageName + TEXT(".Package.json")), true, false, false);

  if (PackageList.Num() == 0)
  {
    UE_LOG(LogTemp, Error, TEXT("Package json file not found."));
    return {};
  }

  return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*PackageList[0]);
}

FAssetsPaths UPrepareAssetsForCookingCommandlet::GetAssetsPathFromPackage(const FString &PackageName) const
{
  const FString PackageJsonFilePath = GetFirstPackagePath(PackageName);

  FAssetsPaths AssetsPaths;

  // 得到所有地图路径
  FString MapsFileJsonContent;
  if (FFileHelper::LoadFileToString(MapsFileJsonContent, *PackageJsonFilePath))
  {
    TSharedPtr<FJsonObject> JsonParsed;
    TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(MapsFileJsonContent);
    if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
    {
      // 添加地图路径。
      auto MapsJsonArray = JsonParsed->GetArrayField(TEXT("maps"));

      for (auto &MapJsonValue : MapsJsonArray)
      {
        TSharedPtr<FJsonObject> MapJsonObject = MapJsonValue->AsObject();

        FMapData MapData;
        MapData.Name = MapJsonObject->GetStringField(TEXT("name"));
        MapData.Path = MapJsonObject->GetStringField(TEXT("path"));
        MapData.bUseCarlaMapMaterials = MapJsonObject->GetBoolField(TEXT("use_carla_materials"));

        AssetsPaths.MapsPaths.Add(std::move(MapData));
      }

      //  添加地图路径
      auto PropJsonArray = JsonParsed->GetArrayField(TEXT("props"));

      for (auto &PropJsonValue : PropJsonArray)
      {
        TSharedPtr<FJsonObject> PropJsonObject = PropJsonValue->AsObject();

        const FString PropAssetPath = PropJsonObject->GetStringField(TEXT("path"));

        AssetsPaths.PropsPaths.Add(std::move(PropAssetPath));
      }
    }
  }
  return AssetsPaths;
}

bool SaveStringTextToFile(
    FString SaveDirectory,
    FString FileName,
    FString SaveText,
    bool bAllowOverWriting)
{
  IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

  // 如果目标目录在调用之前已存在或者在调用期间已被创建，
  //那么“CreateDirectoryTree”（创建目录树）
  //将返回 true。
  if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
  {
    // 获取绝对文件路径。
    const FString AbsoluteFilePath = SaveDirectory + "/" + FileName;

    
    if (bAllowOverWriting || !PlatformFile.FileExists(*AbsoluteFilePath))
    {
      FFileHelper::SaveStringToFile(SaveText, *AbsoluteFilePath);
    }
  }
  return true;
}

bool UPrepareAssetsForCookingCommandlet::SavePackage(const FString &PackagePath, UPackage *Package) const
{
  const FString PackageFileName = FPackageName::LongPackageNameToFilename(
      PackagePath,
      FPackageName::GetMapPackageExtension());

  if (FPaths::FileExists(*PackageFileName))
  {
    // 如果包已存在，则不会保存该包。
    return false;
  }

  return UPackage::SavePackage(
      Package,
      World,
      EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName,
      GError,
      nullptr,
      true,
      true,
      SAVE_NoError);
}

// UPrepareAssetsForCookingCommandlet类的方法，用于生成地图路径文件
void UPrepareAssetsForCookingCommandlet::GenerateMapPathsFile(
    const FAssetsPaths &AssetsPaths,
    const FString &PropsMapPath)
{
  // 存储地图路径数据，用于Windows和Linux
  FString MapPathData;
  FString MapPathDataLinux;
  // 获取文件管理器
  IFileManager &FileManager = IFileManager::Get();
  // 遍历所有地图路径
  for (const auto &Map : AssetsPaths.MapsPaths)
  {
    // 追加地图路径和名称，用于Windows和Linux
    MapPathData.Append(Map.Path + TEXT("/") + Map.Name + TEXT("\n"));
    MapPathDataLinux.Append(Map.Path + TEXT("/") + Map.Name + TEXT("+"));
    // 获取地图路径下的资产数据
    TArray<FAssetData> AssetsData;
    UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), true, true);
    ObjectLibrary->LoadAssetDataFromPath(Map.Path);
    ObjectLibrary->GetAssetDataList(AssetsData);
    int NumTiles = 0;
    // 遍历资产数据，查找与地图名称匹配的tiles
    for (FAssetData &AssetData : AssetsData)
    {
      FString AssetName = AssetData.AssetName.ToString();
      if (AssetName.Contains(Map.Name + "_Tile_"))
      {
        // 追加tiles的路径和名称，用于Windows和Linux
        MapPathData.Append(Map.Path + TEXT("/") + AssetName + TEXT("\n"));
        MapPathDataLinux.Append(Map.Path + TEXT("/") + AssetName + TEXT("+"));
        NumTiles++;
      }
    }
    // 记录找到的tiles数量
    UE_LOG(LogTemp, Warning, TEXT("Found %d tiles"), NumTiles);
  }

  // 如果提供了PropsMapPath，则追加到路径数据中
  if (!PropsMapPath.IsEmpty())
  {
    MapPathData.Append(PropsMapPath + TEXT("/PropsMap"));
    MapPathDataLinux.Append(PropsMapPath + TEXT("/PropsMap"));
  }
  else
  {
     // 否则，从Linux路径数据中移除最后一个"+"
    MapPathDataLinux.RemoveFromEnd(TEXT("+"));
  }

  // 获取保存目录和文件名
  const FString SaveDirectory = FPaths::ProjectContentDir();
  const FString FileName = FString("MapPaths.txt");
  // 将路径数据保存到文件中
  const FString FileNameLinux = FString("MapPathsLinux.txt");
  SaveStringTextToFile(SaveDirectory, FileName, MapPathData, true);
  SaveStringTextToFile(SaveDirectory, FileNameLinux, MapPathDataLinux, true);
}

// UPrepareAssetsForCookingCommandlet类的方法，用于生成包路径文件
void UPrepareAssetsForCookingCommandlet::GeneratePackagePathFile(const FString &PackageName)
{
  // 获取保存目录和文件名
  FString SaveDirectory = FPaths::ProjectContentDir();
  FString FileName = FString("PackagePath.txt");
  // 获取包的路径
  FString PackageJsonFilePath = GetFirstPackagePath(PackageName);
  // 将包路径保存到文件中
  SaveStringTextToFile(SaveDirectory, FileName, PackageJsonFilePath, true);
}

void UPrepareAssetsForCookingCommandlet::PrepareMapsForCooking(
    const FString &PackageName,
    const TArray<FMapData> &MapsPaths)
{
  // 设置基础路径
  FString BasePath = TEXT("/Game/") + PackageName + TEXT("/Static/");

  // 遍历所有地图路径
  for (const auto &Map : MapsPaths)
  {
    // 设置地图路径
    const FString MapPath = TEXT("/") + Map.Name;

    // 设置默认路径、道路路径、道路线路径、地形路径和人行道路径
    const FString DefaultPath   = TEXT("/Game/") + PackageName + TEXT("/Maps/") + Map.Name;
    const FString RoadsPath     = BasePath + SSTags::ROAD      + MapPath;
    const FString RoadLinesPath = BasePath + SSTags::ROADLINE  + MapPath;
    const FString TerrainPath   = BasePath + SSTags::TERRAIN   + MapPath;
    const FString SidewalkPath  = BasePath + SSTags::SIDEWALK  + MapPath;

    // 生成位于语义分割文件夹中的资产。
    TArray<FString> DataPath = {DefaultPath, RoadsPath, RoadLinesPath, TerrainPath, SidewalkPath};

    // 检查我们是有一个单一地图还是有一个分块地图。
    if (!IsMapInTiles(DataPath))
    {
      UE_LOG(LogTemp, Log, TEXT("Cooking map"));
      // 加载世界。
      FAssetData AssetData;
      LoadWorld(AssetData);
      UObjectRedirector *BaseMapRedirector = Cast<UObjectRedirector>(AssetData.GetAsset());
      if (BaseMapRedirector != nullptr) {
        World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
      }
      else {
        World = CastChecked<UWorld>(AssetData.GetAsset());
      }
      // 尝试烘培（处理）整个地图（非分块地图）。
      TArray<AStaticMeshActor *> SpawnedActors = SpawnMeshesToWorld(DataPath, Map.bUseCarlaMapMaterials, -1, -1);
      // 在指定路径中保存世界。
      SaveWorld(AssetData, PackageName, Map.Path, Map.Name);
      // 从世界中移除已生成的参与者，以使其与基础地图保持一致。
      DestroySpawnedActorsInWorld(SpawnedActors);
    }
    else
    {
      // 定义一个数组，用于存储地图路径和对应的ID
      TArray<TPair<FString, FIntVector>> MapPathsIds;

      // 定义一个FVector变量，用于存储第0个瓦片的位置
      FVector PositionTile0 = FVector();
      // 定义瓦片的大小（以米为单位）
      float TileSize = 200000.f;
      // 定义一个字符串变量，用于存储从文件中读取的文本
      FString TxtFile;
      // 定义瓦片信息文件的路径
      FString TilesInfoPath = FPaths::ProjectContentDir() + PackageName + TEXT("/Maps/") + Map.Name + "/TilesInfo.txt";
      // 记录日志，表示正在加载瓦片信息文件
      UE_LOG(LogTemp, Warning, TEXT("Loading %s ..."), *TilesInfoPath);
      // 尝试从文件中加载文本内容到TxtFile变量中
      if (FFileHelper::LoadFileToString(TxtFile, *(TilesInfoPath)) == true) {
        
        // 将文件内容按逗号分隔，并存储到Out数组中
        TArray<FString> Out;
        TxtFile.ParseIntoArray(Out, TEXT(","), true);
        // 检查数组中是否至少有3个元素（X, Y, TileSize）
        if (Out.Num() >= 3)
        {
          // 定义从米到厘米的转换系数
          const float METERSTOCM = 100.f;
          // 从文件中读取第0个瓦片的X和Y坐标，并转换为厘米
          PositionTile0.X = METERSTOCM * FCString::Atof(*Out[0]);
          PositionTile0.Y = METERSTOCM * FCString::Atof(*Out[1]);
          // 从文件中读取瓦片的大小，并转换为厘米
          TileSize = METERSTOCM * FCString::Atof(*Out[2]);
        }
        else
        {
          // 如果数组元素不足3个，则记录日志表示瓦片信息文件格式无效
          UE_LOG(LogTemp, Warning, TEXT("TilesInfo.txt format is invalid file"));
        }
      }
      else {
        // 如果无法加载文件，则记录日志表示找不到瓦片信息文件
        UE_LOG(LogTemp, Warning, TEXT("Could not find TilesInfo.txt file"));
      }

      UE_LOG(LogTemp, Log, TEXT("Cooking tiles:"));
      // 加载世界。
      FAssetData AssetData;
      LoadWorldTile(AssetData);
      UObjectRedirector *BaseMapRedirector = Cast<UObjectRedirector>(AssetData.GetAsset());
      if (BaseMapRedirector != nullptr) {
        World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
      }
      else {
        World = CastChecked<UWorld>(AssetData.GetAsset());
      }
      // 尝试创建地图的每一种可能的分块。
      int  i, j;
      bool Res;
      j = 0;
      do
      {
        i = 0;
        do
        {
          // Spawn
          TArray<AStaticMeshActor *> SpawnedActors = SpawnMeshesToWorld(DataPath, Map.bUseCarlaMapMaterials, i, j);
          Res = SpawnedActors.Num() > 0;
          if (Res)
          {
            UE_LOG(LogTemp, Log, TEXT(" Tile %d,%d found"), i, j);
            FString TileName;
            TileName = FString::Printf(TEXT("%s_Tile_%d_%d"), *Map.Name, i, j);
            // 在指定路径中保存世界。
            // UE_LOG(LogTemp, Log, TEXT("Saving as %s to %s"), *TileName, *Map.Path);
            SaveWorld(AssetData, PackageName, Map.Path, TileName);
            MapPathsIds.Add(
                TPair<FString, FIntVector>(
                  Map.Path + "/" + TileName, FIntVector(i, j, 0)));
            // 从世界中移除生成的参与者，以使其与基础地图保持一致。
            DestroySpawnedActorsInWorld(SpawnedActors);
            ++i;
          }
        }
        while (Res);
        ++j;
      }
      while (i > 0);

      #if WITH_EDITOR
        UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
      #endif
      // 加载分块地图的基础地图。
      LoadLargeMapWorld(AssetData);
      BaseMapRedirector = Cast<UObjectRedirector>(AssetData.GetAsset());
      if (BaseMapRedirector != nullptr) {
        World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
      }
      else {
        World = CastChecked<UWorld>(AssetData.GetAsset());
      }

      // 生成大型地图管理器。
      ALargeMapManager* LargeMapManager = World->SpawnActor<ALargeMapManager>(
          ALargeMapManager::StaticClass(), FTransform());
      LargeMapManager->LargeMapTilePath = Map.Path;
      LargeMapManager->LargeMapName = Map.Name;
      LargeMapManager->SetTile0Offset(PositionTile0);
      LargeMapManager->SetTileSize(TileSize);
      LargeMapManager->GenerateMap(MapPathsIds);

      SaveWorld(AssetData, PackageName, Map.Path, Map.Name, false);

      UE_LOG(LogTemp, Log, TEXT("End cooking tiles"));
    }
  }
}

void UPrepareAssetsForCookingCommandlet::PreparePropsForCooking(
    FString &PackageName,
    const TArray<FString> &PropsPaths,
    FString &MapDestPath)
{
  // 生成大型地图管理器。
  FAssetData AssetData;
  // 加载基础地图。
  LoadWorld(AssetData);
  UObjectRedirector *BaseMapRedirector = Cast<UObjectRedirector>(AssetData.GetAsset());
  if (BaseMapRedirector != nullptr) {
    World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
  }
  else {
    World = CastChecked<UWorld>(AssetData.GetAsset());
  }

  //从道具的原始路径中移除网格名称，
  //以便我们可以加载文件夹内的道具。
  TArray<FString> PropPathDirs = PropsPaths;

  for (auto &PropPath : PropPathDirs)
  {
    PropPath.Split(TEXT("/"), &PropPath, nullptr,
        ESearchCase::Type::IgnoreCase, ESearchDir::Type::FromEnd);
  }

  // 在单个基础地图中添加道具。
  TArray<AStaticMeshActor *> SpawnedActors = SpawnMeshesToWorld(PropPathDirs, false);

  const FString MapName("PropsMap");
  SaveWorld(AssetData, PackageName, MapDestPath, MapName);

  DestroySpawnedActorsInWorld(SpawnedActors);
  MapObjectLibrary->ClearLoaded();
}

int32 UPrepareAssetsForCookingCommandlet::Main(const FString &Params)
{
  FPackageParams PackageParams = ParseParams(Params);

  // 获取道具和地图的路径。
  FAssetsPaths AssetsPaths = GetAssetsPathFromPackage(PackageParams.Name);

  if (PackageParams.bOnlyPrepareMaps)
  {
    PrepareMapsForCooking(PackageParams.Name, AssetsPaths.MapsPaths);
  }
  else
  {
    FString PropsMapPath("");

    if (AssetsPaths.PropsPaths.Num() > 0)
    {
      PropsMapPath = TEXT("/Game/") + PackageParams.Name + TEXT("/Maps/PropsMap");
      PreparePropsForCooking(PackageParams.Name, AssetsPaths.PropsPaths, PropsMapPath);
    }

    // 保存地图路径文件以供进一步使用
    GenerateMapPathsFile(AssetsPaths, PropsMapPath);

    // 保存包路径以供进一步使用
    GeneratePackagePathFile(PackageParams.Name);
  }

#if WITH_EDITOR
  UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
#endif

  return 0;
}
#endif
