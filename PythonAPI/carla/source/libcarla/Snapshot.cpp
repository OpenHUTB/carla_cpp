// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 引入头文件，包括 Carla 的 Python 工具、客户端中的 Actor、ActorList 和 World
#include <carla/PythonUtil.h>
#include <carla/client/Actor.h>
#include <carla/client/ActorList.h>
#include <carla/client/World.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
// 命名空间 carla 和 carla::client
namespace carla {
namespace client {
// 重载输出流运算符，用于输出 ActorSnapshot 对象
  std::ostream &operator<<(std::ostream &out, const ActorSnapshot &snapshot) {
    out << "ActorSnapshot(id=" << std::to_string(snapshot.id) << ')'; // 输出 ActorSnapshot 的 ID
    return out;
  }
// 重载输出流运算符，用于输出 WorldSnapshot 对象
  std::ostream &operator<<(std::ostream &out, const WorldSnapshot &snapshot) {
    out << "WorldSnapshot(frame=" << std::to_string(snapshot.GetTimestamp().frame) << ')';// 输出 WorldSnapshot 的帧编号
    return out;
  }

} // namespace client
} // namespace carla

void export_snapshot() {
  using namespace boost::python;
  namespace cc = carla::client;
// 定义 Python 中的 ActorSnapshot 类
  class_<cc::ActorSnapshot>("ActorSnapshot", no_init)
    .def_readonly("id", &cc::ActorSnapshot::id)// 定义只读属性 id，对应 ActorSnapshot 的成员变量 id
    .def("get_transform", +[](const cc::ActorSnapshot &self) { return self.transform; })// 定义方法 get_transform，返回 ActorSnapshot 的 transform
    .def("get_velocity", +[](const cc::ActorSnapshot &self) { return self.velocity; })// 定义方法 get_velocity，返回 ActorSnapshot 的 velocity
    .def("get_angular_velocity", +[](const cc::ActorSnapshot &self) { return self.angular_velocity; })// 定义方法 get_angular_velocity，返回 ActorSnapshot 的 angular_velocity
    .def("get_acceleration", +[](const cc::ActorSnapshot &self) { return self.acceleration; })// 定义方法 get_acceleration，返回 ActorSnapshot 的 acceleration
    .def(self_ns::str(self_ns::self))// 定义用于将 ActorSnapshot 对象转换为字符串的方法
  ;
  // 定义 Python 中的 WorldSnapshot 类
  class_<cc::WorldSnapshot>("WorldSnapshot", no_init)
        // 为 Python 类添加只读属性 "id"，对应 C++ 类 cc::WorldSnapshot 的 GetId 方法
    .add_property("id", &cc::WorldSnapshot::GetId)
        // 为 Python 类添加只读属性 "frame"，使用 Lambda 函数获取 C++ 类中的帧号 (frame)
    .add_property("frame", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().frame; })
        // 为 Python 类添加只读属性 "timestamp"，调用 C++ 的 GetTimestamp 方法，返回值是拷贝类型
    .add_property("timestamp", CALL_RETURNING_COPY(cc::WorldSnapshot, GetTimestamp))
    /// Deprecated, use timestamp @{
        // 为 Python 类添加只读属性 "frame_count"，等价于 "frame"，获取帧号信息
    .add_property("frame_count", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().frame; })
    .add_property("elapsed_seconds", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().elapsed_seconds; })
    .add_property("delta_seconds", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().delta_seconds; })
    .add_property("platform_timestamp", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().platform_timestamp; })
    /// @}
    .def("has_actor", &cc::WorldSnapshot::Contains, (arg("actor_id")))
    .def("find", CALL_RETURNING_OPTIONAL_1(cc::WorldSnapshot, Find, carla::ActorId), (arg("actor_id")))
    .def("__len__", &cc::WorldSnapshot::size)// 定义方法 __len__，返回 WorldSnapshot 中的元素数量
    .def("__iter__", range(&cc::WorldSnapshot::begin, &cc::WorldSnapshot::end)) // 定义方法 __iter__，用于迭代 WorldSnapshot 的元素
    .def("__eq__", &cc::WorldSnapshot::operator==)// 定义方法 __eq__，用于比较两个 WorldSnapshot 对象是否相等
    .def("__ne__", &cc::WorldSnapshot::operator!=)// 定义方法 __ne__，用于比较两个 WorldSnapshot 对象是否不相等
    .def(self_ns::str(self_ns::self))// 定义用于将 WorldSnapshot 对象转换为字符串的方法
}
