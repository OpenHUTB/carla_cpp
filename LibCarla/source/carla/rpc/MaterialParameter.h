// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>

namespace carla {
namespace rpc {

// 定义一个枚举类 MaterialParameter，用于表示不同的材料参数
enum class MaterialParameter
{
  // 表示纹理法线
  Tex_Normal,
  // 表示环境光遮蔽/粗糙度/金属度/自发光纹理
  Tex_Ao_Roughness_Metallic_Emissive,
  // 表示漫反射纹理
  Tex_Diffuse,
  // 表示自发光纹理
  Tex_Emissive
};

// 声明一个函数，将 MaterialParameter 枚举类型转换为字符串
std::string MaterialParameterToString(MaterialParameter material_parameter);

} // namespace rpc
} // namespace carla

// 为 carla::rpc::MaterialParameter 枚举添加序列化支持
MSGPACK_ADD_ENUM(carla::rpc::MaterialParameter);
