// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/GeoLocation.h"
#include <compiler/enable-ue4-macros.h>

#include "GnssSensor.generated.h"

///GNSS传感器表示类
/// 实际的位置信息计算是在服务器端进行的
UCLASS()
class CARLA_API AGnssSensor : public ASensor// 继承自ASensor类，表示GNSS传感器
{
  GENERATED_BODY()

public:

  AGnssSensor(const FObjectInitializer &ObjectInitializer);

  static FActorDefinition GetSensorDefinition();

  void Set(const FActorDescription &ActorDescription);

  // 虚拟的物理更新函数（已被注释掉）
  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;

  void SetLatitudeDeviation(float Value);
  void SetLongitudeDeviation(float Value);
  void SetAltitudeDeviation(float Value);

  void SetLatitudeBias(float Value);
  void SetLongitudeBias(float Value);
  void SetAltitudeBias(float Value);

  float GetLatitudeDeviation() const;
  float GetLongitudeDeviation() const;
  float GetAltitudeDeviation() const;

  float GetLatitudeBias() const;
  float GetLongitudeBias() const;
  float GetAltitudeBias() const;

protected:

  virtual void BeginPlay() override;

private:

  carla::geom::GeoLocation CurrentGeoReference;

  float LatitudeDeviation;
  float LongitudeDeviation;
  float AltitudeDeviation;

  float LatitudeBias;
  float LongitudeBias;
  float AltitudeBias;
};
