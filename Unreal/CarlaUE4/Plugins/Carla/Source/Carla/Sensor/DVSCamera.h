// Copyright (c) 2020 Robotics and Perception Group (GPR)
// University of Zurich and ETH Zurich
//
// 动态视觉传感器只对运动的目标成像，的每个像素的工作是相互独立、互不干涉的。
// 也就是说每个感光像素都只对自己所负责的一部分区域进行独立感光。
// 同时， 每个感光像素都不再直接采样实值， 而是时时刻刻去比较光电流的相对变化值是否超过了一个阈值。
// 当在对应像素上感受到足够的光强变化时， 传感器就会发出一个“事件信号”。
// 参考：https://blog.csdn.net/Yannan_Strath/article/details/126799956
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/SceneCaptureSensor.h"
#include "Sensor/ShaderBasedSensor.h"
#include <carla/sensor/data/DVSEvent.h>

#include "DVSCamera.generated.h"

namespace dvs
{
  /// 动态视觉传感器 (DVS, Dynamic Vision Sensor) 配置结构
  struct Config
  {
    float Cp;
    float Cm;
    float sigma_Cp;
    float sigma_Cm;
    std::uint64_t refractory_period_ns;
    bool use_log;
    float log_eps;
  };

  inline constexpr std::int64_t secToNanosec(double seconds)
  {
    return static_cast<std::int64_t>(seconds * 1e9);
  }

  inline constexpr double nanosecToSecTrunc(std::int64_t nanoseconds)
  {
    return static_cast<double>(nanoseconds) / 1e9;
  }

} // namespace dvs

/// Sensor that produce Dynamic Vision Events
UCLASS()
class CARLA_API ADVSCamera : public AShaderBasedSensor
{
  GENERATED_BODY()
  using DVSEventArray = std::vector<::carla::sensor::data::DVSEvent>;

public:
  ADVSCamera(const FObjectInitializer &ObjectInitializer);
  static FActorDefinition GetSensorDefinition();
  void Set(const FActorDescription &ActorDescription) override;

protected:
  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime) override;
  void ImageToGray(const TArray<FColor> &image);
  void ImageToLogGray(const TArray<FColor> &image);
  ADVSCamera::DVSEventArray Simulation (float DeltaTime);

private:
  /// Images containing last (current) and previous image
  TArray<float> last_image, prev_image;

  /// Image containing the last reference value to trigger event
  TArray<float> ref_values;

  /// Image containing time of last event in seconds
  TArray<double> last_event_timestamp;

  /// Current time in nanoseconds
  std::int64_t current_time;

  /// DVS simulation configuration
  dvs::Config config;
};
