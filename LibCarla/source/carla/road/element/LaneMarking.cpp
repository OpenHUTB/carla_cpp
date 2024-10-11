// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/LaneMarking.h" // 引入 LaneMarking 头文件

#include "carla/Exception.h" // 引入异常处理头文件
#include "carla/StringUtil.h" // 引入字符串工具头文件
#include "carla/road/element/RoadInfoMarkRecord.h" // 引入道路信息标记记录头文件

namespace carla {
namespace road {
namespace element {

  // 静态函数：根据字符串获取 LaneMarking 类型
  static LaneMarking::Type GetType(std::string str) {
    StringUtil::ToLower(str); // 将字符串转换为小写
    if (str == "broken") { // 检查是否为 "broken"
      return LaneMarking::Type::Broken; // 返回 LaneMarking::Type::Broken
    } else if (str == "solid") { // 检查是否为 "solid"
      return LaneMarking::Type::Solid; // 返回 LaneMarking::Type::Solid
    } else if (str == "solid solid") { // 检查是否为 "solid solid"
      return LaneMarking::Type::SolidSolid; // 返回 LaneMarking::Type::SolidSolid
    } else if (str == "solid broken") { // 检查是否为 "solid broken"
      return LaneMarking::Type::SolidBroken; // 返回 LaneMarking::Type::SolidBroken
    } else if (str == "broken solid") { // 检查是否为 "broken solid"
      return LaneMarking::Type::BrokenSolid; // 返回 LaneMarking::Type::BrokenSolid
    } else if (str == "broken broken") { // 检查是否为 "broken broken"
      return LaneMarking::Type::BrokenBroken; // 返回 LaneMarking::Type::BrokenBroken
    } else if (str == "botts dots") { // 检查是否为 "botts dots"
      return LaneMarking::Type::BottsDots; // 返回 LaneMarking::Type::BottsDots
    } else if (str == "grass") { // 检查是否为 "grass"
      return LaneMarking::Type::Grass; // 返回 LaneMarking::Type::Grass
    } else if (str == "curb") { // 检查是否为 "curb"
      return LaneMarking::Type::Curb; // 返回 LaneMarking::Type::Curb
    } else if (str == "none") { // 检查是否为 "none"
      return LaneMarking::Type::None; // 返回 LaneMarking::Type::None
    } else {
      return LaneMarking::Type::Other; // 返回 LaneMarking::Type::Other
    }
  }

  // 静态函数：根据字符串获取 LaneMarking 颜色
  static LaneMarking::Color GetColor(std::string str) {
    StringUtil::ToLower(str); // 将字符串转换为小写
    if (str == "standard") { // 检查是否为 "standard"
      return LaneMarking::Color::Standard; // 返回 LaneMarking::Color::Standard
    } else if (str == "blue") { // 检查是否为 "blue"
      return LaneMarking::Color::Blue; // 返回 LaneMarking::Color::Blue
    } else if (str == "green") { // 检查是否为 "green"
      return LaneMarking::Color::Green; // 返回 LaneMarking::Color::Green
    } else if (str == "red") { // 检查是否为 "red"
      return LaneMarking::Color::Red; // 返回 LaneMarking::Color::Red
    } else if (str == "white") { // 检查是否为 "white"
      return LaneMarking::Color::White; // 返回 LaneMarking::Color::White
    } else if (str == "yellow") { // 检查是否为 "yellow"
      return LaneMarking::Color::Yellow; // 返回 LaneMarking::Color::Yellow
    } else {
      return LaneMarking::Color::Other; // 返回 LaneMarking::Color::Other
    }
  }

  // 静态函数：根据 RoadInfoMarkRecord 的 LaneChange 获取 LaneMarking 的 LaneChange
  static LaneMarking::LaneChange GetLaneChange(RoadInfoMarkRecord::LaneChange lane_change) {
    switch (lane_change) { // 根据 lane_change 进行判断
      case RoadInfoMarkRecord::LaneChange::Increase: // 如果是 Increase
        return LaneMarking::LaneChange::Right; // 返回 LaneMarking::LaneChange::Right
      case RoadInfoMarkRecord::LaneChange::Decrease: // 如果是 Decrease
        return LaneMarking::LaneChange::Left; // 返回 LaneMarking::LaneChange::Left
      case RoadInfoMarkRecord::LaneChange::Both: // 如果是 Both
        return LaneMarking::LaneChange::Both; // 返回 LaneMarking::LaneChange::Both
      default: // 默认情况
        return LaneMarking::LaneChange::None; // 返回 LaneMarking::LaneChange::None
    }
  }

  // LaneMarking 构造函数，接收一个 RoadInfoMarkRecord 对象
  LaneMarking::LaneMarking(const RoadInfoMarkRecord &info)
    : type(GetType(info.GetType())), // 初始化 type
      color(GetColor(info.GetColor())), // 初始化 color
      lane_change(GetLaneChange(info.GetLaneChange())), // 初始化 lane_change
      width(info.GetWidth()) {} // 初始化 width

} // namespace element
} // namespace road
} // namespace carla
