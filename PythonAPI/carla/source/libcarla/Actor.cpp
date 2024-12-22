// 本作品根据MIT许可证条款进行授权。asssss
// 许可证副本见<https://opensource.org/licenses/MIT>。

#include <carla/client/Actor.h> // 引入Actor功能的头文件
#include <carla/client/TrafficLight.h> // 引入TrafficLight功能的头文件
#include <carla/client/Vehicle.h> // 引入Vehicle功能的头文件
#include <carla/client/Walker.h> // 引入Walker功能的头文件
#include <carla/client/WalkerAIController.h> // 引入Walker AI控制器功能的头文件
#include <carla/rpc/TrafficLightState.h> // 引入交通灯状态的头文件
#include <carla/trafficmanager/TrafficManager.h> // 引入交通管理器功能的头文件

#include <boost/python/suite/indexing/vector_indexing_suite.hpp> // 引入Boost.Python库以支持向量索引

#include <ostream> // 引入输出流的头文件
#include <iostream> // 引入输入输出流的头文件

namespace ctm = carla::traffic_manager; // 为交通管理器命名空间创建别名

namespace carla {
namespace client {

// 重载输出流操作符以打印Actor信息
std::ostream &operator<<(std::ostream &out, const Actor &actor) {
    // 输出Actor的ID和类型
    out << "Actor(id=" << actor.GetId() << ", type=" << actor.GetTypeId() << ')';
    return out;
}

} // namespace client
} // namespace carla

// 模板函数将std::vector转换为Python列表
template<class T>
boost::python::list StdVectorToPyList(const std::vector<T> &vec) {
    boost::python::list l; // 创建一个新的Python列表
    for (auto &e : vec) { // 遍历向量中的每个元素
        l.append(e); // 将每个元素添加到Python列表中
    }
    return l; // 返回填充好的Python列表
}

// 获取Actor的语义标签并转换为Python列表
static boost::python::list GetSemanticTags(const carla::client::Actor &self) {
    const std::vector<uint8_t> &tags = self.GetSemanticTags(); // 获取语义标签
    return StdVectorToPyList(tags); // 转换并返回为Python列表
}

// 为Actor添加冲击力
static void AddActorImpulse(carla::client::Actor &self,
    const carla::geom::Vector3D &impulse) {
    self.AddImpulse(impulse); // 对Actor施加冲击力
}

// 为Actor添加力
static void AddActorForce(carla::client::Actor &self,
    const carla::geom::Vector3D &force) {
    self.AddForce(force); // 对Actor施加力
}

// 获取与交通灯关联的组交通灯
static auto GetGroupTrafficLights(carla::client::TrafficLight &self) {
    auto values = self.GetGroupTrafficLights(); // 获取组交通灯
    return StdVectorToPyList(values); // 转换并返回为Python列表
}

// 模板函数为Walker应用控制
template <typename ControlT>
static void ApplyControl(carla::client::Walker &self, const ControlT &control) {
    self.ApplyControl(control); // 将控制应用于Walker
}

// 获取交通灯的光源盒
static auto GetLightBoxes(const carla::client::TrafficLight &self) {
    boost::python::list result; // 创建一个新的Python列表用于结果
    for (const auto &bb : self.GetLightBoxes()) { // 遍历每个边界框
        result.append(bb); // 将边界框添加到结果列表中
    }
    return result; // 返回填充好的光源盒列表
}

// 将参与者的函数暴露出去
void export_actor() {
  using namespace boost::python;
  namespace cc = carla::client; // carla客户端相关的命名空间
  namespace cr = carla::rpc;// carla远程过程调用相关的命名空间
  namespace ctm = carla::traffic_manager;// carla交通管理相关的命名空间

   // 定义ActorState枚举类型到Python的映射，将C++中的枚举值暴露给Python
    enum_<cr::ActorState>("ActorState")
    .value("Invalid", cr::ActorState::Invalid)
    .value("Active", cr::ActorState::Active)
    .value("Dormant", cr::ActorState::Dormant)
  ;

    // 为std::vector<int>类型定义Python类，使其在Python中能像普通类一样使用，支持索引和转换为字符串等操作
    class_<std::vector<int>>("vector_of_ints")
      .def(vector_indexing_suite<std::vector<int>>())
      .def(self_ns::str(self_ns::self))
  ;
  // 定义Actor类到Python的映射，继承相关特性，设置不可拷贝，通过智能指针管理，定义多个属性和方法的Python接口
  class_<cc::Actor, boost::noncopyable, boost::shared_ptr<cc::Actor>>("Actor", no_init)
  // 变通方法，强制返回副本以解决Actor而不是ActorState
      .add_property("id", CALL_RETURNING_COPY(cc::Actor, GetId))
      .add_property("type_id", CALL_RETURNING_COPY(cc::Actor, GetTypeId))
      .add_property("parent", CALL_RETURNING_COPY(cc::Actor, GetParent))
      .add_property("semantic_tags", &GetSemanticTags)
      .add_property("actor_state", CALL_WITHOUT_GIL(cc::Actor, GetActorState))
      .add_property("is_alive", CALL_WITHOUT_GIL(cc::Actor, IsAlive))
      .add_property("is_dormant", CALL_WITHOUT_GIL(cc::Actor, IsDormant))
      .add_property("is_active", CALL_WITHOUT_GIL(cc::Actor, IsActive))
      .add_property("attributes", +[] (const cc::Actor &self) {
        boost::python::dict attribute_dict;
        for (auto &&attribute_value : self.GetAttributes()) {
          attribute_dict[attribute_value.GetId()] = attribute_value.GetValue();
        }
        return attribute_dict;
      })
      .add_property("bounding_box", CALL_RETURNING_COPY(cc::Actor, GetBoundingBox))
      // 第1个参数：carla.Actor中的get_world(self) Python方法名
      // 第2个参数：LibCarla中的C++方法（函数名称前面加引用符号“&”的意思是返回引用类型）
      // 返回参与者所属C++世界的一份拷贝，成为Python中的世界
      .def("get_world", CALL_RETURNING_COPY(cc::Actor, GetWorld))
      .def("get_location", &cc::Actor::GetLocation)
      .def("get_transform", &cc::Actor::GetTransform)
      .def("get_velocity", &cc::Actor::GetVelocity)
      .def("get_angular_velocity", &cc::Actor::GetAngularVelocity)
      .def("get_acceleration", &cc::Actor::GetAcceleration)
      .def("get_component_world_transform", &cc::Actor::GetComponentWorldTransform, (arg("component_name")))
      .def("get_component_relative_transform", &cc::Actor::GetComponentRelativeTransform, (arg("component_name")))
      // 将返回值转换为Python列表的const请求
      .def("get_bone_world_transforms", CALL_RETURNING_LIST(cc::Actor,GetBoneWorldTransforms))
      .def("get_bone_relative_transforms", CALL_RETURNING_LIST(cc::Actor,GetBoneRelativeTransforms))
      .def("get_component_names", CALL_RETURNING_LIST(cc::Actor,GetComponentNames))
      .def("get_bone_names", CALL_RETURNING_LIST(cc::Actor,GetBoneNames))
      .def("get_socket_world_transforms", CALL_RETURNING_LIST(cc::Actor,GetSocketWorldTransforms))
      .def("get_socket_relative_transforms", CALL_RETURNING_LIST(cc::Actor,GetSocketRelativeTransforms))   
      .def("get_socket_names", CALL_RETURNING_LIST(cc::Actor,GetSocketNames))         
      .def("set_location", &cc::Actor::SetLocation, (arg("location")))
      // 将参与者传送到给定的变换（位置和旋转）
      .def("set_transform", &cc::Actor::SetTransform, (arg("transform")))
      .def("set_target_velocity", &cc::Actor::SetTargetVelocity, (arg("velocity")))
      .def("set_target_angular_velocity", &cc::Actor::SetTargetAngularVelocity, (arg("angular_velocity")))
      .def("enable_constant_velocity", &cc::Actor::EnableConstantVelocity, (arg("velocity")))
      .def("disable_constant_velocity", &cc::Actor::DisableConstantVelocity)
      .def("add_impulse", &AddActorImpulse, (arg("impulse")))
      .def("add_force", &AddActorForce, (arg("force")))
      .def("add_angular_impulse", &cc::Actor::AddAngularImpulse, (arg("angular_impulse")))
      .def("add_torque", &cc::Actor::AddTorque, (arg("torque")))
      .def("set_simulate_physics", &cc::Actor::SetSimulatePhysics, (arg("enabled") = true))
      .def("set_collisions", &cc::Actor::SetCollisions, (arg("enabled") = true))
      .def("set_enable_gravity", &cc::Actor::SetEnableGravity, (arg("enabled") = true))
      .def("destroy", CALL_WITHOUT_GIL(cc::Actor, Destroy))
      .def(self_ns::str(self_ns::self))
  ;

  // 定义VehicleLightState中的LightState枚举类型到Python的映射，将不同灯光状态枚举值暴露给Python
  enum_<cr::VehicleLightState::LightState>("VehicleLightState")
    .value("NONE", cr::VehicleLightState::LightState::None) // None is reserved in Python3
    .value("Position", cr::VehicleLightState::LightState::Position)
    .value("LowBeam", cr::VehicleLightState::LightState::LowBeam)
    .value("HighBeam", cr::VehicleLightState::LightState::HighBeam)
    .value("Brake", cr::VehicleLightState::LightState::Brake)
    .value("RightBlinker", cr::VehicleLightState::LightState::RightBlinker)
    .value("LeftBlinker", cr::VehicleLightState::LightState::LeftBlinker)
    .value("Reverse", cr::VehicleLightState::LightState::Reverse)
    .value("Fog", cr::VehicleLightState::LightState::Fog)
    .value("Interior", cr::VehicleLightState::LightState::Interior)
    .value("Special1", cr::VehicleLightState::LightState::Special1)
    .value("Special2", cr::VehicleLightState::LightState::Special2)
    .value("All", cr::VehicleLightState::LightState::All)
  ;

  // 定义VehicleWheelLocation枚举类型到Python的映射，将车辆车轮位置的枚举值暴露给Python
  enum_<cr::VehicleWheelLocation>("VehicleWheelLocation")
    .value("FL_Wheel", cr::VehicleWheelLocation::FL_Wheel)
    .value("FR_Wheel", cr::VehicleWheelLocation::FR_Wheel)
    .value("BL_Wheel", cr::VehicleWheelLocation::BL_Wheel)
    .value("BR_Wheel", cr::VehicleWheelLocation::BR_Wheel)
    .value("Front_Wheel", cr::VehicleWheelLocation::Front_Wheel)
    .value("Back_Wheel", cr::VehicleWheelLocation::Back_Wheel)
  ;

    // 定义VehicleDoor枚举类型到Python的映射，将车辆车门相关的枚举值暴露给Python
    enum_<cr::VehicleDoor>("VehicleDoor")
    .value("FL", cr::VehicleDoor::FL)
    .value("FR", cr::VehicleDoor::FR)
    .value("RL", cr::VehicleDoor::RL)
    .value("RR", cr::VehicleDoor::RR)
    .value("All", cr::VehicleDoor::All)
  ;

  // 定义VehicleFailureState枚举类型到Python的映射，将车辆故障状态的枚举值暴露给Python
  enum_<cr::VehicleFailureState>("VehicleFailureState")
    .value("NONE", cr::VehicleFailureState::None)
    .value("Rollover", cr::VehicleFailureState::Rollover)
    .value("Engine", cr::VehicleFailureState::Engine)
    .value("TirePuncture", cr::VehicleFailureState::TirePuncture)
  ;

  // 定义Vehicle类到Python的映射，继承自Actor类，设置相关特性，定义多个Vehicle类特有的方法和属性的Python接口
  class_<cc::Vehicle, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Vehicle>>("Vehicle",
      no_init)
      .def("apply_control", &cc::Vehicle::ApplyControl, (arg("control")))
      .def("apply_ackermann_control", &cc::Vehicle::ApplyAckermannControl, (arg("control")))
      .def("get_control", &cc::Vehicle::GetControl)
      .def("set_light_state", &cc::Vehicle::SetLightState, (arg("light_state")))
      .def("open_door", &cc::Vehicle::OpenDoor, (arg("door_idx")))
      .def("close_door", &cc::Vehicle::CloseDoor, (arg("door_idx")))
      .def("set_wheel_steer_direction", &cc::Vehicle::SetWheelSteerDirection, (arg("wheel_location")), (arg("angle_in_deg")))
      .def("get_wheel_steer_angle", &cc::Vehicle::GetWheelSteerAngle, (arg("wheel_location")))
      .def("get_light_state", CONST_CALL_WITHOUT_GIL(cc::Vehicle, GetLightState))
      .def("apply_physics_control", &cc::Vehicle::ApplyPhysicsControl, (arg("physics_control")))
      .def("get_physics_control", CONST_CALL_WITHOUT_GIL(cc::Vehicle, GetPhysicsControl))
      .def("apply_ackermann_controller_settings", &cc::Vehicle::ApplyAckermannControllerSettings, (arg("settings")))
      .def("get_ackermann_controller_settings", CONST_CALL_WITHOUT_GIL(cc::Vehicle, GetAckermannControllerSettings))
      .def("set_autopilot", CALL_WITHOUT_GIL_2(cc::Vehicle, SetAutopilot, bool, uint16_t), (arg("enabled") = true, arg("tm_port") = ctm::TM_DEFAULT_PORT))
      .def("get_telemetry_data", CONST_CALL_WITHOUT_GIL(cc::Vehicle, GetTelemetryData))  // 向客户端暴露遥测数据
      .def("show_debug_telemetry", &cc::Vehicle::ShowDebugTelemetry, (arg("enabled") = true))
      .def("get_speed_limit", &cc::Vehicle::GetSpeedLimit)
      .def("get_traffic_light_state", &cc::Vehicle::GetTrafficLightState)
      .def("is_at_traffic_light", &cc::Vehicle::IsAtTrafficLight)
      .def("get_traffic_light", &cc::Vehicle::GetTrafficLight)
      .def("enable_carsim", &cc::Vehicle::EnableCarSim, (arg("simfile_path") = ""))
      .def("use_carsim_road", &cc::Vehicle::UseCarSimRoad, (arg("enabled")))
      .def("enable_chrono_physics", &cc::Vehicle::EnableChronoPhysics, (arg("max_substeps")=30, arg("max_substep_delta_time")=0.002, arg("vehicle_json")="", arg("powetrain_json")="", arg("tire_json")="", arg("base_json_path")=""))
      .def("restore_physx_physics", &cc::Vehicle::RestorePhysXPhysics)
      .def("get_failure_state", &cc::Vehicle::GetFailureState)
      .def(self_ns::str(self_ns::self))
  ;

    //定义Walker类到Python的映射，继承自Actor类，设置相关特性，定义多个Walker类特有的方法的Python接口
    class_<cc::Walker, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Walker>>("Walker", no_init)
      .def("apply_control", &ApplyControl<cr::WalkerControl>, (arg("control")))
      .def("get_control", &cc::Walker::GetWalkerControl)
      .def("get_bones", &cc::Walker::GetBonesTransform)
      .def("set_bones", &cc::Walker::SetBonesTransform, (arg("bones")))
      .def("blend_pose", &cc::Walker::BlendPose, (arg("blend")))
      .def("show_pose", &cc::Walker::ShowPose)
      .def("hide_pose", &cc::Walker::HidePose)
      .def("get_pose_from_animation", &cc::Walker::GetPoseFromAnimation)
      .def(self_ns::str(self_ns::self))
  ;

    // 定义WalkerAIController类到Python的映射，继承自Actor类，设置相关特性，定义多个该类特有的方法的Python接口
    class_<cc::WalkerAIController, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::WalkerAIController>>("WalkerAIController", no_init)
    .def("start", &cc::WalkerAIController::Start)
    .def("stop", &cc::WalkerAIController::Stop)
    .def("go_to_location", &cc::WalkerAIController::GoToLocation, (arg("destination")))
    .def("set_max_speed", &cc::WalkerAIController::SetMaxSpeed, (arg("speed")))
    .def(self_ns::str(self_ns::self))
  ;

    // 定义TrafficSign类到Python的映射，继承自Actor类，设置相关特性，添加获取触发体积属性的Python接口
    class_<cc::TrafficSign, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::TrafficSign>>(
      "TrafficSign",
      no_init)
      .add_property("trigger_volume", CALL_RETURNING_COPY(cc::TrafficSign, GetTriggerVolume))
  ;

    // 定义TrafficLightState枚举类型到Python的映射，将交通信号灯状态的枚举值暴露给Python
    enum_<cr::TrafficLightState>("TrafficLightState")
      .value("Red", cr::TrafficLightState::Red)
      .value("Yellow", cr::TrafficLightState::Yellow)
      .value("Green", cr::TrafficLightState::Green)
      .value("Off", cr::TrafficLightState::Off)
      .value("Unknown", cr::TrafficLightState::Unknown)
  ;

    // 定义TrafficLight类到Python的映射，继承自TrafficSign类，设置相关特性，定义多个交通信号灯相关操作的Python接口，如设置、获取状态等
   // 定义一个名为"TrafficLight"的类，它继承自"cc::TrafficSign"类
// 该类不可复制（通过boost::noncopyable实现），并且使用boost::shared_ptr进行智能指针管理
class_<cc::TrafficLight, bases<cc::TrafficSign>, boost::noncopyable, boost::shared_ptr<cc::TrafficLight>>(
      "TrafficLight",
      no_init)
      // 添加名为"state"的属性，通过调用cc::TrafficLight::GetState函数来获取交通信号灯的当前状态
     .add_property("state", &cc::TrafficLight::GetState)
      // 定义一个名为"set_state"的函数，用于设置交通信号灯的状态，参数"state"表示要设置的具体状态值
     .def("set_state", &cc::TrafficLight::SetState, (arg("state")))
      // 定义一个名为"get_state"的函数，用于获取交通信号灯的状态，实际上它与上面的"add_property"中获取状态的函数对应，可能是为了方便不同的调用方式而定义
     .def("get_state", &cc::TrafficLight::GetState)
      // 定义一个名为"set_green_time"的函数，用于设置交通信号灯绿灯亮起的时长，参数"green_time"表示要设置的绿灯时长值
     .def("set_green_time", &cc::TrafficLight::SetGreenTime, (arg("green_time")))
      // 定义一个名为"get_green_time"的函数，用于获取交通信号灯绿灯亮起的时长
     .def("get_green_time", &cc::TrafficLight::GetGreenTime)
      // 定义一个名为"set_yellow_time"的函数，用于设置交通信号灯黄灯亮起的时长，参数"yellow_time"表示要设置的黄灯时长值
     .def("set_yellow_time", &cc::TrafficLight::SetYellowTime, (arg("yellow_time")))
      // 定义一个名为"get_yellow_time"的函数，用于获取交通信号灯黄灯亮起的时长
     .def("get_yellow_time", &cc::TrafficLight::GetYellowTime)
      // 定义一个名为"set_red_time"的函数，用于设置交通信号灯红灯亮起的时长，参数"red_time"表示要设置的红灯时长值
     .def("set_red_time", &cc::TrafficLight::SetRedTime, (arg("red_time")))
      // 定义一个名为"get_red_time"的函数，用于获取交通信号灯红灯亮起的时长
     .def("get_red_time", &cc::TrafficLight::GetRedTime)
      // 定义一个名为"get_elapsed_time"的函数，用于获取交通信号灯从当前状态开始已经经过的时间
     .def("get_elapsed_time", &cc::TrafficLight::GetElapsedTime)
      // 定义一个名为"freeze"的函数，用于设置交通信号灯是否冻结（可能暂停状态切换等情况），参数"freeze"表示是否冻结的布尔值
     .def("freeze", &cc::TrafficLight::Freeze, (arg("freeze")))
      // 定义一个名为"is_frozen"的函数，用于判断交通信号灯当前是否处于冻结状态
     .def("is_frozen", &cc::TrafficLight::IsFrozen)
      // 定义一个名为"get_pole_index"的函数，用于获取交通信号灯所在灯杆的索引（可能用于标识不同位置的灯杆等用途）
     .def("get_pole_index", &cc::TrafficLight::GetPoleIndex)
      // 定义一个名为"get_group_traffic_lights"的函数，用于获取与当前交通信号灯同组的其他交通信号灯（可能用于协调控制等场景），具体实现函数在别处（GetGroupTrafficLights）
     .def("get_group_traffic_lights", &GetGroupTrafficLights)
      // 定义一个名为"reset_group"的函数，用于重置交通信号灯所在的组（可能将同组信号灯恢复到初始设置等操作），由cc::TrafficLight::ResetGroup函数实现
     .def("reset_group", &cc::TrafficLight::ResetGroup)
      // 定义一个名为"get_affected_lane_waypoints"的函数，用于获取受当前交通信号灯影响的车道上的路点列表，以列表形式返回，通过调用cc::TrafficLight::GetAffectedLaneWaypoints函数实现
     .def("get_affected_lane_waypoints", CALL_RETURNING_LIST(cc::TrafficLight, GetAffectedLaneWaypoints))
      // 定义一个名为"get_light_boxes"的函数，用于获取交通信号灯的灯箱相关信息（具体信息由GetLightBoxes函数定义）
     .def("get_light_boxes", &GetLightBoxes)
      // 定义一个名为"get_opendrive_id"的函数，用于获取交通信号灯对应的OpenDRIVE中的标识ID，由cc::TrafficLight::GetOpenDRIVEID函数实现
     .def("get_opendrive_id", &cc::TrafficLight::GetOpenDRIVEID)
      // 定义一个名为"get_stop_waypoints"的函数，用于获取需要在当前交通信号灯前停车的路点列表，以列表形式返回，通过调用cc::TrafficLight::GetStopWaypoints函数实现
     .def("get_stop_waypoints", CALL_RETURNING_LIST(cc::TrafficLight, GetStopWaypoints))
      // 定义一个用于将交通信号灯对象转换为字符串表示的函数，具体转换逻辑由对应的self_ns::str函数决定（可能用于输出调试等用途）
     .def(self_ns::str(self_ns::self))
  ;
}
