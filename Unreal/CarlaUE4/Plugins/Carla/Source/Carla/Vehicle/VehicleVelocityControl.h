// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 这是一个预处理指令，用于确保该头文件在整个编译单元中只会被编译一次，避免重复包含导致的编译错误，比如重复定义类、变量等问题。

#include "Components/ActorComponent.h"
// 引入 "Components/ActorComponent.h" 头文件，ActorComponent 是虚幻引擎（Unreal Engine）中用于定义游戏对象（Actor）组件的基类相关的头文件，
// 本类（UVehicleVelocityControl）继承自 UActorComponent，所以需要包含此头文件来获取基类相关的定义以及依赖的一些基础功能等内容。
#include "Components/PrimitiveComponent.h"
// 包含 "Components/PrimitiveComponent.h" 头文件，PrimitiveComponent 通常是用于表示具有基本几何形状、可进行物理模拟等基础特性的组件类相关的头文件，
// 可能在本类中会与这类基础组件进行交互或者基于其实现某些功能，比如获取物体的物理属性等来控制速度等相关操作。
#include "CoreMinimal.h"
// 引入虚幻引擎中的核心基础头文件，它提供了很多基本的类型定义、宏以及常用的基础功能等，是很多虚幻引擎项目代码的基础依赖头文件，
// 例如一些常用的数据类型、引擎基础的常量、函数等都在这个头文件或者由其引入的其他头文件中定义，确保代码能使用这些基础功能。

#include "VehicleVelocityControl.generated.h"
// 包含一个由虚幻引擎代码生成工具自动生成的头文件，里面包含了本类（UVehicleVelocityControl）相关的一些额外代码，例如反射相关的代码、序列化相关代码等，
// 这些代码通常是根据类的定义自动生成的，帮助虚幻引擎更好地管理和使用这个类，比如能在蓝图中可视化地操作该类的实例等。

/// Component that controls that the velocity of an actor is constant.
// 这是一段 Doxygen 风格的注释，用于对下面定义的 UVehicleVelocityControl 类进行简要说明，表明这个组件类的功能是控制游戏中一个 Actor（角色、物体等游戏实体）的速度保持恒定。

UCLASS(Blueprintable,BlueprintType,ClassGroup=(Custom),meta=(BlueprintSpawnableComponent))
// 使用 UCLASS 宏定义一个类，表明这是一个可以被虚幻引擎识别和管理的类，并且具有以下特性：
// - Blueprintable：表示该类可以在虚幻引擎的蓝图可视化编程环境中进行扩展和定制，例如可以基于这个类创建蓝图类来添加额外的功能逻辑等。
// - BlueprintType：意味着这个类的实例可以作为变量类型在蓝图中使用，方便在蓝图中传递、操作该类的对象。
// - ClassGroup=(Custom)：将这个类归到名为 "Custom" 的类组中，方便在虚幻引擎的相关界面（如编辑器中的类查看窗口等）中对类进行分类管理和查找。
// - meta=(BlueprintSpawnableComponent)：指定了元数据信息，表明这个组件类可以在蓝图中被生成（作为组件添加到其他 Actor 上），用于在蓝图中方便地创建和使用该组件。
class CARLA_API UVehicleVelocityControl : public UActorComponent
// 定义名为 UVehicleVelocityControl 的类，它继承自 UActorComponent（虚幻引擎中用于定义游戏对象组件的基类），意味着它拥有 UActorComponent 类的所有成员和行为，
// 并且可以在此基础上扩展与控制车辆速度相关的特定功能，CARLA_API 可能是用于指定该类在整个 Carla 项目（推测是基于虚幻引擎开发的某个项目）中的对外接口访问权限等相关特性。
{
  GENERATED_BODY()
// 这是一个由虚幻引擎代码生成系统使用的宏，用于自动生成类相关的必要代码，比如默认构造函数、析构函数等默认实现部分，以及其他一些和虚幻引擎内部机制相关的代码，
    // 使得类能符合虚幻引擎的要求正常工作。

  // ===========================================================================
  /// @name Constructor and destructor
  // ===========================================================================
  /// @{
public:
  UVehicleVelocityControl();
 // 声明 UVehicleVelocityControl 类的构造函数，用于创建该类的对象，在构造函数中通常会进行一些成员变量的初始化等操作，
    // 具体的初始化逻辑需要在函数体（如果有定义的话，当前这里只是声明，函数体可能在对应的源文件中定义）中实现，比如初始化与速度控制相关的一些初始状态等。

  /// @}
  // ===========================================================================
  /// @name Get functions
  // ===========================================================================
  /// @{
public:

  void BeginPlay() override;
// 重写（override）了父类（UActorComponent）中的 BeginPlay 函数，该函数在游戏开始运行，这个组件所在的 Actor 被创建并开始参与游戏逻辑时被调用，
    // 在这里可以实现一些初始化相关的操作，比如获取初始的速度信息、查找关联的其他组件等准备工作，为后续控制速度的功能做好铺垫。

  virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
// 重写父类的 TickComponent 函数，TickComponent 函数会在每帧（游戏循环的每一帧）被调用，用于执行每帧都需要更新的逻辑，
    // 接收三个参数：DeltaTime 表示从上一帧到当前帧所经过的时间间隔，TickType 是一个枚举类型，用于表示当前是哪种类型的游戏层级（Level）的 Tick（更新），ThisTickFunction 是一个指向 Actor 组件 Tick 函数相关信息的指针，
    // 在这个函数中可以根据时间间隔（DeltaTime）等来更新车辆速度相关的状态，实现速度控制逻辑，确保速度保持恒定等操作。

  // Activate the component setting the target velocity
  virtual void Activate(bool bReset=false) override;
// 重写父类的 Activate 函数，用于激活这个组件，并且可以通过传入的布尔类型参数 bReset（默认值为 false）来决定是否重置某些相关状态，
    // 激活组件时可能会启动速度控制相关的机制，比如开始监测和调整车辆的速度使其达到目标速度等操作，具体逻辑要看函数体的实现。

  // Activate the component setting the target velocity
  virtual void Activate(FVector Velocity, bool bReset=false);
 // 这是另一个 Activate 函数的重载版本，接收一个 FVector 类型的参数 Velocity（在虚幻引擎中通常用于表示三维向量，这里可能用于指定目标速度的向量值）和一个可选的布尔参数 bReset（默认值为 false），
    // 同样用于激活组件并设置目标速度，通过传入具体的速度向量来明确要将车辆速度控制到的目标状态，根据 bReset 参数决定是否重置相关状态，重载函数提供了更灵活的激活组件并设置速度的方式。

  // Deactivate the component
  virtual void Deactivate() override;
// 重写父类的 Deactivate 函数，用于停用（关闭）这个组件，当组件被停用后，可能会停止对车辆速度的控制操作，释放相关资源等，具体的行为取决于函数体中的实现逻辑。

private:
  ///
  UPROPERTY(Category = "Vehicle Velocity Control", VisibleAnywhere)
  FVector TargetVelocity;
// 定义一个私有成员变量 TargetVelocity，类型为 FVector（三维向量），用于存储目标速度信息，通过 UPROPERTY 宏进行标记，
    // 其中 Category 属性指定了在虚幻引擎编辑器的属性面板中该变量所属的分类为 "Vehicle Velocity Control"，VisibleAnywhere 表示该变量在任何地方（只要有权限访问该组件）都可见，
    // 这样在编辑器中可以方便地查看和设置这个目标速度的值，用于控制车辆的速度。

  UPrimitiveComponent* PrimitiveComponent;
// 定义一个私有指针类型成员变量 PrimitiveComponent，指向 UPrimitiveComponent 类型的对象，可能用于与具有基本几何形状、可进行物理模拟等特性的组件进行交互，
    // 例如获取该组件关联的物理实体的一些属性来辅助实现速度控制功能，比如获取物体的质量、摩擦力等信息用于计算和调整速度等操作。
  AActor* OwnerVehicle;
// 定义一个私有指针类型成员变量 OwnerVehicle，指向 AActor 类型的对象，用于存储拥有这个速度控制组件的车辆（Actor）的指针，
    // 通过这个指针可以访问车辆相关的其他属性、组件等信息，方便在速度控制逻辑中与车辆的整体状态进行交互，例如获取车辆当前速度等信息来进行对比和调整。

};
