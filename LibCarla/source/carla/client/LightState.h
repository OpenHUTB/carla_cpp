// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/sensor/data/Color.h"///< 引入CARLA传感器数据中的颜色类 
#include "carla/rpc/LightState.h"///< 引入CARLA RPC接口中的灯光状态类

namespace carla {
namespace client {
    /// @brief 别名定义，将sensor::data::Color命名空间下的Color类重命名为当前命名空间下的Color。
using Color = sensor::data::Color;
/// @brief 定义灯光ID的类型，使用uint32_t表示。 
using LightId = uint32_t;
/// @brief 表示车辆灯光状态的结构体。  
    /// @details 该结构体包含了灯光的强度、颜色、所属组以及是否激活等信息。
struct LightState {
    /// @brief 使用rpc::LightState::LightGroup作为灯光组的类型别名。
  using LightGroup = rpc::LightState::LightGroup;
  /// @brief 默认构造函数，初始化所有成员变量为默认值。
  LightState() {}
  /// @brief 构造函数，根据提供的参数初始化灯光状态。  
       /// @param intensity 灯光的强度，范围为0.0到1.0。  
       /// @param color 灯光的颜色，使用Color结构体表示。  
       /// @param group 灯光所属的组，使用LightGroup枚举表示。  
       /// @param active 表示灯光是否激活的布尔值。
  LightState(
      float intensity,///< 灯光的强度
      Color color,///< 灯光的颜色
      LightGroup group,///< 灯光所属的组
      bool active)///< 灯光是否激活
  : _intensity(intensity),
    _color(color),
    _group(group),
    _active(active){}
  /// @brief 重置灯光状态到默认值。 
  void Reset () {
    _intensity = 0.0f;///< 将灯光强度设置为0  
    _color.r = 0; ///< 将灯光颜色设置为黑色（红色分量）
    _color.g = 0;///< 将灯光颜色设置为黑色（绿色分量）
    _color.b = 0;///< 将灯光颜色设置为黑色（蓝色分量）
    _group = LightGroup::None;///< 将灯光组设置为None 
    _active = false;///< 将灯光设置为非激活状态
  }
  /// @brief 灯光的强度，范围0.0到1.0。
  float _intensity = 0.0f;
  /// @brief 灯光的颜色。
  Color _color;
  /// @brief 灯光所属的组。
  LightGroup _group = LightGroup::None;
  /// @brief 表示灯光是否激活的布尔值。
  bool _active = false;
};

} // namespace client
} // namespace carla
