// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/sensor/data/Color.h" // 包含颜色定义的头文件
#include "carla/sensor/data/ImageTmpl.h" // 包含图像模板定义的头文件

namespace carla {
namespace sensor {
namespace data {

  /// 定义32位BGRA颜色的图像
  using Image = ImageTmpl<Color>;
  
  /// 定义浮点型BGRA颜色的图像
  using FloatImage = ImageTmpl<rpc::FloatColor>;

  /// 定义64位BGRA颜色的图像，使用16位通道
  using OpticalFlowImage = ImageTmpl<OpticalFlowPixel>;

  /// 定义32位BGRA颜色的图像，用于法线图
  using NormalsImage = ImageTmpl<Color>;

} // namespace data
} // namespace sensor
} // namespace carla // 结束命名空间
