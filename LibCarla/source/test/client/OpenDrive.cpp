// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "OpenDrive.h"

// 检测宏 LIBCARLA_TEST_CONTENT_FOLDER 是否被定义，如果没有被定义，则给出错误提示
#ifndef LIBCARLA_TEST_CONTENT_FOLDER
#  error Please define LIBCARLA_TEST_CONTENT_FOLDER.
#endif

#include <carla/FileSystem.h>//包含一个名为FileSystem.h的头文件。这个头文件可能是carla库中与文件系统操作相关的定义文件。

#include <fstream>//包含C++标准库中的<fstream>头文件。
#include <streambuf>//包含C++标准库中的<streambuf>头文件。

namespace util {

  // 获取宏定义目录下的OpenDrive目录下所有后缀为xodr的文件名并存储到向量列表中
  std::vector<std::string> OpenDrive::GetAvailableFiles() {
    return carla::FileSystem::ListFolder(
        LIBCARLA_TEST_CONTENT_FOLDER "/OpenDrive/",
        "*.xodr");
  }

  // 加载获取宏定义目录下的OpenDrive目录下指定文件的内容为字符串
  std::string OpenDrive::Load(const std::string &filename) {
    const std::string opendrive_folder = LIBCARLA_TEST_CONTENT_FOLDER "/OpenDrive/";
    std::ifstream file(opendrive_folder + filename);
    return std::string{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
  }

} // namespace util
