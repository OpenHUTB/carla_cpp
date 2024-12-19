// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 预编译头文件保护指令，确保该头文件在同一个编译单元中只会被包含一次，避免出现重复定义等问题
#pragma once

// 包含用于UE4反射系统生成相关代码的头文件，使得下面定义的 FVehicleControl 结构体能够被UE4的反射机制正确处理，例如在蓝图中使用等
#include "VehicleControl.generated.h"

// 使用USTRUCT宏定义一个结构体，标记为BlueprintType 意味着这个结构体可以在UE4的蓝图可视化编程环境中使用，方便进行数据传递和操作等
USTRUCT(BlueprintType)
struct CARLA_API FVehicleControl
{
    // 由UE4的反射系统生成的代码相关宏，用于自动生成结构体的一些必要的底层代码，比如序列化等相关代码，确保结构体能正确参与UE4中的各种操作
    GENERATED_BODY()

    // UPROPERTY宏用于将结构体中的成员变量暴露给UE4的属性系统，使得这些变量可以在UE4编辑器中显示和编辑，同时可以设置相应属性。
    // 此成员变量属于"Vehicle Control"分类，在编辑器中可以在任何地方进行编辑（EditAnywhere），并且在蓝图中可读可写（BlueprintReadWrite），
    // 用于表示车辆的油门开度，取值范围通常是0.0到1.0（这里初始值设为0.0f，表示油门未踩下的状态）
    UPROPERTY(Category = "Vehicle Control", EditAnywhere, BlueprintReadWrite)
    float Throttle = 0.0f;

    // 同样使用UPROPERTY宏暴露的成员变量，属于"Vehicle Control"分类，可在编辑器任意处编辑、蓝图中可读可写，
    // 用于表示车辆的转向角度，取值范围通常根据具体游戏设定，一般是一个合理的角度范围（这里初始值为0.0f，表示车辆处于直线行驶状态）
    UPROPERTY(Category = "Vehicle Control", EditAnywhere, BlueprintReadWrite)
    float Steer = 0.0f;

    // 还是通过UPROPERTY宏暴露的成员变量，属于"Vehicle Control"分类，可在编辑器任意处编辑、蓝图中可读可写，
    // 用于表示车辆的刹车力度，取值范围通常是0.0到1.0（这里初始值设为0.0f，表示未踩刹车的状态）
    UPROPERTY(Category = "Vehicle Control", EditAnywhere, BlueprintReadWrite)
    float Brake = 0.0f;

    // 借助UPROPERTY宏定义的成员变量，属于"Vehicle Control"分类，可在编辑器任意处编辑、蓝图中可读可写，
    // 用于表示车辆的手刹是否拉起，初始值为false，表示手刹未拉起的状态
    UPROPERTY(Category = "Vehicle Control", EditAnywhere, BlueprintReadWrite)
    bool bHandBrake = false;

    // 通过UPROPERTY宏定义的成员变量，属于"Vehicle Control"分类，可在编辑器任意处编辑、蓝图中可读可写，
    // 用于表示车辆是否处于倒车状态，初始值为false，表示车辆正处于前进状态
    UPROPERTY(Category = "Vehicle Control", EditAnywhere, BlueprintReadWrite)
    bool bReverse = false;

    // 同样使用UPROPERTY宏定义的成员变量，属于"Vehicle Control"分类，可在编辑器任意处编辑、蓝图中可读可写，
    // 用于表示车辆是否处于手动换挡模式，初始值为false，表示车辆处于自动换挡模式
    UPROPERTY(Category = "Vehicle Control", EditAnywhere, BlueprintReadWrite)
    bool bManualGearShift = false;

    // 此成员变量也通过UPROPERTY宏暴露，属于"Vehicle Control"分类，可在编辑器任意处编辑、蓝图中可读可写，
    // 并且设置了元数据（meta）中的编辑条件（EditCondition）为 bManualGearShift，意味着只有当 bManualGearShift 为true（即车辆处于手动换挡模式）时，
    // 这个变量才可以在编辑器中进行编辑，用于表示车辆当前所处的挡位，初始值为0（具体挡位含义根据游戏内设定，一般0可能表示空挡等情况）
    UPROPERTY(Category = "Vehicle Control", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = bManualGearShift))
    int32 Gear = 0;
};
