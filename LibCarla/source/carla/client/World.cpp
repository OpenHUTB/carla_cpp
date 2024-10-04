// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/World.h"  // 引入World类的定义

#include "carla/Logging.h"  // 引入日志记录功能
#include "carla/client/Actor.h"  // 引入参与者类的定义
#include "carla/client/ActorBlueprint.h"  // 引入参与者蓝图类的定义
#include "carla/client/ActorList.h"  // 引入参与者列表类的定义
#include "carla/client/detail/Simulator.h"  // 引入模拟器的详细信息
#include "carla/StringUtil.h"  // 引入字符串工具
#include "carla/road/SignalType.h"  // 引入信号类型定义
#include "carla/road/Junction.h"  // 引入交叉口类的定义
#include "carla/client/TrafficLight.h"  // 引入交通灯类的定义

#include <exception>  // 引入异常处理

namespace carla {
namespace client {

  SharedPtr<Map> World::GetMap() const {  // 获取地图的方法
    return _episode.Lock()->GetCurrentMap();  // 返回当前地图
  }

  void World::LoadLevelLayer(rpc::MapLayer map_layers) const {  // 加载地图层的方法
    _episode.Lock()->LoadLevelLayer(map_layers);  // 加载指定的地图层
  }

  void World::UnloadLevelLayer(rpc::MapLayer map_layers) const {  // 卸载地图层的方法
    _episode.Lock()->UnloadLevelLayer(map_layers);  // 卸载指定的地图层
  }

  SharedPtr<BlueprintLibrary> World::GetBlueprintLibrary() const {  // 获取蓝图库的方法
    return _episode.Lock()->GetBlueprintLibrary();  // 返回蓝图库
  }

  rpc::VehicleLightStateList World::GetVehiclesLightStates() const {  // 获取车辆灯光状态的方法
    return _episode.Lock()->GetVehiclesLightStates();  // 返回车辆灯光状态列表
  }

  boost::optional<geom::Location> World::GetRandomLocationFromNavigation() const {  // 获取随机导航位置的方法
    return _episode.Lock()->GetRandomLocationFromNavigation();  // 返回随机导航位置
  }

  SharedPtr<Actor> World::GetSpectator() const {  // 获取观众的方法
    return _episode.Lock()->GetSpectator();  // 返回当前观众
  }

  rpc::EpisodeSettings World::GetSettings() const {  // 获取设置的方法
    return _episode.Lock()->GetEpisodeSettings();  // 返回当前剧集设置
  }

  uint64_t World::ApplySettings(const rpc::EpisodeSettings &settings, time_duration timeout) {  // 应用设置的方法
    rpc::EpisodeSettings new_settings = settings;  // 复制新的设置
    uint64_t id = _episode.Lock()->SetEpisodeSettings(settings);  // 设置新的剧集设置并返回ID

    time_duration local_timeout = timeout.milliseconds() == 0 ?  // 判断超时设置
        _episode.Lock()->GetNetworkingTimeout() : timeout;  // 如果没有设置，则使用默认网络超时

    if (settings.fixed_delta_seconds.has_value()) {  // 如果有固定的时间间隔设置
      using namespace std::literals::chrono_literals;  // 使用时间字面量

      const auto number_of_attemps = 30u;  // 定义最大尝试次数
      uint64_t tics_correct = 0;  // 记录正确的滴答数
      for (auto i = 0u; i < number_of_attemps; i++) {  // 尝试应用设置
        const auto curr_snapshot = GetSnapshot();  // 获取当前快照

        const double error = abs(new_settings.fixed_delta_seconds.get() - curr_snapshot.GetTimestamp().delta_seconds);  // 计算误差
        if (error < std::numeric_limits<float>::epsilon())  // 如果误差在允许范围内
          tics_correct++;  // 正确滴答数加一

        if (tics_correct >= 2)  // 如果正确滴答数达到2
          return id;  // 返回设置ID

        Tick(local_timeout);  // 执行一次Tick操作
      }

      log_warning("World::ApplySettings: After", number_of_attemps, " attempts, the settings were not correctly set. Please check that everything is consistent.");  // 日志警告
    }
    return id;  // 返回设置ID
  }

  rpc::WeatherParameters World::GetWeather() const {  // 获取天气的方法
    return _episode.Lock()->GetWeatherParameters();  // 返回当前天气参数
  }

  void World::SetWeather(const rpc::WeatherParameters &weather) {  // 设置天气的方法
    _episode.Lock()->SetWeatherParameters(weather);  // 应用新的天气参数
  }

  float World::GetIMUISensorGravity() const {  // 获取IMU传感器重力的方法
    return _episode.Lock()->GetIMUISensorGravity();  // 返回当前重力设置
  }

  void World::SetIMUISensorGravity(float NewIMUISensorGravity) {  // 设置IMU传感器重力的方法
    _episode.Lock()->SetIMUISensorGravity(NewIMUISensorGravity);  // 应用新的重力设置
  }

  WorldSnapshot World::GetSnapshot() const {  // 获取世界快照的方法
    return _episode.Lock()->GetWorldSnapshot();  // 返回当前世界快照
  }

  SharedPtr<Actor> World::GetActor(ActorId id) const {  // 根据ID获取参与者的方法
    auto simulator = _episode.Lock();  // 锁定当前剧集
    auto description = simulator->GetActorById(id);  // 获取指定ID的参与者描述
    return description.has_value() ?  // 如果参与者存在
        simulator->MakeActor(std::move(*description)) :  // 创建并返回参与者实例
        nullptr;  // 否则返回空指针
  }

  SharedPtr<ActorList> World::GetActors() const {  // 获取所有参与者的方法
    return SharedPtr<ActorList>{new ActorList{  // 返回新的参与者列表
                                  _episode,
                                  _episode.Lock()->GetAllTheActorsInTheEpisode()}};  // 获取所有参与者
  }

  SharedPtr<ActorList> World::GetActors(const std::vector<ActorId> &actor_ids) const {  // 根据ID列表获取参与者的方法
    return SharedPtr<ActorList>{new ActorList{  // 返回新的参与者列表
                                  _episode,
                                  _episode.Lock()->GetActorsById(actor_ids)}};  // 根据ID获取参与者列表
  }

  SharedPtr<Actor> World::SpawnActor(
      const ActorBlueprint &blueprint,
      const geom::Transform &transform,
      Actor *parent_actor,
      rpc::AttachmentType attachment_type,
      const std::string& socket_name) {
    return _episode.Lock()->SpawnActor(blueprint, transform, parent_actor, attachment_type, GarbageCollectionPolicy::Inherit, socket_name);
  }

  SharedPtr<Actor> World::TrySpawnActor(
      const ActorBlueprint &blueprint,
      const geom::Transform &transform,
      Actor *parent_actor,
      rpc::AttachmentType attachment_type,
      const std::string& socket_name) noexcept {
    try {
      return SpawnActor(blueprint, transform, parent_actor, attachment_type, socket_name);
    } catch (const std::exception &) {
      return nullptr;
    }
  }

  WorldSnapshot World::WaitForTick(time_duration timeout) const {
    time_duration local_timeout = timeout.milliseconds() == 0 ?
        _episode.Lock()->GetNetworkingTimeout() : timeout;

    return _episode.Lock()->WaitForTick(local_timeout);
  }

  size_t World::OnTick(std::function<void(WorldSnapshot)> callback) {
    return _episode.Lock()->RegisterOnTickEvent(std::move(callback));
  }

  void World::RemoveOnTick(size_t callback_id) {
    _episode.Lock()->RemoveOnTickEvent(callback_id);
  }

  uint64_t World::Tick(time_duration timeout) {
    time_duration local_timeout = timeout.milliseconds() == 0 ?
        _episode.Lock()->GetNetworkingTimeout() : timeout;
    return _episode.Lock()->Tick(local_timeout);
  }

  void World::SetPedestriansCrossFactor(float percentage) {
    _episode.Lock()->SetPedestriansCrossFactor(percentage);
  }

  void World::SetPedestriansSeed(unsigned int seed) {
    _episode.Lock()->SetPedestriansSeed(seed);
  }

  SharedPtr<Actor> World::GetTrafficSign(const Landmark& landmark) const {
    SharedPtr<ActorList> actors = GetActors();
    SharedPtr<TrafficSign> result;
    std::string landmark_id = landmark.GetId();
    for (size_t i = 0; i < actors->size(); i++) {
      SharedPtr<Actor> actor = actors->at(i);
      if (StringUtil::Match(actor->GetTypeId(), "*traffic.*")) {
        TrafficSign* sign = static_cast<TrafficSign*>(actor.get());
        if(sign && (sign->GetSignId() == landmark_id)) {
          return actor;
        }
      }
    }
    return nullptr;
  }

  SharedPtr<Actor> World::GetTrafficLight(const Landmark& landmark) const {
    SharedPtr<ActorList> actors = GetActors();
    SharedPtr<TrafficLight> result;
    std::string landmark_id = landmark.GetId();
    for (size_t i = 0; i < actors->size(); i++) {
      SharedPtr<Actor> actor = actors->at(i);
      if (StringUtil::Match(actor->GetTypeId(), "*traffic_light*")) {
        TrafficLight* tl = static_cast<TrafficLight*>(actor.get());
        if(tl && (tl->GetSignId() == landmark_id)) {
          return actor;
        }
      }
    }
    return nullptr;
  }

  SharedPtr<Actor> World::GetTrafficLightFromOpenDRIVE(const road::SignId& sign_id) const {
    SharedPtr<ActorList> actors = GetActors();
    SharedPtr<TrafficLight> result;
    for (size_t i = 0; i < actors->size(); i++) {
      SharedPtr<Actor> actor = actors->at(i);
      if (StringUtil::Match(actor->GetTypeId(), "*traffic_light*")) {
        TrafficLight* tl = static_cast<TrafficLight*>(actor.get());
        if(tl && (tl->GetSignId() == sign_id)) {
          return actor;
        }
      }
    }
    return nullptr;
  }

  void World::ResetAllTrafficLights() {
    _episode.Lock()->ResetAllTrafficLights();
  }

  SharedPtr<LightManager> World::GetLightManager() const {
    return _episode.Lock()->GetLightManager();
  }

  void World::FreezeAllTrafficLights(bool frozen) {
    _episode.Lock()->FreezeAllTrafficLights(frozen);
  }

  std::vector<geom::BoundingBox> World::GetLevelBBs(uint8_t queried_tag) const {
    return _episode.Lock()->GetLevelBBs(queried_tag);
  }

  std::vector<rpc::EnvironmentObject> World::GetEnvironmentObjects(uint8_t queried_tag) const {
    return _episode.Lock()->GetEnvironmentObjects(queried_tag);
  }

  void World::EnableEnvironmentObjects(
      std::vector<uint64_t> env_objects_ids,
      bool enable) const {
    _episode.Lock()->EnableEnvironmentObjects(env_objects_ids, enable);
  }

  boost::optional<rpc::LabelledPoint> World::ProjectPoint(
      geom::Location location, geom::Vector3D direction, float search_distance) const {
    auto result = _episode.Lock()->ProjectPoint(location, direction, search_distance);
    if (result.first) {
      return result.second;
    }
    return {};
  }

  boost::optional<rpc::LabelledPoint> World::GroundProjection(
      geom::Location location, float search_distance) const {
    const geom::Vector3D DownVector(0,0,-1);
    return ProjectPoint(location, DownVector, search_distance);
  }

  std::vector<rpc::LabelledPoint> World::CastRay(
      geom::Location start_location, geom::Location end_location) const {
    return _episode.Lock()->CastRay(start_location, end_location);
  }

  std::vector<SharedPtr<Actor>> World::GetTrafficLightsFromWaypoint(
      const Waypoint& waypoint, double distance) const {
    std::vector<SharedPtr<Actor>> Result;
    std::vector<SharedPtr<Landmark>> landmarks =
        waypoint.GetAllLandmarksInDistance(distance);
    std::set<std::string> added_signals;
    for (auto& landmark : landmarks) {
      if (road::SignalType::IsTrafficLight(landmark->GetType())) {
        SharedPtr<Actor> traffic_light = GetTrafficLight(*(landmark.get()));
        if (traffic_light) {
          if(added_signals.count(landmark->GetId()) == 0) {
            Result.emplace_back(traffic_light);
            added_signals.insert(landmark->GetId());
          }
        }
      }
    }
    return Result;
  }

  std::vector<SharedPtr<Actor>> World::GetTrafficLightsInJunction(
      const road::JuncId junc_id) const {
    std::vector<SharedPtr<Actor>> Result;
    SharedPtr<Map> map = GetMap();
    const road::Junction* junction = map->GetMap().GetJunction(junc_id);
    for (const road::ContId& cont_id : junction->GetControllers()) {
      const std::unique_ptr<road::Controller>& controller =
          map->GetMap().GetControllers().at(cont_id);
      for (road::SignId sign_id : controller->GetSignals()) {
        SharedPtr<Actor> traffic_light = GetTrafficLightFromOpenDRIVE(sign_id);
        if (traffic_light) {
          Result.emplace_back(traffic_light);
        }
      }
    }
    return Result;
  }

  void World::ApplyColorTextureToObject(
      const std::string &object_name,
      const rpc::MaterialParameter& parameter,
      const rpc::TextureColor& Texture) {
    _episode.Lock()->ApplyColorTextureToObjects({object_name}, parameter, Texture);
  }

  void World::ApplyColorTextureToObjects(
      const std::vector<std::string> &objects_name,
      const rpc::MaterialParameter& parameter,
      const rpc::TextureColor& Texture) {
    _episode.Lock()->ApplyColorTextureToObjects(objects_name, parameter, Texture);
  }

  void World::ApplyFloatColorTextureToObject(
      const std::string &object_name,
      const rpc::MaterialParameter& parameter,
      const rpc::TextureFloatColor& Texture) {
    _episode.Lock()->ApplyColorTextureToObjects({object_name}, parameter, Texture);
  }

  void World::ApplyFloatColorTextureToObjects(
      const std::vector<std::string> &objects_name,
      const rpc::MaterialParameter& parameter,
      const rpc::TextureFloatColor& Texture) {
    _episode.Lock()->ApplyColorTextureToObjects(objects_name, parameter, Texture);
  }

  std::vector<std::string> World::GetNamesOfAllObjects() const {
    return _episode.Lock()->GetNamesOfAllObjects();
  }

  void World::ApplyTexturesToObject(
      const std::string &object_name,
      const rpc::TextureColor& diffuse_texture,
      const rpc::TextureFloatColor& emissive_texture,
      const rpc::TextureFloatColor& normal_texture,
      const rpc::TextureFloatColor& ao_roughness_metallic_emissive_texture)
  {
    if (diffuse_texture.GetWidth() && diffuse_texture.GetHeight()) {
      ApplyColorTextureToObject(
          object_name, rpc::MaterialParameter::Tex_Diffuse, diffuse_texture);
    }
    if (normal_texture.GetWidth() && normal_texture.GetHeight()) {
      ApplyFloatColorTextureToObject(
          object_name, rpc::MaterialParameter::Tex_Normal, normal_texture);
    }
    if (ao_roughness_metallic_emissive_texture.GetWidth() &&
        ao_roughness_metallic_emissive_texture.GetHeight()) {
      ApplyFloatColorTextureToObject(
          object_name,
          rpc::MaterialParameter::Tex_Ao_Roughness_Metallic_Emissive,
          ao_roughness_metallic_emissive_texture);
    }
    if (emissive_texture.GetWidth() && emissive_texture.GetHeight()) {
      ApplyFloatColorTextureToObject(
          object_name, rpc::MaterialParameter::Tex_Emissive, emissive_texture);
    }
  }

  void World::ApplyTexturesToObjects(
      const std::vector<std::string> &objects_names,
      const rpc::TextureColor& diffuse_texture,
      const rpc::TextureFloatColor& emissive_texture,
      const rpc::TextureFloatColor& normal_texture,
      const rpc::TextureFloatColor& ao_roughness_metallic_emissive_texture)
  {
    if (diffuse_texture.GetWidth() && diffuse_texture.GetHeight()) {
      ApplyColorTextureToObjects(
          objects_names, rpc::MaterialParameter::Tex_Diffuse, diffuse_texture);
    }
    if (normal_texture.GetWidth() && normal_texture.GetHeight()) {
      ApplyFloatColorTextureToObjects(
          objects_names, rpc::MaterialParameter::Tex_Normal, normal_texture);
    }
    if (ao_roughness_metallic_emissive_texture.GetWidth() &&
        ao_roughness_metallic_emissive_texture.GetHeight()) {
      ApplyFloatColorTextureToObjects(
          objects_names,
          rpc::MaterialParameter::Tex_Ao_Roughness_Metallic_Emissive,
          ao_roughness_metallic_emissive_texture);
    }
    if (emissive_texture.GetWidth() && emissive_texture.GetHeight()) {
      ApplyFloatColorTextureToObjects(
          objects_names, rpc::MaterialParameter::Tex_Emissive, emissive_texture);
    }
  }

} // namespace client
} // namespace carla
