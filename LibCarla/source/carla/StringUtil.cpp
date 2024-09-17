// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/StringUtil.h"

#ifdef _WIN32
#  include <shlwapi.h>
#else
#  include <fnmatch.h>
#endif // _WIN32

namespace carla {  // 检查字符串是否与模式匹配的函数

  bool StringUtil::Match(const char *str, const char *test) {
#ifdef _WIN32      // 在 Windows 平台上，使用 PathMatchSpecA 函数进行匹配
    return PathMatchSpecA(str, test);
#else
    return 0 == fnmatch(test, str, 0);
#endif // _WIN32
  }

} // namespace carla
