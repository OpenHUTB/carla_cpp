// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Walker/WalkerController.h"
#include "Carla/Walker/WalkerAnim.h"

#include "Components/PoseableMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Containers/Map.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

AWalkerController::AWalkerController(const FObjectInitializer &ObjectInitializer)//这个构造函数接收一个FObjectInitializer类型的参数，这是Unreal Engine中用于初始化对象的常用方式
  : Super(ObjectInitializer)//调用父类的构造函数，确保父类被正确初始化
{
  PrimaryActorTick.bCanEverTick = true;//设置了这个控制器所附着的Actor（游戏对象）可以被引擎的tick系统调用。Tick系统是Unreal Engine中用于定期更新游戏状态（如位置、动画等）的机制。
}

void AWalkerController::OnPossess(APawn *InPawn)
{
  Super::OnPossess(InPawn);

  auto *Character = Cast<ACharacter>(InPawn);
  if (Character == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("Walker is not a character!"));
    return;
  }

  auto *MovementComponent = Character->GetCharacterMovement();
  if (MovementComponent == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("Walker missing character movement component!"));
    return;
  }

  MovementComponent->MaxWalkSpeed = GetMaximumWalkSpeed();//设置角色的最大行走速度为GetMaximumWalkSpeed()方法的返回值。这个方法应该在这个类或其父类中定义，用于获取行走者的最大行走速度。
  MovementComponent->JumpZVelocity = 500.0f;//设置角色跳跃时的垂直速度
  Character->JumpMaxCount = 2;//设置角色可以连续跳跃的最大次数为2次
}

void AWalkerController::ApplyWalkerControl(const FWalkerControl &InControl)
{
  Control = InControl;
}

void AWalkerController::GetBonesTransform(FWalkerBoneControlOut &WalkerBones)
{
  auto *Character = GetCharacter();
  if (!Character) return;

  TArray<USkeletalMeshComponent *> SkeletalMeshes;
  Character->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
  USkeletalMeshComponent *SkeletalMesh = SkeletalMeshes.IsValidIndex(0) ? SkeletalMeshes[0] : nullptr;
  if (!SkeletalMesh) return;

  // 获取行人的动画类
  auto *AnimInst = SkeletalMesh->GetAnimInstance();
  if (!AnimInst) return;
  UWalkerAnim *WalkerAnim = Cast<UWalkerAnim>(AnimInst);
  if (!WalkerAnim) return;
  
  // 获取当前的姿态
  FPoseSnapshot TempSnapshot;
  SkeletalMesh->SnapshotPose(TempSnapshot);
  
  // 拷贝姿态
  // WalkerAnim->Snap = TempSnapshot;

  for (int i=0; i<TempSnapshot.BoneNames.Num(); ++i)
  {
    FWalkerBoneControlOutData Transforms;
    Transforms.World = SkeletalMesh->GetSocketTransform(TempSnapshot.BoneNames[i], ERelativeTransformSpace::RTS_World);
    Transforms.Component = SkeletalMesh->GetSocketTransform(TempSnapshot.BoneNames[i], ERelativeTransformSpace::RTS_Actor);
    Transforms.Relative = SkeletalMesh->GetSocketTransform(TempSnapshot.BoneNames[i], ERelativeTransformSpace::RTS_ParentBoneSpace);
    // Transforms.Relative = TempSnapshot.LocalTransforms[i];

    WalkerBones.BoneTransforms.Add(TempSnapshot.BoneNames[i].ToString(), Transforms);
  }
}

void AWalkerController::SetBonesTransform(const FWalkerBoneControlIn &WalkerBones)
{
  auto *Character = GetCharacter();
  if (!Character) return;

  TArray<USkeletalMeshComponent *> SkeletalMeshes;
  Character->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
  USkeletalMeshComponent *SkeletalMesh = SkeletalMeshes.IsValidIndex(0) ? SkeletalMeshes[0] : nullptr;
  if (!SkeletalMesh) return;

  // 获取行人动画的类
  auto *AnimInst = SkeletalMesh->GetAnimInstance();
  if (!AnimInst) return;
  UWalkerAnim *WalkerAnim = Cast<UWalkerAnim>(AnimInst);
  if (!WalkerAnim) return;
  
  // 如果姿势为空，则获得第一个版本
  if (WalkerAnim->Snap.BoneNames.Num() == 0)
  {
    // 获得当前的姿势
    SkeletalMesh->SnapshotPose(WalkerAnim->Snap);
  }

  TMap<FName, FTransform> BonesMap;
  for (const TPair<FString, FTransform> &pair : WalkerBones.BoneTransforms)
  {
    FName BoneName = FName(*pair.Key);
    BonesMap.Add(BoneName, pair.Value);
  }

  // 分配常用骨架
  for (int i=0; i<WalkerAnim->Snap.BoneNames.Num(); ++i)
  {
    FTransform *Trans = BonesMap.Find(WalkerAnim->Snap.BoneNames[i]);
    if (Trans)
    {
      WalkerAnim->Snap.LocalTransforms[i] = *Trans;
    }
  }
}

void AWalkerController::BlendPose(float Blend)
{
  auto *Character = GetCharacter();
  if (!Character) return;

  TArray<USkeletalMeshComponent *> SkeletalMeshes;
  Character->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
  USkeletalMeshComponent *SkeletalMesh = SkeletalMeshes.IsValidIndex(0) ? SkeletalMeshes[0] : nullptr;
  if (!SkeletalMesh) return;

  // 获得行人动画类
  auto *AnimInst = SkeletalMesh->GetAnimInstance();
  if (!AnimInst) return;
  UWalkerAnim *WalkerAnim = Cast<UWalkerAnim>(AnimInst);
  if (!WalkerAnim) return;
  
  // 设置当前姿势
  WalkerAnim->Blend = Blend;
}

void AWalkerController::GetPoseFromAnimation()
{
  auto *Character = GetCharacter();
  if (!Character) return;

  TArray<USkeletalMeshComponent *> SkeletalMeshes;
  Character->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
  USkeletalMeshComponent *SkeletalMesh = SkeletalMeshes.IsValidIndex(0) ? SkeletalMeshes[0] : nullptr;
  if (!SkeletalMesh) return;

  // 获得行人动画类
  auto *AnimInst = SkeletalMesh->GetAnimInstance();
  if (!AnimInst) return;
  UWalkerAnim *WalkerAnim = Cast<UWalkerAnim>(AnimInst);
  if (!WalkerAnim) return;
  
  // 获得当前姿势
  SkeletalMesh->SnapshotPose(WalkerAnim->Snap);
}

void AWalkerController::Tick(float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AWalkerController::Tick);
  Super::Tick(DeltaSeconds);

  auto *Character = GetCharacter();
  if (!Character) return;

  Character->AddMovementInput(Control.Direction,
        Control.Speed / GetMaximumWalkSpeed());
  if (Control.Jump)
  {
    Character->Jump();
  }
}
