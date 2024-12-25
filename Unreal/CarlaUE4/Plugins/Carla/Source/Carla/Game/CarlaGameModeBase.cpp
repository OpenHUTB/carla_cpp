// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaHUD.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Game/CarlaStaticDelegates.h"
#include "Carla/Lights/CarlaLight.h"
#include "Engine/DecalActor.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LocalPlayer.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMeshActor.h"
#include "Vehicle/VehicleSpawnPoint.h"
#include "Util/BoundingBoxCalculator.h"
#include "EngineUtils.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/opendrive/OpenDriveParser.h"
#include "carla/road/element/RoadInfoSignal.h"
#include <carla/rpc/EnvironmentObject.h>
#include <carla/rpc/WeatherParameters.h>
#include <carla/rpc/MapLayer.h>
#include <compiler/enable-ue4-macros.h>

#include "Async/ParallelFor.h"
#include "DynamicRHI.h"

#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

namespace cr = carla::road;
namespace crp = carla::rpc;
namespace cre = carla::road::element;

ACarlaGameModeBase::ACarlaGameModeBase(const FObjectInitializer& ObjectInitializer)//构造函数，它接收一个FObjectInitializer对象作为参数，并将其传递给父类的构造函数。这是Unreal Engine中常见的初始化模式。
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;//设置主Actor（Primary Actor）可以被tick（即定期更新）
  PrimaryActorTick.TickGroup = TG_PrePhysics;//设置主Actor的tick组为TG_PrePhysics，意味着这个Actor会在物理计算之前被更新。
  bAllowTickBeforeBeginPlay = false;//禁止在游戏正式开始之前（BeginPlay方法调用之前）对Actor进行tick。

  Episode = CreateDefaultSubobject<UCarlaEpisode>(TEXT("Episode"));//创建一个UCarlaEpisode类型的默认子对象，用于管理模拟的某一集（Episode）

  Recorder = CreateDefaultSubobject<ACarlaRecorder>(TEXT("Recorder"));//创建一个ACarlaRecorder类型的默认子对象，用于记录模拟过程中的数据。

  ObjectRegister = CreateDefaultSubobject<UObjectRegister>(TEXT("ObjectRegister"));//创建一个UObjectRegister类型的默认子对象，用于注册和管理模拟中的对象

  // HUD
  HUDClass = ACarlaHUD::StaticClass();

  TaggerDelegate = CreateDefaultSubobject<UTaggerDelegate>(TEXT("TaggerDelegate"));
  CarlaSettingsDelegate = CreateDefaultSubobject<UCarlaSettingsDelegate>(TEXT("CarlaSettingsDelegate"));
}

const FString ACarlaGameModeBase::GetRelativeMapPath() const//一个常量方法，返回一个FString类型的值，表示当前地图的相对路径
{
  UWorld* World = GetWorld();//获取当前游戏世界（World）的指针
  TSoftObjectPtr<UWorld> AssetPtr (World);//创建一个指向当前世界的软指针（Soft Pointer），用于安全地引用资源
  FString Path = FPaths::GetPath(AssetPtr.GetLongPackageName());//通过软指针获取世界的完整包名，并从中提取路径
  Path.RemoveFromStart("/Game/");//从路径中移除前缀"/Game/"，以获取相对于游戏目录的路径
  return Path;
}
//首先调用GetRelativeMapPath来获取地图的相对路径
//然后，使用FPaths::ConvertRelativePathToFull函数将项目的内容目录（FPaths::ProjectContentDir()）转换为绝对路径
//最后，将这个绝对路径与相对地图路径拼接起来，形成完整的地图路径，并返回这个路径
const FString ACarlaGameModeBase::GetFullMapPath() const
{
  FString Path = GetRelativeMapPath();
  return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()) + Path;
}

void ACarlaGameModeBase::InitGame(
    const FString &MapName,
    const FString &Options,
    FString &ErrorMessage)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ACarlaGameModeBase::InitGame);
  Super::InitGame(MapName, Options, ErrorMessage);//面向对象编程中常见的做法，用于确保父类中定义的初始化逻辑得到执行

  UWorld* World = GetWorld();
  check(World != nullptr);//用于验证 World 指针是否不为空
  FString InMapName(MapName);

  checkf(
      Episode != nullptr,
      TEXT("Missing episode, can't continue without an episode!"));//检查 Episode 指针是否不为空。如果 Episode 是空的，则程序将崩溃，并显示错误消息

  AActor* LMManagerActor =
      UGameplayStatics::GetActorOfClass(GetWorld(), ALargeMapManager::StaticClass());
  LMManager = Cast<ALargeMapManager>(LMManagerActor);//验证 Episode 对象是否存在，尝试获取游戏世界中的 ALargeMapManager Actor，并将其存储在成员变量中以供后续使用
  if (LMManager) {
    if (LMManager->GetNumTiles() == 0)
    {
      LMManager->GenerateLargeMap();
    }
    InMapName = LMManager->LargeMapName;
  }

#if WITH_EDITOR
    {
      //在编辑器中播放时，地图名称会得到一个额外的前缀，这里我们
      //删除它。
      FString CorrectedMapName = InMapName;
      constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
      CorrectedMapName.RemoveFromStart(PIEPrefix);
      UE_LOG(LogCarla, Log, TEXT("Corrected map name from %s to %s"), *InMapName, *CorrectedMapName);
      Episode->MapName = CorrectedMapName;
    }
#else
  Episode->MapName = InMapName;
#endif // WITH_EDITOR

  GameInstance = Cast<UCarlaGameInstance>(GetGameInstance());
  checkf(
      GameInstance != nullptr,
      TEXT("GameInstance is not a UCarlaGameInstance, did you forget to set "
           "it in the project settings?"));

  if (TaggerDelegate != nullptr) {
    TaggerDelegate->RegisterSpawnHandler(World);
  } else {
    UE_LOG(LogCarla, Error, TEXT("Missing TaggerDelegate!"));
  }

  if(CarlaSettingsDelegate != nullptr) {
    CarlaSettingsDelegate->ApplyQualityLevelPostRestart();
    CarlaSettingsDelegate->RegisterSpawnHandler(World);
  } else {
    UE_LOG(LogCarla, Error, TEXT("Missing CarlaSettingsDelegate!"));
  }

  AActor* WeatherActor =
      UGameplayStatics::GetActorOfClass(GetWorld(), AWeather::StaticClass());
  if (WeatherActor != nullptr) {
    UE_LOG(LogCarla, Log, TEXT("Existing weather actor. Doing nothing then!"));
    Episode->Weather = static_cast<AWeather*>(WeatherActor);
  }
  else if (WeatherClass != nullptr) {
    Episode->Weather = World->SpawnActor<AWeather>(WeatherClass);
  } else {
    UE_LOG(LogCarla, Error, TEXT("Missing weather class!"));
  }

  GameInstance->NotifyInitGame();

  OnEpisodeSettingsChangeHandle = FCarlaStaticDelegates::OnEpisodeSettingsChange.AddUObject(
        this,
        &ACarlaGameModeBase::OnEpisodeSettingsChanged);

  SpawnActorFactories();

  //在 Episode 和 Recorder 之间建立连接
  Recorder->SetEpisode(Episode);
  Episode->SetRecorder(Recorder);

  ParseOpenDrive();

  if(Map.has_value())
  {
    StoreSpawnPoints();
  }
}

void ACarlaGameModeBase::RestartPlayer(AController *NewPlayer)
{
  if (CarlaSettingsDelegate != nullptr)
  {
    CarlaSettingsDelegate->ApplyQualityLevelPreRestart();
  }

  Super::RestartPlayer(NewPlayer);
}

void ACarlaGameModeBase::BeginPlay()
{
  Super::BeginPlay();

  UWorld* World = GetWorld();
  check(World != nullptr);

  LoadMapLayer(GameInstance->GetCurrentMapLayer());
  ReadyToRegisterObjects = true;

  if (true) { /// @todo 如果启用了语义分割。
    ATagger::TagActorsInLevel(*World, true);
    TaggerDelegate->SetSemanticSegmentationEnabled();
  }

  // HACK: 修复透明度透视问题
  // 问题：透过墙壁可以看到透明物体。
  // 这是由于 SkyAtmosphere 组件
  // 定向光源 （太阳） 的阴影
  //并将 Custom depth 设置为 3，用于语义分割
  // 解决方案：生成一个贴花。
  // It just works!
  World->SpawnActor<ADecalActor>(
      FVector(0,0,-1000000), FRotator(0,0,0), FActorSpawnParameters());

  ATrafficLightManager* Manager = GetTrafficLightManager();
  Manager->InitializeTrafficLights();

  Episode->InitializeAtBeginPlay();
  GameInstance->NotifyBeginEpisode(*Episode);

  if (Episode->Weather != nullptr)
  {
    Episode->Weather->ApplyWeather(carla::rpc::WeatherParameters::Default);
  }

  /// @todoRecorder 不应该在这里打勾，FCarlaEngine 应该这样做。
  // 检查 Replayer 是否正在等待自动启动
  if (Recorder)
  {
    Recorder->GetReplayer()->CheckPlayAfterMapLoaded();
  }

  if(ReadyToRegisterObjects && PendingLevelsToLoad == 0)
  {
    RegisterEnvironmentObjects();
  }

  if (LMManager) {
    LMManager->RegisterInitialObjects();
    LMManager->ConsiderSpectatorAsEgo(Episode->GetSettings().SpectatorAsEgo);
  }

  // 手动运行 begin play on lights（开始在灯光上播放），因为它可能不会在子关卡上运行
  TArray<AActor*> FoundActors;
  UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
  for(AActor* Actor : FoundActors)
  {
    TArray<UCarlaLight*> Lights;
    Actor->GetComponents(Lights, false);
    for(UCarlaLight* Light : Lights)
    {
      Light->RegisterLight();
    }
  }
  EnableOverlapEvents();
}

TArray<FString> ACarlaGameModeBase::GetNamesOfAllActors()
{
  TArray<FString> Names;
  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
  for (AActor* Actor : Actors)
  {
    TArray<UStaticMeshComponent*> StaticMeshes;
    Actor->GetComponents(StaticMeshes);
    if (StaticMeshes.Num())
    {
      Names.Add(Actor->GetName());
    }
  }
  return Names;
}

AActor* ACarlaGameModeBase::FindActorByName(const FString& ActorName)
{
  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
  for (AActor* Actor : Actors)
  {
    if(Actor->GetName() == ActorName)
    {
      return Actor;
      break;
    }
  }
  return nullptr;
}

UTexture2D* ACarlaGameModeBase::CreateUETexture(const carla::rpc::TextureColor& Texture)
{
  FlushRenderingCommands();
  TArray<FColor> Colors;
  for (uint32_t y = 0; y < Texture.GetHeight(); y++)
  {
    for (uint32_t x = 0; x < Texture.GetWidth(); x++)
    {
      auto& Color = Texture.At(x,y);
      Colors.Add(FColor(Color.r, Color.g, Color.b, Color.a));
    }
  }
  UTexture2D* UETexture = UTexture2D::CreateTransient(Texture.GetWidth(), Texture.GetHeight(), EPixelFormat::PF_B8G8R8A8);
  FTexture2DMipMap& Mip = UETexture->PlatformData->Mips[0];
  void* Data = Mip.BulkData.Lock( LOCK_READ_WRITE );
  FMemory::Memcpy( Data,
      &Colors[0],
      Texture.GetWidth()*Texture.GetHeight()*sizeof(FColor));
  Mip.BulkData.Unlock();
  UETexture->UpdateResource();
  return UETexture;
}

UTexture2D* ACarlaGameModeBase::CreateUETexture(const carla::rpc::TextureFloatColor& Texture)
{
  FlushRenderingCommands();
  TArray<FFloat16Color> Colors;
  for (uint32_t y = 0; y < Texture.GetHeight(); y++)
  {
    for (uint32_t x = 0; x < Texture.GetWidth(); x++)
    {
      auto& Color = Texture.At(x,y);
      Colors.Add(FLinearColor(Color.r, Color.g, Color.b, Color.a));
    }
  }
  UTexture2D* UETexture = UTexture2D::CreateTransient(Texture.GetWidth(), Texture.GetHeight(), EPixelFormat::PF_FloatRGBA);
  FTexture2DMipMap& Mip = UETexture->PlatformData->Mips[0];
  void* Data = Mip.BulkData.Lock( LOCK_READ_WRITE );
  FMemory::Memcpy( Data,
      &Colors[0],
      Texture.GetWidth()*Texture.GetHeight()*sizeof(FFloat16Color));
  Mip.BulkData.Unlock();
  UETexture->UpdateResource();
  return UETexture;
}

void ACarlaGameModeBase::ApplyTextureToActor(
    AActor* Actor,
    UTexture2D* Texture,
    const carla::rpc::MaterialParameter& TextureParam)
{
  namespace cr = carla::rpc;
  TArray<UStaticMeshComponent*> StaticMeshes;
  Actor->GetComponents(StaticMeshes);
  for (UStaticMeshComponent* Mesh : StaticMeshes)
  {
    for (int i = 0; i < Mesh->GetNumMaterials(); ++i)
    {
      UMaterialInterface* OriginalMaterial = Mesh->GetMaterial(i);
      UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(OriginalMaterial);
      if(!DynamicMaterial)
      {
        DynamicMaterial = UMaterialInstanceDynamic::Create(OriginalMaterial, NULL);
        Mesh->SetMaterial(i, DynamicMaterial);
      }

      switch(TextureParam)
      {
        case cr::MaterialParameter::Tex_Diffuse:
          DynamicMaterial->SetTextureParameterValue("BaseColor", Texture);
          DynamicMaterial->SetTextureParameterValue("Difuse", Texture);
          DynamicMaterial->SetTextureParameterValue("Difuse 2", Texture);
          DynamicMaterial->SetTextureParameterValue("Difuse 3", Texture);
          DynamicMaterial->SetTextureParameterValue("Difuse 4", Texture);
          break;
        case cr::MaterialParameter::Tex_Normal:
          DynamicMaterial->SetTextureParameterValue("Normal", Texture);
          DynamicMaterial->SetTextureParameterValue("Normal 2", Texture);
          DynamicMaterial->SetTextureParameterValue("Normal 3", Texture);
          DynamicMaterial->SetTextureParameterValue("Normal 4", Texture);
          break;
        case cr::MaterialParameter::Tex_Emissive:
          DynamicMaterial->SetTextureParameterValue("Emissive", Texture);
          break;
        case cr::MaterialParameter::Tex_Ao_Roughness_Metallic_Emissive:
          DynamicMaterial->SetTextureParameterValue("AO / Roughness / Metallic / Emissive", Texture);
          DynamicMaterial->SetTextureParameterValue("ORMH", Texture);
          DynamicMaterial->SetTextureParameterValue("ORMH 2", Texture);
          DynamicMaterial->SetTextureParameterValue("ORMH 3", Texture);
          DynamicMaterial->SetTextureParameterValue("ORMH 4", Texture);
          break;
      }
    }
  }
}

void ACarlaGameModeBase::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  /// @todo Recorder 不应该在这里打勾，FCarlaEngine 应该这样做。
  if (Recorder)
  {
    Recorder->Tick(DeltaSeconds);
  }
}

void ACarlaGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  FCarlaStaticDelegates::OnEpisodeSettingsChange.Remove(OnEpisodeSettingsChangeHandle);

  Episode->EndPlay();
  GameInstance->NotifyEndEpisode();

  Super::EndPlay(EndPlayReason);

  if ((CarlaSettingsDelegate != nullptr) && (EndPlayReason != EEndPlayReason::EndPlayInEditor))
  {
    CarlaSettingsDelegate->Reset();
  }
}

void ACarlaGameModeBase::SpawnActorFactories()
{
  auto *World = GetWorld();
  check(World != nullptr);

  for (auto &FactoryClass : ActorFactories)
  {
    if (FactoryClass != nullptr)
    {
      auto *Factory = World->SpawnActor<ACarlaActorFactory>(FactoryClass);
      if (Factory != nullptr)
      {
        Episode->RegisterActorFactory(*Factory);
        ActorFactoryInstances.Add(Factory);
      }
      else
      {
        UE_LOG(LogCarla, Error, TEXT("Failed to spawn actor spawner"));
      }
    }
  }
}

void ACarlaGameModeBase::StoreSpawnPoints()
{
  for (TActorIterator<AVehicleSpawnPoint> It(GetWorld()); It; ++It)
  {
    SpawnPointsTransforms.Add(It->GetActorTransform());
  }

  if(SpawnPointsTransforms.Num() == 0)
  {
    GenerateSpawnPoints();
  }

  UE_LOG(LogCarla, Log, TEXT("There are %d SpawnPoints in the map"), SpawnPointsTransforms.Num());
}

void ACarlaGameModeBase::GenerateSpawnPoints()
{
  // 记录日志，表明正在生成出生点
  UE_LOG(LogCarla, Log, TEXT("Generating SpawnPoints ..."));
   // 从地图对象中获取拓扑结构，拓扑结构由一系列的路径点对（Waypoint pairs）组成
  std::vector<std::pair<carla::road::element::Waypoint, carla::road::element::Waypoint>> Topology = Map->GenerateTopology();
  // 获取当前的游戏世界对象
  UWorld* World = GetWorld();
  // 遍历拓扑结构中的每一对路径点
  for(auto& Pair : Topology)
  {
    carla::geom::Transform CarlaTransform = Map->ComputeTransform(Pair.first);
    FTransform Transform(CarlaTransform);
 // 将Transform的平移部分增加50个单位的Z轴偏移量
 // 为了将出生点放置在道路的上方一定高度，以避免与地面或其他物体碰撞
    Transform.AddToTranslation(FVector(0.f, 0.f, 50.0f));
    SpawnPointsTransforms.Add(Transform);
  }
}

void ACarlaGameModeBase::ParseOpenDrive()
{
  std::string opendrive_xml = carla::rpc::FromLongFString(UOpenDrive::GetXODR(GetWorld()));
// 使用carla::opendrive::OpenDriveParser的Load方法来解析OpenDrive XML字符串
// 并尝试创建一个地图对象
 // 如果解析失败，则std::optional将不包含值
  Map = carla::opendrive::OpenDriveParser::Load(opendrive_xml);
  // 检查Map是否包含值，即检查OpenDrive XML是否成功解析
  if (!Map.has_value()) {
    UE_LOG(LogCarla, Error, TEXT("Invalid Map"));
  }
  else
  {
    Episode->MapGeoReference = Map->GetGeoReference();
  }
}

ATrafficLightManager* ACarlaGameModeBase::GetTrafficLightManager()
{
  if (!TrafficLightManager)
  {
    UWorld* World = GetWorld();
    AActor* TrafficLightManagerActor = UGameplayStatics::GetActorOfClass(World, ATrafficLightManager::StaticClass());
    if(TrafficLightManagerActor == nullptr)
    {
      FActorSpawnParameters SpawnParams;
      SpawnParams.OverrideLevel = GetULevelFromName("TrafficLights");
      TrafficLightManager = World->SpawnActor<ATrafficLightManager>(SpawnParams);
    }
    else
    {
      TrafficLightManager = Cast<ATrafficLightManager>(TrafficLightManagerActor);
    }
  }
  return TrafficLightManager;
}

void ACarlaGameModeBase::CheckForEmptyMeshes()
{
  // 创建一个AActor指针的数组，用于存储游戏世界中的所有AStaticMeshActor对象
  TArray<AActor*> WorldActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), WorldActors);

  for (AActor *Actor : WorldActors)
  {
    AStaticMeshActor *MeshActor = CastChecked<AStaticMeshActor>(Actor);
// 获取MeshActor的静态网格体组件
// 然后检查该组件是否有关联的静态网格体资源    
    if (MeshActor->GetStaticMeshComponent()->GetStaticMesh() == NULL)
    {
      UE_LOG(LogTemp, Error, TEXT("The object : %s has no mesh"), *MeshActor->GetFullName());
    }
  }
}

//正在遍历游戏世界中的所有AStaticMeshActor对象，并检查它们的静态网格体组件是否有一个有效的静态网格体资源以及一个特定的标签
//如果静态网格体组件没有被标记为道路、人行道、道路线、地面或地形，并且当前没有生成重叠事件，则启用这些事件
void ACarlaGameModeBase::EnableOverlapEvents()
{
  TArray<AActor*> WorldActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), WorldActors);

  for(AActor *Actor : WorldActors)
  {
    AStaticMeshActor *MeshActor = CastChecked<AStaticMeshActor>(Actor);
    if(MeshActor->GetStaticMeshComponent()->GetStaticMesh() != NULL)
    {
      auto MeshTag = ATagger::GetTagOfTaggedComponent(*MeshActor->GetStaticMeshComponent());
      namespace crp = carla::rpc;
      if (MeshTag != crp::CityObjectLabel::Roads && 
          MeshTag != crp::CityObjectLabel::Sidewalks && 
          MeshTag != crp::CityObjectLabel::RoadLines && 
          MeshTag != crp::CityObjectLabel::Ground &&
          MeshTag != crp::CityObjectLabel::Terrain &&
          MeshActor->GetStaticMeshComponent()->GetGenerateOverlapEvents() == false)
      {
        MeshActor->GetStaticMeshComponent()->SetGenerateOverlapEvents(true);
      }
    }
  }
}

void ACarlaGameModeBase::DebugShowSignals(bool enable)
{

  auto World = GetWorld();
  check(World != nullptr);

  if(!Map)
  {
    return;
  }

  if(!enable)
  {
    UKismetSystemLibrary::FlushDebugStrings(World);
    UKismetSystemLibrary::FlushPersistentDebugLines(World);
    return;
  }

  //const std::unordered_map<carla::road::SignId, std::unique_ptr<carla::road::Signal>>
  const auto& Signals = Map->GetSignals();
  const auto& Controllers = Map->GetControllers();

  for(const auto& Signal : Signals) {
    const auto& ODSignal = Signal.second;
    const FTransform Transform = ODSignal->GetTransform();
    const FVector Location = Transform.GetLocation();
    const FQuat Rotation = Transform.GetRotation();
    const FVector Up = Rotation.GetUpVector();
    DrawDebugSphere(
      World,
      Location,
      50.0f,
      10,
      FColor(0, 255, 0),
      true
    );
  }

  TArray<const cre::RoadInfoSignal*> References;
  auto waypoints = Map->GenerateWaypointsOnRoadEntries();
  std::unordered_set<cr::RoadId> ExploredRoads;
  for (auto & waypoint : waypoints)
  {
    // 检查我们是否已经探索过这条路
    if (ExploredRoads.count(waypoint.road_id) > 0)
    {
      continue;
    }
    ExploredRoads.insert(waypoint.road_id);

    //同一条道路多次（性能影响，而不是行为）
    auto SignalReferences = Map->GetLane(waypoint).
        GetRoad()->GetInfos<cre::RoadInfoSignal>();
    for (auto *SignalReference : SignalReferences)
    {
      References.Add(SignalReference);
    }
  }
  for (auto& Reference : References)
  {
    auto RoadId = Reference->GetRoadId();
    const auto* SignalReference = Reference;
    const FTransform SignalTransform = SignalReference->GetSignal()->GetTransform();
    for(auto &validity : SignalReference->GetValidities())
    {
      for(auto lane : carla::geom::Math::GenerateRange(validity._from_lane, validity._to_lane))
      {
        if(lane == 0)
          continue;

        auto signal_waypoint = Map->GetWaypoint(
            RoadId, lane, SignalReference->GetS()).get();

        if(Map->GetLane(signal_waypoint).GetType() != cr::Lane::LaneType::Driving)
          continue;

        FTransform ReferenceTransform = Map->ComputeTransform(signal_waypoint);

        DrawDebugSphere(
            World,
            ReferenceTransform.GetLocation(),
            50.0f,
            10,
            FColor(0, 0, 255),
            true
        );

        DrawDebugLine(
            World,
            ReferenceTransform.GetLocation(),
            SignalTransform.GetLocation(),
            FColor(0, 0, 255),
            true
        );
      }
    }
  }

}

TArray<FBoundingBox> ACarlaGameModeBase::GetAllBBsOfLevel(uint8 TagQueried) const
{
  UWorld* World = GetWorld();

  // 获取该级别的所有 Actor
  TArray<AActor*> FoundActors;
  UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);

  TArray<FBoundingBox> BoundingBoxes;
  BoundingBoxes = UBoundingBoxCalculator::GetBoundingBoxOfActors(FoundActors, TagQueried);

  return BoundingBoxes;
}

void ACarlaGameModeBase::RegisterEnvironmentObjects()
{
  //获取该级别的所有 Actor
  TArray<AActor*> FoundActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
  ObjectRegister->RegisterObjects(FoundActors);
}

void ACarlaGameModeBase::EnableEnvironmentObjects(
  const TSet<uint64>& EnvObjectIds,
  bool Enable)
{
  ObjectRegister->EnableEnvironmentObjects(EnvObjectIds, Enable);
}

void ACarlaGameModeBase::LoadMapLayer(int32 MapLayers)
{
  const UWorld* World = GetWorld();
  UGameplayStatics::FlushLevelStreaming(World);

  TArray<FName> LevelsToLoad;
  ConvertMapLayerMaskToMapNames(MapLayers, LevelsToLoad);

  FLatentActionInfo LatentInfo;
  LatentInfo.CallbackTarget = this;
  LatentInfo.ExecutionFunction = "OnLoadStreamLevel";
  LatentInfo.Linkage = 0;
  LatentInfo.UUID = LatentInfoUUID;

  PendingLevelsToLoad = LevelsToLoad.Num();

  for(FName& LevelName : LevelsToLoad)
  {
    LatentInfoUUID++;
    UGameplayStatics::LoadStreamLevel(World, LevelName, true, true, LatentInfo);
    LatentInfo.UUID = LatentInfoUUID;
    UGameplayStatics::FlushLevelStreaming(World);
  }
}

void ACarlaGameModeBase::UnLoadMapLayer(int32 MapLayers)
{
  const UWorld* World = GetWorld();

  TArray<FName> LevelsToUnLoad;
  ConvertMapLayerMaskToMapNames(MapLayers, LevelsToUnLoad);

  FLatentActionInfo LatentInfo;
  LatentInfo.CallbackTarget = this;
  LatentInfo.ExecutionFunction = "OnUnloadStreamLevel";
  LatentInfo.UUID = LatentInfoUUID;
  LatentInfo.Linkage = 0;

  PendingLevelsToUnLoad = LevelsToUnLoad.Num();

  for(FName& LevelName : LevelsToUnLoad)
  {
    LatentInfoUUID++;
    UGameplayStatics::UnloadStreamLevel(World, LevelName, LatentInfo, false);
    LatentInfo.UUID = LatentInfoUUID;
    UGameplayStatics::FlushLevelStreaming(World);
  }

}

void ACarlaGameModeBase::ConvertMapLayerMaskToMapNames(int32 MapLayer, TArray<FName>& OutLevelNames)
{
  UWorld* World = GetWorld();
  const TArray <ULevelStreaming*> Levels = World->GetStreamingLevels();
  TArray<FString> LayersToLoad;

  // 获取所有请求的图层
  int32 LayerMask = 1;
  int32 AllLayersMask = static_cast<crp::MapLayerType>(crp::MapLayer::All);

  while(LayerMask > 0)
  {
    //将枚举转换为 FString
    FString LayerName = UTF8_TO_TCHAR(MapLayerToString(static_cast<crp::MapLayer>(LayerMask)).c_str());
    bool included = static_cast<crp::MapLayerType>(MapLayer) & LayerMask;
    if(included)
    {
      LayersToLoad.Emplace(LayerName);
    }
    LayerMask = (LayerMask << 1) & AllLayersMask;
  }

  // 获取所有请求的关卡地图
  for(ULevelStreaming* Level : Levels)
  {
    TArray<FString> StringArray;
    FString FullSubMapName = Level->GetWorldAssetPackageFName().ToString();
    // 丢弃完整路径，我们只需要 umap 名称
    FullSubMapName.ParseIntoArray(StringArray, TEXT("/"), false);
    FString SubMapName = StringArray[StringArray.Num() - 1];
    for(FString LayerName : LayersToLoad)
    {
      if(SubMapName.Contains(LayerName))
      {
        OutLevelNames.Emplace(FName(*SubMapName));
        break;
      }
    }
  }

}

ULevel* ACarlaGameModeBase::GetULevelFromName(FString LevelName)
{
  ULevel* OutLevel = nullptr;
  UWorld* World = GetWorld();
  const TArray <ULevelStreaming*> Levels = World->GetStreamingLevels();

  for(ULevelStreaming* Level : Levels)
  {
    FString FullSubMapName = Level->GetWorldAssetPackageFName().ToString();
    if(FullSubMapName.Contains(LevelName))
    {
      OutLevel = Level->GetLoadedLevel();
      if(!OutLevel)
      {
        UE_LOG(LogCarla, Warning, TEXT("%s has not been loaded"), *LevelName);
      }
      break;
    }
  }

  return OutLevel;
}

void ACarlaGameModeBase::OnLoadStreamLevel()
{
  PendingLevelsToLoad--;

  // 注册新 actor 并标记他们
  if(ReadyToRegisterObjects && PendingLevelsToLoad == 0)
  {
    RegisterEnvironmentObjects();
    ATagger::TagActorsInLevel(*GetWorld(), true);
  }
}

void ACarlaGameModeBase::OnUnloadStreamLevel()
{
  PendingLevelsToUnLoad--;
  // 更新已存储的已注册对象（丢弃已删除的对象）
  if(ReadyToRegisterObjects && PendingLevelsToUnLoad == 0)
  {
    RegisterEnvironmentObjects();
  }
}

void ACarlaGameModeBase::OnEpisodeSettingsChanged(const FEpisodeSettings &Settings)
{
  CarlaSettingsDelegate->SetAllActorsDrawDistance(GetWorld(), Settings.MaxCullingDistance);
}
