// 版权所有（c）2017巴塞罗那自治大学计算机视觉中心（CVC）
//
// 本作品根据麻省理工学院许可条款获得许可
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Settings/QualityLevelUE.h"

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

#include "CarlaSettingsDelegate.generated.h"

class UCarlaSettings;

///用于为生成到世界中的每个角色设置设置
UCLASS(BlueprintType)
class CARLA_API UCarlaSettingsDelegate : public UObject
{
  GENERATED_BODY()

public:

  UCarlaSettingsDelegate();

  ///将设置重置为默认值
  void Reset();

  /// Cre为所有新生成的参与者创建事件触发器处理程序此处使用自定义函数进行处理
  ///请访问此处所有新生成的参与者的事件触发器处理程序，以使用此处的定制函数进行处理
  void RegisterSpawnHandler(UWorld *World);

  /// 加载关卡后，应用当前设置
  UFUNCTION(BlueprintCallable, Category = "CARLA Settings", meta = (HidePin = "InWorld"))
  void ApplyQualityLevelPostRestart();

  /// 加载关卡之前，应用当前设置
  UFUNCTION(BlueprintCallable, Category = "CARLA Settings", meta = (HidePin = "InWorld"))
  void ApplyQualityLevelPreRestart();

  void SetAllActorsDrawDistance(UWorld *world, float max_draw_distance) const;

private:

  UWorld *GetLocalWorld();

  ///函数应用于正在生成的参与者，以应用当前设置
  void OnActorSpawned(AActor *Actor);

  /// 检查世界、实例和设置是否有效，并保存CarlaSettings实例
  ///@param world用于获取CarlaSettings的实例
  void CheckCarlaSettings(UWorld *world);

  ///执行引擎命令，将低质量水平应用于世界
  void LaunchLowQualityCommands(UWorld *world) const;

  void SetAllRoads(
      UWorld *world,
      float max_draw_distance,
      const TArray<FStaticMaterial> &road_pieces_materials) const;

  void SetActorComponentsDrawDistance(AActor *actor, float max_draw_distance) const;

  void SetPostProcessEffectsEnabled(UWorld *world, bool enabled) const;

  ///执行引擎命令，将史诗般的质量水平应用于世界
  void LaunchEpicQualityCommands(UWorld *world) const;

  void SetAllLights(
      UWorld *world,
      float max_distance_fade,
      bool cast_shadows,
      bool hide_non_directional) const;

private:

  /// 重新启动级别后当前应用的质量级别
  static EQualityLevel AppliedLowPostResetQualityLevel;

  UCarlaSettings *CarlaSettings = nullptr;

  FOnActorSpawned::FDelegate ActorSpawnedDelegate;
};
