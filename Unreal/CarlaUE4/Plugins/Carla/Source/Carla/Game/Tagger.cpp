// 城市场景中对象的标签类
#include "Carla.h"
#include "Tagger.h"
#include "TaggedComponent.h"
#include "Vehicle/CarlaWheeledVehicle.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "PhysicsEngine/PhysicsAsset.h"
//为carla::rpc命名空间创建别名crp
namespace crp = carla::rpc;
//枚举类型转换模板函数
template <typename T>
static auto CastEnum(T label)
{
  return static_cast<typename std::underlying_type<T>::type>(label);
}
//根据文件夹名称获取标签函数
crp::CityObjectLabel ATagger::GetLabelByFolderName(const FString &String) {
  //// 一系列条件判断，根据输入的文件夹名称返回对应的城市场景对象标签枚举值
  if      (String == "Building")     return crp::CityObjectLabel::Buildings;
  else if (String == "Fence")        return crp::CityObjectLabel::Fences;
  else if (String == "Pedestrian")   return crp::CityObjectLabel::Pedestrians;
  else if (String == "Pole")         return crp::CityObjectLabel::Poles;
  else if (String == "Other")        return crp::CityObjectLabel::Other;
  else if (String == "Road")         return crp::CityObjectLabel::Roads;
  else if (String == "RoadLine")     return crp::CityObjectLabel::RoadLines;
  else if (String == "SideWalk")     return crp::CityObjectLabel::Sidewalks;
  else if (String == "TrafficSign")  return crp::CityObjectLabel::TrafficSigns;
  else if (String == "Vegetation")   return crp::CityObjectLabel::Vegetation;
  else if (String == "Car")          return crp::CityObjectLabel::Car;
  else if (String == "Wall")         return crp::CityObjectLabel::Walls;
  else if (String == "Sky")          return crp::CityObjectLabel::Sky;
  else if (String == "Ground")       return crp::CityObjectLabel::Ground;
  else if (String == "Bridge")       return crp::CityObjectLabel::Bridge;
  else if (String == "RailTrack")    return crp::CityObjectLabel::RailTrack;
  else if (String == "GuardRail")    return crp::CityObjectLabel::GuardRail;
  else if (String == "TrafficLight") return crp::CityObjectLabel::TrafficLight;
  else if (String == "Static")       return crp::CityObjectLabel::Static;
  else if (String == "Dynamic")      return crp::CityObjectLabel::Dynamic;
  else if (String == "Water")        return crp::CityObjectLabel::Water;
  else if (String == "Terrain")      return crp::CityObjectLabel::Terrain;
  else if (String == "Truck")        return crp::CityObjectLabel::Truck;
  else if (String == "Motorcycle")   return crp::CityObjectLabel::Motorcycle;
  else if (String == "Bicycle")      return crp::CityObjectLabel::Bicycle;
  else if (String == "Bus")          return crp::CityObjectLabel::Bus;
  else if (String == "Rider")        return crp::CityObjectLabel::Rider;
  else if (String == "Train")        return crp::CityObjectLabel::Train;
  else                               return crp::CityObjectLabel::None;
}
//设置模板值和渲染深度函数
void ATagger::SetStencilValue(
    UPrimitiveComponent &Component,
    const crp::CityObjectLabel &Label,
    const bool bSetRenderCustomDepth) {
  Component.SetCustomDepthStencilValue(CastEnum(Label));
  Component.SetRenderCustomDepth(
      bSetRenderCustomDepth &&
      (Label != crp::CityObjectLabel::None));
}
//判断是否为物体
bool ATagger::IsThing(const crp::CityObjectLabel &Label)
{
  return (Label == crp::CityObjectLabel::Pedestrians ||
          Label == crp::CityObjectLabel::TrafficSigns ||
          Label == crp::CityObjectLabel::Car ||
          Label == crp::CityObjectLabel::Train ||
          Label == crp::CityObjectLabel::Bicycle ||
          Label == crp::CityObjectLabel::Motorcycle ||
          Label == crp::CityObjectLabel::Bus ||
          Label == crp::CityObjectLabel::Rider ||
          Label == crp::CityObjectLabel::Truck ||
          Label == crp::CityObjectLabel::TrafficLight);
}

/**
 * @brief 获得实例分割中参与者所标注的颜色
 * @param Actor 所需要判断显示颜色的参与者
 * @param Label 
 * @return 
*/
//获取参与者标签颜色函数
FLinearColor ATagger::GetActorLabelColor(const AActor &Actor, const crp::CityObjectLabel &Label)
{
  uint32 id = Actor.GetUniqueID();
  // TODO: Warn if id > 0xffff.

  // 像语义分割一样编码标签和 id
  // TODO: 从红色R通道和可能的A通道借用比特？
  FLinearColor Color(0.0f, 0.0f, 0.0f, 1.0f);
  Color.R = CastEnum(Label) / 255.0f;
  Color.G = ((id & 0x00ff) >> 0) / 255.0f;
  Color.B = ((id & 0xff00) >> 8) / 255.0f;

  return Color;
}


// =============================================================================
// -- ATagger类的静态函数 -------------------------------------------------
// =============================================================================

void ATagger::TagActor(const AActor &Actor, bool bTagForSemanticSegmentation)
{
#ifdef CARLA_TAGGER_EXTRA_LOG
  UE_LOG(LogCarla, Log, TEXT("Actor: %s"), *Actor.GetName());
#endif // CARLA_TAGGER_EXTRA_LOG

  // 遍历静态网格.
  //标记单个参与者函数
  TArray<UStaticMeshComponent *> StaticMeshComponents;
  Actor.GetComponents<UStaticMeshComponent>(StaticMeshComponents);
  for (UStaticMeshComponent *Component : StaticMeshComponents) {
    auto Label = GetLabelByPath(Component->GetStaticMesh());
    if (Label == crp::CityObjectLabel::Pedestrians &&
        Cast<ACarlaWheeledVehicle>(&Actor))
    {
      Label = crp::CityObjectLabel::Rider;
    }
    // 设置组件的模板值和渲染深度
    SetStencilValue(*Component, Label, bTagForSemanticSegmentation);
#ifdef CARLA_TAGGER_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("  + StaticMeshComponent: %s"), *Component->GetName());
    UE_LOG(LogCarla, Log, TEXT("    - Label: \"%s\""), *GetTagAsString(Label));
#endif // CARLA_TAGGER_EXTRA_LOG

    if(!Component->IsVisible() || !Component->GetStaticMesh())
    {
      continue;
       // 查找或创建附加到该组件的带标签的组件，并设置其颜色
    }

    // 查找附加到此组件上的带标签的组件
    UTaggedComponent *TaggedComponent = NULL;
     // 遍历骨骼网格组件，逻辑与静态网格组件类似
    TArray<USceneComponent *> AttachedComponents = Component->GetAttachChildren();
    for (USceneComponent *SceneComponent : AttachedComponents) {
      UTaggedComponent *TaggedSceneComponent = Cast<UTaggedComponent>(SceneComponent);
      if (IsValid(TaggedSceneComponent)) {
          TaggedComponent = TaggedSceneComponent;
#ifdef CARLA_TAGGER_EXTRA_LOG
          UE_LOG(LogCarla, Log, TEXT("    - Found Tag"));
#endif // CARLA_TAGGER_EXTRA_LOG
          break;
      }
    }

    // 如果没有找到，则创建一个新的带标签的组件，并将其附加到此组件上
    if (!TaggedComponent) {
      TaggedComponent = NewObject<UTaggedComponent>(Component);
      TaggedComponent->SetupAttachment(Component);
      TaggedComponent->RegisterComponent();
#ifdef CARLA_TAGGER_EXTRA_LOG
      UE_LOG(LogCarla, Log, TEXT("    - Added Tag"));
#endif // CARLA_TAGGER_EXTRA_LOG
    }

    // 设置带标签的组件颜色
    FLinearColor Color = GetActorLabelColor(Actor, Label);
#ifdef CARLA_TAGGER_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("    - Color: %s"), *Color.ToString());
#endif // CARLA_TAGGER_EXTRA_LOG

    TaggedComponent->SetColor(Color);
    TaggedComponent->MarkRenderStateDirty();
  }

  // 遍历骨骼网格
  TArray<USkeletalMeshComponent *> SkeletalMeshComponents;
  Actor.GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
  for (USkeletalMeshComponent *Component : SkeletalMeshComponents) {
    auto Label = GetLabelByPath(Component->GetPhysicsAsset());
    if (Label == crp::CityObjectLabel::Pedestrians &&
        Cast<ACarlaWheeledVehicle>(&Actor))
    {
      Label = crp::CityObjectLabel::Rider;
    }
    SetStencilValue(*Component, Label, bTagForSemanticSegmentation);
#ifdef CARLA_TAGGER_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("  + SkeletalMeshComponent: %s"), *Component->GetName());
    UE_LOG(LogCarla, Log, TEXT("    - Label: \"%s\""), *GetTagAsString(Label));
#endif // CARLA_TAGGER_EXTRA_LOG

    if(!Component->IsVisible() || !Component->GetSkeletalMeshRenderData())
    {
      continue;
    }

    // 查找附加到此组件上的带标签的组件
    UTaggedComponent *TaggedComponent = NULL;
    TArray<USceneComponent *> AttachedComponents = Component->GetAttachChildren();
    for (USceneComponent *SceneComponent : AttachedComponents) {
      UTaggedComponent *TaggedSceneComponent = Cast<UTaggedComponent>(SceneComponent);
      if (IsValid(TaggedSceneComponent)) {
          TaggedComponent = TaggedSceneComponent;
#ifdef CARLA_TAGGER_EXTRA_LOG
          UE_LOG(LogCarla, Log, TEXT("    - Found Tag"));
#endif // CARLA_TAGGER_EXTRA_LOG
          break;
      }
    }

    // 如果没有找到，则创建一个新的带标签的组件，并将其附加到此组件
    if (!TaggedComponent) {
      TaggedComponent = NewObject<UTaggedComponent>(Component);
      TaggedComponent->SetupAttachment(Component);
      TaggedComponent->RegisterComponent();
#ifdef CARLA_TAGGER_EXTRA_LOG
      UE_LOG(LogCarla, Log, TEXT("    - Added Tag"));
#endif // CARLA_TAGGER_EXTRA_LOG
    }

    // 设置带标签组件的颜色
    FLinearColor Color = GetActorLabelColor(Actor, Label);
#ifdef CARLA_TAGGER_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("    - Color: %s"), *Color.ToString());
#endif // CARLA_TAGGER_EXTRA_LOG

    TaggedComponent->SetColor(Color);
    TaggedComponent->MarkRenderStateDirty();
    TaggedComponent->SetComponentTickEnabled(true);

  }
}
//标记单个参与者函数
void ATagger::TagActorsInLevel(UWorld &World, bool bTagForSemanticSegmentation)
{
  for (TActorIterator<AActor> it(&World); it; ++it) {
    TagActor(**it, bTagForSemanticSegmentation);
  }
}
//标记世界中所有参与者函数
void ATagger::TagActorsInLevel(ULevel &Level, bool bTagForSemanticSegmentation)
{
  for (AActor * Actor : Level.Actors) {
    TagActor(*Actor, bTagForSemanticSegmentation);
  }
}
//获取标记参与者的标签函数
void ATagger::GetTagsOfTaggedActor(const AActor &Actor, TSet<crp::CityObjectLabel> &Tags)
{
  TArray<UPrimitiveComponent *> Components;
  Actor.GetComponents<UPrimitiveComponent>(Components);
  for (auto *Component : Components) {
    if (Component != nullptr) {
      const auto Tag = GetTagOfTaggedComponent(*Component);
      if (Tag != crp::CityObjectLabel::None) {
        Tags.Add(Tag);
      }
    }
  }
}
//将标签转换为字符串函数
FString ATagger::GetTagAsString(const crp::CityObjectLabel Label)
{
  switch (Label) {
#define CARLA_GET_LABEL_STR(lbl) case crp::CityObjectLabel:: lbl : return TEXT(#lbl);
    default:
    CARLA_GET_LABEL_STR(None)
    CARLA_GET_LABEL_STR(Buildings)
    CARLA_GET_LABEL_STR(Fences)
    CARLA_GET_LABEL_STR(Other)
    CARLA_GET_LABEL_STR(Pedestrians)
    CARLA_GET_LABEL_STR(Poles)
    CARLA_GET_LABEL_STR(RoadLines)
    CARLA_GET_LABEL_STR(Roads)
    CARLA_GET_LABEL_STR(Sidewalks)
    CARLA_GET_LABEL_STR(TrafficSigns)
    CARLA_GET_LABEL_STR(Vegetation)
    CARLA_GET_LABEL_STR(Car)
    CARLA_GET_LABEL_STR(Walls)
    CARLA_GET_LABEL_STR(Sky)
    CARLA_GET_LABEL_STR(Ground)
    CARLA_GET_LABEL_STR(Bridge)
    CARLA_GET_LABEL_STR(RailTrack)
    CARLA_GET_LABEL_STR(GuardRail)
    CARLA_GET_LABEL_STR(TrafficLight)
    CARLA_GET_LABEL_STR(Static)
    CARLA_GET_LABEL_STR(Dynamic)
    CARLA_GET_LABEL_STR(Water)
    CARLA_GET_LABEL_STR(Terrain)
    CARLA_GET_LABEL_STR(Truck)
    CARLA_GET_LABEL_STR(Motorcycle)
    CARLA_GET_LABEL_STR(Bicycle)
    CARLA_GET_LABEL_STR(Bus)
    CARLA_GET_LABEL_STR(Train)
    CARLA_GET_LABEL_STR(Rider)

#undef CARLA_GET_LABEL_STR
  }
}

// =============================================================================
// -- non-static ATagger functions ---------------------------------------------
// =============================================================================
//类的默认构造函数
ATagger::ATagger()
{
  PrimaryActorTick.bCanEverTick = false;
}
//编辑后属性更改处理函数
#if WITH_EDITOR
void ATagger::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);
  if (PropertyChangedEvent.Property) {
    if (bTriggerTagObjects && (GetWorld() != nullptr)) {
      TagActorsInLevel(*GetWorld(), bTagForSemanticSegmentation);
    }
  }
  bTriggerTagObjects = false;
}
#endif // 与编辑器集成
