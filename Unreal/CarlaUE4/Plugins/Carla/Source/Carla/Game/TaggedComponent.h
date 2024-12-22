// 这是一个预处理指令，用于确保头文件只被包含一次，避免重复定义的问题
#pragma once  

// 包含核心最小化相关的头文件，通常包含了一些基础的UE4类型和宏等常用定义
#include "CoreMinimal.h"  
// 包含材质实例动态相关的头文件，用于在运行时动态修改材质属性等操作
#include "Materials/MaterialInstanceDynamic.h"  
// 包含实例化静态网格相关的头文件，用于处理场景中的实例化静态网格对象
#include "Engine/InstancedStaticMesh.h"  
// 包含分层实例化静态网格组件相关的头文件，用于更复杂的实例化静态网格场景组织等
#include "Components/HierarchicalInstancedStaticMeshComponent.h"  

// 这个宏用于声明一个类是可被蓝图生成的组件类，并且开启反射相关功能，以便在UE4编辑器等环境中使用
#include "TaggedComponent.generated.h"  

// 定义一个名为UTaggedComponent的类，它继承自UPrimitiveComponent，意味着它可以作为一个可渲染的组件在场景中存在
// 并且具有UPrimitiveComponent的相关特性和功能
UCLASS( meta=(BlueprintSpawnableComponent) )  
class CARLA_API UTaggedComponent : public UPrimitiveComponent
{
    // 这个宏用于生成必要的反射代码，是UE4中实现反射机制的一部分，使得类能够在编辑器等环境中被正确识别和处理
    GENERATED_BODY()  

public:
    // 构造函数，接受一个FObjectInitializer参数用于初始化对象的相关属性等
    UTaggedComponent(const FObjectInitializer& ObjectInitializer);  

    // 重写创建场景代理的虚函数，用于创建该组件对应的场景代理对象，场景代理负责将组件的数据传递给渲染管线等相关操作
    virtual FPrimitiveSceneProxy * CreateSceneProxy() override;  

    // 重写每帧更新组件的虚函数，在这里可以实现组件每帧需要执行的逻辑，例如根据时间变化更新状态等
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) override;  

    // 重写组件注册时调用的虚函数，可用于在组件注册到场景时执行一些初始化或者其他必要操作
    virtual void OnRegister() override;  

    // 计算组件的包围盒（边界范围）信息，根据给定的本地到世界的变换矩阵来确定其在世界空间中的边界范围
    virtual FBoxSphereBounds CalcBounds(const FTransform & LocalToWorld) const ;  

    // 设置颜色的函数，目前的实现存在问题（FIXME注释指出），应该接受一个int32类型参数并转换为FVector4类型来设置颜色，这里使用FLinearColor类型接收颜色参数
    void SetColor(FLinearColor color);  

    // 获取当前组件颜色的函数
    FLinearColor GetColor();  

private:
    // 存储组件的颜色信息，使用FLinearColor类型表示颜色
    FLinearColor Color;  

    // 一个属性，用于存储材质指针，使用UPROPERTY宏标记，这样该属性可以被UE4的反射系统识别和处理，比如可以在编辑器中进行赋值等操作
    UPROPERTY()  
    UMaterial * TaggedMaterial;  

    // 存储材质实例动态对象的指针，用于在运行时动态修改材质相关属性，同样通过UPROPERTY宏标记以便被UE4系统正确处理
    UPROPERTY()  
    UMaterialInstanceDynamic * TaggedMID;  

    // 一个布尔变量，用于标记是否是骨骼网格，初始化为false
    bool bSkeletalMesh = false;  

    // 以下几个重载的CreateSceneProxy函数，分别用于针对不同类型的网格组件（静态网格、骨骼网格、分层实例化静态网格、实例化静态网格）创建对应的场景代理对象
    FPrimitiveSceneProxy * CreateSceneProxy(UStaticMeshComponent * StaticMeshComponent);  
    FPrimitiveSceneProxy * CreateSceneProxy(USkeletalMeshComponent * SkeletalMeshComponent);  
    FPrimitiveSceneProxy * CreateSceneProxy(UHierarchicalInstancedStaticMeshComponent * MeshComponent);  
    FPrimitiveSceneProxy * CreateSceneProxy(UInstancedStaticMeshComponent * MeshComponent);  

    // 一个小的“hack”（临时解决办法），用于让虚幻引擎在骨骼网格情况下初始化基础组件，通过控制等待帧数来实现
    bool bShouldWaitFrame = true;  
    int NumFramesToWait = 2;  
};

// 定义一个名为FTaggedStaticMeshSceneProxy的类，它继承自FStaticMeshSceneProxy，用于处理静态网格组件的场景代理相关特定逻辑
class FTaggedStaticMeshSceneProxy : public FStaticMeshSceneProxy
{
public:
    // 构造函数，接受静态网格组件指针、一个布尔值（用于控制是否强制LOD共享静态光照）以及材质实例指针作为参数，用于初始化该场景代理对象
    FTaggedStaticMeshSceneProxy(UStaticMeshComponent * Component, bool bForceLODsShareStaticLighting, UMaterialInstance * MaterialInstance);  

    // 重写获取视图相关性的虚函数，用于确定该场景代理在给定视图下的相关性，例如是否可见、是否需要渲染等相关判断
    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView * View) const override;  

private:
    // 存储材质实例指针，用于该场景代理相关的材质处理等操作
    UMaterialInstance * TaggedMaterialInstance;  
};

// 定义一个名为FTaggedSkeletalMeshSceneProxy的类，它继承自FSkeletalMeshSceneProxy，用于处理骨骼网格组件的场景代理相关特定逻辑
class FTaggedSkeletalMeshSceneProxy : public FSkeletalMeshSceneProxy
{
public:
    // 构造函数，接受骨骼网格组件指针、骨骼网格渲染数据指针以及材质实例指针作为参数，用于初始化该场景代理对象
    FTaggedSkeletalMeshSceneProxy(const USkinnedMeshComponent * Component, FSkeletalMeshRenderData * InSkeletalMeshRenderData, UMaterialInstance * MaterialInstance);  

    // 重写获取视图相关性的虚函数，用于确定该场景代理在给定视图下的相关性，与上面类似，用于判断渲染相关情况
    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView * View) const override;  

private:
    // 存储材质实例指针，用于该场景代理对应的骨骼网格材质相关操作
    UMaterialInstance * TaggedMaterialInstance;  
};

// 定义一个名为FTaggedInstancedStaticMeshSceneProxy的类，它继承自FInstancedStaticMeshSceneProxy，用于处理实例化静态网格组件的场景代理相关特定逻辑
class FTaggedInstancedStaticMeshSceneProxy : public FInstancedStaticMeshSceneProxy
{
public:
    // 构造函数，接受实例化静态网格组件指针、渲染特性级别类型以及材质实例指针作为参数，用于初始化该场景代理对象
    FTaggedInstancedStaticMeshSceneProxy(UInstancedStaticMeshComponent * Component, ERHIFeatureLevel::Type InFeatureLevel, UMaterialInstance * MaterialInstance);  

    // 重写获取视图相关性的虚函数，用于判断该场景代理在给定视图下是否需要渲染等相关情况
    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView * View) const override;  

private:
    // 存储材质实例指针，用于实例化静态网格场景代理的材质相关处理
    UMaterialInstance * TaggedMaterialInstance;  
};

// 定义一个名为FTaggedHierarchicalStaticMeshSceneProxy的类，它继承自FHierarchicalStaticMeshSceneProxy，用于处理分层实例化静态网格组件的场景代理相关特定逻辑
class FTaggedHierarchicalStaticMeshSceneProxy : public FHierarchicalStaticMeshSceneProxy
{
public:
    // 构造函数，接受分层实例化静态网格组件指针、一个布尔值（是否是草相关标记）、渲染特性级别类型以及材质实例指针作为参数，用于初始化该场景代理对象
    FTaggedHierarchicalStaticMeshSceneProxy(UHierarchicalInstancedStaticMeshComponent * Component, bool bInIsGrass, ERHIFeatureLevel::Type InFeatureLevel, UMaterialInstance * MaterialInstance);  

    // 重写获取视图相关性的虚函数，用于确定该场景代理在给定视图下的渲染相关性情况
    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView * View) const override;  

private:
    // 存储材质实例指针，用于分层实例化静态网格场景代理对应的材质处理操作
    UMaterialInstance * TaggedMaterialInstance;  
};
