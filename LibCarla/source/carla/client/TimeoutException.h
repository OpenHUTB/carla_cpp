// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/// @file  
/// 包含TimeoutException类的声明，该类继承自std::runtime_error，用于表示超时异常
#include "carla/Time.h"///< 引入CARLA的时间处理相关类和函数 

#include <stdexcept>///< 引入标准异常类 
#include <string>///< 引入字符串类

namespace carla {
namespace client {
    /// @brief 表示客户端操作超时的异常类。  
   /// @details 该类继承自std::runtime_error，用于在客户端操作超过指定时间限制时抛出异常。  
   ///         异常中包含了发生超时的端点信息和超时时长。 
  class TimeoutException : public std::runtime_error {
  public:
      /// @brief 构造函数。  
        /// @param endpoint 发生超时的端点信息。  
        /// @param timeout 超时时长，类型为carla::Time::time_duration。  
        /// @details 构造函数初始化基类std::runtime_error的what()方法返回的消息，  
        ///          并保存端点信息和超时时长作为异常的一部分。
    explicit TimeoutException(
        const std::string &endpoint,///< 端点信息
        time_duration timeout);///< 超时时长
  };

} // namespace client
} // namespace carla
