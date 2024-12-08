// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "CoreMinimal.h"

#include "VehicleVelocityControl.generated.h"

/// ����actor�ٶȺ㶨�����
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CARLA_API UVehicleVelocityControl : public UActorComponent
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name ���캯������������
  // ===========================================================================
  /// @{
public:
  UVehicleVelocityControl();

  /// @}
  // ===========================================================================
  /// @name ��ȡ����
  // ===========================================================================
  /// @{
public:

  // ��ʼ����ʱ����
  void BeginPlay() override;

  // ÿ֡���õ�Tick����
  virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

  // �������������Ŀ���ٶ�
  virtual void Activate(bool bReset=false) override;

  // �������������ָ����Ŀ���ٶ�
  virtual void Activate(FVector Velocity, bool bReset=false);

  // ͣ�����
  virtual void Deactivate() override;

private:
  // Ŀ���ٶ�
  UPROPERTY(Category = "Vehicle Velocity Control", VisibleAnywhere)
  FVector TargetVelocity;

  // ԭʼ���ָ��
  UPrimitiveComponent* PrimitiveComponent;

  // ӵ�и�����ĳ���Actor
  AActor* OwnerVehicle;

};