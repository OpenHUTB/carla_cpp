// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
// 包含CARLA模拟框架中Walker控制器的头文件
// Walker控制器负责控制行人的移动和动画
#include "Carla/Walker/WalkerController.h"
// 包含CARLA模拟框架中Walker动画的头文件
// Walker动画涉及到行人的行走、跑步、跳跃等动作的动画处理
#include "Carla/Walker/WalkerAnim.h"
// 包含Unreal Engine中用于可摆放网格组件的头文件
// PoseableMeshComponent允许开发者在运行时动态地调整网格（3D模型）的位置、旋转和缩放
#include "Components/PoseableMeshComponent.h"
// 包含Unreal Engine中用于基本几何体组件的头文件
// PrimitiveComponent是许多基本形状（如立方体、球体、平面等）的基类
#include "Components/PrimitiveComponent.h"
// 包含Unreal Engine中用于映射（键值对）的头文件
// Map是一个模板类，用于存储键值对，类似于标准C++中的std::map
#include "Containers/Map.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
// 包含Unreal Engine中用于Pawn的头文件
// Pawn是一个更通用的基类，用于表示游戏世界中的可控制实体
// Character是Pawn的一个子类，专门用于表示具有移动能力的角色
#include "GameFramework/Pawn.h"
// AWalkerController类的构造函数
// FObjectInitializer参数用于初始化对象属性
AWalkerController::AWalkerController(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{ // 设置PrimaryActorTick.bCanEverTick为true，表示这个Actor（行人控制器）可以在每一帧都被更新
  PrimaryActorTick.bCanEverTick = true;
}
// 当AWalkerController“占有”一个APawn时，会调用此函数
// InPawn参数指向被占有的Pawn
void AWalkerController::OnPossess(APawn *InPawn)
{// 调用父类的OnPossess函数
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
// 设置角色的最大行走速度
  MovementComponent->MaxWalkSpeed = GetMaximumWalkSpeed();
  // 设置角色的跳跃垂直速度
  MovementComponent->JumpZVelocity = 500.0f;// 一个硬编码的跳跃速度值
  Character->JumpMaxCount = 2;
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
// AWalkerController类的Tick成员函数，用于处理每一帧的更新逻辑
// DeltaSeconds参数表示从上一帧到当前帧的时间间隔
void AWalkerController::Tick(float DeltaSeconds)
{// 使用TRACE_CPUPROFILER_EVENT_SCOPE宏来标记这个函数的执行范围
  // 这通常用于性能分析，以便在性能分析工具中查看此函数的执行时间和频率
  TRACE_CPUPROFILER_EVENT_SCOPE(AWalkerController::Tick);
  // 调用父类的Tick函数，以确保父类的更新逻辑得到执行
  // Super通常是一个宏，用于引用当前类的父类
  Super::Tick(DeltaSeconds);
// 获取当前控制的角色实例
  auto *Character = GetCharacter();
  // 如果没有获取到有效的角色实例，则直接返回，不进行后续操作
  if (!Character) return;
// 根据控制输入（方向和速度）来添加角色的移动输入
  // Control.Direction表示移动方向，Control.Speed表示请求的移动速度
  // GetMaximumWalkSpeed()函数返回角色能行走的最大速度
  // 通过将Control.Speed除以最大行走速度，可以得到一个0到1之间的比例，用于控制角色的实际移动速度
  Character->AddMovementInput(Control.Direction,
        Control.Speed / GetMaximumWalkSpeed());
  // 如果控制输入中包含了跳跃指令
  // 则调用角色的Jump函数来执行跳跃动作
  if (Control.Jump)
  {
    Character->Jump();
  }
}
