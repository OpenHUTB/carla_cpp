// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 确保该头文件只被包含一次

#include "carla/image/ImageIOConfig.h"  // 包含图像输入输出配置的头文件

namespace carla {  // 定义命名空间 carla
namespace image {  // 定义命名空间 image

  class ImageIO {  // 定义 ImageIO 类
  public:

    // 静态模板函数，用于读取图像
    template <typename ImageT, typename IO = io::any>
    static void ReadImage(const std::string &in_filename, ImageT &image, IO = IO()) {
      IO::read_image(in_filename, image);  // 调用 IO 类的 read_image 方法读取图像
    }

    // 静态模板函数，用于写入图像视图
    template <typename ViewT, typename IO = io::any>
    static std::string WriteView(std::string out_filename, const ViewT &image_view, IO = IO()) {
      IO::write_view(out_filename, image_view);  // 调用 IO 类的 write_view 方法写入图像视图
      return out_filename;  // 返回输出文件名
    }
  };

} // namespace image
} // namespace carla
