// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace carla {
namespace road {

  /// 使用 OpenDRIVE 1.5M (6.10 国家代码)
  ///
  class SignalType {
  public:
    static const std::string Danger();     // = "101" // 危险类型，从 101 到 151
    static const std::string LanesMerging(); // = "121"; // 车道合并
    static const std::string CautionPedestrian(); // = "133"; // 注意行人
    static const std::string CautionBicycle(); // = "138"; // 注意自行车
    static const std::string LevelCrossing(); // = "150"; // 平交道
    static const std::string YieldSign(); // = "205"; // 让行标志
    static const std::string StopSign(); // = "206"; // 停止标志
    static const std::string MandatoryTurnDirection(); // = "209" // 强制转向方向
    static const std::string MandatoryLeftRightDirection(); // = "211"; // 强制左或右转
    static const std::string TwoChoiceTurnDirection(); // = "214" // 两个选择的转向方向
    static const std::string Roundabout(); // = "215"; // 环形交叉口
    static const std::string PassRightLeft(); // = "222"; // 右侧通过或左侧通过
    static const std::string AccessForbidden(); // = "250"; // 禁止进入
    static const std::string AccessForbiddenMotorvehicles(); // = "251"; // 禁止机动车辆进入
    static const std::string AccessForbiddenTrucks(); // = "253"; // 禁止卡车进入
    static const std::string AccessForbiddenBicycle(); // = "254"; // 禁止自行车进入
    static const std::string AccessForbiddenWeight(); // = "263"; // 禁止超重
    static const std::string AccessForbiddenWidth(); // = "264"; // 禁止超宽
    static const std::string AccessForbiddenHeight(); // = "265"; // 禁止超高
    static const std::string AccessForbiddenWrongDirection(); // = "267"; // 禁止逆行
    static const std::string ForbiddenUTurn(); // = "272"; // 禁止掉头
    static const std::string MaximumSpeed(); // = "274"; // 最高速度
    static const std::string ForbiddenOvertakingMotorvehicles(); // = "276"; // 禁止超越机动车辆
    static const std::string ForbiddenOvertakingTrucks(); // = "277"; // 禁止超越卡车
    static const std::string AbsoluteNoStop(); // = "283"; // 绝对禁止停车
    static const std::string RestrictedStop(); // = "286"; // 限制停车
    static const std::string HasWayNextIntersection(); // = "301"; // 下一个交叉口有优先通行权
    static const std::string PriorityWay(); // = "306"; // 优先通行路
    static const std::string PriorityWayEnd(); // = "307"; // 优先通行结束
    static const std::string CityBegin(); // = "310"; // 城市开始
    static const std::string CityEnd(); // = "311"; // 城市结束
    static const std::string Highway(); // = "330"; // 高速公路
    static const std::string DeadEnd(); // = "357"; // 死胡同
    static const std::string RecomendedSpeed(); // = "380"; // 推荐速度
    static const std::string RecomendedSpeedEnd(); // = "381"; // 推荐速度结束

    static bool IsTrafficLight(const std::string &type); // 判断是否为交通信号灯
  };

} // road
} // carla
