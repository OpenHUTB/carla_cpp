// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被重复包含

#include "carla/NonCopyable.h"  // 包含Carla的非可复制类定义，可能用于防止类的实例被复制 

#ifdef LIBCARLA_WITH_PYTHON_SUPPORT  // 检查是否定义了LIBCARLA_WITH_PYTHON_SUPPORT宏，该宏通常用于控制是否包含Python支持  
#  if defined(__clang__)   // 如果使用Clang编译器，则保存当前的编译诊断设置
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wdeprecated-register"  // 忽略Clang编译器关于"-Wdeprecated-register"的警告，这通常是因为使用了已弃用的寄存器关键字 
#  endif
#    include <boost/python.hpp> // 包含Boost.Python的头文件，用于在C++代码中提供Python绑定 
#  if defined(__clang__)  // 如果之前是为了Clang编译器保存了编译诊断设置，现在恢复它们
#    pragma clang diagnostic pop
#  endif  // 结束LIBCARLA_WITH_PYTHON_SUPPORT宏的检查
#endif //用于在编译时启动LibCarla中与Python绑定的功能
namespace carla {

  class PythonUtil {
  public:

    static bool ThisThreadHasTheGIL() {
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
#  if PY_MAJOR_VERSION >= 3
      return PyGILState_Check();
#  else
      PyThreadState *tstate = _PyThreadState_Current;
      return (tstate != nullptr) && (tstate == PyGILState_GetThisThreadState());
#  endif // PYTHON3
#else
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
