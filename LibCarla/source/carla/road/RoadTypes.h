// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  
  
// 包含C++标准库中的固定宽度整数类型定义  
#include <cstdint>  
// 包含C++标准库中的字符串类定义  
#include <string>  
  
// 定义carla命名空间  
namespace carla {  
  // 在carla命名空间内定义road子命名空间  
  namespace road {  
  
    // 道路ID类型别名，使用32位无符号整数表示  
    using RoadId = uint32_t;  
  
    // 交叉路口ID类型别名，使用32位有符号整数表示  
    using JuncId = int32_t;  
  
    // 车道ID类型别名，使用32位有符号整数表示  
    using LaneId = int32_t;  
  
    // 路段ID类型别名，使用32位无符号整数表示  
    using SectionId = uint32_t;  
  
    // 对象ID类型别名（可能是道路上的某种物体），使用32位无符号整数表示  
    using ObjId = uint32_t;  
  
    // 标志牌ID类型别名，使用std::string表示，可能用于存储唯一标识符或名称  
    using SignId = std::string;  
  
    // 连接ID类型别名（可能是道路之间的连接点），使用32位无符号整数表示  
    using ConId = uint32_t;  
  
    // 容器ID类型别名（可能是某种逻辑或物理容器的标识符），使用std::string表示  
    using ContId = std::string;  
  
  } // road命名空间结束  
} // carla命名空间结束
