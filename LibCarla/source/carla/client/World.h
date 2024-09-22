// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/Time.h"
#include "carla/client/DebugHelper.h"
#include "carla/client/Landmark.h"
#include "carla/client/Waypoint.h"
#include "carla/client/Junction.h"
#include "carla/client/LightManager.h"
#include "carla/client/Timestamp.h"
#include "carla/client/WorldSnapshot.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/geom/Transform.h"
#include "carla/rpc/Actor.h"
#include "carla/rpc/AttachmentType.h"
#include "carla/rpc/EpisodeSettings.h"
#include "carla/rpc/EnvironmentObject.h"
#include "carla/rpc/LabelledPoint.h"
#include "carla/rpc/MapLayer.h"
#include "carla/rpc/VehiclePhysicsControl.h"
#include "carla/rpc/WeatherParameters.h"
#include "carla/rpc/VehicleLightStateList.h"
#include "carla/rpc/Texture.h"
#include "carla/rpc/MaterialParameter.h"

#include <string>
#include <boost/optional.hpp>

namespace carla {
namespace client {

  class Actor;
  class ActorBlueprint;
  class ActorList;
  class BlueprintLibrary;
  class Map;
  class TrafficLight;
  class TrafficSign;

  class World {
  public:

    explicit World(detail::EpisodeProxy episode) : _episode(std::move(episode)) {}

    ~World(){}

    World(const World &) = default;
    World(World &&) = default;

    World &operator=(const World &) = default;
    World &operator=(World &&) = default;

    /// 得到与这个世界相联系的id集.
    uint64_t GetId() const {
      return _episode.GetId();
    }

    /// 返回描述这个世界的地图.
    SharedPtr<Map> GetMap() const;

    void LoadLevelLayer(rpc::MapLayer map_layers) const;

    void UnloadLevelLayer(rpc::MapLayer map_layers) const;

    /// 返回当前世界中可用的蓝图列表. 
    /// 这个蓝图可以用来在世界中生成参与者(actor).
    SharedPtr<BlueprintLibrary> GetBlueprintLibrary() const;

    /// 返回一个元素对列表,
    /// 其中第一个元素是车辆ID,第二个元素是灯光状态.
    rpc::VehicleLightStateList GetVehiclesLightStates() const;

    /// 从行人导航网格获得一个随机位置
    boost::optional<geom::Location> GetRandomLocationFromNavigation() const;

    /// 返回为旁观者的参与者.
    /// 旁观者控制模拟器窗口中的视图.
    SharedPtr<Actor> GetSpectator() const;

    rpc::EpisodeSettings GetSettings() const;

    /// @return 应用设置时的帧id.
    uint64_t ApplySettings(const rpc::EpisodeSettings &settings, time_duration timeout);

    /// 检索当前世界上活动的天气参数.
    rpc::WeatherParameters GetWeather() const;

    /// 在模拟场景中改变天气.
    void SetWeather(const rpc::WeatherParameters &weather);

    /// 获取用于IMUI传感器加速度计计算的重力值.
    float GetIMUISensorGravity() const;
    
    /// 设置用于IMUI传感器加速度计计算的重力值.
    void SetIMUISensorGravity(float NewIMUISensorGravity);

    /// 返回当前世界的快照.
    WorldSnapshot GetSnapshot() const;

    /// 根据id查找actor，如果没有找到则返回nullptr.
    SharedPtr<Actor> GetActor(ActorId id) const;

    /// 返回一个包含当前世界上所有存在的参与者(actor)的列表.
    SharedPtr<ActorList> GetActors() const;

    /// 返回一个包含ActorId请求的参与者(actor)的列表.
    SharedPtr<ActorList> GetActors(const std::vector<ActorId> &actor_ids) const;

    /// 根据 @a 转换中提供的 @a 蓝图，在世界中生成一个参与者(actor).
    /// 如果提供了 @a 父类，则参与者(actor)被附加到 @a 父类.
    SharedPtr<Actor> SpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr,
        rpc::AttachmentType attachment_type = rpc::AttachmentType::Rigid,
        const std::string& socket_name = "");

    /// 和SpawnActor一样，但失败时返回nullptr而不抛出异常.
    SharedPtr<Actor> TrySpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr,
        rpc::AttachmentType attachment_type = rpc::AttachmentType::Rigid,
        const std::string& socket_name = "") noexcept;

    /// 阻塞调用线程，直到接收到一个世界刻.
    WorldSnapshot WaitForTick(time_duration timeout) const;

    /// 注册一个 @a 回调函数，在每次接收到世界刻时调用.
    ///
    /// @return 回调函数的ID，用它来删除回调函数.
    size_t OnTick(std::function<void(WorldSnapshot)> callback);

    /// Remove a callback registered with OnTick.
    void RemoveOnTick(size_t callback_id);

    /// 通知模拟器继续进行下一个节拍(仅对同步模式有效).
    ///
    /// @return 这个调用开始的帧的id.
    uint64_t Tick(time_duration timeout);

    /// 设置一个代理表示在它的路径中穿过道路的概率.
    /// 0.0f表示行人不得过马路
    /// 0.5f表示50%的行人可以过马路
    /// 1.0f表示所有行人在需要时都可以过马路
    void SetPedestriansCrossFactor(float percentage);

    /// 在行人模块中将 seed 设置为使用随机数
    void SetPedestriansSeed(unsigned int seed);

    SharedPtr<Actor> GetTrafficSign(const Landmark& landmark) const;

    SharedPtr<Actor> GetTrafficLight(const Landmark& landmark) const;

    SharedPtr<Actor> GetTrafficLightFromOpenDRIVE(const road::SignId& sign_id) const;

    void ResetAllTrafficLights();

    SharedPtr<LightManager> GetLightManager() const;

    DebugHelper MakeDebugHelper() const {
      return DebugHelper{_episode};
    }

    detail::EpisodeProxy GetEpisode() const {
      return _episode;
    };

    void FreezeAllTrafficLights(bool frozen);

    /// 返回该等级中所有元素的BBs.
    std::vector<geom::BoundingBox> GetLevelBBs(uint8_t queried_tag) const;

    std::vector<rpc::EnvironmentObject> GetEnvironmentObjects(uint8_t queried_tag) const;

    void EnableEnvironmentObjects(
      std::vector<uint64_t> env_objects_ids,
      bool enable) const;

    boost::optional<rpc::LabelledPoint> ProjectPoint(
        geom::Location location, geom::Vector3D direction, float search_distance = 10000.f) const;

    boost::optional<rpc::LabelledPoint> GroundProjection(
        geom::Location location, float search_distance = 10000.0) const;

    std::vector<rpc::LabelledPoint> CastRay(
        geom::Location start_location, geom::Location end_location) const;

    std::vector<SharedPtr<Actor>> GetTrafficLightsFromWaypoint(
        const Waypoint& waypoint, double distance) const;

    std::vector<SharedPtr<Actor>> GetTrafficLightsInJunction(
        const road::JuncId junc_id) const;

    // std::vector<std::string> GetObjectNameList();

    void ApplyColorTextureToObject(
        const std::string &actor_name,
        const rpc::MaterialParameter& parameter,
        const rpc::TextureColor& Texture);

    void ApplyColorTextureToObjects(
        const std::vector<std::string> &objects_names,
        const rpc::MaterialParameter& parameter,
        const rpc::TextureColor& Texture);

    void ApplyFloatColorTextureToObject(
        const std::string &actor_name,
        const rpc::MaterialParameter& parameter,
        const rpc::TextureFloatColor& Texture);

    void ApplyFloatColorTextureToObjects(
        const std::vector<std::string> &objects_names,
        const rpc::MaterialParameter& parameter,
        const rpc::TextureFloatColor& Texture);

    void ApplyTexturesToObject(
        const std::string &actor_name,
        const rpc::TextureColor& diffuse_texture,
        const rpc::TextureFloatColor& emissive_texture,
        const rpc::TextureFloatColor& normal_texture,
        const rpc::TextureFloatColor& ao_roughness_metallic_emissive_texture);

    void ApplyTexturesToObjects(
        const std::vector<std::string> &objects_names,
        const rpc::TextureColor& diffuse_texture,
        const rpc::TextureFloatColor& emissive_texture,
        const rpc::TextureFloatColor& normal_texture,
        const rpc::TextureFloatColor& ao_roughness_metallic_emissive_texture);

    std::vector<std::string> GetNamesOfAllObjects() const;

  private:

    detail::EpisodeProxy _episode;
  };

} // namespace client
} // namespace carla
