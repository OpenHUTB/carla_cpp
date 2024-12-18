// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 防止头文件重复包含

#include <mutex> // 引入互斥锁，用于线程安全
#include <vector> // 引入向量容器
#include <unordered_map> // 引入哈希表容器
#include <unordered_set> // 引入哈希集合容器

#include "carla/Memory.h" // 引入内存管理模块
#include "carla/NonCopyable.h" // 引入不可复制的基类

#include "carla/client/detail/Episode.h" // 引入细节中的Episode模块
#include "carla/client/Light.h" // 引入Light模块
#include "carla/client/LightState.h" // 引入LightState模块
#include "carla/rpc/LightState.h" // 引入RPC中的LightState模块

namespace carla {
namespace client {

class LightManager // 光照管理器类
  : public EnableSharedFromThis<LightManager> { // 允许从自身获取共享指针

  using LightGroup = rpc::LightState::LightGroup; // 定义LightGroup类型

public:

  LightManager() {} // 构造函数

  ~LightManager(); // 析构函数

  LightManager(const LightManager& other) : EnableSharedFromThis<LightManager>() { // 拷贝构造函数
    _lights_state = other._lights_state; // 拷贝灯光状态
    _lights_changes = other._lights_changes; // 拷贝灯光变更状态
    _lights = other._lights; // 拷贝灯光
    _episode = other._episode; // 拷贝Episode
    _on_tick_register_id = other._on_tick_register_id; // 拷贝tick注册ID
    _on_light_update_register_id = other._on_light_update_register_id; // 拷贝灯光更新注册ID
    _dirty = other._dirty; // 拷贝脏标志
  }

  void SetEpisode(detail::WeakEpisodeProxy episode); // 设置当前Episode

  std::vector<Light> GetAllLights(LightGroup type = LightGroup::None) const; // 获取所有灯光
  // TODO: std::vector<Light> GetAllLightsInRoad(RoadId id, LightGroup type = LightGroup::None); // 在特定道路上获取灯光
  // TODO: std::vector<Light> GetAllLightsInDistance(Vec3 origin, float distance, LightGroup type = Light::LightType::None); // 根据距离获取灯光

  void TurnOn(std::vector<Light>& lights); // 打开灯光
  void TurnOff(std::vector<Light>& lights); // 关闭灯光
  void SetActive(std::vector<Light>& lights, std::vector<bool>& active); // 设置灯光的激活状态
  std::vector<bool> IsActive(std::vector<Light>& lights) const; // 获取灯光的激活状态
  std::vector<Light> GetTurnedOnLights(LightGroup type = LightGroup::None) const; // 获取打开的灯光
  std::vector<Light> GetTurnedOffLights(LightGroup type = LightGroup::None) const; // 获取关闭的灯光

  void SetColor(std::vector<Light>& lights, Color color); // 设置灯光颜色
  void SetColor(std::vector<Light>& lights, std::vector<Color>& colors); // 设置多个灯光颜色
  std::vector<Color> GetColor(std::vector<Light>& lights) const; // 获取灯光颜色

  void SetIntensity(std::vector<Light>& lights, float intensity); // 设置灯光强度
  void SetIntensity(std::vector<Light>& lights, std::vector<float>& intensities); // 设置多个灯光强度
  std::vector<float> GetIntensity(std::vector<Light>& lights) const; // 获取灯光强度

  void SetLightGroup(std::vector<Light>& lights, LightGroup group); // 设置灯光组
  void SetLightGroup(std::vector<Light>& lights, std::vector<LightGroup>& groups); // 设置多个灯光组
  std::vector<LightGroup> GetLightGroup(std::vector<Light>& lights) const; // 获取灯光组

  void SetLightState(std::vector<Light>& lights, LightState state); // 设置灯光状态
  void SetLightState(std::vector<Light>& lights, std::vector<LightState>& states); // 设置多个灯光状态
  std::vector<LightState> GetLightState(std::vector<Light>& lights) const; // 获取灯光状态

  Color GetColor(LightId id) const; // 获取单个灯光颜色
  float GetIntensity(LightId id) const; // 获取单个灯光强度
  LightState GetLightState(LightId id) const; // 获取单个灯光状态
  LightGroup GetLightGroup(LightId id) const; // 获取单个灯光组
  bool IsActive(LightId id) const; // 检查单个灯光是否激活

  void SetActive(LightId id, bool active); // 设置单个灯光激活状态
  void SetColor(LightId id, Color color); // 设置单个灯光颜色
  void SetIntensity(LightId id, float intensity); // 设置单个灯光强度
  void SetLightState(LightId id, const LightState& new_state); // 设置单个灯光状态
  void SetLightStateNoLock(LightId id, const LightState& new_state); // 无锁设置单个灯光状态
  void SetLightGroup(LightId id, LightGroup group); // 设置单个灯光组

  void SetDayNightCycle(const bool active); // 设置昼夜循环

private:

  const LightState& RetrieveLightState(LightId id) const; // 检索灯光状态

  void QueryLightsStateToServer(); // 查询灯光状态到服务器
  void UpdateServerLightsState(bool discard_client = false); // 更新服务器灯光状态
  void ApplyChanges(); // 应用变更

  std::unordered_map<LightId, LightState> _lights_state; // 灯光状态映射
  std::unordered_map<LightId, LightState> _lights_changes; // 灯光变更状态映射
  std::unordered_map<LightId, Light> _lights; // 灯光映射

  detail::WeakEpisodeProxy _episode; // 弱引用的Episode

  std::mutex _mutex; // 互斥锁

  LightState _state; // 当前状态
  size_t _on_tick_register_id = 0; // tick注册ID
  size_t _on_light_update_register_id = 0; // 灯光更新注册ID
  bool _dirty = false; // 脏标志
};

} // namespace client
} // namespace carla
