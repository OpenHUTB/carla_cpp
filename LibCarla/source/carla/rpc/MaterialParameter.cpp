// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MaterialParameter.h"

namespace carla {
namespace rpc {

// 函数：将 MaterialParameter 枚举类型转换为对应的字符串表示
std::string MaterialParameterToString(MaterialParameter material_parameter)
{
  // 使用 switch 语句根据不同的 MaterialParameter 枚举值进行分支判断
  switch(material_parameter)
  {
    // 当枚举值为 Tex_Normal 时，返回字符串 "Normal"
    case MaterialParameter::Tex_Normal:                         return "Normal";
    // 当枚举值为 Tex_Ao_Roughness_Metallic_Emissive 时，返回字符串 "AO / Roughness / Metallic / Emissive"
    case MaterialParameter::Tex_Ao_Roughness_Metallic_Emissive: return "AO / Roughness / Metallic / Emissive";
    // 当枚举值为 Tex_Diffuse 时，返回字符串 "Diffuse"
    case MaterialParameter::Tex_Diffuse:                      return "Diffuse";
    // 当枚举值为 Tex_Emissive 时，返回字符串 "Emissive"
    case MaterialParameter::Tex_Emissive:                       return "Emissive";
    // 对于未匹配到上述枚举值的情况，返回字符串 "Invalid"
    default:                                                    return "Invalid";
  }
}

}
}
