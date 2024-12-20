// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 引入CARLA几何模块中的头文件，这些文件定义了各种几何类型，如向量、位置、旋转等。
#include <carla/geom/BoundingBox.h>
#include <carla/geom/GeoLocation.h>
#include <carla/geom/Location.h>
#include <carla/geom/Rotation.h>
#include <carla/geom/Transform.h>
#include <carla/geom/Vector2D.h>
#include <carla/geom/Vector3D.h>
 
// 引入Boost.Python库的头文件，这些文件提供了Python和C++之间交互的接口。
#include <boost/python/implicit.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
 
// 引入标准输出流库，用于输出信息。
#include <ostream>
 
// 声明CARLA的geom命名空间，以便在其中定义函数和操作符重载。
namespace carla {
namespace geom {
 
  // 模板函数，用于将Vector2D对象的x和y值输出到ostream中。
  template <typename T>
  static void WriteVector2D(std::ostream &out, const char *name, const T &vector2D) {
    out << name
        << "(x=" << std::to_string(vector2D.x)
        << ", y=" << std::to_string(vector2D.y) << ')';
  }
 
  // 模板函数，用于将Vector3D对象的x、y和z值输出到ostream中。
  template <typename T>
  static void WriteVector3D(std::ostream &out, const char *name, const T &vector3D) {
    out << name
        << "(x=" << std::to_string(vector3D.x)
        << ", y=" << std::to_string(vector3D.y)
        << ", z=" << std::to_string(vector3D.z) << ')';
  }
 
  // 重载<<操作符，用于将Vector2D对象输出到ostream中。
  std::ostream &operator<<(std::ostream &out, const Vector2D &vector2D) {
    WriteVector2D(out, "Vector2D", vector2D);
    return out;
  }
 
  // 重载<<操作符，用于将Vector3D对象输出到ostream中。
  std::ostream &operator<<(std::ostream &out, const Vector3D &vector3D) {
    WriteVector3D(out, "Vector3D", vector3D);
    return out;
  }
 
  // 重载<<操作符，用于将Location对象输出到ostream中。
  std::ostream &operator<<(std::ostream &out, const Location &location) {
    WriteVector3D(out, "Location", location);
    return out;
  }
 
  // 重载<<操作符，用于将Rotation对象输出到ostream中。
  std::ostream &operator<<(std::ostream &out, const Rotation &rotation) {
        out << "Rotation(pitch=" << std::to_string(rotation.pitch)
        << ", yaw=" << std::to_string(rotation.yaw)
        << ", roll=" << std::to_string(rotation.roll) << ')';
    return out;
  }
   
  // 重载<<操作符，用于将Transform对象输出到ostream中。
  std::ostream &operator<<(std::ostream &out, const Transform &transform) {
    out << "Transform(" << transform.location << ", " << transform.rotation << ')';
    return out;
  }
 
  // 重载<<操作符，用于将BoundingBox对象输出到ostream中。
  std::ostream &operator<<(std::ostream &out, const BoundingBox &box) {
    out << "BoundingBox(" << box.location << ", ";
    WriteVector3D(out, "Extent", box.extent);
    out << ", " << box.rotation;
    out << ')';
    return out;
  }

  // 重载<<操作符，用于将GeoLocation对象输出到ostream中。
  std::ostream &operator<<(std::ostream &out, const GeoLocation &geo_location) {
        out << "GeoLocation(latitude=" << std::to_string(geo_location.latitude)
        << ", longitude=" << std::to_string(geo_location.longitude)
        << ", altitude=" << std::to_string(geo_location.altitude) << ')';
    return out;
  }

} // namespace geom
} // namespace carla
 
// 定义一个函数，用于将Transform对象应用于一个Vector3D对象的列表，并更新这些对象。
static void TransformList(const carla::geom::Transform &self, boost::python::list &list) {
  auto length = boost::python::len(list);
   for (auto i = 0u; i < length; ++i) {
    self.TransformPoint(boost::python::extract<carla::geom::Vector3D &>(list[i]));
  }
}
// 定义一个函数，用于将一个16元素的float数组转换为一个4x4的boost::python::list。
static boost::python::list BuildMatrix(const std::array<float, 16> &m) {
  boost::python::list r_out;
  boost::python::list r[4];
  for (uint8_t i = 0; i < 16; ++i) { r[uint8_t(i / 4)].append(m[i]); }
  for (uint8_t i = 0; i < 4; ++i) { r_out.append(r[i]); }
  return r_out;
}
// 定义一个函数，用于获取Transform对象的矩阵表示。
static auto GetTransformMatrix(const carla::geom::Transform &self) {
  return BuildMatrix(self.GetMatrix());
}
// 定义一个函数，用于获取Transform对象的逆矩阵表示。
static auto GetInverseTransformMatrix(const carla::geom::Transform &self) {
  return BuildMatrix(self.GetInverseMatrix());
}
// 定义一系列静态函数，用于处理Vector3D对象的数学运算，如叉积、点积、距离等。
static auto Cross(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::Cross(self, other);
}
static auto Dot(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::Dot(self, other);
}
static auto Distance(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::Distance(self, other);
}
static auto DistanceSquared(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::DistanceSquared(self, other);
}
static auto Dot2D(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::Dot2D(self, other);
}
static auto Distance2D(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::Distance2D(self, other);
}
static auto DistanceSquared2D(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::DistanceSquared2D(self, other);
}
static auto GetVectorAngle(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::GetVectorAngle(self, other);
}
// 定义export_geom函数，用于将CARLA的geom模块中的类型和函数导出到Python中。
void export_geom() {
  using namespace boost::python;
  namespace cg = carla::geom;
  class_<std::vector<cg::Vector2D>>("vector_of_vector2D")
      .def(boost::python::vector_indexing_suite<std::vector<cg::Vector2D>>()) // 为std::vector<cg::Vector2D>提供Python索引支持
      .def(self_ns::str(self_ns::self)) // 为std::vector<cg::Vector2D>提供字符串表示
  ;
}

  class_<cg::Vector2D>("Vector2D")
    .def(init<float, float>((arg("x")=0.0f, arg("y")=0.0f)))
    .def_readwrite("x", &cg::Vector2D::x)
    .def_readwrite("y", &cg::Vector2D::y)
    .def("squared_length", &cg::Vector2D::SquaredLength)
    .def("length", &cg::Vector2D::Length)
    .def("make_unit_vector", &cg::Vector2D::MakeUnitVector)
    .def("__eq__", &cg::Vector2D::operator==)
    .def("__ne__", &cg::Vector2D::operator!=)
    .def(self += self)
    .def(self + self)
    .def(self -= self)
    .def(self - self)
    .def(self *= double())
    .def(self * double())
    .def(double() * self)
    .def(self /= double())
    .def(self / double())
    .def(double() / self)
    .def(self_ns::str(self_ns::self))
  ;

  implicitly_convertible<cg::Vector3D, cg::Location>();
  implicitly_convertible<cg::Location, cg::Vector3D>();

  class_<cg::Vector3D>("Vector3D")
    .def(init<float, float, float>((arg("x")=0.0f, arg("y")=0.0f, arg("z")=0.0f)))
    .def(init<const cg::Location &>((arg("rhs"))))
    .def_readwrite("x", &cg::Vector3D::x)
    .def_readwrite("y", &cg::Vector3D::y)
    .def_readwrite("z", &cg::Vector3D::z)
    .def("length", &cg::Vector3D::Length)
    .def("squared_length", &cg::Vector3D::SquaredLength)
    .def("make_unit_vector", &cg::Vector3D::MakeUnitVector)
    .def("cross", &Cross, (arg("vector")))
    .def("dot", &Dot, (arg("vector")))
    .def("dot_2d", &Dot2D, (arg("vector")))
    .def("distance", &Distance, (arg("vector")))
    .def("distance_2d", &Distance2D, (arg("vector")))
    .def("distance_squared", &DistanceSquared, (arg("vector")))
    .def("distance_squared_2d", &DistanceSquared2D, (arg("vector")))
    .def("get_vector_angle", &GetVectorAngle, (arg("vector")))
    .def("__eq__", &cg::Vector3D::operator==)
    .def("__ne__", &cg::Vector3D::operator!=)
    .def("__abs__", &cg::Vector3D::Abs)
    .def(self += self)
    .def(self + self)
    .def(self -= self)
    .def(self - self)
    .def(self *= double())
    .def(self * double())
    .def(double() * self)
    .def(self /= double())
    .def(self / double())
    .def(double() / self)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cg::Location, bases<cg::Vector3D>>("Location")
    .def(init<float, float, float>((arg("x")=0.0f, arg("y")=0.0f, arg("z")=0.0f)))
    .def(init<const cg::Vector3D &>((arg("rhs"))))
    .add_property("x", +[](const cg::Location &self) { return self.x; }, +[](cg::Location &self, float x) { self.x = x; })
    .add_property("y", +[](const cg::Location &self) { return self.y; }, +[](cg::Location &self, float y) { self.y = y; })
    .add_property("z", +[](const cg::Location &self) { return self.z; }, +[](cg::Location &self, float z) { self.z = z; })
    .def("distance", &cg::Location::Distance, (arg("location")))
    .def("__eq__", &cg::Location::operator==)
    .def("__ne__", &cg::Location::operator!=)
    .def("__abs__", &cg::Location::Abs)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cg::Rotation>("Rotation")
    .def(init<float, float, float>((arg("pitch")=0.0f, arg("yaw")=0.0f, arg("roll")=0.0f)))
    .def_readwrite("pitch", &cg::Rotation::pitch)
    .def_readwrite("yaw", &cg::Rotation::yaw)
    .def_readwrite("roll", &cg::Rotation::roll)
    .def("get_forward_vector", &cg::Rotation::GetForwardVector)
    .def("get_right_vector", &cg::Rotation::GetRightVector)
    .def("get_up_vector", &cg::Rotation::GetUpVector)
    .def("__eq__", &cg::Rotation::operator==)
    .def("__ne__", &cg::Rotation::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  // inverse_transform 逆变换
  class_<cg::Transform>("Transform")// 将cg::Transform类暴露给Python
    .def(init<cg::Location, cg::Rotation>(
        (arg("location")=cg::Location(), arg("rotation")=cg::Rotation())))
// 定义location属性的读写接口
    .def_readwrite("location", &cg::Transform::location)
 // 定义rotation属性的读写接口
    .def_readwrite("rotation", &cg::Transform::rotation)
 // 定义一个transform方法，接受一个TransformList函数
    .def("transform", &TransformList)
// 定义一个重载的transform方法，接受一个cg::Vector3D对象，通过lambda表达式调用TransformPoint方法并返回修改后的点
    .def("transform", +[](const cg::Transform &self, cg::Vector3D &location) {
      self.TransformPoint(location);
      return location;
    }, arg("in_point"))
  // 定义一个inverse_transform方法，功能类似transform，但调用InverseTransformPoint
    .def("inverse_transform", +[](const cg::Transform &self, cg::Vector3D &location) {
      self.InverseTransformPoint(location);
      return location;
    }, arg("in_point"))
 // 定义一个transform_vector方法，用于变换向量
    .def("transform_vector", +[](const cg::Transform &self, cg::Vector3D &vector) {
      self.TransformVector(vector);
      return vector;
    }, arg("in_point"))
// 定义获取前方向向量的方法
    .def("get_forward_vector", &cg::Transform::GetForwardVector)
 // 定义获取右方向向量的方法
    .def("get_right_vector", &cg::Transform::GetRightVector)
 // 定义获取上方向向量的方法
    .def("get_up_vector", &cg::Transform::GetUpVector)
// 定义获取变换矩阵的方法
    .def("get_matrix", &GetTransformMatrix)
// 定义获取逆变换矩阵的方法
    .def("get_inverse_matrix", &GetInverseTransformMatrix)
// 定义等于操作符的重载
    .def("__eq__", &cg::Transform::operator==)
// 定义不等于操作符的重载
    .def("__ne__", &cg::Transform::operator!=)
  // 定义转换为字符串的方法，用于打印
    .def(self_ns::str(self_ns::self))
  ;
// 将std::vector<cg::Transform>类暴露给Python
  class_<std::vector<cg::Transform>>("vector_of_transform")
 // 启用对向量的索引访问
      .def(boost::python::vector_indexing_suite<std::vector<cg::Transform>>())
 // 定义转换为字符串的方法，用于打印    
  .def(self_ns::str(self_ns::self))
  ;
// 将cg::BoundingBox类暴露给Python
  class_<cg::BoundingBox>("BoundingBox")
 // 定义一个构造函数，接受位置、大小和旋转，这些参数都有默认值
    .def(init<cg::Location, cg::Vector3D>(
        (arg("location")=cg::Location(), arg("extent")=cg::Vector3D(), arg("rotation")=cg::Rotation())))
 // 定义location属性的读写接口
    .def_readwrite("location", &cg::BoundingBox::location)
// 定义extent属性的读写接口
    .def_readwrite("extent", &cg::BoundingBox::extent)
 // 定义rotation属性的读写接口
    .def_readwrite("rotation", &cg::BoundingBox::rotation)
// 定义一个contains方法，检查点是否在边界框内
    .def("contains", &cg::BoundingBox::Contains, arg("point"), arg("bbox_transform"))
// 定义一个获取局部顶点的方法
    .def("get_local_vertices", CALL_RETURNING_LIST(cg::BoundingBox, GetLocalVertices))
// 定义一个获取世界坐标中顶点的方法
    .def("get_world_vertices", CALL_RETURNING_LIST_1(cg::BoundingBox, GetWorldVertices, const cg::Transform&), arg("bbox_transform"))
// 定义等于操作符的重载
    .def("__eq__", &cg::BoundingBox::operator==)
// 定义不等于操作符的重载
    .def("__ne__", &cg::BoundingBox::operator!=)
// 定义转换为字符串的方法，用于打印
    .def(self_ns::str(self_ns::self))
  ;

// 将cg::GeoLocation类暴露给Python
  class_<cg::GeoLocation>("GeoLocation")
// 定义一个构造函数，接受纬度、经度和高度，这些参数都有默认值
    .def(init<double, double, double>((arg("latitude")=0.0, arg("longitude")=0.0, arg("altitude")=0.0)))
// 定义latitude属性的读写接口
    .def_readwrite("latitude", &cg::GeoLocation::latitude)
 // 定义longitude属性的读写接口
    .def_readwrite("longitude", &cg::GeoLocation::longitude)
 // 定义altitude属性的读写接口
    .def_readwrite("altitude", &cg::GeoLocation::altitude)
// 定义等于操作符的重载
    .def("__eq__", &cg::GeoLocation::operator==)
// 定义不等于操作符的重载
    .def("__ne__", &cg::GeoLocation::operator!=)
// 定义转换为字符串的方法，用于打印
    .def(self_ns::str(self_ns::self))
  ;
}
