// 摩擦力触发器
// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "FrictionTrigger.h"
#include "Vehicle/CarlaWheeledVehicle.h"

// AFrictionTrigger类的构造函数，用于初始化该类的相关组件和属性，传入一个FObjectInitializer对象用于创建默认的子对象等初始化操作。
AFrictionTrigger::AFrictionTrigger(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
    // 使用传入的ObjectInitializer创建一个默认的USceneComponent作为根组件，命名为"SceneRootComponent"，并将其赋值给RootComponent成员变量，
    // 根组件通常作为整个Actor的基础，其他组件可以挂载到它上面，这里设置根组件的移动性为静态（不会在游戏运行过程中移动位置等）。
    RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneRootComponent"));
    RootComponent->SetMobility(EComponentMobility::Static);

    // 使用CreateDefaultSubobject函数创建一个UBoxComponent类型的碰撞体组件（用于触发检测等功能，这里作为触发体积），命名为"TriggerVolume"，
    // 这个组件将作为触发区域，当其他物体与之重叠时触发相应的事件。
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    // 将创建的TriggerVolume组件挂载到根组件（RootComponent）上，使其成为根组件的子组件，这样它们在场景中的位置关系就确定了，便于后续进行统一的空间变换等操作。
    TriggerVolume->SetupAttachment(RootComponent);
    // 设置TriggerVolume组件在游戏运行时隐藏，即玩家在游戏画面中看不到这个碰撞体组件，它只是用于逻辑上的触发检测，不影响游戏的视觉呈现。
    TriggerVolume->SetHiddenInGame(true);
    // 设置TriggerVolume组件的移动性为静态，和根组件类似，它在游戏中一般不会主动改变位置等状态，符合其作为固定触发区域的功能设定。
    TriggerVolume->SetMobility(EComponentMobility::Static);
    // 设置TriggerVolume组件的碰撞配置文件名称为"OverlapAll"，这意味着该组件会与所有与之接触的物体产生重叠事件（具体取决于碰撞系统的相关设置和实现），方便后续检测其他物体进入或离开这个触发区域。
    TriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
    // 设置TriggerVolume组件开启生成重叠事件的功能，这样当有其他物体进入或离开它的范围时，相应的重叠开始和结束事件就会被触发，以便执行后续的逻辑处理。
    TriggerVolume->SetGenerateOverlapEvents(true);
}

// AFrictionTrigger类的成员函数Init，用于初始化委托绑定，将相关的重叠事件（开始重叠和结束重叠）与对应的处理函数进行关联，以便在事件触发时能正确响应。
void AFrictionTrigger::Init()
{
    // 判断TriggerVolume组件的OnComponentBeginOverlap委托（用于处理开始重叠事件）是否已经绑定了当前类（AFrictionTrigger）中名为OnTriggerBeginOverlap的处理函数，
    // 如果没有绑定（即IsAlreadyBound返回false），则进行绑定操作，将OnTriggerBeginOverlap函数与开始重叠事件关联起来，
    // 这样当有物体开始与TriggerVolume组件重叠时，OnTriggerBeginOverlap函数就会被调用执行相应逻辑。
    if (!TriggerVolume->OnComponentBeginOverlap.IsAlreadyBound(this, &AFrictionTrigger::OnTriggerBeginOverlap))
    {
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AFrictionTrigger::OnTriggerBeginOverlap);
    }

    // 类似地，判断TriggerVolume组件的OnComponentEndOverlap委托（用于处理结束重叠事件）是否已经绑定了当前类中名为OnTriggerEndOverlap的处理函数，
    // 如果没有绑定，则进行绑定操作，将OnTriggerEndOverlap函数与结束重叠事件关联起来，以便在物体离开TriggerVolume组件重叠区域时，OnTriggerEndOverlap函数能被调用执行相应逻辑。
    if (!TriggerVolume->OnComponentEndOverlap.IsAlreadyBound(this, &AFrictionTrigger::OnTriggerEndOverlap))
    {
        TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AFrictionTrigger::OnTriggerEndOverlap);
    }
}

// AFrictionTrigger类的成员函数UpdateWheelsFriction，用于更新传入的其他Actor（这里期望是车辆Actor）的车轮摩擦力系数，传入要更新的Actor和新的摩擦力系数数组。
void AFrictionTrigger::UpdateWheelsFriction(AActor *OtherActor, TArray<float>& NewFriction)
{
    // 使用Cast函数尝试将传入的OtherActor转换为ACarlaWheeledVehicle类型（假设只有这种类型的车辆Actor才有车轮摩擦力相关的设置操作），
    // 如果转换成功，说明传入的Actor确实是符合要求的车辆，Vehicle指针将指向对应的车辆对象，方便后续操作车辆的相关属性；如果转换失败则Vehicle为nullptr。
    ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
    if (Vehicle!= nullptr)
        // 如果成功转换为车辆类型，调用车辆对象的SetWheelsFrictionScale函数，传入新的摩擦力系数数组NewFriction，来更新车辆车轮的摩擦力系数，实现改变车轮摩擦力的功能。
        Vehicle->SetWheelsFrictionScale(NewFriction);
}

// AFrictionTrigger类的成员函数OnTriggerBeginOverlap，作为处理开始重叠事件的回调函数，当有物体与TriggerVolume组件开始重叠时会被调用，用于处理相应的逻辑，比如改变车辆车轮摩擦力等操作。
void AFrictionTrigger::OnTriggerBeginOverlap(
    UPrimitiveComponent * /*OverlappedComp*/,
    AActor *OtherActor,
    UPrimitiveComponent * /*OtherComp*/,
    int32 /*OtherBodyIndex*/,
    bool /*bFromSweep*/,
    const FHitResult & /*SweepResult*/)
{
    // 首先尝试将传入的OtherActor转换为ACarlaWheeledVehicle类型，判断是否是车辆，如果转换失败（为nullptr），说明不是期望的车辆类型，直接返回，不进行后续操作。
    ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
    if(Vehicle == nullptr)
        return;

    // 获取车辆当前的车轮摩擦力系数数组，并保存到OldFrictionValues成员变量中，用于后续在物体离开触发区域时恢复默认的摩擦力值，这里相当于记录了进入触发区域前车辆原本的摩擦力状态。
    OldFrictionValues = Vehicle->GetWheelsFrictionScale();
    // 创建一个包含四个相同摩擦力值（Friction，这里Friction应该是类的成员变量，代表要设置的特定摩擦力值）的数组，用于设置进入触发区域后车辆的车轮摩擦力。
    TArray<float> TriggerFriction = {Friction, Friction, Friction, Friction};
    // 调用UpdateWheelsFriction函数，传入当前重叠的OtherActor（即车辆）和新创建的TriggerFriction数组，更新车辆车轮的摩擦力系数，使其在进入触发区域后改变为设定的摩擦力值。
    UpdateWheelsFriction(OtherActor, TriggerFriction);
}

// AFrictionTrigger类的成员函数OnTriggerEndOverlap，作为处理结束重叠事件的回调函数，当有物体与TriggerVolume组件结束重叠时会被调用，用于恢复车辆的默认摩擦力值等相关操作。
void AFrictionTrigger::OnTriggerEndOverlap(
    UPrimitiveComponent * /*OverlappedComp*/,
    AActor *OtherActor,
    UPrimitiveComponent * /*OtherComp*/,
    int32 /*OtherBodyIndex*/)
{
    // 调用UpdateWheelsFriction函数，传入当前结束重叠的OtherActor（期望是车辆）和之前保存的OldFrictionValues数组（记录的是进入触发区域前车辆的原始摩擦力系数），
    // 恢复车辆车轮的摩擦力系数为进入触发区域前的默认值，实现离开触发区域后摩擦力恢复的功能。
    UpdateWheelsFriction(OtherActor, OldFrictionValues);

    // 再次尝试将传入的OtherActor转换为ACarlaWheeledVehicle类型，获取当前车辆的车轮摩擦力系数数组，这里可能用于后续检查或者其他相关操作（虽然代码中暂时没有体现后续使用），
    // 如果转换失败则返回，不进行后续操作（不过从逻辑上一般此时应该是车辆才会执行到这一步）。
    ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
    if(Vehicle == nullptr)
        return;

    TArray<float> CurrFriction = Vehicle->GetWheelsFrictionScale();
}

// AFrictionTrigger类的成员函数BeginPlay，重写了UE4中Actor类的BeginPlay函数，在游戏开始或该Actor生成时执行初始化相关操作，这里主要调用了Init函数进行委托绑定等初始化。
void AFrictionTrigger::BeginPlay()
{
    Super::BeginPlay();
    Init();
}

// AFrictionTrigger类的成员函数EndPlay，重写了UE4中Actor类的EndPlay函数，在该Actor结束生命周期（比如被销毁等情况，根据传入的结束原因参数EndPlayReason来确定具体情况）时执行清理相关操作，比如取消委托绑定。
void AFrictionTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 判断TriggerVolume组件的OnComponentBeginOverlap委托是否已经绑定了当前类中名为OnTriggerBeginOverlap的处理函数，如果绑定了，则进行解除绑定操作，
    // 因为Actor即将结束生命周期，不再需要响应这些重叠开始事件了，避免后续可能出现的无效函数调用等问题。
    if (TriggerVolume->OnComponentBeginOverlap.IsAlreadyBound(this, &AFrictionTrigger::OnTriggerBeginOverlap))
    {
        TriggerVolume->OnComponentBeginOverlap.RemoveDynamic(this, &AFrictionTrigger::OnTriggerBeginOverlap);
    }

    // 类似地，判断TriggerVolume组件的OnComponentEndOverlap委托是否已经绑定了当前类中名为OnTriggerEndOverlap的处理函数，如果绑定了，则进行解除绑定操作，
    // 取消与结束重叠事件的关联，确保在Actor销毁等结束生命周期的情况下，相关的事件处理逻辑被正确清理。
    if (TriggerVolume->OnComponentEndOverlap.IsAlreadyBound(this, &AFrictionTrigger::OnTriggerEndOverlap))
    {
        TriggerVolume->OnComponentEndOverlap.RemoveDynamic(this, &AFrictionTrigger::OnTriggerEndOverlap);
    }

    Super::EndPlay(EndPlayReason);
}

// AFrictionTrigger类的成员函数Tick，重写了UE4中Actor类的Tick函数，会在每一帧被调用，通常用于执行每帧都需要更新的逻辑，这里目前只是调用了父类的Tick函数，暂时没有添加额外的每帧更新逻辑。
void AFrictionTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
