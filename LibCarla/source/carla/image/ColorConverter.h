// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 包含 BoostGil 和 CityScapesPalette 的头文件，可能用于图像处理和颜色转换操作
#include "carla/image/BoostGil.h"
#include "carla/image/CityScapesPalette.h"
// 命名空间 carla::image，用于组织与图像相关的功能
namespace carla {
namespace image {
// ColorConverter 类，可能用于实现各种颜色转换操作
  class ColorConverter {
  public:
// LogarithmicLinear 结构体，可能用于进行对数线性转换
    struct LogarithmicLinear {
		// 模板函数，对输入像素 src 进行对数线性转换，并将结果存储在输出像素 dst 中
      template <typename DstPixelT>
      void operator()(const boost::gil::gray32fc_pixel_t &src, DstPixelT &dst) const {
        using namespace boost::gil;
		// 根据输入像素值进行对数计算，得到一个中间值 value
        const float value = 1.0f + std::log(src[0u]) / 5.70378f;
		// 对中间值进行范围限制，确保在 0.005f 和 1.0f 之间，得到 clamped
        const float clamped = std::max(std::min(value, 1.0f), 0.005f);
		// 使用 color_convert 函数将 clamped 的灰度值转换为目标像素类型 dst
        color_convert(gray32fc_pixel_t{clamped}, dst);
      }
    };
// Depth 结构体，可能用于处理深度信息的转换
    struct Depth {
		// 模板函数，对输入像素 src 进行深度转换，并将结果存储在输出像素 dst 中
      template <typename SrcPixelT, typename DstPixelT>
      void operator()(const SrcPixelT &src, DstPixelT &dst) const {
        using namespace boost::gil;
        static_assert(
            sizeof(typename color_space_type<SrcPixelT>::type) == sizeof(uint8_t),
            "Invalid pixel type.");
			// 根据输入像素的红、绿、蓝通道值计算深度值 depth
        const float depth =
             get_color(src, red_t()) +
            (get_color(src, green_t()) * 256) +
            (get_color(src, blue_t())  * 256 * 256);
        const float normalized = depth / static_cast<float>(256 * 256 * 256 - 1);
        color_convert(gray32fc_pixel_t{normalized}, dst);
      }
    };

    struct LogarithmicDepth {};
 // CityScapesPalette 结构体，可能用于将像素转换为 CityScapes 调色板中的颜色
    struct CityScapesPalette {
      template <typename SrcPixelT, typename DstPixelT>
      void operator()(const SrcPixelT &src, DstPixelT &dst) const {
        using namespace boost::gil;
        static_assert(
            sizeof(typename color_space_type<SrcPixelT>::type) == sizeof(uint8_t),
            "Invalid pixel type.");
			// 根据输入像素的红通道值获取 CityScapesPalette 中的颜色值 color
        const auto color = image::CityScapesPalette::GetColor(get_color(src, red_t()));
		// 使用 color_convert 函数将获取的颜色值转换为目标像素类型 dst
        color_convert(rgb8c_pixel_t{color[0u], color[1u], color[2u]}, dst);
      }
    };
  };

} // namespace image
} // namespace carla
