// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 确保头文件只被包含一次

#include "carla/FileSystem.h"  // 引入文件系统相关的头文件
#include "carla/Logging.h"     // 引入日志记录相关的头文件
#include "carla/StringUtil.h"  // 引入字符串工具相关的头文件
#include "carla/image/BoostGil.h"  // 引入Boost.GIL库的图像处理功能

// 检测是否支持PNG格式
#ifndef LIBCARLA_IMAGE_WITH_PNG_SUPPORT
#  if defined(__has_include) && __has_include("png.h")  // 检查是否能找到png.h头文件
#    define LIBCARLA_IMAGE_WITH_PNG_SUPPORT true  // 定义为支持
#  else
#    define LIBCARLA_IMAGE_WITH_PNG_SUPPORT false // 定义为不支持
#  endif
#endif

// 检测是否支持JPEG格式
#ifndef LIBCARLA_IMAGE_WITH_JPEG_SUPPORT
#  if defined(__has_include) && __has_include("jpeglib.h")  // 检查是否能找到jpeglib.h头文件
#    define LIBCARLA_IMAGE_WITH_JPEG_SUPPORT true  // 定义为支持
#  else
#    define LIBCARLA_IMAGE_WITH_JPEG_SUPPORT false // 定义为不支持
#  endif
#endif

// 检测是否支持TIFF格式
#ifndef LIBCARLA_IMAGE_WITH_TIFF_SUPPORT
#  if defined(__has_include) && __has_include("tiffio.h")  // 检查是否能找到tiffio.h头文件
#    define LIBCARLA_IMAGE_WITH_TIFF_SUPPORT true  // 定义为支持
#  else
#    define LIBCARLA_IMAGE_WITH_TIFF_SUPPORT false // 定义为不支持
#  endif
#endif

#if defined(__clang__)  // 如果使用的是Clang编译器
#  pragma clang diagnostic push  // 保存当前的诊断状态
#  pragma clang diagnostic ignored "-Wunused-parameter"  // 忽略未使用的参数警告
#endif

#if LIBCARLA_IMAGE_WITH_PNG_SUPPORT == true  // 如果支持PNG格式
#  ifndef png_infopp_NULL
#    define png_infopp_NULL (png_infopp)NULL  // 定义PNG信息指针为NULL
#  endif // png_infopp_NULL
#  ifndef int_p_NULL
#    define int_p_NULL (int*)NULL  // 定义整数指针为NULL
#  endif // int_p_NULL
#  if defined(__clang__)  // 如果使用的是Clang编译器
#    pragma clang diagnostic push  // 保存当前的诊断状态
#    pragma clang diagnostic ignored "-Wignored-qualifiers"  // 忽略类型限定符被忽略的警告
#    pragma clang diagnostic ignored "-Wparentheses"  // 忽略括号相关的警告
#  endif
#    include <boost/gil/extension/io/png.hpp>  // 引入Boost.GIL的PNG输入输出扩展
#  if defined(__clang__)  // 如果使用的是Clang编译器
#    pragma clang diagnostic pop  // 恢复之前的诊断状态
#  endif
#endif

#if LIBCARLA_IMAGE_WITH_JPEG_SUPPORT == true  // 如果支持JPEG格式
#  include <boost/gil/extension/io/jpeg.hpp>  // 引入Boost.GIL的JPEG输入输出扩展
#endif

#if LIBCARLA_IMAGE_WITH_TIFF_SUPPORT == true  // 如果支持TIFF格式
#  include <boost/gil/extension/io/tiff.hpp>  // 引入Boost.GIL的TIFF输入输出扩展
#endif

#if defined(__clang__)  // 如果使用的是Clang编译器
#  pragma clang diagnostic pop  // 恢复之前的诊断状态
#endif

namespace carla {  // 定义命名空间carla
namespace image {  // 定义命名空间image
namespace io {  // 定义命名空间io

  constexpr bool has_png_support() {  // 检查是否支持PNG格式
    return LIBCARLA_IMAGE_WITH_PNG_SUPPORT;  // 返回支持状态
  }

  constexpr bool has_jpeg_support() {  // 检查是否支持JPEG格式
    return LIBCARLA_IMAGE_WITH_JPEG_SUPPORT;  // 返回支持状态
  }

  constexpr bool has_tiff_support() {  // 检查是否支持TIFF格式
    return LIBCARLA_IMAGE_WITH_TIFF_SUPPORT;  // 返回支持状态
  }

  static_assert(has_png_support() || has_jpeg_support() || has_tiff_support(),
      "No image format supported, please compile with at least one of "
      "LIBCARLA_IMAGE_WITH_PNG_SUPPORT, LIBCARLA_IMAGE_WITH_JPEG_SUPPORT, "
      "or LIBCARLA_IMAGE_WITH_TIFF_SUPPORT");  // 检查至少支持一种图像格式

namespace detail {  // 定义命名空间detail

  template <typename ViewT, typename IOTag>  // 模板结构体，接受视图类型和输入输出标签
  struct is_write_supported {  // 定义写入支持结构体
    static constexpr bool value = boost::gil::is_write_supported<typename boost::gil::get_pixel_type<ViewT>::type, IOTag>::value;  // 判断写入是否支持
  };

  struct io_png {  // 定义PNG输入输出结构体
#if LIBCARLA_IMAGE_WITH_PNG_SUPPORT // 如果支持PNG格式

    static constexpr const char *get_default_extension() { // 获取默认扩展名
      return "png"; // 返回“png”作为默认扩展名
    }

    template <typename Str>
    static bool match_extension(const Str &str) { // 匹配文件扩展名
      return StringUtil::EndsWith(str, get_default_extension()); // 判断字符串是否以默认扩展名结尾
    }

    template <typename Str, typename ImageT>
    static void read_image(Str &&in_filename, ImageT &image) { // 读取图像
      boost::gil::read_and_convert_image(std::forward<Str>(in_filename), image, boost::gil::png_tag()); // 使用boost库读取并转换PNG图像
    }

    template <typename Str, typename ViewT>
    static void write_view(Str &&out_filename, const ViewT &view) { // 写入视图到文件
      boost::gil::write_view(std::forward<Str>(out_filename), view, boost::gil::png_tag()); // 使用boost库写入PNG视图
    }

#endif // LIBCARLA_IMAGE_WITH_PNG_SUPPORT // 结束PNG支持条件编译

  };

  struct io_jpeg { // JPEG输入输出结构体

    static constexpr bool is_supported = has_jpeg_support(); // 检查是否支持JPEG格式

#if LIBCARLA_IMAGE_WITH_JPEG_SUPPORT // 如果支持JPEG格式

    static constexpr const char *get_default_extension() { // 获取默认扩展名
      return "jpeg"; // 返回“jpeg”作为默认扩展名
    }

    template <typename Str>
    static bool match_extension(const Str &str) { // 匹配文件扩展名
      return StringUtil::EndsWith(str, get_default_extension()) || // 判断字符串是否以默认扩展名结尾
             StringUtil::EndsWith(str, "jpg"); // 判断字符串是否以“jpg”结尾
    }

    template <typename Str, typename ImageT>
    static void read_image(Str &&in_filename, ImageT &image) { // 读取图像
      boost::gil::read_image(std::forward<Str>(in_filename), image, boost::gil::jpeg_tag()); // 使用boost库读取JPEG图像
    }

    template <typename Str, typename ViewT>
    static typename std::enable_if<is_write_supported<ViewT, boost::gil::jpeg_tag>::value>::type
    write_view(Str &&out_filename, const ViewT &view) { // 写入视图到文件（支持的情况）
      boost::gil::write_view(std::forward<Str>(out_filename), view, boost::gil::jpeg_tag()); // 使用boost库写入JPEG视图
    }

    template <typename Str, typename ViewT>
    static typename std::enable_if<!is_write_supported<ViewT, boost::gil::jpeg_tag>::value>::type
    write_view(Str &&out_filename, const ViewT &view) { // 写入视图到文件（不支持的情况）
      boost::gil::write_view(
          std::forward<Str>(out_filename),
          boost::gil::color_converted_view<boost::gil::rgb8_pixel_t>(view), // 将视图转换为RGB8像素格式
          boost::gil::jpeg_tag()); // 使用boost库写入JPEG视图
    }
#endif // LIBCARLA_IMAGE_WITH_JPEG_SUPPORT
  };

  struct io_tiff {

    static constexpr bool is_supported = has_tiff_support();

#if LIBCARLA_IMAGE_WITH_TIFF_SUPPORT

    static constexpr const char *get_default_extension() {
      return "tiff";
    }

    template <typename Str>
    static bool match_extension(const Str &str) {
      return StringUtil::EndsWith(str, get_default_extension());
    }

    template <typename Str, typename ImageT>
    static void read_image(Str &&in_filename, ImageT &image) {
      boost::gil::read_and_convert_image(std::forward<Str>(in_filename), image, boost::gil::tiff_tag());
    }

    template <typename Str, typename ViewT>
    static typename std::enable_if<is_write_supported<ViewT, boost::gil::tiff_tag>::value>::type
    write_view(Str &&out_filename, const ViewT &view) {
      boost::gil::write_view(std::forward<Str>(out_filename), view, boost::gil::tiff_tag());
    }

    template <typename Str, typename ViewT>
    static typename std::enable_if<!is_write_supported<ViewT, boost::gil::tiff_tag>::value>::type
    write_view(Str &&out_filename, const ViewT &view) {
      boost::gil::write_view(
          std::forward<Str>(out_filename),
          boost::gil::color_converted_view<boost::gil::rgb8_pixel_t>(view),
          boost::gil::tiff_tag());
    }

#endif // LIBCARLA_IMAGE_WITH_TIFF_SUPPORT
  };

  struct io_resolver {

    template <typename IO, typename Str>
    static typename std::enable_if<IO::is_supported, bool>::type match_extension(const Str &str) {
      return IO::match_extension(str);
    }

    template <typename IO, typename Str>
    static typename std::enable_if<!IO::is_supported, bool>::type match_extension(const Str &) {
      return false;
    }

    template <typename IO, typename Str, typename... Args>
    static typename std::enable_if<IO::is_supported>::type read_image(const Str &path, Args &&... args) {
      log_debug("reading", path, "as", IO::get_default_extension());
      IO::read_image(path, std::forward<Args>(args)...);
    }

    template <typename IO, typename... Args>
    static typename std::enable_if<!IO::is_supported>::type read_image(Args &&...) {
      DEBUG_ASSERT(false);
    }

    template <typename IO, typename... Args>
    static typename std::enable_if<IO::is_supported>::type write_view(std::string &path, Args &&... args) {
      FileSystem::ValidateFilePath(path, IO::get_default_extension());
      log_debug("writing", path, "as", IO::get_default_extension());
      IO::write_view(path, std::forward<Args>(args)...);
    }

    template <typename IO, typename... Args>
    static typename std::enable_if<!IO::is_supported>::type write_view(Args &&...) {
      DEBUG_ASSERT(false);
    }
  };

  template <typename... IOs>
  struct io_impl;

  template <typename IO>
  struct io_impl<IO> {
    constexpr static bool is_supported = IO::is_supported;

    template <typename... Args>
    static void read_image(Args &&... args) {
      io_resolver::read_image<IO>(std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void write_view(Args &&... args) {
      io_resolver::write_view<IO>(std::forward<Args>(args)...);
    }

    template <typename Str, typename... Args>
    static bool try_read_image(const Str &filename, Args &&... args) {
      if (io_resolver::match_extension<IO>(filename)) {
        io_resolver::read_image<IO>(filename, std::forward<Args>(args)...);
        return true;
      }
      return false;
    }

    template <typename Str, typename... Args>
    static bool try_write_view(Str &filename, Args &&... args) {
      if (io_resolver::match_extension<IO>(filename)) {
        io_resolver::write_view<IO>(filename, std::forward<Args>(args)...);
        return true;
      }
      return false;
    }
  };

  template <typename IO, typename... IOs>
  struct io_impl<IO, IOs...> {
  private:
    using self = io_impl<IO>;
    using recursive = io_impl<IOs...>;
  public:

    constexpr static bool is_supported = self::is_supported || recursive::is_supported;

    template <typename... Args>
    static void read_image(Args &... args) {
      if (!recursive::try_read_image(args...)) {
        self::read_image(args...);
      }
    }

    template <typename... Args>
    static bool try_read_image(Args &... args) {
      return recursive::try_read_image(args...) || self::try_read_image(args...);
    }

    template <typename... Args>
    static void write_view(Args &... args) {
      if (!recursive::try_write_view(args...)) {
        self::write_view(args...);
      }
    }

    template <typename... Args>
    static bool try_write_view(Args &... args) {
      return recursive::try_write_view(args...) || self::try_write_view(args...);
    }
  };

  template <typename DefaultIO, typename... IOs>
  struct io_any : detail::io_impl<DefaultIO, IOs...> {
    static_assert(DefaultIO::is_supported, "Default IO needs to be supported.");
  };

} // namespace detail

  struct png : detail::io_impl<detail::io_png> {};

  struct jpeg : detail::io_impl<detail::io_jpeg> {};

  struct tiff : detail::io_impl<detail::io_tiff> {};

#if LIBCARLA_IMAGE_WITH_PNG_SUPPORT

  struct any : detail::io_any<detail::io_png, detail::io_tiff, detail::io_jpeg> {};

#elif LIBCARLA_IMAGE_WITH_TIFF_SUPPORT

  struct any : detail::io_any<detail::io_tiff, detail::io_jpeg> {};

#else // Then for sure this one is available.

  struct any : detail::io_any<detail::io_jpeg> {};

#endif

} // namespace io
} // namespace image
} // namespace carla
