#include "Carla.h"
#include "TaggedComponent.h"
#include "ConstructorHelpers.h"

#include "Rendering/SkeletalMeshRenderData.h"
#include "SkeletalRenderPublic.h"

//
// UTaggedComponent
//
UTaggedComponent::UTaggedComponent(const FObjectInitializer& ObjectInitializer) :
  UPrimitiveComponent(ObjectInitializer),
  Color(1, 1, 1, 1)
{
  FString MaterialPath = TEXT("Material'/Carla/PostProcessingMaterials/AnnotationColor.AnnotationColor'");
  static ConstructorHelpers::FObjectFinder<UMaterial> TaggedMaterialObject(*MaterialPath);
  // TODO: Replace with VertexColorViewModeMaterial_ColorOnly?

  TaggedMaterial = TaggedMaterialObject.Object;
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = false;
}

// UTaggedComponent类的成员函数，用于在组件注册时调用
void UTaggedComponent::OnRegister()
{
  // 调用父类的OnRegister函数
  Super::OnRegister();

  // 创建动态材质实例
  TaggedMID = UMaterialInstanceDynamic::Create(TaggedMaterial, this, TEXT("TaggedMaterialMID"));

  // 检查动态材质实例是否创建成功
  if (!IsValid(TaggedMID))
  {
    // 如果失败，记录错误日志
    UE_LOG(LogCarla, Error, TEXT("Failed to create MID!"));
  }

  // 设置组件的颜色
  SetColor(Color);
}

// UTaggedComponent类的成员函数，用于设置组件的颜色
void UTaggedComponent::SetColor(FLinearColor NewColor)
{
  // 更新颜色成员变量
  Color = NewColor;

  // 如果动态材质实例有效，则设置其颜色参数
  if (IsValid(TaggedMID))
  {
    TaggedMID->SetVectorParameterValue("AnnotationColor", Color);
  }
}

// UTaggedComponent类的成员函数，用于获取组件的颜色
FLinearColor UTaggedComponent::GetColor()
{
  return Color;
}

// UTaggedComponent类的成员函数，用于计算组件的边界
FBoxSphereBounds UTaggedComponent::CalcBounds(const FTransform & LocalToWorld) const
{
  // 获取父组件
  USceneComponent * ParentComponent = GetAttachParent();
  if(ParentComponent)
  {
    // 如果父组件有效，返回父组件的边界
    return ParentComponent->CalcBounds(LocalToWorld);
  }
  // 如果没有父组件，返回空的边界
  return FBoxSphereBounds();
}

// UTaggedComponent类的成员函数，用于创建场景代理
FPrimitiveSceneProxy * UTaggedComponent::CreateSceneProxy()
{
  USceneComponent * ParentComponent = GetAttachParent();

  // 如果父组件无效，记录错误日志并返回NULL
  if (!IsValid(ParentComponent))
  {
    UE_LOG(LogCarla, Error, TEXT("Invalid parent component"));
    return NULL;
  }

  // 尝试将父组件转换为不同类型的网格组件
  USkeletalMeshComponent * SkeletalMeshComponent = Cast<USkeletalMeshComponent>(ParentComponent);
  UStaticMeshComponent * StaticMeshComponent = Cast<UStaticMeshComponent>(ParentComponent);
  UHierarchicalInstancedStaticMeshComponent * HierarchicalInstancedStaticMeshComponent =
      Cast<UHierarchicalInstancedStaticMeshComponent>(ParentComponent);
  UInstancedStaticMeshComponent* InstancedStaticMeshComponent = Cast<UInstancedStaticMeshComponent>(ParentComponent);
  // 如果父组件是层级实例化静态网格组件，创建对应的场景代理
  if (HierarchicalInstancedStaticMeshComponent)
  {
    return CreateSceneProxy(HierarchicalInstancedStaticMeshComponent);
  }
  // 如果父组件是实例化静态网格组件，创建对应的场景代理
  else if (InstancedStaticMeshComponent)
  {
    return CreateSceneProxy(InstancedStaticMeshComponent);
  }
  // 如果父组件是静态网格组件，创建对应的场景代理
  else
  if (IsValid(StaticMeshComponent))
  {
    return CreateSceneProxy(StaticMeshComponent);
  }
  // 如果父组件是骨骼网格组件，设置bSkeletalMesh标志并创建对应的场景代理
  else if (IsValid(SkeletalMeshComponent))
  {
    bSkeletalMesh = true;
    return CreateSceneProxy(SkeletalMeshComponent);
  }

  // 如果父组件类型未知，记录错误日志并返回NULL
  UE_LOG(LogCarla, Error, TEXT("Unknown type of parent component: %s"), *ParentComponent->GetClass()->GetName());

  return NULL;
}

FPrimitiveSceneProxy * UTaggedComponent::CreateSceneProxy(UStaticMeshComponent * StaticMeshComponent)
{
  // 确保静态网格具有渲染数据。
  UStaticMesh * StaticMesh = StaticMeshComponent->GetStaticMesh();

  // 检查静态网格是否为空，如果为空则记录错误日志并返回NULL
  if (StaticMesh == NULL)
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to create scene proxy for static mesh component (because static mesh is null): %s"), *StaticMeshComponent->GetReadableName());
    return NULL;
  }

  // 检查静态网格的渲染数据是否为空，如果为空则记录错误日志并返回NULL
  if (StaticMesh->RenderData == NULL)
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to create scene proxy for static mesh component (because render data is null): %s"), *StaticMeshComponent->GetReadableName());
    return NULL;
  }

  // 检查静态网格的LOD资源数量是否为0，如果是则记录错误日志并返回NULL
  if (StaticMesh->RenderData->LODResources.Num() == 0)
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to create scene proxy for static mesh component (because num LOD resources is 0): %s"), *StaticMeshComponent->GetReadableName());
    return NULL;
  }

  // 如果所有检查通过，则创建并返回标记的静态网格场景代理
  return new FTaggedStaticMeshSceneProxy(StaticMeshComponent, true, TaggedMID);
}

// UTaggedComponent类的成员函数，用于为骨骼网格组件创建场景代理
FPrimitiveSceneProxy * UTaggedComponent::CreateSceneProxy(USkeletalMeshComponent * SkeletalMeshComponent)
{
  // 如果应该等待当前帧，则返回nullptr
  if (bShouldWaitFrame)
  {
    return nullptr;
  }
  // 获取当前场景的特征级别
  ERHIFeatureLevel::Type SceneFeatureLevel = GetWorld()->FeatureLevel;
	// 获取骨骼网格渲染数据
	FSkeletalMeshRenderData* SkelMeshRenderData = SkeletalMeshComponent->GetSkeletalMeshRenderData();

	// 只有在正确初始化后，才为渲染创建场景代理。
	if (SkelMeshRenderData &&
		SkelMeshRenderData->LODRenderData.IsValidIndex(SkeletalMeshComponent->PredictedLODLevel) &&
		!SkeletalMeshComponent->bHideSkin &&
		SkeletalMeshComponent->MeshObject)
	{
		// 只有在使用的骨骼数量被支持的情况下，或者如果没有骨架（如破坏物体的情况），才创建场景代理。
		int32 MinLODIndex = SkeletalMeshComponent->ComputeMinLOD();
		int32 MaxBonesPerChunk = SkelMeshRenderData->GetMaxBonesPerSection(MinLODIndex);
		int32 MaxSupportedNumBones = SkeletalMeshComponent->MeshObject->IsCPUSkinned() ? MAX_int32 : GetFeatureLevelMaxNumberOfBones(SceneFeatureLevel);
		if (MaxBonesPerChunk <= MaxSupportedNumBones)
		{
			return new FTaggedSkeletalMeshSceneProxy(SkeletalMeshComponent, SkelMeshRenderData, TaggedMID);
		}
	}
  return nullptr;
}

FPrimitiveSceneProxy * UTaggedComponent::CreateSceneProxy(UHierarchicalInstancedStaticMeshComponent * MeshComponent)
{
	// 在使用网格之前，验证其是否有效。
	const bool bMeshIsValid =
		//确保我们拥有实例。
		(MeshComponent->PerInstanceRenderData.IsValid()) &&
		// 确保我们拥有一个实际的静态网格。
		MeshComponent->GetStaticMesh() &&
		MeshComponent->GetStaticMesh()->HasValidRenderData(false) &&
		// 在控制台上，无法使用硬件实例化处理多个元素，因为它们共享相同的索引缓冲区。
		// @todo: Level error or something to let LDs know this
		1;//GetStaticMesh()->LODModels(0).Elements.Num() == 1;

	if (bMeshIsValid)
	{
		bool bIsGrass = !MeshComponent->PerInstanceSMData.Num();
		return new FTaggedHierarchicalStaticMeshSceneProxy(MeshComponent, bIsGrass, GetWorld()->FeatureLevel, TaggedMID);
	}
	return nullptr;
}

FPrimitiveSceneProxy * UTaggedComponent::CreateSceneProxy(UInstancedStaticMeshComponent * MeshComponent)
{
	// 在使用网格之前，验证其是否有效。
	const bool bMeshIsValid =
		// 确保我们拥有实例。
		(MeshComponent->PerInstanceRenderData.IsValid()) &&
		// 确保我们拥有一个实际的静态网格。
		MeshComponent->GetStaticMesh() &&
		MeshComponent->GetStaticMesh()->HasValidRenderData(false) &&
		// 在控制台上，无法使用硬件实例化处理多个元素，因为它们共享相同的索引缓冲区。
		// @todo: Level error or something to let LDs know this
		1;//GetStaticMesh()->LODModels(0).Elements.Num() == 1;

	if (bMeshIsValid)
	{
		return new FTaggedInstancedStaticMeshSceneProxy(MeshComponent, GetWorld()->FeatureLevel, TaggedMID);
	}
	return nullptr;
}

void UTaggedComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  // // TODO: Try removing this
  if (bSkeletalMesh)
  {
    // MarkRenderTransformDirty();
    MarkRenderStateDirty();
    if(bShouldWaitFrame)
    {
      if(NumFramesToWait < 0)
      {
        bShouldWaitFrame = false;
      }
      NumFramesToWait--;
    }
  }
}

//
// FTaggedStaticMeshSceneProxy
//
FTaggedStaticMeshSceneProxy::FTaggedStaticMeshSceneProxy(UStaticMeshComponent * Component, bool bForceLODsShareStaticLighting, UMaterialInstance * MaterialInstance) :
  FStaticMeshSceneProxy(Component, bForceLODsShareStaticLighting)
{
  TaggedMaterialInstance = MaterialInstance;

  // 用带标签的材质替换材质。
  bVerifyUsedMaterials = false;

  for (FLODInfo& LODInfo : LODs) {
    for (FLODInfo::FSectionInfo& SectionInfo : LODInfo.Sections) {
        SectionInfo.Material = TaggedMaterialInstance;
    }
  }
}

FPrimitiveViewRelevance FTaggedStaticMeshSceneProxy::GetViewRelevance(const FSceneView * View) const
{
  FPrimitiveViewRelevance ViewRelevance = FStaticMeshSceneProxy::GetViewRelevance(View);

  ViewRelevance.bDrawRelevance = ViewRelevance.bDrawRelevance && !View->Family->EngineShowFlags.NotDrawTaggedComponents;
  ViewRelevance.bShadowRelevance = false;

  return ViewRelevance;
}

//
// FTaggedSkeletalMeshSceneProxy
//
FTaggedSkeletalMeshSceneProxy::FTaggedSkeletalMeshSceneProxy(const USkinnedMeshComponent * Component, FSkeletalMeshRenderData * InSkeletalMeshRenderData, UMaterialInstance * MaterialInstance) :
  FSkeletalMeshSceneProxy(Component, InSkeletalMeshRenderData)
{
  TaggedMaterialInstance = MaterialInstance;

  // Replace materials with tagged material
  bVerifyUsedMaterials = false;

  for (FLODSectionElements& LODSection : LODSections) {
    for (FSectionElementInfo& ElementInfo : LODSection.SectionElements) {
        ElementInfo.Material = TaggedMaterialInstance;
    }
  }
}

FPrimitiveViewRelevance FTaggedSkeletalMeshSceneProxy::GetViewRelevance(const FSceneView * View) const
{
  FPrimitiveViewRelevance ViewRelevance = FSkeletalMeshSceneProxy::GetViewRelevance(View);

  ViewRelevance.bDrawRelevance = ViewRelevance.bDrawRelevance && !View->Family->EngineShowFlags.NotDrawTaggedComponents;
  ViewRelevance.bShadowRelevance = false;

  return ViewRelevance;
}

FTaggedInstancedStaticMeshSceneProxy::FTaggedInstancedStaticMeshSceneProxy(
    UInstancedStaticMeshComponent * Component, ERHIFeatureLevel::Type InFeatureLevel, UMaterialInstance * MaterialInstance)
  : FInstancedStaticMeshSceneProxy(Component, InFeatureLevel)
{
  TaggedMaterialInstance = MaterialInstance;

  // Replace materials with tagged material
  bVerifyUsedMaterials = false;

  for (FLODInfo& LODInfo : LODs) {
    for (FLODInfo::FSectionInfo& SectionInfo : LODInfo.Sections) {
        SectionInfo.Material = TaggedMaterialInstance;
    }
  }
}

FPrimitiveViewRelevance FTaggedInstancedStaticMeshSceneProxy::GetViewRelevance(const FSceneView * View) const
{
  FPrimitiveViewRelevance ViewRelevance = FInstancedStaticMeshSceneProxy::GetViewRelevance(View);

  ViewRelevance.bDrawRelevance = ViewRelevance.bDrawRelevance && !View->Family->EngineShowFlags.NotDrawTaggedComponents;
  ViewRelevance.bShadowRelevance = false;

  return ViewRelevance;
}


FTaggedHierarchicalStaticMeshSceneProxy::FTaggedHierarchicalStaticMeshSceneProxy(
    UHierarchicalInstancedStaticMeshComponent * Component, bool bInIsGrass, ERHIFeatureLevel::Type InFeatureLevel, UMaterialInstance * MaterialInstance)
  : FHierarchicalStaticMeshSceneProxy(bInIsGrass, Component, InFeatureLevel)
{
  TaggedMaterialInstance = MaterialInstance;

  // Replace materials with tagged material
  bVerifyUsedMaterials = false;

  for (FLODInfo& LODInfo : LODs) {
    for (FLODInfo::FSectionInfo& SectionInfo : LODInfo.Sections) {
        SectionInfo.Material = TaggedMaterialInstance;
    }
  }
}

FPrimitiveViewRelevance FTaggedHierarchicalStaticMeshSceneProxy::GetViewRelevance(const FSceneView * View) const
{
  FPrimitiveViewRelevance ViewRelevance = FHierarchicalStaticMeshSceneProxy::GetViewRelevance(View);

  ViewRelevance.bDrawRelevance = ViewRelevance.bDrawRelevance && !View->Family->EngineShowFlags.NotDrawTaggedComponents;
  ViewRelevance.bShadowRelevance = false;

  return ViewRelevance;
}
