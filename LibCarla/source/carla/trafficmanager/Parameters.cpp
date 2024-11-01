// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/Parameters.h"  // 引入参数头文件
#include "carla/trafficmanager/Constants.h"  // 引入常量头文件

namespace carla {
namespace traffic_manager {

Parameters::Parameters() {  // 参数构造函数

  /// 设置默认的同步模式超时。
  synchronous_time_out = std::chrono::duration<int, std::milli>(10);
}

Parameters::~Parameters() {}  // 参数析构函数

//////////////////////////////////// SETTERS //////////////////////////////////

void Parameters::SetHybridPhysicsMode(const bool mode_switch) {  // 设置混合物理模式
  hybrid_physics_mode.store(mode_switch);  // 存储模式开关状态
}

void Parameters::SetRespawnDormantVehicles(const bool mode_switch) {  // 设置重生休眠车辆
  respawn_dormant_vehicles.store(mode_switch);  // 存储重生休眠车辆的状态
}

void Parameters::SetMaxBoundaries(const float lower, const float upper) {  // 设置最大边界
  min_lower_bound = lower;  // 设置最小下边界
  max_upper_bound = upper;  // 设置最大上边界
}

void Parameters::SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) {  // 设置休眠车辆重生边界
  respawn_lower_bound = min_lower_bound > lower_bound ? min_lower_bound : lower_bound;  // 确定重生下边界
  respawn_upper_bound = max_upper_bound < upper_bound ? max_upper_bound : upper_bound;  // 确定重生上边界
}

void Parameters::SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) {  // 设置速度差百分比
  float new_percentage = std::min(100.0f, percentage);  // 限制最大百分比为100
  percentage_difference_from_speed_limit.AddEntry({actor->GetId(), new_percentage});  // 添加速度差记录
  if (exact_desired_speed.Contains(actor->GetId())) {  // 如果参与者的精确期望速度存在
    exact_desired_speed.RemoveEntry(actor->GetId());  // 移除该参与者的精确期望速度
  }
}

void Parameters::SetLaneOffset(const ActorPtr &actor, const float offset) {  // 设置车道偏移
  const auto entry = std::make_pair(actor->GetId(), offset);  // 创建参与者ID和偏移的条目
  lane_offset.AddEntry(entry);  // 添加车道偏移记录
}

void Parameters::SetDesiredSpeed(const ActorPtr &actor, const float value) {  // 设置期望速度
  float new_value = std::max(0.0f, value);  // 确保速度不小于0
  exact_desired_speed.AddEntry({actor->GetId(), new_value});  // 添加参与者的精确期望速度
  if (percentage_difference_from_speed_limit.Contains(actor->GetId())) {  // 如果速度差记录存在
    percentage_difference_from_speed_limit.RemoveEntry(actor->GetId());  // 移除该参与者的速度差记录
  }
}

void Parameters::SetGlobalPercentageSpeedDifference(const float percentage) {  // 设置全局速度差百分比
  float new_percentage = std::min(100.0f, percentage);  // 限制最大百分比为100
  global_percentage_difference_from_limit = new_percentage;  // 设置全局速度差
}

void Parameters::SetGlobalLaneOffset(const float offset) {  // 设置全局车道偏移
  global_lane_offset = offset;  // 设置全局偏移量
}

void Parameters::SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision) {  // 设置碰撞检测
  const ActorId reference_id = reference_actor->GetId();  // 获取参考参与者的ID
  const ActorId other_id = other_actor->GetId();  // 获取其他参与者的ID

  if (detect_collision) {  // 如果需要检测碰撞
    if (ignore_collision.Contains(reference_id)) {  // 如果参考参与者在忽略碰撞列表中
      std::shared_ptr<AtomicActorSet> actor_set = ignore_collision.GetValue(reference_id);  // 获取该参与者的忽略集合
      if (actor_set->Contains(other_id)) {  // 如果其他参与者在集合中
        actor_set->Remove({other_id});  // 从集合中移除该参与者
      }
    }
  } else {  // 如果不需要检测碰撞
    if (ignore_collision.Contains(reference_id)) {  // 如果参考参与者在忽略碰撞列表中
      std::shared_ptr<AtomicActorSet> actor_set = ignore_collision.GetValue(reference_id);  // 获取该参与者的忽略集合
      if (!actor_set->Contains(other_id)) {  // 如果其他参与者不在集合中
        actor_set->Insert({other_actor});  // 将其他参与者加入集合
      }
    } else {  // 如果参考参与者不在列表中
      std::shared_ptr<AtomicActorSet> actor_set = std::make_shared<AtomicActorSet>();  // 创建新的忽略集合
      actor_set->Insert({other_actor});  // 将其他参与者加入集合
      auto entry = std::make_pair(reference_id, actor_set);  // 创建条目
      ignore_collision.AddEntry(entry);  // 添加条目到忽略碰撞列表
    }
  }
}

void Parameters::SetForceLaneChange(const ActorPtr &actor, const bool direction) {  // 设置强制变道
  const ChangeLaneInfo lane_change_info = {true, direction};  // 创建变道信息
  const auto entry = std::make_pair(actor->GetId(), lane_change_info);  // 创建参与者ID和变道信息的条目
  force_lane_change.AddEntry(entry);  // 添加变道记录
}

void Parameters::SetKeepRightPercentage(const ActorPtr &actor, const float percentage) {  // 设置保持右侧的百分比
  const auto entry = std::make_pair(actor->GetId(), percentage);  // 创建参与者ID和保持右侧百分比的条目
  perc_keep_right.AddEntry(entry);  // 添加保持右侧记录
}

void Parameters::SetRandomLeftLaneChangePercentage(const ActorPtr &actor, const float percentage) {  // 设置随机左变道的百分比
  const auto entry = std::make_pair(actor->GetId(), percentage);  // 创建参与者ID和随机左变道百分比的条目
  perc_random_left.AddEntry(entry);  // 添加随机左变道记录
}

void Parameters::SetRandomRightLaneChangePercentage(const ActorPtr &actor, const float percentage) {  // 设置随机右变道的百分比
  const auto entry = std::make_pair(actor->GetId(), percentage);  // 创建参与者ID和随机右变道百分比的条目
  perc_random_right.AddEntry(entry);  // 添加随机右变道记录
}

void Parameters::SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update) {
    // 设置车辆灯光更新状态
    const auto entry = std::make_pair(actor->GetId(), do_update);
    // 创建参与者ID和更新状态的条目
    auto_update_vehicle_lights.AddEntry(entry);
    // 将条目添加到自动更新车辆灯光列表中
}

void Parameters::SetAutoLaneChange(const ActorPtr &actor, const bool enable) {
    // 设置自动变道功能
    const auto entry = std::make_pair(actor->GetId(), enable);
    // 创建参与者ID和变道使能状态的条目
    auto_lane_change.AddEntry(entry);
    // 将条目添加到自动变道列表中
}

void Parameters::SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {
    // 设置与前车的距离
    float new_distance = std::max(0.0f, distance);
    // 确保距离不小于0
    const auto entry = std::make_pair(actor->GetId(), new_distance);
    // 创建参与者ID和距离的条目
    distance_to_leading_vehicle.AddEntry(entry);
    // 将条目添加到前车距离列表中
}

void Parameters::SetSynchronousMode(const bool mode_switch) {
    // 设置同步模式开关
    synchronous_mode.store(mode_switch);
}

void Parameters::SetSynchronousModeTimeOutInMiliSecond(const double time) {
    // 设置同步模式超时时间（毫秒）
    synchronous_time_out = std::chrono::duration<double, std::milli>(time);
}

void Parameters::SetGlobalDistanceToLeadingVehicle(const float dist) {
    // 设置全局前车距离
    distance_margin.store(dist);
}

void Parameters::SetPercentageRunningLight(const ActorPtr &actor, const float perc) {
    // 设置运行信号灯的百分比
    float new_perc = cg::Math::Clamp(perc, 0.0f, 100.0f);
    // 确保百分比在0到100之间
    const auto entry = std::make_pair(actor->GetId(), new_perc);
    // 创建参与者ID和百分比的条目
    perc_run_traffic_light.AddEntry(entry);
    // 将条目添加到运行信号灯百分比列表中
}

void Parameters::SetPercentageRunningSign(const ActorPtr &actor, const float perc) {
    // 设置运行标志的百分比
    float new_perc = cg::Math::Clamp(perc, 0.0f, 100.0f);
    const auto entry = std::make_pair(actor->GetId(), new_perc);
    perc_run_traffic_sign.AddEntry(entry);
}

void Parameters::SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc) {
    // 设置忽略车辆的百分比
    float new_perc = cg::Math::Clamp(perc, 0.0f, 100.0f);
    const auto entry = std::make_pair(actor->GetId(), new_perc);
    perc_ignore_vehicles.AddEntry(entry);
}

void Parameters::SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc) {
    // 设置忽略行人的百分比
    float new_perc = cg::Math::Clamp(perc, 0.0f, 100.0f);
    const auto entry = std::make_pair(actor->GetId(), new_perc);
    perc_ignore_walkers.AddEntry(entry);
}

void Parameters::SetHybridPhysicsRadius(const float radius) {
    // 设置混合物理半径
    float new_radius = std::max(radius, 0.0f);
    // 确保半径不小于0
    hybrid_physics_radius.store(new_radius);
}

void Parameters::SetOSMMode(const bool mode_switch) {
    // 设置开放街图模式开关
    osm_mode.store(mode_switch);
}

void Parameters::SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer) {
    // 设置参与者的自定义路径
    const auto entry = std::make_pair(actor->GetId(), path);
    custom_path.AddEntry(entry);
    // 将路径条目添加到自定义路径列表中
    const auto entry2 = std::make_pair(actor->GetId(), empty_buffer);
    upload_path.AddEntry(entry2);
    // 将空缓冲区条目添加到上传路径列表中
}

void Parameters::RemoveUploadPath(const ActorId &actor_id, const bool remove_path) {
    // 根据参数决定是删除上传路径还是自定义路径
    if (!remove_path) {
        upload_path.RemoveEntry(actor_id);
    } else {
        custom_path.RemoveEntry(actor_id);
    }
}

void Parameters::UpdateUploadPath(const ActorId &actor_id, const Path path) {
    // 更新上传路径
    custom_path.RemoveEntry(actor_id);
    // 移除旧的自定义路径条目
    const auto entry = std::make_pair(actor_id, path);
    custom_path.AddEntry(entry);
    // 添加新的自定义路径条目
}

void Parameters::SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer) {
    // 设置参与者的导入路线
    const auto entry = std::make_pair(actor->GetId(), route);
    custom_route.AddEntry(entry);
    const auto entry2 = std::make_pair(actor->GetId(), empty_buffer);
    upload_route.AddEntry(entry2);
}

void Parameters::RemoveImportedRoute(const ActorId &actor_id, const bool remove_path) {
    // 根据参数决定是删除上传路线还是自定义路线
    if (!remove_path) {
        upload_route.RemoveEntry(actor_id);
    } else {
        custom_route.RemoveEntry(actor_id);
    }
}

void Parameters::UpdateImportedRoute(const ActorId &actor_id, const Route route) {
    // 更新导入路线
    custom_route.RemoveEntry(actor_id);
    // 移除旧的自定义路线条目
    const auto entry = std::make_pair(actor_id, route);
    custom_route.AddEntry(entry);
    // 添加新的自定义路线条目
}

//////////////////////////////////// GETTERS //////////////////////////////////

float Parameters::GetHybridPhysicsRadius() const {
    // 获取混合物理半径
    return hybrid_physics_radius.load();
}

bool Parameters::GetSynchronousMode() const {
    // 获取同步模式状态
    return synchronous_mode.load();
}

double Parameters::GetSynchronousModeTimeOutInMiliSecond() const {
    // 获取同步模式超时时间（毫秒）
    return synchronous_time_out.count();
}

float Parameters::GetVehicleTargetVelocity(const ActorId &actor_id, const float speed_limit) const {
    // 从全局获取参与者与速度限制的百分比差异
    float percentage_difference = global_percentage_difference_from_limit;

    // 如果参与者的速度限制包含在内，获取其特定的百分比差异
    if (percentage_difference_from_speed_limit.Contains(actor_id)) {
        percentage_difference = percentage_difference_from_speed_limit.GetValue(actor_id);
    } 
    // 如果参与者有精确的期望速度，直接返回该速度
    else if (exact_desired_speed.Contains(actor_id)) {
        return exact_desired_speed.GetValue(actor_id);
    }

    // 根据速度限制和百分比差异计算目标速度
    return speed_limit * (1.0f - percentage_difference / 100.0f);
}

float Parameters::GetLaneOffset(const ActorId &actor_id) const {
    // 从全局获取车道偏移
    float offset = global_lane_offset;

    // 如果参与者的车道偏移存在，获取其特定的偏移值
    if (lane_offset.Contains(actor_id)) {
        offset = lane_offset.GetValue(actor_id);
    }

    return offset; // 返回车道偏移
}

bool Parameters::GetCollisionDetection(const ActorId &reference_actor_id, const ActorId &other_actor_id) const {
    // 默认设置为避免碰撞
    bool avoid_collision = true;

    // 如果引用参与者被设置为忽略碰撞，并且其他参与者在其忽略列表中
    if (ignore_collision.Contains(reference_actor_id) &&
        ignore_collision.GetValue(reference_actor_id)->Contains(other_actor_id)) {
        avoid_collision = false; // 不避免碰撞
    }

    return avoid_collision; // 返回碰撞检测结果
}

ChangeLaneInfo Parameters::GetForceLaneChange(const ActorId &actor_id) {
    // 初始化车道变更信息
    ChangeLaneInfo change_lane_info {false, false};

    // 如果参与者的强制车道变更存在，获取其信息
    if (force_lane_change.Contains(actor_id)) {
        change_lane_info = force_lane_change.GetValue(actor_id);
    }

    // 移除该参与者的强制车道变更条目
    force_lane_change.RemoveEntry(actor_id);

    return change_lane_info; // 返回车道变更信息
}

float Parameters::GetKeepRightPercentage(const ActorId &actor_id) {
    // 初始化保持右侧的百分比
    float percentage = -1.0f;

    // 如果参与者的保持右侧百分比存在，获取其值
    if (perc_keep_right.Contains(actor_id)) {
        percentage = perc_keep_right.GetValue(actor_id);
    }

    return percentage; // 返回保持右侧的百分比
}

float Parameters::GetRandomLeftLaneChangePercentage(const ActorId &actor_id) {
    // 初始化随机左侧车道变更的百分比
    float percentage = -1.0f;

    // 如果参与者的随机左侧车道变更百分比存在，获取其值
    if (perc_random_left.Contains(actor_id)) {
        percentage = perc_random_left.GetValue(actor_id);
    }

    return percentage; // 返回随机左侧车道变更的百分比
}

float Parameters::GetRandomRightLaneChangePercentage(const ActorId &actor_id) {
    // 初始化随机右侧车道变更的百分比
    float percentage = -1.0f;

    // 如果参与者的随机右侧车道变更百分比存在，获取其值
    if (perc_random_right.Contains(actor_id)) {
        percentage = perc_random_right.GetValue(actor_id);
    }

    return percentage; // 返回随机右侧车道变更的百分比
}

bool Parameters::GetAutoLaneChange(const ActorId &actor_id) const {
    // 默认自动车道变更政策为真
    bool auto_lane_change_policy = true;

    // 如果参与者的自动车道变更设置存在，获取其值
    if (auto_lane_change.Contains(actor_id)) {
        auto_lane_change_policy = auto_lane_change.GetValue(actor_id);
    }

    return auto_lane_change_policy; // 返回自动车道变更政策
}

float Parameters::GetDistanceToLeadingVehicle(const ActorId &actor_id) const {
    // 初始化与前车的距离边际
    float specific_distance_margin = 0.0f;
    
    // 如果参与者的前车距离存在，获取其值
    if (distance_to_leading_vehicle.Contains(actor_id)) {
        specific_distance_margin = distance_to_leading_vehicle.GetValue(actor_id);
    } else {
        specific_distance_margin = distance_margin; // 否则使用全局默认值
    }

    return specific_distance_margin; // 返回与前车的距离
}

float Parameters::GetPercentageRunningLight(const ActorId &actor_id) const {
    // 初始化红绿灯违规的百分比
    float percentage = 0.0f;

    // 如果参与者的红绿灯违规百分比存在，获取其值
    if (perc_run_traffic_light.Contains(actor_id)) {
        percentage = perc_run_traffic_light.GetValue(actor_id);
    }

    return percentage; // 返回红绿灯违规的百分比
}

float Parameters::GetPercentageRunningSign(const ActorId &actor_id) const {
    // 初始化交通标志违规的百分比
    float percentage = 0.0f;

    // 如果参与者的交通标志违规百分比存在，获取其值
    if (perc_run_traffic_sign.Contains(actor_id)) {
        percentage = perc_run_traffic_sign.GetValue(actor_id);
    }

    return percentage; // 返回交通标志违规的百分比
}

float Parameters::GetPercentageIgnoreWalkers(const ActorId &actor_id) const {
    // 初始化忽略行人的百分比
    float percentage = 0.0f;

    // 如果参与者的忽略行人百分比存在，获取其值
    if (perc_ignore_walkers.Contains(actor_id)) {
        percentage = perc_ignore_walkers.GetValue(actor_id);
    }

    return percentage; // 返回忽略行人的百分比
}

bool Parameters::GetUpdateVehicleLights(const ActorId &actor_id) const {
    // 默认更新车辆灯光为假
    bool do_update = false;

    // 如果参与者的灯光更新设置存在，获取其值
    if (auto_update_vehicle_lights.Contains(actor_id)) {
        do_update = auto_update_vehicle_lights.GetValue(actor_id);
    }

    return do_update; // 返回灯光更新设置
}

float Parameters::GetPercentageIgnoreVehicles(const ActorId &actor_id) const {
    // 初始化忽略其他车辆的百分比
    float percentage = 0.0f;

    // 如果参与者的忽略车辆百分比存在，获取其值
    if (perc_ignore_vehicles.Contains(actor_id)) {
        percentage = perc_ignore_vehicles.GetValue(actor_id);
    }

    return percentage; // 返回忽略车辆的百分比
}

bool Parameters::GetHybridPhysicsMode() const {
    // 返回混合物理模式的状态
    return hybrid_physics_mode.load();
}

bool Parameters::GetRespawnDormantVehicles() const {
    // 返回是否重新生成休眠车辆的设置
    return respawn_dormant_vehicles.load();
}

float Parameters::GetLowerBoundaryRespawnDormantVehicles() const {
    // 返回休眠车辆重新生成的下边界值
    return respawn_lower_bound.load();
}

float Parameters::GetUpperBoundaryRespawnDormantVehicles() const {
    // 返回休眠车辆重新生成的上边界值
    return respawn_upper_bound.load();
}

bool Parameters::GetOSMMode() const {
    // 返回是否启用OSM模式的设置
    return osm_mode.load();
}

bool Parameters::GetUploadPath(const ActorId &actor_id) const {
    // 初始化自定义路径标志
    bool custom_path_bool = false;

    // 如果参与者有自定义上传路径，获取其值
    if (upload_path.Contains(actor_id)) {
        custom_path_bool = upload_path.GetValue(actor_id);
    }

    return custom_path_bool; // 返回自定义路径标志
}

Path Parameters::GetCustomPath(const ActorId &actor_id) const {
    // 初始化自定义路径
    Path custom_path_import;

    // 如果参与者有自定义路径，获取其值
    if (custom_path.Contains(actor_id)) {
        custom_path_import = custom_path.GetValue(actor_id);
    }

    return custom_path_import; // 返回自定义路径
}

bool Parameters::GetUploadRoute(const ActorId &actor_id) const {
    // 初始化自定义路线标志
    bool custom_route_bool = false;

    // 如果参与者有自定义上传路线，获取其值
    if (upload_route.Contains(actor_id)) {
        custom_route_bool = upload_route.GetValue(actor_id);
    }

    return custom_route_bool; // 返回自定义路线标志
}

Route Parameters::GetImportedRoute(const ActorId &actor_id) const {
    // 初始化自定义路线
    Route custom_route_import;

    // 如果参与者有自定义路线，获取其值
    if (custom_route.Contains(actor_id)) {
        custom_route_import = custom_route.GetValue(actor_id);
    }

    return custom_route_import; // 返回自定义路线
}


} // namespace traffic_manager
} // namespace carla
