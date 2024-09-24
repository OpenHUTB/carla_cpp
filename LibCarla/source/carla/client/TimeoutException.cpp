// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/TimeoutException.h" // ����TimeoutExceptionͷ�ļ�

namespace carla { // ����carla�����ռ�
namespace client { // ����client�������ռ�

  using namespace std::string_literals; // ʹ���ַ����������������ռ�

  TimeoutException::TimeoutException( // TimeoutException���캯��
      const std::string &endpoint, // ����������˵��ַ
      time_duration timeout) // �����������ʱʱ��
    : std::runtime_error( // ��ʼ������std::runtime_error
        "time-out of "s + std::to_string(timeout.milliseconds()) + // ����������Ϣ��������ʱʱ��
        "ms while waiting for the simulator, " // ��ʾ��Ϣ���ȴ�ģ����ʱ������ʱ
        "make sure the simulator is ready and connected to " + endpoint) {} // ��ʾ�û�ȷ��ģ�����Ѿ��������ӵ�ָ���˵�

} // namespace client
} // namespace carla
