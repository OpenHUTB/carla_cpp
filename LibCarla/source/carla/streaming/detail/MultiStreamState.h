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
        if (_sessions.size() == 1)
          _session.store(_sessions[0]);
        else
          _session.store(nullptr);
      }
      log_debug("Disconnecting multistream sessions:", _sessions.size());
    }
// 清空所有的会话
    void ClearSessions() final {
      std::lock_guard<std::mutex> lock(_mutex);
      for (auto &s : _sessions) {
        if (s != nullptr) {
          s->Close();
        }
      }
      _sessions.clear();
      _force_active = false;
      _session.store(nullptr);
      log_debug("Disconnecting all multistream sessions");
    }

  private:

    std::mutex _mutex;

    // if there is only one session, then we use atomic
    AtomicSharedPtr<Session> _session;
    // if there are more than one session, we use vector of sessions with mutex
    std::vector<std::shared_ptr<Session>> _sessions;
    bool _force_active {false};
    bool _enabled_for_ros {false};
  };

} // namespace detail
} // namespace streaming
} // namespace carla
