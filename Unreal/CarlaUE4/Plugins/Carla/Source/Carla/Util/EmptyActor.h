// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 预处理指令，其作用是确保该头文件在整个编译过程中只会被包含一次，避免同一个头文件被多次包含从而引发编译错误，这是C++中防止头文件重复包含的常用做法。

#include "GameFramework/Pawn.h"
// 引入 "GameFramework/Pawn.h" 头文件，该头文件中定义了 APawn 类相关的内容，意味着当前要定义的 AEmptyActor 类会继承自或者依赖 APawn 类所提供的功能、成员等基础架构，APawn 类通常是游戏中可被玩家或AI控制的角色等的基类。

#include "EmptyActor.generated.h"
// 包含 "EmptyActor.generated.h" 头文件，在虚幻引擎开发环境下，这类由引擎自动生成代码相关的头文件很重要，它会配合虚幻引擎的代码生成工具生成诸如反射、序列化等相关的必要代码，以此保证类能在虚幻引擎体系下正常工作，例如在蓝图中能进行相关操作、实现对象的持久化等功能。

UCLASS()
// 使用 UCLASS 宏来声明这是一个虚幻引擎中的类，通过这个宏，虚幻引擎会对该类执行一系列的处理，比如把类注册到引擎中，使其可以按照引擎规则来进行实例化、在蓝图里进行可视化编辑以及参与游戏运行时的各种机制等，此处括号内未添加额外参数，表明采用默认的类相关设置。
class CARLA_API AEmptyActor : public APawn
{
  GENERATED_BODY()
// 这个宏用于告知虚幻引擎的代码生成工具在当前位置生成对应的类相关代码体，主要涉及反射等机制相关内容，是保障类在虚幻引擎环境下能够正常实现像属性暴露、蓝图交互、序列化与反序列化等功能的关键所在。


public:
 // 类的公有部分，外部代码可以访问这部分声明的成员，一般用于定义类的构造函数、对外公开的接口等内容。
  AEmptyActor(const FObjectInitializer &ObjectInitializer);
 // 声明类的构造函数，它接受一个 FObjectInitializer 类型的常量引用作为参数，FObjectInitializer 通常用于在创建对象时传递一些初始化相关的配置信息，该构造函数用于初始化 AEmptyActor 类的对象实例。

private:
  // 类的私有部分，这部分声明的成员只能在类内部被访问，用于存放类的内部实现细节、不希望外部直接访问的数据等。

  UPROPERTY(Category="Empty Actor", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  USceneComponent* RootSceneComponent;
// 使用 UPROPERTY 宏声明了一个私有成员变量 RootSceneComponent，它是指向 USceneComponent 类型的指针。
    // UPROPERTY 宏用于将该变量标记为虚幻引擎中的属性，使得它可以受到引擎相关机制的管理。
    // Category="Empty Actor" 表示在虚幻引擎的属性面板（比如蓝图中查看属性时）中，该属性会归类到 "Empty Actor" 这个类别下；
    // VisibleAnywhere 意味着该属性在任何情况下（只要对象存在）都可见；
    // BlueprintReadOnly 表示该属性在蓝图中是只读的，不能被蓝图代码修改；
    // meta = (AllowPrivateAccess = "true") 则允许在蓝图中即使是私有属性也可以被访问（但不能修改，因为前面设置了 BlueprintReadOnly），这里的 RootSceneComponent 大概率是作为该 actor 的根场景组件，用于构建场景层级等相关用途。
};
