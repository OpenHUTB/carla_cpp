// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/ObstacleDetectionSensor.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/CarlaGameModeBase.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/ros2/ROS2.h"
#include <compiler/enable-ue4-macros.h>

AObstacleDetectionSensor::AObstacleDetectionSensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
}

FActorDefinition AObstacleDetectionSensor::GetSensorDefinition()
{
  FActorDefinition SensorDefinition = FActorDefinition();
  UActorBlueprintFunctionLibrary::MakeObstacleDetectorDefinitions(TEXT("other"), TEXT("obstacle"), SensorDefinition);
  return SensorDefinition;
}

void AObstacleDetectionSensor::Set(const FActorDescription &Description)
{
  //为了从米转换为厘米，我们需要将数值乘以100。
  Super::Set(Description);
  Distance = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "distance",
      Description.Variations,
      Distance) * 100.0f;
  HitRadius = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "hit_radius",
      Description.Variations,
      HitRadius) * 100.0f;
  bOnlyDynamics = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool(
      "only_dynamics",
      Description.Variations,
      bOnlyDynamics);
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
  bDebugLineTrace = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool(
      "debug_linetrace",
      Description.Variations,
      bDebugLineTrace);
#endif
}

void AObstacleDetectionSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AObstacleDetectionSensor::PostPhysTick);
  const FVector &Start = GetActorLocation();
  const FVector &End = Start + (GetActorForwardVector() * Distance);
  UWorld* CurrentWorld = GetWorld();

  // 用于保存扫描结果的结构体
  FHitResult HitOut = FHitResult();

  // 查询参数的初始化
  FCollisionQueryParams TraceParams(FName(TEXT("ObstacleDetection Trace")), true, this);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
  // 如果启用了调试模式，我们将创建一个标签来显示扫描结果
  if (bDebugLineTrace)
  {
    const FName TraceTag("ObstacleDebugTrace");
    CurrentWorld->DebugDrawTraceTag = TraceTag;
    TraceParams.TraceTag = TraceTag;
  }
#endif

  // 与复杂网格的碰撞
  TraceParams.bTraceComplex = true;

  // 忽略触发框
  TraceParams.bIgnoreTouches = true;

  // 限制返回的信息
  TraceParams.bReturnPhysicalMaterial = false;

  // 忽略自身
  TraceParams.AddIgnoredActor(this);
  if(Super::GetOwner()!=nullptr)
    TraceParams.AddIgnoredActor(Super::GetOwner());

  bool isHitReturned;
  // 选择一种扫描类型是一种权宜之计，直到所有事情都得到妥善处理
  // 根据正确的碰撞通道和对象类型进行组织
  if (bOnlyDynamics)
  {
    // 如果我们只考虑动态物体，我们会检查对象类型“AllDynamicObjects”
    FCollisionObjectQueryParams TraceChannel = FCollisionObjectQueryParams(
        FCollisionObjectQueryParams::AllDynamicObjects);
    isHitReturned = CurrentWorld->SweepSingleByObjectType(
        HitOut,
        Start,
        End,
        FQuat(),
        TraceChannel,
        FCollisionShape::MakeSphere(HitRadius),
        TraceParams);
  }
  else
  {
    //否则，如果我们考虑所有物体，我们会获取与Pawn交互的所有物体
    ECollisionChannel TraceChannel = ECC_WorldStatic;
    isHitReturned = CurrentWorld->SweepSingleByChannel(
        HitOut,
        Start,
        End,
        FQuat(),
        TraceChannel,
        FCollisionShape::MakeSphere(HitRadius),
        TraceParams);
  }

  if (isHitReturned)
  {
    OnObstacleDetectionEvent(this, HitOut.Actor.Get(), HitOut.Distance, HitOut);
  }
}

void AObstacleDetectionSensor::OnObstacleDetectionEvent(
    AActor *Actor,
    AActor *OtherActor,
    float HitDistance,
    const FHitResult &Hit)
{
  if ((Actor != nullptr) && (OtherActor != nullptr) && IsStreamReady())
  {
    const auto &Episode = GetEpisode();

    auto DataStream = GetDataStream(*this);

    // ROS2
    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("ROS2 Send");
      auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();
      AActor* ParentActor = GetAttachParentActor();
      if (ParentActor)
      {
        FTransform LocalTransformRelativeToParent = GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
        ROS2->ProcessDataFromObstacleDetection(DataStream.GetSensorType(), StreamId, LocalTransformRelativeToParent, Actor, OtherActor, HitDistance/100.0f, this);
      }
      else
      {
        ROS2->ProcessDataFromObstacleDetection(DataStream.GetSensorType(), StreamId, DataStream.GetSensorTransform(), Actor, OtherActor, HitDistance/100.0f, this);
      }
    }
    #endif

    DataStream.SerializeAndSend(*this,
        Episode.SerializeActor(Actor),
        Episode.SerializeActor(OtherActor),
        HitDistance/100.0f);
  }
}
