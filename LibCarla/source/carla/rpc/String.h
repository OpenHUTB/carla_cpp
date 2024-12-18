// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "UnrealString.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

#ifdef LIBCARLA_INCLUDED_FROM_UE4

  // 从FString到std:string的快速转换
  // 与 FName 和 FText 不同，FString 可以与搜索、修改并且与其他字符串比较。
  // 不过，这些操作会导致 FString 的开销比不可变字符串类更大。
  static inline std::string FromFString(const FString &Str) {
    return TCHAR_TO_UTF8(*Str);
  }

  // 从std:string到FString的快速转换
  static inline FString ToFString(const std::string &str) {
    return FString(str.size(), UTF8_TO_TCHAR(str.c_str()));
  }

  constexpr size_t MaxStringLength = 5000000; // 500万

  // 从FString到长文本(std::string)更慢的转换
  static inline std::string FromLongFString(const FString &Str) {
    std::string result;
    size_t i = 0;
    while(i + MaxStringLength < Str.Len()) {
      auto Substr = Str.Mid(i, MaxStringLength);  // 取出Str中i开始，MaxStringLength结束的子串（下标从0开始）
      std::string temp_string = TCHAR_TO_UTF8(*Substr);
      result += temp_string; // 对于大于500万的字符串，每500万长度就附加到结果串中
      i += MaxStringLength;
    }
    auto Substr = Str.Mid(i, Str.Len() - i);  // 附上最后500万除不尽的子串
    std::string temp_string = TCHAR_TO_UTF8(*Substr);
    result += temp_string;
    return result;
  }

  // 从长文本(std:string)到FString更慢的转换
  static inline FString ToLongFString(const std::string &str) {
    FString result = "";
    for (size_t i = 0; i < str.size(); i++)
    {
      result += str[i];
    }
    return result;
  }

#endif // LIBCARLA_INCLUDED_FROM_UE4

} // namespace rpc
} // namespace carla
