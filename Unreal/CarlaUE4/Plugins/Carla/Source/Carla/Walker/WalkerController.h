// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这是一个预编译头文件保护指令，确保该头文件内容在多次包含时只被编译一次
#pragma once

// 包含与行人骨骼控制输入相关的头文件，可能定义了用于控制行人骨骼的输入数据结构等内容
#include "Carla/Walker/WalkerBoneControlIn.h"
// 包含与行人骨骼控制输出相关的头文件，也许用于获取行人骨骼的变换等输出信息
#include "Carla/Walker/WalkerBoneControlOut.h"
// 包含行人控制相关的头文件，推测是定义了对行人整体行为控制的相关结构体或类等
#include "Carla/Walker/WalkerControl.h"

// 包含UE4核心最小化相关的头文件，提供了UE4引擎基础的一些常用功能和类型定义等
#include "CoreMinimal.h"
// 包含游戏框架中控制器相关的头文件，AWalkerController类继承自这个基类，用于实现对游戏角色的控制逻辑
#include "GameFramework/Controller.h"

// 以下两个头文件可能是用于处理一些编译相关的宏定义开关，暂时不清楚具体的详细用途，但大概是和特定编译器环境下的宏处理有关
#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/WalkerBoneControlIn.h>
#include <compiler/enable-ue4-macros.h>

// 这是UE4的反射相关宏，用于生成类的相关反射信息，使得该类可以在UE4的蓝图系统等中被识别和使用
#include "WalkerController.generated.h"

// 定义了一个名为AWalkerController的类，它继承自UE4的AController类，意味着它可以用来控制游戏中的角色（在这里应该是行人角色）
UCLASS()
class CARLA_API AWalkerController : public AController
{
    // 这是UE4用于生成类的必要宏，用于处理类的反射相关代码生成等操作
    GENERATED_BODY()

public:
    // 构造函数，接收一个FObjectInitializer类型的参数，用于初始化对象，通常在创建该类实例时被调用
    AWalkerController(const FObjectInitializer &ObjectInitializer);

    // 重写了AController类中的OnPossess函数，当该控制器占有（控制）一个Pawn（游戏中的角色实体）时会被调用，在这里可以进行一些初始化控制相关的操作
    void OnPossess(APawn *InPawn) override;

    // 重写了AController类中的Tick函数，每一帧都会被调用，通常用于实现每帧需要更新的控制逻辑，比如根据输入等更新行人的状态
    void Tick(float DeltaSeconds) override;

    /// 允许的最大步行速度（以厘米/秒为单位）。
    // 这是一个可以在蓝图中被调用的函数（通过BlueprintCallable标记），用于获取行人允许的最大步行速度，返回值是一个固定的浮点数，约等于147千米/小时（换算为40米/秒，这里以厘米/秒为单位表示就是4096.0f）
    UFUNCTION(BlueprintCallable)
    float GetMaximumWalkSpeed() const
    {
        return 4096.0f; // ~147 km/h（40 米/秒）
    }

    // 这是一个可以在蓝图中被调用的函数，用于应用传入的行人控制数据（FWalkerControl类型），可能会根据传入的数据来改变行人的行为，比如行走方向、速度等
    UFUNCTION(BlueprintCallable)
    void ApplyWalkerControl(const FWalkerControl &InControl);

    // 这是一个可以在蓝图中被调用的函数，用于获取当前行人的控制数据（FWalkerControl类型），返回当前存储的控制信息
    UFUNCTION(BlueprintCallable)
    const FWalkerControl GetWalkerControl() const
    {
        return Control;
    }

    // 这是一个可以在蓝图中被调用的函数，用于获取行人骨骼的变换信息，将结果填充到传入的FWalkerBoneControlOut类型的参数中，可能用于获取当前行人骨骼的姿势等情况
    UFUNCTION(BlueprintCallable)
    void GetBonesTransform(FWalkerBoneControlOut &WalkerBones);

    // 这是一个可以在蓝图中被调用的函数，用于设置行人骨骼的变换信息，根据传入的FWalkerBoneControlIn类型的参数来改变行人骨骼的状态，比如调整姿势等
    UFUNCTION(BlueprintCallable)
    void SetBonesTransform(const FWalkerBoneControlIn &WalkerBones);

    // 这是一个可以在蓝图中被调用的函数，用于混合（融合）姿势，传入一个混合系数Blend，具体如何混合姿势可能在函数内部有相应实现逻辑，可能用于实现动画过渡等效果
    UFUNCTION(BlueprintCallable)
    void BlendPose(float Blend);

    // 这是一个可以在蓝图中被调用的函数，推测是从动画中获取行人的当前姿势，具体获取方式应该在函数内部有实现，可能用于同步动画和实际的行人表现等
    UFUNCTION(BlueprintCallable)
    void GetPoseFromAnimation();

private:
    // 一个FWalkerControl类型的成员变量，用于存储当前行人的控制信息，比如速度、方向等相关数据，在类的多个函数中可能会对其进行操作和使用
    FWalkerControl Control;
};
