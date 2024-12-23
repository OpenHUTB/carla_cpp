// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被重复包含

#include "carla/NonCopyable.h"  // 包含Carla的非可复制类定义，可能用于防止类的实例被复制 
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT  
     // 检查是否定义了LIBCARLA_WITH_PYTHON_SUPPORT宏，以控制是否包含Python支持
#  if defined(__clang__)
     // 如果使用Clang编译器，则保存当前的编译诊断设置
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wdeprecated-register" 
     // 忽略Clang编译器关于已弃用的寄存器关键字的警告
     // 这通常是因为使用了已弃用的寄存器关键字 
#  endif
#    include <boost/python.hpp>
     // 包含Boost.Python的头文件，用于在C++代码中提供Python绑定
#  if defined(__clang__)  
     // 如果之前是为了Clang编译器保存了编译诊断设置，现在恢复它们
#    pragma clang diagnostic pop
#  endif  
    // 结束LIBCARLA_WITH_PYTHON_SUPPORT宏的检查
#endif 
    //用于在编译时启动LibCarla中与Python绑定的功能
namespace carla {
     // 定义carla命名空间
     // 在这里可以添加carla命名空间内的类、函数等
  class PythonUtil {
  public:

    static bool ThisThreadHasTheGIL() {
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT  // 如果项目配置了Python
#  if PY_MAJOR_VERSION >= 3 // 如果是Python 3及以上版本
      return PyGILState_Check();  // 使用Python 3的API检查当前线程是否持有GIL
  #  else
      // 对于Python 2通过检查当前线程的状态是否等于GIL状态来检查
      PyThreadState *tstate = _PyThreadState_Current;
      // 检查tstate是否为非空，并且与通过PyGILState_GetThisThreadState()获取的状态相同
      return (tstate != nullptr) && (tstate == PyGILState_GetThisThreadState());
#  endif  
      // LIBCARLA_WITH_PYTHON_SUPPORT
#else
      return false;  // 如果没有配置Python支持，总是返回false 
#endif // LIBCARLA_WITH_PYTHON_SUPPORT
    }

#ifdef LIBCARLA_WITH_PYTHON_SUPPORT

    /// 获取Python全局解释器锁上的锁，这是从其他线程调用Python代码所必需的。
    class AcquireGIL : private NonCopyable {
   // AcquireGIL类：在构造时获取GIL，在析构时释放GIL
    // 假设NonCopyable是一个已经定义的基类，用于禁止对象的拷贝和赋值操作
    public:

      AcquireGIL() : _state(PyGILState_Ensure()) {}
    // 构造函数：调用PyGILState_Ensure()获取GIL，并将返回的状态保存在_state成员中
      ~AcquireGIL() {
        PyGILState_Release(_state);
      } 
    // 析构函数：调用PyGILState_Release()释放GIL
    // 注意：由于AcquireGIL类继承自NonCopyable（私有继承），
    // 因此外部代码无法拷贝或赋值AcquireGIL对象，这有助于防止GIL状态的不当管理。
    // 使用示例（假设在Python环境中）：
    // {
    //     AcquireGIL gilLock; // 创建AcquireGIL对象时自动获取GIL
    //     // 在此代码块中，可以安全地调用Python C API函数，因为GIL已被获取
    // } // 当gilLock对象超出作用域并被销毁时，GIL将自动被释放
    private:

      PyGILState_STATE _state;
    };
    // 用于保存GIL状态的成员变量
    /// 释放Python的全局解释器锁，在执行阻塞I/O操作时使用它。
      ~ReleaseGIL() {
        PyEval_RestoreThread(_state);
      }
  // 检查_state是否为非空，以避免在非Python线程中调用PyEval_RestoreThread()
  // 析构函数：调用PyEval_RestoreThread(_state)重新获取GIL
  // 注意：这个函数应该只在之前调用PyEval_SaveThread()保存了线程状态的同一个线程中调用。
  // 在Python 3.9及更高版本中，如果_state为NULL，则不需要（也不应该）调用PyEval_RestoreThread()。
  // 在这些版本中，如果当前线程不是Python线程，则不需要执行任何操作来“恢复”GIL，
  // 因为GIL只与活动的Python线程相关。然而，为了向后兼容，这里仍然进行了空指针检查。
    private:

      PyThreadState *_state;
  // 用于保存当前线程状态的成员变量
    }; 
  // 注意：由于ReleaseGIL类继承自NonCopyable（私有继承），
  // 因此外部代码无法拷贝或赋值ReleaseGIL对象，这有助于防止GIL状态的不当管理。
 // 使用示例（假设在Python环境中，并且当前线程是Python线程）：
 // {
 //     ReleaseGIL gilUnlock; // 创建ReleaseGIL对象时自动释放GIL
 //     // 在此代码块中，当前线程可以执行非Python任务，同时允许其他Python线程运行
 //     // （注意：这里不应该调用任何Python C API函数，因为GIL已被释放）
 // } // 当gilUnlock对象超出作用域并被销毁时，GIL将自动被重新获取
#else // LIBCARLA_WITH_PYTHON_SUPPORT

    class AcquireGIL : private NonCopyable {};
    class ReleaseGIL : private NonCopyable {};

#endif // LIBCARLA_WITH_PYTHON_SUPPORT

    /// 可以传递给智能指针的删除器，以便在销毁对象之前获取GIL。
    class AcquireGILDeleter {
    public:// 模板成员函数，用于删除ptr指向的对象，若支持Python，则在获取GIL后进行
    
      template <typename T>
      void operator()(T *ptr) const {
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
        if (ptr != nullptr && !PythonUtil::ThisThreadHasTheGIL()) {
          // 添加空指针检查以提高代码健壮性
          // 若ptr非空且当前线程未持有GIL，则获取GIL后删除对象
          AcquireGIL lock;
          // 获取GIL（假设AcquireGIL的构造函数会执行此操作）
          delete ptr;
          // 现在已持有GIL，可以安全删除对象
        } else
#endif  
           // 若已持有GIL或不支持Python，则直接删除对象
        delete ptr;
      } 
        // LIBCARLA_WITH_PYTHON_SUPPORT
        // 无论是否持有GIL，都执行删除操作（在支持Python且未持有GIL的分支外）
    };

    /// 可以传递给智能指针的删除器，以便在销毁对象之前释放GIL。


 
    class ReleaseGILDeleter {
// ReleaseGILDeleter类：一个自定义的删除器类，用于在删除对象时处理GIL
    public:

      template <typename T>
      void operator()(T *ptr) const {
    // 重载operator()，用于删除对象
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
        if (ptr != nullptr && PythonUtil::ThisThreadHasTheGIL()) {
          // 如果定义了LIBCARLA_WITH_PYTHON_SUPPORT宏，并且当前线程持有GIL
          ReleaseGIL lock;
          // 创建一个ReleaseGIL对象，以暂时释放GIL
          delete ptr;
          // 当lock对象超出作用域时，会自动重新获取GIL
          // 删除对象
        } else
            // 注意：由于lock是局部变量，当这行代码执行完毕后，lock对象会被销毁，
            // 从而自动重新获取GIL。但是，这里的代码逻辑其实有一点冗余，
            // 因为即使不显式地创建ReleaseGIL对象，delete ptr本身也不会持有GIL，
            // 所以这里的ReleaseGIL主要是为了在delete之前确保GIL被释放，
            // 以允许其他Python线程运行。然而，如果delete ptr的操作非常快，
            // 这个GIL的释放和重新获取可能并没有太大的实际意义。
#endif // LIBCARLA_WITH_PYTHON_SUPPORT
        delete ptr;
        // LIBCARLA_WITH_PYTHON_SUPPORT
        // 如果没有定义LIBCARLA_WITH_PYTHON_SUPPORT宏，或者当前线程没有持有GIL，
        // 则直接删除对象
      }
        // 注意：这里的注释应该放在代码块的外部，以解释整个代码块的功能。
        // 注释应该简洁明了，避免与代码混淆。
    };
  };

} // namespace carla
// 使用示例（假设在适当的上下文中）：
// std::unique_ptr<MyClass, ReleaseGILDeleter> myObject(new MyClass());
// 当myObject超出作用域时，ReleaseGILDeleter的operator()会被调用，以删除MyClass对象。
