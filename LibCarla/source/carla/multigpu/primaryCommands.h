// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "carla/Logging.h" // 原本可能计划包含用于日志记录相关功能的头文件，但目前被注释掉了，也许后续根据需求会添加进来用于记录相关操作的日志信息
// 包含多GPU相关命令定义的头文件，里面应该定义了在多GPU环境下涉及的各种操作命令相关的数据结构、函数等内容
#include "carla/multigpu/commands.h" // 包含多GPU环境中主节点相关功能的头文件，可能定义了主节点相关的类、接口等，用于处理主节点的操作逻辑
#include "carla/multigpu/primary.h" // 包含流媒体相关的TCP消息定义的头文件，用于处理在网络传输中基于TCP协议的消息相关操作和数据结构
#include "carla/streaming/detail/tcp/Message.h" // 包含流媒体相关的令牌（Token）定义的头文件，Token可能用于标识不同的流媒体会话、资源等，方便进行相关管理和操作
#include "carla/streaming/detail/Token.h" // 包含流媒体相关的类型定义的头文件，里面定义了在流媒体处理过程中用到的各种自定义类型，便于统一类型管理和代码的清晰性
#include "carla/streaming/detail/Types.h"
// 定义在carla命名空间下的multigpu命名空间中，用于组织和限定多GPU相关代码的作用域，避免命名冲突
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
    PrimaryCommands(std::shared_ptr<Router> router);

    void set_router(std::shared_ptr<Router> router);

    // 向所有辅助服务器广播帧数据
    void SendFrameData(carla::Buffer buffer);

    // 向所有辅助服务器广播要加载的地图
    void SendLoadMap(std::string map);

    // 发送以了解连接是否处于活动状态
    void SendIsAlive();

    token_type GetToken(stream_id sensor_id);

    void EnableForROS(stream_id sensor_id);

    void DisableForROS(stream_id sensor_id);

    bool IsEnabledForROS(stream_id sensor_id);

  private:

    // 发送到一个辅助节点以获取传感器的令牌
    token_type SendGetToken(carla::streaming::detail::stream_id_type sensor_id);

    // 管理 ROS 传感器的启用/禁用
    void SendEnableForROS(stream_id sensor_id); // 与SendEnableForROS函数类似，用于向相关节点发送禁用ROS传感器的消息，是DisableForROS函数的底层实现逻辑的一部分，实现关闭ROS相关功能的具体网络通信操作
    void SendDisableForROS(stream_id sensor_id); // 与IsEnabledForROS函数类似，用于向相关节点发送查询请求以了解ROS传感器是否已被启用的消息，并返回查询结果，是IsEnabledForROS函数的底层实现逻辑的一部分，通过网络通信获取当前传感器在ROS方面的启用状态
    bool SendIsEnabledForROS(stream_id sensor_id);


    std::shared_ptr<Router> _router;
    std::unordered_map<stream_id, token_type> _tokens;// 成员变量，使用无序映射（unordered_map）存储传感器流标识（stream_id）与指向Primary类的弱智能指针（std::weak_ptr<Primary>）之间的映射关系，用于关联传感器流和对应的主节点相关信息，弱智能指针可以避免循环引用等问题
    std::unordered_map<stream_id, std::weak_ptr<Primary>> _servers;
};

} // namespace multigpu
} // namespace carla
