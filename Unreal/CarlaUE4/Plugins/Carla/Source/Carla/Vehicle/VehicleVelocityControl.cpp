// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Kismet/KismetMathLibrary.h"

#include "VehicleVelocityControl.h"

// 构造函数，初始化组件的属性
UVehicleVelocityControl::UVehicleVelocityControl()
{
  // 允许该组件在每帧更新时进行计算
  PrimaryComponentTick.bCanEverTick = true;
}
// 初始化时调用，设置初始状态
void UVehicleVelocityControl::BeginPlay()
{
  Super::BeginPlay(););  // 调用基类的 BeginPlay 方法
  
  // 初始时禁用组件的 Tick，手动控制启用
  SetComponentTickEnabled(false);
  
  // 设置 Tick 的执行时机为物理更新之前
  SetTickGroup(ETickingGroup::TG_PrePhysics);

  // 获取当前拥有该组件的车辆（Actor）
  OwnerVehicle = GetOwner();
   // 获取车辆的根组件，并强制转换为 UPrimitiveComponent 以便操作物理特性
  PrimitiveComponent = Cast<UPrimitiveComponent>(OwnerVehicle->GetRootComponent());
}

// 激活组件并设置目标速度为零向量
void UVehicleVelocityControl::Activate(bool bReset)
{
  Super::Activate(bReset);// 调用基类的 Activate 方法

  // 设置目标速度为零
  TargetVelocity = FVector();
  
  // 启用该组件的 Tick
  SetComponentTickEnabled(true);
}

// 激活组件并设置目标速度为指定的速度
void UVehicleVelocityControl::Activate(FVector Velocity, bool bReset)
{
  Super::Activate(bReset);

  TargetVelocity = Velocity;
  SetComponentTickEnabled(true);
}

void UVehicleVelocityControl::Deactivate()
{
  SetComponentTickEnabled(false);
  Super::Deactivate();
}

void UVehicleVelocityControl::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UVehicleVelocityControl::TickComponent);
  FTransform Transf = OwnerVehicle->GetActorTransform();
  const FVector LocVel = Transf.TransformVector(TargetVelocity);
  // 设置车辆根组件的线性速度（以世界坐标为单位）
  PrimitiveComponent->SetPhysicsLinearVelocity(LocVel, false, "None");
}
