// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 只包含一次该头文件的指令。

#include "carla/image/ImageView.h" // 引入 ImageView 头文件。

namespace carla {  // 定义 carla 命名空间。
namespace image {  // 定义 image 命名空间。

  class ImageConverter {  // 定义图像转换器类。
  public:   // 公有访问修饰符。

      // 模板函数，用于复制像素。
    template <typename SrcViewT, typename DstViewT>
    static void CopyPixels(const SrcViewT &src, DstViewT &dst) {
      boost::gil::copy_pixels(src, dst); // 使用 boost::gil 库复制源视图的像素到目标视图。
    }

     // 模板函数，用于就地转换图像。
    template <typename ColorConverter, typename MutableImageView>
    static void ConvertInPlace(
        MutableImageView &image_view, // 可变图像视图引用。
        ColorConverter converter = ColorConverter()) { // 颜色转换器的默认值为 ColorConverter()。
      using DstPixelT = typename MutableImageView::value_type;  // 获取可变图像视图的像素类型。
      CopyPixels(  // 调用 CopyPixels 函数。
          ImageView::MakeColorConvertedView<MutableImageView, DstPixelT>(image_view, converter),
          image_view);  // 将转换后的视图复制回原始图像视图。
    }
  };

} // namespace image
} // namespace carla
