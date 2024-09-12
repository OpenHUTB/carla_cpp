// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <chrono>
#include <cstdint>

namespace carla {
namespace detail {

  template <typename CLOCK>
  class StopWatchTmpl {
    static_assert(CLOCK::is_steady, "The StopWatch's clock must be steady");
  public:

    using clock = CLOCK;

    StopWatchTmpl() : _start(clock::now()), _end(), _is_running(true) {}

            // _start ����ʼ��Ϊ��ǰʱ��㣬��ʾ���ʼ��ʱ
            // _end ��ʼ��ΪĬ��ֵ����ʾ���δֹͣ
            // _is_running ����Ϊ true����ʾ�����������
    void Restart() {
      _is_running = true;
      // ����ʼʱ�����Ϊ��ǰʱ���
      _start = clock::now();
    }

    void Stop() {
    	// ������ʱ�����Ϊ��ǰʱ���
      _end = clock::now();
      _is_running = false;
    }

    typename clock::duration GetDuration() const {
    	// �������������У����ص�ǰʱ���뿪ʼʱ��Ĳ�ֵ
      return _is_running ? clock::now() - _start : _end - _start;
    }

    template <class RESOLUTION=std::chrono::milliseconds>
    size_t GetElapsedTime() const {
      return static_cast<size_t>(std::chrono::duration_cast<RESOLUTION>(GetDuration()).count());
    }

    bool IsRunning() const {
      return _is_running;
    }

  private:

    typename clock::time_point _start;

    typename clock::time_point _end;

    bool _is_running;
  };

} // namespace detail

  using StopWatch = detail::StopWatchTmpl<std::chrono::steady_clock>;

} // namespace carla
