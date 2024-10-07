// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/primaryCommands.h"

// #include "carla/Logging.h"
#include "carla/multigpu/commands.h"
#include "carla/multigpu/primary.h"
#include "carla/multigpu/router.h"
#include "carla/streaming/detail/tcp/Message.h"
#include "carla/streaming/detail/Token.h"
#include "carla/streaming/detail/Types.h"

namespace carla {
namespace multigpu {

PrimaryCommands::PrimaryCommands() {
}

PrimaryCommands::PrimaryCommands(std::shared_ptr<Router> router) :
  _router(router) {
}

void PrimaryCommands::set_router(std::shared_ptr<Router> router) {
  _router = router;
}

// 向所有辅助服务器广播帧数据
void PrimaryCommands::SendFrameData(carla::Buffer buffer) {
  _router->Write(MultiGPUCommand::SEND_FRAME, std::move(buffer));
  // log_info("sending frame command");
}

// 向所有辅助服务器广播要加载的地图
void PrimaryCommands::SendLoadMap(std::string map) {
  carla::Buffer buf((unsigned char *) map.c_str(), (size_t) map.size() + 1);
  _router->Write(MultiGPUCommand::LOAD_MAP, std::move(buf));
}

// 向路由器需要令牌的人员发送请求
token_type PrimaryCommands::SendGetToken(stream_id sensor_id) {
  log_info("asking for a token");
  carla::Buffer buf((carla::Buffer::value_type *) &sensor_id,
                    (size_t) sizeof(stream_id));
  auto fut = _router->WriteToNext(MultiGPUCommand::GET_TOKEN, std::move(buf));

  auto response = fut.get();
  token_type new_token(*reinterpret_cast<carla::streaming::detail::token_data *>(response.buffer.data()));
  log_info("got a token: ", new_token.get_stream_id(), ", ", new_token.get_port());
  return new_token;
}

// 发送以了解连接是否处于活动状态
void PrimaryCommands::SendIsAlive() {
  std::string msg("Are you alive?");
  carla::Buffer buf((unsigned char *) msg.c_str(), (size_t) msg.size());
  log_info("sending is alive command");
  auto fut = _router->WriteToNext(MultiGPUCommand::YOU_ALIVE, std::move(buf));
  auto response = fut.get();
  log_info("response from alive command: ", response.buffer.data());
}

void PrimaryCommands::SendEnableForROS(stream_id sensor_id) {
  // 搜索传感器是否已在任何辅助服务器中激活
  auto it = _servers.find(sensor_id);
  if (it != _servers.end()) {
    carla::Buffer buf((carla::Buffer::value_type *) &sensor_id,
                      (size_t) sizeof(stream_id));
    auto fut = _router->WriteToOne(it->second, MultiGPUCommand::ENABLE_ROS, std::move(buf));

    auto response = fut.get();
    bool res = (*reinterpret_cast<bool *>(response.buffer.data()));
  } else {
    log_error("enable_for_ros for sensor", sensor_id, " not found on any server");
  }
}

void PrimaryCommands::SendDisableForROS(stream_id sensor_id) {
  // 搜索传感器是否已在任何辅助服务器中激活
  auto it = _servers.find(sensor_id);
  if (it != _servers.end()) {
    carla::Buffer buf((carla::Buffer::value_type *) &sensor_id,
                      (size_t) sizeof(stream_id));
    auto fut = _router->WriteToOne(it->second, MultiGPUCommand::DISABLE_ROS, std::move(buf));

    auto response = fut.get();
    bool res = (*reinterpret_cast<bool *>(response.buffer.data()));
  } else {
    log_error("disable_for_ros for sensor", sensor_id, " not found on any server");
  }
}

bool PrimaryCommands::SendIsEnabledForROS(stream_id sensor_id) {
  // 搜索传感器是否已在任何辅助服务器中激活
  auto it = _servers.find(sensor_id);
  if (it != _servers.end()) {
    carla::Buffer buf((carla::Buffer::value_type *) &sensor_id,
                      (size_t) sizeof(stream_id));
    auto fut = _router->WriteToOne(it->second, MultiGPUCommand::IS_ENABLED_ROS, std::move(buf));

    auto response = fut.get();
    bool res = (*reinterpret_cast<bool *>(response.buffer.data()));
    return res;
  } else {
    log_error("is_enabled_for_ros for sensor", sensor_id, " not found on any server");
    return false;
  }
}

token_type PrimaryCommands::GetToken(stream_id sensor_id) {
  // 搜索传感器是否已在任何辅助服务器中激活
  auto it = _tokens.find(sensor_id);
  if (it != _tokens.end()) {
    // 返回已经激活的传感器令牌
    log_debug("Using token from already activated sensor: ", it->second.get_stream_id(), ", ", it->second.get_port());
    return it->second;
  }
  else {
    // 在一台辅助服务器上启用传感器
    auto server = _router->GetNextServer();
    auto token = SendGetToken(sensor_id);
    // add to the maps
    _tokens[sensor_id] = token;
    _servers[sensor_id] = server;
    log_debug("Using token from new activated sensor: ", token.get_stream_id(), ", ", token.get_port());
    return token;
  }
}

void PrimaryCommands::EnableForROS(stream_id sensor_id) {
  auto it = _servers.find(sensor_id);
  if (it != _servers.end()) {
    SendEnableForROS(sensor_id);
  } else {
    // 我们需要在任何服务器上激活传感器，然后重复
    GetToken(sensor_id);
    EnableForROS(sensor_id);
  }
}

void PrimaryCommands::DisableForROS(stream_id sensor_id) {
  auto it = _servers.find(sensor_id);
  if (it != _servers.end()) {
    SendDisableForROS(sensor_id);
  }
}

bool PrimaryCommands::IsEnabledForROS(stream_id sensor_id) {
  auto it = _servers.find(sensor_id);
  if (it != _servers.end()) {
    return SendIsEnabledForROS(sensor_id);
  }
  return false;
}

} // namespace multigpu
} // namespace carla
