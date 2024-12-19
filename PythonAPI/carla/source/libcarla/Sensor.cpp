// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/client/ClientSideSensor.h>
#include <carla/client/LaneInvasionSensor.h>
#include <carla/client/Sensor.h>
#include <carla/client/ServerSideSensor.h>
// 定义一个静态函数 SubscribeToStream，用于让传感器订阅流并执行回调函数
static void SubscribeToStream(carla::client::Sensor &self, boost::python::object callback) {
	// 通过 MakeCallback 函数将传入的 Python 对象转换为合适的回调函数，并调用传感器的 Listen 方法进行订阅
  self.Listen(MakeCallback(std::move(callback)));
}
// 定义一个静态函数 SubscribeToGBuffer，用于让服务器端传感器订阅图形缓冲区（GBuffer）并执行回调函数
static void SubscribeToGBuffer(
  carla::client::ServerSideSensor &self,
  uint32_t GBufferId,
  boost::python::object callback) {
  self.ListenToGBuffer(GBufferId, MakeCallback(std::move(callback)));
}
// 定义一个名为 export_sensor 的函数，用于将 C++ 中的传感器类暴露给 Python
void export_sensor() {
  using namespace boost::python;
  namespace cc = carla::client;
// 定义一个名为 Sensor 的 Python 类，继承自 cc::Actor，并设置为不可复制，使用智能指针管理
  class_<cc::Sensor, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Sensor>>("Sensor", no_init)
    .def("listen", &SubscribeToStream, (arg("callback")))
    .def("is_listening", &cc::Sensor::IsListening)
    .def("stop", &cc::Sensor::Stop)
    .def(self_ns::str(self_ns::self))
  ;
// 定义一个名为 ServerSideSensor 的 Python 类，继承自 cc::Sensor，并设置为不可复制，使用智能指针管理
  class_<cc::ServerSideSensor, bases<cc::Sensor>, boost::noncopyable, boost::shared_ptr<cc::ServerSideSensor>>
      ("ServerSideSensor", no_init)
    .def("listen_to_gbuffer", &SubscribeToGBuffer, (arg("gbuffer_id"), arg("callback")))
    .def("is_listening_gbuffer", &cc::ServerSideSensor::IsListeningGBuffer, (arg("gbuffer_id")))
    .def("stop_gbuffer", &cc::ServerSideSensor::StopGBuffer, (arg("gbuffer_id")))
    .def("enable_for_ros", &cc::ServerSideSensor::EnableForROS)
    .def("disable_for_ros", &cc::ServerSideSensor::DisableForROS)
    .def("is_enabled_for_ros", &cc::ServerSideSensor::IsEnabledForROS)
    .def("send", &cc::ServerSideSensor::Send, (arg("message")))
    .def(self_ns::str(self_ns::self))
  ;
// 定义一个名为 ClientSideSensor 的 Python 类，继承自 cc::Sensor，并设置为不可复制，使用智能指针管理
  class_<cc::ClientSideSensor, bases<cc::Sensor>, boost::noncopyable, boost::shared_ptr<cc::ClientSideSensor>>
      ("ClientSideSensor", no_init)
	  // 定义一个名为 __str__ 的方法，用于在 Python 中打印客户端传感器对象时调用
    .def(self_ns::str(self_ns::self))
  ;
// 定义一个名为 LaneInvasionSensor 的 Python 类，继承自 cc::ClientSideSensor，并设置为不可复制，使用智能指针管理
  class_<cc::LaneInvasionSensor, bases<cc::ClientSideSensor>, boost::noncopyable, boost::shared_ptr<cc::LaneInvasionSensor>>
      ("LaneInvasionSensor", no_init)
	  // 定义一个名为 __str__ 的方法，用于在 Python 中打印车道入侵传感器对象时调用
    .def(self_ns::str(self_ns::self))
  ;

}
