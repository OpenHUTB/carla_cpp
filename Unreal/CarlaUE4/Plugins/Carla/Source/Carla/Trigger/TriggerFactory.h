// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这是一个预处理指令，用于确保头文件只被包含一次，避免重复定义等问题
#pragma once

// 包含 "Carla/Actor/ActorSpawnResult.h" 头文件，该文件可能定义了与 actor 生成结果相关的结构体或类等，比如生成是否成功、生成的 actor 的相关属性等信息
#include "Carla/Actor/ActorSpawnResult.h"
// 包含 "Carla/Actor/CarlaActorFactory.h" 头文件，推测这里面定义了 Carla 中 actor 工厂的基础类或者相关接口，当前类 ATriggerFactory 可能继承自它来实现特定的 actor 生产功能
#include "Carla/Actor/CarlaActorFactory.h"

// 包含生成代码相关的头文件，这应该是基于虚幻引擎的代码生成机制相关的，用于生成下面定义的类的一些必要的代码，例如反射相关代码等
#include "TriggerFactory.generated.h"

/// 负责制造传感器的工厂。这个工厂能够生成在 carla::sensor::SensorRegistry 中注册的每个传感器。
// 声明 ATriggerFactory 类，它继承自 ACarlaActorFactory，意味着它可以利用父类提供的一些基础功能来创建 actor，并且在这里它主要用于创建传感器类型的 actor
UCLASS()
class CARLA_API ATriggerFactory : public ACarlaActorFactory
{
    GENERATED_BODY()

    // 下面这个函数用于检索在 SensorRegistry 中注册的所有传感器的定义。
    // 传感器必须实现 GetSensorDefinition() 静态方法，这样该函数才能正确获取到每个传感器对应的定义信息，返回值是一个包含 FActorDefinition 元素的数组，用于存放获取到的传感器定义
    TArray<FActorDefinition> GetDefinitions() final;

    // 这个函数用于根据给定的变换信息（SpawnAtTransform，比如生成的位置、旋转等）以及 actor 描述信息（ActorDescription，包含了要生成的 actor 的各种属性等内容）来实际生成一个 actor。
    // 它返回一个 FActorSpawnResult 类型的结果，用于表示 actor 生成的情况，例如是否成功、若成功对应的 actor 实例等相关信息
    FActorSpawnResult SpawnActor(
        const FTransform &SpawnAtTransform,
        const FActorDescription &ActorDescription) final;

};
