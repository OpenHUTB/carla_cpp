// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>
#include <vector>

namespace carla {

  /// 用于访问文件系统的静态函数。
  ///
  /// @warning 使用此文件需要链接 boost_filesystem。
  class FileSystem {
  public:

    ///在创建文件之前验证路径的方便函数。
    ///
    /// 1) 确保所有缺失的父目录被创建。
    /// 2)  如果 @a filepath 缺少扩展名，则将 @a default_extension
    ///      附加到路径上。
    static void ValidateFilePath(
        std::string &filepath,
        const std::string &default_extension = "");

    ///列出 @a folder_path 中匹配 @a wildcard_pattern 的常规文件
    /// （不递归）。
    ///
    /// @throw std::invalid_argument if folder does not exist.
    ///
    /// @todo Do permission check.
    static std::vector<std::string> ListFolder(
        const std::string &folder_path,
        const std::string &wildcard_pattern);
  };

} // namespace carla
