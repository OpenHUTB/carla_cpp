// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"
#include "carla/geom/Rotation.h"
#include "carla/rpc/Color.h"

namespace carla {
namespace rpc {

using LightId = uint32_t;

class LightState {
public:

  using flag_type = uint8_t;

  enum class LightGroup : flag_type {
    None = 0,
    Vehicle,
    Street,
    Building,
    Other
  };

  LightState() {}

  #用于表示某种灯光状态相关信息
  LightState(
      geom::Location location,
      float intensity,
      LightGroup group,
      Color color,
      bool active)
  : _location(location),
    _intensity(intensity),
    _group(static_cast<flag_type>(group)),
    _color(color),
    _active(active) {}

  #定义了一个名为_location的geom::Location变量
  geom::Location _location;
  #定义了一个名为_intensity的float类型变量
  float _intensity = 0.0f;
  #定义了一个名为_id的LightId类型变量
  LightId _id;
  #定义了一个名为_group的flag_type变量
  flag_type _group = static_cast<flag_type>(LightGroup::None);
  #定义了一个 名为_color的Color变量
  Color _color;
  #定义了一个名为_active的bool类型变量
  bool _active = false;

  #使用宏来定义一个数组
  MSGPACK_DEFINE_ARRAY(_id, _location, _intensity, _group, _color, _active);

};

} // namespace rpc
} // namespace carla
