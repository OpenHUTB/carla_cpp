// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#include "SignalType.h" // 引入 SignalType.h 头文件

#include <vector> // 引入 vector 头文件
#include <algorithm> // 引入 algorithm 头文件

namespace carla { // 定义命名空间 carla
namespace road { // 定义命名空间 road

  const std::string SignalType::Danger() { // 定义危险信号类型
    return "101"; // 返回 "101"
  } // 从 101 到 151 的危险类型
  const std::string SignalType::LanesMerging() { // 定义车道合并信号类型
    return "121"; // 返回 "121"
  } // = "121";
  const std::string SignalType::CautionPedestrian() { // 定义小心行人信号类型
    return "133"; // 返回 "133"
  } // = "133";
  const std::string SignalType::CautionBicycle() { // 定义小心自行车信号类型
    return "138"; // 返回 "138"
  } // = "138";
  const std::string SignalType::LevelCrossing() { // 定义平交道口信号类型
    return "150"; // 返回 "150"
  } // = "150";
  const std::string SignalType::YieldSign() { // 定义让行标志信号类型
    return "205"; // 返回 "205"
  } // = "205";
  const std::string SignalType::StopSign() { // 定义停车标志信号类型
    return "206"; // 返回 "206"
  } // = "206";
  const std::string SignalType::MandatoryTurnDirection() { // 定义强制转向方向信号类型
    return "209"; // 返回 "209" // 左转、右转或直行
  } // = "209";
  const std::string SignalType::MandatoryLeftRightDirection() { // 定义强制左转或右转信号类型
    return "211"; // 返回 "211"
  } // = "211";
  const std::string SignalType::TwoChoiceTurnDirection() { // 定义两种选择转向信号类型
    return "214"; // 返回 "214" // 直行-左转、直行-右转、左转-右转
  } // = "214";
  const std::string SignalType::Roundabout() { // 定义环形交叉口信号类型
    return "215"; // 返回 "215"
  } // = "215";
  const std::string SignalType::PassRightLeft() { // 定义右侧左侧通行信号类型
    return "222"; // 返回 "222"
  } // = "222";
  const std::string SignalType::AccessForbidden() { // 定义禁止通行信号类型
    return "250"; // 返回 "250"
  } // = "250";
  const std::string SignalType::AccessForbiddenMotorvehicles() { // 定义禁止机动车通行信号类型
    return "251"; // 返回 "251"
  } // = "251";
  const std::string SignalType::AccessForbiddenTrucks() { // 定义禁止卡车通行信号类型
    return "253"; // 返回 "253"
  } // = "253";
  const std::string SignalType::AccessForbiddenBicycle() { // 定义禁止自行车通行信号类型
    return "254"; // 返回 "254"
  } // = "254";
  const std::string SignalType::AccessForbiddenWeight() { // 定义禁止超重通行信号类型
    return "263"; // 返回 "263"
  } // = "263";
  const std::string SignalType::AccessForbiddenWidth() { // 定义禁止超宽通行信号类型
    return "264"; // 返回 "264"
  } // = "264";
  const std::string SignalType::AccessForbiddenHeight() { // 定义禁止超高通行信号类型
    return "265"; // 返回 "265"
  } // = "265";
  const std::string SignalType::AccessForbiddenWrongDirection() { // 定义禁止错误方向通行信号类型
    return "267"; // 返回 "267"
  } // = "267";
  const std::string SignalType::ForbiddenUTurn() { // 定义禁止掉头信号类型
    return "272"; // 返回 "272"
  } // = "272";
  const std::string SignalType::MaximumSpeed() { // 定义最高速度信号类型
    return "274"; // 返回 "274"
  } // = "274";
  const std::string SignalType::ForbiddenOvertakingMotorvehicles() { // 定义禁止超车信号类型（机动车）
    return "276"; // 返回 "276"
  } // = "276";
  const std::string SignalType::ForbiddenOvertakingTrucks() { // 定义禁止超车信号类型（卡车）
    return "277"; // 返回 "277"
  } // = "277";
  const std::string SignalType::AbsoluteNoStop() { // 定义绝对禁止停车信号类型
    return "283"; // 返回 "283"
  } // = "283";
  const std::string SignalType::RestrictedStop() { // 定义限制停车信号类型
    return "286"; // 返回 "286"
  } // = "286";
  const std::string SignalType::HasWayNextIntersection() { // 定义下个交叉口优先通行信号类型
    return "301"; // 返回 "301"
  } // = "301";
  const std::string SignalType::PriorityWay() { // 定义优先通行信号类型
    return "306"; // 返回 "306"
  } // = "306";
  const std::string SignalType::PriorityWayEnd() { // 定义优先通行结束信号类型
    return "307"; // 返回 "307"
  } // = "307";
  const std::string SignalType::CityBegin() { // 定义城市开始信号类型
    return "310"; // 返回 "310"
  } // = "310";
  const std::string SignalType::CityEnd() { // 定义城市结束信号类型
    return "311"; // 返回 "311"
  } // = "311";
  const std::string SignalType::Highway() { // 定义高速公路信号类型
    return "330"; // 返回 "330"
  } // = "330";
  const std::string SignalType::DeadEnd() { // 定义死胡同信号类型
    return "357"; // 返回 "357"
  } // = "357";
  const std::string SignalType::RecomendedSpeed() { // 定义推荐速度信号类型
    return "380"; // 返回 "380"
  } // = "380";
  const std::string SignalType::RecomendedSpeedEnd() { // 定义推荐速度结束信号类型
    return "381"; // 返回 "381"
  } // = "381";

  bool SignalType::IsTrafficLight(const std::string &type) { // 定义函数判断是否为交通信号灯
    // 交通信号灯对应的类型
    const std::vector<std::string> traffic_light_types = // 定义交通信号灯类型的向量
        {"1000001", "1000002", "1000009", "1000010", "1000011", // 交通灯类型
         "1000007", "1000014", "1000015", "1000016", "1000017",
         "1000018", "1000019", "1000013", "1000020", "1000008",
         "1000012", "F", "W", "A"}; // 其他类型

    auto it = std::find( // 查找类型是否在交通信号灯类型中
        traffic_light_types.begin(), traffic_light_types.end(), type);
    if (it != traffic_light_types.end()){ // 如果找到了
      return true; // 返回 true
    } else {
      return false; // 否则返回 false
    }
  }

}
}