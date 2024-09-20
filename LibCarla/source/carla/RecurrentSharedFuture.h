// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被重复包含

#include "carla/Exception.h"  // 引入CARLA项目中的异常处理头文件 
#include "carla/Time.h"   // 引入CARLA项目中的时间处理头文件

#include <boost/optional.hpp>  // 引入 Boost 库中的 optional 头文件，用于表示可选的值
#ifdef _MSC_VER  // 如果是在 Microsoft Visual C++ (MSVC) 环境下编译
#pragma warning(push)  // 保存当前的警告状态，以便之后恢复 
#pragma warning(disable:4583)  // 禁用特定于 MSVC 的警告 4583，这个警告通常与编译器如何处理模板实例化有关
#pragma warning(disable:4582)  // 禁用特定于 MSVC 的警告 4582，这个警告涉及构造函数或析构函数的隐式转换
#include <boost/variant2/variant.hpp> // 引入 Boost 库中的 variant2 头文件，variant2 是 Boost.Variant 的后续版本，提供了更灵活和强大的类型安全联合体
#pragma warning(pop) // 恢复之前保存的警告状态
#else
#include <boost/variant2/variant.hpp> // 如果不是在 MSVC 环境下，直接引入 Boost.Variant2
#endif

#include <condition_variable> // 引入 C++ 标准库中的条件变量头文件，用于同步操作，如等待某个条件成立 
#include <exception> // 引入 C++ 标准库中的异常处理头文件
#include <map> // 引入 C++ 标准库中的 map 头文件，map 是一个关联容器，存储的元素是键值对 
#include <mutex> // 引入 C++ 标准库中的互斥锁头文件，用于提供互斥锁，以保护共享数据的同步访问 

namespace carla {

namespace detail {

  class SharedException; // 定义一个异常类，用于在文件中共享和传递异常信息

} // namespace detail

  // ===========================================================================
  // -- RecurrentSharedFuture --------------------------------------------------
  // ===========================================================================

  /// 这个类类似于共享未来（shared future）的使用方式，但是它可以被设置任意次数的值。
  template <typename T>
  class RecurrentSharedFuture {
  public:

    using SharedException = detail::SharedException; // 使用detail命名空间下的SharedException类型，作为此模板类的一部分 

    ///等待直到下一个值被设置。任意数量的线程可以同时等待。


    /// @return 如果达到超时时间，则返回空的optional
    boost::optional<T> WaitFor(time_duration timeout);

    /// 设置值并通知所有等待的线程
    template <typename T2>
    void SetValue(const T2 &value);

    /// Set a exception, this exception will be thrown on all the threads
    /// waiting.
    ///
    /// @note The @a exception will be stored on a SharedException and thrown
    /// as such.
    template <typename ExceptionT>
    void SetException(ExceptionT &&exception);

  private:

    std::mutex _mutex;

    std::condition_variable _cv;

    struct mapped_type {
      bool should_wait;
      boost::variant2::variant<SharedException, T> value;
    };

    std::map<const char *, mapped_type> _map;
  };

  // ===========================================================================
  // -- RecurrentSharedFuture implementation -----------------------------------
  // ===========================================================================
// 定义了一个名为 detail 的命名空间
namespace detail {
//// 定义一个线程局部的静态常量字符变量，用于标识或标记当前线程，其值默认为空字符（'\0'）
  static thread_local const char thread_tag{};

  class SharedException : public std::exception {
  public:

    SharedException()
      : _exception(std::make_shared<std::runtime_error>("uninitialized SharedException")) {}

    SharedException(std::shared_ptr<std::exception> e)
      : _exception(std::move(e)) {}

    const char *what() const noexcept override {
      return _exception->what();
    }

    std::shared_ptr<std::exception> GetException() const {
      return _exception;
    }

  private:

    std::shared_ptr<std::exception> _exception;
  };

} // namespace detail

  template <typename T>
  boost::optional<T> RecurrentSharedFuture<T>::WaitFor(time_duration timeout) {
    std::unique_lock<std::mutex> lock(_mutex);
    auto &r = _map[&detail::thread_tag];
    r.should_wait = true;
    if (!_cv.wait_for(lock, timeout.to_chrono(), [&]() { return !r.should_wait; })) {
      return {};
    }
    if (r.value.index() == 0) {
      throw_exception(boost::variant2::get<SharedException>(r.value));
    }
    return boost::variant2::get<T>(std::move(r.value));
  }

  template <typename T>
  template <typename T2>
  void RecurrentSharedFuture<T>::SetValue(const T2 &value) {
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto &pair : _map) {
      pair.second.should_wait = false;
      pair.second.value = value;
    }
    _cv.notify_all();
  }

  template <typename T>
  template <typename ExceptionT>
  void RecurrentSharedFuture<T>::SetException(ExceptionT &&e) {
    SetValue(SharedException(std::make_shared<ExceptionT>(std::forward<ExceptionT>(e))));
  }

} // namespace carla
