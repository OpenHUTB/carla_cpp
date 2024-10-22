// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Util/ActorAttacher.h"

#include "GameFramework/SpringArmComponent.h"

static void UActorAttacher_AttachActorsWithSpringArm(
    AActor *Child,
    AActor *Parent)
{
  auto SpringArm = NewObject<USpringArmComponent>(Parent);

  // Child location negated to compensate for the spring arm rotation (rotated
  // from the "other end" of the arm).
  const auto ChildLocation = -Child->GetActorLocation();
  Child->SetActorLocation(FVector::ZeroVector);

  // Adding Z-offset to avoid colliding against the ground on bumpy terrain.
  SpringArm->TargetOffset = FVector(0.0f, 0.0f, 30.0f);
  SpringArm->bDoCollisionTest = true;

  FRotator LookAt = FRotationMatrix::MakeFromX(ChildLocation).Rotator();
  SpringArm->SetRelativeRotation(LookAt);
  SpringArm->SetupAttachment(Parent->GetRootComponent());

  SpringArm->TargetArmLength = ChildLocation.Size();
  SpringArm->bEnableCameraRotationLag = true;
  SpringArm->CameraRotationLagSpeed = 8.0f;

  SpringArm->bInheritPitch = false;
  SpringArm->bInheritRoll = false;
  SpringArm->bInheritYaw = true;

  SpringArm->AttachToComponent(
      Parent->GetRootComponent(),
      FAttachmentTransformRules::KeepRelativeTransform);
  SpringArm->RegisterComponent();

  auto ChildComp = NewObject<UChildActorComponent>(Parent);
  ChildComp->SetupAttachment(
      SpringArm,
      USpringArmComponent::SocketName);
  Child->AttachToComponent(
      ChildComp,
      FAttachmentTransformRules::KeepRelativeTransform);
  ChildComp->RegisterComponent();
}

static void UActorAttacher_AttachActorsWithSpringArmGhost(
    AActor *Child,
    AActor *Parent)
{
  auto SpringArm = NewObject<USpringArmComponent>(Parent);

  // Child location negated to compensate for the spring arm rotation (rotated
  // from the "other end" of the arm).
  const auto ChildLocation = -Child->GetActorLocation();
  Child->SetActorLocation(FVector::ZeroVector);

  // Adding Z-offset to avoid colliding against the ground on bumpy terrain.
  SpringArm->TargetOffset = FVector(0.0f, 0.0f, 0.0f);
  SpringArm->bDoCollisionTest = false;

  FRotator LookAt = FRotationMatrix::MakeFromX(ChildLocation).Rotator();
  SpringArm->SetRelativeRotation(LookAt);
  SpringArm->SetupAttachment(Parent->GetRootComponent());

  SpringArm->TargetArmLength = ChildLocation.Size();
  SpringArm->bEnableCameraRotationLag = true;
  SpringArm->CameraRotationLagSpeed = 8.0f;

  SpringArm->bInheritPitch = false;
  SpringArm->bInheritRoll = false;
  SpringArm->bInheritYaw = true;

  SpringArm->AttachToComponent(
      Parent->GetRootComponent(),
      FAttachmentTransformRules::KeepRelativeTransform);
  SpringArm->RegisterComponent();

  auto ChildComp = NewObject<UChildActorComponent>(Parent);
  ChildComp->SetupAttachment(
      SpringArm,
      USpringArmComponent::SocketName);
  Child->AttachToComponent(
      ChildComp,
      FAttachmentTransformRules::KeepRelativeTransform);
  ChildComp->RegisterComponent();
}

void UActorAttacher::AttachActors(
    AActor *Child,
    AActor *Parent,
    const EAttachmentType AttachmentType,
    const FString& SocketName)
{
 // 根据AttachmentType的值，选择不同的方式将Child对象附加到Parent对象上
  switch (AttachmentType)
  {
      // FName(*SocketName)是一个名称，用于指定Parent上的一个附着点
  case EAttachmentType::Rigid:
      Child->AttachToActor(Parent, FAttachmentTransformRules::KeepRelativeTransform, FName(*SocketName));
      break;
      // 这个函数可能是用来以弹簧臂的方式将Child附加到Parent上，弹簧臂通常用于相机，以提供一个动态且平滑的视角
  case EAttachmentType::SpringArm:
      UActorAttacher_AttachActorsWithSpringArm(Child, Parent);
      break;
      // 这个函数的行为可能与SpringArm类似，但可能有一些特殊的用途或行为，例如在不渲染弹簧臂本身的情况下提供视角
  case EAttachmentType::SpringArmGhost:
      UActorAttacher_AttachActorsWithSpringArmGhost(Child, Parent);
      break;
      // Fatal表示这是一个严重的错误，可能会导致程序崩溃
  default:
      UE_LOG(LogCarla, Fatal, TEXT("Invalid attachment type"));
  }

  Child->SetOwner(Parent);
}
