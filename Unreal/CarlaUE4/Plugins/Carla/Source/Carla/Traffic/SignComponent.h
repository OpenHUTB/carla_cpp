// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Carla/OpenDrive/OpenDrive.h"
#include <utility>
#include "SignComponent.generated.h"

namespace carla
{
namespace road
{
  class Map;
namespace element
{
  class RoadInfoSignal;
}
}
}

namespace cr = carla::road;
namespace cre = carla::road::element;

/// 表示 OpenDRIVE 标志的类
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API USignComponent : public USceneComponent
{
  GENERATED_BODY()

public:
  USignComponent();

  UFUNCTION(Category = "Traffic Sign", BlueprintPure)
  const FString &GetSignId() const;

  UFUNCTION(Category = "Traffic Sign", BlueprintCallable)
  void SetSignId(const FString &Id);

  // 初始化标志（例如生成触发框）
  virtual void InitializeSign(const cr::Map &Map);

  void AddEffectTriggerVolume(UBoxComponent* TriggerVolume);

  const TArray<UBoxComponent*> GetEffectTriggerVolume() const;

protected:
  // 游戏开始时调用
  virtual void BeginPlay() override;

  // 每帧调用
  virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

  TArray<std::pair<cr::RoadId, const cre::RoadInfoSignal*>>
      GetAllReferencesToThisSignal(const cr::Map &Map);

  const cr::Signal* GetSignal(const cr::Map &Map) const;

  /// 在父级参与者中生成触发框组件
  /// BoxSize 应采用虚幻单位
  UBoxComponent* GenerateTriggerBox(const FTransform &BoxTransform,
      float BoxSize);

  UBoxComponent* GenerateTriggerBox(const FTransform &BoxTransform,
      const FVector &BoxSize);

private:

  UPROPERTY(Category = "Traffic Sign", EditAnywhere)
  FString SignId = "";

  UPROPERTY()
  TArray<UBoxComponent*> EffectTriggerVolumes;

};
