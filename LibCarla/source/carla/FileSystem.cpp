// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 引入 carla 库中的文件系统相关的头文件
#include "carla/FileSystem.h"

// 引入 carla 库中的异常处理头文件
#include "carla/Exception.h"
// 引入 carla 库中的字符串处理工具头文件
#include "carla/StringUtil.h"

// 引入 boost 文件系统库的操作相关头文件
#include <boost/filesystem/operations.hpp>

// 定义命名空间 carla
namespace carla {

// 引入 boost 文件系统库命名空间，并起别名 fs
namespace fs = boost::filesystem;

// 函数：ValidateFilePath
// 作用：验证文件路径，确保路径有扩展名，并创建路径的父目录（如果需要）
  void FileSystem::ValidateFilePath(std::string &filepath, const std::string &ext) {
    fs::path path(filepath);
    if (path.extension().empty() && !ext.empty()) {
      if (ext[0] != '.') {
        path += '.';
      }
      path += ext;
    }
    auto parent = path.parent_path();
    if (!parent.empty()) {
      fs::create_directories(parent);
    }
    filepath = path.string();
  }
// 函数：ListFolder
// 作用：列出指定文件夹下符合特定通配符模式的文件列表
  std::vector<std::string> FileSystem::ListFolder(
      const std::string &folder_path,
      const std::string &wildcard_pattern) {
      	// 将输入的文件夹路径转换为 boost::filesystem::path 类型的对象
    fs::path root(folder_path);
    if (!fs::exists(root) || !fs::is_directory(root)) {
      throw_exception(std::invalid_argument(folder_path + ": no such folder"));
    }

    std::vector<std::string> results;
    fs::directory_iterator end;
    for (fs::directory_iterator it(root); it != end; ++it) {
      if (fs::is_regular_file(*it)) {
        const std::string filename = it->path().filename().string();
        if (StringUtil::Match(filename, wildcard_pattern)) {
          results.emplace_back(filename);
        }
      }
    }
    return results;
  }

} // namespace carla
