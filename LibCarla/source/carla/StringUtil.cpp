// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 引入字符串工具的头文件
#include "carla/StringUtil.h"
// 根据操作系统选择包含的头文件
#ifdef _WIN32
#  include <shlwapi.h>    // Windows 特定的头文件，用于路径匹配
#else
#  include <fnmatch.h>   // POSIX 标准的头文件，用于模式匹配
#endif // _WIN32

namespace carla {
// 实现字符串匹配功能
  bool StringUtil::Match(const char *str, const char *test) {
#ifdef _WIN32
// 在 Windows 上使用 PathMatchSpecA 函数进行字符串匹配
    return PathMatchSpecA(str, test);
#else
// 在其他平台上使用 fnmatch 函数进行模式匹配
    return 0 == fnmatch(test, str, 0);
#endif // _WIN32
  }

} // namespace carla
