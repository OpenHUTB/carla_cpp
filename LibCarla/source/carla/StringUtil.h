// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/algorithm/string.hpp>

namespace carla {
// ������Ϊ StringUtil ���࣬�����ṩ�����ַ��������߷���
  class StringUtil {
  public:
// �� const char* ���͵��ַ���ת��Ϊ const char*��ֱ�ӷ����������
    static const char *ToConstCharPtr(const char *str) {
      return str;
    }
 // ����ģ�����͵��ַ���������ת��Ϊ const char*��ͨ�������� c_str() ����
    template <typename StringT>
    static const char *ToConstCharPtr(const StringT &str) {
      return str.c_str();
    }
// �ж����뷶Χ input �Ƿ��Է�Χ test ��ͷ
    template <typename Range1T, typename Range2T>
    static bool StartsWith(const Range1T &input, const Range2T &test) {
    	// ʹ�� boost::algorithm::istarts_with ���������ж�
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
// ����д��Χ���ַ���ת��ΪСд��ʽ
    template <typename SequenceT>
    static auto ToLowerCopy(const SequenceT &str) {
      return boost::algorithm::to_lower_copy(str);
    }
// ����д��Χ���ַ���ת��Ϊ��д��ʽ
    template <typename WritableRangeT>
    static void ToUpper(WritableRangeT &str) {
      boost::algorithm::to_upper(str);
    }
 
    template <typename SequenceT>
    static auto ToUpperCopy(const SequenceT &str) {
      return boost::algorithm::to_upper_copy(str);
    }
 // ȥ����д��Χ�ַ������˵Ŀհ��ַ�
    template <typename WritableRangeT>
    static void Trim(WritableRangeT &str) {
      boost::algorithm::trim(str);
    }
// ���ڲ���д��Χ���ַ���������ȥ�����˿հ��ַ���ĸ���
    template <typename SequenceT>
    static auto TrimCopy(const SequenceT &str) {
      return boost::algorithm::trim_copy(str);
    }
// ���ַ��� str ���շָ������� separators ���зָ����洢�� destination ������
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
