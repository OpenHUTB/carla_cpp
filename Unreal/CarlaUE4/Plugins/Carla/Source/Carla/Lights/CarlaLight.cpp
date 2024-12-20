// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).

//
// 本作品根据 MIT 许可证的条款进行许可。
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>。

// 包含CarlaLight相关的头文件，可能定义了该类依赖的基础结构、类型等内容
#include "CarlaLight.h"
// 包含CarlaLightSubsystem相关的头文件，应该是和灯光子系统相关的定义所在
#include "CarlaLightSubsystem.h"
// 包含Carla相关的静态工具类的头文件，用于获取游戏中的一些通用静态资源等操作
#include "Carla/Game/CarlaStatics.h"

// UCarlaLight类的定义，通常继承自UE4中的某个组件类（从命名推测），用于表示游戏中的灯光相关逻辑
UCarlaLight::UCarlaLight()
{
    // 设置该组件的Tick机制，这里表示该组件在游戏运行时不会每帧进行更新（bCanEverTick设为false）
    PrimaryComponentTick.bCanEverTick = false;
}

// 当该组件所在的Actor开始游戏时调用的函数（UE4生命周期函数）
void UCarlaLight::BeginPlay()
{
    // 先调用父类的BeginPlay函数，确保父类相关的初始化逻辑先执行
    Super::BeginPlay();

    // 注册灯光，将该灯光组件添加到相应的系统中进行管理等操作
    RegisterLight();
}

// 用于注册灯光的函数
void UCarlaLight::RegisterLight()
{
    // 如果已经注册过了，直接返回，避免重复注册
    if (bRegistered)
    {
        return;
    }

    // 获取该组件所在的世界指针，UE4中世界是管理游戏场景中所有Actor等对象的顶级容器
    UWorld *World = GetWorld();
    if (World)
    {
        // 从世界中获取CarlaLightSubsystem实例，用于后续把自身注册到该子系统中
        UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
        // 将当前灯光组件注册到灯光子系统中，具体注册逻辑在子系统内实现
        CarlaLightSubsystem->RegisterLight(this);
    }
    // 将灯光与天气相关逻辑进行注册（具体功能可能是根据天气情况调整灯光等，从函数名推测）
    RegisterLightWithWeather();
    // 标记该灯光已经注册成功
    bRegistered = true;
}

// 当该组件被销毁时调用的函数（UE4生命周期函数）
void UCarlaLight::OnComponentDestroyed(bool bDestroyingHierarchy)
{
    // 先调用父类的OnComponentDestroyed函数，确保父类相关的销毁逻辑先执行
    Super::OnComponentDestroyed(bDestroyingHierarchy);
}

// 当该组件所在的Actor结束游戏时调用的函数（UE4生命周期函数），传入结束游戏的原因类型参数
void UCarlaLight::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 获取该组件所在的世界指针
    UWorld *World = GetWorld();
    if (World)
    {
        // 从世界中获取CarlaLightSubsystem实例
        UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
        // 将当前灯光组件从灯光子系统中注销，释放相关资源等
        CarlaLightSubsystem->UnregisterLight(this);
    }
    // 调用父类的EndPlay函数，确保父类相关的结束游戏逻辑能执行
    Super::EndPlay(EndPlayReason);
}

// 设置灯光强度的函数，接收一个表示强度的浮点数值参数
void UCarlaLight::SetLightIntensity(float Intensity)
{
    // 更新灯光强度的内部变量
    LightIntensity = Intensity;
    // 根据新的强度值更新灯光的实际显示效果等（具体实现可能在UpdateLights函数内）
    UpdateLights();
}

// 获取当前灯光强度的函数，返回一个表示强度的浮点数值
float UCarlaLight::GetLightIntensity() const
{
    return LightIntensity;
}

// 设置灯光颜色的函数，接收一个FLinearColor类型（UE4中表示线性颜色的结构体）的参数
void UCarlaLight::SetLightColor(FLinearColor Color)
{
    // 更新灯光颜色的内部变量
    LightColor = Color;
    // 根据新的颜色值更新灯光的实际显示效果等
    UpdateLights();
    // 记录灯光颜色发生了改变，可能用于后续的日志记录、回放等相关逻辑
    RecordLightChange();
}

// 获取当前灯光颜色的函数，返回一个FLinearColor类型的颜色值
FLinearColor UCarlaLight::GetLightColor() const
{
    return LightColor;
}

// 设置灯光是否开启的函数，接收一个布尔值参数表示是否开启
void UCarlaLight::SetLightOn(bool bOn)
{
    bLightOn = bOn;
    // 根据新的开启状态更新灯光的实际显示效果等
    UpdateLights();
    // 记录灯光开启状态发生了改变
    RecordLightChange();
}

// 获取灯光当前是否开启的函数，返回一个布尔值表示开启状态
bool UCarlaLight::GetLightOn() const
{
    return bLightOn;
}

// 设置灯光类型的函数，接收一个ELightType类型（自定义的灯光类型枚举，推测）的参数
void UCarlaLight::SetLightType(ELightType Type)
{
    LightType = Type;
}

// 获取当前灯光类型的函数，返回一个ELightType类型的灯光类型值
ELightType UCarlaLight::GetLightType() const
{
    return LightType;
}

// 获取灯光状态的函数，返回一个carla::rpc::LightState类型（应该是和RPC通信相关的灯光状态结构体，推测）的对象
carla::rpc::LightState UCarlaLight::GetLightState()
{
    // 创建一个carla::rpc::LightState对象，并通过当前灯光组件的各项属性进行初始化
    carla::rpc::LightState state(
        GetLocation(),
        LightIntensity,
        static_cast<carla::rpc::LightState::LightGroup>(LightType),
        LightColor,
        bLightOn
    );

    // 设置该灯光状态对象的id为当前灯光组件的id
    state._id = GetId();

    return state;
}

// 设置灯光状态的函数，接收一个carla::rpc::LightState类型的参数表示要设置的目标状态
void UCarlaLight::SetLightState(carla::rpc::LightState LightState)
{
    // 根据传入的灯光状态参数更新灯光强度的内部变量
    LightIntensity = LightState._intensity;
    // 根据传入的灯光状态参数更新灯光颜色的内部变量
    LightColor = LightState._color;
    // 根据传入的灯光状态参数更新灯光类型的内部变量，进行类型转换（从rpc相关类型转换为自定义的ELightType类型）
    LightType = static_cast<ELightType>(LightState._group);
    // 根据传入的灯光状态参数更新灯光开启状态的内部变量
    bLightOn = LightState._active;
    // 根据新的状态值更新灯光的实际显示效果等
    UpdateLights();
    // 记录灯光状态发生了改变
    RecordLightChange();
}

// 获取灯光在游戏世界中的位置的函数，返回一个FVector类型（UE4中表示三维向量的结构体）的位置值
FVector UCarlaLight::GetLocation() const
{
    // 先获取该灯光组件所属的Actor在本地坐标系下的位置
    auto Location = GetOwner()->GetActorLocation();
    // 获取游戏模式实例，通过CarlaStatics工具类从世界中获取
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(GetWorld());
    // 从游戏模式中获取大型地图管理器实例（可能用于地图相关的坐标转换等功能）
    ALargeMapManager* LargeMap = GameMode->GetLMManager();
    if (LargeMap)
    {
        // 如果大型地图管理器存在，将本地坐标转换为全局坐标（具体转换逻辑在LargeMap的对应函数内实现）
        Location = LargeMap->LocalToGlobalLocation(Location);
    }
    return Location;
}

// 获取灯光组件的唯一标识符的函数，返回一个整数类型的id值
int UCarlaLight::GetId() const
{
    return Id;
}

// 设置灯光组件唯一标识符的函数，接收一个整数类型的参数表示要设置的新id值
void UCarlaLight::SetId(int InId)
{
    Id = InId;
}

// 记录灯光发生改变的函数（通常用于一些日志记录、事件记录等相关逻辑），被标记为const表示不会修改类的成员变量（除了可能的mutable成员变量，如果有的话）
void UCarlaLight::RecordLightChange() const
{
    // 获取当前游戏的剧情实例（可能用于记录游戏过程中的一些事件等，从命名推测），通过CarlaStatics工具类从世界中获取
    auto* Episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
    if (Episode)
    {
        // 从剧情实例中获取记录器实例（用于记录游戏中的各种事件等操作）
        auto* Recorder = Episode->GetRecorder();
        if (Recorder && Recorder->IsEnabled())
        {
            // 如果记录器存在且已启用，添加一个灯光场景改变的事件记录，将当前灯光组件作为参数传入（具体记录逻辑在记录器内实现）
            Recorder->AddEventLightSceneChanged(this);
        }
    }
}
