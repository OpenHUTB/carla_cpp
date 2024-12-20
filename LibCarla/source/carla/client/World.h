// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
  //功能：这是文件的版权声明，表示代码版权归属于巴塞罗那大学计算机视觉中心（CVC）。该代码采用MIT开源许可证，用户可以自由使用、修改和分发代码，但需要附带相同的许可证。

#pragma once
  //功能：确保该头文件只会被编译一次，避免重复包含。

#include "carla/Memory.h"  // 包含内存相关的头文件
#include "carla/Time.h"  // 包含时间相关的头文件
#include "carla/client/DebugHelper.h"  // 包含调试辅助工具相关的头文件
#include "carla/client/Landmark.h"  // 包含地标相关的头文件
#include "carla/client/Waypoint.h"  // 包含路径点相关的头文件
#include "carla/client/Junction.h"  // 包含交叉口相关的头文件
#include "carla/client/LightManager.h"  // 包含灯光管理器相关的头文件
#include "carla/client/Timestamp.h"  // 包含时间戳相关的头文件
#include "carla/client/WorldSnapshot.h"  // 包含世界快照相关的头文件
#include "carla/client/detail/EpisodeProxy.h"  // 包含EpisodeProxy相关的头文件
#include "carla/geom/Transform.h"  // 包含变换矩阵相关的头文件
#include "carla/rpc/Actor.h"  // 包含演员（对象）相关的头文件
#include "carla/rpc/AttachmentType.h"  // 包含附加物类型相关的头文件
#include "carla/rpc/EpisodeSettings.h"  // 包含剧集设置相关的头文件
#include "carla/rpc/EnvironmentObject.h"  // 包含环境对象相关的头文件
#include "carla/rpc/LabelledPoint.h"  // 包含带标签点的头文件
#include "carla/rpc/MapLayer.h"  // 包含地图图层相关的头文件
#include "carla/rpc/VehiclePhysicsControl.h"  // 包含车辆物理控制相关的头文件
#include "carla/rpc/WeatherParameters.h"  // 包含天气参数相关的头文件
#include "carla/rpc/VehicleLightStateList.h"  // 包含车辆灯光状态列表相关的头文件
#include "carla/rpc/Texture.h"  // 包含纹理相关的头文件
#include "carla/rpc/MaterialParameter.h"  // 包含材质参数相关的头文件

#include <string>  // 包含字符串处理相关的头文件
#include <boost/optional.hpp>
  //引入了一些必要的头文件，包括内存管理、时间控制、调试工具、地图层信息、车辆和环境对象的RPC接口等。这些模块共同支持CARLA模拟环境的创建和控制。

namespace carla {
namespace client {

  class Actor;
  class ActorBlueprint;
  class ActorList;
  class BlueprintLibrary;
  class Map;
  class TrafficLight;
  class TrafficSign;
  // World类定义开始，代表整个CARLA模拟世界
  class World {
  public:
      // 构造函数，接受一个detail::EpisodeProxy对象作为参数，并将其移动到_episode成员变量中。  
    explicit World(detail::EpisodeProxy episode) : _episode(std::move(episode)) {}
      // 析构函数，负责清理World对象持有的资源。在这个例子中，没有特别的清理代码，但它是虚函数或继承时的重要部分。 
    ~World(){}
      // 拷贝构造函数和拷贝赋值运算符，都使用默认实现。这表明World对象可以安全地被拷贝，但注意，这可能不总是你想要的行为，特别是如果_episode包含了不可拷贝的资源
    World(const World &) = default;
    // 移动构造函数和移动赋值运算符，也使用默认实现。这允许高效地转移World对象的所有权，而不是拷贝它。  
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
    ///交通灯和标志，根据提供的 获取交通标志或交通信号灯。代表地图中的特定位置。

    SharedPtr<Actor> GetTrafficLightFromOpenDRIVE(const road::SignId& sign_id) const;
    ///根据OpenDRIVE标志ID获取对应的交通灯的智能指针

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
    ///GetEnvironmentObjects获取指定标签的环境物体。
    ///EnableEnvironmentObjects启用或禁用环境物体。
    // 这个函数接受一个3D空间中的点（location）和方向（direction），然后沿着这个方向在指定的搜索距离（search_distance）内投影这个点到2D平面上
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
    // 将颜色纹理应用到指定的对象上
    void ApplyColorTextureToObject(
        const std::string &actor_name, // 要应用纹理的对象名称
        const rpc::MaterialParameter& parameter, // 材质参数，可能包含影响纹理显示的其他设置
        const rpc::TextureColor& Texture); // 要应用的颜色纹理

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
        const rpc::TextureFloatColor& Texture); // 要应用的浮点颜色纹理

    void ApplyTexturesToObject(
        const std::string &actor_name,
        const rpc::TextureColor& diffuse_texture, 
        const rpc::TextureFloatColor& emissive_texture, 
        const rpc::TextureFloatColor& normal_texture, 
        const rpc::TextureFloatColor& ao_roughness_metallic_emissive_texture);

    void ApplyTexturesToObjects(
        const std::vector<std::string> &objects_names,
        const rpc::TextureColor& diffuse_texture, // 漫反射纹理
        const rpc::TextureFloatColor& emissive_texture, // 自发光纹理
        const rpc::TextureFloatColor& normal_texture, //法线纹理
        const rpc::TextureFloatColor& ao_roughness_metallic_emissive_texture); //AO粗糙度金属度自发光纹理

    std::vector<std::string> GetNamesOfAllObjects() const;

  private:

    detail::EpisodeProxy _episode;
  };

} // namespace client
} // namespace carla
//这个类是CARLA模拟世界的核心，提供了与模拟世界交互的多种方法，包括获取世界信息、生成参与者（演员）、控制天气、模拟时间步等。
// 通过 类，用户可以对CARLA模拟环境进行全面的控制和管理。
