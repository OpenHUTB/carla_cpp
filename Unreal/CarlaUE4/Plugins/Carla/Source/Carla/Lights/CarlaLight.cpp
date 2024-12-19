// 版权所有 （c） 巴塞罗那自治大学 （UAB） 2020 计算机视觉中心 （CVC）。
//
// 本作品根据 MIT 许可证的条款进行许可。
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>。

// 引入CarlaLight相关的头文件，可能包含了CarlaLight类自身的定义以及相关依赖的前置声明等内容
#include "CarlaLight.h"
// 引入CarlaLightSubsystem相关头文件，推测是用于处理灯光子系统相关功能的类声明所在，会和灯光组件在系统层面的管理相关
#include "CarlaLightSubsystem.h"
// 引入Carla项目中游戏相关的静态工具类头文件，可能包含了获取游戏中各种全局资源、游戏模式等相关的静态函数
#include "Carla/Game/CarlaStatics.h"

// UCarlaLight类的定义，通常继承自UE4中的某个组件类（从命名和功能推测可能和游戏中的灯光组件相关）
UCarlaLight::UCarlaLight()
{
    // 设置该组件的Tick机制，bCanEverTick设为false表示这个灯光组件在游戏运行过程中默认不会每帧进行更新操作，
    // 一般适用于那些不需要频繁改变状态或者只在特定事件触发时才更新的组件
    PrimaryComponentTick.bCanEverTick = false;
}

// 当该组件所在的Actor开始在游戏世界中进行游戏逻辑（如进入游戏场景等情况）时调用的函数（UE4生命周期函数）
void UCarlaLight::BeginPlay()
{
    Super::BeginPlay();  // 先调用父类的BeginPlay函数，确保父类相关的初始化逻辑先执行，以保证继承体系下的初始化完整性

    // 调用注册灯光的函数，将当前灯光组件注册到相应的管理系统中，使其能够参与游戏中灯光相关的各种逻辑处理
    RegisterLight();
}

// 用于将灯光组件注册到相关系统的函数
void UCarlaLight::RegisterLight()
{
    // 如果该灯光组件已经注册过了（通过bRegistered标志来判断），直接返回，避免重复注册造成不必要的逻辑错误或资源浪费
    if (bRegistered)
    {
        return;
    }

    // 获取该灯光组件所在的世界指针，UE4中世界（UWorld）是管理游戏场景中所有Actor、组件等对象的顶级容器，后续很多操作都依赖这个世界对象来获取其他相关资源或执行相关逻辑
    UWorld *World = GetWorld();
    if (World)
    {
        // 从当前世界对象中获取CarlaLightSubsystem实例（灯光子系统实例），用于后续把当前灯光组件注册到该子系统里进行统一管理
        UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
        // 调用灯光子系统的RegisterLight函数，将当前灯光组件（通过this指针传入自身）注册到子系统中，具体的注册逻辑在UCarlaLightSubsystem类的RegisterLight函数里实现
        CarlaLightSubsystem->RegisterLight(this);
    }
    // 调用另一个注册函数（可能是和天气相关联的灯光注册逻辑，比如根据不同天气调整灯光表现等），具体功能需看RegisterLightWithWeather函数的实现
    RegisterLightWithWeather();
    // 标记该灯光组件已经成功注册，将bRegistered置为true，用于后续判断是否已注册的逻辑
    bRegistered = true;
}

// 当该灯光组件被销毁时调用的函数（UE4生命周期函数），传入一个布尔值参数表示是否是在销毁组件所在的整个层级结构（比如包含该组件的Actor及其子对象等一起销毁的情况）
void UCarlaLight::OnComponentDestroyed(bool bDestroyingHierarchy)
{
    Super::OnComponentDestroyed(bDestroyingHierarchy);  // 先调用父类的OnComponentDestroyed函数，确保父类相关的销毁逻辑先执行，维持继承体系下的正确销毁流程
}

// 当该组件所在的Actor结束游戏（例如离开游戏场景、被销毁等情况）时调用的函数（UE4生命周期函数），传入一个表示结束游戏原因的枚举类型参数（EEndPlayReason::Type），用于区分不同的结束情况
void UCarlaLight::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 获取该灯光组件所在的世界指针，和之前在注册等操作中获取世界指针的目的类似，为了获取相关资源进行后续逻辑处理
    UWorld *World = GetWorld();
    if (World)
    {
        // 从世界对象中获取CarlaLightSubsystem实例（灯光子系统实例），用于后续将当前灯光组件从该子系统中注销，释放相关资源等操作
        UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
        // 调用灯光子系统的UnregisterLight函数，将当前灯光组件从子系统里注销，具体注销逻辑在UCarlaLightSubsystem类的UnregisterLight函数中实现
        CarlaLightSubsystem->UnregisterLight(this);
    }
    Super::EndPlay(EndPlayReason);  // 调用父类的EndPlay函数，确保父类相关的结束游戏逻辑能正确执行，保证整个继承体系下的正确处理流程
}

// 设置灯光强度的函数，接收一个浮点数参数（Intensity），用于指定灯光的强度大小，可能影响灯光的亮度等视觉效果
void UCarlaLight::SetLightIntensity(float Intensity)
{
    // 将内部记录灯光强度的成员变量LightIntensity更新为传入的强度值，以便后续使用该新值进行相关逻辑处理（比如更新渲染效果等）
    LightIntensity = Intensity;
    // 调用更新灯光的函数（UpdateLights），根据新设置的强度值来更新灯光的实际显示效果等，具体更新逻辑在UpdateLights函数里实现
    UpdateLights();
}

// 获取当前灯光强度的函数，返回一个浮点数表示当前灯光组件的强度值，外部代码可以通过调用此函数获取灯光的强度信息
float UCarlaLight::GetLightIntensity() const
{
    return LightIntensity;  // 直接返回内部记录灯光强度的成员变量LightIntensity的值
}

// 设置灯光颜色的函数，接收一个FLinearColor类型（UE4中用于表示线性颜色的结构体，包含RGB等颜色通道信息）的参数（Color），用于指定灯光的颜色
void UCarlaLight::SetLightColor(FLinearColor Color)
{
    // 将内部记录灯光颜色的成员变量LightColor更新为传入的颜色值，以便后续依据这个新颜色进行渲染等相关操作
    LightColor = Color;
    // 调用更新灯光的函数（UpdateLights），按照新设置的颜色来更新灯光的实际显示效果，比如改变灯光在游戏场景中的颜色呈现
    UpdateLights();
    // 调用记录灯光变化的函数（RecordLightChange），可能用于记录灯光颜色发生改变这一事件，方便后续进行日志记录、回放等相关逻辑处理
    RecordLightChange();
}

// 获取当前灯光颜色的函数，返回一个FLinearColor类型的颜色值，代表当前灯光组件实际呈现的颜色，供外部代码获取使用
FLinearColor UCarlaLight::GetLightColor() const
{
    return LightColor;  // 返回内部记录灯光颜色的成员变量LightColor的值
}

// 设置灯光是否开启的函数，接收一个布尔值参数（bOn），用于决定灯光是打开还是关闭状态
void UCarlaLight::SetLightOn(bool bOn)
{
    bLightOn = bOn;  // 将内部记录灯光开启状态的成员变量bLightOn更新为传入的布尔值
    // 调用更新灯光的函数（UpdateLights），根据新的开启状态来更新灯光在游戏场景中的实际显示效果，比如打开或关闭灯光的渲染
    UpdateLights();
    // 调用记录灯光变化的函数（RecordLightChange），记录灯光开启状态发生改变这一情况，便于后续相关逻辑使用该记录信息
    RecordLightChange();
}

// 获取灯光当前是否开启的函数，返回一个布尔值表示当前灯光组件的开启状态（true表示开启，false表示关闭），供外部代码查询使用
bool UCarlaLight::GetLightOn() const
{
    return bLightOn;  // 返回内部记录灯光开启状态的成员变量bLightOn的值
}

// 设置灯光类型的函数，接收一个ELightType类型（自定义的灯光类型枚举，可能包含如点光源、聚光灯等不同类型，具体取决于项目定义）的参数（Type），用于指定灯光的具体类型
void UCarlaLight::SetLightType(ELightType Type)
{
    LightType = Type;  // 将内部记录灯光类型的成员变量LightType更新为传入的灯光类型值，后续可依据此类型进行不同的渲染、逻辑处理等操作
}

// 获取当前灯光类型的函数，返回一个ELightType类型的灯光类型值，代表当前灯光组件的具体类型，外部代码可以通过调用此函数获取该信息
ELightType UCarlaLight::GetLightType() const
{
    return LightType;  // 返回内部记录灯光类型的成员变量LightType的值
}

// 获取灯光状态的函数，返回一个carla::rpc::LightState类型（从命名看可能是和RPC（远程过程调用）通信相关的用于表示灯光状态的结构体，包含了多个灯光属性信息）的对象，用于获取当前灯光完整的状态信息
carla::rpc::LightState UCarlaLight::GetLightState()
{
    // 创建一个carla::rpc::LightState类型的对象（state），并通过调用多个函数获取当前灯光组件的各个属性来初始化这个状态对象，
    // 包括位置（GetLocation函数获取）、强度（LightIntensity成员变量）、灯光类型（经过类型转换后的LightType成员变量）、颜色（LightColor成员变量）以及开启状态（bLightOn成员变量）
    carla::rpc::LightState state(
        GetLocation(),
        LightIntensity,
        static_cast<carla::rpc::LightState::LightGroup>(LightType),
        LightColor,
        bLightOn
    );

    // 设置这个灯光状态对象的唯一标识符（_id）为当前灯光组件的标识符（通过GetId函数获取），确保状态对象能准确对应到具体的灯光组件
    state._id = GetId();

    return state;  // 返回包含完整灯光状态信息的对象，供外部代码使用（比如可能用于网络传输、保存当前状态等情况）
}

// 设置灯光状态的函数，接收一个carla::rpc::LightState类型的参数（LightState），用于根据传入的状态信息更新当前灯光组件的各个属性状态
void UCarlaLight::SetLightState(carla::rpc::LightState LightState)
{
    // 根据传入的灯光状态参数中的强度值（_intensity），更新内部记录灯光强度的成员变量LightIntensity，使其与传入状态保持一致
    LightIntensity = LightState._intensity;
    // 根据传入的灯光状态参数中的颜色值（_color），更新内部记录灯光颜色的成员变量LightColor，以改变灯光的颜色显示
    LightColor = LightState._color;
    // 根据传入的灯光状态参数中的灯光类型值（经过转换从rpc相关的类型转换为自定义的ELightType类型），更新内部记录灯光类型的成员变量LightType，调整灯光的类型相关逻辑和渲染等
    LightType = static_cast<ELightType>(LightState._group);
    // 根据传入的灯光状态参数中的开启状态值（_active），更新内部记录灯光开启状态的成员变量bLightOn，改变灯光的开启或关闭状态
    bLightOn = LightState._active;
    // 调用更新灯光的函数（UpdateLights），根据新设置的完整状态信息来更新灯光在游戏场景中的实际显示效果，确保灯光呈现出传入状态所期望的样子
    UpdateLights();
    // 调用记录灯光变化的函数（RecordLightChange），记录灯光状态发生改变这一事件，方便后续进行相关的日志记录、状态追踪等逻辑处理
    RecordLightChange();
}

// 获取灯光在游戏世界中的位置的函数，返回一个FVector类型（UE4中用于表示三维空间向量的结构体，包含X、Y、Z坐标信息）的位置值，代表灯光在游戏世界中的具体坐标位置
FVector UCarlaLight::GetLocation() const
{
    // 先获取该灯光组件所属的Actor（在UE4中组件通常依附于Actor）在本地坐标系下的位置信息，通过调用所属Actor的GetActorLocation函数实现
    auto Location = GetOwner()->GetActorLocation();
    // 通过CarlaStatics静态工具类从当前世界中获取游戏模式实例（ACarlaGameModeBase类型，游戏模式通常管理着游戏中的各种全局规则、资源等）
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(GetWorld());
    // 从游戏模式实例中获取大型地图管理器实例（ALargeMapManager类型，可能用于处理地图相关的坐标转换、大范围地图管理等功能）
    ALargeMapManager* LargeMap = GameMode->GetLMManager();
    if (LargeMap)
    {
        // 如果获取到了大型地图管理器实例，调用其LocalToGlobalLocation函数，将之前获取的本地坐标位置转换为全局坐标位置，以获取在整个游戏世界中的准确位置信息
        Location = LargeMap->LocalToGlobalLocation(Location);
    }
    return Location;  // 返回最终确定的灯光在游戏世界中的位置信息，供外部代码使用（比如用于定位、渲染等相关操作）
}

// 获取灯光组件的唯一标识符的函数，返回一个整数类型的标识符值（Id），用于唯一标识该灯光组件，方便在多个灯光组件中进行区分、查找等操作
int UCarlaLight::GetId() const
{
    return Id;  // 直接返回内部记录灯光组件标识符的成员变量Id的值
}

// 设置灯光组件唯一标识符的函数，接收一个整数类型的参数（InId），用于更新灯光组件的标识符，改变其唯一标识信息
void UCarlaLight::SetId(int InId)
{
    Id = InId;  // 将内部记录灯光组件标识符的成员变量Id更新为传入的新标识符值
}

// 记录灯光发生改变的函数（通常用于一些日志记录、事件记录等相关逻辑，以便后续可以追踪灯光状态的变化情况），被标记为const表示该函数不会修改类的除了mutable修饰的成员变量以外的其他成员变量
void UCarlaLight::RecordLightChange() const
{
    // 通过CarlaStatics静态工具类从当前世界中获取当前游戏的剧情实例（从命名推测可能用于记录游戏过程中的一些事件、状态等，用于实现游戏的剧情相关功能，比如回放、剧情推进等）
    auto* Episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
    if (Episode)
    {
        // 从剧情实例中获取记录器实例（用于实际记录游戏中的各种事件、状态变化等操作，具体记录逻辑在记录器类中实现）
        auto* Recorder = Episode->GetRecorder();
        if (Recorder && Recorder->IsEnabled())
        {
            // 如果记录器实例存在且已启用（意味着可以进行记录操作），调用记录器的AddEventLightSceneChanged函数，将当前灯光组件作为参数传入，添加一个灯光场景改变的事件记录，方便后续查询和分析灯光状态变化情况
            Recorder->AddEventLightSceneChanged(this);
        }
    }
}
