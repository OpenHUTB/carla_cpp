// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/ServerSideSensor.h" // 导入ServerSideSensor类的头文件

#include "carla/Logging.h" // 导入日志记录相关的头文件
#include "carla/client/detail/Simulator.h" // 导入Simulator类的头文件

#include <exception> //导入异常处理的头文件

// 定义一个常量，表示GBuffer的纹理数量
constexpr size_t GBufferTextureCount = 13;

namespace carla {
namespace client {
  
  //ServerSideSensor的析构函数
  ServerSideSensor::~ServerSideSensor() {
    // 检查传感器是否仍在运行且处于监听状态
    if (IsAlive() && IsListening()) {
      // 打印警告信息：传感器对象超出作用域，但传感器仍在模拟中运行
      log_warning(
          "sensor object went out of the scope but the sensor is still alive",
          "in the simulation:",
          GetDisplayId());
    }
    // 如果传感器正在监听并且当前的模拟场景有效
    if (IsListening() && GetEpisode().IsValid()) {
      try {
        // 遍历所有GBuffer纹理，如果正在监听，则停止监听
        for (uint32_t i = 1; i != GBufferTextureCount + 1; ++i) {
          if (listening_mask.test(i)) // 检查是否正在监听
            StopGBuffer(i - 1); // 停止该纹理的监听
        }
        Stop(); // 停止传感器监听
      } catch (const std::exception &e) {
        // 如果发生异常，记录错误信息
        log_error("exception trying to stop sensor:", GetDisplayId(), ':', e.what());
      }
    }
  }
  // Listen函数：开始监听传感器数据流
  void ServerSideSensor::Listen(CallbackFunctionType callback) {
    log_debug("calling sensor Listen() ", GetDisplayId()); // 打印调试信息，表示调用了Listen方法
    log_debug(GetDisplayId(), ": subscribing to stream"); // 记录订阅流的消息
    //锁定当前模拟场景并订阅传感器数据流
    GetEpisode().Lock()->SubscribeToSensor(*this, std::move(callback));
    listening_mask.set(0); // 将监听标志的第0位置为true，表示传感器开始监听
  }

  // stop函数：停止监听传感器数据流
  void ServerSideSensor::Stop() {
    log_debug("calling sensor Stop() ", GetDisplayId()); // 打印调试信息，表示调用了Stop方法
    if (!IsListening()) { 
      // 如果传感器未监听，打印警告信息
      log_warning(
          "attempting to unsubscribe from stream but sensor wasn't listening:",
          GetDisplayId());
      return;
    }
    // 锁定当前模拟场景并取消传感器的订阅
    GetEpisode().Lock()->UnSubscribeFromSensor(*this);
    listening_mask.reset(0); // 将监听标志的第0位置为false，表示停止监听
  }

  // Send函数：向模拟器发送消息
  void ServerSideSensor::Send(std::string message) {
    log_debug("calling sensor Send() ", GetDisplayId()); //记录调试日志，表示调用了send方法
    // 检查传感器类型是否为 "sensor.other.v2x_custom"
    if (GetActorDescription().description.id != "sensor.other.v2x_custom")
     //如果传感器类型不是"sensor.other.v2x_custom",则不支持send方法
    {
      log_warning("Send methods are not supported on non-V2x sensors (sensor.other.v2x_custom).");
      return;
    }
    //通过Episode对象向外发送信息
    GetEpisode().Lock()->Send(*this,message);
  }

  // ListenToGBuffer函数：开始监听GBuffer纹理流
  void ServerSideSensor::ListenToGBuffer(uint32_t GBufferId, CallbackFunctionType callback) {
    log_debug(GetDisplayId(), ": subscribing to gbuffer stream"); //记录订阅GBuffer流的日志
    // 确保GBufferId在有效范围内
    RELEASE_ASSERT(GBufferId < GBufferTextureCount);
    // 检查传感器类型是否为 "sensor.camera.rgb"
    if (GetActorDescription().description.id != "sensor.camera.rgb")
     // 如果传感器不是"sensor.camera.rgb",则不支持GBuffer方法
    {
      log_warning("GBuffer methods are not supported on non-RGB sensors (sensor.camera.rgb).");
      return;
    }
    // 通过Episode对象订阅指定的GBuffer流，并设置回调函数
    GetEpisode().Lock()->SubscribeToGBuffer(*this, GBufferId, std::move(callback));
    // 设置监听掩码的第一位，表示开启了GBuffer监听
    listening_mask.set(0); 
    // 根据GBufferId设置相应的掩码位，标识该GBuffer流被监听
    listening_mask.set(GBufferId + 1); 
  }

  // StopGBuffer函数：停止监听GBuffer纹理流
  void ServerSideSensor::StopGBuffer(uint32_t GBufferId) {
    log_debug(GetDisplayId(), ": unsubscribing from gbuffer stream"); //记录取消订阅GBuffer流的日志
    RELEASE_ASSERT(GBufferId < GBufferTextureCount); // 确保GBufferId在有效范围内
    // 检查传感器类型是否为 "sensor.camera.rgb"
    if (GetActorDescription().description.id != "sensor.camera.rgb")
    // 如果传感器不是"sensor.camera.rgb"，则不支持GBuffer方法
    {
      log_warning("GBuffer methods are not supported on non-RGB sensors (sensor.camera.rgb).");
      return;
    }
    // 通过Episode对象取消订阅指定的GBuffer流
    GetEpisode().Lock()->UnSubscribeFromGBuffer(*this, GBufferId);
    // 重置掩码的相应位，表示停止监听
    listening_mask.reset(GBufferId + 1); 
  }

  // EnableForROS函数：使传感器支持ROS通信
  void ServerSideSensor::EnableForROS() {
    // 通过Episode对象启用传感器的ROS支持
    GetEpisode().Lock()->EnableForROS(*this);
  }

  // DisableForROS函数：禁用传感器对ROS的支持
  void ServerSideSensor::DisableForROS() {
    // 通过Episode对象禁用传感器的ROS支持
    GetEpisode().Lock()->DisableForROS(*this);
  }

  // IsEnabledForROS函数：检查传感器是否启用了ROS支持
  bool ServerSideSensor::IsEnabledForROS(){
    // 检查当前传感器是否启用了ROS支持
    return GetEpisode().Lock()->IsEnabledForROS(*this);
  }

  // Destroy函数：销毁传感器对象
  bool ServerSideSensor::Destroy() {
    log_debug("calling sensor Destroy() ", GetDisplayId()); // 记录调试日志，表示调用了Destroy方法
    // 如果传感器正在监听，则停止监听所有GBuffer流
    if (IsListening()) { 
      for (uint32_t i = 1; i != GBufferTextureCount + 1; ++i) {
        if (listening_mask.test(i)) { // 检查掩码以确认是否有活跃的GBuffer流
          StopGBuffer(i - 1); // 停止每个活跃的GBuffer流
        }
      }
      Stop(); // 停止其他监听任务
    }
    // 调用基类的Destroy方法，完成销毁
    return Actor::Destroy();
  }

} // namespace client
} // namespace carla
