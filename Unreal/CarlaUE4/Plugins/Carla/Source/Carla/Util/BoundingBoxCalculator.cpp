// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/Tagger.h"
#include "Carla/Util/BoundingBoxCalculator.h"

#include "Carla/Traffic/TrafficSignBase.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Game/Tagger.h"
#include "Carla/Traffic/TrafficLightBase.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/ObjectLabel.h>
#include <compiler/enable-ue4-macros.h>

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"

#include "Rendering/SkeletalMeshRenderData.h"
#include "Engine/SkeletalMeshSocket.h"

namespace crp = carla::rpc;

static FBoundingBox ApplyTransformToBB(
    FBoundingBox InBB,
    const FTransform& Transform)
{
  auto Scale = Transform.GetScale3D();
  InBB.Origin *= Scale;
  InBB.Rotation = Transform.GetRotation().Rotator();
  InBB.Origin = InBB.Rotation.RotateVector(InBB.Origin) + Transform.GetLocation();
  InBB.Extent *= Scale;
  return InBB;
}

FBoundingBox UBoundingBoxCalculator::GetActorBoundingBox(const AActor *Actor, uint8 InTagQueried)
{
  if (Actor != nullptr)// 如果Actor不为空
  {
    // Vehicle.
    auto Vehicle = Cast<ACarlaWheeledVehicle>(Actor);
    if (Vehicle != nullptr)
    {
      FVector Origin = Vehicle->GetVehicleBoundingBoxTransform().GetTranslation();
      FVector Extent = Vehicle->GetVehicleBoundingBoxExtent();
      return {Origin, Extent};
    }
    // Walker.
    auto Character = Cast<ACharacter>(Actor);
    if (Character != nullptr)
    {
      auto Capsule = Character->GetCapsuleComponent();
      if (Capsule != nullptr)
      {
        const auto Radius = Capsule->GetScaledCapsuleRadius();
        const auto HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
        // 角色的枢轴点在胶囊体的中心
        FVector Origin = {0.0f, 0.0f, 0.0f};
        FVector Extent = {Radius, Radius, HalfHeight};
        return {Origin, Extent};
      }
    }
    // Traffic sign.
    auto TrafficSign = Cast<ATrafficSignBase>(Actor);
    if (TrafficSign != nullptr)
    {
      // 首先返回合并的触发框（如果有的话）
      auto TriggerVolumes = TrafficSign->GetTriggerVolumes();
      if (TriggerVolumes.Num() > 0)
      {
        FBoundingBox Box = UBoundingBoxCalculator::CombineBoxes(TriggerVolumes);
        FTransform Transform = Actor->GetActorTransform();
        Box.Origin = Transform.InverseTransformPosition(Box.Origin);
        Box.Rotation = Transform.InverseTransformRotation(Box.Rotation.Quaternion()).Rotator();
        return Box;
      }
      //尝试返回原始的边界框
      auto TriggerVolume = TrafficSign->GetTriggerVolume();
      if (TriggerVolume != nullptr)
      {
          auto Transform = TriggerVolume->GetRelativeTransform();
          return
          {
              Transform.GetTranslation(),
              TriggerVolume->GetScaledBoxExtent(),
              Transform.GetRotation().Rotator()
          };
      }
      else
      {
        UE_LOG(LogCarla, Warning, TEXT("Traffic sign missing trigger volume: %s"), *Actor->GetName());
        return {};
      }
    }
    // 其他情况，默认使用边界框
    TArray<FBoundingBox> BBs = GetBBsOfActor(Actor);
    FBoundingBox BB = CombineBBs(BBs);
    // 转换为局部空间；GetBBsOfActor返回的是世界空间的边界框
    FTransform Transform = Actor->GetActorTransform();
    BB.Origin = Transform.InverseTransformPosition(BB.Origin);
    BB.Rotation = Transform.InverseTransformRotation(BB.Rotation.Quaternion()).Rotator();
    BB.Rotation = Transform.GetRotation().Rotator();
    return BB;

  }
  return {};
}

FBoundingBox UBoundingBoxCalculator::GetVehicleBoundingBox(
    const ACarlaWheeledVehicle* Vehicle,
    uint8 InTagQueried)
{
  check(Vehicle);

  crp::CityObjectLabel TagQueried = (crp::CityObjectLabel)InTagQueried;
  bool FilterByTagEnabled = (TagQueried != crp::CityObjectLabel::Any);

  UActorComponent *ActorComp = Vehicle->GetComponentByClass(USkeletalMeshComponent::StaticClass());
  USkeletalMeshComponent* ParentComp = Cast<USkeletalMeshComponent>(ActorComp);

  //根据标签进行过滤
  crp::CityObjectLabel Tag = ATagger::GetTagOfTaggedComponent(*ParentComp);
  if(FilterByTagEnabled && Tag != TagQueried) return {};

  USkeletalMesh* SkeletalMesh = ParentComp->SkeletalMesh;
  FBoundingBox BB = GetSkeletalMeshBoundingBox(SkeletalMesh);

  if(BB.Extent.IsZero())
  {
    UE_LOG(LogCarla, Error, TEXT("%s has no SKM assigned"), *Vehicle->GetName());
    return {};
  }

  //两轮车辆有多个SkeletalMesh
  TArray<USkeletalMeshComponent*> SkeletalMeshComps;
  Vehicle->GetComponents<USkeletalMeshComponent>(SkeletalMeshComps);

  if(SkeletalMeshComps.Num() > 1)
  {
    FVector VehicleLocation = Vehicle->GetActorLocation();
    FRotator VehicleRotation = Vehicle->GetActorRotation();
    float MaxHeadLocation = TNumericLimits<float>::Lowest();
    float MaxKneeLocation = TNumericLimits<float>::Lowest();
    bool SocketFound = false;
    for(USkeletalMeshComponent* Comp : SkeletalMeshComps)
    {
      if(Comp == ParentComp) continue; // ignore vehicle skeleton

      // 获取Socket的世界空间位置
      FVector HeadLocation = Comp->GetSocketLocation("crl_Head__C") - VehicleLocation;
      FVector KneeLocation = Comp->GetSocketLocation("crl_leg__L") - VehicleLocation;

      HeadLocation = VehicleRotation.UnrotateVector(HeadLocation);
      KneeLocation = VehicleRotation.UnrotateVector(KneeLocation);

      MaxHeadLocation = (HeadLocation.Z > MaxHeadLocation) ? HeadLocation.Z : MaxHeadLocation;
      MaxKneeLocation = (KneeLocation.Y > MaxKneeLocation) ? KneeLocation.Y : MaxKneeLocation;

      SocketFound = true;
    }

    if(SocketFound)
    {
      auto Min = BB.Origin - BB.Rotation.RotateVector(BB.Extent);
      auto Max = BB.Origin + BB.Rotation.RotateVector(BB.Extent);
      auto Tmp = Min;
      Min = Min.ComponentMin(Max);
      Max = Max.ComponentMin(Tmp);

      MaxHeadLocation += 20.0f; // 修改头部位置，假设头部骨骼位于底部

      Max.Y = (MaxKneeLocation > Max.Y) ? MaxKneeLocation : Max.Y;
      Min.Y = (-MaxKneeLocation < Min.Y) ? MaxKneeLocation : Min.Y;
      Max.Z = (MaxHeadLocation > Max.Z) ? MaxHeadLocation : Max.Z;

      BB.Extent = (Max - Min) * 0.5f;
      BB.Origin = Min + BB.Extent;
    }
  }

  // 计算组件到世界空间的变换
  auto& CompToWorldTransform = ParentComp->GetComponentTransform();
  BB = ApplyTransformToBB(BB, CompToWorldTransform);
  return BB;
}

FBoundingBox UBoundingBoxCalculator::GetCharacterBoundingBox(
    const ACharacter* Character,
    uint8 InTagQueried)
{
  check(Character);

  crp::CityObjectLabel TagQueried = (crp::CityObjectLabel)InTagQueried;
  bool FilterByTag = TagQueried == crp::CityObjectLabel::Any ||
                     TagQueried == crp::CityObjectLabel::Pedestrians;

  UCapsuleComponent* Capsule = Character->GetCapsuleComponent();


  if (Capsule && FilterByTag)
  {
    const float Radius = Capsule->GetScaledCapsuleRadius();
    const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
    FBoundingBox BoundingBox;
    // 角色的枢轴点位于中心
    BoundingBox.Origin = {0.0f, 0.0f, 0.0f};
    BoundingBox.Extent = {Radius, Radius, HalfHeight};
    // 计算组件到世界空间的变换
    auto CompToWorldTransform = Capsule->GetComponentTransform();
    BoundingBox = ApplyTransformToBB(BoundingBox, CompToWorldTransform);

    return BoundingBox;
  }

  return {};
}

void UBoundingBoxCalculator::GetTrafficLightBoundingBox(
    const ATrafficLightBase* TrafficLight,
    TArray<FBoundingBox>& OutBB,
    TArray<uint8>& OutTag,
    uint8 InTagQueried)
{
  check(TrafficLight);

  TArray<FBoundingBox> BBsOfTL;
  TArray<uint8> TagsOfTL;

  // 这种“魔法数字”的使用是基于假设信号灯的数量不会超过 100。
  // 这里通过平方距离来进行比较
  const float DistanceThreshold = 100.0f * 100.0f;

  //计算每个交通信号灯的边界框，因此需要合并成一个总的边界框
  uint8 TLTag = static_cast<uint8>(crp::CityObjectLabel::TrafficLight);

  CombineBBsOfActor(TrafficLight, BBsOfTL, TagsOfTL, DistanceThreshold, TLTag);
  check(BBsOfTL.Num() == TagsOfTL.Num());

  bool FilterByTagEnabled = (InTagQueried != static_cast<uint8>(crp::CityObjectLabel::Any));

  for(int i = 0; i < BBsOfTL.Num(); i++)
  {
    FBoundingBox& BB = BBsOfTL[i];
    uint8 Tag = TagsOfTL[i];

    if(FilterByTagEnabled && Tag != InTagQueried) continue;

    OutBB.Emplace(BB);
    OutTag.Emplace(Tag);
  }
}

// TODO: 更新为计算当前动画姿势的边界框
FBoundingBox UBoundingBoxCalculator::GetSkeletalMeshBoundingBox(const USkeletalMesh* SkeletalMesh)
{
  if(!SkeletalMesh)
  {
    UE_LOG(LogCarla, Error, TEXT("GetSkeletalMeshBoundingBox no SkeletalMesh"));
    return {};
  }

  // 获取Skeletal Mesh的LOD 0（细节级别0）渲染数据中的顶点位置数据
  FSkeletalMeshRenderData* SkeletalMeshRenderData = SkeletalMesh->GetResourceForRendering();
  FSkeletalMeshLODRenderData& LODRenderData = SkeletalMeshRenderData->LODRenderData[0];
  FStaticMeshVertexBuffers& StaticMeshVertexBuffers = LODRenderData.StaticVertexBuffers;
  FPositionVertexBuffer& FPositionVertexBuffer = StaticMeshVertexBuffers.PositionVertexBuffer;
  uint32 NumVertices = FPositionVertexBuffer.GetNumVertices();

  //查找Skeletal Mesh的边界框（完美顶点）
  FVector Max(TNumericLimits<float>::Lowest());
  FVector Min(TNumericLimits<float>::Max());

  for(uint32 i = 0; i < NumVertices; i++)
  {
    auto Pos = FPositionVertexBuffer.VertexPosition(i);
    Max = Max.ComponentMax(Pos);
    Min = Min.ComponentMin(Pos);
  }

  auto Extent = (Max - Min) * 0.5f;
  auto Origin = Min + Extent;

  return {Origin, Extent};
}

FBoundingBox UBoundingBoxCalculator::GetStaticMeshBoundingBox(const UStaticMesh* StaticMesh)
{
  if(!StaticMesh)
  {
    // UE_LOG(LogCarla, Error, TEXT("GetStaticMeshBoundingBox no StaticMesh"));
    return {};
  }

  FBox Box = StaticMesh->GetBoundingBox();
  return {Box.GetCenter(), Box.GetExtent()};

}

void UBoundingBoxCalculator::GetISMBoundingBox(
    UInstancedStaticMeshComponent* ISMComp,
    TArray<FBoundingBox>& OutBoundingBox)
{
  if(!ISMComp)
  {
    UE_LOG(LogCarla, Error, TEXT("GetISMBoundingBox no ISMComp"));
    return;
  }

  const UStaticMesh *Mesh = ISMComp->GetStaticMesh();
  const FBoundingBox SMBoundingBox = GetStaticMeshBoundingBox(Mesh);

  if(!Mesh)
  {
    UE_LOG(LogCarla, Error, TEXT("%s has no SM assigned to the ISM"), *ISMComp->GetOwner()->GetName());
    return;
  }

  const TArray<FInstancedStaticMeshInstanceData>& PerInstanceSMData = ISMComp->PerInstanceSMData;

  const FTransform ParentTransform = ISMComp->GetComponentTransform();

  for(auto& InstSMIData : PerInstanceSMData)
  {
    const FTransform Transform = FTransform(InstSMIData.Transform) * ParentTransform;
    FBoundingBox BoundingBox = ApplyTransformToBB(SMBoundingBox, Transform);

    OutBoundingBox.Add(BoundingBox);
  }

}

void UBoundingBoxCalculator::GetBBsOfStaticMeshComponents(
    const TArray<UStaticMeshComponent*>& StaticMeshComps,
    TArray<FBoundingBox>& OutBB,
    TArray<uint8>& OutTag,
    uint8 InTagQueried)
{
  crp::CityObjectLabel TagQueried = (crp::CityObjectLabel)InTagQueried;
  bool FilterByTagEnabled = (TagQueried != crp::CityObjectLabel::Any);

  for(UStaticMeshComponent* Comp : StaticMeshComps)
  {

    bool isCrosswalk = Comp->GetOwner()->GetName().Contains("crosswalk");

    // 避免与静态网格组件（SMComp）重复，并且排除不可见的网格
    if( (!Comp->IsVisible() && !isCrosswalk) || Cast<UInstancedStaticMeshComponent>(Comp))
    {
      continue;
    }

    //根据标签进行过滤
    crp::CityObjectLabel Tag = ATagger::GetTagOfTaggedComponent(*Comp);
    if(FilterByTagEnabled && Tag != TagQueried) continue;

    UStaticMesh* StaticMesh = Comp->GetStaticMesh();
    FBoundingBox BoundingBox = GetStaticMeshBoundingBox(StaticMesh);

    if(StaticMesh)
    {
      // 获取该组件的世界坐标变换矩阵
      const FTransform& CompToWorldTransform = Comp->GetComponentTransform();
      BoundingBox = ApplyTransformToBB(BoundingBox, CompToWorldTransform);
      OutBB.Emplace(BoundingBox);
      OutTag.Emplace(static_cast<uint8>(Tag));
    }
  }
}

void UBoundingBoxCalculator::GetBBsOfSkeletalMeshComponents(
    const TArray<USkeletalMeshComponent*>& SkeletalMeshComps,
    TArray<FBoundingBox>& OutBB,
    TArray<uint8>& OutTag,
    uint8 InTagQueried)
{
  crp::CityObjectLabel TagQueried = (crp::CityObjectLabel)InTagQueried;
  bool FilterByTagEnabled = (TagQueried != crp::CityObjectLabel::Any);

  for(USkeletalMeshComponent* Comp : SkeletalMeshComps)
  {
    // Filter by tag
    crp::CityObjectLabel Tag = ATagger::GetTagOfTaggedComponent(*Comp);

    if(!Comp->IsVisible() || (FilterByTagEnabled && Tag != TagQueried)) continue;

    USkeletalMesh* SkeletalMesh = Comp->SkeletalMesh;
    FBoundingBox BoundingBox = GetSkeletalMeshBoundingBox(SkeletalMesh);
    if(BoundingBox.Extent.IsZero())
    {
      UE_LOG(LogCarla, Error, TEXT("%s has no SKM assigned"), *Comp->GetOwner()->GetName());
    }
    else
    {
      // 将该组件的局部变换应用到世界坐标系的变换
      const FTransform& CompToWorldTransform = Comp->GetComponentTransform();
      BoundingBox = ApplyTransformToBB(BoundingBox, CompToWorldTransform);
      OutBB.Emplace(BoundingBox);
      OutTag.Emplace(static_cast<uint8>(Tag));
    }
  }
}

TArray<FBoundingBox> UBoundingBoxCalculator::GetBoundingBoxOfActors(
  const TArray<AActor*>& Actors,
  uint8 InTagQueried)
{
  TArray<FBoundingBox> Result;
  for(AActor* Actor : Actors)
  {
    TArray<FBoundingBox> BBs = GetBBsOfActor(Actor, InTagQueried);
    Result.Append(BBs.GetData(), BBs.Num());
  }

  return Result;
}

TArray<FBoundingBox> UBoundingBoxCalculator::GetBBsOfActor(
  const AActor* Actor,
  uint8 InTagQueried)
{
  TArray<FBoundingBox> Result;
  TArray<uint8> Tags;
  crp::CityObjectLabel TagQueried = (crp::CityObjectLabel)InTagQueried;
  bool FilterByTagEnabled = (TagQueried != crp::CityObjectLabel::Any);

  FString ClassName = Actor->GetClass()->GetName();

  // 避免处理BP_Procedural_Building类的Actor，以避免与其子Actor重复
  //当改进BP_Procedural_Building时，这个检查可能应该移除
  //注意：这里没有使用类型转换，因为基类是蓝图（BP），这种方式更简单，不需要通过获取UClass来进行类型转换。
  if( ClassName.Contains("Procedural_Bulding") ) return Result;

  // 车辆的蓝图使用了低多边形静态网格进行碰撞检测，我们应该避免处理它
  const ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(Actor);
  if (Vehicle)
  {
    FBoundingBox BoundingBox = GetVehicleBoundingBox(Vehicle, InTagQueried);
    if(!BoundingBox.Extent.IsZero())
    {
      Result.Add(BoundingBox);
    }
    return Result;;
  }

  // 行人，我们目前仅使用胶囊组件
  const ACharacter* Character = Cast<ACharacter>(Actor);
  if (Character)
  {
    FBoundingBox BoundingBox = GetCharacterBoundingBox(Character, InTagQueried);
    if(!BoundingBox.Extent.IsZero())
    {
      Result.Add(BoundingBox);
    }
    return Result;
  }

  // 对于交通信号灯，我们需要将所有信号灯的边界框合并为一个
  const ATrafficLightBase* TrafficLight = Cast<ATrafficLightBase>(Actor);
  if(TrafficLight)
  {
    GetTrafficLightBoundingBox(TrafficLight, Result, Tags, InTagQueried);
    return Result;
  }

  // 计算实例化静态网格组件（ISM）的边界框
  TArray<UInstancedStaticMeshComponent *> ISMComps;
  Actor->GetComponents<UInstancedStaticMeshComponent>(ISMComps);
  for(UInstancedStaticMeshComponent* Comp: ISMComps)
  {
    // 通过标签过滤
    crp::CityObjectLabel Tag = ATagger::GetTagOfTaggedComponent(*Comp);
    if(FilterByTagEnabled && Tag != TagQueried) continue;

    GetISMBoundingBox(Comp, Result);
  }

  // 计算静态网格组件（SM）的边界框
  TArray<UStaticMeshComponent*> StaticMeshComps;
  Tags.Reset();
  Actor->GetComponents<UStaticMeshComponent>(StaticMeshComps);
  GetBBsOfStaticMeshComponents(StaticMeshComps, Result, Tags, InTagQueried);

  //计算骨骼网格组件（SK_M）的边界框
  TArray<USkeletalMeshComponent*> SkeletalMeshComps;
  Tags.Reset();
  Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComps);
  GetBBsOfSkeletalMeshComponents(SkeletalMeshComps, Result, Tags, InTagQueried);

  return Result;
}

void UBoundingBoxCalculator::CombineBBsOfActor(
    const AActor* Actor,
    TArray<FBoundingBox>& OutBB,
    TArray<uint8>& OutTag,
    const float DistanceThreshold,
    uint8 TagToCombine)
{
  check(Actor);

  TArray<FBoundingBox> BBsOfTL;
  TArray<uint8> InternalOutTags;
  TArray<UStaticMeshComponent*> StaticMeshComps;
  Actor->GetComponents<UStaticMeshComponent>(StaticMeshComps);
  GetBBsOfStaticMeshComponents(StaticMeshComps, BBsOfTL, InternalOutTags);

  bool IgnoreDistanceFilter = (DistanceThreshold <= 0.0f);
  bool IgnoreTag = (TagToCombine == 0);

  TSet<int> IndicesDiscarded;
  for(int i = 0; i < BBsOfTL.Num(); i++)
  {
    //  检查该索引是否已用于合并之前的边界框
    if(IndicesDiscarded.Contains(i)) continue;

    TArray<FBoundingBox> BBsToCombine;
    FBoundingBox& BB1 = BBsOfTL[i];
    uint8 Tag1 = InternalOutTags[i];

    for(int j = i + 1; j < BBsOfTL.Num(); j++)
    {
      // 检查该索引是否已用于合并之前的边界框
      if(IndicesDiscarded.Contains(j)) continue;

      FBoundingBox& BB2 = BBsOfTL[j];
      uint8 Tag2 = InternalOutTags[j];

      float Distance = FVector::DistSquared(BB1.Origin, BB2.Origin);

      //如果两个边界框足够接近并且属于同一类型，则合并
      if( ((Distance <= DistanceThreshold) || IgnoreDistanceFilter) &&
          ((Tag1 == Tag2) || IgnoreTag) )
      {
        BBsToCombine.Emplace(BB2);
        IndicesDiscarded.Emplace(j);
      }
    }
    if(BBsToCombine.Num() > 0)
    {
      BBsToCombine.Emplace(BB1);
      IndicesDiscarded.Emplace(i);
      FBoundingBox MergedBB = CombineBBs(BBsToCombine);
      MergedBB.Rotation = BB1.Rotation;
      OutBB.Emplace(MergedBB);
      OutTag.Emplace(InternalOutTags[i]);
    }
  }

  // 添加不需要合并的网格的边界框
  for(int i = 0; i < BBsOfTL.Num(); i++)
  {
    //检查该索引是否已用于合并之前的边界框
    if(IndicesDiscarded.Contains(i)) continue;
    FBoundingBox& BB = BBsOfTL[i];
    OutBB.Emplace(BB);
    OutTag.Emplace(InternalOutTags[i]);
  }
}

template <typename Collection, typename ExtractAABB>
static FBoundingBox CombineBoundingBoxesGeneric(
    Collection&& collection,
    ExtractAABB&& extract_aabb)
{
    FVector Max(std::numeric_limits<float>::lowest());
    FVector Min(std::numeric_limits<float>::max());

    for (auto& e : collection)
    {
        auto BB = extract_aabb(e);
        auto LocalMax = BB.Origin + BB.Rotation.RotateVector(BB.Extent);
        auto LocalMin = BB.Origin - BB.Rotation.RotateVector(BB.Extent);
        auto Tmp = LocalMin;
        LocalMin = LocalMin.ComponentMin(LocalMax);
        LocalMax = LocalMax.ComponentMax(Tmp);
        Max = Max.ComponentMax(LocalMax);
        Min = Min.ComponentMin(LocalMin);
    }

    auto Extent = (Max - Min) * 0.5f;
    auto Origin = Min + Extent;

    return { Origin, Extent };
}

template <typename Collection>
static auto CombineBoundingBoxesGeneric(Collection&& collection)
{
    using T = std::remove_reference_t<decltype(collection[0])>;
    return CombineBoundingBoxesGeneric(std::forward<Collection>(collection), [](T& e) { return e; });
}

FBoundingBox UBoundingBoxCalculator::CombineBBs(const TArray<FBoundingBox>& BBsToCombine)
{
    return CombineBoundingBoxesGeneric(BBsToCombine);
}

FBoundingBox UBoundingBoxCalculator::CombineBoxes(const TArray<UBoxComponent *>& BBsToCombine)
{
    return CombineBoundingBoxesGeneric(BBsToCombine, [](UBoxComponent* e)
    {
        return FBoundingBox
        {
            e->GetComponentLocation(),
            e->GetScaledBoxExtent(),
            e->GetComponentRotation()
        };
    });
}

void UBoundingBoxCalculator::GetMeshCompsFromActorBoundingBox(
  const AActor* Actor,
  const FBoundingBox& InBB,
  TArray<UStaticMeshComponent*>& OutStaticMeshComps)
{
  check(Actor);


  const float DistanceThreshold = 100.0f * 100.0f;

  TArray<UStaticMeshComponent*> SMComps;
  Actor->GetComponents<UStaticMeshComponent>(SMComps);

  for(UStaticMeshComponent* Comp : SMComps)
  {
    if(!Comp->IsVisible()) continue;

    TArray<FBoundingBox> BBs;
    TArray<uint8> InternalOutTags;
    GetBBsOfStaticMeshComponents({Comp}, BBs, InternalOutTags);

    if(BBs.Num() == 0) continue;

    const FBoundingBox& BB = BBs[0];
    float Distance = FVector::DistSquared(InBB.Origin, BB.Origin);
    if(Distance <= DistanceThreshold)
    {
      OutStaticMeshComps.Emplace(Comp);
    }
  }
}
