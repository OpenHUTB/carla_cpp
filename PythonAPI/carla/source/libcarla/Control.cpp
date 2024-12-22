// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 车辆的Ackermann控制接口，提供了控制车辆转向和加速度等功能
#include <carla/rpc/VehicleAckermannControl.h>

// 车辆的控制接口，提供了一般的车辆控制功能（如油门、刹车、转向等）
#include <carla/rpc/VehicleControl.h>

// 车辆的物理控制接口，控制与车辆物理相关的属性（如悬挂、牵引力、刹车等）
#include <carla/rpc/VehiclePhysicsControl.h>

// 车辆的遥测数据接口，获取车辆的传感器数据，如速度、位置、方向等
#include <carla/rpc/VehicleTelemetryData.h>

// 车辆的轮胎物理控制接口，提供对单个轮胎物理行为的控制（如轮胎摩擦、牵引力等）
#include <carla/rpc/WheelPhysicsControl.h>

// 车辆的轮胎遥测数据接口，获取关于单个轮胎的状态信息（如轮胎转速、温度、压力等）
#include <carla/rpc/WheelTelemetryData.h>

// 步态控制器接口，控制人类或虚拟行走者的运动（如步伐、速度、方向等）
#include <carla/rpc/WalkerControl.h>

// 步态控制器的骨骼输入数据结构，提供行走者骨骼控制的输入信息
#include <carla/rpc/WalkerBoneControlIn.h>

// 步态控制器的骨骼输出数据结构，提供行走者骨骼控制的输出信息
#include <carla/rpc/WalkerBoneControlOut.h>

#include <ostream>  // 引入标准库中的输出流头文件。<ostream> 主要用于定义输出流类（如 std::cout），允许我们将数据输出到控制台或文件等。

namespace carla {
namespace rpc {

  // 将布尔值转换为字符串 "True" 或 "False"
  static auto boolalpha(bool b) {
    return b ? "True" : "False";
  }

  // 为VehicleControl类型重载<<操作符，用于输出车辆控制信息
  // 包括油门(throttle)、方向盘转角(steer)、刹车(brake)、手刹(hand_brake)、倒车(reverse)
  // 是否手动换挡(manual_gear_shift)、当前挡位(gear)
  std::ostream &operator<<(std::ostream &out, const VehicleControl &control) {
    out << "VehicleControl(throttle=" << std::to_string(control.throttle)
        << ", steer=" << std::to_string(control.steer)
        << ", brake=" << std::to_string(control.brake)
        << ", hand_brake=" << boolalpha(control.hand_brake)
        << ", reverse=" << boolalpha(control.reverse)
        << ", manual_gear_shift=" << boolalpha(control.manual_gear_shift)
        << ", gear=" << std::to_string(control.gear) << ')';
    return out;
  }
 
  // 为VehicleAckermannControl类型重载<<操作符，用于输出Ackermann转向控制模型的车辆信息
  // 包括方向盘转角(steer)、转向速度(steer_speed)、速度(speed)、加速度(acceleration)、加加速度(jerk)
  std::ostream& operator<<(std::ostream& out, const VehicleAckermannControl& control) {
      out << "VehicleAckermannControl(steer=" << std::to_string(control.steer)
          << ", steer_speed=" << std::to_string(control.steer_speed)
          << ", speed=" << std::to_string(control.speed)
          << ", acceleration=" << std::to_string(control.acceleration)
          << ", jerk=" << std::to_string(control.jerk) << ')';
      return out;
  }

  // 为WalkerControl类型重载<<操作符，用于输出行人控制信息
  // 包括行人前进方向(direction)、速度(speed)以及是否跳跃(jump)
  std::ostream& operator<<(std::ostream& out, const WalkerControl& control) {
      out << "WalkerControl(direction=" << control.direction
          << ", speed=" << std::to_string(control.speed)
          << ", jump=" << boolalpha(control.jump) << ')';
      return out;
  }

  // 为WalkerBoneControlIn类型重载<<操作符，用于输出行人骨骼控制输入信息
  // 包括骨骼名称和骨骼变换信息(bone_transforms)，每个变换包含骨骼的名称和变换矩阵
  std::ostream& operator<<(std::ostream& out, const WalkerBoneControlIn& control) {
      out << "WalkerBoneControlIn(bone_transforms(";
      for (auto bone_transform : control.bone_transforms) {
          out << "(name=" << bone_transform.first
              << ", transform=" << bone_transform.second << ')';
      }
      out << "))";
      return out;
  }

  // 为BoneTransformDataOut类型重载<<操作符，用于输出骨骼变换数据
  // 包括骨骼名称(bone_name)，骨骼的世界坐标系变换(world)，组件坐标系变换(component)和相对坐标系变换(relative)
  std::ostream& operator<<(std::ostream& out, const BoneTransformDataOut& data) {
      out << "BoneTransformDataOut(name=" << data.bone_name << ", world=" << data.world << ", component=" << data.component << ", relative=" << data.relative << ')';
      return out;
  }

  // 为WalkerBoneControlOut类型重载<<操作符，用于输出行人骨骼控制输出信息
  // 包括骨骼名称(bone_name)，世界坐标系变换(world)，组件坐标系变换(component)，和相对坐标系变换(relative)
  std::ostream& operator<<(std::ostream& out, const WalkerBoneControlOut& control) {
      out << "WalkerBoneControlOut(bone_transforms(";
      for (auto bone_transform : control.bone_transforms) {
          out << "(name=" << bone_transform.bone_name
              << ", world=" << bone_transform.world << ", component=" << bone_transform.component << ", relative=" << bone_transform.relative << ')';
      }
      out << "))";
      return out;
  }

  // 为GearPhysicsControl类型重载<<操作符，用于输出车辆物理控制信息
  // 包括挡位比(ratio)，降挡比(down_ratio)，升挡比(up_ratio)
  std::ostream& operator<<(std::ostream& out, const GearPhysicsControl& control) {
      out << "GearPhysicsControl(ratio=" << std::to_string(control.ratio)
          << ", down_ratio=" << std::to_string(control.down_ratio)
          << ", up_ratio=" << std::to_string(control.up_ratio) << ')';
      return out;
  }

  // 为WheelPhysicsControl类型重载<<操作符，用于输出车轮物理控制信息
  // 包括轮胎摩擦力(tire_friction)，阻尼率(damping_rate)，最大转向角(max_steer_angle)
  // 半径(radius)，最大刹车扭矩(max_brake_torque)，最大手刹扭矩(max_handbrake_torque)
  // 横向刚度最大载荷(lat_stiff_max_load)，横向刚度(lat_stiff_value)，纵向刚度(long_stiff_value)
  // 车轮位置(position)
  std::ostream &operator<<(std::ostream &out, const WheelPhysicsControl &control) {
    out << "WheelPhysicsControl(tire_friction=" << std::to_string(control.tire_friction)
        << ", damping_rate=" << std::to_string(control.damping_rate)
        << ", max_steer_angle=" << std::to_string(control.max_steer_angle)
        << ", radius=" << std::to_string(control.radius)
        << ", max_brake_torque=" << std::to_string(control.max_brake_torque)
        << ", max_handbrake_torque=" << std::to_string(control.max_handbrake_torque)
        << ", lat_stiff_max_load=" << std::to_string(control.lat_stiff_max_load)
        << ", lat_stiff_value=" << std::to_string(control.lat_stiff_value)
        << ", long_stiff_value=" << std::to_string(control.long_stiff_value)
        << ", position=" << control.position << ')';
 // 输出VehiclePhysicsControl对象的所有成员变量    
    return out;
  }

// 为VehiclePhysicsControl类型重载<<操作符
  std::ostream &operator<<(std::ostream &out, const VehiclePhysicsControl &control) {
    out << "VehiclePhysicsControl(torque_curve=" << control.torque_curve
    << ", max_rpm=" << std::to_string(control.max_rpm)
    << ", moi=" << std::to_string(control.moi)
    << ", damping_rate_full_throttle=" << std::to_string(control.damping_rate_full_throttle)
    << ", damping_rate_zero_throttle_clutch_engaged=" << std::to_string(control.damping_rate_zero_throttle_clutch_engaged)
    << ", damping_rate_zero_throttle_clutch_disengaged=" << std::to_string(control.damping_rate_zero_throttle_clutch_disengaged)
    << ", use_gear_autobox=" << boolalpha(control.use_gear_autobox)
    << ", gear_switch_time=" << std::to_string(control.gear_switch_time)
    << ", clutch_strength=" << std::to_string(control.clutch_strength)
    << ", final_ratio=" << std::to_string(control.final_ratio)
    << ", forward_gears=" << control.forward_gears
    << ", mass=" << std::to_string(control.mass)
    << ", drag_coefficient=" << std::to_string(control.drag_coefficient)
    << ", center_of_mass=" << control.center_of_mass
    << ", steering_curve=" << control.steering_curve
    << ", wheels=" << control.wheels
    << ", use_sweep_wheel_collision=" << control.use_sweep_wheel_collision << ')';
// 输出VehiclePhysicsControl对象的所有成员变量
    return out;
  }

// 为WheelTelemetryData类型重载<<操作符
  std::ostream &operator<<(std::ostream &out, const WheelTelemetryData &telemetry) {
    out << "WheelTelemetryData(tire_friction=" << std::to_string(telemetry.tire_friction)
        << ", lat_slip=" << std::to_string(telemetry.lat_slip)
        << ", long_slip=" << std::to_string(telemetry.long_slip)
        << ", omega=" << std::to_string(telemetry.omega)
        << ", tire_load=" << std::to_string(telemetry.tire_load)
        << ", normalized_tire_load=" << std::to_string(telemetry.normalized_tire_load)
        << ", torque=" << std::to_string(telemetry.torque)
        << ", long_force=" << std::to_string(telemetry.long_force)
        << ", lat_force=" << std::to_string(telemetry.lat_force)
        << ", normalized_long_force=" << std::to_string(telemetry.normalized_long_force)
        << ", normalized_lat_force=" << std::to_string(telemetry.normalized_lat_force) << ')';
// 输出WheelTelemetryData对象的所有成员变量
    return out;
  }

// 为VehicleTelemetryData类型重载<<操作符
  std::ostream &operator<<(std::ostream &out, const VehicleTelemetryData &telemetry) {
    out << "VehicleTelemetryData(speed=" << std::to_string(telemetry.speed)
    << ", steer=" << std::to_string(telemetry.steer)
    << ", throttle=" << std::to_string(telemetry.throttle)
    << ", brake=" << std::to_string(telemetry.brake)
    << ", engine_rpm=" << std::to_string(telemetry.engine_rpm)
    << ", gear=" << std::to_string(telemetry.gear)
    << ", drag=" << std::to_string(telemetry.drag)
    << ", wheels=" << telemetry.wheels << ')';
// 输出VehicleTelemetryData对象的所有成员变量
    return out;
  }

// 为AckermannControllerSettings类型重载<<操作符
  std::ostream &operator<<(std::ostream &out, const AckermannControllerSettings &settings) {
    out << "AckermannControllerSettings(speed_kp=" << std::to_string(settings.speed_kp)
        << ", speed_ki=" << std::to_string(settings.speed_ki)
        << ", speed_kd=" << std::to_string(settings.speed_kd)
        << ", accel_kp=" << std::to_string(settings.accel_kp)
        << ", accel_ki=" << std::to_string(settings.accel_ki)
        << ", accel_kd=" << std::to_string(settings.accel_kd)  << ')';
// 输出AckermannControllerSettings对象的所有成员变量
    return out;
  }

} // namespace rpc
} // namespace carla

// 定义一个静态函数，用于从Python列表中提取carla::geom::Vector2D对象或数据，并转换为std::vector<carla::geom::Vector2D>
static auto GetVectorOfVector2DFromList(const boost::python::list &list) {
  std::vector<carla::geom::Vector2D> v; // 创建一个Vector2D的向量
 
  // 获取Python列表的长度
  auto length = boost::python::len(list);
  // 为向量预留足够的空间以提高效率
  v.reserve(static_cast<size_t>(length));
  // 遍历Python列表
  for (auto i = 0u; i < length; ++i) {
    // 尝试从列表中提取Vector2D对象
    boost::python::extract<carla::geom::Vector2D> ext(list[i]);
    if (ext.check()) {
      // 如果提取成功，添加到向量中
      v.push_back(ext);
    } else {
      // 如果提取失败，尝试从列表中提取两个浮点数（假设是Vector2D的x和y坐标）
      v.push_back(carla::geom::Vector2D{
        boost::python::extract<float>(list[i][0u]), // x坐标
        boost::python::extract<float>(list[i][1u])}); // y坐标
    }
  }
  // 返回转换后的向量
  return v;
}
 
// 定义一个静态函数，用于从Python列表中提取carla::rpc::BoneTransformDataIn对象或数据，并转换为std::vector<carla::rpc::BoneTransformDataIn>
static auto GetVectorOfBoneTransformFromList(const boost::python::list &list) {
  std::vector<carla::rpc::BoneTransformDataIn> v; // 创建一个BoneTransformDataIn的向量
 
  // 获取Python列表的长度
  auto length = boost::python::len(list);
  // 为向量预留足够的空间
  v.reserve(static_cast<size_t>(length));
  // 遍历Python列表
  for (auto i = 0u; i < length; ++i) {
    // 尝试从列表中提取BoneTransformDataIn对象
    boost::python::extract<carla::rpc::BoneTransformDataIn> ext(list[i]);
    if (ext.check()) {
      // 如果提取成功，添加到向量中
      v.push_back(ext);
    } else {
      // 如果提取失败，尝试从列表中提取字符串和Transform对象（假设是BoneTransformDataIn的name和transform）
      v.push_back(carla::rpc::BoneTransformDataIn{
        boost::python::extract<std::string>(list[i][0u]), // name
        boost::python::extract<carla::geom::Transform>(list[i][1u])}); // transform
    }
  }
  // 返回转换后的向量
  return v;
}
 
// 定义一个静态函数，用于从VehiclePhysicsControl对象中获取车轮信息，并将其转换为Python列表
static auto GetWheels(const carla::rpc::VehiclePhysicsControl &self) {
  const auto &wheels = self.GetWheels(); // 获取车轮信息
  // 使用Boost.Python的迭代器功能将C++的vector转换为Python的迭代器
  boost::python::object get_iter = boost::python::iterator<std::vector<carla::rpc::WheelPhysicsControl>>();
  boost::python::object iter = get_iter(wheels);
  // 将迭代器转换为Python列表并返回
  return boost::python::list(iter);
}
 
// 定义一个静态函数，用于设置VehiclePhysicsControl对象的车轮信息，信息来源于Python列表
static void SetWheels(carla::rpc::VehiclePhysicsControl &self, const boost::python::list &list) {
  std::vector<carla::rpc::WheelPhysicsControl> wheels; // 创建一个WheelPhysicsControl的向量
  // 获取Python列表的长度
  auto length = boost::python::len(list);
  // 遍历Python列表
  for (auto i = 0u; i < length; ++i) {
    // 从列表中提取WheelPhysicsControl对象并添加到向量中
    wheels.push_back(boost::python::extract<carla::rpc::WheelPhysicsControl &>(list[i]));
  }
  // 设置VehiclePhysicsControl对象的车轮信息
  self.wheels = wheels;
}
 
// 定义一个静态函数，用于从VehiclePhysicsControl对象中获取前进挡信息，并将其转换为Python列表
static auto GetForwardGears(const carla::rpc::VehiclePhysicsControl &self) {
  const auto &gears = self.GetForwardGears(); // 获取前进挡信息
  // 使用Boost.Python的迭代器功能将C++的vector转换为Python的迭代器
  boost::python::object get_iter = boost::python::iterator<std::vector<carla::rpc::GearPhysicsControl>>();
  boost::python::object iter = get_iter(gears);
  // 将迭代器转换为Python列表并返回
  return boost::python::list(iter);
}

// 设置车辆的前进挡位
static void SetForwardGears(carla::rpc::VehiclePhysicsControl &self, const boost::python::list &list) {
  std::vector<carla::rpc::GearPhysicsControl> gears; // 用于存储挡位信息的向量
  auto length = boost::python::len(list); // 获取Python列表的长度
  for (auto i = 0u; i < length; ++i) {
    // 将Python列表中的每个元素转换为carla::rpc::GearPhysicsControl类型，并添加到gears向量中
    gears.push_back(boost::python::extract<carla::rpc::GearPhysicsControl &>(list[i]));
  }
  self.SetForwardGears(gears); // 使用转换后的挡位信息设置车辆的前进挡位
}

// 获取车辆的扭矩曲线
static auto GetTorqueCurve(const carla::rpc::VehiclePhysicsControl &self) {
  const std::vector<carla::geom::Vector2D> &torque_curve = self.GetTorqueCurve(); // 获取扭矩曲线
  boost::python::object get_iter = boost::python::iterator<const std::vector<carla::geom::Vector2D>>(); // 创建一个迭代器对象
  boost::python::object iter = get_iter(torque_curve); // 获取扭矩曲线的迭代器
  return boost::python::list(iter); // 将迭代器转换为Python列表并返回
}


// 设置车辆的扭矩曲线
static void SetTorqueCurve(carla::rpc::VehiclePhysicsControl &self, const boost::python::list &list) {
  // 将Python列表转换为std::vector<carla::geom::Vector2D>类型，并设置给扭矩曲线
  self.torque_curve = GetVectorOfVector2DFromList(list); // 注意：GetVectorOfVector2DFromList函数未在代码中定义，可能是一个辅助函数
}


// 获取车辆的转向曲线
static auto GetSteeringCurve(const carla::rpc::VehiclePhysicsControl &self) {
  const std::vector<carla::geom::Vector2D> &steering_curve = self.GetSteeringCurve(); // 获取转向曲线
  boost::python::object get_iter = boost::python::iterator<const std::vector<carla::geom::Vector2D>>(); // 创建一个迭代器对象
  boost::python::object iter = get_iter(steering_curve); // 获取转向曲线的迭代器
  return boost::python::list(iter); // 将迭代器转换为Python列表并返回
}


// 设置车辆的转向曲线
static void SetSteeringCurve(carla::rpc::VehiclePhysicsControl &self, const boost::python::list &list) {
  // 将Python列表转换为std::vector<carla::geom::Vector2D>类型，并设置给转向曲线
  self.steering_curve = GetVectorOfVector2DFromList(list); // 注意：GetVectorOfVector2DFromList函数未在代码中定义，可能是一个辅助函数
}

// 初始化VehiclePhysicsControl对象
boost::python::object VehiclePhysicsControl_init(boost::python::tuple args, boost::python::dict kwargs) {
  // 定义参数名称
  const uint32_t NUM_ARGUMENTS = 17;
  const char *args_names[NUM_ARGUMENTS] = {
    "torque_curve",
    "max_rpm",
    "moi",
    "damping_rate_full_throttle",
    "damping_rate_zero_throttle_clutch_engaged",
    "damping_rate_zero_throttle_clutch_disengaged",

    "use_gear_autobox",
    "gear_switch_time",
    "clutch_strength",
    "final_ratio",
    "forward_gears",

    "mass",
    "drag_coefficient",

    "center_of_mass",
    "steering_curve",
    "wheels",
    "use_sweep_wheel_collision",
  };

  boost::python::object self = args[0]; // 获取self对象
  args = boost::python::tuple(args.slice(1, boost::python::_)); // 移除self后的参数
 
  auto res = self.attr("__init__")(); // 调用Python的__init__方法
  // 处理位置参数
  if (len(args) > 0) {
    for (unsigned int i = 0; i < len(args); ++i) {
      self.attr(args_names[i]) = args[i]; // 将位置参数设置到对应的属性上
    }
  }
  // 处理关键字参数
  for (unsigned int i = 0; i < NUM_ARGUMENTS; ++i) {
    if (kwargs.contains(args_names[i])) {
      self.attr(args_names[i]) = kwargs[args_names[i]]; // 将关键字参数设置到对应的属性上
    }
  }
 
  return res;
}

// 获取车辆的轮胎遥测数据
static auto GetWheelsTelemetry(const carla::rpc::VehicleTelemetryData &self) {
  const auto &wheels = self.GetWheels(); // 获取轮胎遥测数据
  boost::python::object get_iter = boost::python::iterator<std::vector<carla::rpc::WheelTelemetryData>>(); // 创建一个迭代器对象
  boost::python::object iter = get_iter(wheels); // 获取轮胎遥测数据的迭代器
  return boost::python::list(iter); // 将迭代器转换为Python列表并返回
}

// 设置车辆的轮胎遥测数据
static void SetWheelsTelemetry(carla::rpc::VehicleTelemetryData &self, const boost::python::list &list) {
  std::vector<carla::rpc::WheelTelemetryData> wheels; // 用于存储轮胎遥测数据的向量
  auto length = boost::python::len(list); // 获取Python列表的长度
  for (auto i = 0u; i < length; ++i) {
    // 将Python列表中的每个元素转换为carla::rpc::WheelTelemetryData类型，并添加到wheels向量中
    wheels.push_back(boost::python::extract<carla::rpc::WheelTelemetryData &>(list[i]));
  }
  self.SetWheels(wheels); // 使用转换后的轮胎遥测数据设置车辆的轮胎信息
}
// 初始化VehicleTelemetryData对象
boost::python::object VehicleTelemetryData_init(boost::python::tuple args, boost::python::dict kwargs) {
  // 定义参数名称
  const uint32_t NUM_ARGUMENTS = 7;
  const char *args_names[NUM_ARGUMENTS] = {
    "speed",
    "steer",
    "throttle",
    "brake",
    "engine_rpm",
    "gear",
    "wheels"
  };

  boost::python::object self = args[0];
  args = boost::python::tuple(args.slice(1, boost::python::_));

  auto res = self.attr("__init__")();
  if (len(args) > 0) {
    for (unsigned int i = 0; i < len(args); ++i) {
      self.attr(args_names[i]) = args[i];
    }
  }

  for (unsigned int i = 0; i < NUM_ARGUMENTS; ++i) {
    if (kwargs.contains(args_names[i])) {
      self.attr(args_names[i]) = kwargs[args_names[i]];
    }
  }

  return res;
}
// 获取Walker的骨骼变换信息（输入）
static auto GetBonesTransform(const carla::rpc::WalkerBoneControlIn &self) {
  // 获取骨骼变换信息，并转换为Python列表返回
  const std::vector<carla::rpc::BoneTransformDataIn> &bone_transform_data = self.bone_transforms;
  boost::python::object get_iter =
      boost::python::iterator<const std::vector<carla::rpc::BoneTransformDataIn>>();
  boost::python::object iter = get_iter(bone_transform_data);
  return boost::python::list(iter);
}
// 设置Walker的骨骼变换信息（输入）
static void SetBonesTransform(carla::rpc::WalkerBoneControlIn &self, const boost::python::list &list) {
  // 将Python列表转换为骨骼变换信息，并设置
  self.bone_transforms = GetVectorOfBoneTransformFromList(list);
}

// 获取Walker的骨骼变换信息（输出）
static auto GetBonesTransformOut(const carla::rpc::WalkerBoneControlOut &self) {
  // 获取骨骼变换信息，并转换为Python列表返回
  const std::vector<carla::rpc::BoneTransformDataOut> &bone_transform_data = self.bone_transforms;
  boost::python::object get_iter =
      boost::python::iterator<const std::vector<carla::rpc::BoneTransformDataOut>>();
  boost::python::object iter = get_iter(bone_transform_data);
  return boost::python::list(iter);
}

// 初始化WalkerBoneControl对象
boost::python::object WalkerBoneControl_init(boost::python::tuple args, boost::python::dict kwargs) {
  // 定义参数名称
  const uint32_t NUM_ARGUMENTS = 1;
  const char *args_names[NUM_ARGUMENTS] = {
    "bone_transforms"
  };

  boost::python::object self = args[0];
  args = boost::python::tuple(args.slice(1, boost::python::_));

  auto res = self.attr("__init__")();
  if (len(args) > 0) {
    for (unsigned int i = 0; i < len(args); ++i) {
      self.attr(args_names[i]) = args[i];
    }
  }

  for (unsigned int i = 0; i < NUM_ARGUMENTS; ++i) {
    if (kwargs.contains(args_names[i])) {
      self.attr(args_names[i]) = kwargs[args_names[i]];
    }
  }

  return res;
}

// 定义一个函数export_control，这个函数没有返回值，也没有参数
void export_control() {
  // 使用boost::python命名空间，简化代码中的引用
  using namespace boost::python;
  // 为方便引用，为命名空间创建别名
  namespace cg = carla::geom; // carla的几何命名空间
  namespace cr = carla::rpc;  // carla的RPC（远程过程调用）命名空间

  // 将cr::VehicleControl类暴露给Python
  class_<cr::VehicleControl>("VehicleControl")
    // 定义一个构造函数，并设置参数的默认值
    .def(init<float, float, float, bool, bool, bool, int>(
      (arg("throttle") = 0.0f,
      arg("steer") = 0.0f,
      arg("brake") = 0.0f,
      arg("hand_brake") = false,
      arg("reverse") = false,
      arg("manual_gear_shift") = false,
      arg("gear") = 0)))
    // 为类的成员变量添加读写属性
    .def_readwrite("throttle", &cr::VehicleControl::throttle)
    .def_readwrite("steer", &cr::VehicleControl::steer)
    .def_readwrite("brake", &cr::VehicleControl::brake)
    .def_readwrite("hand_brake", &cr::VehicleControl::hand_brake)
    .def_readwrite("reverse", &cr::VehicleControl::reverse)
    .def_readwrite("manual_gear_shift", &cr::VehicleControl::manual_gear_shift)
    .def_readwrite("gear", &cr::VehicleControl::gear)
    .def("__eq__", &cr::VehicleControl::operator==)
    .def("__ne__", &cr::VehicleControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;
  // 将cr::VehicleAckermannControl类暴露给Python
  class_<cr::VehicleAckermannControl>("VehicleAckermannControl")
    .def(init<float, float, float, float, float>(
      (arg("steer") = 0.0f,
      arg("steer_speed") = 0.0f,
      arg("speed") = 0.0f,
      arg("acceleration") = 0.0f,
      arg("jerk") = 0.0f)))
    .def_readwrite("steer", &cr::VehicleAckermannControl::steer)
    .def_readwrite("steer_speed", &cr::VehicleAckermannControl::steer_speed)
    .def_readwrite("speed", &cr::VehicleAckermannControl::speed)
    .def_readwrite("acceleration", &cr::VehicleAckermannControl::acceleration)
    .def_readwrite("jerk", &cr::VehicleAckermannControl::jerk)
    .def("__eq__", &cr::VehicleAckermannControl::operator==)
    .def("__ne__", &cr::VehicleAckermannControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;
  // 将cr::AckermannControllerSettings类暴露给Python
  class_<cr::AckermannControllerSettings>("AckermannControllerSettings")
    .def(init<float, float, float, float, float, float>(
      (arg("speed_kp") = 0.0f,
      arg("speed_ki") = 0.0f,
      arg("speed_kd") = 0.0f,
      arg("accel_kp") = 0.0f,
      arg("accel_ki") = 0.0f,
      arg("accel_kd") = 0.0f)))
    .def_readwrite("speed_kp", &cr::AckermannControllerSettings::speed_kp)
    .def_readwrite("speed_ki", &cr::AckermannControllerSettings::speed_ki)
    .def_readwrite("speed_kd", &cr::AckermannControllerSettings::speed_kd)
    .def_readwrite("accel_kp", &cr::AckermannControllerSettings::accel_kp)
    .def_readwrite("accel_ki", &cr::AckermannControllerSettings::accel_ki)
    .def_readwrite("accel_kd", &cr::AckermannControllerSettings::accel_kd)
    .def("__eq__", &cr::AckermannControllerSettings::operator==)
    .def("__ne__", &cr::AckermannControllerSettings::operator!=)
    .def(self_ns::str(self_ns::self))
  ;
  // 将cr::WalkerControl类暴露给Python
  class_<cr::WalkerControl>("WalkerControl")
    .def(init<cg::Vector3D, float, bool>(
       (arg("direction") = cg::Vector3D{1.0f, 0.0f, 0.0f},
       arg("speed") = 0.0f,
       arg("jump") = false)))
    .def_readwrite("direction", &cr::WalkerControl::direction)
    .def_readwrite("speed", &cr::WalkerControl::speed)
    .def_readwrite("jump", &cr::WalkerControl::jump)
    .def("__eq__", &cr::WalkerControl::operator==)
    .def("__ne__", &cr::WalkerControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  // 将std::pair<std::string, cg::Transform>类型的类（别名为bone_transform）暴露给Python
  class_<cr::BoneTransformDataIn>("bone_transform")
    .def(init<>())
    .def_readwrite("name", &std::pair<std::string, cg::Transform>::first)
    .def_readwrite("transform", &std::pair<std::string, cg::Transform>::second)
    .def(self_ns::str(self_ns::self))
  ;

  // 将std::vector<cr::BoneTransformDataIn>类型的类（别名为vector_of_bones）暴露给Python
  class_<std::vector<cr::BoneTransformDataIn>>("vector_of_bones")
    .def(init<>())
    .def(boost::python::vector_indexing_suite<std::vector<cr::BoneTransformDataIn>>())
    .def(self_ns::str(self_ns::self))
  ;

  // 将cr::BoneTransformDataOut类暴露给Python
  class_<cr::BoneTransformDataOut>("bone_transform_out")
    .def(init<>())
    .def_readwrite("name", &cr::BoneTransformDataOut::bone_name)
    .def_readwrite("world", &cr::BoneTransformDataOut::world)
    .def_readwrite("component", &cr::BoneTransformDataOut::component)
    .def_readwrite("relative", &cr::BoneTransformDataOut::relative)
    .def(self_ns::str(self_ns::self))
    .def("__eq__", &cr::BoneTransformDataOut::operator==)
    .def("__ne__", &cr::BoneTransformDataOut::operator!=)
  ;

  // 将std::vector<cr::BoneTransformDataOut>类型的类（别名为vector_of_bones_out）暴露给Python
  class_<std::vector<cr::BoneTransformDataOut>>("vector_of_bones_out")
    .def(init<>())
    .def(boost::python::vector_indexing_suite<std::vector<cr::BoneTransformDataOut>>())
    .def(self_ns::str(self_ns::self))
  ;

  // 将cr::WalkerBoneControlIn类暴露给Python
  // 注意：这里使用了raw_function来处理特殊的初始化逻辑
  class_<cr::WalkerBoneControlIn>("WalkerBoneControlIn")
    .def("__init__", raw_function(WalkerBoneControl_init))
    .def(init<>())
    .add_property("bone_transforms", &GetBonesTransform, &SetBonesTransform)
    .def(self_ns::str(self_ns::self))
  ;

  // 将cr::WalkerBoneControlOut类暴露给Python
  // 注意：这里只提供了getter而没有setter，或者使用了不同的处理逻辑
  class_<cr::WalkerBoneControlOut>("WalkerBoneControlOut")
    .def("__init__", raw_function(WalkerBoneControl_init))
    .def(init<>())
    // .add_property("bone_transforms", &GetBonesTransformOut, &SetBonesTransformOut)
    .add_property("bone_transforms", &GetBonesTransformOut)
    .def(self_ns::str(self_ns::self))
  ;

 // 将std::vector<cr::GearPhysicsControl>类型的类（别名为vector_of_gears）暴露给Python
  class_<std::vector<cr::GearPhysicsControl>>("vector_of_gears")
      .def(boost::python::vector_indexing_suite<std::vector<cr::GearPhysicsControl>>())
      .def(self_ns::str(self_ns::self))
  ;

  class_<cr::GearPhysicsControl>("GearPhysicsControl")
    .def(init<float, float, float>(
        (arg("ratio")=1.0f,
         arg("down_ratio")=0.5f,
         arg("up_ratio")=0.65f)))
    .def_readwrite("ratio", &cr::GearPhysicsControl::ratio)
    .def_readwrite("down_ratio", &cr::GearPhysicsControl::down_ratio)
    .def_readwrite("up_ratio", &cr::GearPhysicsControl::up_ratio)
    .def("__eq__", &cr::GearPhysicsControl::operator==)
    .def("__ne__", &cr::GearPhysicsControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<std::vector<cr::WheelPhysicsControl>>("vector_of_wheels")
    .def(boost::python::vector_indexing_suite<std::vector<cr::WheelPhysicsControl>>())
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::WheelPhysicsControl>("WheelPhysicsControl")
    .def(init<float, float, float, float, float, float, float, float, float, cg::Vector3D>(
        (arg("tire_friction")=2.0f,
         arg("damping_rate")=0.25f,
         arg("max_steer_angle")=70.0f,
         arg("radius")=30.0f,
         arg("max_brake_torque")=1500.0f,
         arg("max_handbrake_torque")=3000.0f,
         arg("lat_stiff_max_load")=2.0f,
         arg("lat_stiff_value")=17.0f,
         arg("long_stiff_value")=1000.0f,
         arg("position")=cg::Vector3D{0.0f, 0.0f, 0.0f})))
    .def_readwrite("tire_friction", &cr::WheelPhysicsControl::tire_friction)
    .def_readwrite("damping_rate", &cr::WheelPhysicsControl::damping_rate)
    .def_readwrite("max_steer_angle", &cr::WheelPhysicsControl::max_steer_angle)
    .def_readwrite("radius", &cr::WheelPhysicsControl::radius)
    .def_readwrite("max_brake_torque", &cr::WheelPhysicsControl::max_brake_torque)
    .def_readwrite("max_handbrake_torque", &cr::WheelPhysicsControl::max_handbrake_torque)
    .def_readwrite("lat_stiff_max_load", &cr::WheelPhysicsControl::lat_stiff_max_load)
    .def_readwrite("lat_stiff_value", &cr::WheelPhysicsControl::lat_stiff_value)
    .def_readwrite("long_stiff_value", &cr::WheelPhysicsControl::long_stiff_value)
    .def_readwrite("position", &cr::WheelPhysicsControl::position)
    .def("__eq__", &cr::WheelPhysicsControl::operator==)
    .def("__ne__", &cr::WheelPhysicsControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::VehiclePhysicsControl>("VehiclePhysicsControl", no_init)
    .def("__init__", raw_function(VehiclePhysicsControl_init))
    .def(init<>())
    .add_property("torque_curve", &GetTorqueCurve, &SetTorqueCurve)
    .def_readwrite("max_rpm", &cr::VehiclePhysicsControl::max_rpm)
    .def_readwrite("moi", &cr::VehiclePhysicsControl::moi)
    .def_readwrite("damping_rate_full_throttle",
        &cr::VehiclePhysicsControl::damping_rate_full_throttle)
    .def_readwrite("damping_rate_zero_throttle_clutch_engaged",
        &cr::VehiclePhysicsControl::damping_rate_zero_throttle_clutch_engaged)
    .def_readwrite("damping_rate_zero_throttle_clutch_disengaged",
        &cr::VehiclePhysicsControl::damping_rate_zero_throttle_clutch_disengaged)
    .def_readwrite("use_gear_autobox", &cr::VehiclePhysicsControl::use_gear_autobox)
    .def_readwrite("gear_switch_time", &cr::VehiclePhysicsControl::gear_switch_time)
    .def_readwrite("clutch_strength", &cr::VehiclePhysicsControl::clutch_strength)
    .def_readwrite("final_ratio", &cr::VehiclePhysicsControl::final_ratio)
    .add_property("forward_gears", &GetForwardGears, &SetForwardGears)
    .def_readwrite("mass", &cr::VehiclePhysicsControl::mass)
    .def_readwrite("drag_coefficient", &cr::VehiclePhysicsControl::drag_coefficient)
    .def_readwrite("center_of_mass", &cr::VehiclePhysicsControl::center_of_mass)
    .add_property("steering_curve", &GetSteeringCurve, &SetSteeringCurve)
    .add_property("wheels", &GetWheels, &SetWheels)
    .def_readwrite("use_sweep_wheel_collision", &cr::VehiclePhysicsControl::use_sweep_wheel_collision)
    .def("__eq__", &cr::VehiclePhysicsControl::operator==)
    .def("__ne__", &cr::VehiclePhysicsControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::WheelTelemetryData>("WheelTelemetryData")
    .def(init<float, float, float, float, float, float, float, float, float, float, float>(
        (arg("tire_friction")=0.0f,
         arg("lat_slip")=0.0f,
         arg("long_slip")=0.0f,
         arg("omega")=0.0f,
         arg("tire_load")=0.0f,
         arg("normalized_tire_load")=0.0f,
         arg("torque")=0.0f,
         arg("long_force")=0.0f,
         arg("lat_force")=0.0f,
         arg("normalized_long_force")=0.0f,
         arg("normalized_lat_force")=0.0f)))
    .def_readwrite("tire_friction", &cr::WheelTelemetryData::tire_friction)
    .def_readwrite("lat_slip", &cr::WheelTelemetryData::lat_slip)
    .def_readwrite("long_slip", &cr::WheelTelemetryData::long_slip)
    .def_readwrite("omega", &cr::WheelTelemetryData::omega)
    .def_readwrite("tire_load", &cr::WheelTelemetryData::tire_load)
    .def_readwrite("normalized_tire_load", &cr::WheelTelemetryData::normalized_tire_load)
    .def_readwrite("torque", &cr::WheelTelemetryData::torque)
    .def_readwrite("long_force", &cr::WheelTelemetryData::long_force)
    .def_readwrite("lat_force", &cr::WheelTelemetryData::lat_force)
    .def_readwrite("normalized_long_force", &cr::WheelTelemetryData::normalized_long_force)
    .def_readwrite("normalized_lat_force", &cr::WheelTelemetryData::normalized_lat_force)
    .def("__eq__", &cr::WheelTelemetryData::operator==)
    .def("__ne__", &cr::WheelTelemetryData::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::VehicleTelemetryData>("VehicleTelemetryData", no_init)
    .def("__init__", raw_function(VehicleTelemetryData_init))
    .def(init<>())
    .def_readwrite("speed", &cr::VehicleTelemetryData::speed)
    .def_readwrite("steer", &cr::VehicleTelemetryData::steer)
    .def_readwrite("throttle", &cr::VehicleTelemetryData::throttle)
    .def_readwrite("brake", &cr::VehicleTelemetryData::brake)
    .def_readwrite("engine_rpm", &cr::VehicleTelemetryData::engine_rpm)
    .def_readwrite("gear", &cr::VehicleTelemetryData::gear)
    .def_readwrite("drag", &cr::VehicleTelemetryData::drag)
    .add_property("wheels", &GetWheelsTelemetry, &SetWheelsTelemetry)
    .def("__eq__", &cr::VehicleTelemetryData::operator==)
    .def("__ne__", &cr::VehicleTelemetryData::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

}
