// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/geom/GeoLocation.h"

#include "carla/geom/Location.h"
#include "carla/geom/Math.h" // 包含数学函数的声明

#include <cmath>

#if defined(_WIN32) && !defined(_USE_MATH_DEFINES) // 如果在Windows平台上且没有定义_USE_MATH_DEFINES
#  define _USE_MATH_DEFINES
#  include <math.h> // cmath is not enough for MSVC
#endif

namespace carla {
namespace geom {

  /// 定义地球赤道半径的常量，单位是米
  static constexpr double EARTH_RADIUS_EQUA = 6378137.0;

  /// 将纬度转换为墨卡托投影所需的比例尺
  /// 
  /// 输入参数：纬度（度）
  /// 返回值：比例尺因子
  /// 注意：在lat/lon和mercator之间转换时
  /// 或者反过来，两次转换中使用相同的比例尺
  static double LatToScale(double lat) {
    return std::cos(Math::ToRadians(lat));
  } // 将纬度转换为弧度并计算余弦值作为比例尺

  /// 将纬度、经度和比例尺转换为墨卡托坐标系中的mx/my
  /// 
  template <class float_type> // 模板函数，允许使用不同的浮点类型
  static void LatLonToMercator(double lat, double lon, double scale, float_type &mx, float_type &my) {
    mx = scale * Math::ToRadians(lon) * EARTH_RADIUS_EQUA; // 计算mx
    my = scale * EARTH_RADIUS_EQUA * std::log(std::tan((90.0 + lat) * Math::Pi<double>() / 360.0)); // 计算my
  }

  /// 将墨卡托坐标系中的mx/my转换回纬度、经度
  static void MercatorToLatLon(double mx, double my, double scale, double &lat, double &lon) {
    lon = mx * 180.0 / (Math::Pi<double>() * EARTH_RADIUS_EQUA * scale); // 计算经度
    lat = 360.0 * std::atan(std::exp(my / (EARTH_RADIUS_EQUA * scale))) / Math::Pi<double>() - 90.0;// 计算纬度
  }

  /// 在给定的纬度、经度上添加米为单位的dx/dy，并返回新的纬度、经度
  static void LatLonAddMeters(
      double lat_start, // 起始纬度
      double lon_start, // 起始经度
      double dx, // 要添加的x方向的米数
      double dy, // 要添加的y方向的米数
      double &lat_end, // 输出参数：结束纬度
      double &lon_end) { // 输出参数：结束经度
    double scale = LatToScale(lat_start); // 计算起始纬度的比例尺
    double mx, my; // 墨卡托坐标系中的临时变量
    LatLonToMercator(lat_start, lon_start, scale, mx, my); // 将起始经纬度转换为墨卡托坐标系
    mx += dx; // 在x方向上添加米数
    my += dy; // 在y方向上添加米数
    MercatorToLatLon(mx, my, scale, lat_end, lon_end); // 将墨卡托坐标系的坐标转换回经纬度
  }

  GeoLocation GeoLocation::Transform(const Location &location) const { // GeoLocation类的成员函数，用于转换Location对象
    GeoLocation result{0.0, 0.0, altitude + location.z}; // 创建一个新的GeoLocation对象，其高度是当前高度加上location的z坐标
    LatLonAddMeters( // 调用函数在当前GeoLocation上添加location的x和y坐标
        latitude, longitude, // 当前纬度和经度
        location.x, -location.y, // 反转y轴，使得纬度向北增加
        result.latitude, result.longitude); // 输出新的纬度和经度
    return result; // 返回新的GeoLocation对象
  }

} // namespace geom
} // namespace carla
