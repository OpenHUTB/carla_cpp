// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/FileSystem.h"   // 引入CARLA客户端传感器的头文件

#include <fstream>  // 引入文件流库
#include <iostream>  // 引入输入输出流库
#include <string>  // 引入字符串库
#include <sys/stat.h>  // 引入用于文件状态的系统调用库
#include <cstdint>   // 引入标准整数类型库

namespace carla {    // 定义carla命名空间
namespace client {   // 定义client命名空间


  class FileTransfer {   // 定义FileTransfer类

  public:

    FileTransfer() = delete;   // 禁止使用默认构造函数

    static bool SetFilesBaseFolder(const std::string &path);   // 设置文件基础目录，返回是否成功

    static const std::string& GetFilesBaseFolder();   // 获取文件基础目录的常量引用

    static bool FileExists(std::string file);    // 检查文件是否存在，返回布尔值

    static bool WriteFile(std::string path, std::vector<uint8_t> content);    // 写入文件，返回是否成功

    static std::vector<uint8_t> ReadFile(std::string path);   // 读取文件内容，返回字节向量

  private:

    static std::string _filesBaseFolder;   // 存储文件基础目录的静态变量

  };

} // namespace client
} // namespace carla
