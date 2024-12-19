// 版权所有 （c） 2017 巴塞罗那自治大学 （UAB） 计算机视觉中心 （CVC）。
//
// 本作品根据 MIT 许可证的条款进行许可。
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#pragma once
// 这是一个预处理器指令，用于确保头文件只被包含一次，避免重复定义等问题

#include "ActorSpawnResult.generated.h"
// 包含一个自动生成的头文件，通常与UE4（Unreal Engine 4）的反射系统相关，用于支持蓝图等功能

// 定义了一个名为EActorSpawnResultStatus的枚举类型，用于表示参与者属性的有效类型列表
// 并且标记为可在蓝图中使用（BlueprintType）
UENUM(BlueprintType)  
enum class EActorSpawnResultStatus : uint8
{
    // 表示生成参与者成功的状态，通过元数据（UMETA）设置在蓝图中显示的名称为 "Success"
    Success                      UMETA(DisplayName = "Success"),  
    // 表示参与者描述无效的状态，在蓝图中显示名称为 "Invalid actor description"
    InvalidDescription           UMETA(DisplayName = "Invalid actor description"),  
    // 表示因为在生成位置发生碰撞而导致生成失败的状态，蓝图中显示名称为相应字符串
    Collision                    UMETA(DisplayName = "Failed because collision at spawn position"),  
    // 表示未知错误的状态，蓝图中显示名称为 "Unknown Error"
    UnknownError                 UMETA(DisplayName = "Unknown Error"),  

    // 这个枚举成员通常用于表示枚举的大小等内部用途，标记为隐藏（Hidden），不在蓝图等外部可见
    SIZE                         UMETA(Hidden)  
};

// 定义了一个名为FActorSpawnResult的结构体，标记为可在蓝图中使用（BlueprintType）
// 该结构体用于表示参与者生成函数的结果
USTRUCT(BlueprintType)  
struct FActorSpawnResult
{
    GENERATED_BODY()
    // 这是UE4的宏，用于生成支持反射等功能所需的代码，具体由引擎的代码生成工具处理

    // 默认构造函数，使用默认的初始化方式
    FActorSpawnResult() = default;  

    // 显式构造函数，接受一个AActor类型的指针参数InActor
    // 根据传入的指针是否为空来初始化结构体中的成员变量Status
    // 如果传入的指针不为空，则状态设为Success，否则设为UnknownError
    explicit FActorSpawnResult(AActor *InActor)
        : Actor(InActor),
          Status(Actor!= nullptr?
              EActorSpawnResultStatus::Success :
              EActorSpawnResultStatus::UnknownError) {}

    // 静态函数，用于将EActorSpawnResultStatus类型的状态转换为FString类型的字符串表示
    static FString StatusToString(EActorSpawnResultStatus Status);  

    // 成员函数，用于判断当前的FActorSpawnResult对象是否有效
    // 有效条件是Actor指针不为空且状态为Success
    bool IsValid() const
    {
        return (Actor!= nullptr) && (Status == EActorSpawnResultStatus::Success);
    }

    // 声明一个可编辑（EditAnywhere）且可在蓝图中读写（BlueprintReadWrite）的属性
    // 用于存储指向AActor的指针，初始化为nullptr
    UPROPERTY(EditAnywhere, BlueprintReadWrite)  
    AActor *Actor = nullptr;

    // 同样声明一个可编辑且可在蓝图中读写的属性，用于存储参与者生成结果的状态
    // 初始化为UnknownError状态
    UPROPERTY(EditAnywhere, BlueprintReadWrite)  
    EActorSpawnResultStatus Status = EActorSpawnResultStatus::UnknownError;
};
