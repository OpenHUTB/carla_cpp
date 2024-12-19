// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "FileTransfer.h" // 引入FileTransfer.h头文件，该文件包含文件传输功能的声明
#include "carla/Version.h" // 引入carla版本信息头文件，用于获取当前Carla的版本

namespace carla {
namespace client {

  // 根据操作系统设置文件存储的基础文件夹路径
  #ifdef _WIN32
        std::string FileTransfer::_filesBaseFolder = std::string(getenv("USERPROFILE")) + "/carlaCache/";
  #else
        std::string FileTransfer::_filesBaseFolder = std::string(getenv("HOME")) + "/carlaCache/";
  #endif
  // 设置文件传输的基文件夹路径，确保路径以斜杠结尾
  bool FileTransfer::SetFilesBaseFolder(const std::string &path) {
    if (path.empty()) return false;

    // 如果路径不以斜杠结尾，自动添加斜杠
    if (path[path.size() - 1] != '/' && path[path.size() - 1] != '\\') {
      _filesBaseFolder = path + "/";
  }

    return true;
  }

  // 获取当前的文件基础路径
  const std::string& FileTransfer::GetFilesBaseFolder() {
    return _filesBaseFolder;
  }

  // 检查指定的文件是否存在
  bool FileTransfer::FileExists(std::string file) {
    // 构建文件的完整路径
    struct stat buffer;
    std::string fullpath = _filesBaseFolder;
    fullpath += "/";
    fullpath += ::carla::version(); // 加入当前的Carla版本号
    fullpath += "/";
    fullpath += file; // 添加目标文件名

    // 使用 stat 函数检查文件是否存在
    return (stat(fullpath.c_str(), &buffer) == 0);
  }

  // 将内容写入指定路径的文件
  bool FileTransfer::WriteFile(std::string path, std::vector<uint8_t> content) {
    // 构建文件的完整路径
    std::string writePath = _filesBaseFolder;
    writePath += "/";
    writePath += ::carla::version(); // 加入当前的Carla版本号
    writePath += "/";
    writePath += path; // 添加目标文件的路径

    // 验证文件路径并创建所需的目录
    carla::FileSystem::ValidateFilePath(writePath);

    // 以二进制模式打开文件，如果文件不存在则创建
    std::ofstream out(writePath, std::ios::trunc | std::ios::binary);
    if(!out.good()) return false;

    // 将内容写入文件
    for(auto file : content) {
          out << file;
    }
    out.close();

    return true;
  }

  // 读取指定路径的文件内容，并返回一个字节向量
  std::vector<uint8_t> FileTransfer::ReadFile(std::string path) {
    // 构建文件的完整路径
    std::string fullpath = _filesBaseFolder;
    fullpath += "/";
    fullpath += ::carla::version(); // 加入当前的Carla版本号
    fullpath += "/";
    fullpath += path; // 添加目标文件路径
    // 从文件中读取内容并返回字节向量
    std::ifstream file(fullpath, std::ios::binary);
    std::vector<uint8_t> content(std::istreambuf_iterator<char>(file), {});
    return content;
  }

} // namespace client
} // namespace carla
