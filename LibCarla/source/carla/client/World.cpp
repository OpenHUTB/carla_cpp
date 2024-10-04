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
      const ActorBlueprint &blueprint, // 参与者蓝图
      const geom::Transform &transform, // 变换信息
      Actor *parent_actor, // 父参与者
      rpc::AttachmentType attachment_type, // 附加类型
      const std::string& socket_name) { // 套接字名称
    return _episode.Lock()->SpawnActor(blueprint, transform, parent_actor, attachment_type, GarbageCollectionPolicy::Inherit, socket_name);
    // 使用给定参数生成新的参与者
  }

  SharedPtr<Actor> World::TrySpawnActor(
      const ActorBlueprint &blueprint, // 参与者蓝图
      const geom::Transform &transform, // 变换信息
      Actor *parent_actor, // 父参与者
      rpc::AttachmentType attachment_type, // 附加类型
      const std::string& socket_name) noexcept { // 套接字名称
    try {
      return SpawnActor(blueprint, transform, parent_actor, attachment_type, socket_name);
      // 尝试生成参与者，若成功返回参与者指针
    } catch (const std::exception &) {
      return nullptr; // 发生异常时返回空指针
    }
  }

  WorldSnapshot World::WaitForTick(time_duration timeout) const { // 等待tick时间
    time_duration local_timeout = timeout.milliseconds() == 0 ?
        _episode.Lock()->GetNetworkingTimeout() : timeout; // 根据情况确定超时时间

    return _episode.Lock()->WaitForTick(local_timeout); // 等待并返回快照
  }

  size_t World::OnTick(std::function<void(WorldSnapshot)> callback) { // 注册tick事件
    return _episode.Lock()->RegisterOnTickEvent(std::move(callback)); // 返回回调ID
  }

  void World::RemoveOnTick(size_t callback_id) { // 移除tick事件
    _episode.Lock()->RemoveOnTickEvent(callback_id); // 根据ID移除
  }

  uint64_t World::Tick(time_duration timeout) { // 执行tick操作
    time_duration local_timeout = timeout.milliseconds() == 0 ?
        _episode.Lock()->GetNetworkingTimeout() : timeout; // 确定超时时间
    return _episode.Lock()->Tick(local_timeout); // 执行tick并返回结果
  }

  void World::SetPedestriansCrossFactor(float percentage) { // 设置行人过街因子
    _episode.Lock()->SetPedestriansCrossFactor(percentage); // 更新因子
  }

  void World::SetPedestriansSeed(unsigned int seed) { // 设置行人种子
    _episode.Lock()->SetPedestriansSeed(seed); // 更新种子值
  }

  SharedPtr<Actor> World::GetTrafficSign(const Landmark& landmark) const { // 获取交通标志
    SharedPtr<ActorList> actors = GetActors(); // 获取所有参与者
    SharedPtr<TrafficSign> result; // 结果变量
    std::string landmark_id = landmark.GetId(); // 获取地标ID
    for (size_t i = 0; i < actors->size(); i++) {
      SharedPtr<Actor> actor = actors->at(i); // 遍历参与者
      if (StringUtil::Match(actor->GetTypeId(), "*traffic.*")) { // 匹配交通标志类型
        TrafficSign* sign = static_cast<TrafficSign*>(actor.get()); // 转换为交通标志指针
        if(sign && (sign->GetSignId() == landmark_id)) { // 匹配ID
          return actor; // 返回匹配的参与者
        }
      }
    }
    return nullptr; // 未找到时返回空指针
  }

  SharedPtr<Actor> World::GetTrafficLight(const Landmark& landmark) const { // 获取交通信号灯
    SharedPtr<ActorList> actors = GetActors(); // 获取所有参与者
    SharedPtr<TrafficLight> result; // 结果变量
    std::string landmark_id = landmark.GetId(); // 获取地标ID
    for (size_t i = 0; i < actors->size(); i++) {
      SharedPtr<Actor> actor = actors->at(i); // 遍历参与者
      if (StringUtil::Match(actor->GetTypeId(), "*traffic_light*")) { // 匹配交通灯类型
        TrafficLight* tl = static_cast<TrafficLight*>(actor.get()); // 转换为交通灯指针
        if(tl && (tl->GetSignId() == landmark_id)) { // 匹配ID
          return actor; // 返回匹配的参与者
        }
      }
    }
    return nullptr; // 未找到时返回空指针
  }

  SharedPtr<Actor> World::GetTrafficLightFromOpenDRIVE(const road::SignId& sign_id) const { // 从OpenDRIVE获取交通信号灯
    SharedPtr<ActorList> actors = GetActors(); // 获取所有参与者
    SharedPtr<TrafficLight> result; // 结果变量
    for (size_t i = 0; i < actors->size(); i++) {
      SharedPtr<Actor> actor = actors->at(i); // 遍历参与者
      if (StringUtil::Match(actor->GetTypeId(), "*traffic_light*")) { // 匹配交通灯类型
        TrafficLight* tl = static_cast<TrafficLight*>(actor.get()); // 转换为交通灯指针
        if(tl && (tl->GetSignId() == sign_id)) { // 匹配ID
          return actor; // 返回匹配的参与者
        }
      }
    }
    return nullptr; // 未找到时返回空指针
  }

  void World::ResetAllTrafficLights() { // 重置所有交通信号灯
    _episode.Lock()->ResetAllTrafficLights(); // 调用重置方法
  }

  SharedPtr<LightManager> World::GetLightManager() const { // 获取光照管理器
    return _episode.Lock()->GetLightManager(); // 返回光照管理器
  }

  void World::FreezeAllTrafficLights(bool frozen) { // 冻结或解冻所有交通信号灯
    _episode.Lock()->FreezeAllTrafficLights(frozen); // 调用冻结方法
  }

  std::vector<geom::BoundingBox> World::GetLevelBBs(uint8_t queried_tag) const { // 获取级别边界框
    return _episode.Lock()->GetLevelBBs(queried_tag); // 返回边界框列表
  }

  std::vector<rpc::EnvironmentObject> World::GetEnvironmentObjects(uint8_t queried_tag) const { // 获取环境对象
    return _episode.Lock()->GetEnvironmentObjects(queried_tag); // 返回环境对象列表
  }


 void World::EnableEnvironmentObjects(
      std::vector<uint64_t> env_objects_ids, // 环境对象的 ID 列表
      bool enable) const { // 是否启用环境对象
    _episode.Lock()->EnableEnvironmentObjects(env_objects_ids, enable); // 锁定并启用或禁用环境对象
  }

boost::optional<rpc::LabelledPoint> World::ProjectPoint(
      geom::Location location, // 要投影的点的位置
      geom::Vector3D direction, // 投影方向
      float search_distance) const { // 搜索距离
    auto result = _episode.Lock()->ProjectPoint(location, direction, search_distance); // 锁定并进行点投影
    if (result.first) { // 如果投影成功
      return result.second; // 返回投影结果
    }
    return {}; // 否则返回空
  }

boost::optional<rpc::LabelledPoint> World::GroundProjection(
      geom::Location location, // 要投影的点的位置
      float search_distance) const { // 搜索距离
    const geom::Vector3D DownVector(0,0,-1); // 定义向下的向量
    return ProjectPoint(location, DownVector, search_distance); // 调用 ProjectPoint 进行地面投影
  }

std::vector<rpc::LabelledPoint> World::CastRay(
      geom::Location start_location, // 射线起始位置
      geom::Location end_location) const { // 射线结束位置
    return _episode.Lock()->CastRay(start_location, end_location); // 锁定并进行射线投射
  }

std::vector<SharedPtr<Actor>> World::GetTrafficLightsFromWaypoint(
      const Waypoint& waypoint, // 轨迹点
      double distance) const { // 距离
    std::vector<SharedPtr<Actor>> Result; // 保存交通信号灯的结果
    std::vector<SharedPtr<Landmark>> landmarks =
        waypoint.GetAllLandmarksInDistance(distance); // 获取指定距离内的所有地标
    std::set<std::string> added_signals; // 用于记录已添加的信号
    for (auto& landmark : landmarks) { // 遍历所有地标
      if (road::SignalType::IsTrafficLight(landmark->GetType())) { // 判断是否为交通信号灯
        SharedPtr<Actor> traffic_light = GetTrafficLight(*(landmark.get())); // 获取交通信号灯
        if (traffic_light) { // 如果找到交通信号灯
          if(added_signals.count(landmark->GetId()) == 0) { // 检查是否未添加
            Result.emplace_back(traffic_light); // 添加到结果中
            added_signals.insert(landmark->GetId()); // 标记为已添加
          }
        }
      }
    }
    return Result; // 返回找到的交通信号灯
  }

std::vector<SharedPtr<Actor>> World::GetTrafficLightsInJunction(
      const road::JuncId junc_id) const { // 获取交叉口的交通信号灯
    std::vector<SharedPtr<Actor>> Result; // 保存结果的向量
    SharedPtr<Map> map = GetMap(); // 获取地图
    const road::Junction* junction = map->GetMap().GetJunction(junc_id); // 获取交叉口
    for (const road::ContId& cont_id : junction->GetControllers()) { // 遍历控制器
      const std::unique_ptr<road::Controller>& controller =
          map->GetMap().GetControllers().at(cont_id); // 获取控制器
      for (road::SignId sign_id : controller->GetSignals()) { // 遍历控制器的信号
        SharedPtr<Actor> traffic_light = GetTrafficLightFromOpenDRIVE(sign_id); // 从 OpenDRIVE 获取交通信号灯
        if (traffic_light) { // 如果找到交通信号灯
          Result.emplace_back(traffic_light); // 添加到结果中
        }
      }
    }
    return Result; // 返回找到的交通信号灯
  }

void World::ApplyColorTextureToObject(
      const std::string &object_name, // 对象名称
      const rpc::MaterialParameter& parameter, // 材质参数
      const rpc::TextureColor& Texture) { // 纹理颜色
    _episode.Lock()->ApplyColorTextureToObjects({object_name}, parameter, Texture); // 锁定并应用颜色纹理到对象
  }

void World::ApplyColorTextureToObjects(
      const std::vector<std::string> &objects_name, // 对象名称列表
      const rpc::MaterialParameter& parameter, // 材质参数
      const rpc::TextureColor& Texture) { // 纹理颜色
    _episode.Lock()->ApplyColorTextureToObjects(objects_name, parameter, Texture); // 锁定并应用颜色纹理到对象列表
  }

void World::ApplyFloatColorTextureToObject(
      const std::string &object_name, // 对象名称
      const rpc::MaterialParameter& parameter, // 材质参数
      const rpc::TextureFloatColor& Texture) { // 浮点纹理颜色
    _episode.Lock()->ApplyColorTextureToObjects({object_name}, parameter, Texture); // 锁定并应用浮点颜色纹理到对象
  }

void World::ApplyFloatColorTextureToObjects(
      const std::vector<std::string> &objects_name, // 对象名称列表
      const rpc::MaterialParameter& parameter, // 材质参数
      const rpc::TextureFloatColor& Texture) { // 浮点纹理颜色
    _episode.Lock()->ApplyColorTextureToObjects(objects_name, parameter, Texture); // 锁定并应用浮点颜色纹理到对象列表
  }

std::vector<std::string> World::GetNamesOfAllObjects() const { // 获取所有对象的名称
    return _episode.Lock()->GetNamesOfAllObjects(); // 锁定并返回所有对象的名称
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
