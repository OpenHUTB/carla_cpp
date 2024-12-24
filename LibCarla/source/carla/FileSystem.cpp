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
// 验证并修正文件路径
  void FileSystem::ValidateFilePath(std::string &filepath, const std::string &ext) {
    // 引用传递的文件路径字符串
    // 创建一个fs::path对象pat
    // 初始化为filepath
    fs::path path(filepath);
    if (path.extension().empty() && !ext.empty()) {
      if (ext[0] != '.') {
        path += '.'; // 在ext前加上'.'
      }
      path += ext; // 将ext添加到path
    }
    auto parent = path.parent_path(); // 获取path的父目录parent
    if (!parent.empty()) {
      fs::create_directories(parent);h  // 创建父目录
    }
    filepath = path.string();将修正后的路径转换回字符串并赋值给filepath
  }
// 函数：ListFolder
// 作用：列出指定文件夹下符合特定通配符模式的文件列表
  std::vector<std::string> FileSystem::ListFolder(
      const std::string &folder_path,
      const std::string &wildcard_pattern) {
      	// 将输入的文件夹路径转换为 boost::filesystem::path 类型的对象
    fs::path root(folder_path);
    // 如果root不存在或不是一个目录，抛出异常
    if (!fs::exists(root) || !fs::is_directory(root)) {
      throw_exception(std::invalid_argument(folder_path + ": no such folder"));
    }
// 创建一个字符串向量results，用于存储匹配的文件名
    std::vector<std::string> results;
    fs::directory_iterator end;// 使用fs::directory_iterator遍历目录中的每个文件和子目录
    for (fs::directory_iterator it(root); it != end; ++it) {
      if (fs::is_regular_file(*it)) {
        const std::string filename = it->path().filename().string();// 对于每个常规文件，获取其文件名filename
        if (StringUtil::Match(filename, wildcard_pattern)) {
          results.emplace_back(filename);// 返回匹配的文件名列表results
        }
      }
    }
    return results;
  }

} // namespace carla
