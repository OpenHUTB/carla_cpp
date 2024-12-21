// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/primaryCommands.h"

// #include "carla/Logging.h"
// #include "carla/Logging.h"  // 用于日志记录（目前未启用）
#include "carla/multigpu/commands.h"
#include "carla/multigpu/primary.h"
#include "carla/multigpu/router.h"
#include "carla/streaming/detail/tcp/Message.h"
#include "carla/streaming/detail/Token.h"
#include "carla/streaming/detail/Types.h"

namespace carla {
namespace multigpu {

// PrimaryCommands类的默认构造函数，目前为空实现，可能后续用于对象的默认初始化等情况
PrimaryCommands::PrimaryCommands() {
}

// PrimaryCommands类的构造函数，接受一个指向Router的智能指针作为参数，用于初始化成员变量_router
// 参数router: 指向Router对象的智能指针，用于后续的消息路由等操作
PrimaryCommands::PrimaryCommands(std::shared_ptr<Router> router) :
  _router(router) {
}

// 设置路由器（Router）的函数，用于更新当前PrimaryCommands对象所使用的路由器
// 参数router: 新的指向Router对象的智能指针
void PrimaryCommands::set_router(std::shared_ptr<Router> router) {
  _router = router;
}

// 向所有辅助服务器广播帧数据的函数
// 参数buffer: 包含帧数据的carla::Buffer类型对象，会将此数据通过路由器发送给所有辅助服务器
// 实现方式是调用_router的Write方法，传递对应的命令类型（MultiGPUCommand::SEND_FRAME）和要发送的数据（移动语义传递buffer）
void PrimaryCommands::SendFrameData(carla::Buffer buffer) {
  _router->Write(MultiGPUCommand::SEND_FRAME, std::move(buffer));
  // log_info("sending frame command");  // 此处原代码有日志输出，可能用于调试等记录发送帧命令的操作，当前被注释掉了
}

// 向所有辅助服务器广播要加载的地图的函数
// 参数map: 表示地图名称的字符串，先将其转换为carla::Buffer类型，再通过路由器发送给所有辅助服务器
// 转换为Buffer时，会包含字符串内容以及结尾的'\0'字符（通过 + 1 来保证包含结尾字符），然后调用_router的Write方法发送，命令类型为MultiGPUCommand::LOAD_MAP
void PrimaryCommands::SendLoadMap(std::string map) {
  carla::Buffer buf((unsigned char *) map.c_str(), (size_t) map.size() + 1);
  // 使用 _router->Write() 方法将地图加载命令广播给所有辅助服务器，命令类型为 MultiGPUCommand::LOAD_MAP
  _router->Write(MultiGPUCommand::LOAD_MAP, std::move(buf));
}

// 向路由器需要令牌的人员发送请求的函数，用于获取令牌（token）
// 参数sensor_id: 传感器的ID，用于标识请求令牌对应的传感器
// 函数先记录请求令牌的日志信息（log_info），然后将sensor_id放入carla::Buffer中，通过路由器的WriteToNext方法异步发送请求（命令类型为MultiGPUCommand::GET_TOKEN）
// 接着等待异步操作完成（fut.get()）获取响应，从响应中解析出新的令牌（token_type），并记录获取到的令牌信息，最后返回该令牌
token_type PrimaryCommands::SendGetToken(stream_id sensor_id) {
    // 记录请求令牌的日志信息
  log_info("asking for a token");
   // 将 sensor_id 放入 carla::Buffer 中
  carla::Buffer buf((carla::Buffer::value_type *) &sensor_id,
                    (size_t) sizeof(stream_id));
   // 使用 _router->WriteToNext() 异步向下游发送请求，命令类型为 MultiGPUCommand::GET_TOKEN
  auto fut = _router->WriteToNext(MultiGPUCommand::GET_TOKEN, std::move(buf));
// 阻塞当前线程，等待异步响应完成
  auto response = fut.get();
  // 记录令牌信息
  token_type new_token(*reinterpret_cast<carla::streaming::detail::token_data *>(response.buffer.data()));
  log_info("got a token: ", new_token.get_stream_id(), ", ", new_token.get_port());
  return new_token;
}

// 发送以了解连接是否处于活动状态的函数
// 先构造一个简单的询问消息字符串，将其转换为carla::Buffer类型，记录发送命令的日志信息（log_info），然后通过路由器的WriteToNext方法异步发送（命令类型为MultiGPUCommand::YOU_ALIVE）
// 等待异步操作完成获取响应，并记录响应内容的日志信息（log_info）
void PrimaryCommands::SendIsAlive() {
  std::string msg("Are you alive?");
  carla::Buffer buf((unsigned char *) msg.c_str(), (size_t) msg.size());
  log_info("sending is alive command");
  // 使用 _router->WriteToNext() 异步向下游发送该命令，命令类型为 MultiGPUCommand::YOU_ALIVE
  auto fut = _router->WriteToNext(MultiGPUCommand::YOU_ALIVE, std::move(buf));
    // 等待异步操作完成，获取响应并记录日志
  auto response = fut.get();
  log_info("response from alive command: ", response.buffer.data());
}

// 发送启用ROS相关功能的命令给特定传感器所在的辅助服务器（如果传感器已在某辅助服务器中激活）
// 参数sensor_id: 传感器的ID，用于查找对应的辅助服务器
// 首先在已记录的服务器列表（_servers）中查找该传感器是否已在某个辅助服务器中激活，如果找到：
//   - 将sensor_id放入carla::Buffer中，通过路由器的WriteToOne方法异步发送启用命令（命令类型为MultiGPUCommand::ENABLE_ROS）给对应的服务器
//   - 等待异步操作完成获取响应，并从响应中解析出布尔值结果（表示启用操作是否成功等情况）
// 如果没找到对应的服务器，记录错误日志，表示该传感器在任何服务器上都没找到
void PrimaryCommands::SendEnableForROS(stream_id sensor_id) {
  // 搜索传感器是否已在任何辅助服务器中激活
  auto it = _servers.find(sensor_id);
  if (it!= _servers.end()) {
    // 如果找到传感器，封装 sensor_id 成 carla::Buffer
    carla::Buffer buf((carla::Buffer::value_type *) &sensor_id,
                      (size_t) sizeof(stream_id));
    // 使用 _router->WriteToOne() 向目标服务器发送启用 ROS 命令，命令类型为 MultiGPUCommand::ENABLE_ROS
    auto fut = _router->WriteToOne(it->second, MultiGPUCommand::ENABLE_ROS, std::move(buf));
   // 等待并解析响应，获取启用是否成功的布尔值
    auto response = fut.get();
    bool res = (*reinterpret_cast<bool *>(response.buffer.data()));
  } else {
    log_error("enable_for_ros for sensor", sensor_id, " not found on any server");
  }
}

// 发送禁用ROS相关功能的命令给特定传感器所在的辅助服务器（如果传感器已在某辅助服务器中激活）
// 参数sensor_id: 传感器的ID，功能和逻辑与SendEnableForROS类似，只是命令类型变为MultiGPUCommand::DISABLE_ROS，用于禁用操作
void PrimaryCommands::SendDisableForROS(stream_id sensor_id) {
  // 搜索传感器是否已在任何辅助服务器中激活
  auto it = _servers.find(sensor_id);
  if (it!= _servers.end()) {
    // 如果找到传感器，封装 sensor_id 成 carla::Buffer
    carla::Buffer buf((carla::Buffer::value_type *) &sensor_id,
                      (size_t) sizeof(stream_id));
// 使用 _router->WriteToOne() 向目标服务器发送禁用 ROS 命令，命令类型为 MultiGPUCommand::DISABLE_ROS
    auto fut = _router->WriteToOne(it->second, MultiGPUCommand::DISABLE_ROS, std::move(buf));

     // 等待并解析响应，获取禁用是否成功的布尔值
    auto response = fut.get();
    bool res = (*reinterpret_cast<bool *>(response.buffer.data()));
  } else {
    log_error("disable_for_ros for sensor", sensor_id, " not found on any server");
  }
}

// 发送查询特定传感器是否已启用ROS相关功能的命令给对应的辅助服务器（如果传感器已在某辅助服务器中激活），并返回查询结果
// 参数sensor_id: 传感器的ID，同样先查找传感器所在服务器，如果找到：
//   - 通过路由器的WriteToOne方法发送查询命令（命令类型为MultiGPUCommand::IS_ENABLED_ROS），等待响应并解析出布尔值结果（表示是否启用），然后返回该结果
// 如果没找到对应的服务器，记录错误日志，并返回false表示未找到该传感器对应的服务器，默认当作未启用
bool PrimaryCommands::SendIsEnabledForROS(stream_id sensor_id) {
  // 搜索传感器是否已在任何辅助服务器中激活
  auto it = _servers.find(sensor_id);// 在_servers中查找传感器sensor_id对应的服务器
  if (it!= _servers.end()) { // 如果找到了对应的服务器
    //创建缓冲区并通过路由器向服务器发送命令
    carla::Buffer buf((carla::Buffer::value_type *) &sensor_id,
                      (size_t) sizeof(stream_id));
    // 调用WriteToOne方法，发送IS_ENABLED_ROS命令到找到的服务器
    auto fut = _router->WriteToOne(it->second, MultiGPUCommand::IS_ENABLED_ROS, std::move(buf));
    //  等待异步操作完成，获取服务器的响应
    auto response = fut.get();// 等待命令执行完成并获取响应
    // 解析响应数据，获取布尔值结果（是否启用ROS功能）
    bool res = (*reinterpret_cast<bool *>(response.buffer.data()));
    return res; // 返回是否启用了ROS功能
  } else {
     // 如果没有找到对应的服务器，记录错误日志并返回false
    log_error("is_enabled_for_ros for sensor", sensor_id, " not found on any server");
    return false; // 默认认为该传感器没有启用ROS功能
  }
}

// 获取特定传感器的令牌（token）的函数
// 参数sensor_id: 传感器的ID，首先在已记录的令牌列表（_tokens）中查找该传感器是否已有对应的令牌，如果有：
//   - 直接返回已有的令牌（从记录中获取并返回，同时记录日志信息表明使用已激活传感器的令牌）
// 如果没有找到对应的令牌，则执行以下操作：
//   - 通过路由器获取下一个可用的服务器（_router->GetNextServer()）
//   - 调用SendGetToken函数向该服务器请求获取令牌
//   - 将获取到的令牌添加到令牌列表（_tokens）和服务器列表（_servers）中，记录日志信息表明使用新激活传感器的令牌，最后返回该令牌
token_type PrimaryCommands::GetToken(stream_id sensor_id) {
  // 搜索传感器是否已在任何辅助服务器中激活
  auto it = _tokens.find(sensor_id);
  if (it!= _tokens.end()) {
    // 返回已经激活的传感器令牌
    log_debug("Using token from already activated sensor: ", it->second.get_stream_id(), ", ", it->second.get_port());
    return it->second; // 直接返回已找到的令牌
  }
  else {
    // 在一台辅助服务器上启用传感器
    auto server = _router->GetNextServer();
     //  向该服务器请求获取令牌
    auto token = SendGetToken(sensor_id);
    // add to the maps
    // 将获取到的令牌和服务器添加到令牌列表（_tokens）和服务器列表（_servers）中
    _tokens[sensor_id] = token;
    _servers[sensor_id] = server;
    //记录日志，表示新激活传感器的令牌
    log_debug("Using token from new activated sensor: ", token.get_stream_id(), ", ", token.get_port());
     // 返回新的令牌
    return token;
  }
}

// 启用特定传感器的ROS相关功能的函数
// 参数sensor_id: 传感器的ID，首先在服务器列表（_servers）中查找该传感器是否已在某个辅助服务器中激活，如果找到：
//   - 直接调用SendEnableForROS函数发送启用命令
// 如果没找到对应的服务器，则先调用GetToken函数获取该传感器的令牌（这可能会激活传感器并记录相关信息），然后再次调用EnableForROS函数自身（递归调用）尝试启用
void PrimaryCommands::EnableForROS(stream_id sensor_id) {
  auto it = _servers.find(sensor_id); // 查找传感器ID是否在服务器列表中
  if (it!= _servers.end()) {  // 如果在服务器中找到了对应的传感器
    SendEnableForROS(sensor_id);  // 直接调用SendEnableForROS启用该传感器
  } else {
    // 我们需要在任何服务器上激活传感器，然后重复
    GetToken(sensor_id);    // 获取令牌，可能激活传感器并记录信息
    EnableForROS(sensor_id);// 递归调用EnableForROS来重新启用传感器
  }
}

// 禁用特定传感器的ROS相关功能的函数
// 参数sensor_id: 传感器的ID，在服务器列表（_servers）中查找该传感器是否已在某个辅助服务器中激活，如果找到：
//   - 调用SendDisableForROS函数发送禁用命令
void PrimaryCommands::DisableForROS(stream_id sensor_id) {
  auto it = _servers.find(sensor_id);     // 查找传感器ID是否在服务器列表中
  if (it!= _servers.end()) {             // 如果在服务器中找到了对应的传感器
    SendDisableForROS(sensor_id);       // 直接调用SendDisableForROS禁用该传感器
  }
}

// 查询特定传感器是否已启用ROS相关功能的函数
// 参数sensor_id: 传感器的ID，在服务器列表（_servers）中查找该传感器是否已在某个辅助服务器中激活，如果找到：
//   - 调用SendIsEnabledForROS函数发送查询命令，并返回查询结果
// 如果没找到对应的服务器，直接返回false，表示未启用
bool PrimaryCommands::IsEnabledForROS(stream_id sensor_id) {
  auto it = _servers.find(sensor_id); // 查找传感器ID是否在服务器列表中
  if (it!= _servers.end()) {  // 如果在服务器中找到了对应的传感器
    return SendIsEnabledForROS(sensor_id);  // 查询该传感器是否启用了ROS功能
  }
  }
  return false; // 如果没有找到传感器，则返回false，表示未启用
}

} // namespace multigpu
} // namespace carla
