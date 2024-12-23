// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 版权声明部分，说明这段代码的版权归属为 Computer Vision Center (CVC) ，并且该代码基于 MIT 许可证进行开源授权，若想查看许可证具体内容，可以通过给定的网址进行访问。
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 预处理指令，用于确保该头文件在同一个编译单元中只会被包含一次，防止出现重复定义等编译问题，这是一种常见的 C/C++ 代码组织的方式。
#pragma once

// 包含 "GameFramework/Character.h" 头文件，这是虚幻引擎中定义角色（Character）相关基础类和功能的头文件，当前定义的 AWalkerBase 类继承自 ACharacter，所以需要包含这个头文件来获取父类相关的定义和功能支持。
#include "GameFramework/Character.h"

// 包含 "WalkerBase.generated.h" 头文件，这通常是在使用虚幻引擎的代码生成机制时需要包含的，它里面包含了为当前类自动生成的一些代码，例如与反射等功能相关的代码，以便该类能更好地在虚幻引擎环境（比如蓝图系统等）中使用。
#include "WalkerBase.generated.h"

// 使用 UCLASS 宏定义类 AWalkerBase，表明这是一个能被虚幻引擎识别并管理的类，并且它继承自 ACharacter 类，意味着它可以继承 ACharacter 类所提供的诸如移动、动画、碰撞等一系列与角色相关的基础功能和特性，在此基础上可以进一步扩展实现特定于行走者（Walker）的功能。
UCLASS()
class CARLA_API AWalkerBase : public ACharacter
{
    GENERATED_BODY()

    // 类 AWalkerBase 的构造函数声明，接受一个 FObjectInitializer 类型的参数，用于在对象初始化时传递一些初始化相关的配置信息，通过这个构造函数可以完成类对象创建时的特定初始化操作，不过这里只是声明，函数的具体实现可能在其他地方（比如对应的.cpp 文件中）。
    AWalkerBase(const FObjectInitializer &ObjectInitializer);
public:

    // 使用 UPROPERTY 宏声明一个名为 bAlive 的布尔类型的成员变量，将其归类到 "Walker Base" 类别下，在虚幻引擎的编辑器中，属性可以通过类别进行分组管理，方便查找和编辑。
    // 该属性具备 BlueprintReadWrite 和 EditAnywhere 的特性，意味着它在蓝图系统中既能被读取也能被修改，并且可以在编辑器的各种编辑场景（如细节面板等）下进行编辑，初始值被设置为 true，表示行走者初始状态是存活的，这个变量可能用于后续判断行走者是否存活的相关逻辑中。
    UPROPERTY(Category="Walker Base", BlueprintReadWrite, EditAnywhere)
    bool bAlive = true;

    // 使用 UPROPERTY 宏声明一个名为 AfterLifeSpan 的浮点型成员变量，同样归类在 "Walker Base" 类别下，在蓝图中可读可写，可在编辑器各处编辑，初始值为 10.0f，单位可能根据具体的游戏或模拟场景而定，它可能表示行走者在某种特定状态（比如死亡后）还能持续存在的时长等相关含义，用于控制生命周期相关逻辑。
    UPROPERTY(Category="Walker Base", BlueprintReadWrite, EditAnywhere)
    float AfterLifeSpan = 10.0f;

    // 使用 UPROPERTY 宏声明一个名为 bUsesWheelChair 的布尔类型成员变量，归属于 "Walker Base" 类别，在蓝图中可读可写，可在编辑器各处编辑，初始值为 false，它可能用于表示行走者是否使用轮椅的状态，用于区分不同行动方式的行走者等相关功能逻辑。
    UPROPERTY(Category="Walker Base", BlueprintReadWrite, EditAnywhere)
    bool bUsesWheelChair = false;

    // 使用 UFUNCTION 宏声明一个名为 StartDeathLifeSpan 的成员函数，并且标记为 BlueprintCallable，这意味着该函数可以在虚幻引擎的蓝图系统中被调用，方便通过可视化编程的方式触发相应功能逻辑。
    // 该函数的功能是调用父类（ACharacter）或者自身（如果有重定义的话）的 SetLifeSpan 函数，将当前行走者对象的生命周期设置为 AfterLifeSpan 变量所指定的值，可能用于在行走者开始进入特定死亡相关阶段时启动一个倒计时等相关生命周期管理的逻辑。
    UFUNCTION(BlueprintCallable)
    void StartDeathLifeSpan()
    {
        SetLifeSpan(AfterLifeSpan);
    }
};
