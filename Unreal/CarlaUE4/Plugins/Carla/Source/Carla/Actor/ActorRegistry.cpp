// 版权所有 （c） 2017 巴塞罗那自治大学 （UAB） 计算机视觉中心 （CVC）。
//
// 本作品根据 MIT 许可证的条款进行许可。
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>。

// 包含Carla相关的主头文件，可能定义了Carla项目中一些基础的类型、宏以及常用的功能等，是整个Carla相关代码的核心头文件
#include "Carla.h"  
// 包含Actor数据相关的头文件，可能定义了和Actor在Carla中存储、传递等相关的数据结构等内容
#include "Carla/Actor/ActorData.h"  
// 包含Actor注册相关的头文件，大概率用于管理Actor在系统中的注册、查找等操作相关的类和函数声明等
#include "Carla/Actor/ActorRegistry.h"  

// 包含Carla游戏中标签相关功能的头文件，可能用于处理游戏中各种对象的标签设定、获取等操作
#include "Carla/Game/Tagger.h"  
// 包含交通信号灯基础相关的头文件，用于定义交通信号灯相关的类以及操作函数等（和交通模拟相关的部分）
#include "Carla/Traffic/TrafficLightBase.h"  
// 包含用于计算边界框相关的头文件，可用于获取游戏中Actor的边界框信息（比如碰撞检测等场景会用到边界框）
#include "Carla/Util/BoundingBoxCalculator.h"  
// 包含传感器相关的头文件，定义了Carla中传感器相关的类以及其操作函数等（用于模拟各种传感器行为）
#include "Carla/Sensor/Sensor.h"  

// 包含用于禁用UE4相关宏的头文件，可能在编译时避免一些UE4宏定义带来的影响（特定编译环境需求）
#include <compiler/disable-ue4-macros.h>  
// 包含Carla流媒体相关的Token头文件，可能用于处理流媒体传输过程中的标识、验证等相关功能（涉及数据传输部分）
#include "carla/streaming/Token.h"  
// 包含Carla流媒体详细的Token头文件，可能是更具体深入的关于流媒体Token的定义和操作相关内容
#include "carla/streaming/detail/Token.h"  
// 包含用于启用UE4相关宏的头文件，和前面禁用的对应，恢复UE4宏在编译时的正常作用
#include <compiler/enable-ue4-macros.h>  

// 使用命名空间别名crp来简化carla::rpc的书写，方便后续代码中使用rpc相关的类型、函数等
namespace crp = carla::rpc;  

// 定义FActorRegistry类中的静态成员变量ID_COUNTER，用于为注册的Actor分配唯一的ID，初始值为0（无符号整数类型）
FActorRegistry::IdType FActorRegistry::ID_COUNTER = 0u;  

// 定义一个静态函数FActorRegistry_GetActorType，用于根据传入的AActor指针判断其对应的Actor类型
// 参数Actor为指向AActor类型的指针，代表需要判断类型的游戏中的Actor对象
static FCarlaActor::ActorType FActorRegistry_GetActorType(const AActor *Actor)  
{
    // 如果传入的Actor指针为空，说明没有有效的Actor对象，返回无效类型（INVALID）
    if (!Actor)  
    {
        return FCarlaActor::ActorType::INVALID;
    }


FCarlaActor* FActorRegistry::Register(AActor &Actor, FActorDescription Description, IdType DesiredId)
{

  FCarlaActor* CarlaActor = FindCarlaActor(DesiredId);
  bool IsDormant = CarlaActor && (CarlaActor->IsDormant());
  if(IsDormant)
  {
    CarlaActor->TheActor = &Actor;
    Actors.Add(DesiredId, &Actor);
    Ids.Add(&Actor, DesiredId);
    return CarlaActor;
  }

  IdType Id = ++FActorRegistry::ID_COUNTER;

  if (DesiredId != 0 && Id != DesiredId) {
    // check if the desired Id is free, then use it instead
    if (!Actors.Contains(DesiredId))

    // 判断传入的Actor是否可以转换为ACarlaWheeledVehicle类型，如果可以，说明是车辆类型的Actor，返回Vehicle类型
    if (nullptr!= Cast<ACarlaWheeledVehicle>(Actor))  

    {
        return FCarlaActor::ActorType::Vehicle;
    }
    // 判断是否可以转换为ACharacter类型，如果可以，说明是行人（角色）类型的Actor，返回Walker类型
    else if (nullptr!= Cast<ACharacter>(Actor))  
    {
        return FCarlaActor::ActorType::Walker;
    }
    // 判断是否可以转换为ATrafficLightBase类型，如果可以，说明是交通信号灯类型的Actor，返回TrafficLight类型
    else if (nullptr!= Cast<ATrafficLightBase>(Actor))  
    {
        return FCarlaActor::ActorType::TrafficLight;
    }
    // 判断是否可以转换为ATrafficSignBase类型，如果可以，说明是交通标识类型的Actor，返回TrafficSign类型
    else if (nullptr!= Cast<ATrafficSignBase>(Actor))  
    {
        return FCarlaActor::ActorType::TrafficSign;
    }
    // 判断是否可以转换为ASensor类型，如果可以，说明是传感器类型的Actor，返回Sensor类型
    else if (nullptr!= Cast<ASensor>(Actor))  
    {
        return FCarlaActor::ActorType::Sensor;
    }
    // 如果都不符合上述类型判断，返回其他类型（Other）表示不属于前面明确分类的Actor类型
    else  
    {
        return FCarlaActor::ActorType::Other;
    }
}

// 定义函数CarlaGetRelevantTagAsString，用于从给定的语义标签集合中获取一个相关的、合适的标签字符串表示
// 参数SemanticTags是一个包含crp::CityObjectLabel类型元素的集合，代表语义标签集合
FString CarlaGetRelevantTagAsString(const TSet<crp::CityObjectLabel> &SemanticTags)  
{
    // 遍历传入的语义标签集合SemanticTags
    for (auto &&Tag : SemanticTags)  
    {
        // 排除特定的无意义标签（None和Other），寻找合适的标签进行处理
        if ((Tag!= crp::CityObjectLabel::None) && (Tag!= crp::CityObjectLabel::Other))  
        {
            // 将标签转换为字符串并转换为小写形式
            auto Str = ATagger::GetTagAsString(Tag).ToLower();  
            // 如果字符串以's'结尾，去除末尾的's'（可能是处理复数形式等情况），然后返回处理后的字符串
            return (Str.EndsWith(TEXT("s"))? Str.LeftChop(1) : Str);  
        }
    }
    // 如果没有找到合适的标签，返回"unknown"表示未知标签情况
    return TEXT("unknown");  
}

// FActorRegistry类的成员函数Register，用于注册一个Actor到Actor注册系统中，返回注册后的FCarlaActor指针
// 参数Actor是要注册的AActor类型的引用，代表实际的游戏Actor对象
// 参数Description是FActorDescription类型，包含了Actor的描述信息（如名称、属性等）
// 参数DesiredId是期望为Actor分配的ID（可选参数，若为0则自动分配）
FCarlaActor* FActorRegistry::Register(AActor &Actor, FActorDescription Description, IdType DesiredId)  
{
    // 尝试查找具有指定DesiredId的已注册的FCarlaActor对象（可能是检查是否已存在或复用等情况）
    FCarlaActor* CarlaActor = FindCarlaActor(DesiredId);  
    // 判断找到的CarlaActor是否存在且处于休眠状态（IsDormant为真表示休眠）
    bool IsDormant = CarlaActor && (CarlaActor->IsDormant());  
    // 如果是休眠状态的已注册Actor，进行重新关联等操作
    if (IsDormant)  
    {
        // 将当前要注册的Actor关联到已有的CarlaActor对象上
        CarlaActor->TheActor = &Actor;  
        // 在Actors映射中添加（或更新）Actor与指定ID的关联
        Actors.Add(DesiredId, &Actor);  
        // 在Ids映射中添加（或更新）Actor指针与ID的反向关联
        Ids.Add(&Actor, DesiredId);  
        // 返回已处理好的CarlaActor指针，表示注册完成（复用已有的）
        return CarlaActor;  
    }

    // 如果不是复用休眠状态的Actor，则为其分配一个新的ID，通过自增全局的ID计数器来获取新ID
    IdType Id = ++FActorRegistry::ID_COUNTER;  

    // 如果传入了期望的ID（DesiredId）且新分配的ID与期望的ID不一致，则进行以下处理
    if (DesiredId!= 0 && Id!= DesiredId) {  
        // 检查期望的ID是否没有被其他Actor占用，如果没被占用则使用期望的ID
        if (!Actors.Contains(DesiredId))  
        {
            Id = DesiredId;  
            // 如果全局ID计数器的值小于期望使用的ID，更新全局ID计数器的值为期望的ID（保证计数器始终是最大的ID值）
            if (ID_COUNTER < Id)  
                ID_COUNTER = Id;  
        }
    }

    // 在Actors映射中添加新的Actor与分配的ID的关联
    Actors.Emplace(Id, &Actor);  
    // 检查是否已经存在当前Actor指针的注册记录，如果存在则输出警告日志（可能存在重复注册等异常情况）
    if (Ids.Contains(&Actor))  
    {
        UE_LOG(
            LogCarla,
            Warning,
            TEXT("This actor's memory address is already registered, "
                 "either you forgot to deregister the actor "
                 "or the actor was garbage collected."));
    }
    // 在Ids映射中添加Actor指针与新分配ID的反向关联
    Ids.Emplace(&Actor, Id);  

    // 创建一个新的FCarlaActor的共享指针View，包含了Actor的各种信息（如ID、描述、状态等）
    TSharedPtr<FCarlaActor> View =
        MakeCarlaActor(Id, Actor, std::move(Description), crp::ActorState::Active);  

    // 将新创建的FCarlaActor共享指针添加到ActorDatabase映射中，使用分配的ID作为键，并获取添加后的引用（Result）
    TSharedPtr<FCarlaActor>& Result = ActorDatabase.Emplace(Id, MoveTemp(View));  

    // 进行一致性检查，确保Actors映射中的元素数量和ActorDatabase映射中的元素数量相等（数据一致性校验）
    check(static_cast<size_t>(Actors.Num()) == ActorDatabase.Num());  
    // 返回注册后的FCarlaActor的指针，代表注册成功后的Actor对象在系统中的表示
    return Result.Get();  
}

// FActorRegistry类的成员函数Deregister，用于根据指定的ID注销一个已注册的Actor
// 参数Id是要注销的Actor的ID类型，通过这个ID来查找并移除对应的Actor注册信息
void FActorRegistry::Deregister(IdType Id)  
{
    // 根据传入的ID查找对应的FCarlaActor对象
    FCarlaActor* CarlaActor = FindCarlaActor(Id);  

    // 如果没找到对应的Actor（CarlaActor为nullptr），直接返回，不进行后续注销操作
    if (!CarlaActor) return;  

    // 获取要注销的Actor对应的AActor指针
    AActor *Actor = CarlaActor->GetActor();  

    // 从ActorDatabase映射中移除指定ID对应的Actor记录
    ActorDatabase.Remove(Id);  
    // 从Actors映射中移除指定ID与Actor的关联
    Actors.Remove(Id);  
    // 从Ids映射中移除Actor指针与ID的反向关联
    Ids.Remove(Actor);  

    // 将对应的FCarlaActor对象中的Actor指针置为空，表示已注销该Actor
    CarlaActor->TheActor = nullptr;  

    // 进行一致性检查，确保Actors映射中的元素数量和ActorDatabase映射中的元素数量相等（数据一致性校验）
    check(static_cast<size_t>(Actors.Num()) == ActorDatabase.Num());  
}

// FActorRegistry类的成员函数Deregister的重载版本，用于根据传入的AActor指针注销对应的Actor
// 参数Actor是指向要注销的AActor类型的指针，通过这个指针来查找并移除对应的Actor注册信息
void FActorRegistry::Deregister(AActor *Actor)  
{
    // 检查传入的Actor指针是否为空，确保有有效的Actor对象要注销
    check(Actor!= nullptr);  
    // 根据传入的Actor指针查找对应的FCarlaActor对象
    FCarlaActor* CarlaActor = FindCarlaActor(Actor);  
    // 检查找到的FCarlaActor对象对应的Actor指针是否和传入的Actor指针一致（确保查找正确）
    check(CarlaActor->GetActor() == Actor);  
    // 调用另一个重载的Deregister函数，通过获取到的CarlaActor对象的ID来进行注销操作
    Deregister(CarlaActor->GetActorId());  
}

// FActorRegistry类的成员函数MakeCarlaActor，用于创建一个新的FCarlaActor对象，返回其共享指针
// 参数Id是要创建的FCarlaActor的ID类型，用于标识这个Actor
// 参数Actor是AActor类型的引用，代表实际的游戏Actor对象，用于关联到创建的FCarlaActor上
// 参数Description是FActorDescription类型，包含了Actor的描述信息（如名称、属性等）
// 参数InState是crp::ActorState类型，用于指定Actor的初始状态（如活跃、休眠等）
TSharedPtr<FCarlaActor> FActorRegistry::MakeCarlaActor(
    IdType Id,
    AActor &Actor,
    FActorDescription Description,
    crp::ActorState InState) const  
{
    // 创建一个新的FActorInfo的共享指针Info，用于存储Actor的详细信息
    auto Info = MakeShared<FActorInfo>();  
    // 将传入的Description移动赋值给Info中的Description成员，转移所有权，存储Actor的描述信息
    Info->Description = std::move(Description);  
    // 通过ATagger的函数获取Actor的语义标签，并存储到Info的SemanticTags成员中
    ATagger::GetTagsOfTaggedActor(Actor, Info->SemanticTags);  
    // 通过边界框计算函数获取Actor的边界框信息，并存储到Info的BoundingBox成员中
    Info->BoundingBox = UBoundingBoxCalculator::GetActorBoundingBox(&Actor);  


  if (Info->Description.Id.IsEmpty())
  {
    // This is a fake actor, let's fake the id based on their semantic tags.
    Info->Description.Id = TEXT("static.") + CarlaGetRelevantTagAsString(Info->SemanticTags);
  }

  Info->SerializedData.id = Id;
  Info->SerializedData.description = Info->Description;
  Info->SerializedData.bounding_box = Info->BoundingBox;
  Info->SerializedData.semantic_tags.reserve(Info->SemanticTags.Num());
  for (auto &&Tag : Info->SemanticTags)
  {
    using tag_t = decltype(Info->SerializedData.semantic_tags)::value_type;
    Info->SerializedData.semantic_tags.emplace_back(static_cast<tag_t>(Tag));
  }
  auto *Sensor = Cast<ASensor>(&Actor);
  if (Sensor != nullptr)
  {
    const auto &Token = Sensor->GetToken();
    Info->SerializedData.stream_token = decltype(Info->SerializedData.stream_token)(
        std::begin(Token.data),
        std::end(Token.data));
  }
  auto Type = FActorRegistry_GetActorType(&Actor);
  TSharedPtr<FCarlaActor> CarlaActor =
      FCarlaActor::ConstructCarlaActor(
        Id, &Actor,
        std::move(Info), Type,
        InState, Actor.GetWorld());
  return CarlaActor;
}

void FActorRegistry::PutActorToSleep(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode)
{
  FCarlaActor* CarlaActor = FindCarlaActor(Id);

  // update id maps
  Actors[Id] = nullptr;
  AActor* Actor = CarlaActor->GetActor();
  if(Actor)
  {
    Ids.Remove(Actor);
  }

  CarlaActor->PutActorToSleep(CarlaEpisode);
  for (const FCarlaActor::IdType& ChildId : CarlaActor->GetChildren())
  {
    PutActorToSleep(ChildId, CarlaEpisode);
  }
  // TODO: update id maps
}

void FActorRegistry::WakeActorUp(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode)
{

  FCarlaActor* CarlaActor = FindCarlaActor(Id);
  CarlaActor->WakeActorUp(CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  if (Actor)
  {
    // update ids
    Actors[Id] = Actor;
    Ids.Add(Actor, Id);
    if (CarlaActor->GetParent())

    // 如果Info中的Description的ID为空字符串，说明可能缺少明确标识，进行以下伪造身份处理
    if (Info->Description.Id.IsEmpty())  
    {
        // 根据Actor的语义标签获取一个相关的字符串作为标识，格式为"static."加上相关标签字符串
        Info->Description.Id = TEXT("static.") + CarlaGetRelevantTagAsString(Info->SemanticTags);  
    }
  }


FString FActorRegistry::GetDescriptionFromStream(carla::streaming::detail::stream_id_type Id)
{
  for (auto &Item : ActorDatabase)
  {
    // check for a sensor
    ASensor *Sensor = Cast<ASensor>(Item.Value->GetActor());
    if (Sensor == nullptr) continue;

    carla::streaming::detail::token_type token(Sensor->GetToken());
    if (token.get_stream_id() == Id)

    // 将当前创建的FCarlaActor的ID赋值给Info中序列化数据的ID成员
    Info->SerializedData.id = Id;  
    // 将Info中的Description赋值给Info中序列化数据的描述成员，存储描述信息
    Info->SerializedData.description = Info->Description;  
    // 将Info中的边界框信息赋值给Info中序列化数据的边界框成员，存储边界框信息
    Info->SerializedData.bounding_box = Info->BoundingBox;  
    // 预留一定空间用于存储语义标签序列化数据，数量和Info中的语义标签集合数量一致
    Info->SerializedData.semantic_tags.reserve(Info->SemanticTags.Num());  
    // 遍历Info中的语义标签集合，将每个标签转换为合适的类型并添加到序列化数据的语义标签成员中
    for (auto &&Tag : Info->SemanticTags)  

    {
        using tag_t = decltype(Info->SerializedData.semantic_tags)::value_type;  
        Info->SerializedData.semantic_tags.emplace_back(static_cast<tag_t>(Tag));  
    }
    // 将传入的Actor指针转换为ASensor类型指针，检查是否是传感器类型的Actor
    auto *Sensor = Cast<ASensor>(&Actor);  
    if (Sensor!= nullptr)  
    {
        // 获取传感器的Token信息，并将其转换为合适的数据类型存储到Info中序列化数据的流Token成员中
        const auto &Token = Sensor->GetToken();  
        Info->SerializedData.stream_token = decltype(Info->SerializedData.stream_token)(
            std::begin(Token.data),
            std::end(Token.data));  
    }
    // 通过函数FActorRegistry_GetActorType获取Actor的类型
    auto Type = FActorRegistry_GetActorType(&Actor);  
    // 调用FCarlaActor的构造函数创建一个新的FCarlaActor对象，传入相关参数，并返回其共享指针
    TSharedPtr<FCarlaActor> CarlaActor =
        FCarlaActor::ConstructCarlaActor(
            Id, &Actor,
            std::move(Info), Type,
            InState, Actor.GetWorld());  
    return CarlaActor;  
}   
}

// FActorRegistry类的成员函数PutActorToSleep，用于将指定ID的Actor及其子Actor设置为休眠状态
// 参数Id是要设置为休眠状态的Actor的ID类型，通过这个ID来查找对应的Actor
// 参数CarlaEpisode可能是和游戏场景、关卡等相关的对象指针，用于关联Actor与具体的游戏环境（具体依Carla架构而定）
void FActorRegistry::PutActorToSleep(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode)  
{
    // 根据传入的ID查找对应的FCarlaActor对象
    FCarlaActor* CarlaActor = FindCarlaActor(Id);  

    // 更新Actors映射中指定ID对应的Actor指针为nullptr，表示该Actor进入休眠状态（从活跃关联中移除）
    Actors[Id] = nullptr;  
    // 获取对应的AActor指针
    AActor* Actor = CarlaActor->GetActor();  
    // 如果Actor指针不为空，从Ids映射中移除该Actor指针与ID的反向关联（解除关联）
    if (Actor)  
    {
        Ids.Remove(Actor);  
    }

    // 调用FCarlaActor对象的PutActorToSleep函数，将其自身设置为休眠状态，并传入相关游戏环境对象指针
    CarlaActor->PutActorToSleep(CarlaEpisode);  
    // 遍历该Actor的所有子Actor的ID列表，递归调用PutActorToSleep函数，将子Actor也设置为休眠状态
    for (const FCarlaActor::IdType& ChildId : CarlaActor->GetChildren())  
    {
        PutActorToSleep(ChildId, CarlaEpisode);  
    }
    // TODO：更新ID映射，这里可能是后续需要完善的部分，比如进一步处理休眠状态下ID映射的一些细节调整等（目前代码未完整实现这部分功能）
}
