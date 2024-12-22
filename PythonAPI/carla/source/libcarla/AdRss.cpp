// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/rss/RssRestrictor.h>// 引入 Carla RSS 限制器头文件
#include <carla/rss/RssSensor.h> // 引入 Carla RSS 传感器头文件
#include <carla/sensor/data/RssResponse.h>// 引入 Carla 传感器数据中的 RssResponse 头文件

#include <ad/physics/python/AdPhysicsPython.hpp>// 引入 AdPhysics Python 接口头文件
#include <ad/map/python/AdMapAccessPython.hpp> // 引入 AdMap 访问 Python 接口头文件
#include <ad/rss/python/AdRssPython.hpp> // 引入 AdRss Python 接口头文件
#include <ad/rss/python/AdRssMapIntegrationPython.hpp>// 引入 AdRss 地图集成 Python 接口头文件
#include <ad/rss/world/RssDynamics.hpp>// 引入 AdRss 世界中的动态头文件
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>// 引入 Boost Python 库中的向量索引套件

//包含了 Carla RSS 模块中关于限制器（RssRestrictor）、传感器（RssSensor）以及传感器数据响应（RssResponse）等相关的头文件，这些文件应该定义了对应的 C++ 类和函数，用于处理 RSS 相关的逻辑，比如车辆的安全限制、传感器数据获取与处理等。
//同时还包含了一些以ad开头的头文件，可能是与特定的物理模拟、地图访问、RSS 相关的 Python 绑定辅助库，用于在 C++ 和 Python 之间进行数据传递和功能调用的对接。
//boost/python/suite/indexing/vector_indexing_suite.hpp 是 Boost.Python 库中的文件，用于在 Python 和 C++ 之间方便地处理vector类型的数据绑定，使得在 Python 中可以像操作原生list一样操作 C++ 的vector类型数据。

namespace carla {
namespace rss {
// 重载输出流运算符，用于打印 RssRestrictor 对象的信息

std::ostream &operator<<(std::ostream &out, const RssRestrictor &) {
    // 向输出流中写入表示 RssRestrictor 对象的字符串 "RssRestrictor()"

  out << "RssRestrictor()";
  return out;
}

}  // namespace rss

namespace sensor {
namespace data {

std::ostream &operator<<(std::ostream &out, const RssResponse &resp) {
  out << "RssResponse(frame=" << resp.GetFrame() << ", timestamp=" << resp.GetTimestamp()
      << ", valid=" << resp.GetResponseValid() << ", proper_response=" << resp.GetProperResponse()
      << ", rss_state_snapshot=" << resp.GetRssStateSnapshot() << ", situation_snapshot=" << resp.GetSituationSnapshot()
      << ", world_model=" << resp.GetWorldModel() << ", ego_dynamics_on_route=" << resp.GetEgoDynamicsOnRoute() << ')';
  return out;
}
//在carla命名空间下，进一步细分了rss和sensor::data等子命名空间。
//分别为RssRestrictor和RssResponse类重载了<<操作符，用于将这些对象以特定的格式输出到流中。这样在需要打印这些对象信息时，可以方便地使用cout等输出流进行操作，输出对象的关键属性信息，方便调试和查看对象状态。

}  // namespace data
}  // namespace sensor
}  // namespace carla

static auto GetEgoVehicleDynamics(const carla::client::RssSensor &self) {
  ad::rss::world::RssDynamics ego_dynamics(self.GetEgoVehicleDynamics());
  return ego_dynamics;
}

static auto GetOtherVehicleDynamics(const carla::client::RssSensor &self) {
  ad::rss::world::RssDynamics other_dynamics(self.GetOtherVehicleDynamics());
  return other_dynamics;
}

static auto GetPedestrianDynamics(const carla::client::RssSensor &self) {
  ad::rss::world::RssDynamics pedestrian_dynamics(self.GetPedestrianDynamics());
  return pedestrian_dynamics;
}

static auto GetRoadBoundariesMode(const carla::client::RssSensor &self) {
  carla::rss::RoadBoundariesMode road_boundaries_mode(self.GetRoadBoundariesMode());
  return road_boundaries_mode;
}

static auto GetRoutingTargets(const carla::client::RssSensor &self) {
  std::vector<carla::geom::Transform> routing_targets(self.GetRoutingTargets());
  return routing_targets;
}
//这些函数都是静态函数，用于从carla::client::RssSensor对象中获取不同类型的动力学数据或模式信息。
//例如，GetEgoVehicleDynamics函数通过调用RssSensor对象的GetEgoVehicleDynamics方法获取车辆自身的动力学数据，并将其转换为ad::rss::world::RssDynamics类型返回，方便后续在 Python 绑定中使用统一的数据类型进行处理。
//类似地，GetOtherVehicleDynamics、GetPedestrianDynamics分别获取其他车辆和行人的动力学数据，GetRoadBoundariesMode获取道路边界模式，GetRoutingTargets获取路由目标信息（以carla::geom::Transform类型的向量形式）

static void RegisterActorConstellationCallback(carla::client::RssSensor &self, boost::python::object callback) {
  namespace py = boost::python;
  // Make sure the callback is actually callable.
  if (!PyCallable_Check(callback.ptr())) {
    PyErr_SetString(PyExc_TypeError, "callback argument must be callable!");
    py::throw_error_already_set();
  }

  // We need to delete the callback while holding the GIL.
  using Deleter = carla::PythonUtil::AcquireGILDeleter;
  auto callback_ptr = carla::SharedPtr<py::object>{new py::object(callback), Deleter()};

  // Make a lambda callback.
  auto callback_function = [callback = std::move(callback_ptr)](
                               carla::SharedPtr<::carla::rss::ActorConstellationData> actor_constellation_data)
                               ->::carla::rss::ActorConstellationResult {
    carla::PythonUtil::AcquireGIL lock;
    ::carla::rss::ActorConstellationResult actor_constellation_result;
    try {
      actor_constellation_result =
          py::call<::carla::rss::ActorConstellationResult>(callback->ptr(), py::object(actor_constellation_data));
    } catch (const py::error_already_set &) {
      PyErr_Print();
    }
    return actor_constellation_result;
  };
  self.RegisterActorConstellationCallback(callback_function);
}

//这个函数用于在RssSensor对象上注册一个演员星座（ActorConstellation）回调函数。
//首先，它检查传入的boost::python::object类型的回调函数是否是可调用的，如果不是则抛出类型错误异常。
//然后，创建一个SharedPtr来管理回调函数对象，并使用carla::PythonUtil::AcquireGILDeleter确保在删除回调函数时能够正确获取和释放全局解释器锁（GIL），这是在多线程环境下与 Python 解释器交互时需要注意的，以保证数据的一致性和线程安全性。
//接着，定义了一个 lambda 函数作为实际要注册到RssSensor的回调函数，在这个 lambda 函数内部，先获取 GIL 锁，然后通过boost::python的call函数调用传入的原始回调函数，并处理可能出现的异常（如果回调函数执行过程中抛出异常，会打印异常信息），最后返回回调函数的执行结果给RssSensor。

static void Stop(carla::client::RssSensor &self) {
  // ensure the RssSensor is stopped with GIL released to sync on processing lock
  carla::PythonUtil::ReleaseGIL unlock;
  self.Stop();
}
//这个函数用于停止RssSensor的运行。
//在调用RssSensor的Stop方法之前，先释放全局解释器锁（GIL），这样可以让其他 Python 线程在传感器停止操作期间能够继续执行，避免因为锁的占用导致其他线程阻塞，同时也能保证传感器停止操作的同步性和正确性。
// 定义一个函数，用于导出与"ad"相关的内容（可能是将C++ 代码中的相关功能、类等暴露给Python使用，从函数名和代码中使用的Boost.Python相关内容推测）
void export_ad() {
// 使用Boost.Python库的命名空间，方便后续调用相关功能，Boost.Python常用于C++ 和Python的混合编程，实现C++ 代码的Python绑定
  using namespace boost::python;
// 创建或导入名为"ad"的Python模块作用域对象。通过PyImport_AddModule函数获取已存在的模块或者创建一个新的模块，并将其句柄转换为Boost.Python中的对象表示形式，这里相当于在Python环境中操作模块相关内容
  // create/import the ad module scope
  object ad_module(handle<>(borrowed(PyImport_AddModule("ad"))));
  scope().attr("ad") = ad_module;
  scope submodule_scope = ad_module;
  scope().attr("__doc__") = "Python binding of ad namespace C++ code";
// 为当前的Python作用域设置版权信息属性，表明代码的版权归属情况
  scope().attr("__copyright__") = "Copyright (C) 2019-2020 Intel Corporation";
// 调用其他函数来导出与"ad"相关的不同部分的Python绑定内容，比如物理、地图访问、RSS等方面，这些函数应该是在其他地方定义，用于进一步细分的绑定操作
  export_ad_physics_python();
  export_ad_map_access_python();
  export_ad_rss_python();
  export_ad_rss_map_integration_python();
}
// 首先调用export_ad函数，进行一些前置的通用或相关基础模块的Python绑定操作
void export_ad_rss() {
// 使用Boost.Python库的命名空间，方便后续调用相关功能，用于在C++ 中进行Python绑定相关操作，如定义Python可访问的类、函数等
  export_ad();
// 使用carla::client命名空间，应该是与Carla客户端相关的代码部分，可能包含了客户端相关的类、函数等定义，用于后续具体类的定义中涉及该命名空间内类型的情况
  using namespace boost::python;
// 使用carla::client命名空间，应该是与Carla客户端相关的代码部分，可能包含了客户端相关的类、函数等定义，用于后续具体类的定义中涉及该命名空间内类型的情况
  namespace cc = carla::client;
// 使用carla::sensor命名空间，大概率是与Carla中的传感器相关的代码部分，例如定义传感器相关的类、数据结构等会用到这个命名空间下的类型
  namespace cs = carla::sensor;
// 使用carla::sensor::data命名空间，可能是传感器相关的数据结构所在的命名空间，用于处理传感器产生的数据等相关内容
  namespace csd = carla::sensor::data;
// 使用Boost.Python的class_模板定义一个Python可访问的类，对应C++ 中的carla::rss::EgoDynamicsOnRoute类，在Python中这个类将被命名为"RssEgoDynamicsOnRoute"，并且通过一系列的.def_readwrite操作定义了Python中可以直接读写的属性，这些属性对应C++ 类中的成员变量，使得Python代码能够方便地访问和修改这些成员变量的值
  class_<carla::rss::EgoDynamicsOnRoute>("RssEgoDynamicsOnRoute")
      .def_readwrite("timestamp", &carla::rss::EgoDynamicsOnRoute::timestamp)
      .def_readwrite("time_since_epoch_check_start_ms",
                     &carla::rss::EgoDynamicsOnRoute::time_since_epoch_check_start_ms)
      .def_readwrite("time_since_epoch_check_end_ms", &carla::rss::EgoDynamicsOnRoute::time_since_epoch_check_end_ms)
      .def_readwrite("ego_speed", &carla::rss::EgoDynamicsOnRoute::ego_speed)
      .def_readwrite("min_stopping_distance", &carla::rss::EgoDynamicsOnRoute::min_stopping_distance)
      .def_readwrite("ego_center", &carla::rss::EgoDynamicsOnRoute::ego_center)
      .def_readwrite("ego_heading", &carla::rss::EgoDynamicsOnRoute::ego_heading)
      .def_readwrite("ego_center_within_route", &carla::rss::EgoDynamicsOnRoute::ego_center_within_route)
      .def_readwrite("crossing_border", &carla::rss::EgoDynamicsOnRoute::crossing_border)
      .def_readwrite("route_heading", &carla::rss::EgoDynamicsOnRoute::route_heading)
      .def_readwrite("route_nominal_center", &carla::rss::EgoDynamicsOnRoute::route_nominal_center)
      .def_readwrite("heading_diff", &carla::rss::EgoDynamicsOnRoute::heading_diff)
      .def_readwrite("route_speed_lat", &carla::rss::EgoDynamicsOnRoute::route_speed_lat)
      .def_readwrite("route_speed_lon", &carla::rss::EgoDynamicsOnRoute::route_speed_lon)
      .def_readwrite("route_accel_lat", &carla::rss::EgoDynamicsOnRoute::route_accel_lat)
      .def_readwrite("route_accel_lon", &carla::rss::EgoDynamicsOnRoute::route_accel_lon)
      .def_readwrite("avg_route_accel_lat", &carla::rss::EgoDynamicsOnRoute::avg_route_accel_lat)
      .def_readwrite("avg_route_accel_lon", &carla::rss::EgoDynamicsOnRoute::avg_route_accel_lon)
      .def(self_ns::str(self_ns::self));
// 使用Boost.Python的class_模板定义一个Python可访问的类，对应C++ 中的carla::rss::ActorConstellationResult类，在Python中这个类将被命名为"RssActorConstellationResult"，同样通过.def_readwrite定义了Python中可读写的属性，对应C++ 类中的成员变量，方便Python代码操作
  class_<carla::rss::ActorConstellationResult>("RssActorConstellationResult")
      .def_readwrite("rss_calculation_mode", &carla::rss::ActorConstellationResult::rss_calculation_mode)
      .def_readwrite("restrict_speed_limit_mode", &carla::rss::ActorConstellationResult::restrict_speed_limit_mode)
      .def_readwrite("ego_vehicle_dynamics", &carla::rss::ActorConstellationResult::ego_vehicle_dynamics)
      .def_readwrite("actor_object_type", &carla::rss::ActorConstellationResult::actor_object_type)
      .def_readwrite("actor_dynamics", &carla::rss::ActorConstellationResult::actor_dynamics)
      .def(self_ns::str(self_ns::self));

  class_<carla::rss::ActorConstellationData, boost::noncopyable, boost::shared_ptr<carla::rss::ActorConstellationData>>(
      "RssActorConstellationData", no_init)
      .def_readonly("ego_match_object", &carla::rss::ActorConstellationData::ego_match_object)
      .def_readonly("ego_route", &carla::rss::ActorConstellationData::ego_route)
      .def_readonly("ego_dynamics_on_route", &carla::rss::ActorConstellationData::ego_dynamics_on_route)
      .def_readonly("other_match_object", &carla::rss::ActorConstellationData::other_match_object)
      .def_readonly("other_actor", &carla::rss::ActorConstellationData::other_actor)
      .def(self_ns::str(self_ns::self));

  enum_<spdlog::level::level_enum>("RssLogLevel")
      .value("trace", spdlog::level::trace)
      .value("debug", spdlog::level::debug)
      .value("info", spdlog::level::info)
      .value("warn", spdlog::level::warn)
      .value("err", spdlog::level::err)
      .value("critical", spdlog::level::critical)
      .value("off", spdlog::level::off);
// 使用Boost.Python的enum_模板定义一个Python可访问的枚举类型，对应C++ 中的carla::rss::RoadBoundariesMode枚举，在Python中这个枚举将被命名为"RssRoadBoundariesMode"，同样通过.value操作进行枚举值与Python名称的映射
  enum_<carla::rss::RoadBoundariesMode>("RssRoadBoundariesMode")
      .value("Off", carla::rss::RoadBoundariesMode::Off)
      .value("On", carla::rss::RoadBoundariesMode::On);
// 使用Boost.Python的class_模板定义一个Python可访问的类，对应C++ 中的csd::RssResponse类，在Python中这个类将被命名为"RssResponse"，通过.add_property操作定义了Python中可以访问的属性，这些属性对应的获取和设置操作通过调用C++ 类中的相应成员函数来实现（有的是直接返回值，有的是返回拷贝等情况通过CALL_RETURNING_COPY等方式指定），同时也定义了将对象转换为字符串表示的方法（.def(self_ns::str(self_ns::self))）
  class_<csd::RssResponse, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::RssResponse>>(
      "RssResponse", no_init)
      .add_property("response_valid", &csd::RssResponse::GetResponseValid)
      .add_property("proper_response", CALL_RETURNING_COPY(csd::RssResponse, GetProperResponse))
      .add_property("rss_state_snapshot", CALL_RETURNING_COPY(csd::RssResponse, GetRssStateSnapshot))
      .add_property("situation_snapshot", CALL_RETURNING_COPY(csd::RssResponse, GetSituationSnapshot))
      .add_property("world_model", CALL_RETURNING_COPY(csd::RssResponse, GetWorldModel))
      .add_property("ego_dynamics_on_route", CALL_RETURNING_COPY(csd::RssResponse, GetEgoDynamicsOnRoute))
      .def(self_ns::str(self_ns::self));

  class_<cc::RssSensor, bases<cc::Sensor>, boost::noncopyable, boost::shared_ptr<cc::RssSensor>>("RssSensor", no_init)
      .add_property("ego_vehicle_dynamics", &GetEgoVehicleDynamics, &cc::RssSensor::SetEgoVehicleDynamics)
      .add_property("other_vehicle_dynamics", &GetOtherVehicleDynamics, &cc::RssSensor::SetOtherVehicleDynamics)
      .add_property("pedestrian_dynamics", &GetPedestrianDynamics, &cc::RssSensor::SetPedestrianDynamics)
      .add_property("road_boundaries_mode", &GetRoadBoundariesMode, &cc::RssSensor::SetRoadBoundariesMode)
      .add_property("routing_targets", &GetRoutingTargets)
      .def("stop", &Stop)
      .def("register_actor_constellation_callback", &RegisterActorConstellationCallback, (arg("callback")))
      .def("append_routing_target", &cc::RssSensor::AppendRoutingTarget, (arg("routing_target")))
      .def("reset_routing_targets", &cc::RssSensor::ResetRoutingTargets)
      .def("drop_route", &cc::RssSensor::DropRoute)
      .def("set_log_level", &cc::RssSensor::SetLogLevel, (arg("log_level")))
      .def("set_map_log_level", &cc::RssSensor::SetMapLogLevel, (arg("map_log_level")))
      .def(self_ns::str(self_ns::self));
// 使用Boost.Python的class_模板定义一个Python可访问的类，对应C++ 中的carla::rss::RssRestrictor类，在Python中这个类将被命名为"RssRestrictor"，定义了默认构造函数（.def(init<>())）以及其他可供Python调用的成员函数，用于实现特定的功能操作，同时也定义了将对象转换为字符串表示的方法
  class_<carla::rss::RssRestrictor, boost::noncopyable, boost::shared_ptr<carla::rss::RssRestrictor>>("RssRestrictor",
                                                                                                      no_init)
      .def(init<>())
      .def("restrict_vehicle_control", &carla::rss::RssRestrictor::RestrictVehicleControl,
           (arg("vehicle_control"), arg("proper_response"), arg("ego_dynamics_on_route"), arg("vehicle_physics")))
      .def("set_log_level", &carla::rss::RssRestrictor::SetLogLevel, (arg("log_level")))
      .def(self_ns::str(self_ns::self));
}
