// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// ��ֹͷ�ļ�����ΰ���

#include "carla/Debug.h"// ���������ص�ͷ�ļ�

#include <boost/date_time/posix_time/posix_time_types.hpp>// ���� Boost ��ʱ�䴦����

#include <chrono>

namespace carla {

  /// Positive time duration up to milliseconds resolution. Automatically casts
  /// between std::chrono::duration and boost::posix_time::time_duration.
  class time_duration {
  public:
// ��̬��Ա���������ڴ���һ����ʾָ��������ʱ����
    static inline time_duration seconds(size_t timeout) {
      return std::chrono::seconds(timeout);
    }
// ��̬��Ա���������ڴ���һ����ʾָ����������ʱ����
    static inline time_duration milliseconds(size_t timeout) {
      return std::chrono::milliseconds(timeout);
    }
// Ĭ�Ϲ��캯��������һ����ʾ 0 �����ʱ����
    constexpr time_duration() noexcept : _milliseconds(0u) {}

// ģ�幹�캯��������һ�� std::chrono::duration ���͵Ĳ���������ת��Ϊ�Ժ���Ϊ��λ��ʱ����
    template <typename Rep, typename Period>
    time_duration(std::chrono::duration<Rep, Period> duration)
      : _milliseconds([=]() {
      	// ������� duration ת��Ϊ�������������ж���ȷ������Ǹ�
          const auto count = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
          DEBUG_ASSERT(count >= 0);
          return static_cast<size_t>(count);
        }()) {}
// ���캯��������һ�� boost::posix_time::time_duration ���͵Ĳ���������ת��Ϊ�Ժ���Ϊ��λ��ʱ����
    time_duration(boost::posix_time::time_duration timeout)
      : time_duration(std::chrono::milliseconds(timeout.total_milliseconds())) {}
// �������캯����ʹ��Ĭ��ʵ��
    time_duration(const time_duration &) = default;
    time_duration &operator=(const time_duration &) = default;
// ����ǰʱ����ת��Ϊ boost::posix_time::time_duration ����
    boost::posix_time::time_duration to_posix_time() const {
      return boost::posix_time::milliseconds(_milliseconds);
    }
// ����ǰʱ����ת��Ϊ std::chrono::milliseconds ����
    constexpr auto to_chrono() const {
      return std::chrono::milliseconds(_milliseconds);
    }
// ����ת�������������ǰ����ת��Ϊ boost::posix_time::time_duration ����
    operator boost::posix_time::time_duration() const {
      return to_posix_time();
    }

    constexpr size_t milliseconds() const noexcept {
      return _milliseconds;
    }

  private:
// �洢ʱ�����ĺ�����
    size_t _milliseconds;
  };

} // namespace carla
