﻿// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 参考PythonAPI：https://openhutb.github.io/carla_doc/python_api/
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/detail/Simulator.h" // 引入Simulator相关的头文件，用于控制和模拟仿真环境中的车辆、传感器等。
#include "carla/client/World.h" // 引入World相关的头文件，提供了对CARLA模拟世界的访问接口，包括控制和交互等功能。
#include "carla/client/Map.h" // 引入Map相关的头文件，用于访问和操作CARLA世界中的地图。
#include "carla/PythonUtil.h" // 引入PythonUtil头文件，这通常用于将C++与Python代码交互或提供Python脚本的调用接口。
#include "carla/trafficmanager/TrafficManager.h" // 引入TrafficManager相关的头文件，用于管理交通流量并控制自动驾驶行为。

namespace carla {
namespace client {

  using namespace carla::traffic_manager;

  class Client {
  public:

    /// 构建一个 carla 客户端。
    ///
    /// @param host 运行模拟器的主机IP地址。
    /// @param port 连接到模拟器的TCP端口。
    /// @param worker_threads 要使用的异步线程数，默认为 0（即使用所有可用的硬件并发线程）。
    explicit Client(
        const std::string &host,
        uint16_t port,
        size_t worker_threads = 0u);

    /// 设置网络操作的超时时间。超时将抛出 rpc::timeout 异常
    void SetTimeout(time_duration timeout) {
      _simulator->SetNetworkingTimeout(timeout);
    }

    // 获取当前客户端的网络超时时间。
    time_duration GetTimeout() {
      return _simulator->GetNetworkingTimeout();
    }

    /// 返回此客户端 API 版本的字符串。
    std::string GetClientVersion() const {
      return _simulator->GetClientVersion();
    }

    ///  获取当前连接的模拟器版本字符串。
    std::string GetServerVersion() const {
      return _simulator->GetServerVersion();
    }

    /// 获取当前服务中心提供的所有可用地图名称。
    std::vector<std::string> GetAvailableMaps() const {
      return _simulator->GetAvailableMaps();
    }

    /// 设置文件系统的基路径（文件夹）。 
    /// 用于设置模拟器读取文件的路径。
    /// 返回操作是否成功。
    bool SetFilesBaseFolder(const std::string &path) {
      return _simulator->SetFilesBaseFolder(path);
    }
    /// 获取指定文件夹中的所需文件列表，默认情况下会下载所需文件。
    /// 如果文件夹路径为空，则使用默认路径。
    std::vector<std::string> GetRequiredFiles(const std::string &folder = "", const bool download = true) const {
      return _simulator->GetRequiredFiles(folder, download);
    }
    /// 请求指定名称的文件，模拟器会处理文件的加载或下载。
    void RequestFile(const std::string &name) const {
      _simulator->RequestFile(name);
    }
    /// 重新加载世界环境。 
    /// 通过调用 _simulator->ReloadEpisode 来实现，是否重置设置由参数决定。
    /// 返回重载后的世界对象。
    World ReloadWorld(bool reset_settings = true) const {
      return World{_simulator->ReloadEpisode(reset_settings)};
    }

    /// 加载指定名称的场景地图。
    /// @param map_name 地图名称。
    /// @param reset_settings 是否重置设置。
    /// @param map_layers 需要加载的地图层。
    World LoadWorld(
        std::string map_name,           // 地图名
        bool reset_settings = true,
        // 实际调用 _simulator->LoadEpisode 返回的场景
        rpc::MapLayer map_layers = rpc::MapLayer::All) const {
      return World{_simulator->LoadEpisode(std::move(map_name), reset_settings, map_layers)};
    }

    /// 如果当前地图与请求地图不同，则加载新地图。
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
      // 仅当当前地图不同于目标地图时，才重新加载世界
      if(!(map_name_without_prefix == current_map_name) && !(map_name_with_prefix == current_map_name)){
        World World{_simulator->LoadEpisode(std::move(map_name), reset_settings, map_layers)};
      }else{}
    }
    
    /// 通过 OpenDrive 文件生成一个新的世界。
    /// @param opendrive OpenDrive 格式的文件路径。
    /// @param params 生成 OpenDrive 世界时的参数。
    /// @param reset_settings 是否重置设置。
    World GenerateOpenDriveWorld(
        std::string opendrive,
        const rpc::OpendriveGenerationParameters & params,
        bool reset_settings = true) const {
      return World{_simulator->LoadOpenDriveEpisode(
          std::move(opendrive), params, reset_settings)};
    }

    /// 获取当前模拟器中的活跃世界实例。
    World GetWorld() const {
      return World{_simulator->GetCurrentEpisode()};
    }

    /// 获取当前模拟器中的 TrafficManager 实例。
    /// @param port TrafficManager 使用的端口，默认为 TM_DEFAULT_PORT。
    TrafficManager GetInstanceTM(uint16_t port = TM_DEFAULT_PORT) const {
      return TrafficManager(_simulator->GetCurrentEpisode(), port);
    }

    /// 获取当前模拟器中的活动场景实例。
    carla::client::detail::EpisodeProxy GetCurrentEpisode() const {
      return _simulator->GetCurrentEpisode();
    }

    // 启动录像功能，开始记录服务器重放仿真所需的所有信息。
    std::string StartRecorder(std::string name, bool additional_data = false) {
      return _simulator->StartRecorder(name, additional_data);
    }

    // 停止记录并保存日志数据。
    void StopRecorder(void) {
      _simulator->StopRecorder();
    }

    // 显示指定记录文件的详细信息。
    /// @param name 文件名。
    /// @param show_all 是否显示所有记录信息。
    std::string ShowRecorderFileInfo(std::string name, bool show_all) {
      return _simulator->ShowRecorderFileInfo(name, show_all);
    }

    // 在终端中显示记录器记录的碰撞。
    // 可以通过指定所涉及参与者的类型来过滤这些内容。
    // 类别将在`type1`和`type2`中指定：
    // `h`表示英雄参与者，一种可以手动控制或由用户管理的车辆。
    // `v`表示车辆，`w`表示行人，`t`表示红绿灯，`o`表示其他，`a`表示所有。
    // 如果您只想看到车辆和行人之间的碰撞，请将`category1`设置为`v`，将`category2`设置为`w`，反之亦然。
    // 如果要查看所有碰撞（过滤掉），可以对两个参数都使用`a`。
    std::string ShowRecorderCollisions(std::string name, char type1, char type2) {
      return _simulator->ShowRecorderCollisions(name, type1, type2);
    }

    // 在终端中显示视为被堵塞的参与者注册信息。
    // 当参与者在一段时间 min_time 内没有移动最小距离 min_distance 时，则视为被堵塞。
    std::string ShowRecorderActorsBlocked(std::string name, double min_time, double min_distance) {
      return _simulator->ShowRecorderActorsBlocked(name, min_time, min_distance);
    }

    // 解析记录器保存的信息将并以文本形式显示在终端中（帧、时间、事件、状态、位置…）。
    std::string ReplayFile(std::string name, double start, double duration,
        uint32_t follow_id, bool replay_sensors) {
      return _simulator->ReplayFile(name, start, duration, follow_id, replay_sensors);
    }

    // 停止当前的重放过程。
    void StopReplayer(bool keep_actors) {
      _simulator->StopReplayer(keep_actors);
    }

    // 使用时，会随意修改重新模拟的时间速度。当播放时，它可以使用多次。
    // 1.0表示正常时间速度。大于1.0表示快速运动（2.0表示双倍速度），小于1.0表示慢速运动（0.5表示一半速度）。
    void SetReplayerTimeFactor(double time_factor) {
      _simulator->SetReplayerTimeFactor(time_factor);
    }

    // ignore_hero：在播放记录的模拟过程中启用或禁用英雄车辆的播放。
    void SetReplayerIgnoreHero(bool ignore_hero) {
      _simulator->SetReplayerIgnoreHero(ignore_hero);
    }

    // 确定回放程序是否会复制记录的观察者运动。
    void SetReplayerIgnoreSpectator(bool ignore_spectator) {
      _simulator->SetReplayerIgnoreSpectator(ignore_spectator);
    }

    // 在单个模拟步上执行命令列表，不检索任何信息。
    void ApplyBatch(
        std::vector<rpc::Command> commands,
        bool do_tick_cue = false) const {
      _simulator->ApplyBatch(std::move(commands), do_tick_cue);
    }

    // 在单个模拟步上阻塞式地执行命令列表，直到命令链接起来，并返回 command.Response 列表。
    // 可用于确定单个命令是否成功的响应。
    std::vector<rpc::CommandResponse> ApplyBatchSync(
        std::vector<rpc::Command> commands,  // 要批量执行的命令列表。
        bool do_tick_cue = false) const {  // 是否在执行后触发 tick 事件。
      auto responses = _simulator->ApplyBatchSync(std::move(commands), false);
      if (do_tick_cue)
        _simulator->Tick(_simulator->GetNetworkingTimeout());

      return responses;  // 返回所有命令的响应列表。
    }

  private:
  
    // 当前仿真器实例的智能指针，用于管理仿真器的生命周期。
    std::shared_ptr<detail::Simulator> _simulator;  
  };

  // Client 构造函数，初始化与仿真器的连接。
  inline Client::Client(
      const std::string &host,  // 仿真器主机地址。
      uint16_t port,  // 仿真器端口号。
      size_t worker_threads)  // 使用的工作线程数量。
    : _simulator(
        new detail::Simulator(host, port, worker_threads),
        PythonUtil::ReleaseGILDeleter()) {}  // 初始化仿真器并传递 GIL 解锁器以支持 Python 集成。

} // namespace client
} // namespace carla
