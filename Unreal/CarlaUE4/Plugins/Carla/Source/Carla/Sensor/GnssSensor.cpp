// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Sensor/GnssSensor.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/MapGen/LargeMapManager.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Vector3D.h"
#include "carla/ros2/ROS2.h"
#include <compiler/enable-ue4-macros.h>

// GNSS传感器的构造函数，初始化随机数引擎和允许每帧调用Tick函数
AGnssSensor::AGnssSensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true; // 启用每帧调用Tick
  RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine")); // 创建随机引擎对象
}

// 获取传感器的定义信息
FActorDefinition AGnssSensor::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeGnssDefinition(); // 调用蓝图函数库获取GNSS定义
}

// 设置传感器属性
void AGnssSensor::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription); // 调用父类的Set方法
  UActorBlueprintFunctionLibrary::SetGnss(ActorDescription, this); // 使用蓝图函数库设置GNSS属性
}

// 在物理更新后调用，用于处理GNSS逻辑
void AGnssSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AGnssSensor::PostPhysTick);

  FVector ActorLocation = GetActorLocation(); // 获取传感器的当前位置
  ALargeMapManager * LargeMap = UCarlaStatics::GetLargeMapManager(GetWorld());
  if (LargeMap) // 如果有大地图管理器，则转换局部坐标到全局坐标
  {
    ActorLocation = LargeMap->LocalToGlobalLocation(ActorLocation);
  }
  carla::geom::Location Location = ActorLocation;
  carla::geom::GeoLocation CurrentLocation = CurrentGeoReference.Transform(Location); // 转换为地理坐标

  // 计算传感器噪声
  const float LatError = RandomEngine->GetNormalDistribution(0.0f, LatitudeDeviation);
  const float LonError = RandomEngine->GetNormalDistribution(0.0f, LongitudeDeviation);
  const float AltError = RandomEngine->GetNormalDistribution(0.0f, AltitudeDeviation);

  // 应用噪声和偏差
  double Latitude = CurrentLocation.latitude + LatitudeBias + LatError;
  double Longitude = CurrentLocation.longitude + LongitudeBias + LonError;
  double Altitude = CurrentLocation.altitude + AltitudeBias + AltError;

  auto Stream = GetDataStream(*this); // 获取数据流

  // 如果启用了ROS2，将数据发送到ROS2系统
  #if defined(WITH_ROS2)
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  if (ROS2->IsEnabled())
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("ROS2 Send");
    auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();
    AActor* ParentActor = GetAttachParentActor();
    if (ParentActor) // 如果有父实体，计算相对变换并发送数据
    {
      FTransform LocalTransformRelativeToParent = GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
      ROS2->ProcessDataFromGNSS(Stream.GetSensorType(), StreamId, LocalTransformRelativeToParent, carla::geom::GeoLocation{Latitude, Longitude, Altitude}, this);
    }
    else // 如果没有父实体，直接发送全局变换数据
    {
      ROS2->ProcessDataFromGNSS(Stream.GetSensorType(), StreamId, Stream.GetSensorTransform(), carla::geom::GeoLocation{Latitude, Longitude, Altitude}, this);
    }
  }
  #endif
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("AGnssSensor Stream Send");
    Stream.SerializeAndSend(*this, carla::geom::GeoLocation{Latitude, Longitude, Altitude}); // 发送数据流
  }
}

// 设置纬度偏差
void AGnssSensor::SetLatitudeDeviation(float Value)
{
  LatitudeDeviation = Value;
}

// 设置经度偏差
void AGnssSensor::SetLongitudeDeviation(float Value)
{
  LongitudeDeviation = Value;
}

// 设置高度偏差
void AGnssSensor::SetAltitudeDeviation(float Value)
{
  AltitudeDeviation = Value;
}

// 设置纬度偏移量
void AGnssSensor::SetLatitudeBias(float Value)
{
  LatitudeBias = Value;
}

// 设置经度偏移量
void AGnssSensor::SetLongitudeBias(float Value)
{
  LongitudeBias = Value;
}

// 设置高度偏移量
void AGnssSensor::SetAltitudeBias(float Value)
{
  AltitudeBias = Value;
}

// 获取纬度偏差
float AGnssSensor::GetLatitudeDeviation() const
{
  return LatitudeDeviation;
}

// 获取经度偏差
float AGnssSensor::GetLongitudeDeviation() const
{
  return LongitudeDeviation;
}

// 获取高度偏差
float AGnssSensor::GetAltitudeDeviation() const
{
  return AltitudeDeviation;
}

// 获取纬度偏移量
float AGnssSensor::GetLatitudeBias() const
{
  return LatitudeBias;
}

// 获取经度偏移量
float AGnssSensor::GetLongitudeBias() const
{
  return LongitudeBias;
}

// 获取高度偏移量
float AGnssSensor::GetAltitudeBias() const
{
  return AltitudeBias;
}

// 游戏开始时调用，初始化传感器的地理参考
void AGnssSensor::BeginPlay()
{
  Super::BeginPlay();

  const UCarlaEpisode* episode = UCarlaStatics::GetCurrentEpisode(GetWorld()); // 获取当前场景
  CurrentGeoReference = episode->GetGeoReference(); // 初始化地理参考
}

