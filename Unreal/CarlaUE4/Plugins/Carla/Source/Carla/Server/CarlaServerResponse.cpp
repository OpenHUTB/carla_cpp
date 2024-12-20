// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 包含CarlaServerResponse相关的头文件，推测其中定义了ECarlaServerResponse枚举类型等相关内容
#include "CarlaServerResponse.h"

// 函数CarlaGetStringError用于根据给定的ECarlaServerResponse枚举值返回对应的错误描述字符串
// 参数Response是一个ECarlaServerResponse类型的枚举值，表示不同的服务器响应状态
FString CarlaGetStringError(ECarlaServerResponse Response)
{
    // 使用switch语句根据不同的Response枚举值来返回相应的错误描述字符串
    switch (Response)
    {
        // 当Response为Success时，表示操作成功，返回对应的成功提示字符串"Sucess"（此处单词拼写有误，正确应为"Success"，可后续修正）
        case ECarlaServerResponse::Success:
            return "Sucess";
        // 当Response为ActorNotFound时，表示在注册表中找不到对应的Actor（游戏等场景中的角色、实体等概念），返回相应错误描述
        case ECarlaServerResponse::ActorNotFound:
            return "Actor could not be found in the registry";
        // 当Response为ComponentNotFound时，表示在这个Actor中找不到对应的组件，返回相应错误描述
        case ECarlaServerResponse::ComponentNotFound:
            return "Component could not be found in this actor";
        // 当Response为ActorTypeMismatch时，表示Actor与期望的类型不匹配，返回相应错误描述
        case ECarlaServerResponse::ActorTypeMismatch:
            return "Actor does not match the expected type";
        // 当Response为MissingActor时，表示Actor缺失了，返回相应错误描述
        case ECarlaServerResponse::MissingActor:
            return "Actor is missing";
        // 当Response为NotAVehicle时，表示该Actor不是车辆类型，返回相应错误描述
        case ECarlaServerResponse::NotAVehicle:
            return "Actor is not a Vehicle";
        // 当Response为WalkerDead时，表示行人（Walker通常可理解为行人角色）已经死亡，返回相应错误描述
        case ECarlaServerResponse::WalkerDead:
            return "Walker is already dead";
        // 当Response为NotAWalker时，表示该Actor不是行人类型，返回相应错误描述
        case ECarlaServerResponse::NotAWalker:
            return "Actor is not a Walker";
        // 当Response为WalkerIncompatibleController时，表示行人有不兼容的控制器，返回相应错误描述
        case ECarlaServerResponse::WalkerIncompatibleController:
            return "Walker has incompatible controller";
        // 当Response为NullActor时，表示Actor为空，返回相应错误描述
        case ECarlaServerResponse::NullActor:
            return "Actor is null";
        // 当Response为AutoPilotNotSupported时，表示该Actor不支持自动驾驶功能，返回相应错误描述
        case ECarlaServerResponse::AutoPilotNotSupported:
            return "Autopilot is not supported by the Actor";
        // 当Response为CarSimPluginNotEnabled时，表示汽车模拟插件未启用，返回相应错误描述
        case ECarlaServerResponse::CarSimPluginNotEnabled:
            return "CarSim plugin is not enabled";
        // 当Response为NotATrafficLight时，表示该Actor不是交通信号灯，返回相应错误描述
        case ECarlaServerResponse::NotATrafficLight:
            return "Actor is not a traffic light";
        // 当Response为FunctionNotAvailiableWhenDormant时，表示当Actor处于休眠状态时该函数不可用，返回相应错误描述
        case ECarlaServerResponse::FunctionNotAvailiableWhenDormant:
            return "Function not availiable when the actor is dormant";
    }
    // 如果传入的Response枚举值不匹配上述任何已定义的情况，返回默认的"unknown error"表示未知错误
    return "unknown error";
}
