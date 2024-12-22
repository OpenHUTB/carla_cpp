// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/rpc/Command.h>
#include <carla/rpc/CommandResponse.h>

// 定义默认端口号
#define TM_DEFAULT_PORT     8000

namespace command_impl {

  // 一个简单的模板函数，将输入的参数原样返回
  template <typename T>
  const T &Convert(const T &in) {
    return in;
  }

  // 将 boost::shared_ptr<carla::client::Actor> 转换为 carla::rpc::ActorId
  carla::rpc::ActorId Convert(const boost::shared_ptr<carla::client::Actor> &actor) {
    return actor->GetId();
  }

  // 将 carla::client::ActorBlueprint 转换为 carla::rpc::ActorDescription
  carla::rpc::ActorDescription Convert(const carla::client::ActorBlueprint &blueprint) {
    return blueprint.MakeActorDescription();
  }

  // 自定义初始化函数模板，调用对象的 __init__ 方法并将转换后的参数传递给它
  template <typename... ArgsT>
  static boost::python::object CustomInit(boost::python::object self, ArgsT... args) {
    return self.attr("__init__")(Convert(args)...);
  }

  // 自定义初始化函数模板，专门用于 SpawnActor，将参数封装为 carla::rpc::Command::SpawnActor 并传递给对象的 __init__ 方法
  template <typename... ArgsT>
  static boost::python::object CustomSpawnActorInit(boost::python::object self, ArgsT... args) {
    return self.attr("__init__")(carla::rpc::Command::SpawnActor{Convert(args)...});
  }

  // 为 carla::rpc::Command::SpawnActor 的 then 成员函数添加一个命令
  static carla::rpc::Command::SpawnActor Then(
      carla::rpc::Command::SpawnActor &self,
      carla::rpc::Command command) {
    self.do_after.push_back(command);
    return self;
  }

} // namespace command_impl

// 导出命令的函数
void export_commands() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cg = carla::geom;
  namespace cr = carla::rpc;

  using ActorPtr = carla::SharedPtr<cc::Actor>;

  // 创建一个 Python 模块对象
  object command_module(handle<>(borrowed(PyImport_AddModule("libcarla.command")));
  // 将该模块添加到当前作用域中
  scope().attr("command") = command_module;
  // 获取该模块的作用域
  scope submodule_scope = command_module;

  // 为 SpawnActor.then 命令设置一个处理器
  submodule_scope.attr("FutureActor") = 0u;

  // 为 carla::rpc::CommandResponse 类添加 Python 绑定
  class_<cr::CommandResponse>("Response", no_init)
    // 添加一个属性 actor_id，根据是否有错误返回不同的值
   .add_property("actor_id", +[](const cr::CommandResponse &self) {
      return self.HasError()? 0u : self.Get();
    })
    // 添加一个属性 error，根据是否有错误返回错误信息或空字符串
   .add_property("error", +[](const cr::CommandResponse &self) {
      return self.HasError()? self.GetError().What() : std::string("");
    })
    // 添加一个 has_error 方法，调用 CommandResponse 的 HasError 方法
   .def("has_error", &cr::CommandResponse::HasError)
  ;

  // 为 carla::rpc::Command::SpawnActor 类添加 Python 绑定
  class_<cr::Command::SpawnActor>("SpawnActor")
    // 定义不同参数列表的构造函数，使用 CustomSpawnActorInit 进行参数转换
   .def(
        "__init__",
        &command_impl::CustomSpawnActorInit<cc::ActorBlueprint, cg::Transform>,
        (arg("blueprint"), arg("transform")))
   .def(
        "__init__",
        &command_impl::CustomSpawnActorInit<cc::ActorBlueprint, cg::Transform, const cr::ActorId &>,
        (arg("blueprint"), arg("transform"), arg("parent_id")))
   .def(
        "__init__",
        &command_impl::CustomSpawnActorInit<cc::ActorBlueprint, cg::Transform, ActorPtr>,
        (arg("blueprint"), arg("transform"), arg("parent")))
   .def(
        "__init__",
        &command_impl::CustomSpawnActorInit<cc::ActorBlueprint, cg::Transform, ActorPtr, cr::AttachmentType, std::string>,
        (arg("blueprint"), arg("transform"), arg("parent"), arg("attachment_type"), arg("socket_name")))
    // 定义默认构造函数
   .def(init<cr::Command::SpawnActor>())
    // 定义可读写的成员变量
   .def_readwrite("transform", &cr::Command::SpawnActor::transform)
   .def_readwrite("parent_id", &cr::Command::SpawnActor::parent)
   .def_readwrite("attachment_type", &cr::Command::SpawnActor::attachment_type)
   .def_readwrite("socket_name", &cr::Command::SpawnActor::socket_name)
    // 定义 then 方法
   .def("then", &command_impl::Then, (arg("command")))
  ;

  // 为 carla::rpc::Command::DestroyActor 类添加 Python 绑定
  class_<cr::Command::DestroyActor>("DestroyActor")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr>, (arg("actor")))
   .def(init<cr::ActorId>((arg("actor_id"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::DestroyActor::actor)
  ;

  // 为 carla::rpc::Command::ApplyVehicleControl 类添加 Python 绑定
  class_<cr::Command::ApplyVehicleControl>("ApplyVehicleControl")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, cr::VehicleControl>, (arg("actor"), arg("control")))
   .def(init<cr::ActorId, cr::VehicleControl>((arg("actor_id"), arg("control"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::ApplyVehicleControl::actor)
   .def_readwrite("control", &cr::Command::ApplyVehicleControl::control)
  ;

  // 为 carla::rpc::Command::ApplyVehicleAckermannControl 类添加 Python 绑定
  class_<cr::Command::ApplyVehicleAckermannControl>("ApplyVehicleAckermannControl")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, cr::VehicleAckermannControl>, (arg("actor"), arg("control")))
   .def(init<cr::ActorId, cr::VehicleAckermannControl>((arg("actor_id"), arg("control"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::ApplyVehicleAckermannControl::actor)
   .def_readwrite("control", &cr::Command::ApplyVehicleAckermannControl::control)
  ;

  // 为 carla::rpc::Command::ApplyWalkerControl 类添加 Python 绑定
  class_<cr::Command::ApplyWalkerControl>("ApplyWalkerControl")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, cr::WalkerControl>, (arg("actor"), arg("control")))
   .def(init<cr::ActorId, cr::WalkerControl>((arg("actor_id"), arg("control"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::ApplyWalkerControl::actor)
   .def_readwrite("control", &cr::Command::ApplyWalkerControl::control)
  ;

  // 为 carla::rpc::Command::ApplyVehiclePhysicsControl 类添加 Python 绑定
  class_<cr::Command::ApplyVehiclePhysicsControl>("ApplyVehiclePhysicsControl")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, cr::VehiclePhysicsControl>, (arg("actor"), arg("physics_control")))
   .def(init<cr::ActorId, cr::VehiclePhysicsControl>((arg("actor_id"), arg("physics_control"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::ApplyVehiclePhysicsControl::actor)
   .def_readwrite("physics_control", &cr::Command::ApplyVehiclePhysicsControl::physics_control)
  ;

  // 为 carla::rpc::Command::ApplyTransform 类添加 Python 绑定
  class_<cr::Command::ApplyTransform>("ApplyTransform")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Transform>, (arg("actor"), arg("transform")))
   .def(init<cr::ActorId, cg::Transform>((arg("actor_id"), arg("transform"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::ApplyTransform::actor)
   .def_readwrite("transform", &cr::Command::ApplyTransform::transform)
  ;

  // 为 carla::rpc::Command::ApplyWalkerState 类添加 Python 绑定
  class_<cr::Command::ApplyWalkerState>("ApplyWalkerState")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Transform, float>, (arg("actor"), arg("transform"), arg("speed")))
   .def(init<cr::ActorId, cg::Transform, float>((arg("actor_id"), arg("transform"), arg("speed"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::ApplyWalkerState::actor)
   .def_readwrite("transform", &cr::Command::ApplyWalkerState::transform)
   .def_readwrite("speed", &cr::Command::ApplyWalkerState::speed)
  ;

  // 为 carla::rpc::Command::ApplyTargetVelocity 类添加 Python 绑定
  class_<cr::Command::ApplyTargetVelocity>("ApplyTargetVelocity")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("velocity")))
   .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("velocity"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::ApplyTargetVelocity::actor)
   .def_readwrite("velocity", &cr::Command::ApplyTargetVelocity::velocity)
  ;

  // 为 carla::rpc::Command::ApplyTargetAngularVelocity 类添加 Python 绑定
  class_<cr::Command::ApplyTargetAngularVelocity>("ApplyTargetAngularVelocity")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("angular_velocity")))
   .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("angular_velocity"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::ApplyTargetAngularVelocity::actor)
   .def_readwrite("angular_velocity", &cr::Command::ApplyTargetAngularVelocity::angular_velocity)
  ;

  // 为 carla::rpc::Command::ApplyImpulse 类添加 Python 绑定
  class_<cr::Command::ApplyImpulse>("ApplyImpulse")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("impulse")))
   .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("impulse"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::ApplyImpulse::actor)
   .def_readwrite("impulse", &cr::Command::ApplyImpulse::impulse)
  ;

  // 为 carla::rpc::Command::ApplyForce 类添加 Python 绑定
  class_<cr::Command::ApplyForce>("ApplyForce")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("force")))
   .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("force"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::ApplyForce::actor)
   .def_readwrite("force", &cr::Command::ApplyForce::force)
  ;

  // 为 carla::rpc::Command::ApplyAngularImpulse 类添加 Python 绑定
  class_<cr::Command::ApplyAngularImpulse>("ApplyAngularImpulse")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("impulse")))
   .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("impulse"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::ApplyAngularImpulse::actor)
   .def_readwrite("impulse", &cr::Command::ApplyAngularImpulse::impulse)
  ;

  // 为 carla::rpc::Command::ApplyTorque 类添加 Python 绑定
  class_<cr::Command::ApplyTorque>("ApplyTorque")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("torque")))
   .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("torque"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::ApplyTorque::actor)
   .def_readwrite("torque", &cr::Command::ApplyTorque::torque)
  ;

  // 为 carla::rpc::Command::SetSimulatePhysics 类添加 Python 绑定
  class_<cr::Command::SetSimulatePhysics>("SetSimulatePhysics")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, bool>, (arg("actor"), arg("enabled")))
   .def(init<cr::ActorId, bool>((arg("actor_id"), arg("enabled"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::SetSimulatePhysics::actor)
   .def_readwrite("enabled", &cr::Command::SetSimulatePhysics::enabled)
  ;

  // 为 carla::rpc::Command::SetEnableGravity 类添加 Python 绑定
  class_<cr::Command::SetEnableGravity>("SetEnableGravity")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, bool>, (arg("actor"), arg("enabled")))
   .def(init<cr::ActorId, bool>((arg("actor_id"), arg("enabled"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::SetEnableGravity::actor)
   .def_readwrite("enabled", &cr::Command::SetEnableGravity::enabled)
  ;

  // 为 carla::rpc::Command::SetAutopilot 类添加 Python 绑定
  class_<cr::Command::SetAutopilot>("SetAutopilot")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, bool, uint16_t>, (arg("actor"), arg("enabled"), arg("tm_port") = TM_DEFAULT_PORT ))
   .def(init<cr::ActorId, bool, uint16_t>((arg("actor_id"), arg("enabled"), arg("tm_port") = TM_DEFAULT_PORT )))
    // 定义可读写的成员变量
    // 通过 Boost.Python 将 C++ 类成员暴露到 Python 中，使 Python 脚本能够直接访问和修改这些成员变量

   .def_readwrite("actor_id", &cr::Command::SetAutopilot::actor)
   .def_readwrite("tm_port", &cr::Command::SetAutopilot::tm_port)
   .def_readwrite("enabled", &cr::Command::SetAutopilot::enabled)
  ;

  // 为 carla::rpc::Command::ShowDebugTelemetry 类添加 Python 绑定
  class_<cr::Command::ShowDebugTelemetry>("ShowDebugTelemetry")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, bool, uint16_t>, (arg("actor"), arg("enabled")))
   .def(init<cr::ActorId, bool>((arg("actor_id"), arg("enabled"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::ShowDebugTelemetry::actor)
   .def_readwrite("enabled", &cr::Command::ShowDebugTelemetry::enabled)
  ;

  // 为 carla::rpc::Command::SetVehicleLightState 类添加 Python 绑定
  class_<cr::Command::SetVehicleLightState>("SetVehicleLightState")
    // 定义不同参数列表的构造函数，使用 CustomInit 进行参数转换
   .def("__init__", &command_impl::CustomInit<ActorPtr, bool>, (arg("actor"), arg("light_state")))
   .def(init<cr::ActorId, cr::VehicleLightState::flag_type>((arg("actor_id"), arg("light_state"))))
    // 定义可读写的成员变量
   .def_readwrite("actor_id", &cr::Command::SetVehicleLightState::actor)
   .def_readwrite("light_state", &cr::Command::SetVehicleLightState::light_state)
  ;

  // 定义不同命令类到 carla::rpc::Command 的隐式转换
  implicitly_convertible<cr::Command::SpawnActor, cr::Command>();
  implicitly_convertible<cr::Command::DestroyActor, cr::Command>();
  implicitly_convertible<cr::Command::ApplyVehicleControl, cr::Command>();
  implicitly_convertible<cr::Command::ApplyVehicleAckermannControl, cr::Command>();
  implicitly_convertible<cr::Command::ApplyWalkerControl, cr::Command>();
  implicitly_convertible<cr::Command::ApplyVehiclePhysicsControl, cr::Command>();
  implicitly_convertible<cr::Command::ApplyTransform, cr::Command>();
  implicitly_convertible<cr::Command::ApplyWalkerState, cr::Command>();
  implicitly_convertible<cr::Command::ApplyTargetVelocity, cr::Command>();
  implicitly_convertible<cr::Command::ApplyTargetAngularVelocity, cr::Command>();
  implicitly_convertible<cr::Command::ApplyImpulse, cr::Command>();
  implicitly_convertible<cr::Command::ApplyForce, cr::Command>();
  implicitly_convertible<cr::Command::ApplyAngularImpulse, cr::Command>();
  implicitly_convertible<cr::Command::ApplyTorque, cr::Command>();
  implicitly_convertible<cr::Command::SetSimulatePhysics, cr::Command>();
  implicitly_convertible<cr::Command::SetEnableGravity, cr::Command>();
  implicitly_convertible<cr::Command::SetAutopilot, cr::Command>();
  implicitly_convertible<cr::Command::SetVehicleLightState, cr::Command>();
}
