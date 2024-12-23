
// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Light.h"
#include "carla/client/LightManager.h"

#include <assert.h>

namespace carla {
namespace client {
// 定义 LightGroup 类型别名，实际是 rpc::LightState::LightGroup
  using LightGroup = rpc::LightState::LightGroup;
// 获取灯光颜色
Color Light::GetColor() const {
  // 获取弱指针指向的 LightManager 对象
  auto light_manager = _light_manager.lock();
  // 断言确保 light_manager 存在
  assert(light_manager && "No light_manager");
  // 调用 LightManager 的 GetColor 方法获取灯光颜色
  return light_manager->GetColor(_id);
}
// 获取灯光强度
float Light::GetIntensity() const {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  return light_manager->GetIntensity(_id);
}
// 获取灯光组
LightGroup Light::GetLightGroup() const {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  return light_manager->GetLightGroup(_id);
}
// 获取灯光状态
LightState Light::GetLightState() const {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  return light_manager->GetLightState(_id);
}
// 判断灯光是否开启
bool Light::IsOn() const {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  // 判断灯光是否激活（开启）
  return light_manager->IsActive(_id) == true;
}
// 判断灯光是否关闭
bool Light::IsOff() const {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  // 判断灯光是否未激活（关闭）
  return light_manager->IsActive(_id) == false;
}
// 设置灯光颜色
void Light::SetColor(Color color) {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  light_manager->SetColor(_id, color);
}
// 设置灯光强度
void Light::SetIntensity(float intensity) {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  light_manager->SetIntensity(_id, intensity);
}
// 设置灯光组
void Light::SetLightGroup(LightGroup group) {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  light_manager->SetLightGroup(_id, group);
}
// 设置灯光状态
void Light::SetLightState(const LightState& state) {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  light_manager->SetLightState(_id, state);
}
// 打开灯光
void Light::TurnOn() {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  light_manager->SetActive(_id, true);
}
// 关闭灯光
void Light::TurnOff() {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  light_manager->SetActive(_id, false);
}


} // namespace client
} // namespace carla
