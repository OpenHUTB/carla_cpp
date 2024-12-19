// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//g++ -o output_file source_file.cpp -finput-charset=UTF-8 -fexec-charset=UTF-8
// 此文件采用 UTF-8 编码，以适应多语言字符集的处理需求。
#pragma once

#include <boost/algorithm/string.hpp>

namespace carla {
// 定义名为 StringUtil 的类，用于提供各种字符串处理工具方法
  class StringUtil {
  public:
// 将 const char* 类型的字符串转换为 const char*，直接返回输入参数。 
 // 在 UTF-8 编码环境下，确保对原始 const char* 字符串的正确引用。
    static const char *ToConstCharPtr(const char *str) {
      return str;
    }
 // 对于模板类型的字符串，将其转换为 const char*，通过调用其 c_str() 方法。 
 // UTF-8 编码中，此方法可处理不同类型的字符串并转换为 const char*，方便后续操作。 
    template <typename StringT>
    static const char *ToConstCharPtr(const StringT &str) {
      return str.c_str();
    }
// 判断输入范围 input 是否以范围 test 开头
// 在 UTF-8 编码下，能够准确判断包含多语言字符的字符串是否以特定子串开头。
    template <typename Range1T, typename Range2T>
    static bool StartsWith(const Range1T &input, const Range2T &test) {// ʹ��Boost�㷨���е�istarts_with������ִ�в����ִ�Сд�Ŀ�ͷƥ���顣
      return boost::algorithm::istarts_with(input, test);
    }// ����һ��ģ�庯�������ڼ��һ����Χ�Ƿ�����һ����Χ��β�������ִ�Сд

    template <typename Range1T, typename Range2T>
    static bool EndsWith(const Range1T &input, const Range2T &test) {// ʹ��Boost�㷨���е�iends_with������ִ�в����ִ�Сд�Ľ�βƥ���顣
      return boost::algorithm::iends_with(input, test);
    }// ����һ��ģ�庯�������ڽ���д��Χ�е������ַ�ת��ΪСд��

    template <typename WritableRangeT>
    static void ToLower(WritableRangeT &str) {// ʹ��Boost�㷨���е�to_lower����������Χ�е������ַ�ת��ΪСд��
      boost::algorithm::to_lower(str);
    }
// 将可写范围的字符串转换为小写形式
// UTF-8 编码下，为不可修改的字符串提供小写形式的副本，适用于多语言环境。
    template <typename SequenceT>
    static auto ToLowerCopy(const SequenceT &str) {
      return boost::algorithm::to_lower_copy(str);
    }
// 将可写范围的字符串转换为大写形式
// UTF-8 编码中，对不同语言字符的字符串进行大写转换操作。
    template <typename WritableRangeT>
    static void ToUpper(WritableRangeT &str) {
      boost::algorithm::to_upper(str);
    }
 // 将传入的字符串序列转换为大写副本并返回
    template <typename SequenceT>
    static auto ToUpperCopy(const SequenceT &str) {
      return boost::algorithm::to_upper_copy(str);
    }
 // 去除可写范围字符串两端的空白字符
 // UTF-8 编码环境下，有效去除多语言字符串两端的空白字符。
    template <typename WritableRangeT>
    static void Trim(WritableRangeT &str) {
      boost::algorithm::trim(str);
    }
// 对于不可写范围的字符串，返回去除两端空白字符后的副本
// 在 UTF-8 编码中，为不可修改的字符串提供去除空白后的副本，适用于各种语言。
    template <typename SequenceT>
    static auto TrimCopy(const SequenceT &str) {
      return boost::algorithm::trim_copy(str);
    }
// 将字符串 str 按照分隔符集合 separators 进行分割，结果存储在 destination 容器中
// UTF-8 编码下，能正确分割包含多语言字符的字符串。
    template<typename Container, typename Range1T, typename Range2T>
    static void Split(Container &destination, const Range1T &str, const Range2T &separators) {
      boost::split(destination, str, boost::is_any_of(separators));
    }

    /// Match @a str with the Unix shell-style @a wildcard_pattern.
    // 匹配 str 与 Unix shell 风格的通配符模式 wildcard_pattern。
    // 在 UTF-8 编码中，尝试对不同语言字符的字符串进行通配符匹配。
    static bool Match(const char *str, const char *wildcard_pattern);

    /// Match @a str with the Unix shell-style @a wildcard_pattern.
    // 模板版本的 Match 方法，对于不同类型的字符串进行转换后调用底层的 Match 方法。
    // 在 UTF-8 编码场景下，确保不同类型字符串能正确与通配符模式匹配。
    template <typename String1T, typename String2T>
    static bool Match(const String1T &str, const String2T &wildcard_pattern) {
      return Match(ToConstCharPtr(str), ToConstCharPtr(wildcard_pattern));
    }
  };

} // namespace carla
