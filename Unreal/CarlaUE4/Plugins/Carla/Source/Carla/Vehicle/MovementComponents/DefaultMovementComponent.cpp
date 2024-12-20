// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "DefaultMovementComponent.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

// 为指定的车辆创建并配置一个新的默认运动组件，并将其注册到引擎中。
void UDefaultMovementComponent::CreateDefaultMovementComponent(ACarlaWheeledVehicle* Vehicle)
{
  UDefaultMovementComponent* DefaultMovementComponent = NewObject<UDefaultMovementComponent>(Vehicle);
  Vehicle->SetCarlaMovementComponent(DefaultMovementComponent);
  DefaultMovementComponent->RegisterComponent();
}

// 在游戏开始时被调用，用于初始化和设置组件的初始状态。
// 该函数调用了父类的 BeginPlay 方法，确保父类的初始化逻辑也得到执行
void UDefaultMovementComponent::BeginPlay()
{
  Super::BeginPlay();
}

// 处理车辆控制输入并更新车辆的运动组件参数。
// 根据给定的控制信息（油门、转向、刹车等）更新车辆的运动状态。
// 还会根据反向、手动档等控制设置调整车辆的档位和自动换档行为。
void UDefaultMovementComponent::ProcessControl(FVehicleControl &Control)
{
  auto *MovementComponent = CarlaVehicle->GetVehicleMovementComponent();
  MovementComponent->SetThrottleInput(Control.Throttle);
  MovementComponent->SetSteeringInput(Control.Steer);
  MovementComponent->SetBrakeInput(Control.Brake);
  MovementComponent->SetHandbrakeInput(Control.bHandBrake);
  if (CarlaVehicle->GetVehicleControl().bReverse != Control.bReverse)
  {
    MovementComponent->SetUseAutoGears(!Control.bReverse);
    MovementComponent->SetTargetGear(Control.bReverse ? -1 : 1, true);
  }
  else
  {
    MovementComponent->SetUseAutoGears(!Control.bManualGearShift);
    if (Control.bManualGearShift)
    {
      MovementComponent->SetTargetGear(Control.Gear, true);
    }
  }
  Control.Gear = MovementComponent->GetCurrentGear();
}

// FVector GetVelocity() const override;

// 获取当前车辆的档位。
int32 UDefaultMovementComponent::GetVehicleCurrentGear() const
{
  return CarlaVehicle->GetVehicleMovementComponent()->GetCurrentGear();
}

// 获取当前车辆的前进速度。
float UDefaultMovementComponent::GetVehicleForwardSpeed() const
{
  return CarlaVehicle->GetVehicleMovementComponent()->GetForwardSpeed();
}
