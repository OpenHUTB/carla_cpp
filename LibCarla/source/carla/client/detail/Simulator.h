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

    EpisodeProxy LoadEpisode(std::string map_name, bool reset_settings = true, rpc::MapLayer map_layers = rpc::MapLayer::All);

    void LoadLevelLayer(rpc::MapLayer map_layers) const {
      _client.LoadLevelLayer(map_layers);
    }

    void UnloadLevelLayer(rpc::MapLayer map_layers) const {
      _client.UnloadLevelLayer(map_layers);
    }

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
    auto GetCurrentEpisodeId() {
      GetReadyCurrentEpisode();
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetId();
    }

    void GetReadyCurrentEpisode();
    EpisodeProxy GetCurrentEpisode();

    /// @}
    // =========================================================================
    /// @name 世界快照
    // =========================================================================
    /// @{

    World GetWorld() {
      return World{GetCurrentEpisode()};
    }

    /// @}
    // =========================================================================
    /// @name 世界快照
    // =========================================================================
    /// @{

    WorldSnapshot GetWorldSnapshot() const {
      DEBUG_ASSERT(_episode != nullptr);
      return WorldSnapshot{_episode->GetState()};
    }

    /// @}
    // =========================================================================
    /// @name 地图相关的方法
    // =========================================================================
    /// @{

    SharedPtr<Map> GetCurrentMap();

    std::vector<std::string> GetAvailableMaps() {
      return _client.GetAvailableMaps();
    }

    /// @}
    // =========================================================================
    /// @name 所需文件相关的方法
    // =========================================================================
    /// @{

    bool SetFilesBaseFolder(const std::string &path);

    std::vector<std::string> GetRequiredFiles(const std::string &folder = "", const bool download = true) const;

    void RequestFile(const std::string &name) const;

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

    void SetNetworkingTimeout(time_duration timeout) {
      _client.SetTimeout(timeout);
    }

    time_duration GetNetworkingTimeout() {
      return _client.GetTimeout();
    }

    std::string GetClientVersion() {
      return _client.GetClientVersion();
    }

    std::string GetServerVersion() {
      return _client.GetServerVersion();
    }

    /// @}
    // =========================================================================
    /// @name 节拍
    // =========================================================================
    /// @{

    WorldSnapshot WaitForTick(time_duration timeout);

    size_t RegisterOnTickEvent(std::function<void(WorldSnapshot)> callback) {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->RegisterOnTickEvent(std::move(callback));
    }

    void RemoveOnTickEvent(size_t id) {
      DEBUG_ASSERT(_episode != nullptr);
      _episode->RemoveOnTickEvent(id);
    }

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
      _episode->AddPendingException(e);
    }

    SharedPtr<BlueprintLibrary> GetBlueprintLibrary();

    /// 返回一个列表，其中第一个元素是车辆 ID，第二个元素是灯光状态
    rpc::VehicleLightStateList GetVehiclesLightStates();

    SharedPtr<Actor> GetSpectator();

    rpc::EpisodeSettings GetEpisodeSettings() {
      return _client.GetEpisodeSettings();
    }

    uint64_t SetEpisodeSettings(const rpc::EpisodeSettings &settings);

    rpc::WeatherParameters GetWeatherParameters() {
      return _client.GetWeatherParameters();
    }

    void SetWeatherParameters(const rpc::WeatherParameters &weather) {
      _client.SetWeatherParameters(weather);
    }

    float GetIMUISensorGravity() const {
      return _client.GetIMUISensorGravity();
    }

    void SetIMUISensorGravity(float NewIMUISensorGravity) {
      _client.SetIMUISensorGravity(NewIMUISensorGravity);
    }

    rpc::VehiclePhysicsControl GetVehiclePhysicsControl(const Vehicle &vehicle) const {
      return _client.GetVehiclePhysicsControl(vehicle.GetId());
    }

    rpc::VehicleLightState GetVehicleLightState(const Vehicle &vehicle) const {
      return _client.GetVehicleLightState(vehicle.GetId());
    }

    /// Returns all the BBs of all the elements of the level
    std::vector<geom::BoundingBox> GetLevelBBs(uint8_t queried_tag) const {
      return _client.GetLevelBBs(queried_tag);
    }

    std::vector<rpc::EnvironmentObject> GetEnvironmentObjects(uint8_t queried_tag) const {
      return _client.GetEnvironmentObjects(queried_tag);
    }

    void EnableEnvironmentObjects(
      std::vector<uint64_t> env_objects_ids,
      bool enable) const {
      _client.EnableEnvironmentObjects(env_objects_ids, enable);
    }

    std::pair<bool,rpc::LabelledPoint> ProjectPoint(
        geom::Location location, geom::Vector3D direction, float search_distance) const {
      return _client.ProjectPoint(location, direction, search_distance);
    }

    std::vector<rpc::LabelledPoint> CastRay(
        geom::Location start_location, geom::Location end_location) const {
      return _client.CastRay(start_location, end_location);
    }

    /// @}
    // =========================================================================
    /// @name 人工智能
    // =========================================================================
    /// @{

    std::shared_ptr<WalkerNavigation> GetNavigation();

    void NavigationTick();

    void RegisterAIController(const WalkerAIController &controller);

    void UnregisterAIController(const WalkerAIController &controller);

    boost::optional<geom::Location> GetRandomLocationFromNavigation();

    void SetPedestriansCrossFactor(float percentage);

    void SetPedestriansSeed(unsigned int seed);

    /// @}
    // =========================================================================
    /// @name 参与者的一般操作
    // =========================================================================
    /// @{

    boost::optional<rpc::Actor> GetActorById(ActorId id) const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetActorById(id);
    }

    std::vector<rpc::Actor> GetActorsById(const std::vector<ActorId> &actor_ids) const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetActorsById(actor_ids);
    }

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

    bool DestroyActor(Actor &actor);

    bool DestroyActor(ActorId actor_id)
    {
      return _client.DestroyActor(actor_id);
    }

    ActorSnapshot GetActorSnapshot(ActorId actor_id) const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetState()->GetActorSnapshot(actor_id);
    }

    ActorSnapshot GetActorSnapshot(const Actor &actor) const {
      return GetActorSnapshot(actor.GetId());
    }

    rpc::ActorState GetActorState(const Actor &actor) const {
      return GetActorSnapshot(actor).actor_state;
    }

    geom::Location GetActorLocation(const Actor &actor) const {
      return GetActorSnapshot(actor).transform.location;
    }

    geom::Transform GetActorTransform(const Actor &actor) const {
      return GetActorSnapshot(actor).transform;
    }

    geom::Vector3D GetActorVelocity(const Actor &actor) const {
      return GetActorSnapshot(actor).velocity;
    }

    void SetActorTargetVelocity(const Actor &actor, const geom::Vector3D &vector) {
      _client.SetActorTargetVelocity(actor.GetId(), vector);
    }

    geom::Vector3D GetActorAngularVelocity(const Actor &actor) const {
      return GetActorSnapshot(actor).angular_velocity;
    }

    void SetActorTargetAngularVelocity(const Actor &actor, const geom::Vector3D &vector) {
      _client.SetActorTargetAngularVelocity(actor.GetId(), vector);
    }
    void EnableActorConstantVelocity(const Actor &actor, const geom::Vector3D &vector) {
      _client.EnableActorConstantVelocity(actor.GetId(), vector);
    }

    void DisableActorConstantVelocity(const Actor &actor) {
      _client.DisableActorConstantVelocity(actor.GetId());
    }

    void AddActorImpulse(const Actor &actor, const geom::Vector3D &impulse) {
      _client.AddActorImpulse(actor.GetId(), impulse);
    }

    void AddActorImpulse(const Actor &actor, const geom::Vector3D &impulse, const geom::Vector3D &location) {
      _client.AddActorImpulse(actor.GetId(), impulse, location);
    }

    void AddActorForce(const Actor &actor, const geom::Vector3D &force) {
      _client.AddActorForce(actor.GetId(), force);
    }

    void AddActorForce(const Actor &actor, const geom::Vector3D &force, const geom::Vector3D &location) {
      _client.AddActorForce(actor.GetId(), force, location);
    }

    void AddActorAngularImpulse(const Actor &actor, const geom::Vector3D &vector) {
      _client.AddActorAngularImpulse(actor.GetId(), vector);
    }

    void AddActorTorque(const Actor &actor, const geom::Vector3D &torque) {
      _client.AddActorAngularImpulse(actor.GetId(), torque);
    }

    geom::Vector3D GetActorAcceleration(const Actor &actor) const {
      return GetActorSnapshot(actor).acceleration;
    }

    geom::Transform GetActorComponentWorldTransform(const Actor &actor, const std::string componentName) {
      return _client.GetActorComponentWorldTransform(actor.GetId(), componentName);
    }

    geom::Transform GetActorComponentRelativeTransform(const Actor &actor, std::string componentName) {
      return _client.GetActorComponentRelativeTransform(actor.GetId(), componentName);
    }

    std::vector<geom::Transform> GetActorBoneWorldTransforms(const Actor &actor) {
      return _client.GetActorBoneWorldTransforms(actor.GetId());
    }

    std::vector<geom::Transform> GetActorBoneRelativeTransforms(const Actor &actor) {
      return _client.GetActorBoneRelativeTransforms(actor.GetId());
    }

    std::vector<std::string> GetActorComponentNames(const Actor &actor) {
      return _client.GetActorComponentNames(actor.GetId());
    }

    std::vector<std::string> GetActorBoneNames(const Actor &actor) {
      return _client.GetActorBoneNames(actor.GetId());
    }

    std::vector<geom::Transform> GetActorSocketWorldTransforms(const Actor &actor) {
      return _client.GetActorSocketWorldTransforms(actor.GetId());
    }

    std::vector<geom::Transform> GetActorSocketRelativeTransforms(const Actor &actor) {
      return _client.GetActorSocketRelativeTransforms(actor.GetId());
    }

    std::vector<std::string> GetActorSocketNames(const Actor &actor) {
      return _client.GetActorSocketNames(actor.GetId());
    }    

    void SetActorLocation(Actor &actor, const geom::Location &location) {
      _client.SetActorLocation(actor.GetId(), location);
    }

    void SetActorTransform(Actor &actor, const geom::Transform &transform) {
      _client.SetActorTransform(actor.GetId(), transform);
    }

    void SetActorSimulatePhysics(Actor &actor, bool enabled) {
      _client.SetActorSimulatePhysics(actor.GetId(), enabled);
    }

    void SetActorCollisions(Actor &actor, bool enabled) {
      _client.SetActorCollisions(actor.GetId(), enabled);
    }

    void SetActorCollisions(ActorId actor_id, bool enabled) {
      _client.SetActorCollisions(actor_id, enabled);
    }

    void SetActorDead(Actor &actor) {
      _client.SetActorDead(actor.GetId());
    }

    void SetActorDead(ActorId actor_id) {
      _client.SetActorDead(actor_id);
    }

    void SetActorEnableGravity(Actor &actor, bool enabled) {
      _client.SetActorEnableGravity(actor.GetId(), enabled);
    }

    /// @}
    // =========================================================================
    /// @name 车辆的操作
    // =========================================================================
    /// @{

    void SetVehicleAutopilot(Vehicle &vehicle, bool enabled = true) {
      _client.SetActorAutopilot(vehicle.GetId(), enabled);
    }

    rpc::VehicleTelemetryData GetVehicleTelemetryData(const Vehicle &vehicle) const {
      return _client.GetVehicleTelemetryData(vehicle.GetId());
    }

    void ShowVehicleDebugTelemetry(Vehicle &vehicle, bool enabled = true) {
      _client.ShowVehicleDebugTelemetry(vehicle.GetId(), enabled);
    }

    void SetLightsToVehicle(Vehicle &vehicle, const rpc::VehicleControl &control) {
      _client.ApplyControlToVehicle(vehicle.GetId(), control);
    }

    void ApplyControlToVehicle(Vehicle &vehicle, const rpc::VehicleControl &control) {
      _client.ApplyControlToVehicle(vehicle.GetId(), control);
    }

    void ApplyAckermannControlToVehicle(Vehicle &vehicle, const rpc::VehicleAckermannControl &control) {
      _client.ApplyAckermannControlToVehicle(vehicle.GetId(), control);
    }

    rpc::AckermannControllerSettings GetAckermannControllerSettings(const Vehicle &vehicle) const {
      return _client.GetAckermannControllerSettings(vehicle.GetId());
    }

    void ApplyAckermannControllerSettings(Vehicle &vehicle, const rpc::AckermannControllerSettings &settings) {
      _client.ApplyAckermannControllerSettings(vehicle.GetId(), settings);
    }

    void ApplyControlToWalker(Walker &walker, const rpc::WalkerControl &control) {
      _client.ApplyControlToWalker(walker.GetId(), control);
    }

    rpc::WalkerBoneControlOut GetBonesTransform(Walker &walker) {
      return _client.GetBonesTransform(walker.GetId());
    }

    void SetBonesTransform(Walker &walker, const rpc::WalkerBoneControlIn &bones) {
      return _client.SetBonesTransform(walker.GetId(), bones);
    }

    void BlendPose(Walker &walker, float blend) {
      return _client.BlendPose(walker.GetId(), blend);
    }

    void GetPoseFromAnimation(Walker &walker) {
      return _client.GetPoseFromAnimation(walker.GetId());
    }
    // 应用物理控制到指定车辆
    void ApplyPhysicsControlToVehicle(Vehicle &vehicle, const rpc::VehiclePhysicsControl &physicsControl) {
      _client.ApplyPhysicsControlToVehicle(vehicle.GetId(), physicsControl);
    }
    // 设置指定车辆的灯光状态
    void SetLightStateToVehicle(Vehicle &vehicle, const rpc::VehicleLightState light_state) {
      _client.SetLightStateToVehicle(vehicle.GetId(), light_state);
    }
    // 打开指定车辆的某个车门
    void OpenVehicleDoor(Vehicle &vehicle, const rpc::VehicleDoor door_idx) {
      _client.OpenVehicleDoor(vehicle.GetId(), door_idx);
    }
    // 关闭指定车辆的某个车门
    void CloseVehicleDoor(Vehicle &vehicle, const rpc::VehicleDoor door_idx) {
      _client.CloseVehicleDoor(vehicle.GetId(), door_idx);
    }
    // 设置指定车辆的车轮转向角度
    void SetWheelSteerDirection(Vehicle &vehicle, rpc::VehicleWheelLocation wheel_location, float angle_in_deg) {
      _client.SetWheelSteerDirection(vehicle.GetId(), wheel_location, angle_in_deg);
    }
    // 获取指定车辆的车轮当前转向角度
    float GetWheelSteerAngle(Vehicle &vehicle, rpc::VehicleWheelLocation wheel_location) {
      return _client.GetWheelSteerAngle(vehicle.GetId(), wheel_location);
    }

    void EnableCarSim(Vehicle &vehicle, std::string simfile_path) {
      _client.EnableCarSim(vehicle.GetId(), simfile_path);
    }

    void UseCarSimRoad(Vehicle &vehicle, bool enabled) {
      _client.UseCarSimRoad(vehicle.GetId(), enabled);
    }

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

    // 启动录制器
    std::string StartRecorder(std::string name, bool additional_data) {
     // _client对象调用StartRecorder方法来启动录制。
    // additional_data: 是否记录额外的数据，控制录制的详细程度。
    // 使用std::move()转移name的所有权，提高效率。
      return _client.StartRecorder(std::move(name), additional_data);
    }
    // 停止录制器
    void StopRecorder(void) {
      _client.StopRecorder();
    }
    // 显示录制文件的相关信息
    std::string ShowRecorderFileInfo(std::string name, bool show_all) {
        // show_all: 是否显示所有信息。
      return _client.ShowRecorderFileInfo(std::move(name), show_all);
    }
    // 显示录制期间的碰撞信息
    std::string ShowRecorderCollisions(std::string name, char type1, char type2) {
      return _client.ShowRecorderCollisions(std::move(name), type1, type2);
    }
    // 显示在录制期间被阻挡的角色信息
    std::string ShowRecorderActorsBlocked(std::string name, double min_time, double min_distance) {
      return _client.ShowRecorderActorsBlocked(std::move(name), min_time, min_distance);
    }
    // 回放录制的文件
    std::string ReplayFile(std::string name, double start, double duration,
        uint32_t follow_id, bool replay_sensors) {
      return _client.ReplayFile(std::move(name), start, duration, follow_id, replay_sensors);
    }
    // 设置回放器的时间比例因子
    void SetReplayerTimeFactor(double time_factor) {
        // time_factor: 时间比例因子，控制回放速度。
      _client.SetReplayerTimeFactor(time_factor);
    }
    // 设置回放器是否忽略英雄角色
    void SetReplayerIgnoreHero(bool ignore_hero) {
      _client.SetReplayerIgnoreHero(ignore_hero);
    }
    // 设置回放器是否忽略观众角色
    void SetReplayerIgnoreSpectator(bool ignore_spectator) {
      _client.SetReplayerIgnoreSpectator(ignore_spectator);
    }
    // 停止回放器
    void StopReplayer(bool keep_actors) {
      _client.StopReplayer(keep_actors);
  }

    /// @}
    // =========================================================================
    /// @name 传感器的操作
    // =========================================================================
    /// @{

    // 订阅传感器数据
    void SubscribeToSensor(
        const Sensor &sensor,
        std::function<void(SharedPtr<sensor::SensorData>)> callback);
    // callback: 回调函数，当传感器数据更新时会被调用，传入数据指针。
    // SharedPtr是智能指针，用于共享传感器数据的所有权，防止内存泄漏。

    // 取消订阅传感器数据
    void UnSubscribeFromSensor(Actor &sensor);

    // 启用传感器以便与ROS通信
    void EnableForROS(const Sensor &sensor);

    // 禁用传感器与ROS通信
    void DisableForROS(const Sensor &sensor);

    // 检查传感器是否启用了ROS通信
    bool IsEnabledForROS(const Sensor &sensor);

    // 订阅传感器的GBuffer（图形缓冲区）数据
    void SubscribeToGBuffer(
        Actor & sensor,
        uint32_t gbuffer_id,
        std::function<void(SharedPtr<sensor::SensorData>)> callback);

    // 取消订阅传感器的GBuffer数据
    void UnSubscribeFromGBuffer(
        Actor & sensor,
        uint32_t gbuffer_id);

    // 向传感器发送消息
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
    // 设置交通灯的绿灯时间
    void SetTrafficLightGreenTime(TrafficLight &trafficLight, float greenTime) {
      _client.SetTrafficLightGreenTime(trafficLight.GetId(), greenTime);
    }
    // 设置交通灯的黄灯时间
    void SetTrafficLightYellowTime(TrafficLight &trafficLight, float yellowTime) {
      _client.SetTrafficLightYellowTime(trafficLight.GetId(), yellowTime);
    }
    // 设置交通灯的红灯时间
    void SetTrafficLightRedTime(TrafficLight &trafficLight, float redTime) {
      _client.SetTrafficLightRedTime(trafficLight.GetId(), redTime);
    }
    // 冻结或解冻交通灯（使交通灯保持当前状态）
    void FreezeTrafficLight(TrafficLight &trafficLight, bool freeze) {
      _client.FreezeTrafficLight(trafficLight.GetId(), freeze);
    }
    // 重置单个交通灯组的状态
    void ResetTrafficLightGroup(TrafficLight &trafficLight) {
      _client.ResetTrafficLightGroup(trafficLight.GetId());
    }
    // 重置所有交通灯
    void ResetAllTrafficLights() {
      _client.ResetAllTrafficLights();
    }
    // 获取与给定交通灯相关的所有 BoundingBox 对象（通常用于碰撞检测或可视化调试）
    std::vector<geom::BoundingBox> GetLightBoxes(const TrafficLight &trafficLight) const {
      return _client.GetLightBoxes(trafficLight.GetId());
    }
    // 获取与给定交通灯属于同一组的所有交通灯的 ID
    std::vector<ActorId> GetGroupTrafficLights(TrafficLight &trafficLight) {
      return _client.GetGroupTrafficLights(trafficLight.GetId());
    }

    /// @}
    // =========================================================================
    /// @name 调试
    // =========================================================================
    /// @{

    void DrawDebugShape(const rpc::DebugShape &shape) {
        // 绘制一个调试用的形状，用于调试或可视化
      _client.DrawDebugShape(shape);
    }

    /// @}
    // =========================================================================
    /// @name Apply commands in batch
    // =========================================================================
    /// @{

    void ApplyBatch(std::vector<rpc::Command> commands, bool do_tick_cue) {
        // 将多个命令批量应用到服务端
      _client.ApplyBatch(std::move(commands), do_tick_cue);
    }
    // 批量应用命令并同步等待结果
    auto ApplyBatchSync(std::vector<rpc::Command> commands, bool do_tick_cue) {
        // 同步执行批量命令
      return _client.ApplyBatchSync(std::move(commands), do_tick_cue);
    }

    /// @}
    // =========================================================================
    /// @name 操作灯
    // =========================================================================
    /// @{

    SharedPtr<LightManager> GetLightManager() const {
        // 获取当前的灯光管理器
      return _light_manager;
    }

    std::vector<rpc::LightState> QueryLightsStateToServer() const {
        // 查询服务器上所有灯光的状态
      return _client.QueryLightsStateToServer();
      // 向服务器查询当前所有灯光的状态并返回
    }

    // 更新服务器上的灯光状态
    void UpdateServerLightsState(
        std::vector<rpc::LightState>& lights,
        bool discard_client = false) const {
      _client.UpdateServerLightsState(lights, discard_client);
    }

    // 更新昼夜循环（控制白天/黑夜的切换）
    void UpdateDayNightCycle(const bool active) const {
      _client.UpdateDayNightCycle(active);
    }

    // 注册一个灯光状态更新的事件回调
    size_t RegisterLightUpdateChangeEvent(std::function<void(WorldSnapshot)> callback) {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->RegisterLightUpdateChangeEvent(std::move(callback));
    }
    // 移除一个灯光状态更新的事件回调
    void RemoveLightUpdateChangeEvent(size_t id) {
      DEBUG_ASSERT(_episode != nullptr);
      _episode->RemoveLightUpdateChangeEvent(id);
    }
    // 冻结或解冻所有交通灯
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
    // 将颜色纹理应用到一组对象上

    void ApplyColorTextureToObjects(
        const std::vector<std::string> &objects_name,
        const rpc::MaterialParameter& parameter,
        const rpc::TextureFloatColor& Texture);
    // 将浮动颜色纹理应用到一组对象上

    std::vector<std::string> GetNamesOfAllObjects() const;
    // 获取所有对象的名称

    /// @}

  private:
      // 判断是否需要更新地图
    bool ShouldUpdateMap(rpc::MapInfo& map_info);

    // 客户端对象，负责与服务器交互
    Client _client;

    // 灯光管理器，用于管理和操作场景中的灯光
    SharedPtr<LightManager> _light_manager;

    // 任务场景的 Episode（任务执行环境）
    std::shared_ptr<Episode> _episode;

    // 垃圾回收策略
    const GarbageCollectionPolicy _gc_policy;

    // 缓存的地图对象
    SharedPtr<Map> _cached_map;

    // 存储打开的道路文件
    std::string _open_drive_file;
  };

} // namespace detail
} // namespace client
} // namespace carla
