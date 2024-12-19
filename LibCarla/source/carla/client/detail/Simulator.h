// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Logging.h"
#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/client/Actor.h"
#include "carla/client/GarbageCollectionPolicy.h"
#include "carla/client/TrafficLight.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Walker.h"
#include "carla/client/World.h"
#include "carla/client/WorldSnapshot.h"
#include "carla/client/detail/ActorFactory.h"
#include "carla/client/detail/Client.h"
#include "carla/client/detail/Episode.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/profiler/LifetimeProfiled.h"
#include "carla/rpc/TrafficLightState.h"
#include "carla/rpc/VehicleLightStateList.h"
#include "carla/rpc/LabelledPoint.h"
#include "carla/rpc/VehicleWheels.h"
#include "carla/rpc/Texture.h"
#include "carla/rpc/MaterialParameter.h"

#include <boost/optional.hpp>

#include <memory>

namespace carla {
namespace client {

  class ActorBlueprint;
  class BlueprintLibrary;
  class Map;
  class Sensor;
  class WalkerAIController;
  class WalkerNavigation;

namespace detail {

  /// 连接并控制 CARLA 模拟器。
  class Simulator
    : public std::enable_shared_from_this<Simulator>,
      private profiler::LifetimeProfiled,
      private NonCopyable {
  public:

    // =========================================================================
    /// @name 构造函数
    // =========================================================================
    /// @{

    explicit Simulator(
        const std::string &host,      // 主服务器的IP地址
        uint16_t port,                // 连接主服务器的端口号，默认为 2000
        size_t worker_threads = 0u,   // 仿真器使用的工作线程数，默认全部启用
        bool enable_garbage_collection = false);    // 是否启用垃圾回收，默认不启用

    /// @}
    // =========================================================================
    /// @name 加载新的场景
    // =========================================================================
    /// @{

    EpisodeProxy ReloadEpisode(bool reset_settings = true) {
      return LoadEpisode("", reset_settings);
    }

    // 根据地图名和可选的重置设置以及地图层来加载一个场景
    EpisodeProxy LoadEpisode(std::string map_name, bool reset_settings = true, rpc::MapLayer map_layers = rpc::MapLayer::All);
    // 加载指定的地图层
    void LoadLevelLayer(rpc::MapLayer map_layers) const {
      _client.LoadLevelLayer(map_layers);
    }
    // 卸载指定的地图层
    void UnloadLevelLayer(rpc::MapLayer map_layers) const {
      _client.UnloadLevelLayer(map_layers);
    }

    // 根据OpenDrive文件和生成参数加载一个场景
    EpisodeProxy LoadOpenDriveEpisode(
        std::string opendrive,
        const rpc::OpendriveGenerationParameters & params,
        bool reset_settings = true);

    /// @}
    // =========================================================================
    /// @name 访问当前场景
    // =========================================================================
    /// @{

    /// @pre Cannot be called previous to GetCurrentEpisode.
    // 获取当前场景的ID，确保当前场景已准备好
    auto GetCurrentEpisodeId() {
      GetReadyCurrentEpisode();
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetId();
    }
    // 确保当前场景已准备好
    void GetReadyCurrentEpisode();
    // 获取当前场景的代理
    EpisodeProxy GetCurrentEpisode();

    /// @}
    // =========================================================================
    /// @name 世界快照
    // =========================================================================
    /// @{

    // 获取当前世界（场景）的实例
    World GetWorld() {
      return World{GetCurrentEpisode()};
    }

    /// @}
    // =========================================================================
    /// @name 世界快照
    // =========================================================================
    /// @{


// 获取当前世界的快照
    WorldSnapshot GetWorldSnapshot() const {
      DEBUG_ASSERT(_episode != nullptr);
      return WorldSnapshot{_episode->GetState()};
    }

    /// @}
    // =========================================================================
    /// @name 地图相关的方法
    // =========================================================================
    /// @{


// 获取当前场景的地图实例
    SharedPtr<Map> GetCurrentMap();

    // 获取所有可用的地图名
    std::vector<std::string> GetAvailableMaps() {
      return _client.GetAvailableMaps();
    }

    /// @}
    // =========================================================================
    /// @name 所需文件相关的方法
    // =========================================================================
    /// @{

// 设置文件基础文件夹
    bool SetFilesBaseFolder(const std::string &path);

    // 获取指定文件夹中所需的文件列表，可以选择是否下载
    std::vector<std::string> GetRequiredFiles(const std::string &folder = "", const bool download = true) const;

    // 请求特定文件
    void RequestFile(const std::string &name) const;

    // 从缓存中获取文件，如果缓存中没有且request_otherwise为true，则尝试请求
    std::vector<uint8_t> GetCacheFile(const std::string &name, const bool request_otherwise) const;

    /// @}
    // =========================================================================
    /// @name 垃圾收集策略
    // =========================================================================
    /// @{

    GarbageCollectionPolicy GetGarbageCollectionPolicy() const {
      return _gc_policy;
    }

    /// @}
    // =========================================================================
    /// @name 纯网络操作
    // =========================================================================
    /// @{

    // 设置网络超时时间
    void SetNetworkingTimeout(time_duration timeout) {
      _client.SetTimeout(timeout); // 设置客户端的超时时间
    }

    time_duration GetNetworkingTimeout() {
      return _client.GetTimeout();// 获取客户端的超时时间
    }
    // 获取客户端版本
    std::string GetClientVersion() {
      return _client.GetClientVersion();// 从客户端获取版本信息
    }
    // 获取服务器版本
    std::string GetServerVersion() {
      return _client.GetServerVersion();// 从服务器获取版本信息
    }

    /// @}
    // =========================================================================
    /// @name 节拍
    // =========================================================================
    /// @{

// 等待一个节拍（模拟时间步），返回该时间步的世界快照
    WorldSnapshot WaitForTick(time_duration timeout);
    // 注册一个在每个节拍（模拟时间步）被调用的回调函数
    size_t RegisterOnTickEvent(std::function<void(WorldSnapshot)> callback) {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->RegisterOnTickEvent(std::move(callback));
    }
    // 移除一个节拍回调
    void RemoveOnTickEvent(size_t id) {
      DEBUG_ASSERT(_episode != nullptr);
      _episode->RemoveOnTickEvent(id);
    }
    // 执行一个节拍（模拟时间步），返回该时间步的模拟时间（通常以微秒为单位）
    uint64_t Tick(time_duration timeout);

    /// @}
    // =========================================================================
    /// @name 访问场景中的全局对象
    // =========================================================================
    /// @{

    std :: string GetEndpoint() {
    	return _client.GetEndpoint();
    }

    /// 查询交通管理器是否正在端口上运行
    bool IsTrafficManagerRunning(uint16_t port) const {
      return _client.IsTrafficManagerRunning(port);
    }

    /// 获取一个填充了在端口上运行的交通管理器的 <IP, 端口> 对。
    /// 如果没有正在运行的流量管理器，则该对将为 ("", 0)
    std::pair<std::string, uint16_t> GetTrafficManagerRunning(uint16_t port) const {
      return _client.GetTrafficManagerRunning(port);
    }

    /// 通知交通管理器正在 <IP, 端口> 上运行
    bool AddTrafficManagerRunning(std::pair<std::string, uint16_t> trafficManagerInfo) const {
      return _client.AddTrafficManagerRunning(trafficManagerInfo);
    }

    // 根据端口销毁交通管理器
    void DestroyTrafficManager(uint16_t port) const {
      _client.DestroyTrafficManager(port);
    }

    void AddPendingException(std::string e) {
        // 调用当前章节对象的AddPendingException方法，传入异常字符串e
      _episode->AddPendingException(e);
    }

    // 获取蓝图库的共享指针
    SharedPtr<BlueprintLibrary> GetBlueprintLibrary();

    /// 返回一个列表，其中第一个元素是车辆 ID，第二个元素是灯光状态
    rpc::VehicleLightStateList GetVehiclesLightStates();

    // 获取当前观察者（Spectator）对象的共享指针
    SharedPtr<Actor> GetSpectator();

    rpc::EpisodeSettings GetEpisodeSettings() {
      return _client.GetEpisodeSettings();
    }

    uint64_t SetEpisodeSettings(const rpc::EpisodeSettings &settings);

    rpc::WeatherParameters GetWeatherParameters() {
      return _client.GetWeatherParameters();
    }

    // 设置新的天气参数
    void SetWeatherParameters(const rpc::WeatherParameters &weather) {
        // 调用客户端的SetWeatherParameters方法，传入新的天气参数
      _client.SetWeatherParameters(weather);
    }

    // 获取IMU传感器的重力值
    float GetIMUISensorGravity() const {
        // 调用客户端的GetIMUISensorGravity方法，返回IMU传感器的重力值
      return _client.GetIMUISensorGravity();
    }

    // 设置IMU传感器的重力值
    void SetIMUISensorGravity(float NewIMUISensorGravity) {
      _client.SetIMUISensorGravity(NewIMUISensorGravity);
    }

    // 获取指定车辆的物理控制状态
    rpc::VehiclePhysicsControl GetVehiclePhysicsControl(const Vehicle &vehicle) const {
        // 调用客户端的GetVehiclePhysicsControl方法，传入车辆的ID，返回车辆的物理控制状态
      return _client.GetVehiclePhysicsControl(vehicle.GetId());
    }
    // 获取指定车辆的灯光状态
    rpc::VehicleLightState GetVehicleLightState(const Vehicle &vehicle) const {
      return _client.GetVehicleLightState(vehicle.GetId());
    }

    /// Returns all the BBs of all the elements of the level
    std::vector<geom::BoundingBox> GetLevelBBs(uint8_t queried_tag) const {
      return _client.GetLevelBBs(queried_tag);
    }
    // 获取具有指定标签的环境对象
    std::vector<rpc::EnvironmentObject> GetEnvironmentObjects(uint8_t queried_tag) const {
      return _client.GetEnvironmentObjects(queried_tag);
    }

    // 启用或禁用具有指定ID的环境对象
    void EnableEnvironmentObjects(
      std::vector<uint64_t> env_objects_ids,
      bool enable) const {
        // 调用客户端的EnableEnvironmentObjects方法，传入环境对象的ID列表和启用状态
      _client.EnableEnvironmentObjects(env_objects_ids, enable);
    }
    // 投影一个点，并返回该点在地图上的标记点和是否成功投影
    std::pair<bool,rpc::LabelledPoint> ProjectPoint(
        geom::Location location, geom::Vector3D direction, float search_distance) const {
        // 调用客户端的ProjectPoint方法，传入起始位置、方向和搜索距离，返回是否成功投影和标记点
      return _client.ProjectPoint(location, direction, search_distance);
    }

    // 从一个起始位置到结束位置投射一条光线，并返回所有碰撞点
    std::vector<rpc::LabelledPoint> CastRay(
        geom::Location start_location, geom::Location end_location) const {
        // 调用客户端的CastRay方法，传入起始位置和结束位置，返回光线碰撞的所有标记点列表
      return _client.CastRay(start_location, end_location);
    }

    /// @}
    // =========================================================================
    /// @name 人工智能
    // =========================================================================
    /// @{
    // 获取当前导航对象
    std::shared_ptr<WalkerNavigation> GetNavigation();
    // 通过调用此函数来更新导航状态，可能是为了处理路径计算、目标点的选择等
    void NavigationTick();
    // 注册AI控制器，可能是某个虚拟角色或物体的AI控制器
    void RegisterAIController(const WalkerAIController &controller);
    // 注销AI控制器，移除控制器与特定角色或物体的关联
    void UnregisterAIController(const WalkerAIController &controller);
    // 从导航对象中获取一个随机位置，这可能用于生成随机目标或逃逸点
    boost::optional<geom::Location> GetRandomLocationFromNavigation();
    // 设置行人交叉因素的百分比，可能影响路径规划或碰撞处理
    void SetPedestriansCrossFactor(float percentage);
    // 设置行人行为的随机种子，可能影响行人生成或路径选择的随机性
    void SetPedestriansSeed(unsigned int seed);

    /// @}
    // =========================================================================
    /// @name 参与者的一般操作
    // =========================================================================
    /// @{

    //根据参与者的ID获取对应的演员信息
    boost::optional<rpc::Actor> GetActorById(ActorId id) const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetActorById(id);
    }
    /// 根据一组演员ID获取对应的演员信息
    std::vector<rpc::Actor> GetActorsById(const std::vector<ActorId> &actor_ids) const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetActorsById(actor_ids);
    }
    /// 获取当前 episode 中所有的演员信息
    std::vector<rpc::Actor> GetAllTheActorsInTheEpisode() const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetActors();
    }

    /// 根据现有参与者的描述创建一个参与者实例。请注意，这不会生成参与者。
    ///
    /// If @a gc is GarbageCollectionPolicy::Enabled, the shared pointer
    /// returned is provided with a custom deleter that calls Destroy() on the
    /// actor. This method does not support GarbageCollectionPolicy::Inherit.
    SharedPtr<Actor> MakeActor(
        rpc::Actor actor_description,
        GarbageCollectionPolicy gc = GarbageCollectionPolicy::Disabled) {
      RELEASE_ASSERT(gc != GarbageCollectionPolicy::Inherit);
      return ActorFactory::MakeActor(GetCurrentEpisode(), std::move(actor_description), gc);
    }

    /// 在模拟中生成一个参与者
    ///
    /// If @a gc is GarbageCollectionPolicy::Enabled, the shared pointer
    /// returned is provided with a custom deleter that calls Destroy() on the
    /// actor. If @gc is GarbageCollectionPolicy::Inherit, the default garbage
    /// collection policy is used.
    SharedPtr<Actor> SpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr,
        rpc::AttachmentType attachment_type = rpc::AttachmentType::Rigid,
        GarbageCollectionPolicy gc = GarbageCollectionPolicy::Inherit,
        const std::string& socket_name = "");
    /// 销毁指定的演员
    bool DestroyActor(Actor &actor);
    // 根据演员ID销毁指定的演员
    bool DestroyActor(ActorId actor_id)
    {
      return _client.DestroyActor(actor_id);
    }
    // 获取指定ID演员的快照信息
    ActorSnapshot GetActorSnapshot(ActorId actor_id) const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetState()->GetActorSnapshot(actor_id);
    }
    // 获取指定演员的快照信息
    ActorSnapshot GetActorSnapshot(const Actor &actor) const {
      return GetActorSnapshot(actor.GetId());
    }
    // 获取指定演员的状态信息
    rpc::ActorState GetActorState(const Actor &actor) const {
      return GetActorSnapshot(actor).actor_state;
    }
    // 获取指定演员的位置
    geom::Location GetActorLocation(const Actor &actor) const {
      return GetActorSnapshot(actor).transform.location;
    }
    /// 获取指定演员的变换信息（包括位置、旋转、缩放）
    geom::Transform GetActorTransform(const Actor &actor) const {
      return GetActorSnapshot(actor).transform;
    }

    /// 获取指定演员的速度（可能包括线性速度和角速度）
    geom::Vector3D GetActorVelocity(const Actor &actor) const {
      return GetActorSnapshot(actor).velocity;
    }
    /// 设置指定演员的目标速度
    void SetActorTargetVelocity(const Actor &actor, const geom::Vector3D &vector) {
      _client.SetActorTargetVelocity(actor.GetId(), vector);
    }
    /// 获取指定演员的角速度
    geom::Vector3D GetActorAngularVelocity(const Actor &actor) const {
      return GetActorSnapshot(actor).angular_velocity;
    }
    /// 设置指定演员的目标角速度
    void SetActorTargetAngularVelocity(const Actor &actor, const geom::Vector3D &vector) {
      _client.SetActorTargetAngularVelocity(actor.GetId(), vector);
    }
    /// 启用演员的恒定速度（用于模拟持续的线性运动）
    void EnableActorConstantVelocity(const Actor &actor, const geom::Vector3D &vector) {
      _client.EnableActorConstantVelocity(actor.GetId(), vector);
    }
    /// 禁用演员的恒定速度
    void DisableActorConstantVelocity(const Actor &actor) {
      _client.DisableActorConstantVelocity(actor.GetId());
    }
    /// 给指定演员施加脉冲力
    void AddActorImpulse(const Actor &actor, const geom::Vector3D &impulse) {
      _client.AddActorImpulse(actor.GetId(), impulse);
    }

    /// 给指定演员在特定位置施加脉冲力
    void AddActorImpulse(const Actor &actor, const geom::Vector3D &impulse, const geom::Vector3D &location) {
      _client.AddActorImpulse(actor.GetId(), impulse, location);
    }
    /// 给指定演员施加持续的力
    void AddActorForce(const Actor &actor, const geom::Vector3D &force) {
      _client.AddActorForce(actor.GetId(), force);
    }
    /// 给指定演员在特定位置施加持续的力
    void AddActorForce(const Actor &actor, const geom::Vector3D &force, const geom::Vector3D &location) {
      _client.AddActorForce(actor.GetId(), force, location);
    }
    /// 给指定演员施加角脉冲（影响角度运动）
    void AddActorAngularImpulse(const Actor &actor, const geom::Vector3D &vector) {
      _client.AddActorAngularImpulse(actor.GetId(), vector);
    }
    /// 给指定演员施加扭矩
    void AddActorTorque(const Actor &actor, const geom::Vector3D &torque) {
      _client.AddActorAngularImpulse(actor.GetId(), torque);
    }
    /// 获取指定演员的加速度
    geom::Vector3D GetActorAcceleration(const Actor &actor) const {
      return GetActorSnapshot(actor).acceleration;
    }
    /// 获取指定演员组件的世界坐标变换信息
    geom::Transform GetActorComponentWorldTransform(const Actor &actor, const std::string componentName) {
      return _client.GetActorComponentWorldTransform(actor.GetId(), componentName);
    }
    /// 获取指定演员组件的相对坐标变换信息
    geom::Transform GetActorComponentRelativeTransform(const Actor &actor, std::string componentName) {
      return _client.GetActorComponentRelativeTransform(actor.GetId(), componentName);
    }
    // 获取Actor所有骨骼的世界变换（位置、旋转、缩放等）
    std::vector<geom::Transform> GetActorBoneWorldTransforms(const Actor &actor) {
      return _client.GetActorBoneWorldTransforms(actor.GetId());
    }
    // 获取Actor所有骨骼的相对变换（相对于父对象的位置、旋转、缩放）
    std::vector<geom::Transform> GetActorBoneRelativeTransforms(const Actor &actor) {
      return _client.GetActorBoneRelativeTransforms(actor.GetId());
    }
    // 获取Actor的所有组件名称
    std::vector<std::string> GetActorComponentNames(const Actor &actor) {
      return _client.GetActorComponentNames(actor.GetId());
    }
    // 获取Actor的所有骨骼名称
    std::vector<std::string> GetActorBoneNames(const Actor &actor) {
      return _client.GetActorBoneNames(actor.GetId());
    }
    // 获取Actor所有插座的世界变换
    std::vector<geom::Transform> GetActorSocketWorldTransforms(const Actor &actor) {
      return _client.GetActorSocketWorldTransforms(actor.GetId());
    }
    // 获取Actor所有插座的相对变换
    std::vector<geom::Transform> GetActorSocketRelativeTransforms(const Actor &actor) {
      return _client.GetActorSocketRelativeTransforms(actor.GetId());
    }
    // 获取Actor的所有插座名称
    std::vector<std::string> GetActorSocketNames(const Actor &actor) {
      return _client.GetActorSocketNames(actor.GetId());
    }    
    ///设置 Actor 状态
    // 设置Actor的位置
    void SetActorLocation(Actor &actor, const geom::Location &location) {
      _client.SetActorLocation(actor.GetId(), location);
    }
    // 设置Actor的变换（包括位置、旋转、缩放）
    void SetActorTransform(Actor &actor, const geom::Transform &transform) {
      _client.SetActorTransform(actor.GetId(), transform);
    }
    // 设置Actor是否启用物理仿真
    void SetActorSimulatePhysics(Actor &actor, bool enabled) {
      _client.SetActorSimulatePhysics(actor.GetId(), enabled);
    }
    // 设置Actor是否启用碰撞
    void SetActorCollisions(Actor &actor, bool enabled) {
      _client.SetActorCollisions(actor.GetId(), enabled);
    }

    void SetActorCollisions(ActorId actor_id, bool enabled) {
      _client.SetActorCollisions(actor_id, enabled);
    }
    // 设置Actor是否死亡
    void SetActorDead(Actor &actor) {
      _client.SetActorDead(actor.GetId());
    }

    void SetActorDead(ActorId actor_id) {
      _client.SetActorDead(actor_id);
    }
    // 设置Actor是否启用重力
    void SetActorEnableGravity(Actor &actor, bool enabled) {
      _client.SetActorEnableGravity(actor.GetId(), enabled);
    }

    /// @}
    // =========================================================================
    /// @name 车辆的操作
    // =========================================================================
    /// @{

    // 设置车辆是否启用自动驾驶
    void SetVehicleAutopilot(Vehicle &vehicle, bool enabled = true) {
      _client.SetActorAutopilot(vehicle.GetId(), enabled);
    }
    // 获取车辆的遥测数据
    rpc::VehicleTelemetryData GetVehicleTelemetryData(const Vehicle &vehicle) const {
      return _client.GetVehicleTelemetryData(vehicle.GetId());
    }
    // 显示或隐藏车辆的调试遥测数据
    void ShowVehicleDebugTelemetry(Vehicle &vehicle, bool enabled = true) {
      _client.ShowVehicleDebugTelemetry(vehicle.GetId(), enabled);
    }
    // 给车辆应用控制指令（如开关灯等）
    void SetLightsToVehicle(Vehicle &vehicle, const rpc::VehicleControl &control) {
      _client.ApplyControlToVehicle(vehicle.GetId(), control);
    }
    // 应用控制到车辆（例如转向、加速等）
    void ApplyControlToVehicle(Vehicle &vehicle, const rpc::VehicleControl &control) {
      _client.ApplyControlToVehicle(vehicle.GetId(), control);
    }
    // 应用Ackermann控制（用于像汽车这类车辆）
    void ApplyAckermannControlToVehicle(Vehicle &vehicle, const rpc::VehicleAckermannControl &control) {
      _client.ApplyAckermannControlToVehicle(vehicle.GetId(), control);
    }

    rpc::AckermannControllerSettings GetAckermannControllerSettings(const Vehicle &vehicle) const {
      return _client.GetAckermannControllerSettings(vehicle.GetId());
    }
    //Walker 操作
    void ApplyAckermannControllerSettings(Vehicle &vehicle, const rpc::AckermannControllerSettings &settings) {
      _client.ApplyAckermannControllerSettings(vehicle.GetId(), settings);
    }
    // 给Walker应用控制指令（如行走、跳跃等）
    void ApplyControlToWalker(Walker &walker, const rpc::WalkerControl &control) {
      _client.ApplyControlToWalker(walker.GetId(), control);
    }
    // 获取Walker的骨骼变换
    rpc::WalkerBoneControlOut GetBonesTransform(Walker &walker) {
      return _client.GetBonesTransform(walker.GetId());
    }
    // 设置Walker的骨骼变换
    void SetBonesTransform(Walker &walker, const rpc::WalkerBoneControlIn &bones) {
      return _client.SetBonesTransform(walker.GetId(), bones);
    }
    // 混合Walker的姿势（动画过渡）
    void BlendPose(Walker &walker, float blend) {
      return _client.BlendPose(walker.GetId(), blend);
    }
    // 从动画中获取Walker的姿势
    void GetPoseFromAnimation(Walker &walker) {
      return _client.GetPoseFromAnimation(walker.GetId());
    }

    void ApplyPhysicsControlToVehicle(Vehicle &vehicle, const rpc::VehiclePhysicsControl &physicsControl) {
      _client.ApplyPhysicsControlToVehicle(vehicle.GetId(), physicsControl);
    }

    void SetLightStateToVehicle(Vehicle &vehicle, const rpc::VehicleLightState light_state) {
      _client.SetLightStateToVehicle(vehicle.GetId(), light_state);
    }

    void OpenVehicleDoor(Vehicle &vehicle, const rpc::VehicleDoor door_idx) {
      _client.OpenVehicleDoor(vehicle.GetId(), door_idx);
    }

    void CloseVehicleDoor(Vehicle &vehicle, const rpc::VehicleDoor door_idx) {
      _client.CloseVehicleDoor(vehicle.GetId(), door_idx);
    }

    void SetWheelSteerDirection(Vehicle &vehicle, rpc::VehicleWheelLocation wheel_location, float angle_in_deg) {
      _client.SetWheelSteerDirection(vehicle.GetId(), wheel_location, angle_in_deg);
    }

    float GetWheelSteerAngle(Vehicle &vehicle, rpc::VehicleWheelLocation wheel_location) {
      return _client.GetWheelSteerAngle(vehicle.GetId(), wheel_location);
    }
   // 车辆物理仿真与车辆部件操作
    // 启用CarSim模拟器
    void EnableCarSim(Vehicle &vehicle, std::string simfile_path) {
      _client.EnableCarSim(vehicle.GetId(), simfile_path);
    }
    // 启用或禁用CarSim道路
    void UseCarSimRoad(Vehicle &vehicle, bool enabled) {
      _client.UseCarSimRoad(vehicle.GetId(), enabled);
    }
    // 启用Chrono物理仿真
    void EnableChronoPhysics(Vehicle &vehicle,
        uint64_t MaxSubsteps,
        float MaxSubstepDeltaTime,
        std::string VehicleJSON,
        std::string PowertrainJSON,
        std::string TireJSON,
        std::string BaseJSONPath) {
      _client.EnableChronoPhysics(vehicle.GetId(),
          MaxSubsteps,
          MaxSubstepDeltaTime,
          VehicleJSON,
          PowertrainJSON,
          TireJSON,
          BaseJSONPath);
    }

    void RestorePhysXPhysics(Vehicle &vehicle) {
      _client.RestorePhysXPhysics(vehicle.GetId());
    }

    /// @}
    // =========================================================================
    /// @name 记录器的操作
    // =========================================================================
    /// @{

    std::string StartRecorder(std::string name, bool additional_data) {
      return _client.StartRecorder(std::move(name), additional_data);
    }

    void StopRecorder(void) {
      _client.StopRecorder();
    }

    std::string ShowRecorderFileInfo(std::string name, bool show_all) {
      return _client.ShowRecorderFileInfo(std::move(name), show_all);
    }

    std::string ShowRecorderCollisions(std::string name, char type1, char type2) {
      return _client.ShowRecorderCollisions(std::move(name), type1, type2);
    }

    std::string ShowRecorderActorsBlocked(std::string name, double min_time, double min_distance) {
      return _client.ShowRecorderActorsBlocked(std::move(name), min_time, min_distance);
    }

    std::string ReplayFile(std::string name, double start, double duration,
        uint32_t follow_id, bool replay_sensors) {
      return _client.ReplayFile(std::move(name), start, duration, follow_id, replay_sensors);
    }

    void SetReplayerTimeFactor(double time_factor) {
      _client.SetReplayerTimeFactor(time_factor);
    }

    void SetReplayerIgnoreHero(bool ignore_hero) {
      _client.SetReplayerIgnoreHero(ignore_hero);
    }

    void SetReplayerIgnoreSpectator(bool ignore_spectator) {
      _client.SetReplayerIgnoreSpectator(ignore_spectator);
    }

    void StopReplayer(bool keep_actors) {
      _client.StopReplayer(keep_actors);
  }

    /// @}
    // =========================================================================
    /// @name 传感器的操作
    // =========================================================================
    /// @{

    void SubscribeToSensor(
        const Sensor &sensor,
        std::function<void(SharedPtr<sensor::SensorData>)> callback);

    void UnSubscribeFromSensor(Actor &sensor);

    void EnableForROS(const Sensor &sensor);

    void DisableForROS(const Sensor &sensor);

    bool IsEnabledForROS(const Sensor &sensor);

    void SubscribeToGBuffer(
        Actor & sensor,
        uint32_t gbuffer_id,
        std::function<void(SharedPtr<sensor::SensorData>)> callback);

    void UnSubscribeFromGBuffer(
        Actor & sensor,
        uint32_t gbuffer_id);

    void Send(const Sensor &sensor, std::string message);        

    /// @}
    // =========================================================================
    /// @name 交通灯的操作
    // =========================================================================
    /// @{

    // 设置交通灯的状态
    void SetTrafficLightState(TrafficLight &trafficLight, const rpc::TrafficLightState trafficLightState) {
      _client.SetTrafficLightState(trafficLight.GetId(), trafficLightState);
    }

    void SetTrafficLightGreenTime(TrafficLight &trafficLight, float greenTime) {
      _client.SetTrafficLightGreenTime(trafficLight.GetId(), greenTime);
    }

    void SetTrafficLightYellowTime(TrafficLight &trafficLight, float yellowTime) {
      _client.SetTrafficLightYellowTime(trafficLight.GetId(), yellowTime);
    }

    void SetTrafficLightRedTime(TrafficLight &trafficLight, float redTime) {
      _client.SetTrafficLightRedTime(trafficLight.GetId(), redTime);
    }

    void FreezeTrafficLight(TrafficLight &trafficLight, bool freeze) {
      _client.FreezeTrafficLight(trafficLight.GetId(), freeze);
    }

    void ResetTrafficLightGroup(TrafficLight &trafficLight) {
      _client.ResetTrafficLightGroup(trafficLight.GetId());
    }

    void ResetAllTrafficLights() {
      _client.ResetAllTrafficLights();
    }

    std::vector<geom::BoundingBox> GetLightBoxes(const TrafficLight &trafficLight) const {
      return _client.GetLightBoxes(trafficLight.GetId());
    }

    std::vector<ActorId> GetGroupTrafficLights(TrafficLight &trafficLight) {
      return _client.GetGroupTrafficLights(trafficLight.GetId());
    }

    /// @}
    // =========================================================================
    /// @name 调试
    // =========================================================================
    /// @{

    void DrawDebugShape(const rpc::DebugShape &shape) {
      _client.DrawDebugShape(shape);
    }

    /// @}
    // =========================================================================
    /// @name Apply commands in batch
    // =========================================================================
    /// @{

    void ApplyBatch(std::vector<rpc::Command> commands, bool do_tick_cue) {
      _client.ApplyBatch(std::move(commands), do_tick_cue);
    }

    auto ApplyBatchSync(std::vector<rpc::Command> commands, bool do_tick_cue) {
      return _client.ApplyBatchSync(std::move(commands), do_tick_cue);
    }

    /// @}
    // =========================================================================
    /// @name 操作灯
    // =========================================================================
    /// @{

    SharedPtr<LightManager> GetLightManager() const {
      return _light_manager;
    }

    std::vector<rpc::LightState> QueryLightsStateToServer() const {
      return _client.QueryLightsStateToServer();
    }

    void UpdateServerLightsState(
        std::vector<rpc::LightState>& lights,
        bool discard_client = false) const {
      _client.UpdateServerLightsState(lights, discard_client);
    }

    void UpdateDayNightCycle(const bool active) const {
      _client.UpdateDayNightCycle(active);
    }

    size_t RegisterLightUpdateChangeEvent(std::function<void(WorldSnapshot)> callback) {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->RegisterLightUpdateChangeEvent(std::move(callback));
    }

    void RemoveLightUpdateChangeEvent(size_t id) {
      DEBUG_ASSERT(_episode != nullptr);
      _episode->RemoveLightUpdateChangeEvent(id);
    }

    void FreezeAllTrafficLights(bool frozen);

    /// @}
    // =========================================================================
    /// @name 纹理更新操作
    // =========================================================================
    /// @{

    void ApplyColorTextureToObjects(
        const std::vector<std::string> &objects_name,
        const rpc::MaterialParameter& parameter,
        const rpc::TextureColor& Texture);

    void ApplyColorTextureToObjects(
        const std::vector<std::string> &objects_name,
        const rpc::MaterialParameter& parameter,
        const rpc::TextureFloatColor& Texture);

    std::vector<std::string> GetNamesOfAllObjects() const;

    /// @}

  private:

    bool ShouldUpdateMap(rpc::MapInfo& map_info);

    Client _client;

    SharedPtr<LightManager> _light_manager;

    std::shared_ptr<Episode> _episode;

    const GarbageCollectionPolicy _gc_policy;

    SharedPtr<Map> _cached_map;

    std::string _open_drive_file;
  };

} // namespace detail
} // namespace client
} // namespace carla
