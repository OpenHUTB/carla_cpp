// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 版权声明部分，表明这段代码的版权归属以及其遵循的开源许可协议情况。此代码受 MIT 许可证条款的许可，若需要查看许可证的具体内容，可以通过对应的网址进行访问。
// For a copy, see <https://opensource.org/licenses/MIT>.

// 预处理指令，用于保证该头文件在一个编译单元中只会被包含一次，避免重复定义等编译错误。
#pragma once

// 包含 "WalkerControl.generated.h" 头文件，这通常是基于虚幻引擎等使用代码生成机制时所需要包含的头文件，它会包含一些自动生成代码相关的内容，比如用于支持反射等功能的代码，使得定义的结构体等类型能在蓝图系统等环境中正常使用。
#include "WalkerControl.generated.h"

// 使用 USTRUCT 宏定义结构体 FWalkerControl，同时指定其具有 BlueprintType 属性，意味着这个结构体可以在虚幻引擎的蓝图系统中使用，方便进行可视化编程和数据传递等操作。
USTRUCT(BlueprintType)
struct CARLA_API FWalkerControl
{
    GENERATED_BODY()

    // 使用 UPROPERTY 宏声明结构体的成员变量 Direction，将其归类到 "Walker Control" 类别下，在虚幻引擎的编辑器中，通过这个类别可以对相关属性进行组织管理。
    // EditAnywhere 表示该属性可以在编辑器的各种编辑模式下（比如蓝图编辑、细节面板编辑等）进行修改。
    // BlueprintReadWrite 表示该属性在蓝图系统中既能被读取也能被写入，即可以在蓝图中获取其值也可以对其值进行设置。
    // 该属性类型为 FVector，是虚幻引擎中表示三维向量的类型，这里初始化为 {1.0f, 0.0f, 0.0f}，表示在 X 轴正方向上的一个单位向量，可能用于表示行走者（Walker）的移动方向等含义。
    UPROPERTY(Category = "Walker Control", EditAnywhere, BlueprintReadWrite)
    FVector Direction = {1.0f, 0.0f, 0.0f};

    // 同样使用 UPROPERTY 宏声明结构体的成员变量 Speed，归类在 "Walker Control" 类别下，可在编辑器各处修改，在蓝图中可读可写。
    // 其类型为 float，用于表示速度，初始值为 0.0f，可能代表行走者的移动速度，单位或许根据具体游戏场景或模拟环境而定。
    UPROPERTY(Category = "Walker Control", EditAnywhere, BlueprintReadWrite)
    float Speed = 0.0f;

    // 使用 UPROPERTY 宏声明结构体的成员变量 Jump，归类在 "Walker Control" 类别下，可在编辑器各处修改，在蓝图中可读可写。
    // 其类型为 bool，用于表示是否跳跃的状态，初始值为 false，即默认情况下行走者不处于跳跃状态。
    UPROPERTY(Category = "Walker Control", EditAnywhere, BlueprintReadWrite)
    bool Jump = false;
};
