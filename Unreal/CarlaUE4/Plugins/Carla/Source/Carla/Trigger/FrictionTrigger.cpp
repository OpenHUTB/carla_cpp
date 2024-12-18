// 摩擦力触发器
// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "FrictionTrigger.h"
#include "Vehicle/CarlaWheeledVehicle.h"

AFrictionTrigger::AFrictionTrigger(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneRootComponent"));
  RootComponent->SetMobility(EComponentMobility::Static);

  TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
  TriggerVolume->SetupAttachment(RootComponent);
  TriggerVolume->SetHiddenInGame(true);
  TriggerVolume->SetMobility(EComponentMobility::Static);
  TriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
  TriggerVolume->SetGenerateOverlapEvents(true);
}

void AFrictionTrigger::Init()
{
  // 在开始重叠时注册委托。
  if (!TriggerVolume->OnComponentBeginOverlap.IsAlreadyBound(this, &AFrictionTrigger::OnTriggerBeginOverlap))
  {
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AFrictionTrigger::OnTriggerBeginOverlap);
  }

  // 在末端重叠处注册委托。
  if (!TriggerVolume->OnComponentEndOverlap.IsAlreadyBound(this, &AFrictionTrigger::OnTriggerEndOverlap))
  {
    TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AFrictionTrigger::OnTriggerEndOverlap);
  }
}

// 更新轮胎的摩擦力
void AFrictionTrigger::UpdateWheelsFriction(AActor *OtherActor, TArray<float>& NewFriction)
{
  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle != nullptr)
    Vehicle->SetWheelsFrictionScale(NewFriction);
}

void AFrictionTrigger::OnTriggerBeginOverlap(
    UPrimitiveComponent * /*OverlappedComp*/,
    AActor *OtherActor,
    UPrimitiveComponent * /*OtherComp*/,
    int32 /*OtherBodyIndex*/,
    bool /*bFromSweep*/,
    const FHitResult & /*SweepResult*/)
{

  // 保存车轮的原始摩擦力。
  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if(Vehicle == nullptr)
    return;

  OldFrictionValues = Vehicle->GetWheelsFrictionScale();
  TArray<float> TriggerFriction = {Friction, Friction, Friction, Friction};
  UpdateWheelsFriction(OtherActor, TriggerFriction);
}

void AFrictionTrigger::OnTriggerEndOverlap(
    UPrimitiveComponent * /*OverlappedComp*/,
    AActor *OtherActor,
    UPrimitiveComponent * /*OtherComp*/,
    int32 /*OtherBodyIndex*/)
{
  // 恢复默认摩擦力值
  UpdateWheelsFriction(OtherActor, OldFrictionValues);

  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if(Vehicle == nullptr)
    return;

  TArray<float> CurrFriction = Vehicle->GetWheelsFrictionScale();
}

// 游戏开始或生成时调用
void AFrictionTrigger::BeginPlay()
{
  Super::BeginPlay();
  Init();
}

void AFrictionTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  // 取消注册委托
  if (TriggerVolume->OnComponentBeginOverlap.IsAlreadyBound(this, &AFrictionTrigger::OnTriggerBeginOverlap))
  {
    TriggerVolume->OnComponentBeginOverlap.RemoveDynamic(this, &AFrictionTrigger::OnTriggerBeginOverlap);
  }

  if (TriggerVolume->OnComponentEndOverlap.IsAlreadyBound(this, &AFrictionTrigger::OnTriggerEndOverlap))
  {
    TriggerVolume->OnComponentEndOverlap.RemoveDynamic(this, &AFrictionTrigger::OnTriggerEndOverlap);
  }

  Super::EndPlay(EndPlayReason);
}

// 每一帧都调用
void AFrictionTrigger::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
}
