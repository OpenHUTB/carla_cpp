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

/// A simulation episode.
///
/// Each time the level is restarted a new episode is created.
/// @brief `UCarlaEpisode` 类表示一个模拟情节，在Carla模拟中，每次重新加载关卡（地图）时会创建一个新的情节实例，用于管理该次模拟过程中的各种元素和操作。
UCLASS(BlueprintType, Blueprintable)
class CARLA_API UCarlaEpisode : public UObject
{
  GENERATED_BODY()

  // ===========================================================================
  // --构造函数相关部分 ------------------------------------------------------------
  // ===========================================================================

public:
/// @brief 构造函数，接受一个 `FObjectInitializer` 类型的参数，用于初始化对象的相关属性，按照UE4的对象初始化机制进行操作。
  UCarlaEpisode(const FObjectInitializer &ObjectInitializer);

  // ===========================================================================
  // -- 加载新情节相关部分-----------------------------------------------------
  // ===========================================================================

  /// Load a new map and start a new episode.
  ///
  /// If @a MapString is empty, the current map is reloaded.
  /// @brief 加载新的地图并启动一个新的模拟情节。
  /// @param MapString 表示要加载的地图名称的字符串，如果为空字符串，则重新加载当前地图。
  /// @param ResetSettings 一个布尔值，默认为 `true`，表示是否重置情节设置，若为 `true`，则使用默认或预设的情节设置来启动新情节。
  /// @return 如果地图加载成功并成功启动新情节，则返回 `true`；否则返回 `false`。
  UFUNCTION(BlueprintCallable)
  bool LoadNewEpisode(const FString &MapString, bool ResetSettings = true);
    /// @brief 加载新的地图，该地图根据OpenDRIVE数据生成网格，并启动一个新的模拟情节。
    /// @param OpenDriveString 包含OpenDRIVE数据的字符串，用于生成地图的相关信息。
    /// @param Params OpenDRIVE生成参数对象，用于指定地图生成过程中的详细参数设置。
    /// @return 如果地图成功根据OpenDRIVE数据生成并启动新情节，则返回 `true`；否则返回 `false`，注意如果 `OpenDriveString` 为空字符串则会导致加载失败。
  /// Load a new map generating the mesh from OpenDRIVE data and
  /// start a new episode.
  ///
  /// If @a MapString is empty, it fails.
  bool LoadNewOpendriveEpisode(
      const FString &OpenDriveString,
      const carla::rpc::OpendriveGenerationParameters &Params);

  // ===========================================================================
  // -- 情节设置相关部分 -------------------------------------------------------
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
  // -- 获取本情节相关信息部分 ---------------------------------------
  // ===========================================================================

  /// Return the unique id of this episode.
/// @brief 返回当前模拟情节的唯一标识符，用于区分不同的模拟情节实例，方便在多个情节存在的场景下进行识别和管理。
  auto GetId() const
  {
    return Id;
  }
/// @brief 返回当前模拟情节中加载的地图名称，以 `FString` 类型返回，方便在UI显示、日志记录等场景中使用地图名称信息。
  /// Return the name of the map loaded in this episode.
  UFUNCTION(BlueprintCallable)
  const FString &GetMapName() const
  {
    return MapName;
  }
/// @brief 记录模拟过程的时间流逝情况，例如可以根据时间来触发某些事件等。
  /// Game seconds since the start of this episode.
  double GetElapsedGameTime() const
  {
    return ElapsedGameTime;
  }
 /// @brief 获取视觉游戏时间行
  double GetVisualGameTime() const
  {
    return VisualGameTime;
  }
/// @brief 设置视觉游戏时间，同时会更新材质参数中的对应时间参数确保相关视觉效果能根据新设置的时间进行更新。
    /// @param Time 要设置的视觉游戏时间值（以秒为单位）。
  void SetVisualGameTime(double Time)
  {
    VisualGameTime = Time;
// 如果材质参数实例存在，更新材质参数集合实例中名为 "VisualTime" 的标量参数值为新的视觉游戏时间，
        // 这样可能会影响使用该材质的视觉效果，使其与设置的时间相关联
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
/// @brief 返回推荐的车辆生成点列表，方便快速找到合适的位置来创建车辆Actor
  UFUNCTION(BlueprintCallable)
  TArray<FTransform> GetRecommendedSpawnPoints() const;
/// @brief 返回当前加载地图的地理位置参考信息
  /// Return the GeoLocation point of the map loaded
  const carla::geom::GeoLocation &GetGeoReference() const
  {
    return MapGeoReference;
  }

  // ===========================================================================
  // -- 获取特殊Actor相关部分------------------------------------------------
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
  // -- Actor查找方法相关部分  --------------------------------------------------
  // ===========================================================================

  /// invalid./// @brief 根据给定的Actor标识符（`ActorId`）查找对应的Carla Actor
    /// @param ActorId 要查找的Actor的唯一标识符，类型为 `FCarlaActor::IdType`。
    /// @return 如果找到对应的Actor且该Actor未处于待销毁状态，则返回指向该Actor的有效指针（`FCarlaActor*` 类型）
//否则返回无效指针（表示未找到或Actor即将被销毁）。
  FCarlaActor* FindCarlaActor(FCarlaActor::IdType ActorId)
  {
    return ActorDispatcher->GetActorRegistry().FindCarlaActor(ActorId);
  }

   /// @brief 根据给定的 `AActor` 指针查找对应的Carla Actor
    /// @param Actor 指向要查找的 `AActor` 对象的指针。
    /// @return 如果找到对应的Actor且该Actor未处于待销毁状态，则返回指向该Actor的有效指针（`FCarlaActor*` 类型）；
  //否则返回无效指针（表示未找到或Actor即将被销毁）。
    FCarlaActor* FindCarlaActor(AActor *Actor) constspatcher->GetActorRegistry().FindCarlaActor(Actor);
  }

 /// @brief 根据给定的流标识符（`StreamId`）获取对应的Carla Actor（传感器）的描述信息，如果未找到该Actor则返回空字符串。
    /// @param StreamId 用于标识特定数据流的标识符通过该标识符查找对应的Actor描述信息。
    /// @return 如果找到对应的Actor，则返回其描述信息（以 `FString` 类型表示）；否则返回空字符串，表示未找到相关Actor。
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

  void InitializeAtBeginPlay//进行初始化相关的操作

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
  bool SetActorDead(FCarlaActor &CarlaActor);

  void TickTimers(float DeltaSeconds)
  {
    ElapsedGameTime += DeltaSeconds;
    SetVisualGameTime(VisualGameTime + DeltaSeconds);
    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
      ROS2->SetTimestamp(GetElapsedGameTime());
    #endif

  }

  const uint64 Id = 0u;

  // simulation time
  double ElapsedGameTime = 0.0;

  // visual time (used by clounds and other FX that need to be deterministic)
  double VisualGameTime = 0.0;

  UPROPERTY(VisibleAnywhere)
  FString MapName;

  UPROPERTY(VisibleAnywhere)
  FEpisodeSettings EpisodeSettings;

  UPROPERTY(VisibleAnywhere)
  UActorDispatcher *ActorDispatcher = nullptr;

  UPROPERTY(VisibleAnywhere)
  APawn *Spectator = nullptr;

  UPROPERTY(VisibleAnywhere)
  AWeather *Weather = nullptr;

  UPROPERTY(VisibleAnywhere)
  UMaterialParameterCollectionInstance *MaterialParameters = nullptr;

  ACarlaRecorder *Recorder = nullptr;

  carla::geom::GeoLocation MapGeoReference;

  FIntVector CurrentMapOrigin;

  FFrameData FrameData;

  FSensorManager SensorManager;
};

FString CarlaGetRelevantTagAsString(const TSet<crp::CityObjectLabel> &SemanticTags);
