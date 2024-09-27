// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/detail/Simulator.h"
#include "carla/client/World.h"
#include "carla/client/Map.h"
#include "carla/PythonUtil.h"
#include "carla/trafficmanager/TrafficManager.h"

namespace carla {
namespace client {

  using namespace carla::traffic_manager;

  class Client {
  public:

    /// 构建一个 carla 客户端。
    ///
    /// @param host 运行模拟器的主机IP地址。
    /// @param port 连接到模拟器的TCP端口。
    /// @param worker_threads 要使用的异步线程数，或 0 以使用所有可用的硬件并发。
    explicit Client(
        const std::string &host,
        uint16_t port,
        size_t worker_threads = 0u);

    /// 设置网络操作的超时时间。如果设置，任何超过 @a 超时时间的网络操作都会抛出 rpc::timeout 异常。 
    void SetTimeout(time_duration timeout) {
      _simulator->SetNetworkingTimeout(timeout);
    }

    // 获取当前客户端的超时时间
    time_duration GetTimeout() {
      return _simulator->GetNetworkingTimeout();
    }

    /// 返回此客户端 API 版本的字符串。
    std::string GetClientVersion() const {
      return _simulator->GetClientVersion();
    }

    /// 返回我们连接的模拟器版本的字符串。
    std::string GetServerVersion() const {
      return _simulator->GetServerVersion();
    }

    // 获得当前连接服务中心所有可用的地图
    std::vector<std::string> GetAvailableMaps() const {
      return _simulator->GetAvailableMaps();
    }

    bool SetFilesBaseFolder(const std::string &path) {
      return _simulator->SetFilesBaseFolder(path);
    }

    std::vector<std::string> GetRequiredFiles(const std::string &folder = "", const bool download = true) const {
      return _simulator->GetRequiredFiles(folder, download);
    }

    void RequestFile(const std::string &name) const {
      _simulator->RequestFile(name);
    }

    World ReloadWorld(bool reset_settings = true) const {
      return World{_simulator->ReloadEpisode(reset_settings)};
    }

    // 加载场景世界
    World LoadWorld(
        std::string map_name,           // 地图名
        bool reset_settings = true,
        // 实际调用 _simulator->LoadEpisode 返回的场景
        rpc::MapLayer map_layers = rpc::MapLayer::All) const {
      return World{_simulator->LoadEpisode(std::move(map_name), reset_settings, map_layers)};
    }

    /// 仅当请求的地图与当前地图不同时才返回（并加载）一个新的世界（地图）

    void LoadWorldIfDifferent(
        std::string map_name,
        bool reset_settings = true,
        rpc::MapLayer map_layers = rpc::MapLayer::All) const {
      carla::client::World world = GetWorld();
      carla::SharedPtr<carla::client::Map> current_map = world.GetMap();
      std::string current_map_name = current_map->GetName();
      std::string map_name_prefix = "Carla/Maps/";
      std::string map_name_without_prefix = map_name;
      std::string map_name_with_prefix = map_name_prefix + map_name;
      if(!(map_name_without_prefix == current_map_name) && !(map_name_with_prefix == current_map_name)){
        World World{_simulator->LoadEpisode(std::move(map_name), reset_settings, map_layers)};
      }else{}
    }
    
    World GenerateOpenDriveWorld(
        std::string opendrive,
        const rpc::OpendriveGenerationParameters & params,
        bool reset_settings = true) const {
      return World{_simulator->LoadOpenDriveEpisode(
          std::move(opendrive), params, reset_settings)};
    }

    /// 返回模拟器中当前活跃世界的一个实例。
    World GetWorld() const {
      return World{_simulator->GetCurrentEpisode()};
    }

    /// 返回模拟器中当前活动的 TrafficManager 实例。
    TrafficManager GetInstanceTM(uint16_t port = TM_DEFAULT_PORT) const {
      return TrafficManager(_simulator->GetCurrentEpisode(), port);
    }

    /// Return an instance of the Episode currently active in the simulator.
    carla::client::detail::EpisodeProxy GetCurrentEpisode() const {
      return _simulator->GetCurrentEpisode();
    }

    std::string StartRecorder(std::string name, bool additional_data = false) {
      return _simulator->StartRecorder(name, additional_data);
    }

    void StopRecorder(void) {
      _simulator->StopRecorder();
    }

    std::string ShowRecorderFileInfo(std::string name, bool show_all) {
      return _simulator->ShowRecorderFileInfo(name, show_all);
    }

    std::string ShowRecorderCollisions(std::string name, char type1, char type2) {
      return _simulator->ShowRecorderCollisions(name, type1, type2);
    }

    std::string ShowRecorderActorsBlocked(std::string name, double min_time, double min_distance) {
      return _simulator->ShowRecorderActorsBlocked(name, min_time, min_distance);
    }

    std::string ReplayFile(std::string name, double start, double duration,
        uint32_t follow_id, bool replay_sensors) {
      return _simulator->ReplayFile(name, start, duration, follow_id, replay_sensors);
    }

    void StopReplayer(bool keep_actors) {
      _simulator->StopReplayer(keep_actors);
    }

    void SetReplayerTimeFactor(double time_factor) {
      _simulator->SetReplayerTimeFactor(time_factor);
    }

    void SetReplayerIgnoreHero(bool ignore_hero) {
      _simulator->SetReplayerIgnoreHero(ignore_hero);
    }

    void SetReplayerIgnoreSpectator(bool ignore_spectator) {
      _simulator->SetReplayerIgnoreSpectator(ignore_spectator);
    }

    void ApplyBatch(
        std::vector<rpc::Command> commands,
        bool do_tick_cue = false) const {
      _simulator->ApplyBatch(std::move(commands), do_tick_cue);
    }

    std::vector<rpc::CommandResponse> ApplyBatchSync(
        std::vector<rpc::Command> commands,
        bool do_tick_cue = false) const {
      auto responses = _simulator->ApplyBatchSync(std::move(commands), false);
      if (do_tick_cue)
        _simulator->Tick(_simulator->GetNetworkingTimeout());

      return responses;
    }

  private:

    std::shared_ptr<detail::Simulator> _simulator;  // 当前仿真器的智能指针
  };

  inline Client::Client(
      const std::string &host,
      uint16_t port,
      size_t worker_threads)
    : _simulator(
        new detail::Simulator(host, port, worker_threads),
        PythonUtil::ReleaseGILDeleter()) {}

} // namespace client
} // namespace carla
