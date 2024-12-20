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

  UCarlaEpisode(const FObjectInitializer &ObjectInitializer);

  // ===========================================================================
  // -- 加载新剧集 -----------------------------------------------------
  // ===========================================================================

  /// 加载新地图并开始新剧集。
  ///
  /// If @a MapString is empty, the current map is reloaded.
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

  UFUNCTION(BlueprintCallable)
  const FEpisodeSettings &GetSettings() const
  {
    return EpisodeSettings;
  }

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

  void SetVisualGameTime(double Time)
  {
    VisualGameTime = Time;

    // Material Parameters 中的更新时间
    if (MaterialParameters)
    {
      MaterialParameters->SetScalarParameterValue(FName("VisualTime"), VisualGameTime);
    }
  }

  /// Return the list of actor definitions that are available to be spawned this
  /// episode.
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

  UFUNCTION(BlueprintCallable)
  APawn *GetSpectatorPawn() const
  {
    return Spectator;
  }

  UFUNCTION(BlueprintCallable)
  AWeather *GetWeather() const
  {
    return Weather;
  }

  const FActorRegistry &GetActorRegistry() const
  {
    return ActorDispatcher->GetActorRegistry();
  }

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
  // -- Actor handling methods -------------------------------------------------
  // ===========================================================================

  /// Spawns an actor based on @a ActorDescription at @a Transform. To properly
  /// despawn an actor created with this function call DestroyActor.
  ///
  /// @return A pair containing the result of the spawn function and a view over
  /// the actor and its properties. If the status is different of Success the
  /// view is invalid.
  TPair<EActorSpawnResultStatus, FCarlaActor*> SpawnActorWithInfo(
      const FTransform &Transform,
      FActorDescription thisActorDescription,
      FCarlaActor::IdType DesiredId = 0);

  /// Spawns an actor based on @a ActorDescription at @a Transform.
  ///
  /// @return the actor to be spawned
  AActor* ReSpawnActorWithInfo(
      const FTransform &Transform,
      FActorDescription thisActorDescription)
  {
    FTransform NewTransform = Transform;
    auto result = ActorDispatcher->ReSpawnActor(NewTransform, thisActorDescription);
    if (Recorder->IsEnabled())
    {
      // do something?
    }

    return result;
  }

  /// Spawns an actor based on @a ActorDescription at @a Transform. To properly
  /// despawn an actor created with this function call DestroyActor.
  ///
  /// @return nullptr on failure.
  ///
  /// @note Special overload for blueprints.
  UFUNCTION(BlueprintCallable)
  AActor *SpawnActor(
      const FTransform &Transform,
      FActorDescription ActorDescription)
  {
    return SpawnActorWithInfo(Transform, std::move(ActorDescription)).Value->GetActor();
  }

  /// Attach @a Child to @a Parent.
  ///
  /// @pre Actors cannot be null.
  UFUNCTION(BlueprintCallable)
  void AttachActors(
      AActor *Child,
      AActor *Parent,
      EAttachmentType InAttachmentType = EAttachmentType::Rigid,
      const FString& SocketName = "");

  /// @copydoc FActorDispatcher::DestroyActor(AActor*)
  UFUNCTION(BlueprintCallable)
  bool DestroyActor(AActor *Actor)
  {
    FCarlaActor* CarlaActor = FindCarlaActor(Actor);
    if (CarlaActor)
    {
      carla::rpc::ActorId ActorId = CarlaActor->GetActorId();
      return DestroyActor(ActorId);
    }
    return false;
  }

  bool DestroyActor(carla::rpc::ActorId ActorId)
  {
    if (bIsPrimaryServer)
    {
      GetFrameData().AddEvent(
          CarlaRecorderEventDel{ActorId});
    }
    if (Recorder->IsEnabled())
    {
      // recorder event
      CarlaRecorderEventDel RecEvent{ActorId};
      Recorder->AddEvent(std::move(RecEvent));
    }

    return ActorDispatcher->DestroyActor(ActorId);
  }

  void PutActorToSleep(carla::rpc::ActorId ActorId)
  {
    ActorDispatcher->PutActorToSleep(ActorId, this);
  }

  void WakeActorUp(carla::rpc::ActorId ActorId)
  {
    ActorDispatcher->WakeActorUp(ActorId, this);
  }

  // ===========================================================================
  // -- Other methods ----------------------------------------------------------
  // ===========================================================================

  /// Create a serializable object describing the actor.
  carla::rpc::Actor SerializeActor(FCarlaActor* CarlaActor) const;

  /// Create a serializable object describing the actor.
  /// Can be used to serialized actors that are not in the registry
  carla::rpc::Actor SerializeActor(AActor* Actor) const;

  // ===========================================================================
  // -- Private methods and members --------------------------------------------
  // ===========================================================================

  ACarlaRecorder *GetRecorder() const
  {
    return Recorder;
  }

  void SetRecorder(ACarlaRecorder *Rec)
  {
    Recorder = Rec;
  }

  CarlaReplayer *GetReplayer() const
  {
    return Recorder->GetReplayer();
  }

  std::string StartRecorder(std::string name, bool AdditionalData);

  FIntVector GetCurrentMapOrigin() const { return CurrentMapOrigin; }

  void SetCurrentMapOrigin(const FIntVector& NewOrigin) { CurrentMapOrigin = NewOrigin; }

  FFrameData& GetFrameData() { return FrameData; }

  FSensorManager& GetSensorManager() { return SensorManager; }

  bool bIsPrimaryServer = true;

private:

  friend class ACarlaGameModeBase;
  friend class FCarlaEngine;

  void InitializeAtBeginPlay();

  void EndPlay();

  void RegisterActorFactory(ACarlaActorFactory &ActorFactory)
  {
    ActorDispatcher->Bind(ActorFactory);
  }

  std::pair<int, FCarlaActor&> TryToCreateReplayerActor(
    FVector &Location,
    FVector &Rotation,
    FActorDescription &ActorDesc,
    unsigned int desiredId);

  bool SetActorSimulatePhysics(FCarlaActor &CarlaActor, bool bEnabled);

  bool SetActorCollisions(FCarlaActor &CarlaActor, bool bEnabled);

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
