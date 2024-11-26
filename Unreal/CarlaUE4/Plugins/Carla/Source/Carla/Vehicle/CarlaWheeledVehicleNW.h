// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CarlaWheeledVehicle.h"

#include "CarlaWheeledVehicleNW.generated.h"

/// CARLA轮式车辆的基类
UCLASS()
class CARLA_API ACarlaWheeledVehicleNW : public ACarlaWheeledVehicle
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name Constructor and destructor
  // ===========================================================================
  /// @{
public:

  ACarlaWheeledVehicleNW(const FObjectInitializer &ObjectInitializer);

  ~ACarlaWheeledVehicleNW();
};
