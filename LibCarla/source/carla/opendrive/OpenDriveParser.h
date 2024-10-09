// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/Map.h" // 引入 carla 交通道路地图的头文件

#include <boost/optional.hpp> // 引入 Boost 库中的可选类型头文件

#include <string>
// 引入CARLA项目的命名空间，CARLA是一个开源的自动驾驶模拟器
namespace carla {
namespace opendrive {
// 定义一个名为OpenDriveParser的类，该类用于解析OpenDRIVE格式的数据 ，OpenDRIVE是一个用于道路网络描述的XML格式标准，广泛应用于自动驾驶仿真领域 
  class OpenDriveParser {
  public:
// 函数返回一个boost::optional<road::Map>类型的值 ， boost::optional是一个模板类，用于表示一个可能不存在的值  
// 在这里，它表示可能成功解析并生成一个road::Map对象，也可能因为某些原因（如文件不存在、解析错误等）而失败  
// road::Map是CARLA中定义的一个类，用于表示一个完整的道路网络地图  
    static boost::optional<road::Map> Load(const std::string &opendrive);
  };

} // namespace opendrive
} // namespace carla
