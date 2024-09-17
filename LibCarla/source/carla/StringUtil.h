// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/algorithm/string.hpp>

namespace carla {
// 定义名为 StringUtil 的类，用于提供各种字符串处理工具方法
  class StringUtil {
  public:
// 将 const char* 类型的字符串转换为 const char*，直接返回输入参数
    static const char *ToConstCharPtr(const char *str) {
      return str;
    }
 // 对于模板类型的字符串，将其转换为 const char*，通过调用其 c_str() 方法
    template <typename StringT>
    static const char *ToConstCharPtr(const StringT &str) {
      return str.c_str();
    }
// 判断输入范围 input 是否以范围 test 开头
    template <typename Range1T, typename Range2T>
    static bool StartsWith(const Range1T &input, const Range2T &test) {
    	// 使用 boost::algorithm::istarts_with 函数进行判断
      return boost::algorithm::istarts_with(input, test);
    }

    template <typename Range1T, typename Range2T>
    static bool EndsWith(const Range1T &input, const Range2T &test) {
      return boost::algorithm::iends_with(input, test);
    }

    template <typename WritableRangeT>
    static void ToLower(WritableRangeT &str) {
      boost::algorithm::to_lower(str);
    }
// 将可写范围的字符串转换为小写形式
    template <typename SequenceT>
    static auto ToLowerCopy(const SequenceT &str) {
      return boost::algorithm::to_lower_copy(str);
    }
// 将可写范围的字符串转换为大写形式
    template <typename WritableRangeT>
    static void ToUpper(WritableRangeT &str) {
      boost::algorithm::to_upper(str);
    }
 
    template <typename SequenceT>
    static auto ToUpperCopy(const SequenceT &str) {
      return boost::algorithm::to_upper_copy(str);
    }
 // 去除可写范围字符串两端的空白字符
    template <typename WritableRangeT>
    static void Trim(WritableRangeT &str) {
      boost::algorithm::trim(str);
    }
// 对于不可写范围的字符串，返回去除两端空白字符后的副本
    template <typename SequenceT>
    static auto TrimCopy(const SequenceT &str) {
      return boost::algorithm::trim_copy(str);
    }
// 将字符串 str 按照分隔符集合 separators 进行分割，结果存储在 destination 容器中
    template<typename Container, typename Range1T, typename Range2T>
    static void Split(Container &destination, const Range1T &str, const Range2T &separators) {
      boost::split(destination, str, boost::is_any_of(separators));
    }

    /// Match @a str with the Unix shell-style @a wildcard_pattern.
    static bool Match(const char *str, const char *wildcard_pattern);

    /// Match @a str with the Unix shell-style @a wildcard_pattern.
    template <typename String1T, typename String2T>
    static bool Match(const String1T &str, const String2T &wildcard_pattern) {
      return Match(ToConstCharPtr(str), ToConstCharPtr(wildcard_pattern));
    }
  };

} // namespace carla
