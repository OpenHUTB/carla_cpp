// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaStatics.h"
#include "TrafficLightGroup.h"


// 设置默认值
ATrafficLightGroup::ATrafficLightGroup()
{
  // 将此参与者设置为每帧调用 Tick()。如果不需要，可以关闭此功能以提高性能。
  PrimaryActorTick.bCanEverTick = true;
  SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
  RootComponent = SceneComponent;
}

void ATrafficLightGroup::SetFrozenGroup(bool InFreeze)
{
  bIsFrozen = InFreeze;
}

bool ATrafficLightGroup::IsFrozen() const
{
  return bIsFrozen;
}

void ATrafficLightGroup::ResetGroup()
{
  for(auto * Controller : Controllers)
  {
    Controller->ResetState();
  }
  CurrentController = 0;
  UTrafficLightController* controller = Controllers[CurrentController];
  controller->StartCycle();
}

// 每帧调用
void ATrafficLightGroup::Tick(float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ATrafficLightGroup::Tick);
  Super::Tick(DeltaTime);

  // 如果重播器正在重播，请勿更新
  auto* Episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
  if (Episode)
  {
    auto* Replayer = Episode->GetReplayer();
    if (Replayer)
    {
      if(Replayer->IsEnabled())
      {
        return;
      }
    }
  }

  if (bIsFrozen)
  {
    return;
  }

  UTrafficLightController* controller = Controllers[CurrentController];
  if (controller->AdvanceTimeAndCycleFinished(DeltaTime))
  {
    NextController();
  }
}

void ATrafficLightGroup::NextController()
{
  CurrentController = (CurrentController + 1) % Controllers.Num();
  UTrafficLightController* controller = Controllers[CurrentController];
  controller->StartCycle();
}

int ATrafficLightGroup::GetJunctionId() const
{
  return JunctionId;
}

void ATrafficLightGroup::AddController(UTrafficLightController* Controller)
{
  Controllers.Add(Controller);
  Controller->SetGroup(this);
}
