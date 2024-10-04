// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <thread>
// 引入线程库

#include "carla/client/detail/Simulator.h"
// 引入 Carla 客户端的模拟器实现细节头文件

#include "carla/trafficmanager/TrafficManagerRemote.h"
// 引入 Carla 的远程交通管理器头文件

namespace carla {
namespace traffic_manager {
// 命名空间 carla 和 traffic_manager

TrafficManagerRemote::TrafficManagerRemote(
    const std::pair<std::string, uint16_t> &_serverTM,
    carla::client::detail::EpisodeProxy &episodeProxy)
  : client(_serverTM.first, _serverTM.second),
    episodeProxyTM(episodeProxy) {
// 远程交通管理器的构造函数，接收服务器地址对和场景代理对象，初始化成员变量

  Start();
// 启动远程交通管理器
}

/// Destructor.
TrafficManagerRemote::~TrafficManagerRemote() {
  Release();
// 析构函数，释放资源
}

void TrafficManagerRemote::Start() {
  _keep_alive = true;
// 设置存活标志为真

  std::thread _thread = std::thread([this] () {
// 创建一个新线程

    std::chrono::milliseconds wait_time(TM_TIMEOUT);
// 设置等待时间

    try {
      do {
        std::this_thread::sleep_for(wait_time);
// 线程睡眠一段时间

        client.HealthCheckRemoteTM();
// 对远程交通管理器进行健康检查

        /// Until connection active
      } while (_keep_alive);
// 当存活标志为真时循环
    } catch (...) {

      std::string rhost("");
      uint16_t rport = 0;
// 初始化远程主机地址和端口

      client.getServerDetails(rhost, rport);
// 获取服务器详细信息

      std::string strtmserver(rhost + ":" + std::to_string(rport));
// 构建服务器地址字符串

      /// Create error msg
      std::string errmsg(
          "Trying to connect rpc server of traffic manager; "
          "but the system failed to connect at " + strtmserver);
// 创建错误消息

      /// TSet the error message
      if(_keep_alive) {
        this->episodeProxyTM.Lock()->AddPendingException(errmsg);
// 如果存活标志为真，将错误消息添加到场景代理的待处理异常中
      }
    }
    _keep_alive = false;
    _cv.notify_one();
// 设置存活标志为假，并通知条件变量
  });

  _thread.detach();
// 分离线程
}

void TrafficManagerRemote::Stop() {
  if(_keep_alive) {
    _keep_alive = false;
    std::unique_lock<std::mutex> lock(_mutex);
    std::chrono::milliseconds wait_time(TM_TIMEOUT + 1000);
    _cv.wait_for(lock, wait_time);
// 如果存活标志为真，设置存活标志为假，加锁互斥量，等待条件变量一段时间
  }
}

void TrafficManagerRemote::Release() {
  Stop();
// 释放资源，调用停止函数
}

void TrafficManagerRemote::Reset() {
  Stop();
// 停止远程交通管理器

  carla::client::detail::EpisodeProxy episode_proxy = episodeProxyTM.Lock()->GetCurrentEpisode();
  episodeProxyTM = episode_proxy;
// 获取当前场景代理，并更新成员变量

  Start();
// 重新启动远程交通管理器
}

void TrafficManagerRemote::RegisterVehicles(const std::vector<ActorPtr> &_actor_list) {
  std::vector<carla::rpc::Actor> actor_list;
  for (auto &&actor : _actor_list) {
    actor_list.emplace_back(actor->Serialize());
// 将输入的车辆列表转换为 rpc 格式的车辆列表
  }
  client.RegisterVehicle(actor_list);
// 通过客户端注册车辆
}

void TrafficManagerRemote::UnregisterVehicles(const std::vector<ActorPtr> &_actor_list) {
  std::vector<carla::rpc::Actor> actor_list;
  for (auto &&actor : _actor_list) {
    actor_list.emplace_back(actor->Serialize());
// 将输入的车辆列表转换为 rpc 格式的车辆列表
  }
  client.UnregisterVehicle(actor_list);
// 通过客户端注销车辆
}

void TrafficManagerRemote::SetPercentageSpeedDifference(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetPercentageSpeedDifference(actor, percentage);
// 通过客户端设置车辆的速度差异百分比
}

void TrafficManagerRemote::SetDesiredSpeed(const ActorPtr &_actor, const float value) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetDesiredSpeed(actor, value);
// 通过客户端设置车辆的期望速度
}

void TrafficManagerRemote::SetGlobalPercentageSpeedDifference(const float percentage) {
  client.SetGlobalPercentageSpeedDifference(percentage);
// 通过客户端设置全局速度差异百分比
}

void TrafficManagerRemote::SetLaneOffset(const ActorPtr &_actor, const float offset) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetLaneOffset(actor, offset);
// 通过客户端设置车辆的车道偏移量
}

void TrafficManagerRemote::SetGlobalLaneOffset(const float offset) {
  client.SetGlobalLaneOffset(offset);
// 通过客户端设置全局车道偏移量
}

void TrafficManagerRemote::SetUpdateVehicleLights(const ActorPtr &_actor, const bool do_update) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetUpdateVehicleLights(actor, do_update);
// 通过客户端设置车辆的灯光更新状态
}

void TrafficManagerRemote::SetCollisionDetection(const ActorPtr &_reference_actor, const ActorPtr &_other_actor, const bool detect_collision) {
  carla::rpc::Actor reference_actor(_reference_actor->Serialize());
  carla::rpc::Actor other_actor(_other_actor->Serialize());
// 将输入的两辆车辆转换为 rpc 格式的车辆

  client.SetCollisionDetection(reference_actor, other_actor, detect_collision);
// 通过客户端设置两辆车辆之间的碰撞检测状态
}

void TrafficManagerRemote::SetForceLaneChange(const ActorPtr &_actor, const bool direction) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetForceLaneChange(actor, direction);
// 通过客户端设置车辆的强制换道方向
}

void TrafficManagerRemote::SetAutoLaneChange(const ActorPtr &_actor, const bool enable) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetAutoLaneChange(actor, enable);
// 通过客户端设置车辆的自动换道状态
}

void TrafficManagerRemote::SetDistanceToLeadingVehicle(const ActorPtr &_actor, const float distance) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetDistanceToLeadingVehicle(actor, distance);
// 通过客户端设置车辆与前车的距离
}

void TrafficManagerRemote::SetGlobalDistanceToLeadingVehicle(const float distance) {
  client.SetGlobalDistanceToLeadingVehicle(distance);
// 通过客户端设置全局与前车的距离
}


void TrafficManagerRemote::SetPercentageIgnoreWalkers(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetPercentageIgnoreWalkers(actor, percentage);
// 通过客户端设置车辆忽略行人的百分比
}

void TrafficManagerRemote::SetPercentageIgnoreVehicles(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetPercentageIgnoreVehicles(actor, percentage);
// 通过客户端设置车辆忽略其他车辆的百分比
}

void TrafficManagerRemote::SetPercentageRunningLight(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetPercentageRunningLight(actor, percentage);
// 通过客户端设置车辆运行灯光的百分比
}

void TrafficManagerRemote::SetPercentageRunningSign(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetPercentageRunningSign(actor, percentage);
// 通过客户端设置车辆运行标志的百分比
}

void TrafficManagerRemote::SetKeepRightPercentage(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetKeepRightPercentage(actor, percentage);
// 通过客户端设置车辆靠右行驶的百分比
}

void TrafficManagerRemote::SetRandomLeftLaneChangePercentage(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetRandomLeftLaneChangePercentage(actor, percentage);
// 通过客户端设置车辆随机向左换道的百分比
}

void TrafficManagerRemote::SetRandomRightLaneChangePercentage(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetRandomRightLaneChangePercentage(actor, percentage);
// 通过客户端设置车辆随机向右换道的百分比
}

void TrafficManagerRemote::SetHybridPhysicsMode(const bool mode_switch) {
  client.SetHybridPhysicsMode(mode_switch);
// 通过客户端设置混合物理模式开关
}

void TrafficManagerRemote::SetHybridPhysicsRadius(const float radius) {
  client.SetHybridPhysicsRadius(radius);
// 通过客户端设置混合物理模式半径
}

void TrafficManagerRemote::SetOSMMode(const bool mode_switch) {
  client.SetOSMMode(mode_switch);
// 通过客户端设置 OSM 模式开关
}

void TrafficManagerRemote::SetCustomPath(const ActorPtr &_actor, const Path path, const bool empty_buffer) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetCustomPath(actor, path, empty_buffer);
// 通过客户端为车辆设置自定义路径
}

void TrafficManagerRemote::RemoveUploadPath(const ActorId &actor_id, const bool remove_path) {
  client.RemoveUploadPath(actor_id, remove_path);
// 通过客户端移除车辆的上传路径
}

void TrafficManagerRemote::UpdateUploadPath(const ActorId &actor_id, const Path path) {
  client.UpdateUploadPath(actor_id, path);
// 通过客户端更新车辆的上传路径
}

void TrafficManagerRemote::SetImportedRoute(const ActorPtr &_actor, const Route route, const bool empty_buffer) {
  carla::rpc::Actor actor(_actor->Serialize());
// 将输入的车辆转换为 rpc 格式的车辆

  client.SetImportedRoute(actor, route, empty_buffer);
// 通过客户端为车辆设置导入的路线
}

void TrafficManagerRemote::RemoveImportedRoute(const ActorId &actor_id, const bool remove_path) {
  client.RemoveImportedRoute(actor_id, remove_path);
// 通过客户端移除车辆的导入路线
}

void TrafficManagerRemote::UpdateImportedRoute(const ActorId &actor_id, const Route route) {
  client.UpdateImportedRoute(actor_id, route);
// 通过客户端更新车辆的导入路线
}

void TrafficManagerRemote::SetRespawnDormantVehicles(const bool mode_switch) {
  client.SetRespawnDormantVehicles(mode_switch);
// 通过客户端设置是否复活休眠车辆
}

void TrafficManagerRemote::SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) {
  client.SetBoundariesRespawnDormantVehicles(lower_bound, upper_bound);
// 通过客户端设置复活休眠车辆的边界
}

void TrafficManagerRemote::SetMaxBoundaries(const float lower, const float upper) {
  client.SetMaxBoundaries(lower, upper);
// 通过客户端设置最大边界
}

void TrafficManagerRemote::ShutDown() {
  client.ShutDown();
// 通过客户端关闭远程交通管理器
}

void TrafficManagerRemote::SetSynchronousMode(bool mode) {
  client.SetSynchronousMode(mode);
// 通过客户端设置同步模式开关
}

void TrafficManagerRemote::SetSynchronousModeTimeOutInMiliSecond(double time) {
  client.SetSynchronousModeTimeOutInMiliSecond(time);
// 通过客户端设置同步模式超时时间（毫秒）
}

Action TrafficManagerRemote::GetNextAction(const ActorId &actor_id) {
  return client.GetNextAction(actor_id);
// 通过客户端获取指定车辆的下一个动作
}

ActionBuffer TrafficManagerRemote::GetActionBuffer(const ActorId &actor_id) {
  return client.GetActionBuffer(actor_id);
// 通过客户端获取指定车辆的动作缓冲区
}

bool TrafficManagerRemote::SynchronousTick() {
  return false;
// 返回假，表示同步时钟滴答函数未实现
}

void TrafficManagerRemote::HealthCheckRemoteTM() {
  client.HealthCheckRemoteTM();
// 通过客户端对远程交通管理器进行健康检查
}

carla::client::detail::EpisodeProxy& TrafficManagerRemote::GetEpisodeProxy() {
  return episodeProxyTM;
// 返回场景代理对象的引用
}

void TrafficManagerRemote::SetRandomDeviceSeed(const uint64_t seed) {
  client.SetRandomDeviceSeed(seed);
// 通过客户端设置随机数生成器的种子
}

} // namespace traffic_manager
} // namespace carla