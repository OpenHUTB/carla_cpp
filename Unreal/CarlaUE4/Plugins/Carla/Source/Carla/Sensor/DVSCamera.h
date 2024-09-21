// Copyright (c) 2020 Robotics and Perception Group (GPR)
// 苏黎世大学和苏黎世联邦理工学院
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
    float Cp;        // 与亮度变化增量相关的正阈值 C，范围为 (0-1)。
    float Cm;        // 与亮度变化减少相关的负阈值 C，范围为(0-1)。
    float sigma_Cp;  // 正事件的白噪声标准差，范围为 (0-1)。
    float sigma_Cm;  // 负事件的白噪声标准差，范围为 (0-1)。
    std::uint64_t refractory_period_ns;  // 不应期（像素在触发事件后无法触发事件的时间），以纳秒为单位。它限制了触发事件的最高频率。
    bool use_log;    // 是否以对数强度刻度工作。
    float log_eps;   // 用于将图像转换为对数的 Epsilon 值
  };

  // 秒转纳秒
  inline constexpr std::int64_t secToNanosec(double seconds)
  {
    return static_cast<std::int64_t>(seconds * 1e9);
  }

  // 纳秒转秒（小于1纳秒进行截断）
  inline constexpr double nanosecToSecTrunc(std::int64_t nanoseconds)
  {
    return static_cast<double>(nanoseconds) / 1e9;
  }

} // namespace dvs

/// 产生动态视觉事件的传感器
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
  /// 包含最新（当前）图像和先前图像的图像
  TArray<float> last_image, prev_image;

  /// 包含触发事件最新参考值的图像
  TArray<float> ref_values;

  /// 包含以秒为单位的最新事件时间的图像
  TArray<double> last_event_timestamp;

  /// 以纳秒为单位的当前时间
  std::int64_t current_time;

  /// 动态时间传感器的仿真配置
  dvs::Config config;
};
