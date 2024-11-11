// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/**
 * @file
 * @brief 包含StreamStateBase类的定义，它是流状态的基础类。
 */
#include "carla/NonCopyable.h"
#include "carla/streaming/detail/Session.h"
#include "carla/streaming/detail/Token.h"

#include <memory>

namespace carla {
    /**
   * @class BufferPool
   * @brief 缓冲区池的类声明（前向声明），用于在StreamStateBase中管理缓冲区。
   */
  class BufferPool;

namespace streaming {
namespace detail {

    /**
      * @class StreamStateBase
      * @brief 所有流副本之间的共享状态类。如果处于活动状态，则提供对底层服务器会话的访问。
      *
      * 该类继承自NonCopyable，以防止对象被复制。
      */
  class StreamStateBase : private NonCopyable {
  public:
      /**
     * @brief 构造函数，使用给定的令牌初始化对象。
     *
     * @param token 用于初始化对象的令牌。
     */
    explicit StreamStateBase(const token_type &token);
    /**
     * @brief 析构函数，清理资源。
     */
    virtual ~StreamStateBase();
    /**
     * @brief 获取用于初始化对象的令牌。
     *
     * @return 返回用于初始化对象的令牌。
     */
    const token_type &token() const {
      return _token;
    }
    /**
     * @brief 创建一个缓冲区。
     *
     * @return 返回一个新创建的缓冲区。
     */
    Buffer MakeBuffer();
    /**
     * @brief 连接到会话。
     *
     * 这是一个纯虚函数，需要在派生类中实现。
     *
     * @param session 要连接的会话的共享指针。
     */
    virtual void ConnectSession(std::shared_ptr<Session> session) = 0;
    /**
     * @brief 断开与会话的连接。
     *
     * 这是一个纯虚函数，需要在派生类中实现。
     *
     * @param session 要断开的会话的共享指针。
     */
    virtual void DisconnectSession(std::shared_ptr<Session> session) = 0;
    /**
     * @brief 清除所有会话。
     *
     * 这是一个纯虚函数，需要在派生类中实现。
     */
    virtual void ClearSessions() = 0;

  private:
      /**
     * @brief 用于初始化对象的令牌。
     */
    const token_type _token;
    /**
     * @brief 指向缓冲区池的共享指针，用于管理缓冲区。
     */
    const std::shared_ptr<BufferPool> _buffer_pool;
  };

} // namespace detail
} // namespace streaming
} // namespace carla
