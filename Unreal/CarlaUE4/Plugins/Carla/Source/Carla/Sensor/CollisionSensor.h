// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/Sensor.h"

#include "CollisionSensor.generated.h"

class UCarlaEpisode;
class UCarlaGameInstance;

///用于记录碰撞的传感器。
UCLASS()
class CARLA_API ACollisionSensor : public ASensor
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition();

  ACollisionSensor(const FObjectInitializer& ObjectInitializer);

  virtual void PrePhysTick(float DeltaSeconds) override;
  void SetOwner(AActor *NewOwner) override;

  UFUNCTION()
  void OnCollisionEvent(
      AActor *Actor,
      AActor *OtherActor,
      FVector NormalImpulse,
      const FHitResult &Hit);

  UFUNCTION(BlueprintCallable, Category="Collision")
  void OnActorCollisionEvent(
      AActor *Actor,
      AActor *OtherActor,
      FVector NormalImpulse,
      const FHitResult &Hit);

  UFUNCTION()
  void OnComponentCollisionEvent(
      UPrimitiveComponent* HitComp,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComp,
      FVector NormalImpulse,
      const FHitResult& Hit);

private:
  /// 保存所有碰撞的注册表。
  /// 用于避免每帧发送多次相同的碰撞，因为碰撞传感器使用 PhysX 子步节拍信号。有助于传感器的使用和流过载。
  std::vector<std::tuple<uint64_t, AActor*, AActor*>> CollisionRegistry;
};
