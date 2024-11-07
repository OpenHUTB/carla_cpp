// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/Dispatcher.h"

#include "carla/Exception.h"
#include "carla/Logging.h"
#include "carla/streaming/detail/MultiStreamState.h"

#include <exception>

namespace carla {
namespace streaming {
namespace detail {

  // Dispatcher 析构函数
  Dispatcher::~Dispatcher() {
    
     // 断开所有会话与其流的连接，确保在此时 io_context 已停止
    for (auto &pair : _stream_map) {
#ifndef LIBCARLA_NO_EXCEPTIONS
      try {
#endif // LIBCARLA_NO_EXCEPTIONS
        auto stream_state = pair.second;// 获取流状态
        stream_state->ClearSessions();// 清除会话
#ifndef LIBCARLA_NO_EXCEPTIONS
      } catch (const std::exception &e) {
        log_error("failed to clear sessions:", e.what());
      }
#endif // LIBCARLA_NO_EXCEPTIONS
    }
  }

  // 创建一个新的流或重用现有流
  carla::streaming::Stream Dispatcher::MakeStream() {
    std::lock_guard<std::mutex> lock(_mutex);// 确保线程安全
    ++_cached_token._token.stream_id; // 增加流ID，防止溢出
    log_debug("New stream:", _cached_token._token.stream_id);
    std::shared_ptr<MultiStreamState> ptr;// 声明流状态的智能指针
    auto search = _stream_map.find(_cached_token.get_stream_id());// 查找现有流
    if (search == _stream_map.end()) {
      // 如果没有找到，创建新的流
      ptr = std::make_shared<MultiStreamState>(_cached_token);
      auto result = _stream_map.emplace(std::make_pair(_cached_token.get_stream_id(), ptr));
      if (!result.second) {
        throw_exception(std::runtime_error("failed to create stream!"));
      }
      log_debug("Stream created");
      return carla::streaming::Stream(ptr);
    } else {
      // 将新流插入流映射表中
      log_debug("Stream reused");// 日志记录流创建信息
      ptr = search->second;// 获取现有流状态
      return carla::streaming::Stream(ptr);// 返回现有流
    }
  }

  // 关闭指定ID的流
  void Dispatcher::CloseStream(carla::streaming::detail::stream_id_type id) {
    std::lock_guard<std::mutex> lock(_mutex);
    log_debug("Calling CloseStream for ", id);
    auto search = _stream_map.find(id);
    if (search != _stream_map.end()) {
      auto stream_state = search->second;
      if (stream_state) {
        log_debug("Disconnecting all sessions (stream ", id, ")");
        stream_state->ClearSessions();
      }
      _stream_map.erase(search);
    }
  }

  // 注册会话到指定流
  bool Dispatcher::RegisterSession(std::shared_ptr<Session> session) {
    DEBUG_ASSERT(session != nullptr);
    std::lock_guard<std::mutex> lock(_mutex);
    auto search = _stream_map.find(session->get_stream_id());
    if (search != _stream_map.end()) {
      auto stream_state = search->second;
      if (stream_state) {
        log_debug("Connecting session (stream ", session->get_stream_id(), ")");
        stream_state->ConnectSession(std::move(session));
        log_debug("Current streams: ", _stream_map.size());
        return true;
      }
    }
    log_error("Invalid session: no stream available with id", session->get_stream_id());
    return false;
  }

  // 从流中注销会话
  void Dispatcher::DeregisterSession(std::shared_ptr<Session> session) {
    DEBUG_ASSERT(session != nullptr);
    std::lock_guard<std::mutex> lock(_mutex);
    log_debug("Calling DeregisterSession for ", session->get_stream_id());
    auto search = _stream_map.find(session->get_stream_id());
    if (search != _stream_map.end()) {
      auto stream_state = search->second;
      if (stream_state) {
        log_debug("Disconnecting session (stream ", session->get_stream_id(), ")");
        stream_state->DisconnectSession(session);
        log_debug("Current streams: ", _stream_map.size());
      }
    }
  }
  
  token_type Dispatcher::GetToken(stream_id_type sensor_id) {
    std::lock_guard<std::mutex> lock(_mutex);
    log_debug("Searching sensor id: ", sensor_id);
    auto search = _stream_map.find(sensor_id);
    if (search != _stream_map.end()) {
      log_debug("Found sensor id: ", sensor_id);
      auto stream_state = search->second;
      stream_state->ForceActive();
      log_debug("Getting token from stream ", sensor_id, " on port ", stream_state->token().get_port());
      return stream_state->token();
    } else {
      
      // 如果没有找到，创建新的传感器流
      log_debug("Not Found sensor id, creating sensor stream: ", sensor_id);
      token_type temp_token(_cached_token);
      temp_token.set_stream_id(sensor_id);
      auto ptr = std::make_shared<MultiStreamState>(temp_token);
      auto result = _stream_map.emplace(std::make_pair(temp_token.get_stream_id(), ptr));
      ptr->ForceActive();
      if (!result.second) {
        log_debug("Failed to create multistream for stream ", sensor_id, " on port ", temp_token.get_port());
      }
      log_debug("Created token from stream ", sensor_id, " on port ", temp_token.get_port());
      return temp_token;
    }
    return token_type();// 如果未找到流，返回默认令牌
  }

} // namespace detail
} // namespace streaming
} // namespace carla
