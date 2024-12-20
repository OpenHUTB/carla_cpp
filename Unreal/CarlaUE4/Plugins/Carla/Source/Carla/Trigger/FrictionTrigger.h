// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Carla/Game/CarlaEpisode.h"

#include "FrictionTrigger.generated.h"
// 定义一个 Unreal Engine 的类
UCLASS()
class CARLA_API AFrictionTrigger : public AActor
{
  GENERATED_BODY()

private:
  // 初始化函数，具体实现未给出
  void Init();
  // 更新车轮摩擦力的函数，接收另一个 Actor 和新的摩擦力数组作为参数
  void UpdateWheelsFriction(AActor *OtherActor, TArray<float>& NewFriction);

public:
  // 构造函数，接收对象初始化器作为参数
  AFrictionTrigger(const FObjectInitializer &ObjectInitializer);
  // 当触发体开始重叠时调用的函数
  UFUNCTION()
  void OnTriggerBeginOverlap(
      UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);
   // 当触发体结束重叠时调用的函数
  UFUNCTION()
  void OnTriggerEndOverlap(
      UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex);
   // 设置 Episode 的函数
  void SetEpisode(const UCarlaEpisode &InEpisode)
  {
    Episode = &InEpisode;
  }
  // 设置触发体的盒状范围的函数
  void SetBoxExtent(const FVector &Extent)
  {
    TriggerVolume->SetBoxExtent(Extent);
  }
  // 设置摩擦力的函数
  void SetFriction(float NewFriction)
  {
    Friction = NewFriction;
  }

protected:
  // 在开始游戏时调用的函数
  virtual void BeginPlay() override;
   // 在结束游戏时调用的函数
  virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
  // 每帧调用的函数
  virtual void Tick(float DeltaTime) override;

  // 存储旧的摩擦力值的数组
  TArray<float> OldFrictionValues;

public:
   // 可在编辑器中编辑的属性，存储摩擦力值
  UPROPERTY(EditAnywhere)
  float Friction = 0.0f;
  // 可在编辑器中编辑的属性，指向一个 BoxComponent 类型的触发体
  UPROPERTY(EditAnywhere)
  UBoxComponent *TriggerVolume;
  // 指向 CarlaEpisode 的指针，存储 Episode 信息
  const UCarlaEpisode *Episode = nullptr;
};
