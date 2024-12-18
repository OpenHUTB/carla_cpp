// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


#include "SignComponent.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/opendrive/OpenDriveParser.h"
#include "carla/road/element/RoadInfoSignal.h"
#include "carla/rpc/String.h"
#include <compiler/enable-ue4-macros.h>

USignComponent::USignComponent()
{
  PrimaryComponentTick.bCanEverTick = false;
}

// 游戏开始时调用
void USignComponent::BeginPlay()
{
  Super::BeginPlay();

}

// 每帧调用
void USignComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

const FString& USignComponent::GetSignId() const
{
  return SignId;
}

void USignComponent::SetSignId(const FString &Id) {
  SignId = Id;
}

void USignComponent::InitializeSign(const cr::Map &Map)
{

}

TArray<std::pair<cr::RoadId, const cre::RoadInfoSignal*>>
    USignComponent::GetAllReferencesToThisSignal(const cr::Map &Map)
{
  TArray<std::pair<cr::RoadId, const cre::RoadInfoSignal*>> Result;
  auto waypoints = Map.GenerateWaypointsOnRoadEntries();
  std::unordered_set<cr::RoadId> ExploredRoads;
  for (auto & waypoint : waypoints)
  {
    // 检查我们是否已经探索过这条路
    if (ExploredRoads.count(waypoint.road_id) > 0)
    {
      continue;
    }
    ExploredRoads.insert(waypoint.road_id);

    // 同一条路探索多次（影响性能，不影响行为）
    auto SignalReferences = Map.GetLane(waypoint).
        GetRoad()->GetInfos<cre::RoadInfoSignal>();
    for (auto *SignalReference : SignalReferences)
    {
      FString SignalId(SignalReference->GetSignalId().c_str());
      if(SignalId == GetSignId())
      {
        Result.Add({waypoint.road_id, SignalReference});
      }
    }
  }
  return Result;
}

const cr::Signal* USignComponent::GetSignal(const cr::Map &Map) const
{
  std::string std_signal_id = carla::rpc::FromFString(GetSignId());
  if (Map.GetSignals().count(std_signal_id))
  {
    return Map.GetSignals().at(std_signal_id).get();
  }
  return nullptr;
}

UBoxComponent* USignComponent::GenerateTriggerBox(const FTransform &BoxTransform,
    float BoxSize)
{
  AActor *ParentActor = GetOwner();
  UBoxComponent *BoxComponent = NewObject<UBoxComponent>(ParentActor);
  BoxComponent->RegisterComponent();
  BoxComponent->AttachToComponent(
      ParentActor->GetRootComponent(),
      FAttachmentTransformRules::KeepRelativeTransform);
  BoxComponent->SetWorldTransform(BoxTransform);
  BoxComponent->SetBoxExtent(FVector(BoxSize, BoxSize, BoxSize), true);
  return BoxComponent;
}

UBoxComponent* USignComponent::GenerateTriggerBox(const FTransform &BoxTransform,
    const FVector &BoxSize)
{
  AActor *ParentActor = GetOwner();
  UBoxComponent *BoxComponent = NewObject<UBoxComponent>(ParentActor);
  BoxComponent->RegisterComponent();
  BoxComponent->AttachToComponent(
      ParentActor->GetRootComponent(),
      FAttachmentTransformRules::KeepRelativeTransform);
  BoxComponent->SetWorldTransform(BoxTransform);
  BoxComponent->SetBoxExtent(BoxSize, true);
  return BoxComponent;
}

void USignComponent::AddEffectTriggerVolume(UBoxComponent* TriggerVolume)
{
  EffectTriggerVolumes.Add(TriggerVolume);
}

const TArray<UBoxComponent*> USignComponent::GetEffectTriggerVolume() const
{
  return EffectTriggerVolumes;
}
