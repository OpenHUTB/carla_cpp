// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "SpeedLimitComponent.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Game/CarlaStatics.h"
#include "Carla/Vehicle/WheeledVehicleAIController.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Transform.h"
#include "carla/road/element/Waypoint.h"
#include "carla/road/element/RoadInfoSignal.h"
#include <compiler/enable-ue4-macros.h>

void USpeedLimitComponent::SetSpeedLimit(float Limit)
{
  SpeedLimit = Limit;
}

void USpeedLimitComponent::InitializeSign(const carla::road::Map &Map)
{
  const double epsilon = 0.00001;

  auto References = GetAllReferencesToThisSignal(Map);
  auto* Signal = GetSignal(Map);
  if (Signal)
  {
    SetSpeedLimit(Signal->GetValue());
  }

  for (auto& Reference : References)
  {
    auto RoadId = Reference.first;
    const auto* SignalReference = Reference.second;
    for(auto &val : SignalReference->GetValidities())
    {
      for(auto lane : carla::geom::Math::GenerateRange(val._from_lane, val._to_lane))
      {
        if(lane == 0)
          continue;

        auto signal_waypoint = Map.GetWaypoint(
            RoadId, lane, SignalReference->GetS()).get();

        if(Map.GetLane(signal_waypoint).GetType() != cr::Lane::LaneType::Driving)
          continue;

        // 获得车道宽度一半的 90%
        float BoxSize = static_cast<float>(
            0.7f*Map.GetLaneWidth(signal_waypoint)*0.5);
        // 防止出现道路宽度为 0 的情况，这种情况可能发生在刚刚出现的车道上
        BoxSize = std::max(0.01f, BoxSize);
        // Get min and max
        double LaneLength = Map.GetLane(signal_waypoint).GetLength();
        double LaneDistance = Map.GetLane(signal_waypoint).GetDistance();
        if(lane < 0)
        {
          signal_waypoint.s = FMath::Clamp(signal_waypoint.s - BoxSize,
              LaneDistance + epsilon, LaneDistance + LaneLength - epsilon);
        }
        else
        {
          signal_waypoint.s = FMath::Clamp(signal_waypoint.s + BoxSize,
              LaneDistance + epsilon, LaneDistance + LaneLength - epsilon);
        }
        float UnrealBoxSize = 100*BoxSize;
        FTransform BoxTransform = Map.ComputeTransform(signal_waypoint);
        ALargeMapManager* LargeMapManager = UCarlaStatics::GetLargeMapManager(GetWorld());
        if (LargeMapManager)
        {
          BoxTransform = LargeMapManager->GlobalToLocalTransform(BoxTransform);
        }
        GenerateSpeedBox(BoxTransform, UnrealBoxSize);
      }
    }
  }

}

void USpeedLimitComponent::GenerateSpeedBox(const FTransform BoxTransform, float BoxSize)
{
  UBoxComponent* BoxComponent = GenerateTriggerBox(BoxTransform, BoxSize);
  BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &USpeedLimitComponent::OnOverlapBeginSpeedLimitBox);
  AddEffectTriggerVolume(BoxComponent);
}

void USpeedLimitComponent::OnOverlapBeginSpeedLimitBox(UPrimitiveComponent *OverlappedComp,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult &SweepResult)
{
  ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle)
  {
    auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (Controller)
    {
      Controller->SetSpeedLimit(SpeedLimit);
    }
  }
}
