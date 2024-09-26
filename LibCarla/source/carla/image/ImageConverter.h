// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 确保头文件只被包含一次

#include "carla/image/ImageView.h" // 引入ImageView头文件

namespace carla { // carla命名空间
namespace image { // image子命名空间

  class ImageConverter { // 定义ImageConverter类
  public:

    template <typename SrcViewT, typename DstViewT> // 模板函数，接受源视图和目标视图类型
    static void CopyPixels(const SrcViewT &src, DstViewT &dst) { // 静态成员函数，复制像素
      boost::gil::copy_pixels(src, dst); // 使用Boost.GIL库复制像素
    }

    template <typename ColorConverter, typename MutableImageView> // 模板函数，接受颜色转换器和可变图像视图类型
    static void ConvertInPlace( // 静态成员函数，原地转换图像
        MutableImageView &image_view, // 可变图像视图引用
        ColorConverter converter = ColorConverter()) { // 默认颜色转换器
      using DstPixelT = typename MutableImageView::value_type; // 获取目标像素类型
      CopyPixels( // 调用CopyPixels函数
          ImageView::MakeColorConvertedView<MutableImageView, DstPixelT>(image_view, converter), // 创建颜色转换后的视图
          image_view); // 目标为原始图像视图
    }
  };

} // namespace image
} // namespace carla
