// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"    // 包含内存管理相关的头文件


#include "carla/client/LightState.h"  // 包含光照状态相关的客户端头文件
#include "carla/geom/Location.h"   // 包含位置几何信息的头文件
#include "carla/geom/Rotation.h"   // 包含旋转几何信息的头文件
#include "carla/rpc/LightState.h"   // 包含远程过程调用的光照状态定义头文件


namespace carla {  // 开始 carla 命名空间
namespace client {  // 开始 client 命名空间

class LightManager;   // 前向声明 LightManager 类

class Light {    // 定义 Light 类


  using LightGroup = rpc::LightState::LightGroup;   // 定义 LightGroup 类型别名

public:

  Light() {}  // 默认构造函数

  Color GetColor() const;    // 获取颜色的成员函数

  LightId GetId() const {  // 获取光源 ID 的成员函数
    return _id;  // 返回光源的 ID
  }

  float GetIntensity() const;    // 获取光强度的成员函数

  const geom::Location GetLocation() const {  // 获取位置的成员函数
    return _location;   // 返回光源的位置
  }

  LightGroup GetLightGroup() const;   // 获取光源组的成员函数

  LightState GetLightState() const;   // 获取光照状态的成员函数


  bool IsOn() const;    // 检查光源是否开启的成员函数

  bool IsOff() const;     // 检查光源是否关闭的成员函数

  void SetColor(Color color);   // 设置光源颜色的成员函数

  void SetIntensity(float intensity);    // 设置光源强度的成员函数

  void SetLightGroup(LightGroup group);    // 设置光源组的成员函数

  void SetLightState(const LightState& state);    // 设置光照状态的成员函数

  void TurnOn();   // 开启光源的成员函数


  void TurnOff();     // 关闭光源的成员函数

private:   // 私有成员


  friend class LightManager;    // 声明 LightManager 为友元类，可以访问私有成员

  Light(WeakPtr<LightManager> light_manager,    // 构造函数，接受弱指针的 LightManager
    geom::Location location,   // 位置参数
    LightId id)     // 光源 ID 参数
  : _light_manager(light_manager),   // 初始化光源管理器
    _location (location),   // 初始化位置
    _id (id) {}   // 初始化光源 ID

  WeakPtr<LightManager> _light_manager;    // 存储光源管理器的弱指针

  geom::Location _location;   // 存储光源位置

  LightId _id;   // 存储光源 ID

};

} // namespace client
} // namespace carla
