// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "OpenDriveToMap.h"
#include "DesktopPlatform/Public/IDesktopPlatform.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/FileHelper.h"
#include "Engine/LevelBounds.h"
#include "Engine/SceneCapture2D.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "KismetProceduralMeshLibrary.h"
#include "StaticMeshAttributes.h"

#include "Traffic/TrafficLightManager.h"
#include "Online/CustomFileDownloader.h"
#include "Util/ProceduralCustomMesh.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/BlueprintLibary/MapGenFunctionLibrary.h"
#include "OpenDrive/OpenDriveGenerator.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/Map.h>
#include <carla/geom/Simplification.h>
#include <carla/road/Deformation.h>
#include <carla/rpc/String.h>
#include <OSM2ODR.h>
#include <compiler/enable-ue4-macros.h>

#include "Engine/Classes/Interfaces/Interface_CollisionDataProvider.h"
#include "Engine/TriggerBox.h"
#include "Engine/AssetManager.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "PhysicsCore/Public/BodySetupEnums.h"
#include "PhysicsEngine/BodySetup.h"
#include "RawMesh.h"
#include "AssetRegistryModule.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "MeshDescription.h"
#include "EditorLevelLibrary.h"
#include "ProceduralMeshConversion.h"
#include "EditorLevelLibrary.h"

#include "ContentBrowserModule.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Math/Vector.h"
#include "GameFramework/Actor.h"

#include "DrawDebugHelpers.h"

#if WITH_EDITOR
UOpenDriveToMap::UOpenDriveToMap()
{
  AddToRoot();
  bRoadsFinished = false;
  bHasStarted = false;
  bMapLoaded = false;
}

UOpenDriveToMap::~UOpenDriveToMap()
{

}

// 车道类型转换为对应的车道字符串
FString LaneTypeToFString(carla::road::Lane::LaneType LaneType)
{
  switch (LaneType)
  {
  case carla::road::Lane::LaneType::Driving:
    return FString("Driving");
    break;
  case carla::road::Lane::LaneType::Stop:
    return FString("Stop");
    break;
  case carla::road::Lane::LaneType::Shoulder:
    return FString("Shoulder");
    break;
  case carla::road::Lane::LaneType::Biking:
    return FString("Biking");
    break;
  case carla::road::Lane::LaneType::Sidewalk:
    return FString("Sidewalk");
    break;
  case carla::road::Lane::LaneType::Border:
    return FString("Border");
    break;
  case carla::road::Lane::LaneType::Restricted:
    return FString("Restricted");
    break;
  case carla::road::Lane::LaneType::Parking:
    return FString("Parking");
    break;
  case carla::road::Lane::LaneType::Bidirectional:
    return FString("Bidirectional");
    break;
  case carla::road::Lane::LaneType::Median:
    return FString("Median");
    break;
  case carla::road::Lane::LaneType::Special1:
    return FString("Special1");
    break;
  case carla::road::Lane::LaneType::Special2:
    return FString("Special2");
    break;
  case carla::road::Lane::LaneType::Special3:
    return FString("Special3");
    break;
  case carla::road::Lane::LaneType::RoadWorks:
    return FString("RoadWorks");
    break;
  case carla::road::Lane::LaneType::Tram:
    return FString("Tram");
    break;
  case carla::road::Lane::LaneType::Rail:
    return FString("Rail");
    break;
  case carla::road::Lane::LaneType::Entry:
    return FString("Entry");
    break;
  case carla::road::Lane::LaneType::Exit:
    return FString("Exit");
    break;
  case carla::road::Lane::LaneType::OffRamp:
    return FString("OffRamp");
    break;
  case carla::road::Lane::LaneType::OnRamp:
    return FString("OnRamp");
    break;
  case carla::road::Lane::LaneType::Any:
    return FString("Any");
    break;
  }

  return FString("Empty");
}

// 将OpenStreetMap地图格式(.osm)转换为OpenDrive格式(.xodr)
void UOpenDriveToMap::ConvertOSMInOpenDrive()
{
  // OpenDrive文件的绝对路径：Unreal/CarlaUE4/Content/CustomMaps/{MapName}/OpenDrive/{MapName}.osm
  FilePath = FPaths::ProjectContentDir() + "CustomMaps/" + MapName + "/OpenDrive/" + MapName + ".osm";
  // 根据地图原点坐标(x,y)将OpenDrive坐标转为Carla坐标
  FileDownloader->ConvertOSMInOpenDrive( FilePath , OriginGeoCoordinates.X, OriginGeoCoordinates.Y);
  FilePath.RemoveFromEnd(".osm", ESearchCase::Type::IgnoreCase);
  FilePath += ".xodr";

  DownloadFinished(); // 下载OpenStreetMap地图完成
  UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);  // 保存已修改的包
  LoadMap();  // 加载地图
}

// 创建地图
void UOpenDriveToMap::CreateMap()
{
  // 如果地图名为空，则报错。
  if( MapName.IsEmpty() )
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Map Name Is Empty") );
    return;
  }

  // 如果OpenStreetMap地图的链接为空，则进行地图的下载
  if( !Url.IsEmpty() ) {
    if ( !IsValid(FileDownloader) )
    {
      FileDownloader = NewObject<UCustomFileDownloader>();
    }

    FileDownloader->ResultFileName = MapName;
    FileDownloader->Url = Url;

    FileDownloader->DownloadDelegate.BindUObject( this, &UOpenDriveToMap::ConvertOSMInOpenDrive );
    FileDownloader->StartDownload();  // 开始下载
  }
  else if(LocalFilePath.EndsWith(".xodr"))
  {
    ImportXODR();  // 如果本地已有OpenDrive文件则导入。
  }
  else if(LocalFilePath.EndsWith(".osm"))
  {
    ImportOSM();  // 如果本地已有OpenStreetMap地图文件，则导入
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("URL and Local FilePath are Empty. URL: %s  Local FilePath: %s"), *Url, *LocalFilePath );
  }

}

// 创建地图的整块地面
void UOpenDriveToMap::CreateTerrain( const int MeshGridSize, const float MeshGridSectionSize)
{
  TArray<AActor*> FoundActors;
  UGameplayStatics::GetAllActorsOfClass(UEditorLevelLibrary::GetEditorWorld(), AStaticMeshActor::StaticClass(), FoundActors);
  FVector BoxExtent = FVector(TileSize, TileSize,0);  // 初始化地图框大小：瓦片大小 x 瓦片大小
  FVector MinBox = FVector(MinPosition.X, MaxPosition.Y,0);

  // 横向瓦片数 = 地图框横向长度 / 网格大小
  int NumI = BoxExtent.X  / MeshGridSize;
  // 纵向瓦片数 = 地图纵向长度 / 网格大小
  int NumJ = BoxExtent.Y  / MeshGridSize;

  for( int i = 0; i <= NumI; i++ )
  {
    for( int j = 0; j <= NumJ; j++ )
    {
      // 偏移每个程序化网格以容纳所有的地图瓦片
      FVector2D Offset( MinBox.X + i * MeshGridSize, MinBox.Y + j * MeshGridSize);
      CreateTerrainMesh(i * NumJ + j, Offset, MeshGridSize, MeshGridSectionSize );  // 创建地面的一个小网格
    }
  }
}

// 创建地面网格
void UOpenDriveToMap::CreateTerrainMesh(const int MeshIndex, const FVector2D Offset, const int GridSize, const float GridSectionSize)
{
  // const float GridSectionSize = 100.0f; // In cm
  const float HeightScale = 3.0f;

  UWorld* World = UEditorLevelLibrary::GetEditorWorld();
  // 程序化网格的创建
  AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>();
  MeshActor->SetActorLocation(FVector(Offset.X, Offset.Y, 0));
  UStaticMeshComponent* Mesh = MeshActor->GetStaticMeshComponent();  // 得到静态网格组件

  TArray<FVector> Vertices;
  TArray<int32> Triangles;

  TArray<FVector> Normals;  // 法线
  TArray<FLinearColor> Colors;
  TArray<FProcMeshTangent> Tangents;
  TArray<FVector2D> UVs;

  int VerticesInLine = (GridSize / GridSectionSize) + 1.0f;
  static int StaticMeshIndex = 0;
  for( int i = 0; i < VerticesInLine; i++ )  // 横向线上的顶点数
  {
    float X = (i * GridSectionSize);
    const int RoadMapX = i * 255 / VerticesInLine;
    for( int j = 0; j < VerticesInLine; j++ )  // 纵向线上的顶点数
    {
      float Y = (j * GridSectionSize);
      const int RoadMapY = j * 255 / VerticesInLine;
      const int CellIndex = RoadMapY + 255 * RoadMapX;
      float HeightValue = GetHeightForLandscape( FVector( (Offset.X + X),
                                                          (Offset.Y + Y),
                                                          0));
      Vertices.Add(FVector( X, Y, HeightValue));
      UVs.Add(FVector2D(i, j));
    }
  }

  //// 三角形的形成。每段2个三角形。
  for(int i = 0; i < VerticesInLine - 1; i++)
  {
    for(int j = 0; j < VerticesInLine - 1; j++)
    {
      Triangles.Add(   j       + (   i       * VerticesInLine ) );
      Triangles.Add( ( j + 1 ) + (   i       * VerticesInLine ) );
      Triangles.Add(   j       + ( ( i + 1 ) * VerticesInLine ) );

      Triangles.Add( ( j + 1 ) + (   i       * VerticesInLine ) );
      Triangles.Add( ( j + 1 ) + ( ( i + 1 ) * VerticesInLine ) );
      Triangles.Add(   j       + ( ( i + 1 ) * VerticesInLine ) );
    }
  }

  UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
    Vertices,
    Triangles,
    UVs,
    Normals,
    Tangents
  );

  FProceduralCustomMesh MeshData;
  MeshData.Vertices = Vertices;
  MeshData.Triangles = Triangles;
  MeshData.Normals = Normals;
  MeshData.UV0 = UVs;
  UStaticMesh* MeshToSet = UMapGenFunctionLibrary::CreateMesh(MeshData,  Tangents, DefaultLandscapeMaterial, MapName, "Terrain", FName(TEXT("SM_LandscapeMesh" + FString::FromInt(StaticMeshIndex) + GetStringForCurrentTile() )));
  Mesh->SetStaticMesh(MeshToSet);
  MeshActor->SetActorLabel("SM_LandscapeActor" + FString::FromInt(StaticMeshIndex) + GetStringForCurrentTile() );
  MeshActor->Tags.Add(FName("LandscapeToMove"));
  Mesh->CastShadow = false;
  Landscapes.Add(MeshActor);
  StaticMeshIndex++;
}

// 带冲突检查的参与者生成
AActor* UOpenDriveToMap::SpawnActorWithCheckNoCollisions(UClass* ActorClassToSpawn, FTransform Transform)
{
  UWorld* World = UEditorLevelLibrary::GetEditorWorld();
  FActorSpawnParameters SpawnParameters;
  SpawnParameters.bNoFail = true;
  // 生成参数 设置为 总是生成
  SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

  // 程序化网格的创建
  return World->SpawnActor<AActor>(ActorClassToSpawn, Transform, SpawnParameters);

}

// 独立地（启动单独的虚幻进程）生成地图瓦片
void UOpenDriveToMap::GenerateTileStandalone(){
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("UOpenDriveToMap::GenerateTileStandalone Function called"));

#if PLATFORM_WINDOWS  // Windows平台上调用 ExecuteTileCommandlet() 会导致虚幻编辑器崩溃
  GenerateTile();
#else
  ExecuteTileCommandlet();
#endif
  UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);  // 保存修改后的包
  UEditorLevelLibrary::SaveCurrentLevel();  // 保存当前关卡
}

// 生成地图瓦片
void UOpenDriveToMap::GenerateTile(){

  if( FilePath.IsEmpty() ){
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("UOpenDriveToMap::GenerateTile(): Failed to load %s"), *FilePath );
    return;
  }

  FString FileContent;
  UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("UOpenDriveToMap::GenerateTile(): File to load %s"), *FilePath );
  FFileHelper::LoadFileToString(FileContent, *FilePath);
  std::string opendrive_xml = carla::rpc::FromLongFString(FileContent);
  CarlaMap = carla::opendrive::OpenDriveParser::Load(opendrive_xml);  // 解析OpenDrive的xml文件

  if (!CarlaMap.has_value())
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Invalid Map"));
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("Valid Map loaded"));
    MapName = FPaths::GetCleanFilename(FilePath);  // 根据完整的路径获取文件名
    MapName.RemoveFromEnd(".xodr", ESearchCase::Type::IgnoreCase);  // 移除文件后缀获得文件名
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("MapName %s"), *MapName);
    UEditorLevelLibrary::LoadLevel(*BaseLevelName);  // 根据关卡名加载关卡

    AActor* QueryActor = UGameplayStatics::GetActorOfClass(
                            GEditor->GetEditorWorldContext().World(),
                            ALargeMapManager::StaticClass() );
    if( QueryActor != nullptr ){
      ALargeMapManager* LmManager = Cast<ALargeMapManager>(QueryActor);  // 获得大地图管理器
      LmManager->GenerateMap_Editor();
      NumTilesInXY  = LmManager->GetNumTilesInXY();  // 获得横向和纵向的瓦片数
      TileSize = LmManager->GetTileSize();  // 获得地图瓦片大小
      Tile0Offset = LmManager->GetTile0Offset();  // 获得瓦片的偏移

      FCarlaMapTile& CarlaTile =  LmManager->GetCarlaMapTile(CurrentTilesInXY);
      UEditorLevelLibrary::SaveCurrentLevel();

      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("Current Tile is %s"), *( CurrentTilesInXY.ToString() ) );
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("NumTilesInXY is %s"), *( NumTilesInXY.ToString() ) );
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("TileSize is %f"), ( TileSize ) );
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("Tile0Offset is %s"), *( Tile0Offset.ToString() ) );
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("Tile Name is %s"), *(CarlaTile.Name) );

      UEditorLevelLibrary::LoadLevel(CarlaTile.Name);

      MinPosition = FVector(CurrentTilesInXY.X * TileSize, CurrentTilesInXY.Y * -TileSize, 0.0f);  // 位置的最小值
      // 位置的最大值
      MaxPosition = FVector((CurrentTilesInXY.X + 1.0f ) * TileSize, (CurrentTilesInXY.Y + 1.0f) * -TileSize, 0.0f);

      GenerateAll(CarlaMap, MinPosition, MaxPosition);  // 生成所有地图
      Landscapes.Empty();
      bHasStarted = true;
      bRoadsFinished = true;
      bMapLoaded = true;
      bTileFinished = false;
    }else{
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Largemapmanager not found ") );
    }

    UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
    UEditorLevelLibrary::SaveCurrentLevel();
#if PLATFORM_LINUX  // 如果在Windows上移除根节点会导致虚幻编辑器崩溃（变量为空）
    RemoveFromRoot();
#endif

  }
}

// 转向下一个瓦片
bool UOpenDriveToMap::GoNextTile(){
  CurrentTilesInXY.X++;
  if( CurrentTilesInXY.X >= NumTilesInXY.X ){
    CurrentTilesInXY.X = 0;
    CurrentTilesInXY.Y++;
    if( CurrentTilesInXY.Y >= NumTilesInXY.Y ){
      return false;
    }
  }
  return true;
}

// 返回主关卡
void UOpenDriveToMap::ReturnToMainLevel(){
  Landscapes.Empty();
  FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false, nullptr);
  UEditorLevelLibrary::LoadLevel(*BaseLevelName);
}

// 在当前瓦片中校正所有参与者的位置
void UOpenDriveToMap::CorrectPositionForAllActorsInCurrentTile(){
  TArray<AActor*> FoundActors;
  // 获得所有参与者
  UGameplayStatics::GetAllActorsOfClass(UEditorLevelLibrary::GetEditorWorld(), AActor::StaticClass(), FoundActors);
  for( AActor* Current : FoundActors){
    Current->AddActorWorldOffset(-MinPosition, false);
    if( AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Current) ){
      UStaticMesh* StaticMesh = MeshActor->GetStaticMeshComponent()->GetStaticMesh();
      if(StaticMesh)
        StaticMesh->ClearFlags(RF_Standalone);
    }
  }
  CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
  GEngine->PerformGarbageCollectionAndCleanupActors();
}

// 获得当前瓦片的字符串
FString UOpenDriveToMap::GetStringForCurrentTile(){
  return FString("_X_") + FString::FromInt(CurrentTilesInXY.X) + FString("_Y_") + FString::FromInt(CurrentTilesInXY.Y);
}

AActor* UOpenDriveToMap::SpawnActorInEditorWorld(UClass* Class, FVector Location, FRotator Rotation){
  return UEditorLevelLibrary::GetEditorWorld()->SpawnActor<AActor>(Class,
    Location, Rotation);
}

void UOpenDriveToMap::OpenFileDialog()
{
  TArray<FString> OutFileNames;
  void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
  IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
  if (DesktopPlatform)
  {
    DesktopPlatform->OpenFileDialog(ParentWindowPtr, "Select xodr file", FPaths::ProjectDir(), FString(""), ".xodr", 1, OutFileNames);
  }
  for(FString& CurrentString : OutFileNames)
  {
    FilePath = CurrentString;
    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("FileObtained %s"), *CurrentString );
  }
}

// 加载OpenDrive地图
void UOpenDriveToMap::LoadMap()
{
  if( FilePath.IsEmpty() ){
    return;
  }

  FString FileContent;
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("UOpenDriveToMap::LoadMap(): File to load %s"), *FilePath );
  FFileHelper::LoadFileToString(FileContent, *FilePath);  // 将OpenDrive文件加载为字符串
  std::string opendrive_xml = carla::rpc::FromLongFString(FileContent);
  CarlaMap = carla::opendrive::OpenDriveParser::Load(opendrive_xml);  // 将OpenDrive的xml字符串解析为Carla的地图数据

  if (!CarlaMap.has_value())
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Invalid Map"));
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("Valid Map loaded"));
    MapName = FPaths::GetCleanFilename(FilePath);
    MapName.RemoveFromEnd(".xodr", ESearchCase::Type::IgnoreCase);
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("MapName %s"), *MapName);

    AActor* QueryActor = UGameplayStatics::GetActorOfClass(
                                UEditorLevelLibrary::GetEditorWorld(),
                                ALargeMapManager::StaticClass() );

    if( QueryActor != nullptr )
    {
      ALargeMapManager* LargeMapManager = Cast<ALargeMapManager>(QueryActor);
      NumTilesInXY  = LargeMapManager->GetNumTilesInXY();
      TileSize = LargeMapManager->GetTileSize();
      Tile0Offset = LargeMapManager->GetTile0Offset();
      CurrentTilesInXY = FIntVector(0,0,0);
      ULevel* PersistantLevel = UEditorLevelLibrary::GetEditorWorld()->PersistentLevel;
      BaseLevelName = LargeMapManager->LargeMapTilePath + "/" + LargeMapManager->LargeMapName;
      do{
        GenerateTileStandalone();
      }while(GoNextTile());
      ReturnToMainLevel();
    }
  }
}

TArray<AActor*> UOpenDriveToMap::GenerateMiscActors(float Offset, FVector MinLocation, FVector MaxLocation )
{
  carla::geom::Vector3D CarlaMinLocation(MinLocation.X / 100, MinLocation.Y / 100, MinLocation.Z /100);
  carla::geom::Vector3D CarlaMaxLocation(MaxLocation.X / 100, MaxLocation.Y / 100, MaxLocation.Z /100);

  std::vector<std::pair<carla::geom::Transform, std::string>>
    Locations = CarlaMap->GetTreesTransform(CarlaMinLocation, CarlaMaxLocation, DistanceBetweenTrees, DistanceFromRoadEdge, Offset);
  TArray<AActor*> Returning;
  static int i = 0;
  for (auto& cl : Locations)
  {
    const FVector scale{ 1.0f, 1.0f, 1.0f };
    cl.first.location.z = GetHeight(cl.first.location.x, cl.first.location.y) + 0.3f;
    FTransform NewTransform ( FRotator(cl.first.rotation), FVector(cl.first.location), scale );

    NewTransform = GetSnappedPosition(NewTransform);

    AActor* Spawner = UEditorLevelLibrary::GetEditorWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
      NewTransform.GetLocation(), NewTransform.Rotator());
    Spawner->Tags.Add(FName("MiscSpawnPosition"));
    Spawner->Tags.Add(FName(cl.second.c_str()));
    Spawner->SetActorLabel("MiscSpawnPosition" + FString::FromInt(i));
    ++i;
    Returning.Add(Spawner);
  }
  return Returning;
}

// 生成所有的资产
void UOpenDriveToMap::GenerateAll(const boost::optional<carla::road::Map>& ParamCarlaMap,
  FVector MinLocation,
  FVector MaxLocation )
{
  if (!ParamCarlaMap.has_value())
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Invalid Map"));
  }else
  {
    GenerateRoadMesh(ParamCarlaMap, MinLocation, MaxLocation);  // 生成道路网格
    GenerateLaneMarks(ParamCarlaMap, MinLocation, MaxLocation);  // 
    GenerateSpawnPoints(ParamCarlaMap, MinLocation, MaxLocation);
    CreateTerrain(12800, 256);
    GenerateTreePositions(ParamCarlaMap, MinLocation, MaxLocation);
    GenerationFinished(MinLocation, MaxLocation);
  }
}

void UOpenDriveToMap::GenerateRoadMesh( const boost::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation )
{
  opg_parameters.vertex_distance = 0.5f;
  opg_parameters.vertex_width_resolution = 8.0f;
  opg_parameters.simplification_percentage = 50.0f;
  double start = FPlatformTime::Seconds();

  carla::geom::Vector3D CarlaMinLocation(MinLocation.X / 100, MinLocation.Y / 100, MinLocation.Z /100);
  carla::geom::Vector3D CarlaMaxLocation(MaxLocation.X / 100, MaxLocation.Y / 100, MaxLocation.Z /100);
  const auto Meshes = ParamCarlaMap->GenerateOrderedChunkedMeshInLocations(opg_parameters, CarlaMinLocation, CarlaMaxLocation);
  double end = FPlatformTime::Seconds();
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT(" GenerateOrderedChunkedMesh code executed in %f seconds. Simplification percentage is %f"), end - start, opg_parameters.simplification_percentage);

  start = FPlatformTime::Seconds();
  static int index = 0;
  for (const auto &PairMap : Meshes)
  {
    for( auto& Mesh : PairMap.second )
    {
      if (!Mesh->GetVertices().size())
      {
        continue;
      }
      if (!Mesh->IsValid()) {
        continue;
      }

      if(PairMap.first == carla::road::Lane::LaneType::Driving)
      {
        for( auto& Vertex : Mesh->GetVertices() )
        {
          FVector VertexFVector = Vertex.ToFVector();
          Vertex.z += GetHeight(Vertex.x, Vertex.y, DistanceToLaneBorder(ParamCarlaMap,VertexFVector) > 65.0f );
        }
        carla::geom::Simplification Simplify(0.15);
        Simplify.Simplificate(Mesh);
      }else{
        for( auto& Vertex : Mesh->GetVertices() )
        {
          Vertex.z += GetHeight(Vertex.x, Vertex.y, false) + 0.15f;
        }
      }

      AStaticMeshActor* TempActor = UEditorLevelLibrary::GetEditorWorld()->SpawnActor<AStaticMeshActor>();
      UStaticMeshComponent* StaticMeshComponent = TempActor->GetStaticMeshComponent();
      TempActor->SetActorLabel(FString("SM_Lane_") + FString::FromInt(index));

      StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

      if(DefaultRoadMaterial && PairMap.first == carla::road::Lane::LaneType::Driving)
      {
        StaticMeshComponent->SetMaterial(0, DefaultRoadMaterial);
        StaticMeshComponent->CastShadow = false;
        TempActor->SetActorLabel(FString("SM_DrivingLane_") + FString::FromInt(index));
      }
      if(DefaultSidewalksMaterial && PairMap.first == carla::road::Lane::LaneType::Sidewalk)
      {
        StaticMeshComponent->SetMaterial(0, DefaultSidewalksMaterial);
        TempActor->SetActorLabel(FString("SM_Sidewalk_") + FString::FromInt(index));
      }
      FVector MeshCentroid = FVector(0,0,0);
      for( auto Vertex : Mesh->GetVertices() )
      {
        MeshCentroid += Vertex.ToFVector();
      }

      MeshCentroid /= Mesh->GetVertices().size();

      for( auto& Vertex : Mesh->GetVertices() )
      {
       Vertex.x -= MeshCentroid.X;
       Vertex.y -= MeshCentroid.Y;
       Vertex.z -= MeshCentroid.Z;
      }

      const FProceduralCustomMesh MeshData = *Mesh;
      TArray<FVector> Normals;
      TArray<FProcMeshTangent> Tangents;

      UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
        MeshData.Vertices,
        MeshData.Triangles,
        MeshData.UV0,
        Normals,
        Tangents
      );

      if(PairMap.first == carla::road::Lane::LaneType::Sidewalk)
      {
        UStaticMesh* MeshToSet = UMapGenFunctionLibrary::CreateMesh(MeshData,  Tangents, DefaultSidewalksMaterial, MapName, "DrivingLane", FName(TEXT("SM_SidewalkMesh" + FString::FromInt(index) + GetStringForCurrentTile() )));
        StaticMeshComponent->SetStaticMesh(MeshToSet);
      }

      if(PairMap.first == carla::road::Lane::LaneType::Driving)
      {
        UStaticMesh* MeshToSet = UMapGenFunctionLibrary::CreateMesh(MeshData,  Tangents, DefaultRoadMaterial, MapName, "DrivingLane", FName(TEXT("SM_DrivingLaneMesh" + FString::FromInt(index) + GetStringForCurrentTile() )));
        StaticMeshComponent->SetStaticMesh(MeshToSet);
      }
      TempActor->SetActorLocation(MeshCentroid * 100);
      TempActor->Tags.Add(FName("RoadLane"));
      // ActorMeshList.Add(TempActor);
      StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
      TempActor->SetActorEnableCollision(true);
      index++;
    }
  }

  end = FPlatformTime::Seconds();
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("Mesh spawnning and translation code executed in %f seconds."), end - start);
}

void UOpenDriveToMap::GenerateLaneMarks(const boost::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation )
{
  opg_parameters.vertex_distance = 0.5f;
  opg_parameters.vertex_width_resolution = 8.0f;
  opg_parameters.simplification_percentage = 15.0f;
  std::vector<std::string> lanemarkinfo;
  carla::geom::Vector3D CarlaMinLocation(MinLocation.X / 100, MinLocation.Y / 100, MinLocation.Z /100);
  carla::geom::Vector3D CarlaMaxLocation(MaxLocation.X / 100, MaxLocation.Y / 100, MaxLocation.Z /100);
  auto MarkingMeshes = ParamCarlaMap->GenerateLineMarkings(opg_parameters, CarlaMinLocation, CarlaMaxLocation, lanemarkinfo);
  TArray<AActor*> LaneMarkerActorList;
  static int meshindex = 0;
  int index = 0;
  for (const auto& Mesh : MarkingMeshes)
  {

    if ( !Mesh->GetVertices().size() )
    {
      index++;
      continue;
    }
    if ( !Mesh->IsValid() ) {
      index++;
      continue;
    }

    FVector MeshCentroid = FVector(0, 0, 0);
    for (auto& Vertex : Mesh->GetVertices())
    {
      FVector VertexFVector = Vertex.ToFVector();
      Vertex.z += GetHeight(Vertex.x, Vertex.y, DistanceToLaneBorder(ParamCarlaMap,VertexFVector) > 65.0f ) + 0.0001f;
      MeshCentroid += Vertex.ToFVector();
    }

    MeshCentroid /= Mesh->GetVertices().size();

    for (auto& Vertex : Mesh->GetVertices())
    {
      Vertex.x -= MeshCentroid.X;
      Vertex.y -= MeshCentroid.Y;
      Vertex.z -= MeshCentroid.Z;
    }

    // TODO: Improve this code
    float MinDistance = 99999999.9f;
    for(auto SpawnedActor : LaneMarkerActorList)
    {
      float VectorDistance = FVector::Distance(MeshCentroid*100, SpawnedActor->GetActorLocation());
      if(VectorDistance < MinDistance)
      {
        MinDistance = VectorDistance;
      }
    }

    if(MinDistance < 250)
    {
      UE_LOG(LogCarlaToolsMapGenerator, VeryVerbose, TEXT("Skkipped is %f."), MinDistance);
      index++;
      continue;
    }

    AStaticMeshActor* TempActor = UEditorLevelLibrary::GetEditorWorld()->SpawnActor<AStaticMeshActor>();
    UStaticMeshComponent* StaticMeshComponent = TempActor->GetStaticMeshComponent();
    TempActor->SetActorLabel(FString("SM_LaneMark_") + FString::FromInt(meshindex));
    StaticMeshComponent->CastShadow = false;
    if (lanemarkinfo[index].find("yellow") != std::string::npos) {
      if(DefaultLaneMarksYellowMaterial)
        StaticMeshComponent->SetMaterial(0, DefaultLaneMarksYellowMaterial);
    }else{
      if(DefaultLaneMarksWhiteMaterial)
        StaticMeshComponent->SetMaterial(0, DefaultLaneMarksWhiteMaterial);

    }

    const FProceduralCustomMesh MeshData = *Mesh;
    TArray<FVector> Normals;
    TArray<FProcMeshTangent> Tangents;
    UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
      MeshData.Vertices,
      MeshData.Triangles,
      MeshData.UV0,
      Normals,
      Tangents
    );

    UStaticMesh* MeshToSet = UMapGenFunctionLibrary::CreateMesh(MeshData,  Tangents, DefaultLandscapeMaterial, MapName, "LaneMark", FName(TEXT("SM_LaneMarkMesh" + FString::FromInt(meshindex) + GetStringForCurrentTile() )));
    StaticMeshComponent->SetStaticMesh(MeshToSet);
    TempActor->SetActorLocation(MeshCentroid * 100);
    TempActor->Tags.Add(*FString(lanemarkinfo[index].c_str()));
    TempActor->Tags.Add(FName("RoadLane"));
    LaneMarkerActorList.Add(TempActor);
    index++;
    meshindex++;
    TempActor->SetActorEnableCollision(false);
    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  }
}

void UOpenDriveToMap::GenerateSpawnPoints( const boost::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation  )
{
  float SpawnersHeight = 300.f;
  const auto Waypoints = ParamCarlaMap->GenerateWaypointsOnRoadEntries();
  TArray<AActor*> ActorsToMove;
  for (const auto &Wp : Waypoints)
  {
    const FTransform Trans = ParamCarlaMap->ComputeTransform(Wp);
    if( Trans.GetLocation().X >= MinLocation.X && Trans.GetLocation().Y >= MinLocation.Y &&
        Trans.GetLocation().X <= MaxLocation.X && Trans.GetLocation().Y <= MaxLocation.Y)
    {
      AVehicleSpawnPoint *Spawner = UEditorLevelLibrary::GetEditorWorld()->SpawnActor<AVehicleSpawnPoint>();
      Spawner->SetActorRotation(Trans.GetRotation());
      Spawner->SetActorLocation(Trans.GetTranslation() + FVector(0.f, 0.f, SpawnersHeight));
      ActorsToMove.Add(Spawner);
    }
  }
}

void UOpenDriveToMap::GenerateTreePositions( const boost::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation  )
{
  carla::geom::Vector3D CarlaMinLocation(MinLocation.X / 100, MinLocation.Y / 100, MinLocation.Z /100);
  carla::geom::Vector3D CarlaMaxLocation(MaxLocation.X / 100, MaxLocation.Y / 100, MaxLocation.Z /100);

  std::vector<std::pair<carla::geom::Transform, std::string>> Locations =
    ParamCarlaMap->GetTreesTransform(CarlaMinLocation, CarlaMaxLocation,DistanceBetweenTrees, DistanceFromRoadEdge );
  int i = 0;
  for (auto &cl : Locations)
  {
    const FVector scale{ 1.0f, 1.0f, 1.0f };
    cl.first.location.z  = GetHeight(cl.first.location.x, cl.first.location.y) + 0.3f;
    FTransform NewTransform ( FRotator(cl.first.rotation), FVector(cl.first.location), scale );
    NewTransform = GetSnappedPosition(NewTransform);

    AActor* Spawner = UEditorLevelLibrary::GetEditorWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
      NewTransform.GetLocation(), NewTransform.Rotator());

    Spawner->Tags.Add(FName("TreeSpawnPosition"));
    Spawner->Tags.Add(FName(cl.second.c_str()));
    Spawner->SetActorLabel("TreeSpawnPosition" + FString::FromInt(i) + GetStringForCurrentTile() );
    ++i;
  }
}

float UOpenDriveToMap::GetHeight(float PosX, float PosY, bool bDrivingLane){
  if( DefaultHeightmap ){
    const FColor* FormatedImageData = static_cast<const FColor*>( DefaultHeightmap->PlatformData->Mips[0].BulkData.LockReadOnly());

    int32 TextureSizeX = DefaultHeightmap->GetSizeX();
    int32 TextureSizeY = DefaultHeightmap->GetSizeY();

    int32 PixelX = ( ( PosX - WorldOriginPosition.X/100) / (WorldEndPosition.X/100 - WorldOriginPosition.X/100) ) * ((float)TextureSizeX);
    int32 PixelY = ( ( PosY - WorldOriginPosition.Y/100) / (WorldEndPosition.Y/100 - WorldOriginPosition.Y/100) ) * ((float)TextureSizeY);

    if( PixelX < 0 ){
      PixelX += TextureSizeX;
    }

    if( PixelY < 0 ){
      PixelY += TextureSizeY;
    }

    if( PixelX > TextureSizeX ){
      PixelX -= TextureSizeX;
    }

    if( PixelY > TextureSizeY ){
      PixelY -= TextureSizeY;
    }

    FColor PixelColor = FormatedImageData[PixelY * TextureSizeX + PixelX];

    //UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("PosX %f PosY %f "), PosX, PosY );
    //UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("WorldOriginPosition %s "), *WorldOriginPosition.ToString() );
    //UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("WorldEndPosition %s "), *WorldEndPosition.ToString() );
    //UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("PixelColor %s "), *WorldEndPosition.ToString() );
    //UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Reading Pixel X: %d Y %d Total Size X %d Y %d"), PixelX, PixelY, TextureSizeX, TextureSizeY );

    DefaultHeightmap->PlatformData->Mips[0].BulkData.Unlock();

    float LandscapeHeight = ( (PixelColor.R/255.0f ) * ( MaxHeight - MinHeight ) ) + MinHeight;

    if( bDrivingLane ){
      return LandscapeHeight -
        carla::geom::deformation::GetBumpDeformation(PosX,PosY);
    }else{
      return LandscapeHeight;
    }
  }else{
    if( bDrivingLane ){
      return carla::geom::deformation::GetZPosInDeformation(PosX, PosY) +
        (carla::geom::deformation::GetZPosInDeformation(PosX, PosY) * -0.3f) -
        carla::geom::deformation::GetBumpDeformation(PosX,PosY);
    }else{
      return carla::geom::deformation::GetZPosInDeformation(PosX, PosY) + (carla::geom::deformation::GetZPosInDeformation(PosX, PosY) * -0.3f);
    }
  }
}

FTransform UOpenDriveToMap::GetSnappedPosition( FTransform Origin ){
  FTransform ToReturn = Origin;
  FVector Start = Origin.GetLocation() + FVector( 0, 0, 10000);
  FVector End = Origin.GetLocation() - FVector( 0, 0, 10000);
  FHitResult HitResult;
  FCollisionQueryParams CollisionQuery;
  CollisionQuery.bTraceComplex = true;
  FCollisionResponseParams CollisionParams;

  if( UEditorLevelLibrary::GetEditorWorld()->LineTraceSingleByChannel(
    HitResult,
    Start,
    End,
    ECollisionChannel::ECC_WorldStatic,
    CollisionQuery,
    CollisionParams
  ) )
  {
    ToReturn.SetLocation(HitResult.Location);
  }
  return ToReturn;
}

float UOpenDriveToMap::GetHeightForLandscape( FVector Origin ){
  FVector Start = Origin + FVector( 0, 0, 10000);
  FVector End = Origin - FVector( 0, 0, 10000);
  FHitResult HitResult;
  FCollisionQueryParams CollisionQuery;
  CollisionQuery.AddIgnoredActors(Landscapes);
  FCollisionResponseParams CollisionParams;

  if( UEditorLevelLibrary::GetEditorWorld()->LineTraceSingleByChannel(
    HitResult,
    Start,
    End,
    ECollisionChannel::ECC_WorldStatic,
    CollisionQuery,
    CollisionParams) )
  {
    return GetHeight(Origin.X * 0.01f, Origin.Y * 0.01f, true) * 100.0f - 80.0f;
  }else{
    return GetHeight(Origin.X * 0.01f, Origin.Y * 0.01f, true) * 100.0f - 1.0f;
  }
  return 0.0f;
}

float UOpenDriveToMap::DistanceToLaneBorder(const boost::optional<carla::road::Map>& ParamCarlaMap,
        FVector &location, int32_t lane_type ) const
{
  carla::geom::Location cl(location);
  //wp = GetClosestWaypoint(pos). if distance wp - pos == lane_width --> estas al borde de la carretera
  auto wp = ParamCarlaMap->GetClosestWaypointOnRoad(cl, lane_type);
  if(wp)
  {
    carla::geom::Transform ct = ParamCarlaMap->ComputeTransform(*wp);
    double LaneWidth = ParamCarlaMap->GetLaneWidth(*wp);
    return cl.Distance(ct.location) - LaneWidth;
  }
  return 100000.0f;
}

bool UOpenDriveToMap::IsInRoad(
  const boost::optional<carla::road::Map>& ParamCarlaMap,
  FVector &location) const
{
  int32_t start = static_cast<int32_t>(carla::road::Lane::LaneType::Driving);
  int32_t end = static_cast<int32_t>(carla::road::Lane::LaneType::Sidewalk);
  for( int32_t i = start; i < end; ++i)
  {
    if(ParamCarlaMap->GetWaypoint(location, i))
    {
      return true;
    }
  }
  return false;
}

void UOpenDriveToMap::ImportXODR(){
  IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
  FString MyFileDestination = FPaths::ProjectContentDir() + "CustomMaps/" + MapName + "/OpenDrive/" + MapName + ".xodr";

  if(FileManager.CopyFile(  *MyFileDestination, *LocalFilePath,
                              EPlatformFileRead::None,
                              EPlatformFileWrite::None))
  {
    UE_LOG(LogCarlaToolsMapGenerator, Verbose, TEXT("FilePaths: File Copied!"));
    FilePath = MyFileDestination;
    LoadMap();
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("FilePaths local xodr file not copied: File not Copied!"));
  }
}

void UOpenDriveToMap::ImportOSM(){
  IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
  FString MyFileDestination = FPaths::ProjectContentDir() + "CustomMaps/" + MapName + "/OpenDrive/" + MapName + ".osm";

  if(FileManager.CopyFile(  *MyFileDestination, *LocalFilePath,
                              EPlatformFileRead::None,
                              EPlatformFileWrite::None))
  {
    UE_LOG(LogCarlaToolsMapGenerator, Verbose, TEXT("FilePaths: File Copied!"));
    ConvertOSMInOpenDrive();
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("FilePaths local osm file not copied: File not Copied!"));
  }
}

void UOpenDriveToMap::MoveActorsToSubLevels(TArray<AActor*> ActorsToMove)
{
  AActor* QueryActor = UGameplayStatics::GetActorOfClass(
                                UEditorLevelLibrary::GetEditorWorld(),
                                ALargeMapManager::StaticClass() );

  if( QueryActor != nullptr ){
    ALargeMapManager* LmManager = Cast<ALargeMapManager>(QueryActor);
    if( LmManager ){
      UEditorLevelLibrary::SaveCurrentLevel();
      UHoudiniImporterWidget::MoveActorsToSubLevelWithLargeMap(ActorsToMove, LmManager);
    }
  }
}

#endif
