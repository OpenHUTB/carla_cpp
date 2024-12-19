// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "FileTransfer.h"
#include "carla/Version.h"

namespace carla {
namespace client {

  #ifdef _WIN32
        std::string FileTransfer::_filesBaseFolder = std::string(getenv("USERPROFILE")) + "/carlaCache/";
  #else
        std::string FileTransfer::_filesBaseFolder = std::string(getenv("HOME")) + "/carlaCache/";
  #endif
  //设置文件传输的基文件夹路径
  bool FileTransfer::SetFilesBaseFolder(const std::string &path) {
    // 首先判断传入的路径字符串是否为空。如果path为空字符串（即没有实际内容，长度为0）
    // 那就意味着传入的路径不合法或者不符合预期，无法进行有效的基础文件夹路径设置操作
    // 所以此时函数直接返回false，表示设置失败
    if (path.empty()) return false;

    // 检查路径是否以斜线结尾，否则添加
    if (path[path.size() - 1] != '/' && path[path.size() - 1] != '\\') {
      _filesBaseFolder = path + "/";
  }

    return true;
  }

  const std::string& FileTransfer::GetFilesBaseFolder() {
    return _filesBaseFolder;
  }

  bool FileTransfer::FileExists(std::string file) {
    // 检查文件是否存在
    struct stat buffer;
    std::string fullpath = _filesBaseFolder;
    fullpath += "/";
    fullpath += ::carla::version();
    fullpath += "/";
    fullpath += file;

    return (stat(fullpath.c_str(), &buffer) == 0);
  }

  bool FileTransfer::WriteFile(std::string path, std::vector<uint8_t> content) {
    std::string writePath = _filesBaseFolder;
    writePath += "/";
    writePath += ::carla::version();
    writePath += "/";
    writePath += path;

    // 验证并创建文件路径
    carla::FileSystem::ValidateFilePath(writePath);

    // 以二进制模式打开文件并截断
    std::ofstream out(writePath, std::ios::trunc | std::ios::binary);
    if(!out.good()) return false;

    // 写下内容并关闭
    for(auto file : content) {
          out << file;
    }
    out.close();

    return true;
  }
  //取文件内容并返回一个字节向量
  std::vector<uint8_t> FileTransfer::ReadFile(std::string path) {
    std::string fullpath = _filesBaseFolder;
    fullpath += "/";
    fullpath += ::carla::version();
    fullpath += "/";
    fullpath += path;
    // 从基础文件夹读取二进制文件
    std::ifstream file(fullpath, std::ios::binary);
    std::vector<uint8_t> content(std::istreambuf_iterator<char>(file), {});
    return content;
  }

} // namespace client
} // namespace carla
