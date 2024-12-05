// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

namespace carla {
namespace geom {

  class Location;

  class GeoLocation {
  public: ///定义 GeoLocation 类，它是一个公开的成员。

    // =========================================================================
    // -- 公共数据成员 --------------------------------------------------
    // =========================================================================

    double latitude = 0.0; ///纬度，初始化为0.0。

    double longitude = 0.0; ///经度，初始化为0.0。

    double altitude = 0.0; ///海拔，初始化为0.0。

    // =========================================================================
    // -- 构造函数 ---------------------------------------------------------
    // =========================================================================

    GeoLocation() = default; // 默认构造函数：使用默认值初始化。

    GeoLocation(double latitude, double longitude, double altitude) // 参数化构造函数：接受纬度、经度和海拔作为参数，并初始化对象。
      : latitude(latitude),
        longitude(longitude),
        altitude(altitude) {}

    // =========================================================================
    // -- 变换位置 --------------------------------------------------------------
    // =========================================================================

    /// 使用此对象作为地理参考，将给定的 @a location 转换为 GeoLocation。
    /// 
    GeoLocation Transform(const Location &location) const;

    // =========================================================================
    // -- 比较运算符 -------------------------------------------------
    // =========================================================================

    bool operator==(const GeoLocation &rhs) const {
      return (latitude == rhs.latitude) && (longitude == rhs.longitude) && (altitude == rhs.altitude);
    } ///等于运算符：如果两个 GeoLocation 对象的纬度、经度和海拔都相等，则返回 true。

    bool operator!=(const GeoLocation &rhs) const {
      return !(*this == rhs);
    } ///不等于运算符：如果两个 GeoLocation 对象的纬度、经度或海拔不相等，则返回 true。

    MSGPACK_DEFINE_ARRAY(latitude, longitude, altitude);
  };

} // namespace geom
} // namespace carla

