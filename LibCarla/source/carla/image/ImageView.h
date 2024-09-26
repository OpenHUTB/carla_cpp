// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 防止重复包含头文件

#include "carla/image/BoostGil.h" // 引入Boost.GIL库的头文件
#include "carla/image/ColorConverter.h" // 引入颜色转换器的头文件
#include "carla/sensor/data/Color.h" // 引入颜色数据的头文件
#include "carla/sensor/data/ImageTmpl.h" // 引入图像模板的头文件

#include <type_traits> // 引入类型特性相关的头文件

namespace carla {
namespace image {

  class ImageView { // 定义ImageView类
  private:

    // 定义灰度像素布局的类型，使用Boost.GIL库
    template <typename SrcViewT>
    using GrayPixelLayout = boost::gil::pixel<typename boost::gil::channel_type<SrcViewT>::type, boost::gil::gray_layout_t>;

    // 从传感器图像生成目标像素类型的视图
    template <typename DstPixelT, typename ImageT>
    static auto MakeViewFromSensorImage(ImageT &image) {
      using namespace boost::gil; // 引入Boost.GIL命名空间
      namespace sd = carla::sensor::data; // 简化命名空间引用
      static_assert(
          std::is_same<typename ImageT::pixel_type, sd::Color>::value, // 确保像素类型为Color
          "Invalid pixel type"); // 如果不是，则触发编译错误
      static_assert(
          sizeof(sd::Color) == sizeof(DstPixelT), // 确保像素大小相同
          "Invalid pixel size"); // 如果不是，则触发编译错误
      return interleaved_view( // 创建交错视图
          image.GetWidth(), // 图片宽度
          image.GetHeight(), // 图片高度
          reinterpret_cast<DstPixelT*>(image.data()), // 数据指针转换为目标像素类型
          sizeof(sd::Color) * image.GetWidth()); // 行长度（字节）
    }

  public:

    // 创建图像视图
    template <typename ImageT>
    static auto MakeView(ImageT &image) {
      return boost::gil::view(image); // 使用Boost.GIL创建视图
    }

    // 针对传感器图像类型创建视图
    static auto MakeView(sensor::data::ImageTmpl<sensor::data::Color> &image) {
      return MakeViewFromSensorImage<boost::gil::bgra8_pixel_t>(image); // 使用BGRA8像素类型
    }

    // 针对传感器图像类型创建常量视图
    static auto MakeView(const sensor::data::ImageTmpl<sensor::data::Color> &image) {
      return MakeViewFromSensorImage<boost::gil::bgra8c_pixel_t>(image); // 使用BGRA8C像素类型
    }

    // 创建颜色转换视图
    template <typename SrcViewT, typename DstPixelT, typename CC>
    static auto MakeColorConvertedView(const SrcViewT &src, CC cc) {
      return _MakeColorConvertedView<DstPixelT>(src, cc); // 调用私有方法进行颜色转换
    }

    // 创建颜色转换视图，默认目标像素类型为灰度布局
    template <typename SrcViewT, typename DstPixelT = GrayPixelLayout<SrcViewT>>
    static auto MakeColorConvertedView(const SrcViewT &src, ColorConverter::Depth cc) {
      return _MakeColorConvertedView<DstPixelT>(src, cc); // 调用私有方法进行深度颜色转换
    }

    // 创建颜色转换视图，使用对数深度
    template <typename SrcViewT, typename DstPixelT = GrayPixelLayout<SrcViewT>>
    static auto MakeColorConvertedView(const SrcViewT &src, ColorConverter::LogarithmicDepth) {
      auto intermediate_view = _MakeColorConvertedView<boost::gil::gray32f_pixel_t>(src, ColorConverter::Depth()); // 中间转换为32位灰度像素
      return _MakeColorConvertedView<DstPixelT>(intermediate_view, ColorConverter::LogarithmicLinear()); // 最终转换为目标像素类型
    }

    // 创建颜色转换视图，使用CityScapes调色板
    template <typename SrcViewT, typename DstPixelT = typename SrcViewT::value_type>
    static auto MakeColorConvertedView(const SrcViewT &src, ColorConverter::CityScapesPalette cc) {
      return _MakeColorConvertedView<DstPixelT>(src, cc); // 调用私有方法进行调色板转换
    }

  private:

    // 内部结构，用于定义颜色转换后的类型
    template <typename SrcView, typename DstP, typename CC>
    struct color_converted_type {
    private:
      typedef boost::gil::color_convert_deref_fn<typename SrcView::const_t::reference, DstP, CC> deref_t; // 定义解引用类型
      typedef typename SrcView::template add_deref<deref_t> add_ref_t; // 添加解引用
    public:
      typedef typename add_ref_t::type type; // 定义最终类型
      static type make(const SrcView &sv, CC cc) { return add_ref_t::make(sv, deref_t(cc)); } // 创建实例
    };

    // 私有方法，创建颜色转换视图
    template <typename DstPixelT, typename SrcViewT, typename CC>
    static auto _MakeColorConvertedView(const SrcViewT &src, CC cc) {
      return color_converted_type<SrcViewT, DstPixelT, CC>::make(src, cc); // 返回转换后的视图
    }
  };

} // namespace image
} // namespace carla
