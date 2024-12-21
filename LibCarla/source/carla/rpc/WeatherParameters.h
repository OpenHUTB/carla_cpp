// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 该指令确保头文件只会被编译器包含一次，防止重复包含导致的编译错误。
#pragma once

// 引入MsgPack库头文件，用于序列化和反序列化数据。
#include "carla/MsgPack.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Carla/Weather/WeatherParameters.h"
#include <compiler/disable-ue4-macros.h>
// 在 Unreal Engine 4 (UE4) 环境中编译时，
// 包含FWeatherParameters相关头文件，启用和禁用UE4宏。
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

  class WeatherParameters {
  public:

    /// @name Weather presets
    /// @{
    // 一组静态预设值，表示不同的天气条件。
    static WeatherParameters Default;// 默认天气
    static WeatherParameters ClearNoon;// 晴朗的中午
    static WeatherParameters CloudyNoon;// 多云的中午
    static WeatherParameters WetNoon;///< 湿润的中午
    static WeatherParameters WetCloudyNoon;// 湿润多云的中午
    static WeatherParameters MidRainyNoon;// 小雨的中午
    static WeatherParameters HardRainNoon;// 大雨的中午
    static WeatherParameters SoftRainNoon;// 柔和的小雨中午
    static WeatherParameters ClearSunset;// 晴朗的日落
    static WeatherParameters CloudySunset;// 多云的日落
    static WeatherParameters WetSunset;// 湿润的日落
    static WeatherParameters WetCloudySunset;// 湿润多云的日落
    static WeatherParameters MidRainSunset;// 小雨的日落
    static WeatherParameters HardRainSunset;// 大雨的日落
    static WeatherParameters SoftRainSunset;// 柔和的小雨日落
    static WeatherParameters ClearNight;// 晴朗的夜晚
    static WeatherParameters CloudyNight;// 多云的夜晚
    static WeatherParameters WetNight;// 湿润的夜晚
    static WeatherParameters WetCloudyNight;// 湿润多云的夜晚
    static WeatherParameters SoftRainNight;// 小雨的夜晚
    static WeatherParameters MidRainyNight;// 中等强度的雨夜
    static WeatherParameters HardRainNight;// 大雨的夜晚
    static WeatherParameters DustStorm;// 尘暴天气

    /// @}

    //默认构造函数，所有参数初始化为默认值0。
    WeatherParameters() = default;

    //带参数的构造函数，用于初始化天气参数。
    WeatherParameters(
        float in_cloudiness,
        float in_precipitation,
        float in_precipitation_deposits,
        float in_wind_intensity,
        float in_sun_azimuth_angle,
        float in_sun_altitude_angle,
        float in_fog_density,
        float in_fog_distance,
        float in_fog_falloff,
        float in_wetness,
        float in_scattering_intensity,
        float in_mie_scattering_scale,
        float in_rayleigh_scattering_scale,
        float in_dust_storm)
      : cloudiness(in_cloudiness),
        precipitation(in_precipitation),
        precipitation_deposits(in_precipitation_deposits),
        wind_intensity(in_wind_intensity),
        sun_azimuth_angle(in_sun_azimuth_angle),
        sun_altitude_angle(in_sun_altitude_angle),
        fog_density(in_fog_density),
        fog_distance(in_fog_distance),
        fog_falloff(in_fog_falloff),
        wetness(in_wetness),
        scattering_intensity(in_scattering_intensity),
        mie_scattering_scale(in_mie_scattering_scale),
        rayleigh_scattering_scale(in_rayleigh_scattering_scale),
        dust_storm(in_dust_storm) {}

    float cloudiness = 0.0f;// 云量
    float precipitation = 0.0f;// 降水量
    float precipitation_deposits = 0.0f;// 降水残留物的积累程度
    float wind_intensity = 0.0f;// 风力强度
    float sun_azimuth_angle = 0.0f;// 太阳方位角度
    float sun_altitude_angle = 0.0f;// 太阳仰角
    float fog_density = 0.0f;// 雾的密度
    float fog_distance = 0.0f;// 雾的能见度距离
    float fog_falloff = 0.0f;// 雾的衰减程度
    float wetness = 0.0f;// 湿润度
    float scattering_intensity = 0.0f;//光散射强度
    float mie_scattering_scale = 0.0f;//Mie散射缩放
    float rayleigh_scattering_scale = 0.0331f;//Rayleigh散射缩放
    float dust_storm = 0.0f;//尘暴强度

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    //构造函数：从UE4的FWeatherParameters初始化WeatherParameters
    //Weather传入的FWeatherParameters对象，用于设置当前对象的属性。
    WeatherParameters(const FWeatherParameters &Weather)
      : cloudiness(Weather.Cloudiness),
        precipitation(Weather.Precipitation),
        precipitation_deposits(Weather.PrecipitationDeposits),
        wind_intensity(Weather.WindIntensity),
        sun_azimuth_angle(Weather.SunAzimuthAngle),
        sun_altitude_angle(Weather.SunAltitudeAngle),
        fog_density(Weather.FogDensity),
        fog_distance(Weather.FogDistance),
        fog_falloff(Weather.FogFalloff),
        wetness(Weather.Wetness),
        scattering_intensity(Weather.ScatteringIntensity),
        mie_scattering_scale(Weather.MieScatteringScale),
        rayleigh_scattering_scale(Weather.RayleighScatteringScale),
        dust_storm(Weather.DustStorm) {}

    operator FWeatherParameters() const {
      FWeatherParameters Weather;
      Weather.Cloudiness = cloudiness;
      Weather.Precipitation = precipitation;
      Weather.PrecipitationDeposits = precipitation_deposits;
      Weather.WindIntensity = wind_intensity;
      Weather.SunAzimuthAngle = sun_azimuth_angle;
      Weather.SunAltitudeAngle = sun_altitude_angle;
      Weather.FogDensity = fog_density;
      Weather.FogDistance = fog_distance;
      Weather.FogFalloff = fog_falloff;
      Weather.Wetness = wetness;
      Weather.ScatteringIntensity = scattering_intensity;
      Weather.MieScatteringScale = mie_scattering_scale;
      Weather.RayleighScatteringScale = rayleigh_scattering_scale;
      Weather.DustStorm = dust_storm;
      return Weather;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    //检查两个WeatherParameters对象是否不相等，如果两个对象的任意属性不同，则返回 true，否则返回 false。
    bool operator!=(const WeatherParameters &rhs) const {
      return
          cloudiness != rhs.cloudiness ||
          precipitation != rhs.precipitation ||
          precipitation_deposits != rhs.precipitation_deposits ||
          wind_intensity != rhs.wind_intensity ||
          sun_azimuth_angle != rhs.sun_azimuth_angle ||
          sun_altitude_angle != rhs.sun_altitude_angle ||
          fog_density != rhs.fog_density ||
          fog_distance != rhs.fog_distance ||
          fog_falloff != rhs.fog_falloff ||
          wetness != rhs.wetness ||
          scattering_intensity != rhs.scattering_intensity ||
          mie_scattering_scale != rhs.mie_scattering_scale ||
          rayleigh_scattering_scale != rhs.rayleigh_scattering_scale ||
          dust_storm != rhs.dust_storm;
    }

    bool operator==(const WeatherParameters &rhs) const {
      return !(*this != rhs);
    }

    //定义 WeatherParameters对象的序列化顺序，便于打包和解包
    MSGPACK_DEFINE_ARRAY(
        cloudiness,
        precipitation,
        precipitation_deposits,
        wind_intensity,
        sun_azimuth_angle,
        sun_altitude_angle,
        fog_density,
        fog_distance,
        fog_falloff,
        wetness,
        scattering_intensity,
        mie_scattering_scale,
        rayleigh_scattering_scale,
        dust_storm);
  };

} // namespace rpc
} // namespace carla
