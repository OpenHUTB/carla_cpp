// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h" // 包含主Carla头文件，提供Carla仿真框架的核心功能
#include "Carla/Game/CarlaEpisode.h" // 包含Carla游戏环节的头文件，表示仿真会话
#include "Carla/Game/CarlaStatics.h" // 包含Carla静态变量和函数的头文件，提供全局访问点

#include <compiler/disable-ue4-macros.h> // 禁用Unreal Engine的宏，防止与Carla代码冲突
#include <carla/opendrive/OpenDriveParser.h> // 包含Carla OpenDrive解析器的头文件，用于解析OpenDrive文件
#include <carla/rpc/String.h> // 包含Carla RPC字符串类型的头文件，用于网络通信
#include <compiler/enable-ue4-macros.h> // 启用Unreal Engine的宏

#include "Carla/Sensor/Sensor.h" // 包含Carla传感器的头文件，表示仿真中的传感器
#include "Carla/Util/BoundingBoxCalculator.h" // 包含边界框计算器的头文件，用于计算Actor的边界框
#include "Carla/Util/RandomEngine.h" // 包含随机引擎的头文件，提供随机数生成功能
#include "Carla/Vehicle/VehicleSpawnPoint.h" // 包含车辆生成点的头文件，表示车辆在地图上的生成位置
#include "Carla/Game/CarlaStatics.h" // 重复包含，可能是为了强调其重要性
#include "Carla/Game/CarlaStaticDelegates.h" // 包含Carla静态委托的头文件，用于注册和管理事件
#include "Carla/MapGen/LargeMapManager.h" // 包含大地图管理器的头文件，管理大型开放世界地图

#include "Engine/StaticMeshActor.h" // 包含Unreal Engine静态网格Actor的头文件，表示3D模型
#include "EngineUtils.h" // 包含Unreal Engine工具的头文件，提供通用工具函数
#include "GameFramework/SpectatorPawn.h" // 包含观察者Pawn的头文件，表示玩家的观察角色
#include "GenericPlatform/GenericPlatformProcess.h" // 包含通用平台进程的头文件，提供跨平台进程功能
#include "Kismet/GameplayStatics.h" // 包含Kismet游戏逻辑静态函数的头文件，提供蓝图可用的静态函数
#include "Materials/MaterialParameterCollection.h" // 包含材质参数集合的头文件，用于材质参数管理
#include "Materials/MaterialParameterCollectionInstance.h" // 包含材质参数集合实例的头文件，用于材质参数的实例化
#include "Misc/FileHelper.h" // 包含文件帮助函数的头文件，提供文件操作功能
#include "Misc/Paths.h" // 包含路径管理的头文件，提供路径相关功能

// 静态函数，根据交通标志状态返回对应的标识符
static FString UCarlaEpisode_GetTrafficSignId(ETrafficSignState State)
{
  using TSS = ETrafficSignState; // 使用别名TSS简化ETrafficSignState的调用
  switch (State) // 根据交通标志状态进行switch判断
  {
    case TSS::TrafficLightRed: // 红灯状态
    case TSS::TrafficLightYellow: // 黄灯状态
    case TSS::TrafficLightGreen: // 绿灯状态
      return TEXT("traffic.traffic_light"); // 返回交通灯的标识符
    case TSS::SpeedLimit_30: // 速度限制30标识
      return TEXT("traffic.speed_limit.30");
    case TSS::SpeedLimit_40: // 速度限制40标识
      return TEXT("traffic.speed_limit.40");
    case TSS::SpeedLimit_50: // 速度限制50标识
      return TEXT("traffic.speed_limit.50");
    case TSS::SpeedLimit_60: // 速度限制60标识
      return TEXT("traffic.speed_limit.60");
    case TSS::SpeedLimit_90: // 速度限制90标识
      return TEXT("traffic.speed_limit.90");
    case TSS::SpeedLimit_100: // 速度限制100标识
      return TEXT("traffic.speed_limit.100");
    case TSS::SpeedLimit_120: // 速度限制120标识
      return TEXT("traffic.speed_limit.120");
    case TSS::SpeedLimit_130: // 速度限制130标识
      return TEXT("traffic.speed_limit.130");
    case TSS::StopSign: // 停车标识
      return TEXT("traffic.stop");
    case TSS::YieldSign: // 让路标识
      return TEXT("traffic.yield");
    default: // 其他未知状态
      return TEXT("traffic.unknown"); // 返回未知交通标志的标识符
  }
}


UCarlaEpisode::UCarlaEpisode(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer),
    Id(URandomEngine::GenerateRandomId())
{
  ActorDispatcher = CreateDefaultSubobject<UActorDispatcher>(TEXT("ActorDispatcher"));
  FrameData.SetEpisode(this);
}

bool UCarlaEpisode::LoadNewEpisode(const FString &MapString, bool ResetSettings)
{
  bool bIsFileFound = false;

  FString FinalPath = MapString.IsEmpty() ? GetMapName() : MapString;
  FinalPath += !MapString.EndsWith(".umap") ? ".umap" : "";

  if (MapString.StartsWith("/Game"))
  {
    // 一些转化...
    FinalPath.RemoveFromStart(TEXT("/Game/"));
    FinalPath = FPaths::ProjectContentDir() + FinalPath;
    FinalPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FinalPath);

    if (FPaths::FileExists(FinalPath)) {
      bIsFileFound = true;
      FinalPath = MapString;
    }
  }
  else
  {
    if (MapString.Contains("/")) return false;

    // 在 Carla 下查找完整路径
    TArray<FString> TempStrArray, PathList;
    IFileManager::Get().FindFilesRecursive(PathList, *FPaths::ProjectContentDir(), *FinalPath, true, false, false);
    if (PathList.Num() > 0)
    {
      FinalPath = PathList[0];
      FinalPath.ParseIntoArray(TempStrArray, TEXT("Content/"), true);
      FinalPath = TempStrArray[1];
      FinalPath.ParseIntoArray(TempStrArray, TEXT("."), true);
      FinalPath = "/Game/" + TempStrArray[0];

      return LoadNewEpisode(FinalPath, ResetSettings);
    }
  }

  if (bIsFileFound)
  {
    UE_LOG(LogCarla, Warning, TEXT("Loading a new episode: %s"), *FinalPath);
    UGameplayStatics::OpenLevel(GetWorld(), *FinalPath, true);
    if (ResetSettings)
      ApplySettings(FEpisodeSettings{});

    // 向所有辅助服务器发送 'LOAD_MAP' 命令（如果有）
    if (bIsPrimaryServer)
    {
      UCarlaGameInstance *GameInstance = UCarlaStatics::GetGameInstance(GetWorld());
      if(GameInstance)
      {
        FCarlaEngine *CarlaEngine = GameInstance->GetCarlaEngine();
        auto SecondaryServer = CarlaEngine->GetSecondaryServer();
        if (SecondaryServer->HasClientsConnected())
        {
          SecondaryServer->GetCommander().SendLoadMap(std::string(TCHAR_TO_UTF8(*FinalPath)));
        }
      }
    }
  }
  return bIsFileFound;
}

static FString BuildRecastBuilderFile()
{
  // 定义带扩展名的文件名，具体取决于我们是否在 Windows 上
#if PLATFORM_WINDOWS
  const FString RecastToolName = "RecastBuilder.exe";
#else
  const FString RecastToolName = "RecastBuilder";
#endif // PLATFORM_WINDOWS

  // 根据 UE4 构建类型（包或编辑器）定义路径
#if UE_BUILD_SHIPPING
  const FString AbsoluteRecastBuilderPath = FPaths::ConvertRelativePathToFull(
      FPaths::RootDir() + "Tools/" + RecastToolName);
#else
  const FString AbsoluteRecastBuilderPath = FPaths::ConvertRelativePathToFull(
      FPaths::ProjectDir() + "../../Util/DockerUtils/dist/" + RecastToolName);
#endif
  return AbsoluteRecastBuilderPath;
}

bool UCarlaEpisode::LoadNewOpendriveEpisode(
    const FString &OpenDriveString,
    const carla::rpc::OpendriveGenerationParameters &Params)
{
  if (OpenDriveString.IsEmpty())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive string is empty."));
    return false;
  }

  // 根据 OpenDRIVE 数据构建地图
  const auto CarlaMap = carla::opendrive::OpenDriveParser::Load(
      carla::rpc::FromLongFString(OpenDriveString));

  // 检查地图是否正确生成
  if (!CarlaMap.has_value())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive string is invalid or not supported"));
    return false;
  }

  // 生成 OBJ（作为字符串）
  const auto RoadMesh = CarlaMap->GenerateMesh(Params.vertex_distance);
  const auto CrosswalksMesh = CarlaMap->GetAllCrosswalkMesh();
  const auto RecastOBJ = (RoadMesh + CrosswalksMesh).GenerateOBJForRecast();

  const FString AbsoluteOBJPath = FPaths::ConvertRelativePathToFull(
      FPaths::ProjectContentDir() + "Carla/Maps/Nav/OpenDriveMap.obj");

  // 将 OBJ 字符串存储到文件中，以便 RecastBuilder 可以加载它
  FFileHelper::SaveStringToFile(
      carla::rpc::ToLongFString(RecastOBJ),
      *AbsoluteOBJPath,
      FFileHelper::EEncodingOptions::ForceUTF8,
      &IFileManager::Get());

  const FString AbsoluteXODRPath = FPaths::ConvertRelativePathToFull(
      FPaths::ProjectContentDir() + "Carla/Maps/OpenDrive/OpenDriveMap.xodr");

  // 将 OpenDrive 作为文件复制到服务器端
  FFileHelper::SaveStringToFile(
      OpenDriveString,
      *AbsoluteXODRPath,
      FFileHelper::EEncodingOptions::ForceUTF8,
      &IFileManager::Get());

  if (!FPaths::FileExists(AbsoluteXODRPath))
  {
    UE_LOG(LogCarla, Error, TEXT("ERROR: XODR not copied!"));
    return false;
  }

  UCarlaGameInstance * GameInstance = UCarlaStatics::GetGameInstance(GetWorld());
  if(GameInstance)
  {
    GameInstance->SetOpendriveGenerationParameters(Params);
  }
  else
  {
    carla::log_warning("Missing game instance");
  }

  const FString AbsoluteRecastBuilderPath = BuildRecastBuilderFile();

  if (FPaths::FileExists(AbsoluteRecastBuilderPath) &&
      Params.enable_pedestrian_navigation)
  {
    /// @todo 这可能需要很长时间才能完成，客户端需要一个方法
   /// 了解导航是否可用。
    FPlatformProcess::CreateProc(
        *AbsoluteRecastBuilderPath, *AbsoluteOBJPath,
        true, true, true, nullptr, 0, nullptr, nullptr);
  }
  else
  {
    UE_LOG(LogCarla, Warning, TEXT("'RecastBuilder' not present under '%s', "
        "the binaries for pedestrian navigation will not be created."),
        *AbsoluteRecastBuilderPath);
  }

  return true;
}

void UCarlaEpisode::ApplySettings(const FEpisodeSettings &Settings)
{
  EpisodeSettings = Settings;
  if(EpisodeSettings.ActorActiveDistance > EpisodeSettings.TileStreamingDistance)
  {
    UE_LOG(LogCarla, Warning, TEXT("Setting ActorActiveDistance is smaller that TileStreamingDistance, TileStreamingDistance will be increased"));
    EpisodeSettings.TileStreamingDistance = EpisodeSettings.ActorActiveDistance;
  }
  FCarlaStaticDelegates::OnEpisodeSettingsChange.Broadcast(EpisodeSettings);
}

TArray<FTransform> UCarlaEpisode::GetRecommendedSpawnPoints() const
{
  ACarlaGameModeBase *GM = UCarlaStatics::GetGameMode(GetWorld());

  return GM->GetSpawnPointsTransforms();
}

carla::rpc::Actor UCarlaEpisode::SerializeActor(FCarlaActor *CarlaActor) const
{
  carla::rpc::Actor Actor;
  if (CarlaActor)
  {
    Actor = CarlaActor->GetActorInfo()->SerializedData;
    auto ParentId = CarlaActor->GetParent();
    if (ParentId)
    {
      Actor.parent_id = ParentId;
    }
  }
  else
  {
    UE_LOG(LogCarla, Warning, TEXT("Trying to serialize invalid actor"));
  }
  return Actor;
}

carla::rpc::Actor UCarlaEpisode::SerializeActor(AActor* Actor) const
{
  FCarlaActor* CarlaActor = FindCarlaActor(Actor);
  if (CarlaActor)
  {
    return SerializeActor(CarlaActor);
  }
  else
  {
    carla::rpc::Actor SerializedActor;
    SerializedActor.id = 0u;
    SerializedActor.bounding_box = UBoundingBoxCalculator::GetActorBoundingBox(Actor);
    TSet<crp::CityObjectLabel> SemanticTags;
    ATagger::GetTagsOfTaggedActor(*Actor, SemanticTags);
    FActorDescription Description;
    Description.Id = TEXT("static.") + CarlaGetRelevantTagAsString(SemanticTags);
    SerializedActor.description = Description;
    SerializedActor.semantic_tags.reserve(SemanticTags.Num());
    for (auto &&Tag : SemanticTags)
    {
      using tag_t = decltype(SerializedActor.semantic_tags)::value_type;
      SerializedActor.semantic_tags.emplace_back(static_cast<tag_t>(Tag));
    }
    return SerializedActor;
  }
}

void UCarlaEpisode::AttachActors(
    AActor *Child,
    AActor *Parent,
    EAttachmentType InAttachmentType,
    const FString& SocketName)
{
  Child->AddActorWorldOffset(FVector(CurrentMapOrigin));

  UActorAttacher::AttachActors(Child, Parent, InAttachmentType, SocketName);

  if (bIsPrimaryServer)
  {
    GetFrameData().AddEvent(
        CarlaRecorderEventParent{
          FindCarlaActor(Child)->GetActorId(),
          FindCarlaActor(Parent)->GetActorId()});
  }
  // 记录器事件
  if (Recorder->IsEnabled())
  {
    CarlaRecorderEventParent RecEvent
    {
      FindCarlaActor(Child)->GetActorId(),
      FindCarlaActor(Parent)->GetActorId()
    };
    Recorder->AddEvent(std::move(RecEvent));
  }
}

void UCarlaEpisode::InitializeAtBeginPlay()
{
  auto World = GetWorld();
  check(World != nullptr);
  auto PlayerController = UGameplayStatics::GetPlayerController(World, 0);
  if (PlayerController == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("Can't find player controller!"));
    return;
  }
  Spectator = PlayerController->GetPawn();
  if (Spectator != nullptr)
  {
    FActorDescription Description;
    Description.Id = TEXT("spectator");
    Description.Class = Spectator->GetClass();
    ActorDispatcher->RegisterActor(*Spectator, Description);
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("Can't find spectator!"));
  }

  // 材质参数集合
  UMaterialParameterCollection *Collection = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/Carla/Blueprints/Game/CarlaParameters.CarlaParameters"), nullptr, LOAD_None, nullptr);
	if (Collection != nullptr)
  {
    MaterialParameters = World->GetParameterCollectionInstance(Collection);
    if (MaterialParameters == nullptr)
    {
      UE_LOG(LogCarla, Error, TEXT("Can't find CarlaParameters instance!"));
    }
  }
  else
	{
    UE_LOG(LogCarla, Error, TEXT("Can't find CarlaParameters asset!"));
	}

  for (TActorIterator<ATrafficSignBase> It(World); It; ++It)
  {
    ATrafficSignBase *Actor = *It;
    check(Actor != nullptr);
    FActorDescription Description;
    Description.Id = UCarlaEpisode_GetTrafficSignId(Actor->GetTrafficSignState());
    Description.Class = Actor->GetClass();
    ActorDispatcher->RegisterActor(*Actor, Description);
  }

  // 获取 static.prop.mesh 的定义 ID
  auto Definitions = GetActorDefinitions();
  uint32 StaticMeshUId = 0;
  for (auto& Definition : Definitions)
  {
    if (Definition.Id == "static.prop.mesh")
    {
      StaticMeshUId = Definition.UId;
      break;
    }
  }

  for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
  {
    auto Actor = *It;
    check(Actor != nullptr);
    auto MeshComponent = Actor->GetStaticMeshComponent();
    check(MeshComponent != nullptr);
    if (MeshComponent->Mobility == EComponentMobility::Movable)
    {
      FActorDescription Description;
      Description.Id = TEXT("static.prop.mesh");
      Description.UId = StaticMeshUId;
      Description.Class = Actor->GetClass();
      Description.Variations.Add("mesh_path",
          FActorAttribute{"mesh_path", EActorAttributeType::String,
          MeshComponent->GetStaticMesh()->GetPathName()});
      Description.Variations.Add("mass",
          FActorAttribute{"mass", EActorAttributeType::Float,
          FString::SanitizeFloat(MeshComponent->GetMass())});
      ActorDispatcher->RegisterActor(*Actor, Description);
    }
  }
}

void UCarlaEpisode::EndPlay(void)
{
  // 停止录制器和播放器
  if (Recorder)
  {
    Recorder->Stop();
    if (Recorder->GetReplayer()->IsEnabled())
    {
      Recorder->GetReplayer()->Stop();
    }
  }
}

std::string UCarlaEpisode::StartRecorder(std::string Name, bool AdditionalData)
{
  std::string result;

  if (Recorder)
  {
    result = Recorder->Start(Name, MapName, AdditionalData);
  }
  else
  {
    result = "Recorder is not ready";
  }

  return result;
}

TPair<EActorSpawnResultStatus, FCarlaActor*> UCarlaEpisode::SpawnActorWithInfo(
    const FTransform &Transform,
    FActorDescription thisActorDescription,
    FCarlaActor::IdType DesiredId)
{
  ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(GetWorld());
  FTransform LocalTransform = Transform;
  if(LargeMap)
  {
    LocalTransform = LargeMap->GlobalToLocalTransform(LocalTransform);
  }

  // NewTransform.AddToTranslation(-1.0f * FVector(CurrentMapOrigin));
  auto result = ActorDispatcher->SpawnActor(LocalTransform, thisActorDescription, DesiredId);
  if (result.Key == EActorSpawnResultStatus::Success && bIsPrimaryServer)
  {
    if (Recorder->IsEnabled())
    {
      Recorder->CreateRecorderEventAdd(
        result.Value->GetActorId(),
        static_cast<uint8_t>(result.Value->GetActorType()),
        Transform,
        thisActorDescription
      );
    }
    if (bIsPrimaryServer)
    {
      GetFrameData().CreateRecorderEventAdd(
          result.Value->GetActorId(),
          static_cast<uint8_t>(result.Value->GetActorType()),
          Transform,
          std::move(thisActorDescription));
    }
  }

  return result;
}
