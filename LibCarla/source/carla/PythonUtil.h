// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h" // 引入非拷贝构造基类的头文件

#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
#  if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wdeprecated-register"
#  endif
#    include <boost/python.hpp> // 引入Boost Python库的头文件
#  if defined(__clang__)
#    pragma clang diagnostic pop
#  endif
#endif // LIBCARLA_WITH_PYTHON_SUPPORT

namespace carla {

  class PythonUtil {
  public:

    static bool ThisThreadHasTheGIL() {
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
#  if PY_MAJOR_VERSION >= 3
      return PyGILState_Check(); // Python 3: 检查当前线程是否持有GIL
#  else
      PyThreadState *tstate = _PyThreadState_Current;
      return (tstate != nullptr) && (tstate == PyGILState_GetThisThreadState()); // Python 2: 检查当前线程是否持有GIL
#  endif // PYTHON3
#else
      return false; // 如果没有Python支持，返回false
#endif // LIBCARLA_WITH_PYTHON_SUPPORT
    }

#ifdef LIBCARLA_WITH_PYTHON_SUPPORT

    /// 获取Python的全局解释器锁 (GIL)，用于确保线程安全
    class AcquireGIL : private NonCopyable {
    public:

      AcquireGIL() : _state(PyGILState_Ensure()) {} // 构造函数中确保当前线程持有GIL

      ~AcquireGIL() {
        PyGILState_Release(_state); // 析构函数中释放GIL
      }

    private:

      PyGILState_STATE _state; // 存储GIL状态
    };

    /// 释放Python的全局解释器锁 (GIL)，用于允许其他线程执行Python代码
    class ReleaseGIL : private NonCopyable {
    public:

      ReleaseGIL() : _state(PyEval_SaveThread()) {} // 构造函数中保存当前线程状态并释放GIL

      ~ReleaseGIL() {
        PyEval_RestoreThread(_state); // 析构函数中恢复线程状态
      }

    private:

      PyThreadState *_state; // 存储线程状态
    };

#else // LIBCARLA_WITH_PYTHON_SUPPORT

    class AcquireGIL : private NonCopyable {}; // 没有Python支持时，类为空
    class ReleaseGIL : private NonCopyable {}; // 没有Python支持时，类为空

#endif // LIBCARLA_WITH_PYTHON_SUPPORT

    /// 删除Python对象时，自动管理GIL状态
    class AcquireGILDeleter {
    public:

      template <typename T>
      void operator()(T *ptr) const {
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
        if (ptr != nullptr && !PythonUtil::ThisThreadHasTheGIL()) {
          AcquireGIL lock; // 如果当前线程未持有GIL，则获取GIL
          delete ptr; // 删除对象
        } else
#endif // LIBCARLA_WITH_PYTHON_SUPPORT
        delete ptr; // 删除对象
      }
    };

    /// 删除Python对象时，自动管理GIL状态
    class ReleaseGILDeleter {
    public:

      template <typename T>
      void operator()(T *ptr) const {
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
        if (ptr != nullptr && PythonUtil::ThisThreadHasTheGIL()) {
          ReleaseGIL lock; // 如果当前线程持有GIL，则释放GIL
          delete ptr; // 删除对象
        } else
#endif // LIBCARLA_WITH_PYTHON_SUPPORT
        delete ptr; // 删除对象
      }
    };
  };

} // namespace carla
