// 版权所有 （c） 巴塞罗那自治大学 （UAB） 2020 计算机视觉中心 （CVC）。
//
// 本作品根据 MIT 许可证的条款进行许可。
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>。



// 包含静态网格物体工厂相关的头文件，可能定义了创建静态网格物体相关的类、结构体等基础内容
#include "StaticMeshFactory.h"  
// 包含Carla中用于Actor蓝图功能库的头文件，里面应该有和Actor蓝图操作相关的函数等定义
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"  
// 包含虚幻引擎中静态网格物体Actor的头文件，用于操作相关的Actor类型（继承自Actor，代表场景中的静态网格物体实体）
#include "Engine/StaticMeshActor.h"  
// 包含Carla游戏中Carla剧集相关的头文件，可能和游戏中的场景、关卡等相关概念有关联（具体依Carla框架而定）
#include "Carla/Game/CarlaEpisode.h"  

// AStaticMeshFactory类的成员函数，用于获取Actor定义相关信息，返回一个包含FActorDefinition类型元素的数组
TArray<FActorDefinition> AStaticMeshFactory::GetDefinitions()  
{
    // 使用别名ABFL来简化UActorBlueprintFunctionLibrary的书写，方便后续代码使用（一种代码简洁性的写法）
    using ABFL = UActorBlueprintFunctionLibrary;  
    // 通过UActorBlueprintFunctionLibrary中的MakeGenericDefinition函数创建一个通用的Actor定义（名为StaticMeshDefinition）
    // 参数依次为：定义的名称（这里是"static"），类型（"prop"），分类（"mesh"），具体含义取决于Carla框架中对Actor定义的规范
    auto StaticMeshDefinition = ABFL::MakeGenericDefinition(
        TEXT("static"),
        TEXT("prop"),
        TEXT("mesh"));  
    // 设置这个Actor定义对应的类为AStaticMeshActor类，表明这个定义关联到具体的静态网格物体Actor类型
    StaticMeshDefinition.Class = AStaticMeshActor::StaticClass();  
    // 向StaticMeshDefinition的Variations数组中添加一个Actor变化（FActorVariation类型）的元素，用于描述Actor的可变属性
    // 这里属性名为"mesh_path"，类型是字符串类型（EActorAttributeType::String），初始值为空字符串，并且不是必需属性（false表示非必需）
    StaticMeshDefinition.Variations.Emplace(FActorVariation{
        TEXT("mesh_path"),
        EActorAttributeType::String,
        {""}, false});  
    // 同样添加一个名为"mass"的Actor变化元素，类型是浮点型，初始值为空字符串，非必需属性，用于描述Actor质量相关的可变属性
    StaticMeshDefinition.Variations.Emplace(FActorVariation{
        TEXT("mass"),
        EActorAttributeType::Float,
        {""}, false});  
    // 再添加一个名为"scale"的Actor变化元素，类型是浮点型，初始值为"1.0f"，非必需属性，可能用于描述Actor的缩放相关可变属性
    StaticMeshDefinition.Variations.Emplace(FActorVariation{
        TEXT("scale"),
        EActorAttributeType::Float,
        {"1.0f"}, false});  
    // 返回包含这个定义的数组（目前数组中就这一个元素，即刚刚配置好的StaticMeshDefinition）
    return { StaticMeshDefinition };  
}

// AStaticMeshFactory类的成员函数，用于生成一个Actor（在指定的变换位置和根据给定的Actor描述信息），返回生成结果（FActorSpawnResult类型）
FActorSpawnResult AStaticMeshFactory::SpawnActor(
    const FTransform &SpawnAtTransform,
    const FActorDescription &ActorDescription)  
{
    using ABFL = UActorBlueprintFunctionLibrary;
    // 获取当前所在的世界指针，如果为空，表示没有有效的世界场景，那就无法生成Actor（后续会进行相应错误处理）
    auto *World = GetWorld();  
    if (World == nullptr)
    {
        // 使用UE_LOG宏输出错误日志信息，表示在空世界中无法生成网格物体Actor
        UE_LOG(LogCarla, Error, TEXT
            ("AStaticMeshFactory: cannot spawn mesh into an empty world."));  
        // 返回一个默认构造的空的FActorSpawnResult，表示生成失败
        return {};  
    }

    // 从Actor描述信息（ActorDescription）的可变属性中获取名为"scale"的属性值，并转换为浮点数类型，若获取失败则使用默认值1.0f
    float Scale = ABFL::RetrieveActorAttributeToFloat("scale", ActorDescription.Variations, 1.0f);  
    // 创建一个新的变换（FTransform类型）对象ScaledTransform，初始值为传入的生成位置变换（SpawnAtTransform）
    FTransform ScaledTransform(SpawnAtTransform);  
    // 设置ScaledTransform的缩放比例为通过前面获取到的Scale值构成的三维向量（用于对生成的Actor进行缩放设置）
    ScaledTransform.SetScale3D(FVector(Scale));  

    // 创建Actor生成参数对象，用于配置生成Actor时的一些碰撞处理等相关参数
    FActorSpawnParameters SpawnParameters;  
    // 设置Actor生成时碰撞处理的覆盖方式为总是生成（即使有碰撞也强制生成，具体碰撞处理逻辑依虚幻引擎的机制）
    SpawnParameters.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;  
    // 在世界（World）中按照给定的Actor描述信息中的类（ActorDescription.Class）、缩放后的变换（ScaledTransform）以及生成参数（SpawnParameters）生成一个静态网格物体Actor
    auto *StaticMeshActor = World->SpawnActor<AStaticMeshActor>(
        ActorDescription.Class, ScaledTransform, SpawnParameters);  

    // 将生成的静态网格物体Actor的根组件转换为静态网格物体组件（UStaticMeshComponent类型），方便后续对网格物体相关属性进行操作
    auto *StaticMeshComponent = Cast<UStaticMeshComponent>(
        StaticMeshActor->GetRootComponent());  

    if (StaticMeshComponent)
    {
        // 检查Actor描述信息的可变属性中是否包含名为"mesh_path"的属性（意味着是否有指定网格物体的路径相关信息）
        if (ActorDescription.Variations.Contains("mesh_path"))
        {
            // 从Actor描述信息的可变属性中获取名为"mesh_path"的属性值，并转换为字符串类型，若获取失败则使用默认值空字符串
            FString MeshPath = ABFL::ActorAttributeToString(
                ActorDescription.Variations["mesh_path"], "");  

            // 根据获取到的网格物体路径（MeshPath）加载对应的静态网格物体资源（UStaticMesh类型），nullptr表示默认的加载路径相关上下文（可能是全局范围查找等情况）
            UStaticMesh *Mesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);  
            // 先将静态网格物体组件的移动性设置为可移动（EComponentMobility::Movable，方便后续设置网格物体等操作）
            StaticMeshComponent->SetMobility(EComponentMobility::Movable);  
            // 尝试将加载的静态网格物体（Mesh）设置到静态网格物体组件上，如果设置失败则输出警告日志信息
            if (!StaticMeshComponent->SetStaticMesh(Mesh))
                UE_LOG(LogCarla, Warning, TEXT("Failed to set the mesh"));  
            // 再将静态网格物体组件的移动性设置回静态（EComponentMobility::Static，完成设置网格物体后的状态调整）
            StaticMeshComponent->SetMobility(EComponentMobility::Static);  

            // 检查Actor描述信息的可变属性中是否包含名为"mass"的属性（意味着是否有指定质量相关信息）
            if (ActorDescription.Variations.Contains("mass"))
            {
                // 从Actor描述信息的可变属性中获取名为"mass"的属性值，并转换为浮点数类型，若获取失败则使用默认值0.0f
                float Mass = ABFL::RetrieveActorAttributeToFloat("mass", ActorDescription.Variations, 0.0f);  
                if (Mass > 0)
                {
                    // 如果质量大于0，将静态网格物体组件的移动性设置为可移动，使其可以参与物理模拟等操作
                    StaticMeshComponent->SetMobility(EComponentMobility::Movable);  
                    // 开启静态网格物体组件的物理模拟功能，使其能受物理引擎影响（如重力、碰撞等）
                    StaticMeshComponent->SetSimulatePhysics(true);  
                    // 设置静态网格物体组件的碰撞配置文件名称为"PhysicsActor"，用于定义其碰撞相关的行为（依虚幻引擎中碰撞配置设定）
                    StaticMeshComponent->SetCollisionProfileName("PhysicsActor");  
                    // 设置静态网格物体组件的质量（以千克为单位），使用获取到的Mass值进行设置
                    StaticMeshComponent->SetMassOverrideInKg("", Mass);  
                }
            }
        }
    }
    // 返回包含生成的静态网格物体Actor指针的生成结果对象（FActorSpawnResult类型），表示生成操作完成并返回对应的Actor信息
    return FActorSpawnResult(StaticMeshActor);  
}
  }
  return FActorSpawnResult(StaticMeshActor);
}
