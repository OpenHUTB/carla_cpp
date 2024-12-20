// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 包含用于生成代码相关内容的头文件，这里用于生成与当前结构体相关的代码
#include "AckermannControllerSettings.generated.h"

// 使用USTRUCT宏定义一个结构体，标记为可在蓝图中使用（BlueprintType），该结构体属于CARLA_API这个命名空间（假设CARLA_API是相关项目定义的命名空间相关宏）
USTRUCT(BlueprintType)
struct CARLA_API FAckermannControllerSettings
{
    GENERATED_BODY()

    // UPROPERTY宏用于将结构体中的成员变量暴露给UE的属性系统，使其可以在编辑器中编辑等。
    // 此属性属于"Ackermann Controller Settings"分类，可在任何地方编辑（EditAnywhere），并且在蓝图中可读可写（BlueprintReadWrite）
    // SpeedKp是一个用于速度控制的比例系数（Proportional），初始值设为0.0f
    UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
    float SpeedKp = 0.0f;

    // 同样属于"Ackermann Controller Settings"分类，可在任何地方编辑、在蓝图中可读可写的属性，
    // 用于速度控制的积分系数（Integral），初始值为0.0f，通常在PID控制器等相关控制逻辑中使用
    UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
    float SpeedKi = 0.0f;

    // 属于"Ackermann Controller Settings"分类，可在任何地方编辑、在蓝图中可读可写的属性，
    // 用于速度控制的微分系数（Derivative），初始值设为0.0f，和速度控制的PID逻辑相关
    UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
    float SpeedKd = 0.0f;

    // 属于"Ackermann Controller Settings"分类，可在任何地方编辑、在蓝图中可读可写的属性，
    // 用于加速度控制的比例系数（Proportional），初始值设为0.0f，可能在基于加速度控制的相关逻辑中起作用
    UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
    float AccelKp = 0.0f;

    // 属于"Ackermann Controller Settings"分类，可在任何地方编辑、在蓝图中可读可写的属性，
    // 用于加速度控制的积分系数（Integral），初始值设为0.0f，在加速度控制相关的PID等控制策略里会用到
    UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
    float AccelKi = 0.0f;

    // 属于"Ackermann Controller Settings"分类，可在任何地方编辑、在蓝图中可读可写的属性，
    // 用于加速度控制的微分系数（Derivative），初始值设为0.0f，是加速度控制相关逻辑的一部分，比如在PID控制中
    UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
    float AccelKd = 0.0f;
};
