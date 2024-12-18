
} // namespace multigpu
} // namespace carla// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "carla/Logging.h"
#include "carla/multigpu/commands.h"
#include "carla/multigpu/primary.h"
#include "carla/streaming/detail/tcp/Message.h"
#include "carla/streaming/detail/Token.h"
#include "carla/streaming/detail/Types.h"

namespace carla {
namespace multigpu {

// using session = std::shared_ptr<Primary>;
// using callback_response = std::function<void(std::shared_ptr<Primary>, carla::Buffer)>;
using token_type = carla::streaming::detail::token_type;
using stream_id = carla::streaming::detail::stream_id_type;

class Router;

class PrimaryCommands {
  public:


    PrimaryCommands();
// 默认构造函数，用于创建PrimaryCommands类的对象

    PrimaryCommands(std::shared_ptr<Router> router);

    void set_router(std::shared_ptr<Router> router);

    // 向所有辅助服务器广播帧数据
    void SendFrameData(carla::Buffer buffer);

    // 向所有辅助服务器广播要加载的地图
    void SendLoadMap(std::string map);

    // 发送以了解连接是否处于活动状态
    void SendIsAlive();

    token_type GetToken(stream_id sensor_id);
// 向一个辅助节点发送请求以获取指定传感器的令牌，返回获取到的令牌（token_type类型）

    // 管理ROS传感器的启用/禁用
    void EnableForROS(stream_id sensor_id);
// 内部使用的函数，用于向相关节点发送启用与ROS相关的指定传感器（通过传入的传感器流ID来指定）的请求

    void DisableForROS(stream_id sensor_id);

    bool IsEnabledForROS(stream_id sensor_id);

  private:

    // 发送到一个辅助节点以获取传感器的令牌
    token_type SendGetToken(carla::streaming::detail::stream_id_type sensor_id);

    // 管理 ROS 传感器的启用/禁用
    void SendEnableForROS(stream_id sensor_id);
    void SendDisableForROS(stream_id sensor_id);
    bool SendIsEnabledForROS(stream_id sensor_id);


    std::shared_ptr<Router> _router;
    std::unordered_map<stream_id, token_type> _tokens;
    std::unordered_map<stream_id, std::weak_ptr<Primary>> _servers;
};

