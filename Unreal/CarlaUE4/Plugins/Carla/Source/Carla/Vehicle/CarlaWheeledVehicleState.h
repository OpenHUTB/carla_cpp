// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CarlaWheeledVehicleState.generated.h"

/// 用于调试目的，在编辑器中显示的CARLA轮式车辆状态。
UENUM(BlueprintType)
enum class ECarlaWheeledVehicleState : uint8
{
  AutopilotOff UMETA(DisplayName        = "Autopilot Off"),
  FreeDriving UMETA(DisplayName         = "Free driving"),
  FollowingFixedRoute UMETA(DisplayName = "Following fixed route"),
  WaitingForRedLight UMETA(DisplayName  = "Waiting for red light"),
  ObstacleAhead UMETA(DisplayName       = "Obstacle ahead"),
  UNKNOWN UMETA(DisplayName             = "Unknown")
};
