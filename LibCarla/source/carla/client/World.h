// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
  //功能：这是文件的版权声明，表示代码版权归属于巴塞罗那大学计算机视觉中心（CVC）。该代码采用MIT开源许可证，用户可以自由使用、修改和分发代码，但需要附带相同的许可证。

#pragma once
  //功能：确保该头文件只会被编译一次，避免重复包含。

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
  //引入了一些必要的头文件，包括内存管理、时间控制、调试工具、地图层信息、车辆和环境对象的RPC接口等。这些模块共同支持CARLA模拟环境的创建和控制。
  // 这些模块共同支持CARLA模拟环境的创建和控制。

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
      // EpisodeProxy对象可能包含了与模拟世界某一“剧集”（类似一次模拟场景实例）相关的关键信息，
      // 通过移动语义，高效地将其所有权转移到World类对象内部进行管理。
    explicit World(detail::EpisodeProxy episode) : _episode(std::move(episode)) {}
      // 析构函数，负责清理World对象持有的资源。在这个例子中，没有特别的清理代码，但它是虚函数或继承时的重要部分。 
      // 但它被定义为虚函数，在类存在继承关系时，便于子类进行适当的资源释放操作，是很重要的一部分。
    ~World(){}
      // 拷贝构造函数和拷贝赋值运算符，都使用默认实现。这表明World对象可以安全地被拷贝，但注意，这可能不总是你想要的行为，特别是如果_episode包含了不可拷贝的资源
      // 但要注意，如果_episode包含了不可拷贝的资源（比如指向外部特定资源的指针且不支持拷贝语义），
      // 这种默认行为可能会导致问题，使用时需谨慎考虑是否符合实际需求。
    World(const World &) = default;
    // 移动构造函数和移动赋值运算符，也使用默认实现。这允许高效地转移World对象的所有权，而不是拷贝它。
    // 例如在将一个World对象赋值给另一个对象或者将其作为函数返回值传递时，通过移动而不是拷贝操作，
    // 避免不必要的资源复制开销，提高性能。
    World(World &&) = default;

    World &operator=(const World &) = default;
    World &operator=(World &&) = default;

    /// 得到与这个世界相联系的id集。
    /// 这个ID可能是用于唯一标识该模拟世界实例的编号，在整个CARLA系统中，用于区分不同的模拟场景等用途。
    uint64_t GetId() const {
      return _episode.GetId();
    }

    /// 返回描述这个世界的地图。
    /// 返回的是一个智能指针指向的Map对象，该Map对象包含了模拟世界中的地理信息、道路布局等地图相关的数据结构，
    /// 供外部调用者进一步查询和操作地图相关的功能。
    SharedPtr<Map> GetMap() const;

    /// 加载指定的地图层级（由rpc::MapLayer类型参数指定）到模拟世界中。
    /// 可以根据需要选择性地加载地图的不同部分或者功能层，比如只加载交通道路层或者建筑物层等，具体取决于MapLayer的定义。
    void LoadLevelLayer(rpc::MapLayer map_layers) const;
    /// 卸载指定的地图层级（由rpc::MapLayer类型参数指定）。
    /// 与LoadLevelLayer相对应，用于移除已经加载的地图部分，释放相关资源或者改变模拟世界的显示内容等。

    void UnloadLevelLayer(rpc::MapLayer map_layers) const;

    /// 返回当前世界中可用的蓝图列表。
    /// 这个蓝图可以用来在世界中生成参与者(actor)。
    /// 通过这些蓝图可以在世界中生成相应的实体对象，调用者可以基于返回的蓝图库来选择合适的蓝图创建所需的参与者。
    SharedPtr<BlueprintLibrary> GetBlueprintLibrary() const;

    /// 返回一个元素对列表,
    /// 其中第一个元素是车辆ID,第二个元素是灯光状态.
    /// 方便外部查询当前模拟世界中所有车辆的灯光状态信息，例如哪些车辆的大灯开着、转向灯状态等，用于模拟交通场景中的灯光效果展示和相关逻辑判断。
    rpc::VehicleLightStateList GetVehiclesLightStates() const;

    /// 从行人导航网格获得一个随机位置
    /// 在模拟行人行为等场景时，可以利用这个函数获取一个在行人导航范围内的随机地点，
    /// 比如用于生成行人的初始位置或者随机行走的目标位置等。
    boost::optional<geom::Location> GetRandomLocationFromNavigation() const;

    /// 返回为旁观者的参与者.
    /// 旁观者控制模拟器窗口中的视图.
    /// 方便外部查询当前模拟世界中所有车辆的灯光状态信息，例如哪些车辆的大灯开着、转向灯状态等，用于模拟交通场景中的灯光效果展示和相关逻辑判断。
    SharedPtr<Actor> GetSpectator() const;

    /// 获取当前模拟世界的设置信息，比如时间步长、同步模式等相关的参数配置情况，
    /// 返回的rpc::EpisodeSettings对象包含了这些详细的设置内容，供外部查询和可能的修改参考。
    rpc::EpisodeSettings GetSettings() const;

    /// @return 应用设置时的帧id。
    /// 当应用新的模拟世界设置（通过传入的rpc::EpisodeSettings参数指定）时，返回该操作对应的帧编号，
    /// 帧编号可以用于在时间序列上定位和跟踪这个设置操作发生的时刻，在一些需要精确控制模拟进度和记录操作顺序的场景中很有用。
    uint64_t ApplySettings(const rpc::EpisodeSettings &settings, time_duration timeout);

    /// 检索当前世界上活动的天气参数。
    /// 可以获取到当前模拟世界中的天气状况相关的参数，例如晴天、雨天、雾天等天气类型对应的具体参数设置，
    /// 像光照强度、雾气浓度等，用于呈现不同天气下的模拟场景效果。
    rpc::WeatherParameters GetWeather() const;

    /// 在模拟场景中改变天气。
    /// 通过传入指定的rpc::WeatherParameters参数，来设置模拟世界的天气情况，实现模拟不同天气环境下的交通、行人等行为场景。
    void SetWeather(const rpc::WeatherParameters &weather);

    /// 获取用于IMUI传感器加速度计计算的重力值。
    /// 在涉及到模拟车辆、机器人等带有IMUI传感器的实体时，需要获取这个重力值用于传感器相关物理量的准确计算，
    /// 以符合真实物理世界的规律或者模拟特定的物理环境。
    float GetIMUISensorGravity() const;

    /// 设置用于IMUISensor加速度计计算的重力值。
    /// 允许外部根据实际模拟需求，调整IMUI传感器加速度计计算所使用的重力值，改变模拟物理环境的相关参数。
    void SetIMUISensorGravity(float NewIMUISensorGravity);

    /// 返回当前世界的快照。
    /// 快照（Snapshot）包含了模拟世界在某一时刻的整体状态信息，例如所有参与者的位置、状态，天气情况等，
    /// 可以用于记录、对比不同时刻的世界状态或者进行一些基于特定时刻状态的分析和操作。
    WorldSnapshot GetSnapshot() const;

    /// 根据id查找actor，如果没有找到则返回nullptr。
    /// 通过传入的ActorId参数，在当前模拟世界中查找对应的参与者对象，方便快速定位特定的实体，
    /// 比如查找某一辆特定编号的车辆或者某个行人等。
    SharedPtr<Actor> GetActor(ActorId id) const;

    /// 返回一个包含当前世界上所有存在的参与者（actor）的列表。
    /// 获取整个模拟世界中所有参与者的集合，便于进行批量操作、统计或者遍历所有实体执行某些通用的操作等。
    SharedPtr<ActorList> GetActors() const;

    /// 返回一个包含ActorId请求的参与者（actor）的列表。
    /// 根据传入的包含多个ActorId的vector参数，返回对应的参与者列表，实现按照指定的一组ID来查找和获取相应的参与者对象集合。
    SharedPtr<ActorList> GetActors(const std::vector<ActorId> &actor_ids) const;

    /// 根据 @a 转换中提供的 @a 蓝图，在世界中生成一个参与者（actor）。
    /// 如果提供了 @a 父类，则参与者（actor）被附加到 @a 父类。
    /// 基于给定的ActorBlueprint（参与者创建蓝图）以及指定的几何变换（geom::Transform，用于确定在世界中的位置、姿态等信息），
    /// 在模拟世界中创建一个新的参与者对象，并且可以选择将其附加到一个已有的父类参与者上，建立父子关系，
    /// 这种父子关系可能在物理模拟、行为关联等方面有相应的作用，例如车辆挂载拖车等场景。
    SharedPtr<Actor> SpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr,
        rpc::AttachmentType attachment_type = rpc::AttachmentType::Rigid,
        const std::string& socket_name = "");

    /// 和SpawnActor一样，但失败时返回nullptr而不抛出异常。
    /// 与SpawnActor功能类似，不过在创建参与者失败的情况下，不会抛出异常打断程序流程，而是安静地返回nullptr，
    /// 这样在一些需要稳健处理创建操作失败情况的场景中，调用者可以自行根据返回值进行相应的错误处理逻辑。
    SharedPtr<Actor> TrySpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr,
        rpc::AttachmentType attachment_type = rpc::AttachmentType::Rigid,
        const std::string& socket_name = "") noexcept;

    /// 阻塞调用线程，直到接收到一个世界刻。
    /// 世界刻（Tick）可以理解为模拟世界的时间推进的一个基本单位，通过阻塞等待一个世界刻，
    /// 可以确保在获取后续操作所需的最新世界状态时，是基于已经更新到下一个时间步的情况，常用于同步模拟流程与世界状态更新。
    WorldSnapshot WaitForTick(time_duration timeout) const;

    /// 注册一个 @a 回调函数，在每次接收到世界刻时调用。
    /// 可以注册一个自定义的函数，在模拟世界每推进一个时间步（即接收到世界刻）时被自动调用，
    /// 便于在每个时间步执行一些自定义的逻辑，比如更新统计信息、检查特定条件等，返回的回调函数ID用于后续删除该回调函数。
    ///
    /// @return 回调函数的ID，用它来删除回调函数。
    size_t OnTick(std::function<void(WorldSnapshot)> callback);

    /// Remove a callback registered with OnTick.
    /// 根据之前注册回调函数时返回的ID，来移除对应的在每个世界刻调用的回调函数，实现对回调机制的动态管理。
    void RemoveOnTick(size_t callback_id);

    /// 通知模拟器继续进行下一个节拍（仅对同步模式有效）。
    /// 在同步模拟模式下，通过调用这个函数来告知模拟器可以推进到下一个时间步了，
    /// 并且返回这个调用开始的帧的id，用于跟踪和记录模拟进度在时间轴上的位置。
    ///
    /// @return 这个调用开始的帧的id.
    uint64_t Tick(time_duration timeout);

    /// 设置一个代理表示在它的路径中穿过道路的概率。
    /// 0.0f表示行人不得过马路
    /// 0.5f表示50%的行人可以过马路
    /// 1.0f表示所有行人在需要时都可以过马路
    /// 用于控制模拟世界中行人过马路行为的概率，通过调整这个概率值，可以模拟出不同行人遵守交通规则或者交通繁忙程度不同的场景情况。
    void SetPedestriansCrossFactor(float percentage);

    /// 在行人模块中将 seed 设置为使用随机数。
    /// 设置行人相关随机行为的种子值，相同的种子值可以使得行人模块在每次模拟时产生相同的随机行为序列，便于复现和对比不同模拟情况，
    /// 或者设置不同的种子值来获取真正的随机行为表现。
   void SetPedestriansSeed(unsigned int seed);

    /// 根据提供的地标（Landmark）获取对应的交通标志（TrafficSign）的智能指针。
    /// 地标通常代表地图中的特定位置，通过它可以定位和获取对应的交通标志对象，用于查询交通规则相关的指示信息等。
    SharedPtr<Actor> GetTrafficSign(const Landmark& landmark) const;

    /// 根据提供的地标（Landmark）获取对应的交通信号灯（TrafficLight）的智能指针。
    /// 与获取交通标志类似，通过地标来查找对应的交通信号灯对象，以便获取信号灯状态、控制信号灯等相关操作，模拟交通灯变化对交通流的影响。
    SharedPtr<Actor> GetTrafficLight(const Landmark& landmark) const;

    /// 根据OpenDRIVE标志ID获取对应的交通灯的智能指针。
    /// OpenDRIVE是一种用于描述道路网络等信息的标准格式，利用其中的标志ID可以准确地在模拟世界中找到对应的交通灯对象，
    // 便于与基于OpenDRIVE标准的其他系统或数据进行交互和集成。
    SharedPtr<Actor> GetTrafficLightFromOpenDRIVE(const road::SignId& sign_id) const;
    ///交通灯和标志，根据提供的 获取交通标志或交通信号灯。代表地图中的特定位置。

    SharedPtr<Actor> GetTrafficLightFromOpenDRIVE(const road::SignId& sign_id) const;
    /// 根据OpenDRIVE标志ID获取对应的交通灯的智能指针。
    /// 重置所有交通信号灯的状态，可能将其恢复到初始默认状态或者按照一定规则重新初始化状态，
    /// 用于在模拟过程中重新调整交通灯的整体情况，比如模拟交通拥堵后重新规划交通流时使用。
    void ResetAllTrafficLights();

    /// 获取交通灯管理器（LightManager）的智能指针。
    /// 交通灯管理器可以用于统一管理和控制模拟世界中所有交通灯的各种操作，比如批量设置状态、定时切换等，提供更便捷的交通灯管理功能。
    SharedPtr<LightManager> GetLightManager() const;

    /// 创建并返回一个DebugHelper对象，该对象基于当前的EpisodeProxy创建，
    /// DebugHelper通常用于在模拟世界中进行调试相关的操作，比如可视化一些内部数据结构、辅助排查问题等。
    DebugHelper MakeDebugHelper() const {
      return DebugHelper{_episode};
    }

    /// 返回当前World对象关联的detail::EpisodeProxy对象，
    /// 供外部在需要直接操作EpisodeProxy所包含的相关剧集信息或者传递给其他函数进行进一步处理时使用。
    detail::EpisodeProxy GetEpisode() const {
      return _episode;
    };
    /// 冻结或解冻所有交通信号灯，通过传入的布尔值参数来控制，
    /// 冻结时交通信号灯状态将保持不变，常用于暂停交通灯变化来观察特定交通场景或者进行调试等情况。
    void FreezeAllTrafficLights(bool frozen);

    /// 返回该等级中所有元素的BBs。
    /// 边界框可以描述物体在三维空间中的大致范围，通过获取所有元素的边界框，
    /// 可以用于碰撞检测、空间划分、可视化展示等多种与空间位置和范围相关的操作，这里返回的是指定标签（queried_tag）对应的元素的边界框列表。
    std::vector<geom::BoundingBox> GetLevelBBs(uint8_t queried_tag) const;

    /// 获取指定标签（queried_tag）的环境物体（EnvironmentObject）列表。
    /// 环境物体可以包括路边的障碍物、建筑物等各种非参与者但影响模拟场景的物体，通过获取它们可以进行相关的查询、操作或者场景渲染等处理。
    std::vector<rpc::EnvironmentObject> GetEnvironmentObjects(uint8_t queried_tag) const;

    /// 启用或禁用环境物体，根据传入的环境物体ID列表（env_objects_ids）以及布尔值参数enable来操作，
    /// 可以动态地控制某些环境物体在模拟世界中是否显示或者参与物理模拟等，实现灵活的场景配置。
    void EnableEnvironmentObjects(
      std::vector<uint64_t> env_objects_ids,
      bool enable) const;
    ///GetEnvironmentObjects获取指定标签的环境物体。
    ///EnableEnvironmentObjects启用或禁用环境物体。

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
//这个类是CARLA模拟世界的核心，提供了与模拟世界交互的多种方法，包括获取世界信息、生成参与者（演员）、控制天气、模拟时间步等。
// 通过 类，用户可以对CARLA模拟环境进行全面的控制和管理。
