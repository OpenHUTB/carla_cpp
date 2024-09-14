// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

  #pragma once // ��ֹͷ�ļ�����ΰ�������߱���Ч��

#include <chrono> // ���� chrono �⣬����ʱ���������
#include <cstdint> // ���� cstdint �⣬�ṩ��׼��������

namespace carla { // ���� carla �����ռ�
namespace detail { // ���� detail �����ռ䣬���ڷ�װʵ��ϸ��

  template <typename CLOCK> // ģ���࣬�����벻ͬ���͵�ʱ��
  class StopWatchTmpl {
    static_assert(CLOCK::is_steady, "The StopWatch's clock must be steady");
    // ��̬���ԣ�ȷ�������ʱ���������ȶ��ģ�steady��������ʱ���������

  public:
    using clock = CLOCK; // ���� clock Ϊ�����ʱ�����ͱ���

    StopWatchTmpl() 
      : _start(clock::now()), // ��ʼ����ʼʱ��Ϊ��ǰʱ��
        _end(), // ��ʼ������ʱ��ΪĬ��ֵ
        _is_running(true) {} // ��ʼ��Ϊ�����������״̬

    void Restart() {
      _is_running = true; // �������Ϊ����״̬
      _start = clock::now(); // ���¿�ʼʱ��Ϊ��ǰʱ��
    }

    void Stop() {
      _end = clock::now(); // ���½���ʱ��Ϊ��ǰʱ��
      _is_running = false; // �������Ϊֹͣ״̬
    }

    typename clock::duration GetDuration() const {
      // �������������У����ص�ǰʱ���뿪ʼʱ��Ĳ�ֵ
      // ���򣬷��ؽ���ʱ���뿪ʼʱ��Ĳ�ֵ
      return _is_running ? clock::now() - _start : _end - _start;
    }

    template <class RESOLUTION=std::chrono::milliseconds>
    size_t GetElapsedTime() const {
      // ��ʱ����ת��Ϊָ�����ȵ�ʱ�䵥λ��Ĭ��Ϊ���룩�������������ֵ
      return static_cast<size_t>(std::chrono::duration_cast<RESOLUTION>(GetDuration()).count());
    }

    bool IsRunning() const {
      return _is_running; // ��������Ƿ��������е�״̬
    }

  private:
    typename clock::time_point _start; // ���ʼʱ���
    typename clock::time_point _end;   // ������ʱ���
    bool _is_running;                  // ����Ƿ��������е�״̬
  };

} // namespace detail

  using StopWatch = detail::StopWatchTmpl<std::chrono::steady_clock>;
  // ʵ���� StopWatchTmpl ģ�壬ʹ�� std::chrono::steady_clock ��Ϊʱ�����ͣ����� StopWatch ����

} // namespace carla

