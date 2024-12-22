// 版权所有 （c） 巴塞罗那自治大学 （UAB） 的 2023 计算机视觉中心 （CVC）。
//
// 本作品根据 MIT 许可证的条款进行许可。
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#include "ActorROS2Handler.h"

#include "Carla/Vehicle/CarlaWheeledVehicle.h"// 引入Carla车辆模拟器的WheeledVehicle类定义
#include "Carla/Vehicle/VehicleControl.h"// 引入Carla车辆控制的定义

// 定义一个重载的函数调用运算符，用于将ROS2中的车辆控制命令转化为Carla车辆控制命令。
void ActorROS2Handler::operator()(carla::ros2::VehicleControl &Source)
{
  if (!_Actor) return;// 如果没有有效的Actor对象（可能为空或无效），则直接返回，不进行任何操作。

  // 将Actor对象转换为ACarlaWheeledVehicle类型的指针
  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(_Actor);
  if (!Vehicle) return;// 如果转换失败（即Actor不是车辆对象），则返回

  // 设置控制值
  // 创建一个新的FVehicleControl对象，用于设置车辆的控制参数
  FVehicleControl NewControl;
  // 将ROS2中传来的车辆控制数据填充到FVehicleControl结构体中。
  NewControl.Throttle = Source.throttle; // 设置油门值
  NewControl.Steer = Source.steer;  // 设置转向值
  NewControl.Brake = Source.brake; // 设置刹车值
  NewControl.bHandBrake = Source.hand_brake; // 设置手刹状态
  NewControl.bReverse = Source.reverse; // 设置是否为倒车状态
  NewControl.bManualGearShift = Source.manual_gear_shift; // 设置是否为手动换挡模式
  NewControl.Gear = Source.gear;  // 设置当前的挡位

  // 将新构建的控制命令应用到车辆上，使用用户输入的优先级（EVehicleInputPriority::User）。
  Vehicle->ApplyVehicleControl(NewControl, EVehicleInputPriority::User);
}
