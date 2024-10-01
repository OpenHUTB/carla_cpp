// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/Map.h" // 引入 carla 交通道路地图的头文件

#include <boost/optional.hpp> // 引入 Boost 库中的可选类型头文件

#include <string> // 引入字符串类型的头文件

namespace carla { // 定义命名空间 carla
namespace opendrive { // 定义命名空间 opendrive

  class OpenDriveParser { // 声明 OpenDriveParser 类
  public:

    static boost::optional<road::Map> Load(const std::string &opendrive); // 静态成员函数 Load，返回可选的道路地图对象
  };

} // namespace opendrive
} // namespace carla
