// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/LightManager.h"

#include "carla/client/detail/Simulator.h"


namespace carla {
namespace client {

  using LightGroup = rpc::LightState::LightGroup; // 定义 LightGroup类型，用于表示灯光组
// 析构函数，清理灯光管理器资源
LightManager::~LightManager(){
  // 检查是否存在有效的场景实例
  if(_episode.IsValid()) {
    // 移除灯光相关的事件回调
    _episode.Lock()->RemoveOnTickEvent(_on_light_update_register_id);
    _episode.Lock()->RemoveLightUpdateChangeEvent(_on_light_update_register_id);
  }
  // 同步灯光状态至服务器并强制更新
  UpdateServerLightsState(true);
}

// 设置当前场景实例并注册事件
void LightManager::SetEpisode(detail::WeakEpisodeProxy episode) {

  _episode = episode; // 保存场景引用
  // 注册在每帧更新时的回调函数
  _on_tick_register_id = _episode.Lock()->RegisterOnTickEvent(
    [&](const WorldSnapshot&) {
      UpdateServerLightsState(); // 同步灯光状态到服务器
    });

  // 注册灯光更新事件的回调函数
  _on_light_update_register_id = _episode.Lock()->RegisterLightUpdateChangeEvent(
    [&](const WorldSnapshot& ) {
      QueryLightsStateToServer(); // 查询灯光状态 
      ApplyChanges(); // 应用灯光的改变
    });

    QueryLightsStateToServer(); // 初始化时查询灯光状态
}
// 获取指定灯光组类型的所有灯光
std::vector<Light> LightManager::GetAllLights(LightGroup type) const {
  std::vector<Light> result;
  // 遍历所有灯光状态
  for(auto lights_state : _lights_state) {
    LightGroup group = lights_state.second._group;
    // 如果类型为None 或匹配指定类型，则添加到结果中
    if((type == LightGroup::None) || (group == type)) {
      auto it_light = _lights.find(lights_state.first);
      result.push_back(it_light->second);
    }
  }

  return result; // 返回符合条件的灯光
}
// 打开指定的灯光
void LightManager::TurnOn(std::vector<Light>& lights) {
  for(Light& light : lights) {
    SetActive(light._id, true); // 设置灯光为激活状态
  }
}
// 关闭指定的灯光
void LightManager::TurnOff(std::vector<Light>& lights) {
  for(Light& light : lights) {
    SetActive(light._id, false); // 设置灯光为非激活状态
  }
}

// 根据布尔值数组设置灯光激活状态
void LightManager::SetActive(std::vector<Light>& lights, std::vector<bool>& active) {
  size_t lights_to_update = (lights.size() < active.size()) ? lights.size() : active.size();
  for(size_t i = 0; i < lights_to_update; i++) {
    SetActive(lights[i]._id, active[i]); // 根据对应索引设置灯光状态
  }
}

// 检查指定灯光是否为激活状态
std::vector<bool> LightManager::IsActive(std::vector<Light>& lights) const {
  std::vector<bool> result;
  for(Light& light : lights) {
    result.push_back( IsActive(light._id) ); // 查询每个灯光的激活状态
  }
  return result; // 返回激活状态列表
}
// 获取所有开启状态的灯光
std::vector<Light> LightManager::GetTurnedOnLights(LightGroup type) const {
  std::vector<Light> result;
  // 遍历灯光状态
  for(auto lights_state : _lights_state) {
    LightState& state = lights_state.second;
    LightGroup group = state._group;
    // 检查类型匹配且灯光处于开启状态
    if( (type == LightGroup::None || group == type) && state._active ) {
      auto it_light = _lights.find(lights_state.first);
      result.push_back(it_light->second);
    }
  }

  return result; // 返回所有开启状态的灯光
}
// 获取所有关闭状态的灯光
std::vector<Light> LightManager::GetTurnedOffLights(LightGroup type) const {
  std::vector<Light> result;
  // 遍历灯光状态
  for(auto lights_state : _lights_state) {
    LightState& state = lights_state.second;
    LightGroup group = state._group;
    // 检查类型匹配且灯光处于非关闭状态
    if( (type == LightGroup::None || group == type) && !state._active ) {
      auto it_light = _lights.find(lights_state.first);
      result.push_back(it_light->second);
    }
  }

  return result; // 返回所有关闭状态的灯光
}

// 设置多个灯光的相同颜色
void LightManager::SetColor(std::vector<Light>& lights, Color color) {
  // 遍历灯光列表，逐个设置颜色
  for(Light& light : lights) {
    SetColor(light._id, color); // 调用内部方法根据灯光ID设置颜色
  }
}

// 设置多个灯光的不同颜色
void LightManager::SetColor(std::vector<Light>& lights, std::vector<Color>& colors) {
  // 确定需要更新的灯光数量，以灯光和颜色列表较短者为准
  size_t lights_to_update = (lights.size() < colors.size()) ? lights.size() : colors.size();
  // 遍历灯光和颜色列表，逐个设置颜色
  for(size_t i = 0; i < lights_to_update; i++) {
    SetColor(lights[i]._id, colors[i]); // 调用内部方法设置每个灯光的颜色
  }
}

// 获取多个灯光的颜色
std::vector<Color> LightManager::GetColor(std::vector<Light>& lights) const {
  std::vector<Color> result; // 保存颜色结果的向量
  for(Light& light : lights) {
    result.push_back( GetColor(light._id) ); // 调用内部方法获取颜色
  }
  return result; // 返回所有灯光的颜色
}

// 设置多个灯光的相同亮度
void LightManager::SetIntensity(std::vector<Light>& lights, float intensity) {
  for(Light& light : lights) {
    SetIntensity(light._id, intensity); // 调用内部方法设置亮度
  }
}

//设置多个灯光的不同亮度
void LightManager::SetIntensity(std::vector<Light>& lights, std::vector<float>& intensities) {
  size_t lights_to_update = (lights.size() < intensities.size()) ? lights.size() : intensities.size();
  for(size_t i = 0; i < lights_to_update; i++) {
    SetIntensity(lights[i]._id, intensities[i]); // 根据灯光ID和亮度列表设置亮度
  }
}

// 获取多个灯光的亮度
std::vector<float> LightManager::GetIntensity(std::vector<Light>& lights) const {
  std::vector<float> result; // 保存亮度结果的向量 
  for(Light& light : lights) {
    result.push_back( GetIntensity(light._id) ); // 调用内部方法获取亮度
  }
  return result; // 返回所有灯光的亮度
}

// 设置多个灯光的相同分组
void LightManager::SetLightGroup(std::vector<Light>& lights, LightGroup group) {
  for(Light& light : lights) {
    SetLightGroup(light._id, group); // 调用内部方法设置分组
  }
}

// 设置多个灯光的不同分组
void LightManager::SetLightGroup(std::vector<Light>& lights, std::vector<LightGroup>& groups) {
  size_t lights_to_update = (lights.size() < groups.size()) ? lights.size() : groups.size();
  for(size_t i = 0; i < lights_to_update; i++) {
    SetLightGroup(lights[i]._id, groups[i]); // 根据灯光ID和分组列表设置分组
  }
}

// 获取多个灯光的分组
std::vector<LightGroup> LightManager::GetLightGroup(std::vector<Light>& lights) const {
  std::vector<LightGroup> result; // 保存分组结果的向量
  for(Light& light : lights) {
    result.push_back( GetLightGroup(light._id) ); // 调用内部方法获取分组
  }
  return result; // 返回所有灯光的分组
}

// 设置多个灯光的相同状态
void LightManager::SetLightState(std::vector<Light>& lights, LightState state) {
  for(Light& light : lights) {
    SetLightState(light._id, state); // 调用内部方法设置状态
  }
}

// 设置多个灯光的不同状态
void LightManager::SetLightState(std::vector<Light>& lights, std::vector<LightState>& states) {
  size_t lights_to_update = (lights.size() < states.size()) ? lights.size() : states.size();
  for(size_t i = 0; i < lights_to_update; i++) {
    SetLightState(lights[i]._id, states[i]); // 根据灯光ID和状态列表设置状态
  }
}

// 获取多个灯光的状态
std::vector<LightState> LightManager::GetLightState(std::vector<Light>& lights) const {
  std::vector<LightState> result; // 保存状态结果的向量
  for(Light& light : lights) {
    result.push_back( RetrieveLightState(light._id) ); // 调用内部方法获取状态
  }
  return result; // 返回所有灯光的状态
}

Color LightManager::GetColor(LightId id) const {
  // 返回指定灯光的颜色属性
  return RetrieveLightState(id)._color;
}

float LightManager::GetIntensity(LightId id) const {
  // 返回指定灯光的亮度属性
  return RetrieveLightState(id)._intensity;
}

LightState LightManager::GetLightState(LightId id) const {
  // 返回指定灯光的完整状态信息
  return RetrieveLightState(id);
}

LightGroup LightManager::GetLightGroup(LightId id) const {
  // 返回指定灯光的分组属性
  return RetrieveLightState(id)._group;
}

bool LightManager::IsActive(LightId id) const {
  // 返回指定灯光是否处于激活状态
  return RetrieveLightState(id)._active;
}

void LightManager::SetActive(LightId id, bool active) {
  std::lock_guard<std::mutex> lock(_mutex); // 加锁以保护多线程访问
  LightState& state = const_cast<LightState&>(RetrieveLightState(id)); // 获取灯光状态
  state._active = active; // 更新激活状态
  _lights_changes[id] = state; // 将更改记录到修改列表
  _dirty = true; // 标记有更改
}

void LightManager::SetColor(LightId id, Color color) {
  std::lock_guard<std::mutex> lock(_mutex); // 加锁以保护多线程访问
  LightState& state = const_cast<LightState&>(RetrieveLightState(id)); // 获取灯光状态
  state._color = color; // 更新颜色属性
  _lights_changes[id] = state; // 将更改记录到修改列表中
  _dirty = true; // 标记有更改
}

void LightManager::SetIntensity(LightId id, float intensity) {
  std::lock_guard<std::mutex> lock(_mutex); // 加锁以保护多线程访问
  LightState& state = const_cast<LightState&>(RetrieveLightState(id)); // 获取灯光状态
  state._intensity = intensity; // 更新亮度属性
  _lights_changes[id] = state; // 将更改记录到修改列表
  _dirty = true; // 标记有更改
}

void LightManager::SetLightState(LightId id, const LightState& new_state) {
  std::lock_guard<std::mutex> lock(_mutex); // 加锁以保护多线程访问
  LightState& state = const_cast<LightState&>(RetrieveLightState(id)); // 获取灯光状态
  state = new_state; //更新完整状态
  _lights_changes[id] = state; // 将更改记录到修改列表
  _dirty = true; // 标记有更改
}

void LightManager::SetLightStateNoLock(LightId id, const LightState& new_state) {
  // 无需加锁设置指定灯光的完整状态信息，用于内部调用
  LightState& state = const_cast<LightState&>(RetrieveLightState(id));
  state = new_state; // 更新完整状态
  _lights_changes[id] = state; // 将更改记录到修改列表
}

void LightManager::SetLightGroup(LightId id, LightGroup group) {
  std::lock_guard<std::mutex> lock(_mutex); // 加锁以保护多线程访问
  LightState& state = const_cast<LightState&>(RetrieveLightState(id)); // 获取灯光状态
  state._group = group; // 更新分组属性
  _lights_changes[id] = state; // 将更改记录到修改列表
  _dirty = true; // 标记有更改
}

const LightState& LightManager::RetrieveLightState(LightId id) const {
  // 从存储中检索指定灯光的状态信息
  auto it = _lights_state.find(id);
  if(it == _lights_state.end()) {
    carla::log_warning("Invalid light", id); // 记录无效灯光警告
    return _state; // 返回默认状态
  }
  return it->second; // 返回找到的灯光状态
}

void LightManager::QueryLightsStateToServer() {
  std::lock_guard<std::mutex> lock(_mutex);
  // 发送 blocking 查询到服务器以获取灯光状态
  std::vector<rpc::LightState> lights_snapshot = _episode.Lock()->QueryLightsStateToServer();

  // 更新本地灯光状态
  SharedPtr<LightManager> lm = _episode.Lock()->GetLightManager();

  for(const auto& it : lights_snapshot) {
    _lights_state[it._id] = LightState(
        it._intensity,
        Color(it._color.r, it._color.g, it._color.b),
        static_cast<LightState::LightGroup>(it._group),
        it._active
    );
    // 如果灯光ID不在本地记录，则创建新灯光
    if(_lights.find(it._id) == _lights.end())
    {
      _lights[it._id] = Light(lm, it._location, it._id);
    }
  }
}

void LightManager::UpdateServerLightsState(bool discard_client) {
  std::lock_guard<std::mutex> lock(_mutex);

  if(_dirty) {
    std::vector<rpc::LightState> message;
    for(auto it : _lights_changes) {
      auto it_light = _lights.find(it.first);
      if(it_light != _lights.end()) {
        rpc::LightState state(
          it_light->second.GetLocation(),
          it.second._intensity,
          it.second._group,
          rpc::Color(it.second._color.r, it.second._color.g, it.second._color.b),
          it.second._active
        );
        state._id = it.first;
        // 添加到命令
        message.push_back(state);
      }
    }
    _episode.Lock()->UpdateServerLightsState(message, discard_client);

    _lights_changes.clear();
    _dirty = false;
  }
}

void LightManager::ApplyChanges() {
  std::lock_guard<std::mutex> lock(_mutex);
  for(const auto& it : _lights_changes) {
    SetLightStateNoLock(it.first, it.second);
  }
}

void LightManager::SetDayNightCycle(const bool active) {
  _episode.Lock()->UpdateDayNightCycle(active);
}

} // namespace client
} // namespace carla
