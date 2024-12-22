// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/rpc/WeatherParameters.h>

#include <ostream>

namespace carla {

//rpc命名空间，通常用于存放与远程过程调用（RPC）相关的功能和数据类型相关的代码
namespace rpc {

  std::ostream &operator<<(std::ostream &out, const WeatherParameters &weather) {
    out << "WeatherParameters(cloudiness=" << std::to_string(weather.cloudiness)
        << ", precipitation=" << std::to_string(weather.precipitation)
        << ", precipitation_deposits=" << std::to_string(weather.precipitation_deposits)
        << ", wind_intensity=" << std::to_string(weather.wind_intensity)
        << ", sun_azimuth_angle=" << std::to_string(weather.sun_azimuth_angle)
        << ", sun_altitude_angle=" << std::to_string(weather.sun_altitude_angle)
        << ", fog_density=" << std::to_string(weather.fog_density)
        << ", fog_distance=" << std::to_string(weather.fog_distance)
        << ", fog_falloff=" << std::to_string(weather.fog_falloff)
        << ", wetness=" << std::to_string(weather.wetness)
        << ", scattering_intensity=" << std::to_string(weather.scattering_intensity)
        << ", mie_scattering_scale=" << std::to_string(weather.mie_scattering_scale)
        << ", rayleigh_scattering_scale=" << std::to_string(weather.rayleigh_scattering_scale)
        << ", dust_storm=" << std::to_string(weather.dust_storm) << ')';
    return out;//返回输出流对象
  }

} // namespace rpc//结束rcp命名空间
} // namespace carla

void export_weather() {//将WeatherParameters相关的类型、类等信息导出到Python环境中
    // 引入 Boost.Python 的命名空间以简化代码

  using namespace boost::python;
    // 创建一个别名，用于引用 carla::rpc 命名空间

  namespace cr = carla::rpc;

  // 使用class_模板在Python环境中定义一个名为WeatherParameters的类，对应于C++中的carla::rpc::WeatherParameters类
  auto cls = class_<cr::WeatherParameters>("WeatherParameters")
     // 定义WeatherParameters类的构造函数，可接受多个默认参数，用于初始化WeatherParameters对象的各个天气参数
     // 参数分别对应不同的天气参数，如cloudiness、precipitation等，每个参数都有默认值，如cloudiness的默认值为0.0f
    .def(init<float, float, float, float, float, float, float, float, float, float, float, float, float, float>(
        (arg("cloudiness")=0.0f,
         arg("precipitation")=0.0f,
         arg("precipitation_deposits")=0.0f,
         arg("wind_intensity")=0.0f,
         arg("sun_azimuth_angle")=0.0f,
         arg("sun_altitude_angle")=0.0f,
         arg("fog_density")=0.0f,
         arg("fog_distance")=0.0f,
         arg("fog_falloff")=0.0f,
         arg("wetness")=0.0f,
         arg("scattering_intensity")=0.0f,
         arg("mie_scattering_scale")=0.0f,
         arg("rayleigh_scattering_scale")=0.0331f,
         arg("dust_storm")=0.0f)))
     #定义一系列宏，用于将变量的值写入到对应的结构体中
    .def_readwrite("cloudiness", &cr::WeatherParameters::cloudiness)
    .def_readwrite("precipitation", &cr::WeatherParameters::precipitation)
    .def_readwrite("precipitation_deposits", &cr::WeatherParameters::precipitation_deposits)
    .def_readwrite("wind_intensity", &cr::WeatherParameters::wind_intensity)
    .def_readwrite("sun_azimuth_angle", &cr::WeatherParameters::sun_azimuth_angle)
    .def_readwrite("sun_altitude_angle", &cr::WeatherParameters::sun_altitude_angle)
    .def_readwrite("fog_density", &cr::WeatherParameters::fog_density)
    .def_readwrite("fog_distance", &cr::WeatherParameters::fog_distance)
    .def_readwrite("fog_falloff", &cr::WeatherParameters::fog_falloff)
    .def_readwrite("wetness", &cr::WeatherParameters::wetness)
    .def_readwrite("scattering_intensity", &cr::WeatherParameters::scattering_intensity)
    .def_readwrite("mie_scattering_scale", &cr::WeatherParameters::mie_scattering_scale)
    .def_readwrite("rayleigh_scattering_scale", &cr::WeatherParameters::rayleigh_scattering_scale)
    .def_readwrite("dust_storm", &cr::WeatherParameters::dust_storm)
    .def("__eq__", &cr::WeatherParameters::operator==)
    .def("__ne__", &cr::WeatherParameters::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  cls.attr("Default") = cr::WeatherParameters::Default;
  cls.attr("ClearNoon") = cr::WeatherParameters::ClearNoon;
  cls.attr("CloudyNoon") = cr::WeatherParameters::CloudyNoon;
  cls.attr("WetNoon") = cr::WeatherParameters::WetNoon;
  cls.attr("WetCloudyNoon") = cr::WeatherParameters::WetCloudyNoon;
  cls.attr("MidRainyNoon") = cr::WeatherParameters::MidRainyNoon;
  cls.attr("HardRainNoon") = cr::WeatherParameters::HardRainNoon;
  cls.attr("SoftRainNoon") = cr::WeatherParameters::SoftRainNoon;
  cls.attr("ClearSunset") = cr::WeatherParameters::ClearSunset;
  cls.attr("CloudySunset") = cr::WeatherParameters::CloudySunset;
  cls.attr("WetSunset") = cr::WeatherParameters::WetSunset;
  cls.attr("WetCloudySunset") = cr::WeatherParameters::WetCloudySunset;
  cls.attr("MidRainSunset") = cr::WeatherParameters::MidRainSunset;
  cls.attr("HardRainSunset") = cr::WeatherParameters::HardRainSunset;
  cls.attr("SoftRainSunset") = cr::WeatherParameters::SoftRainSunset;
  cls.attr("ClearNight") = cr::WeatherParameters::ClearNight;
  cls.attr("CloudyNight") = cr::WeatherParameters::CloudyNight;
  cls.attr("WetNight") = cr::WeatherParameters::WetNight;
  cls.attr("WetCloudyNight") = cr::WeatherParameters::WetCloudyNight;
  cls.attr("SoftRainNight") = cr::WeatherParameters::SoftRainNight;
  cls.attr("MidRainyNight") = cr::WeatherParameters::MidRainyNight;
  cls.attr("HardRainNight") = cr::WeatherParameters::HardRainNight;
  cls.attr("DustStorm") = cr::WeatherParameters::DustStorm;
}
