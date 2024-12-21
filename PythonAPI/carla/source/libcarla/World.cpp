// 版权所有 (c) 2017 巴萨罗那自治大学 (UAB) 的计算机视觉中心 (CVC)。
//
// 本作品遵循MIT许可证的条款进行许可。
// 许可证副本请参见 <https://opensource.org/licenses/MIT>。

// 引入carla库中的相关头文件
#include <carla/PythonUtil.h>
#include <carla/client/Actor.h>
#include <carla/client/ActorList.h>
#include <carla/client/World.h>
#include <carla/rpc/EnvironmentObject.h>
#include <carla/rpc/ObjectLabel.h>

// 引入标准库中的字符串处理功能
#include <string>

// 引入Boost Python库中的vector容器相关的功能
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

// carla命名空间
namespace carla {
namespace client {

  // 重载输出流操作符 "<<" 以方便打印ActorList对象
  // 该操作符将输出ActorList中的内容
  std::ostream &operator<<(std::ostream &out, const ActorList &actors) {
    // 调用PrintList来打印actor列表中的所有内容
    return PrintList(out, actors);
  }

  // 重载输出流操作符 "<<" 以方便打印World对象
  // 该操作符将输出World的id
  std::ostream &operator<<(std::ostream &out, const World &world) {
    out << "World(id=" << world.GetId() << ')';  // 输出World的id
    return out;
  }

} // namespace client
} // namespace carla

// carla命名空间中的rpc部分
namespace carla {
namespace rpc {

  // 重载输出流操作符 "<<" 以方便打印EpisodeSettings对象
  std::ostream &operator<<(std::ostream &out, const EpisodeSettings &settings) {
    // 帮助函数：将布尔值转换为字符串"True"或"False"
    auto BoolToStr = [](bool b) { return b ? "True" : "False"; };

    // 输出世界设置的各个参数，包括同步模式、渲染模式、固定时间步等
    out << "WorldSettings(synchronous_mode=" << BoolToStr(settings.synchronous_mode)
        << ",no_rendering_mode=" << BoolToStr(settings.no_rendering_mode)
        << ",fixed_delta_seconds=" << settings.fixed_delta_seconds.get()
        << ",substepping=" << BoolToStr(settings.substepping)
        << ",max_substep_delta_time=" << settings.max_substep_delta_time
        << ",max_substeps=" << settings.max_substeps
        << ",max_culling_distance=" << settings.max_culling_distance
        << ",deterministic_ragdolls=" << BoolToStr(settings.deterministic_ragdolls) << ')';
    return out;
  }

  // 重载输出流操作符 "<<" 以方便打印EnvironmentObject对象
  // 该操作符将输出环境对象的id、名称、变换矩阵和包围盒
  std::ostream &operator<<(std::ostream &out, const EnvironmentObject &environment_object) {
    out << "Mesh(id=" << environment_object.id << ", ";  // 输出Mesh的id
    out << "name=" << environment_object.name << ", ";  // 输出Mesh的名称
    out << "transform=" << environment_object.transform << ", ";  // 输出Mesh的变换矩阵
    out << "bounding_box=" << environment_object.bounding_box << ")";  // 输出Mesh的包围盒
    return out;
  }

} // namespace rpc
} // namespace carla


// 等待世界对象的一次 tick（时间步更新），在等待期间释放全局解释器锁（GIL），方便Python多线程等操作，返回等待后的结果
static auto WaitForTick(const carla::client::World &world, double seconds) {
// 释放Python全局解释器锁（GIL），以便在多线程环境中允许其他Python线程运行
  carla::PythonUtil::ReleaseGIL unlock;
  return world.WaitForTick(TimeDurationFromSeconds(seconds));
}

// 为世界对象注册一个 tick 回调函数，返回注册的回调函数的相关标识
static size_t OnTick(carla::client::World &self, boost::python::object callback) {
  return self.OnTick(MakeCallback(std::move(callback)));
}

// 执行世界对象的一次 tick（时间步更新）操作，期间释放全局解释器锁（GIL），并返回操作后的结果
static auto Tick(carla::client::World &world, double seconds) {
  carla::PythonUtil::ReleaseGIL unlock;
  return world.Tick(TimeDurationFromSeconds(seconds));
}

// 将给定的剧集设置应用到世界对象上，操作过程中释放全局解释器锁（GIL），并返回应用设置后的结果
static auto ApplySettings(carla::client::World &world, carla::rpc::EpisodeSettings settings, double seconds) {
  carla::PythonUtil::ReleaseGIL unlock;
  return world.ApplySettings(settings, TimeDurationFromSeconds(seconds));
}

// 根据给定的演员（Actor）ID列表，从世界对象中获取对应的演员列表，先将Python列表形式的ID转换为C++的向量形式，再获取演员，操作时释放GIL
static auto GetActorsById(carla::client::World &self, const boost::python::list &actor_ids) {
  std::vector<carla::ActorId> ids{
      boost::python::stl_input_iterator<carla::ActorId>(actor_ids),
      boost::python::stl_input_iterator<carla::ActorId>()};
  carla::PythonUtil::ReleaseGIL unlock;
  return self.GetActors(ids);
}

// 获取世界对象中所有车辆的灯光状态，并以Python字典形式返回，字典的键为车辆相关标识，值为对应的灯光状态
static auto GetVehiclesLightStates(carla::client::World &self) {
  boost::python::dict dict;
  auto list = self.GetVehiclesLightStates();
  for (auto &vehicle : list) {
    dict[vehicle.first] = vehicle.second;
  }
  return dict;
}

// 获取世界对象中特定标签对应的关卡边界框（Bounding Boxes）信息，并以Python列表形式返回，方便在Python环境中使用这些数据
static auto GetLevelBBs(const carla::client::World &self, uint8_t queried_tag) {
  boost::python::list result;
  for (const auto &bb : self.GetLevelBBs(queried_tag)) {
    result.append(bb);
  }
  return result;
}

// 获取世界对象中特定标签对应的环境对象信息，并以Python列表形式返回，便于在Python中进一步处理这些环境对象相关数据
static auto GetEnvironmentObjects(const carla::client::World &self, uint8_t queried_tag) {
  boost::python::list result;
  for (const auto &object : self.GetEnvironmentObjects(queried_tag)) {
    result.append(object);
  }
  return result;
}

// 根据Python对象中包含的环境对象ID列表，启用或禁用世界对象中的相应环境对象，先将Python对象中的ID转换为C++向量形式再操作
static void EnableEnvironmentObjects(
  carla::client::World &self,
  const boost::python::object& py_env_objects_ids,
  const bool enable ) {

  std::vector<uint64_t> env_objects_ids {
    boost::python::stl_input_iterator<uint64_t>(py_env_objects_ids),
    boost::python::stl_input_iterator<uint64_t>()
  };

  self.EnableEnvironmentObjects(env_objects_ids, enable);
}

// 定义函数export_world，用于将Carla相关的一些C++类通过Boost.Python库导出到Python环境，使其能在Python中使用
void export_world() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cg = carla::geom;
  namespace cr = carla::rpc;
  namespace csd = carla::sensor::data;

  // 定义Timestamp类到Python的映射，设置初始化参数、可读可写属性以及相等和不相等比较等操作的Python接口
  class_<cc::Timestamp>("Timestamp")
    .def(init<size_t, double, double, double>(
        (arg("frame")=0u,
         arg("elapsed_seconds")=0.0,
         arg("delta_seconds")=0.0,
         arg("platform_timestamp")=0.0)))
    .def_readwrite("frame", &cc::Timestamp::frame)
    .def_readwrite("frame_count", &cc::Timestamp::frame) // deprecated.
    .def_readwrite("elapsed_seconds", &cc::Timestamp::elapsed_seconds)
    .def_readwrite("delta_seconds", &cc::Timestamp::delta_seconds)
    .def_readwrite("platform_timestamp", &cc::Timestamp::platform_timestamp)
    .def("__eq__", &cc::Timestamp::operator==)
    .def("__ne__", &cc::Timestamp::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  // 将cc::ActorList类型绑定到Python中名为"ActorList"的类，无默认构造函数
  // 为Python中的"ActorList"类定义一些方法，使其能调用对应的C++方法
  class_<cc::ActorList, boost::shared_ptr<cc::ActorList>>("ActorList", no_init)
    // 绑定C++中ActorList类的Find方法到Python类的"find"方法，参数为"id"
    .def("find", &cc::ActorList::Find, (arg("id")))
    // 绑定Filter方法到Python类的"filter"方法，参数是"wildcard_pattern"
    .def("filter", &cc::ActorList::Filter, (arg("wildcard_pattern")))
    // 绑定at方法，使Python类支持通过索引访问，对应Python的"__getitem__"操作
    .def("__getitem__", &cc::ActorList::at)
    // 绑定size方法，让Python中可用len获取其长度，对应Python的"__len__"操作
    .def("__len__", &cc::ActorList::size)
    // 绑定迭代相关逻辑，使Python类可迭代，对应Python的"__iter__"操作
    .def("__iter__", range(&cc::ActorList::begin, &cc::ActorList::end))
    // 绑定流输出相关逻辑，让Python中可用str函数转为字符串表示
    .def(self_ns::str(self_ns::self))
  ;

  // 将cr::EpisodeSettings类型绑定到Python里名为"WorldSettings"的类
  // 定义构造函数及各参数默认值，方便Python中创建对象
  class_<cr::EpisodeSettings>("WorldSettings")
    .def(init<bool, bool, double, bool, double, int, float, bool, float, float, bool>(
        (arg("synchronous_mode")=false,
         arg("no_rendering_mode")=false,
         arg("fixed_delta_seconds")=0.0,
         arg("substepping")=true,
         arg("max_substep_delta_time")=0.01,
         arg("max_substeps")=10,
         arg("max_culling_distance")=0.0f,
         arg("deterministic_ragdolls")=false,
         arg("tile_stream_distance")=3000.f,
         arg("actor_active_distance")=2000.f,
         arg("spectator_as_ego")=true)))
    // 暴露C++类中的成员变量为Python类的可读写属性
    .def_readwrite("synchronous_mode", &cr::EpisodeSettings::synchronous_mode)
    .def_readwrite("no_rendering_mode", &cr::EpisodeSettings::no_rendering_mode)
    .def_readwrite("substepping", &cr::EpisodeSettings::substepping)
    .def_readwrite("max_substep_delta_time", &cr::EpisodeSettings::max_substep_delta_time)
    .def_readwrite("max_substeps", &cr::EpisodeSettings::max_substeps)
    .def_readwrite("max_culling_distance", &cr::EpisodeSettings::max_culling_distance)
    .def_readwrite("deterministic_ragdolls", &cr::EpisodeSettings::deterministic_ragdolls)
    // 定义名为"fixed_delta_seconds"的属性，有获取和设置的逻辑（通过lambda表达式实现）
    .add_property("fixed_delta_seconds",
        +[](const cr::EpisodeSettings &self) {
          return OptionalToPythonObject(self.fixed_delta_seconds);
        },
        +[](cr::EpisodeSettings &self, object value) {
          double fds = (value == object{} ? 0.0 : extract<double>(value));
          self.fixed_delta_seconds = fds > 0.0 ? fds : boost::optional<double>{};
        })
    .def_readwrite("tile_stream_distance", &cr::EpisodeSettings::tile_stream_distance)
    .def_readwrite("actor_active_distance", &cr::EpisodeSettings::actor_active_distance)
    .def_readwrite("spectator_as_ego", &cr::EpisodeSettings::spectator_as_ego)
     // 绑定相等比较（==）和不等比较（!=）的操作到Python类对应的方法
    .def("__eq__", &cr::EpisodeSettings::operator==)
    .def("__ne__", &cr::EpisodeSettings::operator!=)
     // 绑定流输出相关逻辑，使Python中可用str函数转换为字符串表示  
    .def(self_ns::str(self_ns::self))
  ;

  // 将cr::EnvironmentObject类型绑定到Python中名为"EnvironmentObject"的类
  // 把C++类的多个成员变量暴露为Python类的可读写属性 
  class_<cr::EnvironmentObject>("EnvironmentObject", no_init)
    .def_readwrite("transform", &cr::EnvironmentObject::transform)
    .def_readwrite("bounding_box", &cr::EnvironmentObject::bounding_box)
    .def_readwrite("id", &cr::EnvironmentObject::id)
    .def_readwrite("name", &cr::EnvironmentObject::name)
    .def_readwrite("type", &cr::EnvironmentObject::type)
    .def(self_ns::str(self_ns::self))
  ;

  enum_<cr::AttachmentType>("AttachmentType")
    .value("Rigid", cr::AttachmentType::Rigid)
    .value("SpringArm", cr::AttachmentType::SpringArm)
    .value("SpringArmGhost", cr::AttachmentType::SpringArmGhost)
  ;

  enum_<cr::CityObjectLabel>("CityObjectLabel")
    .value("NONE", cr::CityObjectLabel::None)
    .value("Buildings", cr::CityObjectLabel::Buildings)
    .value("Fences", cr::CityObjectLabel::Fences)
    .value("Other", cr::CityObjectLabel::Other)
    .value("Pedestrians", cr::CityObjectLabel::Pedestrians)
    .value("Poles", cr::CityObjectLabel::Poles)
    .value("RoadLines", cr::CityObjectLabel::RoadLines)
    .value("Roads", cr::CityObjectLabel::Roads)
    .value("Sidewalks", cr::CityObjectLabel::Sidewalks)
    .value("TrafficSigns", cr::CityObjectLabel::TrafficSigns)
    .value("Vegetation", cr::CityObjectLabel::Vegetation)
    .value("Car", cr::CityObjectLabel::Car)
    .value("Walls", cr::CityObjectLabel::Walls)
    .value("Sky", cr::CityObjectLabel::Sky)
    .value("Ground", cr::CityObjectLabel::Ground)
    .value("Bridge", cr::CityObjectLabel::Bridge)
    .value("RailTrack", cr::CityObjectLabel::RailTrack)
    .value("GuardRail", cr::CityObjectLabel::GuardRail)
    .value("TrafficLight", cr::CityObjectLabel::TrafficLight)
    .value("Static", cr::CityObjectLabel::Static)
    .value("Dynamic", cr::CityObjectLabel::Dynamic)
    .value("Water", cr::CityObjectLabel::Water)
    .value("Terrain", cr::CityObjectLabel::Terrain)
    .value("Truck", cr::CityObjectLabel::Truck)
    .value("Motorcycle", cr::CityObjectLabel::Motorcycle)
    .value("Bicycle", cr::CityObjectLabel::Bicycle)
    .value("Bus", cr::CityObjectLabel::Bus)
    .value("Rider", cr::CityObjectLabel::Rider)
    .value("Train", cr::CityObjectLabel::Train)
    .value("Any", cr::CityObjectLabel::Any)
  ;

  class_<cr::LabelledPoint>("LabelledPoint", no_init)
    .def_readonly("location", &cr::LabelledPoint::_location)
    .def_readonly("label", &cr::LabelledPoint::_label)
  ;

  enum_<cr::MapLayer>("MapLayer")
    .value("NONE", cr::MapLayer::None)
    .value("Buildings", cr::MapLayer::Buildings)
    .value("Decals", cr::MapLayer::Decals)
    .value("Foliage", cr::MapLayer::Foliage)
    .value("Ground", cr::MapLayer::Ground)
    .value("ParkedVehicles", cr::MapLayer::ParkedVehicles)
    .value("Particles", cr::MapLayer::Particles)
    .value("Props", cr::MapLayer::Props)
    .value("StreetLights", cr::MapLayer::StreetLights)
    .value("Walls", cr::MapLayer::Walls)
    .value("All", cr::MapLayer::All)
  ;

  enum_<cr::MaterialParameter>("MaterialParameter")
    .value("Normal", cr::MaterialParameter::Tex_Normal)
    .value("AO_Roughness_Metallic_Emissive", cr::MaterialParameter::Tex_Ao_Roughness_Metallic_Emissive)
    .value("Diffuse", cr::MaterialParameter::Tex_Diffuse)
    .value("Emissive", cr::MaterialParameter::Tex_Emissive)
  ;

  class_<cr::TextureColor>("TextureColor")
    .def(init<uint32_t, uint32_t>())
    .add_property("width", &cr::TextureColor::GetWidth)
    .add_property("height", &cr::TextureColor::GetHeight)
    .def("set_dimensions", &cr::TextureColor::SetDimensions)
    .def("get", +[](const cr::TextureColor &self, int x, int y) -> csd::Color{
      return self.At(static_cast<uint32_t>(x), static_cast<uint32_t>(y));
    })
    .def("set", +[](cr::TextureColor &self, int x, int y, csd::Color& value) {
      self.At(static_cast<uint32_t>(x), static_cast<uint32_t>(y)) = value;
    })
  ;

  class_<cr::TextureFloatColor>("TextureFloatColor")
    .def(init<uint32_t, uint32_t>())
    .add_property("width", &cr::TextureFloatColor::GetWidth)
    .add_property("height", &cr::TextureFloatColor::GetHeight)
    .def("set_dimensions", &cr::TextureFloatColor::SetDimensions)
    .def("get", +[](const cr::TextureFloatColor &self, int x, int y) -> cr::FloatColor{
      return self.At(static_cast<uint32_t>(x), static_cast<uint32_t>(y));
    })
    .def("set", +[](cr::TextureFloatColor &self, int x, int y, cr::FloatColor& value) {
      self.At(static_cast<uint32_t>(x), static_cast<uint32_t>(y)) = value;
    })
  ;

#define SPAWN_ACTOR_WITHOUT_GIL(fn) +[]( \
        cc::World &self, \
        const cc::ActorBlueprint &blueprint, \
        const cg::Transform &transform, \
        cc::Actor *parent, \
        cr::AttachmentType attachment_type, \
        const std::string& bone) { \
      carla::PythonUtil::ReleaseGIL unlock; \
      return self.fn(blueprint, transform, parent, attachment_type, bone); \
    }, \
    ( \
      arg("blueprint"), \
      arg("transform"), \
      arg("attach_to")=carla::SharedPtr<cc::Actor>(), \
      arg("attachment_type")=cr::AttachmentType::Rigid, \
      arg("bone")=std::string())

  class_<cc::World>("World", no_init)
    .add_property("id", &cc::World::GetId)
    .add_property("debug", &cc::World::MakeDebugHelper)
    .def("load_map_layer", CONST_CALL_WITHOUT_GIL_1(cc::World, LoadLevelLayer, cr::MapLayer), arg("map_layers"))
    .def("unload_map_layer", CONST_CALL_WITHOUT_GIL_1(cc::World, UnloadLevelLayer, cr::MapLayer), arg("map_layers"))
    .def("get_blueprint_library", CONST_CALL_WITHOUT_GIL(cc::World, GetBlueprintLibrary))
    .def("get_vehicles_light_states", &GetVehiclesLightStates)
    .def("get_map", CONST_CALL_WITHOUT_GIL(cc::World, GetMap))
    .def("get_random_location_from_navigation", CALL_RETURNING_OPTIONAL_WITHOUT_GIL(cc::World, GetRandomLocationFromNavigation))
    .def("get_spectator", CONST_CALL_WITHOUT_GIL(cc::World, GetSpectator))
    .def("get_settings", CONST_CALL_WITHOUT_GIL(cc::World, GetSettings))
    .def("apply_settings", &ApplySettings, (arg("settings"), arg("seconds")=0.0))
    .def("get_weather", CONST_CALL_WITHOUT_GIL(cc::World, GetWeather))
    .def("set_weather", &cc::World::SetWeather)
    .def("get_imui_sensor_gravity", CONST_CALL_WITHOUT_GIL(cc::World, GetIMUISensorGravity))
    .def("set_imui_sensor_gravity", &cc::World::SetIMUISensorGravity, (arg("NewIMUISensorGravity")) )
    .def("get_snapshot", &cc::World::GetSnapshot)
    .def("get_actor", CONST_CALL_WITHOUT_GIL_1(cc::World, GetActor, carla::ActorId), (arg("actor_id")))
    .def("get_actors", CONST_CALL_WITHOUT_GIL(cc::World, GetActors))
    .def("get_actors", &GetActorsById, (arg("actor_ids")))
    .def("spawn_actor", SPAWN_ACTOR_WITHOUT_GIL(SpawnActor))
    .def("try_spawn_actor", SPAWN_ACTOR_WITHOUT_GIL(TrySpawnActor))
    .def("wait_for_tick", &WaitForTick, (arg("seconds")=0.0))
    .def("on_tick", &OnTick, (arg("callback")))
    .def("remove_on_tick", &cc::World::RemoveOnTick, (arg("callback_id")))
    .def("tick", &Tick, (arg("seconds")=0.0))
    .def("set_pedestrians_cross_factor", CALL_WITHOUT_GIL_1(cc::World, SetPedestriansCrossFactor, float), (arg("percentage")))
    .def("set_pedestrians_seed", CALL_WITHOUT_GIL_1(cc::World, SetPedestriansSeed, unsigned int), (arg("seed")))
    .def("get_traffic_sign", CONST_CALL_WITHOUT_GIL_1(cc::World, GetTrafficSign, cc::Landmark), arg("landmark"))
    .def("get_traffic_light", CONST_CALL_WITHOUT_GIL_1(cc::World, GetTrafficLight, cc::Landmark), arg("landmark"))
    .def("get_traffic_light_from_opendrive_id", CONST_CALL_WITHOUT_GIL_1(cc::World, GetTrafficLightFromOpenDRIVE, const carla::road::SignId&), arg("traffic_light_id"))
    .def("get_traffic_lights_from_waypoint", CALL_RETURNING_LIST_2(cc::World, GetTrafficLightsFromWaypoint, const cc::Waypoint&, double), (arg("waypoint"), arg("distance")))
    .def("get_traffic_lights_in_junction", CALL_RETURNING_LIST_1(cc::World, GetTrafficLightsInJunction, carla::road::JuncId), (arg("junction_id")))
    .def("reset_all_traffic_lights", &cc::World::ResetAllTrafficLights)
    .def("get_lightmanager", CONST_CALL_WITHOUT_GIL(cc::World, GetLightManager))
    .def("freeze_all_traffic_lights", &cc::World::FreezeAllTrafficLights, (arg("frozen")))
    .def("get_level_bbs", &GetLevelBBs, (arg("bb_type")=cr::CityObjectLabel::Any))
    .def("get_environment_objects", &GetEnvironmentObjects, (arg("object_type")=cr::CityObjectLabel::Any))
    .def("enable_environment_objects", &EnableEnvironmentObjects, (arg("env_objects_ids"), arg("enable")))
    .def("cast_ray", CALL_RETURNING_LIST_2(cc::World, CastRay, cg::Location, cg::Location), (arg("initial_location"), arg("final_location")))
    .def("project_point", CALL_RETURNING_OPTIONAL_3(cc::World, ProjectPoint, cg::Location, cg::Vector3D, float), (arg("location"), arg("direction"), arg("search_distance")=10000.f))
    .def("ground_projection", CALL_RETURNING_OPTIONAL_2(cc::World, GroundProjection, cg::Location, float), (arg("location"), arg("search_distance")=10000.f))
    .def("get_names_of_all_objects", CALL_RETURNING_LIST(cc::World, GetNamesOfAllObjects))
    .def("apply_color_texture_to_object", &cc::World::ApplyColorTextureToObject, (arg("object_name"), arg("material_parameter"), arg("texture")))
    .def("apply_float_color_texture_to_object", &cc::World::ApplyFloatColorTextureToObject, (arg("object_name"), arg("material_parameter"), arg("texture")))
    .def("apply_textures_to_object", &cc::World::ApplyTexturesToObject, (arg("object_name"), arg("diffuse_texture"), arg("emissive_texture"), arg("normal_texture"), arg("ao_roughness_metallic_emissive_texture")))
    .def("apply_color_texture_to_objects", +[](cc::World &self, boost::python::list &list, const cr::MaterialParameter& parameter, const cr::TextureColor& Texture) {
        self.ApplyColorTextureToObjects(PythonLitstToVector<std::string>(list), parameter, Texture);
      }, (arg("objects_name_list"), arg("material_parameter"), arg("texture")))
    .def("apply_float_color_texture_to_objects", +[](cc::World &self, boost::python::list &list, const cr::MaterialParameter& parameter, const cr::TextureFloatColor& Texture) {
        self.ApplyFloatColorTextureToObjects(PythonLitstToVector<std::string>(list), parameter, Texture);
      }, (arg("objects_name_list"), arg("material_parameter"), arg("texture")))
    .def("apply_textures_to_objects", +[](cc::World &self, boost::python::list &list, const cr::TextureColor& diffuse_texture, const cr::TextureFloatColor& emissive_texture, const cr::TextureFloatColor& normal_texture, const cr::TextureFloatColor& ao_roughness_metallic_emissive_texture) {
        self.ApplyTexturesToObjects(PythonLitstToVector<std::string>(list), diffuse_texture, emissive_texture, normal_texture, ao_roughness_metallic_emissive_texture);
      }, (arg("objects_name_list"), arg("diffuse_texture"), arg("emissive_texture"), arg("normal_texture"), arg("ao_roughness_metallic_emissive_texture")))
    .def(self_ns::str(self_ns::self))
  ;

#undef SPAWN_ACTOR_WITHOUT_GIL

  class_<cc::DebugHelper>("DebugHelper", no_init)
    .def("draw_point", &cc::DebugHelper::DrawPoint,
        (arg("location"),
         arg("size")=0.1f,
         arg("color")=cc::DebugHelper::Color(255u, 0u, 0u),
         arg("life_time")=-1.0f,
         arg("persistent_lines")=true))
    .def("draw_hud_point", &cc::DebugHelper::DrawHUDPoint,
        (arg("location"),
         arg("size")=0.1f,
         arg("color")=cc::DebugHelper::Color(255u, 0u, 0u),
         arg("life_time")=-1.0f,
         arg("persistent_lines")=true))
    .def("draw_line", &cc::DebugHelper::DrawLine,
        (arg("begin"),
         arg("end"),
         arg("thickness")=0.1f,
         arg("color")=cc::DebugHelper::Color(255u, 0u, 0u),
         arg("life_time")=-1.0f,
         arg("persistent_lines")=true))
    .def("draw_hud_line", &cc::DebugHelper::DrawHUDLine,
        (arg("begin"),
         arg("end"),
         arg("thickness")=0.1f,
         arg("color")=cc::DebugHelper::Color(255u, 0u, 0u),
         arg("life_time")=-1.0f,
         arg("persistent_lines")=true))
    .def("draw_arrow", &cc::DebugHelper::DrawArrow,
        (arg("begin"),
         arg("end"),
         arg("thickness")=0.1f,
         arg("arrow_size")=0.1f,
         arg("color")=cc::DebugHelper::Color(255u, 0u, 0u),
         arg("life_time")=-1.0f,
         arg("persistent_lines")=true))
    .def("draw_hud_arrow", &cc::DebugHelper::DrawHUDArrow,
        (arg("begin"),
         arg("end"),
         arg("thickness")=0.1f,
         arg("arrow_size")=0.1f,
         arg("color")=cc::DebugHelper::Color(255u, 0u, 0u),
         arg("life_time")=-1.0f,
         arg("persistent_lines")=true))
    .def("draw_box", &cc::DebugHelper::DrawBox,
        (arg("box"),
         arg("rotation"),
         arg("thickness")=0.1f,
         arg("color")=cc::DebugHelper::Color(255u, 0u, 0u),
         arg("life_time")=-1.0f,
         arg("persistent_lines")=true))
    .def("draw_hud_box", &cc::DebugHelper::DrawHUDBox,
        (arg("box"),
         arg("rotation"),
         arg("thickness")=0.1f,
         arg("color")=cc::DebugHelper::Color(255u, 0u, 0u),
         arg("life_time")=-1.0f,
         arg("persistent_lines")=true))
    .def("draw_string", &cc::DebugHelper::DrawString,
        (arg("location"),
         arg("text"),
         arg("draw_shadow")=false,
         arg("color")=cc::DebugHelper::Color(255u, 0u, 0u),
         arg("life_time")=-1.0f,
         arg("persistent_lines")=true))
  ;
  // scope HUD = class_<cc::DebugHelper>(

}
