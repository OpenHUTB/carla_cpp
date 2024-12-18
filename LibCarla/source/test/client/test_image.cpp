// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/image/ImageConverter.h>
#include <carla/image/ImageIO.h>
#include <carla/image/ImageView.h>

#include <memory>

template <typename ViewT, typename PixelT>
struct TestImage {
  TestImage(TestImage &&) = default;
  // 定义像素类型别名
  using pixel_type = PixelT;
  std::unique_ptr<PixelT[]> data;
  ViewT view;
};

template <typename PixelT>
static auto MakeTestImage(size_t width, size_t height) {
	// 创建一个动态分配的像素数组
  auto data = std::make_unique<PixelT[]>(sizeof(PixelT) * width * height);
  // 创建一个交错视图，用于访问图像数据
  auto view = boost::gil::interleaved_view(
      width,
      height,
      reinterpret_cast<PixelT*>(data.get()),
      static_cast<long>(sizeof(PixelT) * width));
  return TestImage<decltype(view), PixelT>{std::move(data), view};
}
// 在非Windows平台下执行的测试用例，用于测试图像格式支持
#ifndef PLATFORM_WINDOWS
TEST(image, support){
  using namespace carla::image::io;
  // 记录PNG格式是否支持
  carla::logging::log("PNG  support =", has_png_support());
  // 记录JPEG格式是否支持
  carla::logging::log("JPEG support =", has_jpeg_support());
  // 记录TIFF格式是否支持
  carla::logging::log("TIFF support =", has_tiff_support());
}
#endif // PLATFORM_WINDOWS
// 测试图像深度相关功能的测试用例
TEST(image, depth) {
#ifndef NDEBUG
// 在调试模式下输出提示信息，表示该测试在发布模式下执行（因为在调试模式下太慢）
  carla::log_info("This test only happens in release (too slow).");
#else
  using namespace boost::gil;
  using namespace carla::image;
// 定义图像宽度和高度的常量
  constexpr auto width = 256 * 256 * 256;
  constexpr auto height = 1u;
// 创建一个bgra8像素类型的测试图像
  auto img_bgra8 = MakeTestImage<bgra8_pixel_t>(width, height);
  // 创建一个gray8像素类型的测试图像
  auto img_gray8 = MakeTestImage<gray8_pixel_t>(width, height);
 // 根据bgra8图像视图创建一个深度转换后的视图
  auto depth_view = ImageView::MakeColorConvertedView(
      img_bgra8.view,
      ColorConverter::Depth());
       // 根据bgra8图像视图创建一个对数深度转换后的视图
  auto ldepth_view = ImageView::MakeColorConvertedView(
      img_bgra8.view,
      ColorConverter::LogarithmicDepth());
 // 获取深度视图的第一个像素，用于类型检查
  auto p = *depth_view.begin();
  // 静态断言，确保像素类型符合预期
  static_assert(std::is_same<decltype(p), gray8_pixel_t>::value, "Not the pixel I was looking for!");
  // 获取对数深度视图的第一个像素，用于类型检查
  auto lp = *ldepth_view.begin();
  // 静态断言，确保像素类型符合预期
  static_assert(std::is_same<decltype(lp), gray8_pixel_t>::value, "Not the pixel I was looking for!");
// 断言深度视图和原始bgra8图像视图的大小相同
  ASSERT_EQ(depth_view.size(), img_bgra8.view.size());
   // 断言对数深度视图和原始bgra8图像视图的大小相同
  ASSERT_EQ(ldepth_view.size(), img_bgra8.view.size());
// 填充bgra8图像和gray8图像的数据
  {
    auto it_bgra8 = img_bgra8.view.begin();
    auto it_gray8 = img_gray8.view.begin();

    for (auto r = 0u; r < 256u; ++r) {
      for (auto g = 0u; g < 256u; ++g) {
        for (auto b = 0u; b < 256u; ++b) {
        	// 获取bgra8图像当前像素的引用
          decltype(img_bgra8)::pixel_type &p_bgra8 = *it_bgra8;
          // 获取gray8图像当前像素的引用
          decltype(img_gray8)::pixel_type &p_gray8 = *it_gray8;
          // 设置bgra8图像当前像素的红色通道值
          get_color(p_bgra8, red_t()) = static_cast<uint8_t>(r);
          // 设置bgra8图像当前像素的绿色通道值
          get_color(p_bgra8, green_t()) = static_cast<uint8_t>(g);
          // 设置bgra8图像当前像素的蓝色通道值
          get_color(p_bgra8, blue_t()) = static_cast<uint8_t>(b);
          // 计算深度值
          const float depth = r + (g * 256) + (b  * 256 * 256);
          // 计算归一化深度值
          const float normalized = depth / static_cast<float>(256 * 256 * 256 - 1);
          // 设置gray8图像当前像素的值
          p_gray8[0] = static_cast<uint8_t>(255.0 * normalized);

          ++it_bgra8;
          ++it_gray8;
        }
      }
    }
  }
// 创建一个bgra8像素类型的图像副本
  auto img_copy = MakeTestImage<bgra8_pixel_t>(width, height);
   // 将原始bgra8图像的数据复制到副本中
  ImageConverter::CopyPixels(img_bgra8.view, img_copy.view);
   // 在副本图像上进行对数深度转换
  ImageConverter::ConvertInPlace(img_copy.view, ColorConverter::LogarithmicDepth());
  // 比较原始gray8图像、深度视图、对数深度视图和副本图像的数据

  {
    auto it_gray8 = img_gray8.view.begin();
    auto it_depth = depth_view.begin();
    auto it_ldepth = ldepth_view.begin();
    auto it_copy = img_copy.view.begin();

    for (auto i = 0u; i < width; ++i) {
      auto p_gray8 = *it_gray8;
      auto p_depth = *it_depth;
      auto p_ldepth = *it_ldepth;
      auto p_copy = *it_copy;
      // 断言深度视图和gray8图像在当前位置的像素值相近
      ASSERT_NEAR(int(p_depth[0]), int(p_gray8[0]), 1)
          << "at XY(" << i << ",0)";
      decltype(p_copy) ld;
      color_convert(p_ldepth, ld);
      // 断言对数深度视图转换后和副本图像在当前位置的像素值相等
      ASSERT_EQ(ld, p_copy)
          << "at XY(" << i << ",0)";
      ++it_gray8;
      ++it_depth;
      ++it_ldepth;
      ++it_copy;
    }
  }
#endif // NDEBUG
}
// 测试图像语义分割相关功能的测试用例
TEST(image, semantic_segmentation) {
  using namespace boost::gil;
  using namespace carla::image;
// 定义图像宽度为CityScapesPalette标签数量，高度为1
  constexpr auto width = CityScapesPalette::GetNumberOfTags();
  constexpr auto height = 1u;
// 创建一个bgra8像素类型的测试图像
  auto img_bgra8 = MakeTestImage<bgra8_pixel_t>(width, height);
  // 创建一个rgb8像素类型的测试图像
  auto img_ss = MakeTestImage<rgb8_pixel_t>(width, height);
// 根据bgra8图像视图创建一个语义分割转换后的视图
  auto semseg_view = ImageView::MakeColorConvertedView(
      img_bgra8.view,
      ColorConverter::CityScapesPalette());
// 获取语义分割视图的第一个像素，用于类型检查
  auto p = *semseg_view.begin();
  // 静态断言，确保像素类型符合预期
  static_assert(std::is_same<decltype(p), bgra8_pixel_t>::value, "Not the pixel I was looking for!");
// 断言语义分割视图和原始bgra8图像视图的大小相同
  ASSERT_EQ(semseg_view.size(), img_bgra8.view.size());
 // 填充bgra8图像和rgb8图像的数据
  {
    auto it_bgra8 = img_bgra8.view.begin();
    auto it_ss = img_ss.view.begin();

    for (auto tag = 0u; tag < width; ++tag) {
      decltype(img_bgra8)::pixel_type &p_bgra8 = *it_bgra8;
      get_color(p_bgra8, red_t()) = static_cast<uint8_t>(tag);
      get_color(p_bgra8, green_t()) = 0u;
      get_color(p_bgra8, blue_t()) = 0u;
      decltype(img_ss)::pixel_type &p_ss = *it_ss;
      auto color = CityScapesPalette::GetColor(static_cast<uint8_t>(tag));
      get_color(p_ss, red_t()) =  color[0u];
      get_color(p_ss, green_t()) = color[1u];
      get_color(p_ss, blue_t()) = color[2u];
      ++it_bgra8;
      ++it_ss;
    }
  }
// 创建一个rgba8像素类型的图像副本
  auto img_copy = MakeTestImage<rgba8_pixel_t>(width, height);
  // 将原始bgra8图像的数据复制到副本中
  ImageConverter::CopyPixels(img_bgra8.view, img_copy.view);
  // 在副本图像上进行语义分割转换
  ImageConverter::ConvertInPlace(img_copy.view, ColorConverter::CityScapesPalette());
// 比较原始rgb8图像、语义分割视图和副本图像的数据
  {
    auto it_ss = img_ss.view.begin();
    auto it_ssv = semseg_view.begin();
    auto it_copy = img_copy.view.begin();

    for (auto i = 0u; i < width; ++i) {
      auto p_ssv = *it_ssv;
      auto p_copy = *it_copy;
      auto _p_ss = *it_ss;
      decltype(p_ssv) p_ss;
      color_convert(_p_ss, p_ss);
      // 断言语义分割视图和转换后的rgb8图像在当前位置的像素值相等
      ASSERT_EQ(p_ssv, p_ss)
          << "at XY(" << i << ",0)";
      decltype(p_copy) css;
      color_convert(p_ssv, css);
      // 断言语义分割视图和副本图像在当前 instance:1:510
      ASSERT_EQ(p_ssv, p_copy)
          << "at XY(" << i << ",0)";
      ++it_ss;
      ++it_ssv;
      ++it_copy;
    }
  }
}
