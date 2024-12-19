// 版权所有 （c） 2017 巴塞罗那自治大学 （UAB） 计算机视觉中心 （CVC）。

//本作品根据 MIT 许可证的条款进行许可。
//有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#pragma once
// 预处理器指令，确保该头文件在整个编译单元中只会被包含一次，避免重复定义问题

#include "Carla/Actor/ActorDefinition.h"
// 包含Carla项目中与参与者（Actor）定义相关的头文件，可能定义了描述参与者各种属性、类型等的结构体或类
#include "Carla/Actor/ActorDescription.h"
// 包含Carla项目中与参与者描述相关的头文件，用于表示参与者具体特征等信息
#include "Carla/Actor/ActorSpawnResult.h"
// 包含Carla项目中与参与者生成结果相关的头文件，应该包含了表示生成结果状态等内容的结构体

#include "Containers/Array.h"
// 引入UE4（Unreal Engine 4）中用于处理数组容器的头文件，方便操作数组类型的数据
#include "GameFramework/Actor.h"
// 包含UE4中基础的参与者（Actor）类的定义头文件，定义了Actor的基本行为、属性等基础功能，很多自定义的Actor类都会继承自它

#include "CarlaActorFactory.generated.h"
// 包含一个自动生成的头文件，通常与UE4的反射系统相关，用于支持蓝图等功能，使该类能被UE4的相关机制正确识别和处理

// 定义了一个名为ACarlaActorFactory的类，标记为抽象类（Abstract），它继承自AActor类（UE4中的基础Actor类）
// 该类属于CARLA_API命名空间，意味着它向外提供的接口等遵循CARLA相关的API规范
UCLASS(Abstract)  
class CARLA_API ACarlaActorFactory : public AActor
{
    GENERATED_BODY()
    // 这是UE4的宏，用于生成支持反射等功能所需的代码，具体由引擎的代码生成工具处理

public:
    // 构造函数，接受一个FObjectInitializer类型的参数ObjectInitializer
    // 通过调用父类（AActor）的构造函数并传入ObjectInitializer来完成初始化工作
    ACarlaActorFactory(const FObjectInitializer& ObjectInitializer)
        : Super(ObjectInitializer)
    {
        // 设置该Actor的主Tick（每帧更新）功能是否可以启用，这里设置为false，表示该Actor默认情况下不会每帧进行更新操作
        PrimaryActorTick.bCanEverTick = false;  
    }

    // 虚函数，用于检索此类能够生成的参与者（Actor）定义列表，返回值是一个TArray类型（数组类型）的FActorDefinition结构体
    // 目前函数体里调用了unimplemented()函数，表示该函数的具体实现还未完成（可能由子类去重写实现具体逻辑），暂时返回一个空的数组
    virtual TArray<FActorDefinition> GetDefinitions() {
        unimplemented();
        return {};
    }

    // 虚函数，用于基于给定的生成位置变换（SpawnAtTransform）和参与者描述（ActorDescription）来生成一个参与者
    // 要求传入的ActorDescription参数应该派生自通过GetDefinitions函数检索到的定义之一（这是前置条件，由调用者保证）
    // 同样，函数体里调用了unimplemented()函数，意味着具体生成参与者的逻辑还未实现，暂时返回一个空的FActorSpawnResult结构体
    // 该函数预期由子类重写来提供实际的参与者生成逻辑
    virtual FActorSpawnResult SpawnActor(
        const FTransform &SpawnAtTransform,
        const FActorDescription &ActorDescription) {
        unimplemented();
        return {};
    }
};
