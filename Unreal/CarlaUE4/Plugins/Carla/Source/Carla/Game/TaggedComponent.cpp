#include "Carla.h"
#include "TaggedComponent.h"
#include "ConstructorHelpers.h"

#include "Rendering/SkeletalMeshRenderData.h"
#include "SkeletalRenderPublic.h"

//
// UTagged组件
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

void UTaggedComponent::OnRegister()
{
  Super::OnRegister();

  TaggedMID = UMaterialInstanceDynamic::Create(TaggedMaterial, this, TEXT("TaggedMaterialMID"));

  if (!IsValid(TaggedMID))
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to create MID!"));
  }

  SetColor(Color);
}

void UTaggedComponent::SetColor(FLinearColor NewColor)
{
  Color = NewColor;

  if (IsValid(TaggedMID))
  {
    TaggedMID->SetVectorParameterValue("AnnotationColor", Color);
  }
}

FLinearColor UTaggedComponent::GetColor()
{
  return Color;
}

FBoxSphereBounds UTaggedComponent::CalcBounds(const FTransform & LocalToWorld) const
{
  USceneComponent * ParentComponent = GetAttachParent();
  if(ParentComponent)
  {
    return ParentComponent->CalcBounds(LocalToWorld);
  }
  return FBoxSphereBounds();
}

FPrimitiveSceneProxy * UTaggedComponent::CreateSceneProxy()
{
  USceneComponent * ParentComponent = GetAttachParent();

  if (!IsValid(ParentComponent))
  {
    UE_LOG(LogCarla, Error, TEXT("Invalid parent component"));
    return NULL;
  }

  USkeletalMeshComponent * SkeletalMeshComponent = Cast<USkeletalMeshComponent>(ParentComponent);
  UStaticMeshComponent * StaticMeshComponent = Cast<UStaticMeshComponent>(ParentComponent);
  UHierarchicalInstancedStaticMeshComponent * HierarchicalInstancedStaticMeshComponent =
      Cast<UHierarchicalInstancedStaticMeshComponent>(ParentComponent);
  UInstancedStaticMeshComponent* InstancedStaticMeshComponent = Cast<UInstancedStaticMeshComponent>(ParentComponent);
  if (HierarchicalInstancedStaticMeshComponent)
  {
    return CreateSceneProxy(HierarchicalInstancedStaticMeshComponent);
  }
  else if (InstancedStaticMeshComponent)
  {
    return CreateSceneProxy(InstancedStaticMeshComponent);
  }
  else
  if (IsValid(StaticMeshComponent))
  {
    return CreateSceneProxy(StaticMeshComponent);
  }
  else if (IsValid(SkeletalMeshComponent))
  {
    bSkeletalMesh = true;
    return CreateSceneProxy(SkeletalMeshComponent);
  }

  UE_LOG(LogCarla, Error, TEXT("Unknown type of parent component: %s"), *ParentComponent->GetClass()->GetName());

  return NULL;
}

FPrimitiveSceneProxy * UTaggedComponent::CreateSceneProxy(UStaticMeshComponent * StaticMeshComponent)
{
  // 确保静态网格具有渲染数据
  UStaticMesh * StaticMesh = StaticMeshComponent->GetStaticMesh();

  if (StaticMesh == NULL)
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to create scene proxy for static mesh component (because static mesh is null): %s"), *StaticMeshComponent->GetReadableName());
    return NULL;
  }

  if (StaticMesh->RenderData == NULL)
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to create scene proxy for static mesh component (because render data is null): %s"), *StaticMeshComponent->GetReadableName());
    return NULL;
  }


  if (StaticMesh->RenderData->LODResources.Num() == 0)
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to create scene proxy for static mesh component (because num LOD resources is 0): %s"), *StaticMeshComponent->GetReadableName());
    return NULL;
  }

  return new FTaggedStaticMeshSceneProxy(StaticMeshComponent, true, TaggedMID);
}

FPrimitiveSceneProxy * UTaggedComponent::CreateSceneProxy(USkeletalMeshComponent * SkeletalMeshComponent)
{
  if (bShouldWaitFrame)
  {
    return nullptr;
  }
  ERHIFeatureLevel::Type SceneFeatureLevel = GetWorld()->FeatureLevel;
	FSkeletalMeshRenderData* SkelMeshRenderData = SkeletalMeshComponent->GetSkeletalMeshRenderData();

	// 仅在正确初始化时创建用于渲染的场景代理
	if (SkelMeshRenderData &&
		SkelMeshRenderData->LODRenderData.IsValidIndex(SkeletalMeshComponent->PredictedLODLevel) &&
		!SkeletalMeshComponent->bHideSkin &&
		SkeletalMeshComponent->MeshObject)
	{
		// 仅当支持正在使用的骨骼数量，或者我们没有骨架时（可破坏物就是这种情况），才创建场景代理
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
	//在使用网格之前，请验证网格是否有效。
	const bool bMeshIsValid =
		// 确保我们有实例
		(MeshComponent->PerInstanceRenderData.IsValid()) &&
		// 确保我们有一个实际的 StaticMesh
		MeshComponent->GetStaticMesh() &&
		MeshComponent->GetStaticMesh()->HasValidRenderData(false) &&
		// YOU 真的不能在具有多个元素的控制台上使用硬件实例化，因为它们共享相同的索引缓冲区。
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
	// 在使用网格之前，请验证网格是否有效。
	const bool bMeshIsValid =
		//确保我们有实例
		(MeshComponent->PerInstanceRenderData.IsValid()) &&
		// 确保我们有一个实际的 StaticMesh
		MeshComponent->GetStaticMesh() &&
		MeshComponent->GetStaticMesh()->HasValidRenderData(false) &&
		// YOU 真的不能在具有多个元素的控制台上使用硬件实例化，因为它们共享相同的索引缓冲区。
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

  // // TODO： 尝试删除此
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

  //将材质替换为标记的材质
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

  // 将材质替换为标记的材质
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

  //将材质替换为标记的材质
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

  // 将材质替换为标记的材质
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
