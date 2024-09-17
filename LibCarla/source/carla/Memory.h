// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被重复包含

#include <boost/enable_shared_from_this.hpp>  // 引入boost库中的enable_shared_from_this，允许对象安全地生成自己的shared_ptr 
#include <boost/make_shared.hpp>  // 引入boost库中的make_shared，用于创建shared_ptr实例  
#include <boost/shared_ptr.hpp>   // 引入boost库中的shared_ptr，智能指针，用于自动管理资源 
#include <boost/weak_ptr.hpp>  // 引入boost库中的weak_ptr，弱引用智能指针，不拥有对象，但可以检查对象是否存在 

namespace carla {

  /// Use this SharedPtr (boost::shared_ptr) to keep compatibility with
  /// boost::python, but it would be nice if in the future we can make a Python
  /// adaptor for std::shared_ptr.
  template <typename T>
  using SharedPtr = boost::shared_ptr<T>;

  template <typename T>
  using WeakPtr = boost::weak_ptr<T>;

  template <typename T>
  using EnableSharedFromThis = boost::enable_shared_from_this<T>;

  template <typename T, typename... Args>
  static inline auto MakeShared(Args &&... args) {
    return boost::make_shared<T>(std::forward<Args>(args)...);
  }

} // namespace carla
