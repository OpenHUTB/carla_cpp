// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 这是一个预处理指令，用于确保头文件只被编译一次，避免重复包含导致的编译错误。

#include "Carla/Walker/WalkerBoneControlIn.h"
// 包含名为 "Carla/Walker/WalkerBoneControlIn.h" 的头文件，可能其中定义了与步行者骨骼控制输入相关的数据结构、类或函数等内容，具体取决于该头文件的实际定义。

#include "Carla/Walker/WalkerBoneControlOut.h"
// 包含 "Carla/Walker/WalkerBoneControlOut.h" 头文件，推测这里面包含了和步行者骨骼控制输出相关的一些定义，例如结构体、类声明或者函数原型等，用于后续在本类中处理相关的控制输出操作。
#include "Carla/Walker/WalkerControl.h"
// 包含 "Carla/Walker/WalkerControl.h" 头文件，大概率定义了与步行者整体控制相关的类型、函数等，可能是用于描述步行者的各种控制参数、行为等方面的内容，为本类实现步行者控制功能提供支持。

#include "CoreMinimal.h"
// 包含虚幻引擎（Unreal Engine）中的核心基础头文件，它提供了很多基本的类型定义、宏以及常用的基础功能等，是很多虚幻引擎项目代码的基础依赖头文件。
#include "GameFramework/Controller.h"
// 包含虚幻引擎中游戏框架（GameFramework）里的 Controller 类的头文件，Controller 类通常用于控制游戏中的角色（Pawn）等对象，
// 本类（AWalkerController）继承自 AController，所以需要包含这个头文件来获取父类相关的定义。

#include <compiler/disable-ue4-macros.h>
// 包含一个用于禁用虚幻引擎 4（UE4）相关特定宏的头文件，可能在某些编译场景下，需要临时关闭一些 UE4 自定义的宏来避免冲突或者满足特定的编译需求。
#include <carla/rpc/WalkerBoneControlIn.h>
// 包含名为 "carla/rpc/WalkerBoneControlIn.h" 的头文件，从路径和文件名推测，可能是和远程过程调用（RPC）中步行者骨骼控制输入相关的定义，
// 也许用于与外部系统或者其他模块通过 RPC 方式交互步行者骨骼控制输入信息。
#include <compiler/enable-ue4-macros.h>
// 包含用于重新启用虚幻引擎 4（UE4）相关特定宏的头文件，与之前的 disable-ue4-macros.h 相对应，在完成特定编译需求的宏禁用后，再恢复这些宏的启用状态。

#include "WalkerController.generated.h"
// 包含一个由虚幻引擎代码生成工具自动生成的头文件，里面包含了本类（AWalkerController）相关的一些额外代码，例如反射相关的代码、序列化相关代码等，
// 这些代码通常是根据类的定义自动生成的，帮助虚幻引擎更好地管理和使用这个类。

UCLASS()
// 使用 UCLASS 宏定义一个类，表明这是一个可以被虚幻引擎识别和管理的类，并且可以在蓝图中使用（具有相应的蓝图相关特性）。
class CARLA_API AWalkerController : public AController
// 定义名为 AWalkerController 的类，它继承自 AController（虚幻引擎中用于控制角色的基类），意味着它拥有 AController 类的所有成员和行为，
// 并且可以在此基础上扩展与步行者控制相关的特定功能，CARLA_API 可能是用于指定该类在整个 Carla 项目（推测是基于虚幻引擎开发的某个项目）中的对外接口访问权限等相关特性。
{
  GENERATED_BODY()
 // 这是一个由虚幻引擎代码生成系统使用的宏，用于自动生成类相关的必要代码，比如默认构造函数、析构函数等默认实现部分，以及其他一些和虚幻引擎内部机制相关的代码，
    // 使得类能符合虚幻引擎的要求正常工作。

public:

  AWalkerController(const FObjectInitializer &ObjectInitializer);
// 定义 AWalkerController 类的构造函数，接收一个 FObjectInitializer 类型的常量引用参数 ObjectInitializer，
    // FObjectInitializer 通常用于在创建对象时传递一些初始化相关的配置信息，通过这个构造函数可以根据传入的初始化配置来创建 AWalkerController 类的对象。

  void OnPossess(APawn *InPawn) override;
// 重写（override）了父类 AController 中的 OnPossess 函数，该函数在控制器占有（控制）一个 Pawn（游戏中的角色、棋子等代表可控制实体的对象）时被调用，
    // 在这里可以实现当本步行者控制器占有某个具体角色时需要执行的特定逻辑，例如初始化步行者相关的状态等操作。

  void Tick(float DeltaSeconds) override;
// 重写父类的 Tick 函数，Tick 函数会在每帧（游戏循环的每一帧）被调用，DeltaSeconds 参数表示从上一帧到当前帧所经过的时间间隔，
    // 在这个函数中可以实现每帧都需要更新的步行者控制相关逻辑，比如根据当前状态和时间间隔更新步行者的运动、姿态等。

  /// 允许的最大步行速度（以厘米/秒为单位）。
  UFUNCTION(BlueprintCallable)
// 使用 UFUNCTION 宏声明一个函数，该函数可以在蓝图中被调用（BlueprintCallable），意味着可以在虚幻引擎的蓝图可视化编程环境中使用这个函数来实现相关逻辑。
  float GetMaximumWalkSpeed() const
// 定义一个名为 GetMaximumWalkSpeed 的成员函数，返回类型为单精度浮点数（float），函数被声明为 const，表明调用该函数不会修改对象的状态，
    // 其功能是获取允许的最大步行速度，从代码中看单位是厘米/秒。
  {
    return 4096.0f; // ~147 km/h（40 米/秒）
// 函数体直接返回一个固定的浮点数 4096.0f，表示最大步行速度，注释中还额外说明了这个速度大约相当于 147 千米/小时（换算为 40 米/秒），方便阅读代码者理解速度的实际大小概念。
  }

  UFUNCTION(BlueprintCallable)
  void ApplyWalkerControl(const FWalkerControl &InControl);
// 使用 UFUNCTION 宏声明一个函数，可在蓝图中调用，函数名为 ApplyWalkerControl，接收一个 FWalkerControl 类型的常量引用参数 InControl，
    // 其功能应该是应用传入的步行者控制参数（通过 FWalkerControl 结构体或者类来表示具体的控制信息），实现对步行者具体行为、状态等方面的控制操作。

  UFUNCTION(BlueprintCallable)
  const FWalkerControl GetWalkerControl() const
// 使用 UFUNCTION 宏声明函数，可在蓝图中调用，定义名为 GetWalkerControl 的函数，返回类型是 const FWalkerControl（表示返回一个常量的 FWalkerControl 类型对象），
    // 函数同样被声明为 const，用于获取当前步行者的控制参数（返回当前设置的 FWalkerControl 对象），调用此函数不会改变对象自身的状态。
  {
    return Control;
// 直接返回类的私有成员变量 Control（FWalkerControl 类型），将当前的步行者控制参数返回给调用者。
  }

  UFUNCTION(BlueprintCallable)
  void GetBonesTransform(FWalkerBoneControlOut &WalkerBones);
// 使用 UFUNCTION 宏声明函数，可在蓝图中调用，函数名为 GetBonesTransform，接收一个 FWalkerBoneControlOut 类型的引用参数 WalkerBones，
    // 其功能可能是获取步行者骨骼的变换信息（比如骨骼的位置、旋转、缩放等变换相关的数据），并通过传入的引用参数 WalkerBones 将获取到的信息传递出去。


  UFUNCTION(BlueprintCallable)
  void SetBonesTransform(const FWalkerBoneControlIn &WalkerBones);
// 使用 UFUNCTION 宏声明函数，可在蓝图中调用，函数名为 SetBonesTransform，接收一个 FWalkerBoneControlIn 类型的常量引用参数 WalkerBones，
    // 用于设置步行者骨骼的变换信息，根据传入的 WalkerBones 参数中的具体内容来更新步行者骨骼相关的变换状态。

  UFUNCTION(BlueprintCallable)
  void BlendPose(float Blend);
 // 使用 UFUNCTION 宏声明函数，可在蓝图中调用，函数名为 BlendPose，接收一个单精度浮点数（float）参数 Blend，
    // 其功能可能是根据传入的 Blend 参数来混合（融合）步行者的姿态（例如可能是在不同动画姿态或者状态之间进行平滑过渡等相关操作）。

  UFUNCTION(BlueprintCallable)
  void GetPoseFromAnimation();
 // 使用 UFUNCTION 宏声明函数，可在蓝图中调用，函数名为 GetPoseFromAnimation，从函数名推测其功能是从动画中获取步行者的姿态信息，
    // 具体实现可能涉及到查找、解析相关动画数据来获取对应的姿态数据用于更新步行者当前的姿态状态。


private:

  FWalkerControl Control;
 // 定义一个私有成员变量 Control，类型为 FWalkerControl，用于存储步行者的控制参数，这个变量在类内部使用，外部无法直接访问，
    // 通过一些公有的函数（如 GetWalkerControl 和 ApplyWalkerControl 等）来间接操作这个变量的值，以实现对步行者控制参数的封装和管理。
};
