#include "Carla.h"
#include "TaggedComponent.h"
#include "ConstructorHelpers.h"

#include "Rendering/SkeletalMeshRenderData.h"
#include "SkeletalRenderPublic.h"

//
// UTaggedComponent类实现
//

// 构造函数：初始化基本成员变量和默认材质
UTaggedComponent::UTaggedComponent(const FObjectInitializer& ObjectInitializer) :
  UPrimitiveComponent(ObjectInitializer),
  Color(1, 1, 1, 1)  // 默认颜色为白色
{
  // 设置材质路径
  FString MaterialPath = TEXT("Material'/Carla/PostProcessingMaterials/AnnotationColor.AnnotationColor'");
  static ConstructorHelpers::FObjectFinder<UMaterial> TaggedMaterialObject(*MaterialPath);

  // 获取材质对象
  TaggedMaterial = TaggedMaterialObject.Object;

  // 启用每帧的Tick
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = false;
}

// 注册组件时调用：用于创建动态材质实例，并初始化颜色
void UTaggedComponent::OnRegister()
{
  // 调用父类的注册方法
  Super::OnRegister();

  // 创建动态材质实例
  TaggedMID = UMaterialInstanceDynamic::Create(TaggedMaterial, this, TEXT("TaggedMaterialMID"));

  // 检查材质实例是否成功创建
  if (!IsValid(TaggedMID))
  {
    // 创建失败时输出错误信息
    UE_LOG(LogCarla, Error, TEXT("Failed to create MID!"));
  }

  // 设置组件的颜色
  SetColor(Color);
}

// 设置组件颜色
void UTaggedComponent::SetColor(FLinearColor NewColor)
{
  Color = NewColor;

  // 如果材质实例有效，则设置颜色参数
  if (IsValid(TaggedMID))
  {
    TaggedMID->SetVectorParameterValue("AnnotationColor", Color);
  }
}

// 获取当前组件的颜色
FLinearColor UTaggedComponent::GetColor()
{
  return Color;
}

// 计算并返回组件的边界
FBoxSphereBounds UTaggedComponent::CalcBounds(const FTransform & LocalToWorld) const
{
  // 获取父组件
  USceneComponent* ParentComponent = GetAttachParent();
  if(ParentComponent)
  {
    // 如果父组件有效，返回其边界
    return ParentComponent->CalcBounds(LocalToWorld);
  }

  // 如果没有父组件，则返回空边界
  return FBoxSphereBounds();
}

// 创建场景代理：根据父组件类型创建不同的场景代理
FPrimitiveSceneProxy* UTaggedComponent::CreateSceneProxy()
{
  // 获取父组件
  USceneComponent* ParentComponent = GetAttachParent();

  // 如果父组件无效，记录错误日志并返回null
  if (!IsValid(ParentComponent))
  {
    UE_LOG(LogCarla, Error, TEXT("Invalid parent component"));
    return NULL;
  }

  // 尝试将父组件转换为不同类型的网格组件
  USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(ParentComponent);
  UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(ParentComponent);
  UHierarchicalInstancedStaticMeshComponent* HierarchicalInstancedStaticMeshComponent =
      Cast<UHierarchicalInstancedStaticMeshComponent>(ParentComponent);
  UInstancedStaticMeshComponent* InstancedStaticMeshComponent = Cast<UInstancedStaticMeshComponent>(ParentComponent);

  // 根据父组件类型创建不同的场景代理
  if (HierarchicalInstancedStaticMeshComponent)
  {
    return CreateSceneProxy(HierarchicalInstancedStaticMeshComponent);
  }
  else if (InstancedStaticMeshComponent)
  {
    return CreateSceneProxy(InstancedStaticMeshComponent);
  }
  else if (IsValid(StaticMeshComponent))
  {
    return CreateSceneProxy(StaticMeshComponent);
  }
  else if (IsValid(SkeletalMeshComponent))
  {
    bSkeletalMesh = true;
    return CreateSceneProxy(SkeletalMeshComponent);
  }

  // 如果父组件类型未知，记录错误并返回null
  UE_LOG(LogCarla, Error, TEXT("Unknown type of parent component: %s"), *ParentComponent->GetClass()->GetName());

  return NULL;
}

// 为静态网格组件创建场景代理
FPrimitiveSceneProxy* UTaggedComponent::CreateSceneProxy(UStaticMeshComponent* StaticMeshComponent)
{
  // 获取静态网格和渲染数据
  UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();

  // 检查静态网格是否有效
  if (StaticMesh == NULL || StaticMesh->RenderData == NULL || StaticMesh->RenderData->LODResources.Num() == 0)
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to create scene proxy for static mesh component: %s"), *StaticMeshComponent->GetReadableName());
    return NULL;
  }

  // 创建静态网格场景代理
  return new FTaggedStaticMeshSceneProxy(StaticMeshComponent, true, TaggedMID);
}

// 为骨骼网格组件创建场景代理
FPrimitiveSceneProxy* UTaggedComponent::CreateSceneProxy(USkeletalMeshComponent* SkeletalMeshComponent)
{
  // 骨骼网格渲染数据
  FSkeletalMeshRenderData* SkelMeshRenderData = SkeletalMeshComponent->GetSkeletalMeshRenderData();

  // 检查骨骼网格的有效性
  if (SkelMeshRenderData && SkelMeshRenderData->LODRenderData.IsValidIndex(SkeletalMeshComponent->PredictedLODLevel) &&
      !SkeletalMeshComponent->bHideSkin && SkeletalMeshComponent->MeshObject)
  {
    int32 MinLODIndex = SkeletalMeshComponent->ComputeMinLOD();
    int32 MaxBonesPerChunk = SkelMeshRenderData->GetMaxBonesPerSection(MinLODIndex);
    int32 MaxSupportedNumBones = SkeletalMeshComponent->MeshObject->IsCPUSkinned() ? MAX_int32 : GetFeatureLevelMaxNumberOfBones(GetWorld()->FeatureLevel);

    if (MaxBonesPerChunk <= MaxSupportedNumBones)
    {
      // 返回带标签的骨骼网格场景代理
      return new FTaggedSkeletalMeshSceneProxy(SkeletalMeshComponent, SkelMeshRenderData, TaggedMID);
    }
  }

  // 骨骼网格无效时返回nullptr
  return nullptr;
}

// 为层级实例化静态网格组件创建场景代理
FPrimitiveSceneProxy* UTaggedComponent::CreateSceneProxy(UHierarchicalInstancedStaticMeshComponent* MeshComponent)
{
  const bool bMeshIsValid =
    MeshComponent->PerInstanceRenderData.IsValid() &&
    MeshComponent->GetStaticMesh() && MeshComponent->GetStaticMesh()->HasValidRenderData(false);

  // 如果网格有效，创建层级实例化静态网格场景代理
  if (bMeshIsValid)
  {
    bool bIsGrass = !MeshComponent->PerInstanceSMData.Num();
    return new FTaggedHierarchicalStaticMeshSceneProxy(MeshComponent, bIsGrass, GetWorld()->FeatureLevel, TaggedMID);
  }

  // 如果无效返回nullptr
  return nullptr;
}

// 为实例化静态网格组件创建场景代理
FPrimitiveSceneProxy* UTaggedComponent::CreateSceneProxy(UInstancedStaticMeshComponent* MeshComponent)
{
  const bool bMeshIsValid =
    MeshComponent->PerInstanceRenderData.IsValid() &&
    MeshComponent->GetStaticMesh() && MeshComponent->GetStaticMesh()->HasValidRenderData(false);

  // 如果网格有效，创建实例化静态网格场景代理
  if (bMeshIsValid)
  {
    return new FTaggedInstancedStaticMeshSceneProxy(MeshComponent, GetWorld()->FeatureLevel, TaggedMID);
  }

  return nullptr;
}

// 每帧调用的Tick函数，用于处理动态更新
void UTaggedComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  // 对骨骼网格组件进行特殊处理
  if (bSkeletalMesh)
  {
    // 标记渲染状态脏
    MarkRenderStateDirty();

    // 控制延迟帧数
    if (bShouldWaitFrame)
    {
      if (NumFramesToWait < 0)
      {
        bShouldWaitFrame = false;
      }
      NumFramesToWait--;
    }
  }
}

//
// FTaggedStaticMeshSceneProxy实现
//

FTaggedStaticMeshSceneProxy::FTaggedStaticMeshSceneProxy(UStaticMeshComponent* Component, bool bForceLODsShareStaticLighting, UMaterialInstance* MaterialInstance) :
  FStaticMeshSceneProxy(Component, bForceLODsShareStaticLighting)
{
  TaggedMaterialInstance = MaterialInstance;

  // 用带标签的材质替换材质
  bVerifyUsedMaterials = false;

  for (FLODInfo& LODInfo : LODs) {
    for (FLODInfo::FSectionInfo& SectionInfo : LODInfo.Sections) {
        SectionInfo.Material = TaggedMaterialInstance;
    }
  }
}

// 获取视图的可见性
FPrimitiveViewRelevance FTaggedStaticMeshSceneProxy::GetViewRe
