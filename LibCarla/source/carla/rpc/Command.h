// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once	// 确保头文件只被包含一次，避免重复定义等问题

#include "carla/MsgPack.h"	// 包含Carla的MsgPack相关头文件，可能用于序列化和反序列化等数据处理操作
#include "carla/MsgPackAdaptors.h"	// 包含MsgPack的适配器相关头文件，用于使一些类型能更好地适配MsgPack的序列化和反序列化机制
#include "carla/geom/Transform.h"	// 包含几何变换相关的头文件，用于处理如位置、旋转等变换信息，可能在控制角色位置等方面会用到
#include "carla/rpc/ActorDescription.h"	// 包含Actor描述相关的头文件，用于定义场景中各种角色（如车辆、行人等）的相关属性描述信息
#include "carla/rpc/AttachmentType.h"	// 包含附件类型相关的头文件，可能用于描述角色之间的附着关系等
#include "carla/rpc/ActorId.h"	// 包含Actor唯一标识符相关的头文件，用于区分不同的角色实例
#include "carla/rpc/TrafficLightState.h"	// 包含交通信号灯状态相关的头文件，用于表示交通信号灯的不同状态（如红、绿、黄等）
#include "carla/rpc/VehicleAckermannControl.h"	// 包含车辆阿克曼转向控制相关的头文件，用于控制采用阿克曼转向原理的车辆运动
#include "carla/rpc/VehicleControl.h"	// 包含车辆常规控制（如油门、刹车、转向等）相关的头文件
#include "carla/rpc/VehiclePhysicsControl.h"	// 包含车辆物理属性控制相关的头文件，可能涉及车辆的质量、摩擦力等物理特性控制
#include "carla/rpc/VehicleLightState.h"	// 包含车辆灯光状态相关的头文件，用于控制车辆各种灯光（如大灯、转向灯等）的开关状态
#include "carla/rpc/WalkerControl.h"	// 包含行人控制相关的头文件，用于控制行人在场景中的移动等行为

#include <string>	// 包含标准库中的字符串头文件，用于处理文本相关操作，很多地方可能会用到字符串来表示名称、命令等信息

#ifdef _MSC_VER	// 以下是针对微软编译器（MSC_VER）的特定编译警告处理设置
#pragma warning(push)	// 保存当前的警告状态
#pragma warning(disable:4583)	// 禁用特定的警告（4583，可能是和类模板相关的某种警告）
#pragma warning(disable:4582)	// 禁用特定的警告（4582，可能和类模板相关的另一种警告）
#include <boost/variant2/variant.hpp>	// 包含boost库中的variant2变体类型头文件，用于定义可以容纳多种不同类型值的变量类型
#pragma warning(pop)	// 恢复之前保存的警告状态
#else	
#include <boost/variant2/variant.hpp>	// 如果不是微软编译器，直接包含boost库中的variant2变体类型头文件
#endif

namespace carla {	// 定义Carla命名空间，以下相关的类、结构体等都在这个命名空间下组织

namespace traffic_manager {	// 定义交通管理器相关的内部命名空间，里面可能包含交通管理相关的具体实现类等，但此处只是前置声明了TrafficManager类
  class TrafficManager;
}

namespace ctm = carla::traffic_manager;	// 为了方便使用，给carla::traffic_manager命名空间定义一个简短的别名ctm

namespace rpc {	// 定义rpc命名空间，用于存放远程过程调用（RPC）相关的类型和函数等，这里主要定义了各种控制命令相关的结构体和类型

  class Command {	// 定义Command类，用于封装各种不同类型的控制命令，是整个命令体系的核心类
  private:

    template <typename T>	// 定义一个模板结构体CommandBase，作为各种具体命令结构体的基类模板，用于实现类型转换到Command类型的操作
    struct CommandBase {
      operator Command() const {	// 重载类型转换运算符，将派生类类型转换为Command类型
        return Command{*static_cast<const T *>(this)};
      }
    };

  public:

    struct SpawnActor : CommandBase<SpawnActor> {	// 定义SpawnActor结构体，表示生成角色（如车辆、行人等）的命令结构体，继承自CommandBase<SpawnActor>
      SpawnActor() = default;	// 默认构造函数，用于创建一个默认初始化的SpawnActor命令对象
      SpawnActor(ActorDescription description, const geom::Transform &transform)	// 构造函数，用于创建一个生成角色的命令，传入角色描述和初始变换信息
        : description(std::move(description)),
          transform(transform) {}
      SpawnActor(ActorDescription description, const geom::Transform &transform, ActorId parent)	// 构造函数，用于创建一个生成角色的命令，传入角色描述、初始变换信息以及父角色的ID（可能用于表示父子关系，比如车辆挂载某个附件等情况）
        : description(std::move(description)),
          transform(transform),
          parent(parent) {}
      SpawnActor(ActorDescription description, const geom::Transform &transform, ActorId parent, AttachmentType attachment_type, const std::string& bone)	// 构造函数，用于创建一个更详细的生成角色的命令，除了上述信息外，还传入附着类型和骨骼名称（可能用于更精细的角色附着相关操作）
        : description(std::move(description)),
          transform(transform),
          parent(parent),
          attachment_type(attachment_type),
          socket_name(bone) {}
      ActorDescription description;// 角色的描述信息，包含了角色的各种属性，如类型、模型等
      geom::Transform transform;// 角色的初始几何变换信息，用于确定在场景中的位置、朝向等
      boost::optional<ActorId> parent;// 可选的父角色ID，用于表示与其他角色的关联关系（如果有）
      AttachmentType attachment_type;// 附着类型，定义了如何与其他角色进行附着连接
      std::string socket_name;// 骨骼名称（可能用于特定的模型绑定等情况，比如在游戏中角色模型的骨骼绑定相关操作）
      std::vector<Command> do_after;// 一个命令列表，表示在生成该角色后需要执行的其他命令
      MSGPACK_DEFINE_ARRAY(description, transform, parent, attachment_type, socket_name, do_after); // 使用MSGPACK_DEFINE_ARRAY宏定义了如何将SpawnActor结构体中的成员进行MsgPack序列化和反序列化，按照给定的成员顺序进行处理
    };

    struct DestroyActor : CommandBase<DestroyActor> {// 定义DestroyActor结构体，表示销毁角色的命令结构体，继承自CommandBase<DestroyActor>
      DestroyActor() = default;// 默认构造函数，创建一个默认初始化的DestroyActor命令对象
      DestroyActor(ActorId id)// 构造函数，传入要销毁的角色的ID，用于指定具体要销毁的角色
        : actor(id) {}
      ActorId actor;// 要销毁的角色的ID
      MSGPACK_DEFINE_ARRAY(actor);// 使用MSGPACK_DEFINE_ARRAY宏定义如何对DestroyActor结构体中的成员（仅actor成员）进行MsgPack序列化和反序列化
    };

    struct ApplyVehicleControl : CommandBase<ApplyVehicleControl> {// 定义ApplyVehicleControl结构体，表示应用车辆常规控制（如油门、刹车、转向等）的命令结构体，继承自CommandBase<ApplyVehicleControl>
      ApplyVehicleControl() = default;// 默认构造函数，创建一个默认初始化的ApplyVehicleControl命令对象
      ApplyVehicleControl(ActorId id, const VehicleControl &value)// 构造函数，传入要控制的车辆角色的ID以及具体的车辆控制信息
        : actor(id),
          control(value) {}
      ActorId actor;// 要控制的车辆角色的ID
      VehicleControl control;// 车辆的控制信息，包含油门、刹车、转向等控制参数
      MSGPACK_DEFINE_ARRAY(actor, control);// 使用MSGPACK_DEFINE_ARRAY宏定义如何对ApplyVehicleControl结构体中的成员进行MsgPack序列化和反序列化
    };

    struct ApplyVehicleAckermannControl : CommandBase<ApplyVehicleAckermannControl> {// 定义ApplyVehicleAckermannControl结构体，表示应用车辆阿克曼转向控制的命令结构体，继承自CommandBase<ApplyVehicleAckermannControl>
      ApplyVehicleAckermannControl() = default;// 默认构造函数，创建一个默认初始化的ApplyVehicleAckermannControl命令对象
      ApplyVehicleAckermannControl(ActorId id, const VehicleAckermannControl &value)// 构造函数，传入要控制的车辆角色的ID以及具体的阿克曼转向控制信息
        : actor(id),
          control(value) {}
      ActorId actor;// 要控制的车辆角色的ID
      VehicleAckermannControl control;// 车辆的阿克曼转向控制信息，包含转向相关的参数等
      MSGPACK_DEFINE_ARRAY(actor, control);// 使用MSGPACK_DEFINE_ARRAY宏定义如何对ApplyVehicleAckermannControl结构体中的成员进行MsgPack序列化和反序列化
    };

    struct ApplyWalkerControl : CommandBase<ApplyWalkerControl> {// 定义ApplyWalkerControl结构体，表示应用行人控制的命令结构体，继承自CommandBase<ApplyWalkerControl>
      ApplyWalkerControl() = default; // 默认构造函数，创建一个默认初始化的ApplyWalkerControl命令对象
      ApplyWalkerControl(ActorId id, const WalkerControl &value)// 构造函数，传入要控制的行人角色的ID以及具体的行人控制信息
        : actor(id),
          control(value) {}
      ActorId actor;// 要控制的行人角色的ID
      WalkerControl control;// 行人的控制信息，用于控制行人的移动方向、速度等行为
      MSGPACK_DEFINE_ARRAY(actor, control);// 使用MSGPACK_DEFINE_ARRAY宏定义如何对ApplyWalkerControl结构体中的成员进行MsgPack序列化和反序列化
    };

    struct ApplyVehiclePhysicsControl : CommandBase<ApplyVehiclePhysicsControl> {// 定义ApplyVehiclePhysicsControl结构体，表示应用车辆物理属性控制的命令结构体，继承自CommandBase<ApplyVehiclePhysicsControl>
      ApplyVehiclePhysicsControl() = default;// 默认构造函数，创建一个默认初始化的ApplyVehiclePhysicsControl命令对象
      ApplyVehiclePhysicsControl(ActorId id, const VehiclePhysicsControl &value)// 构造函数，传入要控制的车辆角色的ID以及具体的车辆物理控制信息
        : actor(id),
          physics_control(value) {}
      ActorId actor;// 要控制的车辆角色的ID
      VehiclePhysicsControl physics_control;// 车辆的物理控制信息，涉及车辆的质量、摩擦力等物理特性参数
      MSGPACK_DEFINE_ARRAY(actor, physics_control);// 使用MSGPACK_DEFINE_ARRAY宏定义如何对ApplyVehiclePhysicsControl结构体中的成员进行MsgPack序列化和反序列化
    };

    struct ApplyTransform : CommandBase<ApplyTransform> {// 定义ApplyTransform结构体，表示应用几何变换（如平移、旋转等）到角色上的命令结构体，继承自CommandBase<ApplyTransform>
      ApplyTransform() = default; // 默认构造函数，创建一个默认初始化的ApplyTransform命令对象
      ApplyTransform(ActorId id, const geom::Transform &value)// 构造函数，传入要应用变换的角色的ID以及具体的几何变换信息
        : actor(id),
          transform(value) {}
      ActorId actor;// 要应用变换的角色的ID
      geom::Transform transform;// 具体的几何变换信息，用于改变角色在场景中的位置、朝向等
      MSGPACK_DEFINE_ARRAY(actor, transform);// 使用MSGPACK_DEFINE_ARRAY宏定义如何对ApplyTransform结构体中的成员进行MsgPack序列化和反序列化
    };

    struct ApplyLocation : CommandBase<ApplyLocation> {// 定义ApplyLocation结构体，表示应用位置信息到角色上的命令结构体，继承自CommandBase<ApplyLocation>
      ApplyLocation() = default; // 默认构造函数，创建一个默认初始化的ApplyLocation命令对象
      ApplyLocation(ActorId id, const geom::Location &value) // 构造函数，传入要应用位置的角色的ID以及具体的位置信息
        : actor(id),
          location(value) {}
      ActorId actor;// 要应用位置的角色的ID
      geom::Location location; // 具体的位置信息，用于改变角色在场景中的坐标位置
      MSGPACK_DEFINE_ARRAY(actor, location);// 使用MSGPACK_DEFINE_ARRAY宏定义如何对ApplyLocation结构体中的成员进行MsgPack序列化和反序列化
    };

    struct ApplyWalkerState : CommandBase<ApplyWalkerState> {// 定义ApplyWalkerState结构体，表示应用行人状态（包含位置和速度等信息）的命令结构体，继承自CommandBase<ApplyWalkerState>
      ApplyWalkerState() = default; // 默认构造函数，创建一个默认初始化的ApplyWalkerState命令对象
      ApplyWalkerState(ActorId id, const geom::Transform &value, const float speed) : actor(id), transform(value), speed(speed) {}// 构造函数，传入要应用状态的行人角色的ID、几何变换信息（包含位置、朝向等）以及速度信息
      ActorId actor;// 要应用状态的行人角色的ID
      geom::Transform transform;// 行人的几何变换信息，确定行人在场景中的位置和朝向
      float speed;// 行人的移动速度信息
      MSGPACK_DEFINE_ARRAY(actor, transform, speed);// 使用MSGPACK_DEFINE_ARRAY宏定义如何对ApplyWalkerState结构体中的成员进行MsgPack序列化和反序列化
    };

    struct ApplyTargetVelocity : CommandBase<ApplyTargetVelocity> {// 定义ApplyTargetVelocity结构体，表示应用目标速度到角色（可能是车辆、行人等）上的命令结构体，继承自CommandBase<ApplyTargetVelocity>
      ApplyTargetVelocity() = default;// 默认构造函数，创建一个默认初始化的ApplyTargetVelocity命令对象
      ApplyTargetVelocity(ActorId id, const geom::Vector3D &value)// 构造函数，传入要应用目标速度的角色的ID以及具体的目标速度向量信息
        : actor(id),
          velocity(value) {}
      ActorId actor;// 要应用目标速度的角色的ID
      geom::Vector3D velocity;// 目标速度向量信息，用于设定角色的移动速度方向和大小
      MSGPACK_DEFINE_ARRAY(actor, velocity);// 使用MSGPACK_DEFINE_ARRAY宏定义如何对ApplyTargetVelocity结构体中的成员进行MsgPack序列化和反序列化
    };

    struct ApplyTargetAngularVelocity : CommandBase<ApplyTargetAngularVelocity> { // 定义ApplyTargetAngularVelocity结构体，表示应用目标角速度到角色上的命令结构体，继承自CommandBase<ApplyTargetAngularVelocity>
      ApplyTargetAngularVelocity() = default;// 默认构造函数，创建一个默认初始化的ApplyTargetAngularVelocity命令对象
      ApplyTargetAngularVelocity(ActorId id, const geom::Vector3D &value)// 构造函数，传入要应用目标角速度的角色的ID以及具体的目标角速度向量信息
        : actor(id),
          angular_velocity(value) {}
      ActorId actor;// 要应用目标角速度的角色的ID
      geom::Vector3D angular_velocity;// 目标角速度向量信息，用于设定角色的旋转角速度方向和大小
      MSGPACK_DEFINE_ARRAY(actor, angular_velocity);// 使用MSGPACK_DEFINE_ARRAY宏定义如何对ApplyTargetAngularVelocity结构体中的成员进行MsgPack序列化和反序列化
    };

    struct ApplyImpulse : CommandBase<ApplyImpulse> {// 定义ApplyImpulse结构体，表示应用冲量到角色上的命令结构体，继承自CommandBase<ApplyImpulse>
      ApplyImpulse() = default;// 默认构造函数，创建一个默认初始化的ApplyImpulse命令对象
      ApplyImpulse(ActorId id, const geom::Vector3D &value)// 构造函数，传入要应用冲量的角色的ID以及具体的冲量向量信息
        : actor(id),
          impulse(value) {}
      ActorId actor;// 要应用冲量的角色的ID
      geom::Vector3D impulse;// 冲量向量信息，用于给角色施加瞬间的力的作用效果（根据物理学原理改变角色的运动状态等）
      MSGPACK_DEFINE_ARRAY(actor, impulse);// 使用MSGPACK_DEFINE_ARRAY宏定义如何对ApplyImpulse结构体中的成员进行MsgPack序列化和反序列化
    };

    struct ApplyForce : CommandBase<ApplyForce> {// 定义ApplyForce结构体，表示应用力到角色上的命令结构体，继承自CommandBase<ApplyForce>
      ApplyForce() = default;// 默认构造函数，创建一个默认初始化的ApplyForce命令对象
      ApplyForce(ActorId id, const geom::Vector3D &value)// 构造函数，传入要应用力的角色的ID以及具体的力向量信息
        : actor(id),
          force(value) {}
      ActorId actor;// 要应用力的角色的ID
      geom::Vector3D force;// 表示要施加的力向量信息，它定义了力的大小、方向等物理属性，用于模拟角色在受力情况下的物理行为变化
      MSGPACK_DEFINE_ARRAY(actor, force);// 使用MSGPACK_DEFINE_ARRAY宏定义如何对ApplyForce结构体中的成员（actor和force）进行MsgPack序列化和反序列化操作
    };

    struct ApplyAngularImpulse : CommandBase<ApplyAngularImpulse> {// 定义ApplyAngularImpulse结构体，表示应用角冲量到角色上的命令结构体，继承自CommandBase<ApplyAngularImpulse>
      ApplyAngularImpulse() = default;// 构造函数，传入要应用角冲量的角色的ID以及具体的角冲量向量信息
      ApplyAngularImpulse(ActorId id, const geom::Vector3D &value)// 用于创建一个明确指定了施加角冲量的角色以及具体角冲量向量的命令对象
        : actor(id),
          impulse(value) {}
      ActorId actor;// 要应用角冲量的角色的ID，用于确定场景中哪个角色会受到此角冲量的作用
      geom::Vector3D impulse; // 表示要施加的角冲量向量信息，它决定了角冲量的大小和方向，进而影响角色的旋转特性改变
      MSGPACK_DEFINE_ARRAY(actor, impulse);// 使用MSGPACK_DEFINE_ARRAY宏定义如何对ApplyAngularImpulse结构体中的成员进行MsgPack序列化和反序列化操作
    };

    struct ApplyTorque : CommandBase<ApplyTorque> {// ApplyTorque结构体继承自CommandBase<ApplyTorque>，用于表示施加扭矩的命令相关信息
      ApplyTorque() = default;// 默认构造函数，使用编译器默认生成的版本，方便对象的默认初始化
      ApplyTorque(ActorId id, const geom::Vector3D &value)// 带参数的构造函数，用于创建一个ApplyTorque对象，传入要施加扭矩的对象的ActorId以及扭矩的具体数值（以三维向量表示）
        : actor(id),
          torque(value) {}
      ActorId actor;// 成员变量，表示要施加扭矩的对象的唯一标识符，用于在模拟场景中准确找到对应的对象
      geom::Vector3D torque;// 成员变量，表示要施加的扭矩值，以三维向量的形式体现扭矩在三个维度上的分量大小和方向
      MSGPACK_DEFINE_ARRAY(actor, torque); // 使用MSGPACK_DEFINE_ARRAY宏定义，可能是用于序列化或者和消息打包相关的操作，这里将actor和torque成员变量包含在定义中
    };

    struct SetSimulatePhysics : CommandBase<SetSimulatePhysics> {// SetSimulatePhysics结构体继承自CommandBase<SetSimulatePhysics>，用于控制某个对象是否开启物理模拟
      SetSimulatePhysics() = default;
      SetSimulatePhysics(ActorId id, bool value)// 带参数的构造函数，传入要设置物理模拟状态的对象的ActorId以及是否启用物理模拟的布尔值
        : actor(id),
          enabled(value) {}
      ActorId actor; // 成员变量，表示要设置物理模拟状态的对象的唯一标识符
      bool enabled;// 成员变量，代表是否启用物理模拟，true表示启用，false表示禁用
      MSGPACK_DEFINE_ARRAY(actor, enabled);
    };

    struct SetEnableGravity : CommandBase<SetEnableGravity> {// SetEnableGravity结构体继承自CommandBase<SetEnableGravity>，用于控制某个对象是否受重力影响
      SetEnableGravity() = default;
      SetEnableGravity(ActorId id, bool value)// 带参数的构造函数，传入要设置重力状态的对象的ActorId以及是否启用重力的布尔值
        : actor(id),
          enabled(value) {}
      ActorId actor;// 成员变量，表示要设置重力状态的对象的唯一标识符
      bool enabled;// 成员变量，代表是否启用重力，true表示启用，对象会受到重力作用；false表示禁用，对象不受重力影响
      MSGPACK_DEFINE_ARRAY(actor, enabled);
    };

    struct SetAutopilot : CommandBase<SetAutopilot> {// SetAutopilot结构体继承自CommandBase<SetAutopilot>，用于设置某个对象（可能是车辆等可自动驾驶的实体）的自动驾驶状态以及相关的通信端口信息
      SetAutopilot() = default;
      SetAutopilot(// 带参数的构造函数，传入要设置自动驾驶状态的对象的ActorId、是否启用自动驾驶的布尔值以及通信端口号（uint16_t类型）
          ActorId id,
          bool value,
          uint16_t tm_port)
        : actor(id),
          enabled(value),
          tm_port(tm_port) {}
      ActorId actor;// 成员变量，表示要设置自动驾驶状态的对象的唯一标识符
      bool enabled;// 成员变量，代表是否启用自动驾驶，true表示开启自动驾驶功能，false表示关闭
      uint16_t tm_port;// 成员变量，代表用于自动驾驶相关通信的端口号，用于和其他系统或模块进行数据交互等操作
      MSGPACK_DEFINE_ARRAY(actor, enabled);
    };

    struct ShowDebugTelemetry : CommandBase<ShowDebugTelemetry> {// ShowDebugTelemetry结构体继承自CommandBase<ShowDebugTelemetry>，用于控制是否显示某个对象的调试遥测信息
      ShowDebugTelemetry() = default;
      ShowDebugTelemetry(// 带参数的构造函数，传入要设置调试遥测显示状态的对象的ActorId以及是否显示的布尔值
          ActorId id,
          bool value)
        : actor(id),
          enabled(value) {}
      ActorId actor;// 成员变量，表示要设置调试遥测显示状态的对象的唯一标识符
      bool enabled;// 成员变量，代表是否显示调试遥测信息，true表示显示，false表示不显示
      MSGPACK_DEFINE_ARRAY(actor, enabled);
    };

    struct SetVehicleLightState : CommandBase<SetVehicleLightState> {// SetVehicleLightState结构体继承自CommandBase<SetVehicleLightState>，用于设置某个车辆对象的灯光状态
      SetVehicleLightState() = default;
      SetVehicleLightState(// 带参数的构造函数，传入要设置灯光状态的车辆对象的ActorId以及代表灯光状态的枚举类型值（VehicleLightState::flag_type）
          ActorId id,
          VehicleLightState::flag_type value)
        : actor(id),
          light_state(value) {}
      ActorId actor;// 成员变量，表示要设置灯光状态的车辆对象的唯一标识符
      VehicleLightState::flag_type light_state;// 成员变量，代表车辆的灯光状态，具体的取值和含义由VehicleLightState::flag_type枚举类型定义决定
      MSGPACK_DEFINE_ARRAY(actor, light_state);
    };

    struct ConsoleCommand : CommandBase<ConsoleCommand> {// ConsoleCommand结构体继承自CommandBase<ConsoleCommand>，它用于表示一个控制台命令相关的信息封装。
      ConsoleCommand() = default;// 默认构造函数，使用编译器自动生成的默认构造函数实现，创建一个默认初始化的ConsoleCommand对象。
      ConsoleCommand(std::string cmd) : cmd(cmd) {}// 带参数的构造函数，用于创建一个带有具体命令内容的ConsoleCommand对象。
      std::string cmd;// 成员变量，用于存储从控制台输入的具体命令内容。
      MSGPACK_DEFINE_ARRAY(cmd);// 使用MSGPACK_DEFINE_ARRAY宏来定义如何对ConsoleCommand结构体中的成员
    };

    struct SetTrafficLightState : CommandBase<SetTrafficLightState> {// SetTrafficLightState结构体继承自CommandBase<SetTrafficLightState>，用于表示设置交通信号灯状态的相关命令信息。
      SetTrafficLightState() = default;// 默认构造函数，利用编译器生成的默认构造函数来初始化对象，创建一个默认状态的SetTrafficLightState对象，通常意味着各成员变量都处于默认值（未明确设置的状态）。
      SetTrafficLightState(// 带参数的构造函数，用于创建一个明确指定要设置状态的交通信号灯以及设置的目标状态的命令对象。
          ActorId id,
          rpc::TrafficLightState state)
        : actor(id),
          traffic_light_state(state) {}
      ActorId actor;// 成员变量，代表要设置状态的交通信号灯在系统中的唯一标识符。
      rpc::TrafficLightState traffic_light_state;// 成员变量，用于存储要设置的交通信号灯的具体状态，其类型rpc::TrafficLightState定义了交通信号灯所有可能的状态值。
      MSGPACK_DEFINE_ARRAY(actor, traffic_light_state);// 使用MSGPACK_DEFINE_ARRAY宏来定义如何对SetTrafficLightState结构体中的成员（actor和traffic_light_state）进行MsgPack序列化和反序列化操作。
    };

    using CommandType = boost::variant2::variant<// 使用boost库中的variant2::variant类型创建一个类型别名CommandType。
        SpawnActor,
        DestroyActor,
        ApplyVehicleControl,
        ApplyVehicleAckermannControl,
        ApplyWalkerControl,
        ApplyVehiclePhysicsControl,
        ApplyTransform,
        ApplyWalkerState,
        ApplyTargetVelocity,
        ApplyTargetAngularVelocity,
        ApplyImpulse,
        ApplyForce,
        ApplyAngularImpulse,
        ApplyTorque,
        SetSimulatePhysics,
        SetEnableGravity,
        SetAutopilot,
        ShowDebugTelemetry,
        SetVehicleLightState,
        ApplyLocation,
        ConsoleCommand,
        SetTrafficLightState>;

    CommandType command;// 定义一个CommandType类型的变量command，它可以存储上述各种不同命令类型中的任意一种实例。

    MSGPACK_DEFINE_ARRAY(command);// 使用MSGPACK_DEFINE_ARRAY宏来定义如何对CommandType类型的变量command进行MsgPack序列化和反序列化操作。
  };

} // namespace rpc
} // namespace carla
