// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/BlueprintLibrary.h>
#include <carla/client/ActorBlueprint.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <ostream>

// 命名空间carla的定义开始
namespace carla {
// 传感器相关的命名空间开始，这里主要涉及传感器数据相关的内容定义
namespace sensor {
namespace data {
    // 重载 << 运算符，用于将Color类型的对象以特定格式输出到流中
    // 输出格式类似 "Color(r,g,b,a)"，其中 r、g、b、a 分别是Color对象的成员变量
  std::ostream &operator<<(std::ostream &out, const Color &color) {
    out << "Color(" << std::to_string(color.r)
        << ',' << std::to_string(color.g)
        << ',' << std::to_string(color.b)
        << ',' << std::to_string(color.a) << ')';
    return out;
  }
    // 重载 << 运算符，用于将OpticalFlowPixel类型的对象以特定格式输出到流中
    // 输出格式类似 "Color(x,y)"，其中 x、y 分别是OpticalFlowPixel对象的成员变量
  std::ostream &operator<<(std::ostream &out, const OpticalFlowPixel &color) {
    out << "Color(" << std::to_string(color.x)
        << ',' << std::to_string(color.y) << ')';
    return out;
  }

} // 数据命名空间
} // 传感器命名空间
// 客户端相关的命名空间开始，涉及与客户端交互、蓝图等相关功能的定义
namespace client {
    // 重载 << 运算符，用于将ActorAttribute类型的对象以特定格式输出到流中
    // 输出内容包含属性的ID、类型、值以及是否可修改等信息
  std::ostream &operator<<(std::ostream &out, const ActorAttribute &attr) {
    using Type = carla::rpc::ActorAttributeType;
// 静态断言，确保ActorAttributeType的枚举大小符合预期，若不符合则提示更新此函数
    static_assert(static_cast<uint8_t>(Type::SIZE) == 6u, "Please update this function.");
    out << "ActorAttribute(id=" << attr.GetId();
// 根据属性的类型（如布尔、整数、浮点数等），输出对应类型和值的信息
    switch (attr.GetType()) {
      case Type::Bool:
        out << ",type=bool,value=" << (attr.As<bool>() ? "True" : "False");
        break;
      case Type::Int:
        out << ",type=int,value=" << attr.As<int>();
        break;
      case Type::Float:
        out << ",type=float,value=" << attr.As<float>();
        break;
      case Type::String:
        out << ",type=str,value=" << attr.As<std::string>();
        break;
      case Type::RGBColor:
        out << ",type=Color,value=" << attr.As<sensor::data::Color>();
        break;
      default:
        out << ",INVALID";
    }
// 如果属性不可修改，添加(const)标识
    if (!attr.IsModifiable()) {
      out << "(const)";
    }
    out << ')';
    return out;
  }
  // 重载 << 运算符，用于将ActorBlueprint类型的对象以特定格式输出到流中
    // 输出内容包含蓝图的ID和标签信息
  std::ostream &operator<<(std::ostream &out, const ActorBlueprint &bp) {
    out << "ActorBlueprint(id=" << bp.GetId() << ",tags=" << bp.GetTags() << ')';
    return out;
  }
// 重载 << 运算符，用于将BlueprintLibrary类型的对象以特定方式输出到流中
    // 这里调用了PrintList函数（未在当前代码中展示其定义）来进行输出
  std::ostream &operator<<(std::ostream &out, const BlueprintLibrary &blueprints) {
    return PrintList(out, blueprints);
  }

} // 客户端命名空间
} //Carla命名空间
// 用于在Python中导出相关的蓝图相关功能的函数定义
void export_blueprint() {
  using namespace boost::python;
// 引入carla::client命名空间的快捷引用方式
  namespace cc = carla::client;
// 引入carla::rpc命名空间的快捷引用方式
  namespace crpc = carla::rpc;
// 引入carla::sensor::data命名空间的快捷引用方式
  namespace csd = carla::sensor::data;
// 在Python中定义ActorAttributeType枚举类型，使其可被Python代码访问
  enum_<crpc::ActorAttributeType>("ActorAttributeType")
    .value("Bool", crpc::ActorAttributeType::Bool)
    .value("Int", crpc::ActorAttributeType::Int)
    .value("Float", crpc::ActorAttributeType::Float)
    .value("String", crpc::ActorAttributeType::String)
    .value("RGBColor", crpc::ActorAttributeType::RGBColor)
  ;
// 在Python中定义Color类，使其可被Python代码访问和操作
  class_<csd::Color>("Color")
    .def(init<uint8_t, uint8_t, uint8_t, uint8_t>(
        (arg("r")=0, arg("g")=0, arg("b")=0, arg("a")=255)))
    .def_readwrite("r", &csd::Color::r)
    .def_readwrite("g", &csd::Color::g)
    .def_readwrite("b", &csd::Color::b)
    .def_readwrite("a", &csd::Color::a)
    .def("__eq__", &csd::Color::operator==)
    .def("__ne__", &csd::Color::operator!=)
    .def(self_ns::str(self_ns::self))
  ;
// 在Python中定义FloatColor类，使其可被Python代码访问和操作
  class_<crpc::FloatColor>("FloatColor")
    .def(init<float, float, float, float>(
        (arg("r")=0, arg("g")=0.f, arg("b")=0.f, arg("a")=1.0f)))
    .def_readwrite("r", &crpc::FloatColor::r)
    .def_readwrite("g", &crpc::FloatColor::g)
    .def_readwrite("b", &crpc::FloatColor::b)
    .def_readwrite("a", &crpc::FloatColor::a)
    .def("__eq__", &crpc::FloatColor::operator==)
    .def("__ne__", &crpc::FloatColor::operator!=)
  ;
// 在Python中定义OpticalFlowPixel类，使其可被Python代码访问和操作
  class_<csd::OpticalFlowPixel>("OpticalFlowPixel")
    .def(init<float, float>(
        (arg("x")=0, arg("y")=0)))
    .def_readwrite("x", &csd::OpticalFlowPixel::x)
    .def_readwrite("y", &csd::OpticalFlowPixel::y)
    .def("__eq__", &csd::OpticalFlowPixel::operator==)
    .def("__ne__", &csd::OpticalFlowPixel::operator!=)
    .def(self_ns::str(self_ns::self))
  ;
// 在Python中定义ActorAttribute类，使其可被Python代码访问和操作
  class_<cc::ActorAttribute>("ActorAttribute", no_init)
    .add_property("id", CALL_RETURNING_COPY(cc::ActorAttribute, GetId))
    .add_property("type", &cc::ActorAttribute::GetType)
    .add_property("recommended_values", CALL_RETURNING_LIST(cc::ActorAttribute, GetRecommendedValues))
    .add_property("is_modifiable", &cc::ActorAttribute::IsModifiable)
    .def("as_bool", &cc::ActorAttribute::As<bool>)
    .def("as_int", &cc::ActorAttribute::As<int>)
    .def("as_float", &cc::ActorAttribute::As<float>)
    .def("as_str", &cc::ActorAttribute::As<std::string>)
    .def("as_color", &cc::ActorAttribute::As<csd::Color>)
// 多次重载__eq__运算符，用于比较不同类型数据与ActorAttribute对象是否相等
    .def("__eq__", &cc::ActorAttributeValueAccess::operator==<bool>)
    .def("__eq__", &cc::ActorAttributeValueAccess::operator==<int>)
    .def("__eq__", &cc::ActorAttributeValueAccess::operator==<float>)
    .def("__eq__", &cc::ActorAttributeValueAccess::operator==<std::string>)
    .def("__eq__", &cc::ActorAttributeValueAccess::operator==<csd::Color>)
    .def("__eq__", &cc::ActorAttributeValueAccess::operator==<cc::ActorAttribute>)
// 多次重载__ne__运算符，用于比较不同类型数据与ActorAttribute对象是否不相等
    .def("__ne__", &cc::ActorAttributeValueAccess::operator!=<bool>)
    .def("__ne__", &cc::ActorAttributeValueAccess::operator!=<int>)
    .def("__ne__", &cc::ActorAttributeValueAccess::operator!=<float>)
    .def("__ne__", &cc::ActorAttributeValueAccess::operator!=<std::string>)
    .def("__ne__", &cc::ActorAttributeValueAccess::operator!=<csd::Color>)
    .def("__ne__", &cc::ActorAttributeValueAccess::operator!=<cc::ActorAttribute>)
    .def("__nonzero__", &cc::ActorAttribute::As<bool>)
    .def("__bool__", &cc::ActorAttribute::As<bool>)
    .def("__int__", &cc::ActorAttribute::As<int>)
    .def("__float__", &cc::ActorAttribute::As<float>)
    .def("__str__", &cc::ActorAttribute::As<std::string>)
    .def(self_ns::str(self_ns::self))
  ;
// 在Python中定义ActorBlueprint类，使其可被Python代码访问和操作
  class_<cc::ActorBlueprint>("ActorBlueprint", no_init)
    .add_property("id", CALL_RETURNING_COPY(cc::ActorBlueprint, GetId))
    .add_property("tags", CALL_RETURNING_LIST(cc::ActorBlueprint, GetTags))
    .def("has_tag", &cc::ActorBlueprint::ContainsTag)
    .def("match_tags", &cc::ActorBlueprint::MatchTags)
    .def("has_attribute", &cc::ActorBlueprint::ContainsAttribute)
    .def("get_attribute", CALL_RETURNING_COPY_1(cc::ActorBlueprint, GetAttribute, const std::string &))
    .def("set_attribute", &cc::ActorBlueprint::SetAttribute)
    .def("__len__", &cc::ActorBlueprint::size)
    .def("__iter__", range(&cc::ActorBlueprint::begin, &cc::ActorBlueprint::end))
    .def(self_ns::str(self_ns::self))
  ;
// 在Python中定义BlueprintLibrary类，使其可被Python代码访问和操作
    // 标记此类不可复制，并且使用智能指针来管理
  class_<cc::BlueprintLibrary, boost::noncopyable, boost::shared_ptr<cc::BlueprintLibrary>>("BlueprintLibrary", no_init)
    .def("find", +[](const cc::BlueprintLibrary &self, const std::string &key) -> cc::ActorBlueprint {
      return self.at(key);
    }, (arg("id")))
    .def("filter", &cc::BlueprintLibrary::Filter, (arg("wildcard_pattern")))
    .def("filter_by_attribute", &cc::BlueprintLibrary::FilterByAttribute, (arg("name"), arg("value")))
    .def("__getitem__", +[](const cc::BlueprintLibrary &self, size_t pos) -> cc::ActorBlueprint {
      return self.at(pos);
    })
    .def("__len__", &cc::BlueprintLibrary::size)
    .def("__iter__", range(&cc::BlueprintLibrary::begin, &cc::BlueprintLibrary::end))
    .def(self_ns::str(self_ns::self))
  ;
}
