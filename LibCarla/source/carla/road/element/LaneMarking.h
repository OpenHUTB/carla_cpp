// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 只包含一次

#include <cstdint>  // 引入整数类型定义
#include <string>   // 引入字符串类型定义

namespace carla {  // 定义命名空间carla
namespace road {   // 定义命名空间road
namespace element {  // 定义命名空间element

  class RoadInfoMarkRecord;  // 前向声明RoadInfoMarkRecord类

  struct LaneMarking {  // 定义LaneMarking结构体

    enum class Type {  // 定义车道标记类型的枚举
      Other,  // 其他
      Broken,  // 断开的
      Solid,  // 实线
      // (用于双实线)
      SolidSolid,  // 双实线
      // (从内到外，例外：中心车道 - 从左到右)
      SolidBroken,  // 实线-断线
      // (从内到外，例外：中心车道 - 从左到右)
      BrokenSolid,  // 断线-实线
      // (从内到外，例外：中心车道 - 从左到右)
      BrokenBroken,  // 断线-断线
      BottsDots,  // Botts点
      // (表示草地边缘)
      Grass,  // 草地
      Curb,  // 路缘石
      None  // 无
    };

    enum class Color : uint8_t {  // 定义颜色的枚举
      Standard = 0u, // (等同于“白色”)
      Blue     = 1u,  // 蓝色
      Green    = 2u,  // 绿色
      Red      = 3u,  // 红色
      White    = Standard,  // 白色
      Yellow   = 4u,  // 黄色
      Other    = 5u   // 其他
    };

    /// 可以用作标志。
    enum class LaneChange : uint8_t {  // 定义车道变换的枚举
      None  = 0x00, // 00  - 无
      Right = 0x01, // 01  - 向右
      Left  = 0x02, // 10  - 向左
      Both  = 0x03  // 11  - 双向
    };

    explicit LaneMarking(const RoadInfoMarkRecord &info);  // 构造函数，接受RoadInfoMarkRecord对象

    Type type = Type::None;  // 车道标记类型，默认值为None

    Color color = Color::Standard;  // 车道标记颜色，默认值为Standard（白色）

    LaneChange lane_change = LaneChange::None;  // 车道变换标志，默认值为None

    double width = 0.0;  // 车道标记宽度，默认值为0.0

    std::string GetColorInfoAsString() {  // 获取颜色信息的字符串表示
      switch(color) {  // 根据颜色类型进行选择
        case Color::Yellow:  // 如果颜色是黄色
          return std::string("yellow");  // 返回“yellow”
          break;  // 结束当前case
        case Color::Standard:  // 如果颜色是标准（白色）
          return std::string("white");  // 返回“white”
          break;  // 结束当前case
        default:  // 默认情况
          return std::string("white");  // 返回“white”
          break;  // 结束当前case
      }
      return std::string("white");  // 最终返回“white”
    }
  };

} // namespace element
} // namespace road
} // namespace carla
