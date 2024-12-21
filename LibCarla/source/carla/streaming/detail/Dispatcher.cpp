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
// 作用：在对象销毁时断开所有会话与其流的连接，确保此时 io_context 已停止
  Dispatcher::~Dispatcher() {
    // 遍历存储流状态的映射表
     // 断开所有会话与其流的连接，确保在此时 io_context 已停止
    for (auto &pair : _stream_map) {
#ifndef LIBCARLA_NO_EXCEPTIONS // 如果没有定义 LIBCARLA_NO_EXCEPTIONS，即允许异常
      try {
#endif // LIBCARLA_NO_EXCEPTIONS
        auto stream_state = pair.second;// 获取流状态
        stream_state->ClearSessions();// 清除会话
#ifndef LIBCARLA_NO_EXCEPTIONS
      } catch (const std::exception &e) {
         // 如果在清除会话时发生异常，记录错误信息
        log_error("failed to clear sessions:", e.what());
      }
#endif // LIBCARLA_NO_EXCEPTIONS
    }
  }

  // Dispatcher 类成员函数
  // 创建一个新的流或重用现有流
// 返回值：carla::streaming::Stream 对象，代表创建的或重用的流
  carla::streaming::Stream Dispatcher::MakeStream() {
    std::lock_guard<std::mutex> lock(_mutex);// 确保线程安全
    ++_cached_token._token.stream_id; // 增加流ID，防止溢出
    log_debug("New stream:", _cached_token._token.stream_id);
    std::shared_ptr<MultiStreamState> ptr;// 声明流状态的智能指针
    auto search = _stream_map.find(_cached_token.get_stream_id());// 查找现有流
    if (search == _stream_map.end()) {
      // 如果没有找到，创建新的流
      ptr = std::make_shared<MultiStreamState>(_cached_token);
      
      // 尝试将新创建的流状态对象插入到流映射表中
      auto result = _stream_map.emplace(std::make_pair(_cached_token.get_stream_id(), ptr));
      if (!result.second) {
        // 如果插入失败（理论上不应该发生，除非有并发问题），抛出异常
        throw_exception(std::runtime_error("failed to create stream!"));
      }
       // 记录新流的创建
      log_debug("Stream created");
      // 返回代表新创建的流的 Stream 对象
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
      // 使用互斥锁保护共享资源_stream_map，避免多线程同时访问导致数据竞争
    std::lock_guard<std::mutex> lock(_mutex);
    // 打印日志，记录关闭流的请求，包括流的ID
    log_debug("Calling CloseStream for ", id);
  // 在_stream_map中查找指定ID的流
    auto search = _stream_map.find(id);
  // 如果找到了指定ID的流
    if (search != _stream_map.end()) {
      // 获取流的状态（可能是一个指向流状态对象的智能指针或其他容器）
      auto stream_state = search->second;
         // 如果流状态有效（非空）
      if (stream_state) {
        // 打印日志，记录断开所有会话的操作，包括流的ID
        log_debug("Disconnecting all sessions (stream ", id, ")");
        // 断开与该流关联的所有会话
        stream_state->ClearSessions();
      }
    // 从_stream_map中删除该流的状态信息
      _stream_map.erase(search);
    }
  }

  // 注册会话到指定流
  bool Dispatcher::RegisterSession(std::shared_ptr<Session> session) {
      // 确保传入的会话指针不为空
    DEBUG_ASSERT(session != nullptr);
     // 使用互斥锁保护共享资源_stream_map，避免多线程同时访问导致数据竞争
    std::lock_guard<std::mutex> lock(_mutex);
      // 在_stream_map中查找与会话关联的流的ID
    auto search = _stream_map.find(session->get_stream_id());
     // 如果找到了对应的流
    if (search != _stream_map.end()) {
       // 获取流的状态（可能是一个指向流状态对象的智能指针或其他容器）
      auto stream_state = search->second;
       // 如果流状态有效（非空）
      if (stream_state) {
         // 打印日志，记录连接会话的操作，包括流的ID
        log_debug("Connecting session (stream ", session->get_stream_id(), ")");
        // 将会话连接到该流
        stream_state->ConnectSession(std::move(session));
        // 打印日志，记录当前_stream_map中的流数量
        log_debug("Current streams: ", _stream_map.size());
        return true;
      }
    }
     // 如果流不存在或无效，打印错误日志
    log_error("Invalid session: no stream available with id", session->get_stream_id());
    return false;
  // 返回false，表示会话注册失败
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
