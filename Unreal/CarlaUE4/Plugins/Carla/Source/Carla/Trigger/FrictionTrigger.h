// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 引入虚幻引擎的基础Actor类头文件，用于创建游戏中的基础可放置对象，包含了Actor相关的基本功能和定义
#include "GameFramework/Actor.h"
// 引入盒体组件（BoxComponent）的头文件，常用于定义具有盒体形状碰撞、触发等功能的组件
#include "Components/BoxComponent.h"
// 引入Carla游戏中与关卡（Episode）相关的头文件，可能包含了关卡相关的信息、操作等内容
#include "Carla/Game/CarlaEpisode.h"

// 引入用于自动生成代码相关的宏定义头文件，通常和UE4/UE5的反射系统配合使用，用于实现类的序列化、蓝图可见性等功能
#include "FrictionTrigger.generated.h"

// 使用UE4/UE5的宏定义来声明一个类，使其可以被引擎识别并参与到诸如反射、蓝图等相关机制中
// CARLA_API 可能是用于指定该类在Carla项目中的对外接口可见性等相关设置
// AFrictionTrigger类继承自AActor，意味着它可以作为游戏世界中的一个独立对象存在，具备Actor的基本特性和功能
UCLASS()
class CARLA_API AFrictionTrigger : public AActor
{
    GENERATED_BODY()

private:
    // 私有函数声明，用于初始化相关的内部状态、资源等操作，具体实现应该在类的源文件中定义
    void Init();
    // 私有函数声明，用于更新其他Actor（车轮相关可能）的摩擦力，传入与之重叠的其他Actor指针以及用于存储新摩擦力值的数组引用
    void UpdateWheelsFriction(AActor *OtherActor, TArray<float>& NewFriction);

public:
    // AFrictionTrigger类的构造函数，接受一个FObjectInitializer类型的参数，用于初始化对象的各种属性和组件等，
    // 通过这个构造函数可以按照指定的初始化规则来创建该类的实例
    AFrictionTrigger(const FObjectInitializer &ObjectInitializer);

    // 声明一个函数为可在蓝图中调用的函数（UFUNCTION宏的作用），此函数用于处理触发体开始重叠事件，
    // 当有其他Actor开始与该触发体（比如基于盒体组件定义的触发区域）重叠时会被调用，传入了多个参数，
    // 包括重叠的组件、重叠的其他Actor、其他Actor的对应组件、其他Actor的身体索引、是否来自扫掠以及碰撞结果等信息
    UFUNCTION()
    void OnTriggerBeginOverlap(
        UPrimitiveComponent *OverlappedComp,
        AActor *OtherActor,
        UPrimitiveComponent *OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult &SweepResult);

    // 同样是声明一个可在蓝图中调用的函数，用于处理触发体结束重叠事件，当其他Actor与该触发体结束重叠时会被调用，
    // 传入了重叠的组件、重叠的其他Actor、其他Actor的对应组件以及其他Actor的身体索引等参数
    UFUNCTION()
    void OnTriggerEndOverlap(
        UPrimitiveComponent *OverlappedComp,
        AActor *OtherActor,
        UPrimitiveComponent *OtherComp,
        int32 OtherBodyIndex);

    // 设置当前触发体所属的关卡（Episode），通过传入一个UCarlaEpisode类型的引用，将内部的Episode指针指向传入的关卡对象，
    // 使得触发体可以与特定的游戏关卡相关联，获取关卡相关的信息等
    void SetEpisode(const UCarlaEpisode &InEpisode)
    {
        Episode = &InEpisode;
    }

    // 设置触发体（可能基于盒体组件定义的触发区域）的范围（盒体的大小），传入一个FVector类型的参数来指定新的范围大小，
    // 通过调用TriggerVolume（盒体组件）的SetBoxExtent函数来实际改变触发区域的大小
    void SetBoxExtent(const FVector &Extent)
    {
        TriggerVolume->SetBoxExtent(Extent);
    }

    // 设置触发体对应的摩擦力值，传入一个浮点数参数，将其赋值给内部的Friction成员变量，用于后续影响相关的物理交互等情况
    void SetFriction(float NewFriction)
    {
        Friction = NewFriction;
    }

protected:
    // 重写Actor的BeginPlay函数，当该Actor首次进入游戏世界并开始运行时会被调用，用于进行一些初始化、资源加载等操作，
    // 在这里可以进行与该触发体相关的初始设置，比如初始化一些成员变量、注册事件等
    virtual void BeginPlay() override;

    // 重写Actor的EndPlay函数，当该Actor从游戏世界中移除（比如销毁、关卡切换等原因）时会被调用，
    // 可以在这里进行资源清理、取消注册事件等收尾工作，传入一个EEndPlayReason::Type类型的参数来说明结束的原因
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

    // 重写Actor的Tick函数，每帧都会被调用，传入一个表示每帧时间间隔（DeltaTime）的浮点数参数，
    // 可以在这里进行每帧都需要更新的逻辑，比如根据时间变化更新触发体的状态、检测条件等
    virtual void Tick(float DeltaTime) override;

    // 定义一个数组，用于存储旧的摩擦力值，可能用于在某些情况下对比、恢复或者记录摩擦力变化的历史情况等
    TArray<float> OldFrictionValues;

public:
    // 将成员变量Friction声明为可在编辑器中编辑（EditAnywhere属性）的属性，初始值设为0.0f，
    // 它代表了该触发体相关的摩擦力参数，外部可以在编辑器中方便地调整这个值来改变触发体的物理效果
    UPROPERTY(EditAnywhere)
    float Friction = 0.0f;

    // 将成员变量TriggerVolume声明为可在编辑器中编辑（EditAnywhere属性）的属性，它是一个指向UBoxComponent类型的指针，
    // 通常用于定义触发体的形状、碰撞范围等，外部可以在编辑器中配置这个盒体组件的相关属性，比如大小、位置等
    UPROPERTY(EditAnywhere)
    UBoxComponent *TriggerVolume;

    // 定义一个指向UCarlaEpisode类型的指针，用于存储该触发体所属的游戏关卡对象的引用，初始值设为nullptr，
    // 通过相关函数可以将其指向实际的关卡对象，以便触发体与关卡进行交互并获取关卡相关的信息
    const UCarlaEpisode *Episode = nullptr;
};
