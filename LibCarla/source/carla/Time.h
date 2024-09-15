// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

��Ȼ�����������������ע�͵Ĵ��룺

```cpp
#pragma once // ��ֹͷ�ļ�����ΰ���

#include "carla/Debug.h" // ���������ص�ͷ�ļ�

#include <boost/date_time/posix_time/posix_time_types.hpp> // ���� Boost ��ʱ�䴦����

#include <chrono> // �����׼���ʱ�䴦����

namespace carla {

    /// ��ʾ�����ʱ�����ʱ�䣬��ȷ�����롣�Զ��� std::chrono::duration �� boost::posix_time::time_duration ֮��ת����
    class time_duration {
    public:

        /// ��̬����������ָ������������һ�� time_duration ʵ����
        static inline time_duration seconds(size_t timeout) {
            return std::chrono::seconds(timeout); // ʹ�� std::chrono::seconds ����ʱ�����ʱ��
        }

        /// ��̬����������ָ���ĺ���������һ�� time_duration ʵ����
        static inline time_duration milliseconds(size_t timeout) {
            return std::chrono::milliseconds(timeout); // ʹ�� std::chrono::milliseconds ����ʱ�����ʱ��
        }

        /// Ĭ�Ϲ��캯������ʱ�����ʱ���ʼ��Ϊ����롣
        constexpr time_duration() noexcept : _milliseconds(0u) {}

        /// ʹ�� std::chrono::duration ���캯��������ת��Ϊ time_duration��
        template <typename Rep, typename Period>
        time_duration(std::chrono::duration<Rep, Period> duration)
            : _milliseconds([=]() {
            // ��ʱ�����ʱ��ת��Ϊ���룬��ȷ�����ǷǸ���
            const auto count = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            DEBUG_ASSERT(count >= 0); // ȷ�������ǷǸ���
            return static_cast<size_t>(count); // ������ת��Ϊ size_t ���洢
                }()) {}

        /// ʹ�� Boost �� time_duration ���캯��������ת��Ϊ time_duration��
        time_duration(boost::posix_time::time_duration timeout)
            : time_duration(std::chrono::milliseconds(timeout.total_milliseconds())) {} // �� Boost �� time_duration ת��Ϊ����

        /// Ĭ�Ͽ������캯����
        time_duration(const time_duration&) = default;

        /// Ĭ�Ͽ�����ֵ�������
        time_duration& operator=(const time_duration&) = default;

        /// �� time_duration ת��Ϊ Boost �� time_duration��
        boost::posix_time::time_duration to_posix_time() const {
            return boost::posix_time::milliseconds(_milliseconds); // ������ת��Ϊ Boost �� time_duration
        }

        /// �� time_duration ת��Ϊ std::chrono::milliseconds ��ʱ�����ʱ�䡣
        constexpr auto to_chrono() const {
            return std::chrono::milliseconds(_milliseconds); // �����Ժ���Ϊ��λ��ʱ�����ʱ��
        }

        /// ת����������� time_duration ת��Ϊ Boost �� time_duration��
        operator boost::posix_time::time_duration() const {
            return to_posix_time(); // ת��Ϊ Boost �� time_duration
        }

        /// �����Ժ���Ϊ��λ��ʱ�����ʱ�䡣
        constexpr size_t milliseconds() const noexcept {
            return _milliseconds; // ���ش洢�ĺ�����
        }

    private:

        size_t _milliseconds; // �ڲ��洢��ʱ�����ʱ�䣬�Ժ���Ϊ��λ
    };

} // namespace carla
