// 版权所有 (c) 2017 巴萨罗那自治大学 (UAB) 计算机视觉中心 (CVC)。  
//  
// 本作品依照 MIT 许可条款进行许可。  
// 许可证副本请见 <https://opensource.org/licenses/MIT>。

#pragma once

#include "carla/client/Sensor.h"
#include <bitset>

namespace carla {
namespace client {

  class ServerSideSensor final : public Sensor {
  public:

    using Sensor::Sensor; // 继承构造函数  

    ~ServerSideSensor();

    /// 注册一个回调函数，当有新的测量值到达时执行。  
    ///   
    /// @warning 如果在已经设置了回调的传感器上调用此函数，  
    /// 会盗取之前设置的回调的数据流。注意，即使在不同的进程中，  
    /// 多个传感器实例也可能指向模拟器中的同一传感器。
    void Listen(CallbackFunctionType callback) override;

    /// 停止监听新的测量值。
    void Stop() override;

    /// 检查此传感器实例是否正在监听与模拟器中的关联传感器。  
    /// @return 如果传感器正在监听，则返回 true；否则返回 false。
    bool IsListening() const override {
      return listening_mask.test(0);// 检查 listening_mask 的第一个位
    }
    /// 开始监听特定的 GBuffer 流。  
    /// @param GBufferId GBuffer 的唯一标识符。  
    /// @param callback 回调函数，当该 GBuffer 有新的数据时调用。
    void ListenToGBuffer(uint32_t GBufferId, CallbackFunctionType callback);

    /// 停止监听特定的 GBuffer 流。  
    /// @param GBufferId 要停止监听的 GBuffer 的唯一标识符。
    void StopGBuffer(uint32_t GBufferId);

    /// 检查是否正在监听特定的 GBuffer。  
    /// @param id GBuffer 的唯一标识符。  
    /// @return 如果正在监听该 GBuffer，则返回 true；否则返回 false。
    inline bool IsListeningGBuffer(uint32_t id) const {
      return listening_mask.test(id + 1); // 检查 listening_mask 的对应位
    }

    /// 为 ROS2 发布启用此传感器。  
    void EnableForROS();

    /// 禁用此传感器用于 ROS2 发布。  
    void DisableForROS();

    /// 检查传感器是否正在为 ROS2 发布。  
    /// @return 如果传感器正在发布，则返回 true；否则返回 false。  
    bool IsEnabledForROS();

    /// 通过此传感器发送数据。  
    void Send(std::string message);

    /// 重写 Actor::Destroy() 方法。  
    /// 此外，确保在销毁传感器之前停止监听。  
    /// @return 返回销毁操作是否成功。
    bool Destroy() override;

  private:

    std::bitset<16> listening_mask;
  };

} // namespace client
} // namespace carla
