// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这是一个预编译头文件保护指令，确保该头文件内容在一个编译单元中只被包含一次
#pragma once

// 包含自动生成相关的头文件，通常用于与虚幻引擎的反射系统配合，实现一些如蓝图相关的功能等
#include "WalkerBoneControlOut.generated.h"

// 使用 USTRUCT 宏定义了一个结构体，标记为可以在蓝图中使用（BlueprintType）
// 该结构体用于存储与角色行走相关的骨骼控制输出数据
USTRUCT(BlueprintType)
struct CARLA_API FWalkerBoneControlOutData
{
    GENERATED_BODY()
    // 表示骨骼在世界空间中的变换信息（位置、旋转、缩放等）
    FTransform World;
    // 表示骨骼在组件空间中的变换信息，可能相对于所属的某个组件而言
    FTransform Component;
    // 表示骨骼相对于某个父骨骼或者参考骨骼的相对变换信息
    FTransform Relative;
};

// 同样使用 USTRUCT 宏定义的结构体，也标记为可在蓝图中使用（BlueprintType）
// 整体用于封装多个骨骼的控制输出相关数据
USTRUCT(BlueprintType)
struct CARLA_API FWalkerBoneControlOut
{
    GENERATED_BODY()

    // 一个属性声明，用于存储骨骼变换数据的映射表。
    // 键是 FString 类型，代表骨骼的名称或者标识等，值是 FWalkerBoneControlOutData 结构体类型，包含了具体该骨骼的多种变换信息。
    // 该属性所属类别为 "Walker Bone Control"，可以在编辑器中任何地方编辑（EditAnywhere），并且可以在蓝图中读写（BlueprintReadWrite）
    UPROPERTY(Category = "Walker Bone Control", EditAnywhere, BlueprintReadWrite)
    TMap<FString, FWalkerBoneControlOutData> BoneTransforms;

};
