// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/ServerSideSensor.h" // 导入ServerSideSensor类的头文件

#include "carla/Logging.h" // 导入日志记录相关的头文件
#include "carla/client/detail/Simulator.h" // 导入Simulator类的头文件

#include <exception> //导入异常处理的头文件

// 定义一个常量表示GBuffer的纹理数量
constexpr size_t GBufferTextureCount = 13;

namespace carla {
namespace client {
  
  //ServerSideSensor的析构函数
  ServerSideSensor::~ServerSideSensor() {
    // 检查传感器是否在运行且处于监听状态
    if (IsAlive() && IsListening()) {
      // 打印警告信息：传感器对象超出作用域，但传感器仍在模拟中运行
      log_warning(
          "sensor object went out of the scope but the sensor is still alive",
          "in the simulation:",
          GetDisplayId());
    }
    // 如果传感器在监听并且当前的模拟场景有效
    if (IsListening() && GetEpisode().IsValid()) {
      try {
        // 遍历所有GBuffer纹理，如果正在监听，则停止监听
        for (uint32_t i = 1; i != GBufferTextureCount + 1; ++i) {
          if (listening_mask.test(i)) // 检查是否正在监听
            StopGBuffer(i - 1); // 停止该纹理的监听
        }
        Stop(); // 停止传感器
      } catch (const std::exception &e) {
        // 如果发生异常，记录错误信息
        log_error("exception trying to stop sensor:", GetDisplayId(), ':', e.what());
      }
    }
  }
  // Listen函数：开始监听传感器数据流
  void ServerSideSensor::Listen(CallbackFunctionType callback) {
    log_debug("calling sensor Listen() ", GetDisplayId()); // 打印调试信息
    log_debug(GetDisplayId(), ": subscribing to stream"); // 记录订阅流的消息
    //锁定当前模拟场景并订阅传感器数据流
    GetEpisode().Lock()->SubscribeToSensor(*this, std::move(callback));
    listening_mask.set(0); // 将监听标志的第0位置为true
  }

  // stop函数：停止监听传感器数据流
  void ServerSideSensor::Stop() {
    log_debug("calling sensor Stop() ", GetDisplayId()); // 打印调试信息
    if (!IsListening()) { 
      // 如果没有监听，打印警告信息
      log_warning(
          "attempting to unsubscribe from stream but sensor wasn't listening:",
          GetDisplayId());
      return;
    }
    // 锁定当前模拟场景并取消传感器的订阅
    GetEpisode().Lock()->UnSubscribeFromSensor(*this);
    listening_mask.reset(0); // 将监听标志的第0位置为false
  }

  void ServerSideSensor::Send(std::string message) {
    log_debug("calling sensor Send() ", GetDisplayId());
    if (GetActorDescription().description.id != "sensor.other.v2x_custom")
    {
      log_warning("Send methods are not supported on non-V2x sensors (sensor.other.v2x_custom).");
      return;
    }
    GetEpisode().Lock()->Send(*this,message);
  }

  void ServerSideSensor::ListenToGBuffer(uint32_t GBufferId, CallbackFunctionType callback) {
    log_debug(GetDisplayId(), ": subscribing to gbuffer stream");
    RELEASE_ASSERT(GBufferId < GBufferTextureCount);
    if (GetActorDescription().description.id != "sensor.camera.rgb")
    {
      log_warning("GBuffer methods are not supported on non-RGB sensors (sensor.camera.rgb).");
      return;
    }
    GetEpisode().Lock()->SubscribeToGBuffer(*this, GBufferId, std::move(callback));
    listening_mask.set(0);
    listening_mask.set(GBufferId + 1);
  }

  void ServerSideSensor::StopGBuffer(uint32_t GBufferId) {
    log_debug(GetDisplayId(), ": unsubscribing from gbuffer stream");
    RELEASE_ASSERT(GBufferId < GBufferTextureCount);
    if (GetActorDescription().description.id != "sensor.camera.rgb")
    {
      log_warning("GBuffer methods are not supported on non-RGB sensors (sensor.camera.rgb).");
      return;
    }
    GetEpisode().Lock()->UnSubscribeFromGBuffer(*this, GBufferId);
    listening_mask.reset(GBufferId + 1);
  }

  void ServerSideSensor::EnableForROS() {
    GetEpisode().Lock()->EnableForROS(*this);
  }

  void ServerSideSensor::DisableForROS() {
    GetEpisode().Lock()->DisableForROS(*this);
  }

  bool ServerSideSensor::IsEnabledForROS(){
    return GetEpisode().Lock()->IsEnabledForROS(*this);
  }

  bool ServerSideSensor::Destroy() {
    log_debug("calling sensor Destroy() ", GetDisplayId());
    if (IsListening()) {
      for (uint32_t i = 1; i != GBufferTextureCount + 1; ++i) {
        if (listening_mask.test(i)) {
          StopGBuffer(i - 1);
        }
      }
      Stop();
    }
    return Actor::Destroy();
  }

} // namespace client
} // namespace carla
