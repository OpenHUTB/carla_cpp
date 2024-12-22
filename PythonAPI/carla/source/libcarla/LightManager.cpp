// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <chrono> // 用于时间相关的功能
#include <memory> // 用于内存管理和智能指针
 
#include "carla/PythonUtil.h" // CARLA项目中用于Python集成的工具
#include "boost/python/suite/indexing/vector_indexing_suite.hpp" // Boost.Python库，用于向Python暴露STL容器
 
#include "carla/client/LightManager.h" // 包含CARLA客户端的交通灯管理器的定义
 
// 定义命名空间别名，方便代码编写
namespace cc = carla::client;
namespace cr = carla::rpc;
namespace csd = carla::sensor::data;
 
/**
 ******* 交通灯管理激活 *******
 * 以下函数用于管理交通灯的激活状态
 */
 
// 打开指定的交通灯
static void LightManagerTurnOn(
  cc::LightManager& self, // 引用交通灯管理器实例
  const boost::python::object& py_lights) { // 从Python接收的交通灯列表
 
  // 将Python列表转换为C++的std::vector<cc::Light>
  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };
 
  // 调用交通灯管理器的TurnOn方法
  self.TurnOn(lights);
}
 
// 关闭指定的交通灯
static void LightManagerTurnOff(
  cc::LightManager& self,
  const boost::python::object& py_lights) {
 
  // 转换并关闭交通灯
  std::vector<cc::Light> lights { ... }; // 同上
  self.TurnOff(lights);
}
 
// 设置指定交通灯的活动状态
static void LightManagerSetActive(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const boost::python::object& py_active) {
 
  // 转换交通灯和活动状态列表
  std::vector<cc::Light> lights { ... }; // 同上
  std::vector<bool> active { ... }; // 同上，但转换为bool类型
 
  // 设置活动状态
  self.SetActive(lights, active);
}
 
// 检查指定交通灯是否活动
static boost::python::list LightManagerIsActive(
    cc::LightManager& self,
    const boost::python::object& py_lights) {
 
  // 转换并查询交通灯的活动状态
  std::vector<cc::Light> lights { ... }; // 同上
  boost::python::list result;
 
  for (auto &&item : self.IsActive(lights)) {
    // 将结果转换为Python列表，避免位引用转换
    result.append(static_cast<bool>(item));
  }
 
  return result;
}
 
/*******************/
 
/**
 ******* 交通灯颜色管理器 *******
 * 以下函数用于管理交通灯的颜色
 */
 
// 为指定的交通灯设置单一颜色
static void LightManagerSetColor(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const csd::Color color) {
 
  // 转换交通灯列表并设置颜色
  std::vector<cc::Light> lights { ... }; // 同上
  self.SetColor(lights, color);
}
 
// 为指定的交通灯设置不同的颜色
static void LightManagerSetVectorColor(
  cc::LightManager& self,// LightManager 实例，用于执行灯光设置操作
  const boost::python::object& py_lights,
  const boost::python::object& py_colors) {
 
  // 转换交通灯和颜色列表
  std::vector<cc::Light> lights { ... }; // 同上
  std::vector<csd::Color> colors { ... }; // 同上，但转换为Color类型
 
  // 设置颜色
  self.SetColor(lights, colors);
}
 
// 获取指定交通灯的颜色
static boost::python::list LightManagerGetColor(
    cc::LightManager& self,
    const boost::python::object& py_lights) {
 
  // 转换并查询交通灯的颜色
  std::vector<cc::Light> lights { ... }; // 同上
  boost::python::list result;
 
  for (auto &&item : self.GetColor(lights)) {
    // 将结果转换为Python列表
    result.append(item);
  }
 
  return result;
}
/*******************/

/****** 交通灯亮度管理器 ******/
static void LightManagerSetIntensity(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const float intensity) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  self.SetIntensity(lights, intensity);
}

static void LightManagerSetVectorIntensity(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const boost::python::object& py_intensities) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  std::vector<float> intensities {
    boost::python::stl_input_iterator<float>(py_intensities),
    boost::python::stl_input_iterator<float>()
  };

  self.SetIntensity(lights, intensities);
}

static boost::python::list LightManagerGetIntensity(
    cc::LightManager& self,
    const boost::python::object& py_lights) {

 // 创建一个空的 Python 列表，用于存储结果
  boost::python::list result;

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  for (auto &&item : self.GetIntensity(lights)) {
    result.append(item);
  }

  return result;
}

/*******************/

/****** 红绿灯组 ******/
static void LightManagerSetLightGroup(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const cr::LightState::LightGroup light_group) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  self.SetLightGroup(lights, light_group);
}

static void LightManagerSetVectorLightGroup(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const boost::python::object& py_light_group) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  std::vector<cr::LightState::LightGroup> light_groups {
    boost::python::stl_input_iterator<cr::LightState::LightGroup>(py_light_group),
    boost::python::stl_input_iterator<cr::LightState::LightGroup>()
  };

  self.SetLightGroup(lights, light_groups);
}

static boost::python::list LightManagerGetLightGroup(
    cc::LightManager& self,
    const boost::python::object& py_lights) {

  boost::python::list result;

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  for (auto &&item : self.GetLightGroup(lights)) {
    result.append(item);
  }

  return result;
}

/*******************/

/****** 红绿灯状态 ******/
static void LightManagerSetLightState(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  cc::LightState& light_state) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  self.SetLightState(lights, light_state);
}

static void LightManagerSetVectorLightState(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const boost::python::object& py_light_state) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  std::vector<cc::LightState> light_states {
    boost::python::stl_input_iterator<cc::LightState>(py_light_state),
    boost::python::stl_input_iterator<cc::LightState>()
  };

  self.SetLightState(lights, light_states);
}

static boost::python::list LightManagerGetLightState(
    cc::LightManager& self,
    const boost::python::object& py_lights) {

  boost::python::list result;

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  for (auto &&item : self.GetLightState(lights)) {
    result.append(item);
  }

  return result;
}

static void LightManagerSetDayNightCycle(
  cc::LightManager& self,
  const bool active) {
  self.SetDayNightCycle(active);
}

/*******************/
// 导出用于灯光管理的函数
void export_lightmanager() {
    // 在 Python 模块中封装这些函数指针
    using namespace boost::python;

    enum_<cr::LightState::LightGroup>("LightGroup")
      .value("NONE", cr::LightState::LightGroup::None)
      .value("Vehicle", cr::LightState::LightGroup::Vehicle)
      .value("Street", cr::LightState::LightGroup::Street)
      .value("Building", cr::LightState::LightGroup::Building)
      .value("Other", cr::LightState::LightGroup::Other)
    ;

    class_<cc::LightState>("LightState")
      .def(init<float, csd::Color, cr::LightState::LightGroup, bool>((arg("intensity")=0.0f, arg("color")=csd::Color(), arg("group")=cr::LightState::LightGroup::None, arg("active")=false )))
      .def_readwrite("intensity", &cc::LightState::_intensity)
      .def_readwrite("color", &cc::LightState::_color)
      .def_readwrite("group", &cc::LightState::_group)
      .def_readwrite("active", &cc::LightState::_active)
    ;

    class_<cc::Light, boost::shared_ptr<cc::Light>>("Light", no_init)
      .add_property("color", &cc::Light::GetColor)
      .add_property("id", &cc::Light::GetId)
      .add_property("intensity", &cc::Light::GetIntensity)
      .add_property("is_on", &cc::Light::IsOn)
      .add_property("location", &cc::Light::GetLocation)
      .add_property("light_group", &cc::Light::GetLightGroup)
      .add_property("light_state", &cc::Light::GetLightState)
      .def("set_color", &cc::Light::SetColor, (arg("color")))
      .def("set_intensity", &cc::Light::SetIntensity, (arg("intensity")))
      .def("set_light_group", &cc::Light::SetLightGroup, (arg("light_group")))
      .def("set_light_state", &cc::Light::SetLightState, (arg("light_state")))
      .def("turn_on", &cc::Light::TurnOn)
      .def("turn_off", &cc::Light::TurnOff)
    ;

    // boost::python::def 函数可以帮助我们将C++中的函数导出到Python中，供Python使用。
    // get_all_lights(self, light_group=carla.LightGroup.None) 返回包含特定组中的灯光的列表。
    // turn_on(self, lights) 打开 lights 中的所有灯（参数：python中的函数名、C++中的函数指针、参数列表）
    class_<cc::LightManager, boost::shared_ptr<cc::LightManager>>("LightManager", no_init)
      .def("get_all_lights", CALL_RETURNING_LIST_1(cc::LightManager, GetAllLights, cr::LightState::LightGroup), (args("light_group") = cr::LightState::LightGroup::None ))
      .def("turn_on", &LightManagerTurnOn, (arg("lights")))
      .def("turn_off", &LightManagerTurnOff, (arg("lights")))
      .def("set_active", &LightManagerSetActive, (arg("lights"), arg("active")))
      .def("is_active", &LightManagerIsActive, (arg("lights")))
      .def("get_turned_on_lights", CALL_RETURNING_LIST_1(cc::LightManager, GetTurnedOnLights, cr::LightState::LightGroup), (args("light_group") = cr::LightState::LightGroup::None ))
      .def("get_turned_off_lights", CALL_RETURNING_LIST_1(cc::LightManager, GetTurnedOffLights, cr::LightState::LightGroup), (args("light_group") = cr::LightState::LightGroup::None ))
      .def("set_color", &LightManagerSetColor, (arg("lights"), arg("color")))
      .def("set_colors", &LightManagerSetVectorColor, (arg("lights"), arg("colors")))
      .def("get_color", &LightManagerGetColor, (arg("lights")))
      .def("set_intensity", &LightManagerSetIntensity, (arg("lights"), arg("intensity")))
      .def("set_intensities", &LightManagerSetVectorIntensity, (arg("lights"), arg("intensities")))
      .def("get_intensity", &LightManagerGetIntensity, (arg("lights")))
      .def("set_light_group", &LightManagerSetLightGroup, (arg("lights"), arg("light_group")))
      .def("set_light_groups", &LightManagerSetVectorLightGroup, (arg("lights"), arg("light_groups")))
      .def("get_light_group", &LightManagerGetLightGroup, (arg("lights")))
      .def("set_light_state", &LightManagerSetLightState, (arg("lights"), arg("light_state")))
      .def("set_light_states", &LightManagerSetVectorLightState, (arg("lights"), arg("light_states")))
      .def("get_light_state", &LightManagerGetLightState, (arg("lights")))
      .def("set_day_night_cycle", &LightManagerSetDayNightCycle, (arg("active")))
    ;

}
