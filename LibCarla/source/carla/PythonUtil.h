// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"   // 引入NonCopyable头文件

#ifdef LIBCARLA_WITH_PYTHON_SUPPORT  // 如果启用了Python支持
#  if defined(__clang__)  // 如果使用clang编译器
#    pragma clang diagnostic push  // 推送当前的诊断状态
#    pragma clang diagnostic ignored "-Wdeprecated-register"  // 忽略过时注册器的警告
#  endif
#    include <boost/python.hpp>   // 引入Boost.Python库
#  if defined(__clang__)  // 如果使用clang编译器
#    pragma clang diagnostic pop  // 恢复之前的诊断状态
#  endif
#endif // LIBCARLA_WITH_PYTHON_SUPPORT

namespace carla {   // 定义carla命名空间

  class PythonUtil {
  public:

    static bool ThisThreadHasTheGIL() {  // 检查当前线程是否拥有GIL
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT   // 如果启用了Python支持
#  if PY_MAJOR_VERSION >= 3   // 如果Python版本大于等于3
      return PyGILState_Check();   返回GIL状态检查结果
#  else
      PyThreadState *tstate = _PyThreadState_Current;  // 获取当前线程状态
      return (tstate != nullptr) && (tstate == PyGILState_GetThisThreadState());   // 检查状态是否匹配
#  endif // PYTHON3
#else    如果未启用Python支持
      return false;
#endif // LIBCARLA_WITH_PYTHON_SUPPORT
    }

#ifdef LIBCARLA_WITH_PYTHON_SUPPORT

    /// 获取Python全局解释器锁上的锁，这是从其他线程调用Python代码所必需的。
    class AcquireGIL : private NonCopyable {
    public:

      AcquireGIL() : _state(PyGILState_Ensure()) {}

      ~AcquireGIL() {
        PyGILState_Release(_state);
      }

    private:

      PyGILState_STATE _state;
    };

    /// 释放Python的全局解释器锁，在执行阻塞I/O操作时使用它。
    class ReleaseGIL : private NonCopyable {
    public:

      ReleaseGIL() : _state(PyEval_SaveThread()) {}

      ~ReleaseGIL() {
        PyEval_RestoreThread(_state);
      }

    private:

      PyThreadState *_state;
    };

#else // LIBCARLA_WITH_PYTHON_SUPPORT

    class AcquireGIL : private NonCopyable {};
    class ReleaseGIL : private NonCopyable {};

#endif // LIBCARLA_WITH_PYTHON_SUPPORT

    /// 可以传递给智能指针的删除器，以便在销毁对象之前获取GIL。
    class AcquireGILDeleter {
    public:

      template <typename T>
      void operator()(T *ptr) const {
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
        if (ptr != nullptr && !PythonUtil::ThisThreadHasTheGIL()) {
          AcquireGIL lock;
          delete ptr;
        } else
#endif // LIBCARLA_WITH_PYTHON_SUPPORT
        delete ptr;
      }
    };

    /// 可以传递给智能指针的删除器，以便在销毁对象之前释放GIL。
    class ReleaseGILDeleter {
    public:

      template <typename T>
      void operator()(T *ptr) const {
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
        if (ptr != nullptr && PythonUtil::ThisThreadHasTheGIL()) {
          ReleaseGIL lock;
          delete ptr;
        } else
#endif // LIBCARLA_WITH_PYTHON_SUPPORT
        delete ptr;
      }
    };
  };

} // namespace carla
