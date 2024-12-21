// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 包含必要的头文件
#include "carla/AtomicSharedPtr.h"
// 用于原子性的共享指针操作
#include "carla/Logging.h"
// 用于日志记录
#include "carla/streaming/detail/StreamStateBase.h"
// 基类，可能提供了一些基本的流状态管理功能
#include "carla/streaming/detail/tcp/Message.h"

#include <mutex>
#include <vector>
#include <atomic>

namespace carla {
namespace streaming {
namespace detail {

  /// A stream state that can hold any number of sessions.
  ///
  /// @todo Lacking some optimization.
  class MultiStreamState final : public StreamStateBase {
  public:
 // 调用基类的构造函数
    using StreamStateBase::StreamStateBase;
 // 构造函数，接受一个 token 并初始化成员变量
    MultiStreamState(const token_type &token) :
      StreamStateBase(token),
      _session(nullptr)
      {};
// 模板函数，用于写入数据到流中
    template <typename... Buffers>
    void Write(Buffers... buffers) {
      // try write single stream
      auto session = _session.load();
      if (session != nullptr) {
		  // 创建消息并写入单个会话
        auto message = Session::MakeMessage(buffers...);
        session->Write(std::move(message));
        log_debug("sensor ", session->get_stream_id()," data sent");
        // Return here, _session is only valid if we have a
        // single session.
        return;
      }

      // try write multiple stream
      std::lock_guard<std::mutex> lock(_mutex);
      if (_sessions.size() > 0) {
		  // 创建消息并写入多个会话
        auto message = Session::MakeMessage(buffers...);
        for (auto &s : _sessions) {
          if (s != nullptr) {
            s->Write(message);
            log_debug("sensor ", s->get_stream_id()," data sent ");
         }
        }
      }
    }
 // 设置强制激活标志
    void ForceActive() {
      _force_active = true;
    }
// 启用与 ROS 的交互
    void EnableForROS() {
      _enabled_for_ros = true;
    }
// 禁用与 ROS 的交互
    void DisableForROS() {
      _enabled_for_ros = false;
    }
 // 检查是否为 ROS 启用了流状态
    bool IsEnabledForROS() {
      return _enabled_for_ros;
    }
 // 检查是否有客户端正在监听流
    bool AreClientsListening() {
      return (_sessions.size() > 0 || _force_active || _enabled_for_ros);
    }
// 连接一个新的会话
    void ConnectSession(std::shared_ptr<Session> session) final {
      DEBUG_ASSERT(session != nullptr);
      std::lock_guard<std::mutex> lock(_mutex);
	  // 将新会话添加到会话列表中
      _sessions.emplace_back(std::move(session));
      log_debug("Connecting multistream sessions:", _sessions.size());
      if (_sessions.size() == 1) {
		  // 如果只有一个会话，设置 _session 指向这个会话
        _session.store(_sessions[0]);
      }
      else if (_sessions.size() > 1) {
		  // 如果有多个会话，设置 _session 为空指针
        _session.store(nullptr);
      }
    }
// 断开一个会话
    void DisconnectSession(std::shared_ptr<Session> session) final {
      DEBUG_ASSERT(session != nullptr);
      std::lock_guard<std::mutex> lock(_mutex);
      log_debug("Calling DisconnectSession for ", session->get_stream_id());
      if (_sessions.size() == 0) return;
      if (_sessions.size() == 1) {
        DEBUG_ASSERT(session == _session.load());
        _session.store(nullptr);
        _sessions.clear();
        _force_active = false;
        log_debug("Last session disconnected");
      } else {
		   // 从会话列表中移除指定的会话
        _sessions.erase(
            std::remove(_sessions.begin(), _sessions.end(), session),
            _sessions.end());

        // set single session if only one
        if (_sessions.size() == 1)// 检查当前会话集合的大小
          _session.store(_sessions[0]);  // 如果会话集合中只有一个会话，
  // 则将该会话存储到某个全局或成员变量 _session 中
        else
          _session.store(nullptr);
      }  
    // 如果会话集合中有多个会话或没有会话，
    // 则将 nullptr 存储到 _session 中，
    // 表示当前没有活动的会话或不确定哪个会话是活动的
      log_debug("Disconnecting multistream sessions:", _sessions.size());
    }
     // 使用 log_debug 函数记录一条调试信息，
     // 显示正在断开连接的多流会话的数量
     // 清空所有的会话
    void ClearSessions() final {
     // ClearSessions 是一个成员函数，用于清除所有会话并重置相关状态
      std::lock_guard<std::mutex> lock(_mutex);  
      // 使用 std::lock_guard 自动管理互斥锁 _mutex 的锁定和解锁
      // 这确保了在多线程环境中对 _sessions 的访问是线程安全的
      for (auto &s : _sessions) {
      // 遍历 _sessions 容器中的每个会话对象
        if (s != nullptr) {  
          s->Close();
      // 如果会话对象不是 nullptr，则调用其 Close 方法来关闭会话
        }
      }
     _sessions.clear();
     // 清空 _sessions 容器，移除所有会话对象
      _force_active = false;   // 将 _force_active 标志设置为 false，表示没有强制活动的会话
      _session.store(nullptr);
    // 将 _session 变量重置为 nullptr，表示当前没有活动的会话
      log_debug("Disconnecting all multistream sessions"); 
    // 使用 log_debug 函数记录一条调试信息，表示正在断开所有多流会话
    }

  private:

    std::mutex _mutex;
    // 私有成员变量
    // _mutex 是一个互斥锁，用于保护对 _sessions 容器的并发访问
    // if there is only one session, then we use atomic
    AtomicSharedPtr<Session> _session;
    // if there are more than one session, we use vector of sessions with mutex
    std::vector<std::shared_ptr<Session>> _sessions; 
    // _sessions 是一个向量（动态数组），存储了多个指向 Session 对象的智能指针
    // 这些智能指针是 std::shared_ptr 类型，它们自动管理 Session 对象的生命周期
    // 当没有任何 std::shared_ptr 指向一个 Session 对象时，该对象会被自动删除
    bool _force_active {false};   // _force_active 是一个布尔变量，用于指示是否存在一个或多个会话被强制标记为活动状态
    // 如果为 true，则可能表示有会话需要被特别处理，即使按照正常逻辑它们可能不应该处于活动状态
    // 初始化为 false，表示默认没有会话被强制标记为活动状态
    bool _enabled_for_ros {false};    // _enabled_for_ros 是一个布尔变量，用于指示该类或其中的会话是否启用了对 ROS（Robot Operating System）的支持
    // 如果为 true，则可能表示该类或其中的会话能够与 ROS 系统进行交互，例如发送或接收消息
    // 类的其他成员变量、方法和构造函数应该在这里定义
    // 注意：这些变量是公开的（public），但在实际设计中，您可能希望将它们设为私有（private）
    // 并通过公共的 getter 和 setter 方法来访问它们，以封装类的内部状态
  };

} // namespace detail
} // namespace streaming
} // namespace carla
