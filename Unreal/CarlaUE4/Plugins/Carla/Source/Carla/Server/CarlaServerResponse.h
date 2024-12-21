// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 定义一个枚举类型 ECarlaServerResponse，表示 Carla 服务器的响应状态
enum class ECarlaServerResponse
{
  Success,// 操作成功
  ActorNotFound, // 找不到指定的 actor
  ComponentNotFound,// 找不到指定的组件
  ActorTypeMismatch,// actor 类型不匹配
  FunctionNotSupported,// 不支持的函数调用
  NullActor,// 空的 actor
  MissingActor,// 缺少 actor
  NotAVehicle,// 不是车辆类型的 actor
  WalkerDead,// 行人已死亡
  NotAWalker,// 不是行人类型的 actor
  WalkerIncompatibleController,// 行人控制器不兼容
  AutoPilotNotSupported,// 自动驾驶不支持
  CarSimPluginNotEnabled, // 车辆模拟插件未启用
  NotATrafficLight,// 不是交通信号灯
  FunctionNotAvailiableWhenDormant// 当处于休眠状态时函数不可用
};

FString CarlaGetStringError(ECarlaServerResponse Response);
