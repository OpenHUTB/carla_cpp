// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDispatcher.h"
#include "Carla/Recorder/CarlaRecorder.h"
#include "Carla/Sensor/WorldObserver.h"
#include "Carla/Server/CarlaServer.h"
#include "Carla/Settings/EpisodeSettings.h"
#include "Carla/Util/ActorAttacher.h"
#include "Carla/Weather/Weather.h"
#include "Carla/Game/FrameData.h"
#include "Carla/Sensor/SensorManager.h"

#include "GameFramework/Pawn.h"
#include "Materials/MaterialParameterCollectionInstance.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/geom/BoundingBox.h>
#include <carla/geom/GeoLocation.h>
#include <carla/ros2/ROS2.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/ActorDescription.h>
#include <carla/rpc/OpendriveGenerationParameters.h>
#include <carla/streaming/Server.h>
#include <compiler/enable-ue4-macros.h>

#include "CarlaEpisode.generated.h"

/// 模拟剧集。
///
/// 每次重新启动关卡时，都会创建一个新的剧集。
UCLASS(BlueprintType, Blueprintable)
class CARLA_API UCarlaEpisode : public UObject
{
  GENERATED_BODY()

  // ===========================================================================
  // -- 构造 函数 ------------------------------------------------------------
  // ===========================================================================

public:
/// @brief 构造函数，接受一个 `FObjectInitializer` 类型的参数，用于初始化对象的相关属性，按照UE4的对象初始化机制进行操作。
  UCarlaEpisode(const FObjectInitializer &ObjectInitializer);

  // ===========================================================================
  // -- 加载新剧集 -----------------------------------------------------
  // ===========================================================================

  /// 加载新地图并开始新剧集。
  ///
  /// If @a MapString is empty, the current map is reloaded.
  /// @brief 加载新的地图并启动一个新的模拟情节。
  /// @param MapString 表示要加载的地图名称的字符串，如果为空字符串，则重新加载当前地图。
  /// @param ResetSettings 一个布尔值，默认为 `true`，表示是否重置情节设置，若为 `true`，则使用默认或预设的情节设置来启动新情节。
  /// @return 如果地图加载成功并成功启动新情节，则返回 `true`；否则返回 `false`。
  UFUNCTION(BlueprintCallable)
  bool LoadNewEpisode(const FString &MapString, bool ResetSettings = true);

  ///加载从 OpenDRIVE 数据生成网格的新地图，然后
  ///开始新剧集。
  ///
  /// If @a MapString is empty, it fails.
  bool LoadNewOpendriveEpisode(
      const FString &OpenDriveString,
      const carla::rpc::OpendriveGenerationParameters &Params);

  // ===========================================================================
  // -- 剧集设置 -------------------------------------------------------
  // ===========================================================================
/// @brief 获取当前模拟情节的设置信息，返回一个常量引用，确保不会意外修改设置内容，外部只能读取当前的设置情况。
    UFUNCTION(BlueprintCallable)
  UFUNCTION(BlueprintCallable)
  const FEpisodeSettings &GetSettings() const
  {
    return EpisodeSettings;
  }
/// @brief 应用给定的情节设置信息到当前模拟情节中，用于更新情节的各种参数配置，比如改变地图、Actor生成规则等设置。
  UFUNCTION(BlueprintCallable)
  void ApplySettings(const FEpisodeSettings &Settings);

  // ===========================================================================
  // -- 检索有关此剧集的信息---------------------------------------
  // ===========================================================================

  /// 返回此剧集的唯一 ID。
  auto GetId() const
  {
    return Id;
  }

  ///返回本集中加载的地图的名称。
  UFUNCTION(BlueprintCallable)
  const FString &GetMapName() const
  {
    return MapName;
  }

  /// 游戏秒数。
  double GetElapsedGameTime() const
  {
    return ElapsedGameTime;
  }

  /// 视觉游戏秒
  double GetVisualGameTime() const
  {
    return VisualGameTime;
  }
/// @brief 设置视觉游戏时间，同时会更新材质参数中的对应时间参数确保相关视觉效果能根据新设置的时间进行更新。
    /// @param Time 要设置的视觉游戏时间值（以秒为单位）。
  void SetVisualGameTime(double Time)
  {
    VisualGameTime = Time;
    // Material Parameters 中的更新时间
    if (MaterialParameters)
    {
      MaterialParameters->SetScalarParameterValue(FName("VisualTime"), VisualGameTime);
    }
  }
  /// @brief 返回在本情节中可生成的Actor定义列表，用于了解当前情节下能够创建哪些类型的Actor。

  UFUNCTION(BlueprintCallable)
  const TArray<FActorDefinition> &GetActorDefinitions() const
  {
    return ActorDispatcher->GetActorDefinitions();
  }

  ///返回推荐的载具重生点列表。
  UFUNCTION(BlueprintCallable)
  TArray<FTransform> GetRecommendedSpawnPoints() const;

  /// 返回加载的地图的 GeoLocation 点
  const carla::geom::GeoLocation &GetGeoReference() const
  {
    return MapGeoReference;
  }

  // ===========================================================================
  // -- 检索特殊演员 ------------------------------------------------
  // ===========================================================================
/// @brief 获取当前模拟情节中的旁观者Pawn
  UFUNCTION(BlueprintCallable)
  APawn *GetSpectatorPawn() const
  {
    return Spectator;
  }
/// @brief 获取当前模拟情节中的天气对象
  UFUNCTION(BlueprintCallable)
  AWeather *GetWeather() const
  {
    return Weather;
  }
/// @brief 获取Actor注册信息的常量引用
  const FActorRegistry &GetActorRegistry() const
  {
    return ActorDispatcher->GetActorRegistry();
  }
/// @brief 获取Actor注册信息的引用
  FActorRegistry &GetActorRegistry()
  {
    return ActorDispatcher->GetActorRegistry();
  }

  // ===========================================================================
  // -- Actor 查找方法 --------------------------------------------------
  // ===========================================================================

  ///按 id 查找 Carla 演员。
  ///
  /// 如果未找到 actor 或正在等待 kill，则返回的视图为
  ///无效。
  FCarlaActor* FindCarlaActor(FCarlaActor::IdType ActorId)
  {
    return ActorDispatcher->GetActorRegistry().FindCarlaActor(ActorId);
  }
  /// 找到 @a Actor 的 actor 视图。
  ///
  /// 如果未找到 actor 或正在等待 kill，则返回的视图为
  ///无效。
  FCarlaActor* FindCarlaActor(AActor *Actor) const
  {
    return ActorDispatcher->GetActorRegistry().FindCarlaActor(Actor);
  }

  ///使用特定流 ID 获取 Carla actor （sensor） 的描述。
  ///
  /// 如果未找到 actor，则返回空字符串
  FString GetActorDescriptionFromStream(carla::streaming::detail::stream_id_type StreamId)
  {
    return ActorDispatcher->GetActorRegistry().GetDescriptionFromStream(StreamId);
  }

  // ===========================================================================
  // -- Actor处理方法相关部分 -------------------------------------------------
  // ===========================================================================

  /// @brief 在给定的变换信息（`Transform`）位置
    /// @param thisActorDescription 描述要创建的Actor的详细信息
    /// @param DesiredId 可选的期望Actor标识符，默认为0
    /// @return 返回一个 `TPair` 类型对象
  TPair<EActorSpawnResultStatus, FCarlaActor*> SpawnActorWithInfo(
      const FTransform &Transform,
      FActorDescription thisActorDescription,
      FCarlaActor::IdType DesiredId = 0);

  /// @brief 在给定的变换信息（`Transform`）位置，根据给定的 `ActorDescription` 重新创建一个Actor
    /// @param Transform 表示Actor生成位置和姿态的变换信息
    /// @param thisActorDescription 描述要创建的Actor的详细信息
    /// @return 返回指向重新创建的Actor的指针（`AActor*` 类型），如果创建失败则返回 `nullptr`。
    AActor* ReSpawnActorWithInfo(
  AActor* ReSpawnActorWithInfo(
      const FTransform &Transform,
      FActorDescription thisActorDescription)
  {
    FTransform NewTransform = Transform;
    auto result = ActorDispatcher->ReSpawnActor(NewTransform, thisActorDescription);
    if (Recorder->IsEnabled())
    {
     // 这里可能后续需要添加一些针对录制功能的操作，比如记录Actor重新生成的事件等，但目前代码中没有具体实现，只是预留了位置。
    }

    return result;
  }

  /// @brief 在给定的变换信息（`Transform`）位置，根据给定的 `ActorDescription` 创建一个Actor，这是一个供蓝图调用的函数版本
    /// @param Transform 表示Actor生成位置和姿态的变换信息，以 `FTransform` 类型传递
    /// @param ActorDescription 描述要创建的Actor的详细信息，用于确定创建何种Actor。
    /// @return 返回指向创建的Actor的指针（`AActor*` 类型），如果创建失败则返回 `nullptr`。
    UFUNCTION(BlueprintCallable)
  UFUNCTION(BlueprintCallable)
  AActor *SpawnActor(
      const FTransform &Transform,
      FActorDescription ActorDescription)
  {
    // 调用 `SpawnActorWithInfo` 函数来实际创建Actor，传入 `Transform` 和移动后的 `ActorDescription`（使用 `std::move` 进行右值引用传递，避免不必要的拷贝开销，提高性能），
    // 然后获取返回结果中的 `Value`（这可能是 `TPair` 类型返回值中的实际Actor相关部分，根据前面 `SpawnActorWithInfo` 的定义推测），再通过 `GetActor` 函数（具体实现应该在相关类中定义）获取最终指向创建的Actor的指针并返回。
    return SpawnActorWithInfo(Transform, std::move(ActorDescription)).Value->GetActor();
  }

  // 以下这个宏声明了一个函数，使其可以在UE4的蓝图系统中被调用
  void AttachActors(
      AActor *Child,// 参数 `Child`，指向要附着的子Actor的指针
      AActor *Child,
      AActor *Parent,// 参数 `InAttachmentType`，用于指定附着的类型，默认值为 `EAttachmentType::Rigid`
      EAttachmentType InAttachmentType = EAttachmentType::Rigid, // 参数 `SocketName`，用于指定附着的具体插槽名称
      const FString& SocketName = "");

// 以下这个宏声明了一个函数，使其可以在UE4的蓝图系统中被调用
  UFUNCTION(BlueprintCallable)
  bool DestroyActor(AActor *Actor)
  {
    FCarlaActor* CarlaActor = FindCarlaActor(Actor);//根据传入的 `Actor` 指针查找对应的 `CarlaActor`
    // 如果找到了对应的 `CarlaActor`，则获取其Actor标识符
    if (CarlaActor)
    {
      carla::rpc::ActorId ActorId = CarlaActor->GetActorId();
      return DestroyActor(ActorId);
    }
     // 如果没有找到对应的 `CarlaActor`，则直接返回 `false`，表示销毁操作失败。
    return false;
  }
//执行具体的销毁逻辑并处理相关的记录。
  bool DestroyActor(carla::rpc::ActorId ActorId)
  {
    // 如果当前对象是主服务器添加一个表示Actor删除的事件
    if (bIsPrimaryServer)
    {
      GetFrameData().AddEvent(
          CarlaRecorderEventDel{ActorId});
    }
    // 如果录制器（`Recorder`）处于启用状态，则创建一个表示Actor删除的事件对象
    if (Recorder->IsEnabled())
    {
      // recorder event
      CarlaRecorderEventDel RecEvent{ActorId};
      Recorder->AddEvent(std::move(RecEvent));
    }

    return ActorDispatcher->DestroyActor(ActorId);
  }
// 函数用于将指定的Actor设置为睡眠状态
  void PutActorToSleep(carla::rpc::ActorId ActorId)
  {
    ActorDispatcher->PutActorToSleep(ActorId, this);
  }
// 函数用于唤醒指定的处于睡眠状态的Actor
  void WakeActorUp(carla::rpc::ActorId ActorId)
  {
    ActorDispatcher->WakeActorUp(ActorId, this);
  }

  // ===========================================================================
  // --Other methods（其他方法）相关部分 ----------------------------------------------------------
  // ===========================================================================

   // 函数用于创建一个可序列化的对象
  carla::rpc::Actor SerializeActor(FCarlaActor* CarlaActor) const;

  ///创建一个描述角色的可序列化对象。
///可以用来序列化注册表中没有的actor
  carla::rpc::Actor SerializeActor(AActor* Actor) const;

  // ===========================================================================
  // -- Private methods and members（私有方法和成员变量）相关部分 --------------------------------------------
  // ===========================================================================
// 函数用于获取当前的 `ACarlaRecorder` 对象返回一个常量指针外部只能通过这个函数获取该对象进行读取相关信息，不能修改其内容。
  ACarlaRecorder *GetRecorder() const
  {
    return Recorder;
  }
// 函数用于设置当前的 `ACarlaRecorder` 对象
  void SetRecorder(ACarlaRecorder *Rec)
  {
    Recorder = Rec;
  }
// 函数用于获取与当前录制器（`Recorder`）关联的回放器（`CarlaReplayer`）对象
  CarlaReplayer *GetReplayer() const
  {
    return Recorder->GetReplayer();
  }
// 启动录制器
  std::string StartRecorder(std::string name, bool AdditionalData);
//获取当前地图的原点信息
  FIntVector GetCurrentMapOrigin() const { return CurrentMapOrigin; }
//设置当前地图的原点信息
  void SetCurrentMapOrigin(const FIntVector& NewOrigin) { CurrentMapOrigin = NewOrigin; }
//获取当前的帧数据
  FFrameData& GetFrameData() { return FrameData; }
//获取当前的传感器管理器
  FSensorManager& GetSensorManager() { return SensorManager; }
//表示当前对象是否是主服务器
  bool bIsPrimaryServer = true;

private:

  friend class ACarlaGameModeBase;// 将 `ACarlaGameModeBase` 类声明为友元类
  friend class FCarlaEngine;// 将 `FCarlaEngine` 类声明为友元类

  void InitializeAtBeginPlay()//进行初始化相关的操作

  void EndPlay();//执行清理、资源释放等操作

  void RegisterActorFactory(ACarlaActorFactory &ActorFactory)//用于注册一个Actor工厂
  {
    ActorDispatcher->Bind(ActorFactory);
  }
//尝试创建一个用于回放的Actor对象
  std::pair<int, FCarlaActor&> TryToCreateReplayerActor(
    FVector &Location,
    FVector &Rotation,
    FActorDescription &ActorDesc,
    unsigned int desiredId);
//设置给定的 `CarlaActor` 的物理模拟状态
  bool SetActorSimulatePhysics(FCarlaActor &CarlaActor, bool bEnabled);
//设置给定的 `CarlaActor` 的碰撞属性
  bool SetActorCollisions(FCarlaActor &CarlaActor, bool bEnabled);
//更新游戏中的各种定时器相关信息
  // 声明一个函数，用于设置Actor的状态为死亡，具体实现未提供
bool SetActorDead(FCarlaActor &CarlaActor);

// 每个Tick调用的函数，用于更新计时器
void TickTimers(float DeltaSeconds)
{
  // 累加游戏时间
  ElapsedGameTime += DeltaSeconds;
  // 更新视觉游戏时间，用于确保云和其他特效的确定性
  SetVisualGameTime(VisualGameTime + DeltaSeconds);
  #if defined(WITH_ROS2) // 如果定义了WITH_ROS2宏，表示支持ROS2集成
  auto ROS2 = carla::ros2::ROS2::GetInstance(); // 获取ROS2实例
  if (ROS2->IsEnabled()) // 如果ROS2实例已启用
    ROS2->SetTimestamp(GetElapsedGameTime()); // 设置ROS2的时间戳
  #endif
}

// 一个常量，用于标识ID，初始值设置为0
const uint64 Id = 0u;

// 累积的游戏时间，用于跟踪仿真的总时间
double ElapsedGameTime = 0.0;

// 视觉游戏时间，用于云和其他需要确定性效果的元素
double VisualGameTime = 0.0;

// 可从任何地方访问的属性，存储当前地图的名称
UPROPERTY(VisibleAnywhere)
FString MapName;

// 可从任何地方访问的属性，存储当前的Episode设置
UPROPERTY(VisibleAnywhere)
FEpisodeSettings EpisodeSettings;

// 可从任何地方访问的属性，存储Actor调度器的指针
UPROPERTY(VisibleAnywhere)
UActorDispatcher *ActorDispatcher = nullptr;

// 可从任何地方访问的属性，存储观察者Pawn的指针
UPROPERTY(VisibleAnywhere)
APawn *Spectator = nullptr;

// 可从任何地方访问的属性，存储天气控制器的指针
UPROPERTY(VisibleAnywhere)
AWeather *Weather = nullptr;

// 可从任何地方访问的属性，存储材质参数集合实例的指针
UPROPERTY(VisibleAnywhere)
UMaterialParameterCollectionInstance *MaterialParameters = nullptr;

// 录像机的指针，用于记录和回放仿真
ACarlaRecorder *Recorder = nullptr;

// 地图的地理参考信息
carla::geom::GeoLocation MapGeoReference;

// 当前地图的原点位置
FIntVector CurrentMapOrigin;

// 存储帧数据的结构
FFrameData FrameData;

// 传感器管理器，用于管理仿真中的传感器
FSensorManager SensorManager;

// 将语义标签转换为字符串的函数，用于获取相关的标签描述
FString CarlaGetRelevantTagAsString(const TSet<crp::CityObjectLabel> &SemanticTags);
