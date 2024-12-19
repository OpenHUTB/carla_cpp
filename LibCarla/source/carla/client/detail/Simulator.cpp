// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/Simulator.h"

#include "carla/Debug.h"
#include "carla/Exception.h"
#include "carla/Logging.h"
#include "carla/RecurrentSharedFuture.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/FileTransfer.h"
#include "carla/client/Map.h"
#include "carla/client/Sensor.h"
#include "carla/client/TimeoutException.h"
#include "carla/client/WalkerAIController.h"
#include "carla/client/detail/ActorFactory.h"
#include "carla/client/detail/WalkerNavigation.h"
#include "carla/trafficmanager/TrafficManager.h"
#include "carla/sensor/Deserializer.h"

#include <exception>
#include <thread>

using namespace std::string_literals;

namespace carla {
namespace client {
namespace detail {

  // ===========================================================================
  // -- 静态局部方法 ------------------------------------------------------------
  // ===========================================================================

  static void ValidateVersions(Client &client) {
    const auto vc = client.GetClientVersion();
    const auto vs = client.GetServerVersion();
    if (vc != vs) {
      log_warning(
          "Version mismatch detected: You are trying to connect to a simulator",
          "that might be incompatible with this API");
      log_warning("Client API version     =", vc);
      log_warning("Simulator API version  =", vs);
    }
  }

  static bool SynchronizeFrame(uint64_t frame, const Episode &episode, time_duration timeout) {
    bool result = true;//初始化结果为true，表示默认同步成功
    auto start = std::chrono::system_clock::now();//获取当前时间点作为开始时间
    while (frame > episode.GetState()->GetTimestamp().frame) {//当当前帧大于episode中的状态时，循环等待
      std::this_thread::yield();//放弃当前线程的剩余时间片，允许其他线程运行
      auto end = std::chrono::system_clock::now();//获取当前时间点作为结束时间
      auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);//计算从开始到结束的时间差，并转换为毫秒
      if(timeout.to_chrono() < diff) {//如果已经超过了指定的超时时间，则设置结果为false并退出循环
        result = false;
        break;
      }
    }
    if(result) {//如果成功同步，则调用TrafficManager的Tick方法
      carla::traffic_manager::TrafficManager::Tick();
    }

    return result;//返回同步结果
  }

  // ===========================================================================
  // -- 构造函数 ----------------------------------------------------------------
  // ===========================================================================

  Simulator::Simulator(//Simulator类的构造函数定义
      const std::string &host,//连接到CARLA服务器的主机名或IP地址
      const uint16_t port,//与CARLA服务器通信的端口号
      const size_t worker_threads,//用于处理通信的工作线程的数量
      const bool enable_garbage_collection)//是否启用垃圾回收
    : LIBCARLA_INITIALIZE_LIFETIME_PROFILER("SimulatorClient("s + host + ":" + std::to_string(port) + ")"),//初始化性能分析器
      _client(host, port, worker_threads),//初始化与CARLA服务器的连接
      _light_manager(new LightManager()),//动态分配LightManager对象
      _gc_policy(enable_garbage_collection ?//根据参数设置垃圾回收攻略
        GarbageCollectionPolicy::Enabled : GarbageCollectionPolicy::Disabled) {}
        //构造函数完成对象的初始化，所有工作都在初始化列表中完成

  // ===========================================================================
  // -- 加载新的场景 -------------------------------------------------------------
  // ===========================================================================

  EpisodeProxy Simulator::LoadEpisode(std::string map_name, bool reset_settings, rpc::MapLayer map_layers) {
    const auto id = GetCurrentEpisode().GetId();
    _client.LoadEpisode(std::move(map_name), reset_settings, map_layers);

    // 修复在切换地图时无法加载导航信息的漏洞：https://github.com/OpenHUTB/carla/commit/9e94feb3a52e6f5ba01d8a0e579e5cd3c008a507
    // 以前，加载新地图时，旧地图的导航信息会被保留。
    // 这是因为 Episode 对象未被替换。这会导致意外行为，例如，行人会生成在地图上不合适的位置。
    assert(_episode.use_count() == 1);
    // 删除指向_episode的指针，以便为正确的地图加载导航信息
    _episode.reset();  // 释放 _episode 资源并转换为空 shared_ptr 对象
    GetReadyCurrentEpisode();  // 访问当前的（新的）场景

    // 我们正在等待服务器重新加载地图片段的50毫秒。
    // 如果此时没有检测到事件的变化，将再次尝试“number_of_attempts”次。
    // TODO 这个时间是完全任意的，所以我们需要改进这个管线，使其不依赖于这个时间，
    // 因为这个超时可能导致客户端在以同步模式加载地图时以不同的初始节拍恢复模拟。
    // 尝试的次数 = 最大连接服务端的超时时间（单位毫秒） / 每次尝试连接的时间间隔（50毫秒）
    size_t number_of_attempts = _client.GetTimeout().milliseconds() / 50u;

    for (auto i = 0u; i < number_of_attempts; ++i) {
      using namespace std::literals::chrono_literals; // 使用chrono中的有序常量集合
      if (_client.GetEpisodeSettings().synchronous_mode)
        _client.SendTickCue();  // 如果是同步模式，则客户端向服务端发送节拍信号

      _episode->WaitForState(50ms);  // 每次等待50毫秒
      auto episode = GetCurrentEpisode();  // 获取当前的场景

      // 如果当前（等待之后）的场景和进入函数时的场景不一样，表示已经切换到新的场景，则返回当前场景
      if (episode.GetId() != id) {
        return episode;
      }
    }
    // 如果尝试“number_of_attempts”次后仍然没有切换到新的场景，则报错
    throw_exception(std::runtime_error("failed to connect to newly created map"));
  }

  EpisodeProxy Simulator::LoadOpenDriveEpisode(
      std::string opendrive,
      const rpc::OpendriveGenerationParameters & params, bool reset_settings) {
    // The "OpenDriveMap" is an ".umap" located in:
    // "carla/Unreal/CarlaUE4/Content/Carla/Maps/"
    // It will load the last sended OpenDRIVE by client's "LoadOpenDriveEpisode()"
    constexpr auto custom_opendrive_map = "OpenDriveMap";
    _client.CopyOpenDriveToServer(std::move(opendrive), params);
    return LoadEpisode(custom_opendrive_map, reset_settings);
  }

  // ===========================================================================
  // -- 访问当前场景 ------------------------------------------------------------
  // ===========================================================================

  void Simulator::GetReadyCurrentEpisode() {
    if (_episode == nullptr) {
      ValidateVersions(_client);
      _episode = std::make_shared<Episode>(_client, std::weak_ptr<Simulator>(shared_from_this()));
      _episode->Listen();
      if (!GetEpisodeSettings().synchronous_mode) {
        WaitForTick(_client.GetTimeout());
      }
      _light_manager->SetEpisode(WeakEpisodeProxy{shared_from_this()});
    }
  }
EpisodeProxy Simulator::GetCurrentEpisode() {
    GetReadyCurrentEpisode();
    return EpisodeProxy{shared_from_this()};
  }

  bool Simulator::ShouldUpdateMap(rpc::MapInfo& map_info) {
    if (!_cached_map) {
      return true;
    }
    if (map_info.name != _cached_map->GetName() ||
        _open_drive_file.size() != _cached_map->GetOpenDrive().size()) {
      return true;
    }
    return false;
  }

  SharedPtr<Map> Simulator::GetCurrentMap() {
    DEBUG_ASSERT(_episode != nullptr);
    if (!_cached_map || _episode->HasMapChangedSinceLastCall()) {
      rpc::MapInfo map_info = _client.GetMapInfo();
      std::string map_name;
      std::string map_base_path;
      bool fill_base_string = false;
      for (int i = map_info.name.size() - 1; i >= 0; --i) {
        if (fill_base_string == false && map_info.name[i] != '/') {
          map_name += map_info.name[i];
        } else {
          map_base_path += map_info.name[i];
          fill_base_string = true;
        }
      }
      std::reverse(map_name.begin(), map_name.end());
      std::reverse(map_base_path.begin(), map_base_path.end());
      std::string XODRFolder = map_base_path + "/OpenDrive/" + map_name + ".xodr";
      if (FileTransfer::FileExists(XODRFolder) == false) _client.GetRequiredFiles();
      _open_drive_file = _client.GetMapData();
      _cached_map = MakeShared<Map>(map_info, _open_drive_file);
    }

    return _cached_map;
  }

  // ===========================================================================
  // -- 所需要的文件 ------------------------------------------------------------
  // ===========================================================================


    bool Simulator::SetFilesBaseFolder(const std::string &path) {
      return _client.SetFilesBaseFolder(path);
    }

    std::vector<std::string> Simulator::GetRequiredFiles(const std::string &folder, const bool download) const {
      return _client.GetRequiredFiles(folder, download);
    }

    void Simulator::RequestFile(const std::string &name) const {
      _client.RequestFile(name);
    }

    std::vector<uint8_t> Simulator::GetCacheFile(const std::string &name, const bool request_otherwise) const {
      return _client.GetCacheFile(name, request_otherwise);
    }

  // ===========================================================================
  // -- 节拍 -------------------------------------------------------------------
  // ===========================================================================

  WorldSnapshot Simulator::WaitForTick(time_duration timeout) {
    DEBUG_ASSERT(_episode != nullptr);

    // 发出行人导航节拍
    NavigationTick();

    auto result = _episode->WaitForState(timeout);
    if (!result.has_value()) {
      throw_exception(TimeoutException(_client.GetEndpoint(), timeout));
    }
    return *result;
  }

  uint64_t Simulator::Tick(time_duration timeout) {
    DEBUG_ASSERT(_episode != nullptr);

    // 发出行人导航节拍
    NavigationTick();

    // 发送节拍命令
    const auto frame = _client.SendTickCue();

    // 等待，直到收到新的场景
    bool result = SynchronizeFrame(frame, *_episode, timeout);
    if (!result) {
      throw_exception(TimeoutException(_client.GetEndpoint(), timeout));
    }
    return frame;
  }

  // ===========================================================================
  // -- 在场景中访问全局对象 -----------------------------------------------------
  // ===========================================================================
  // 获取蓝图库对象
  SharedPtr<BlueprintLibrary> Simulator::GetBlueprintLibrary() {
      // 获取 Actor 定义的列表
    auto defs = _client.GetActorDefinitions();
    // 返回一个智能指针，指向 BlueprintLibrary 对象，构造时传入定义的列表
    return MakeShared<BlueprintLibrary>(std::move(defs));
  }
  // 获取车辆灯光状态列表
  rpc::VehicleLightStateList Simulator::GetVehiclesLightStates() {
    return _client.GetVehiclesLightStates();
  }
  // 获取观众对象
  SharedPtr<Actor> Simulator::GetSpectator() {
      // 调用 _client 的 GetSpectator 方法获取 Spectator（观众）对象
    return MakeActor(_client.GetSpectator());
  }

  // 设置仿真（Episode）配置
  uint64_t Simulator::SetEpisodeSettings(const rpc::EpisodeSettings &settings) {
      // 检查同步模式下是否没有设置固定的时间增量
    if (settings.synchronous_mode && !settings.fixed_delta_seconds) {
      log_warning(
          "synchronous mode enabled with variable delta seconds. It is highly "
          "recommended to set 'fixed_delta_seconds' when running on synchronous mode.");
    }
    // 检查同步模式和子步进模式同时启用的情况下，子步数是否合理
    else if (settings.synchronous_mode && settings.substepping) {
      // 最大物理子步数 必须在[1,16]范围之内
      if(settings.max_substeps < 1 || settings.max_substeps > 16) {
        log_warning(
            "synchronous mode and substepping are enabled but the number of substeps is not valid. "
            "Please be aware that this value needs to be in the range [1-16].");
      }
      // 检查每个子步增量与最大子步数的乘积是否小于等于固定的时间增量
      double n_substeps = settings.fixed_delta_seconds.get() / settings.max_substep_delta_time;

      if (n_substeps > static_cast<double>(settings.max_substeps)) {
        log_warning(
            "synchronous mode and substepping are enabled but the values for the simulation are not valid. "
            "The values should fulfil fixed_delta_seconds <= max_substep_delta_time * max_substeps. "
            "Be very careful about that, the time deltas are not guaranteed.");
      }
    }
    // 调用 _client 的 SetEpisodeSettings 方法，设置仿真环境的配置
    const auto frame = _client.SetEpisodeSettings(settings);
    // 同步当前帧与目标帧
    using namespace std::literals::chrono_literals;
    SynchronizeFrame(frame, *_episode, 1s);

    return frame;
  }

  // ===========================================================================
  // -- AI ---------------------------------------------------------------------
  // ===========================================================================

  std::shared_ptr<WalkerNavigation> Simulator::GetNavigation() {
    DEBUG_ASSERT(_episode != nullptr);// 确保_episode不为空
    auto nav = _episode->CreateNavigationIfMissing();// 获取导航实例，如果没有，则创建一个
    return nav;
  }

  // 行人导航的节拍
  void Simulator::NavigationTick() {
    DEBUG_ASSERT(_episode != nullptr);
    auto nav = _episode->CreateNavigationIfMissing();
    nav->Tick(_episode);// 调用导航实例的Tick方法，传递_episode作为参数
  }

  void Simulator::RegisterAIController(const WalkerAIController &controller) {
    auto walker = controller.GetParent();
    if (walker == nullptr) { // 获取AI控制器所控制的行人对象
      throw_exception(std::runtime_error(controller.GetDisplayId() + ": not attached to walker"));
      return;
    }
    DEBUG_ASSERT(_episode != nullptr);// 确保_episode不为空
    auto nav = _episode->CreateNavigationIfMissing();// 获取导航实例，如果没有则创建
    nav->RegisterWalker(walker->GetId(), controller.GetId());// 注册该控制器和对应的行人ID到导航系统
  }

  void Simulator::UnregisterAIController(const WalkerAIController &controller) {
    auto walker = controller.GetParent();
    if (walker == nullptr) {// 如果行人对象为空，则抛出异常
      throw_exception(std::runtime_error(controller.GetDisplayId() + ": not attached to walker"));
      return;
    }
    DEBUG_ASSERT(_episode != nullptr);
    auto nav = _episode->CreateNavigationIfMissing();// 获取导航实例，如果没有则创建
    nav->UnregisterWalker(walker->GetId(), controller.GetId());// 从导航系统注销该控制器和对应的行人ID
  }

  boost::optional<geom::Location> Simulator::GetRandomLocationFromNavigation() {
    DEBUG_ASSERT(_episode != nullptr);
    auto nav = _episode->CreateNavigationIfMissing();
    return nav->GetRandomLocation();// 从导航中获取一个随机位置
  }

  void Simulator::SetPedestriansCrossFactor(float percentage) {
    DEBUG_ASSERT(_episode != nullptr);
    auto nav = _episode->CreateNavigationIfMissing();
    nav->SetPedestriansCrossFactor(percentage);// 设置行人穿越系数
  }

  void Simulator::SetPedestriansSeed(unsigned int seed) {
    DEBUG_ASSERT(_episode != nullptr);
    auto nav = _episode->CreateNavigationIfMissing();
    nav->SetPedestriansSeed(seed);// 设置行人种子值，用于随机生成行人的位置等
  }

  // ===========================================================================
  // -- 参与者的一般操作 --------------------------------------------------------
  // ===========================================================================

    // 生成参与者
    SharedPtr<Actor> Simulator::SpawnActor(
      const ActorBlueprint &blueprint,
      const geom::Transform &transform,
      Actor *parent,
      rpc::AttachmentType attachment_type,
      GarbageCollectionPolicy gc,
      const std::string& socket_name) {
    rpc::Actor actor;
    // 如果指定了父Actor，则调用带父Actor的SpawnActor方法
    if (parent != nullptr) {
      actor = _client.SpawnActorWithParent(
          blueprint.MakeActorDescription(),
          transform,
          parent->GetId(),
          attachment_type,
          socket_name);
    } else {
        // 否则，调用不带父Actor的SpawnActor方法
      actor = _client.SpawnActor(
          blueprint.MakeActorDescription(),
          transform);
    }
    // 确保_episode不为空
    DEBUG_ASSERT(_episode != nullptr);
    // 将生成的Actor注册到当前Episode中
    _episode->RegisterActor(actor);
    // 如果垃圾回收策略是继承，则使用当前Episode的垃圾回收策略，否则使用传入的gc策略
    const auto gca = (gc == GarbageCollectionPolicy::Inherit ? _gc_policy : gc);
    // 使用工厂方法创建Actor对象并返回
    auto result = ActorFactory::MakeActor(GetCurrentEpisode(), actor, gca);
    // 记录日志，显示生成Actor的ID和是否启用了垃圾回收
    log_debug(
        result->GetDisplayId(),
        "created",
        gca == GarbageCollectionPolicy::Enabled ? "with" : "without",
        "garbage collection");
    return result;
  }

// DestroyActor函数用于销毁一个给定的Actor。
// 它会从客户端销毁Actor并清除Actor的持久状态，确保该Actor无法再访问客户端。
  bool Simulator::DestroyActor(Actor &actor) {
    bool success = true;
    success = _client.DestroyActor(actor.GetId());
    if (success) {
      // Remove it's persistent state so it cannot access the client anymore.
      actor.GetEpisode().Clear();
      log_debug(actor.GetDisplayId(), "destroyed.");
    } else {
      log_debug("failed to destroy", actor.GetDisplayId());
    }
    return success;
  }

  // ===========================================================================
  // -- 传感器的操作 ------------------------------------------------------------
  // ===========================================================================

  // Simulator 类的一个成员函数，用于订阅传感器的数据
  void Simulator::SubscribeToSensor(
      const Sensor &sensor, //引用传递，表示要订阅的传感器对象
      std::function<void(SharedPtr<sensor::SensorData>)> callback) {
    DEBUG_ASSERT(_episode != nullptr);
    _client.SubscribeToStream(
        sensor.GetActorDescription().GetStreamToken(),
        [cb=std::move(callback), ep=WeakEpisodeProxy{shared_from_this()}](auto buffer) {
          auto data = sensor::Deserializer::Deserialize(std::move(buffer));
          data->_episode = ep.TryLock();
          cb(std::move(data));
        });
  }
  // 取消订阅传感器的数据
  void Simulator::UnSubscribeFromSensor(Actor &sensor) {
    _client.UnSubscribeFromStream(sensor.GetActorDescription().GetStreamToken());
    // 如果将来我们需要单独取消订阅每个 gbuffer，则应该在这里完成。
  }
  // 为ROS启用传感器数据
  void Simulator::EnableForROS(const Sensor &sensor) {
    _client.EnableForROS(sensor.GetActorDescription().GetStreamToken());
  }
  // 为ROS禁用传感器数据
  void Simulator::DisableForROS(const Sensor &sensor) {
    _client.DisableForROS(sensor.GetActorDescription().GetStreamToken());
  }
  // 检查传感器是否为ROS启用
  bool Simulator::IsEnabledForROS(const Sensor &sensor) {
    return _client.IsEnabledForROS(sensor.GetActorDescription().GetStreamToken());
  }
  // 订阅GBuffer（一种图形缓冲区）数据
  void Simulator::SubscribeToGBuffer(
      Actor &actor,
      uint32_t gbuffer_id,
      std::function<void(SharedPtr<sensor::SensorData>)> callback) {
    _client.SubscribeToGBuffer(actor.GetId(), gbuffer_id,
        [cb=std::move(callback), ep=WeakEpisodeProxy{shared_from_this()}](auto buffer) {
          auto data = sensor::Deserializer::Deserialize(std::move(buffer));
          data->_episode = ep.TryLock();
          cb(std::move(data));
        });
  }
  // 取消订阅GBuffer数据
  void Simulator::UnSubscribeFromGBuffer(Actor &actor, uint32_t gbuffer_id) {
    _client.UnSubscribeFromGBuffer(actor.GetId(), gbuffer_id);
  }

  // 冻结或解冻所有交通信号灯
  void Simulator::FreezeAllTrafficLights(bool frozen) {
    _client.FreezeAllTrafficLights(frozen);// 传递冻结状态给客户端
  }
  // 向传感器发送消息
  void Simulator::Send(const Sensor &sensor, std::string message) {
    _client.Send(sensor.GetId(), message);
  }

  // =========================================================================
  /// -- 纹理更新操作
  // =========================================================================

// 应用颜色纹理到一组对象上，使用TextureColor类型
  void Simulator::ApplyColorTextureToObjects(
      const std::vector<std::string> &objects_name, // 对象名称的列表
      const rpc::MaterialParameter& parameter,// 材质参数
      const rpc::TextureColor& Texture) {// 颜色纹理
    _client.ApplyColorTextureToObjects(objects_name, parameter, Texture);
  }
  // 应用颜色纹理到一组对象上，使用TextureFloatColor类型
  void Simulator::ApplyColorTextureToObjects(
      const std::vector<std::string> &objects_name,
      const rpc::MaterialParameter& parameter,
      const rpc::TextureFloatColor& Texture) {
    _client.ApplyColorTextureToObjects(objects_name, parameter, Texture);
  }
  // 获取场景中所有对象的名称列表
  std::vector<std::string> Simulator::GetNamesOfAllObjects() const {
    return _client.GetNamesOfAllObjects();
  }

} // namespace detail
} // namespace client
} // namespace carla
