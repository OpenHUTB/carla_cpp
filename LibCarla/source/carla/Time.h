// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

<<<<<<< HEAD
#pragma once // ȷ��ͷ�ļ�ֻ������һ��

#include "carla/Debug.h" // �����Զ���ĵ��Թ���

#include <boost/date_time/posix_time/posix_time_types.hpp> // ���� Boost �� POSIX ʱ������
=======
#pragma once// ��ֹͷ�ļ�����ΰ���

#include "carla/Debug.h"// ���������ص�ͷ�ļ�

#include <boost/date_time/posix_time/posix_time_types.hpp>// ���� Boost ��ʱ�䴦����
>>>>>>> 4abc25176ac5d5930e03168b4e3d67d59cc132e7

#include <chrono> // ������׼���е�ʱ�䴦��֧��

namespace carla {

  /// ��ʱ�����ʱ�䣬��ȷ�����롣�����Զ��� std::chrono::duration �� boost::posix_time::time_duration ֮��ת����
  class time_duration {
  public:
// ��̬��Ա���������ڴ���һ����ʾָ��������ʱ����
    static inline time_duration seconds(size_t timeout) {
      // ����һ����ʼ��Ϊָ�������� time_duration ����
      return std::chrono::seconds(timeout);
    }
// ��̬��Ա���������ڴ���һ����ʾָ����������ʱ����
    static inline time_duration milliseconds(size_t timeout) {
      // ����һ����ʼ��Ϊָ���������� time_duration ����
      return std::chrono::milliseconds(timeout);
    }
// Ĭ�Ϲ��캯��������һ����ʾ 0 �����ʱ����
    constexpr time_duration() noexcept : _milliseconds(0u) {}
      // Ĭ�Ϲ��캯����������ʱ���ʼ��Ϊ 0 ����

// ģ�幹�캯��������һ�� std::chrono::duration ���͵Ĳ���������ת��Ϊ�Ժ���Ϊ��λ��ʱ����
    template <typename Rep, typename Period>
    time_duration(std::chrono::duration<Rep, Period> duration)
      : _milliseconds([=]() {
<<<<<<< HEAD
          // ������ std::chrono::duration ת��Ϊ����
=======
      	// ������� duration ת��Ϊ�������������ж���ȷ������Ǹ�
>>>>>>> 4abc25176ac5d5930e03168b4e3d67d59cc132e7
          const auto count = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
          DEBUG_ASSERT(count >= 0); // ���Գ���ʱ���ǷǸ���
          return static_cast<size_t>(count); // ת��Ϊ size_t ����
        }()) {}
// ���캯��������һ�� boost::posix_time::time_duration ���͵Ĳ���������ת��Ϊ�Ժ���Ϊ��λ��ʱ����
    time_duration(boost::posix_time::time_duration timeout)
      : time_duration(std::chrono::milliseconds(timeout.total_milliseconds())) {}
<<<<<<< HEAD
      // �� Boost �� time_duration ��ʼ��������ת��Ϊ����

    time_duration(const time_duration &) = default;
    time_duration &operator=(const time_duration &) = default;
      // Ĭ�ϵĸ��ƹ��캯���͸�ֵ�����

=======
// �������캯����ʹ��Ĭ��ʵ��
    time_duration(const time_duration &) = default;
    time_duration &operator=(const time_duration &) = default;
// ����ǰʱ����ת��Ϊ boost::posix_time::time_duration ����
>>>>>>> 4abc25176ac5d5930e03168b4e3d67d59cc132e7
    boost::posix_time::time_duration to_posix_time() const {
      // ���ڲ��ĺ���ת��Ϊ Boost �� time_duration
      return boost::posix_time::milliseconds(_milliseconds);
    }
// ����ǰʱ����ת��Ϊ std::chrono::milliseconds ����
    constexpr auto to_chrono() const {
      // ���ڲ�����ʱ����Ϊ std::chrono::milliseconds ����
      return std::chrono::milliseconds(_milliseconds);
    }
// ����ת�������������ǰ����ת��Ϊ boost::posix_time::time_duration ����
    operator boost::posix_time::time_duration() const {
      // ��ʽת�������������ڲ�����ʱ��ת��Ϊ Boost �� time_duration
      return to_posix_time();
    }

    constexpr size_t milliseconds() const noexcept {
      // ���س���ʱ��ĺ�����
      return _milliseconds;
    }

  private:
<<<<<<< HEAD

    size_t _milliseconds; // ���ڴ洢ʱ�����ʱ��ĺ�����
=======
// �洢ʱ�����ĺ�����
    size_t _milliseconds;
>>>>>>> 4abc25176ac5d5930e03168b4e3d67d59cc132e7
  };

} // namespace carla
