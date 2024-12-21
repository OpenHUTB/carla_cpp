// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "Carla/Weather/WeatherParameters.h"

#include "Weather.generated.h"

class ASensor;
class ASceneCaptureCamera;

UCLASS(Abstract)
class CARLA_API AWeather : public AActor
{
  GENERATED_BODY()

public:

  AWeather(const FObjectInitializer& ObjectInitializer);

  /// 更新天气参数并将其通知到蓝图的事件
  UFUNCTION(BlueprintCallable)
  void ApplyWeather(const FWeatherParameters &WeatherParameters);

  /// 将天气通知到蓝图的事件
  void NotifyWeather(ASensor* Sensor = nullptr);

  /// 在不通知蓝图事件的情况下更新天气参数
  UFUNCTION(BlueprintCallable)
  void SetWeather(const FWeatherParameters &WeatherParameters);

  /// 返回当前的 天气参数 WeatherParameters
  UFUNCTION(BlueprintCallable)
  const FWeatherParameters &GetCurrentWeather() const
  {
    return Weather;
  }

  /// 返回昼夜循环是否有效（切换到夜间模式时自动 开/关 切换）
  UFUNCTION(BlueprintCallable)
  const bool &GetDayNightCycle() const
  {
    return DayNightCycle;
  }

  /// 更新昼夜周期
  void SetDayNightCycle(const bool &active);

protected:

  UFUNCTION(BlueprintImplementableEvent)
// 这是一个在蓝图中可实现的事件
// 刷新天气参数
  void RefreshWeather(const FWeatherParameters &WeatherParameters);

private:

// 检查天气后处理效果
  void CheckWeatherPostProcessEffects();

// 这是一个在任何地方都可见的属性
// 天气参数
  UPROPERTY(VisibleAnywhere)
  FWeatherParameters Weather;

// 材质指针，用于降水后处理
  UMaterial* PrecipitationPostProcessMaterial;

// 材质指针，用于沙尘暴后处理
  UMaterial* DustStormPostProcessMaterial;


// 映射，键是材质指针，值是浮点数
// 活动混合
  TMap<UMaterial*, float> ActiveBlendables;

  UPROPERTY(EditAnywhere, Category = "Weather")
// 这是一个在任何地方都可编辑的属性，分类为"Weather"
// 日夜循环是否启用
  bool DayNightCycle = true;
};
