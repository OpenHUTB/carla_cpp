// ��Ȩ���У�c��2017�����������δ�ѧ������Ӿ����ģ�CVC��
//
// ����Ʒ������ʡ��ѧԺ������������
// �йظ���������� <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Settings/QualityLevelUE.h"

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

#include "CarlaSettingsDelegate.generated.h"

class UCarlaSettings;

///����Ϊ���ɵ������е�ÿ����ɫ��������
UCLASS(BlueprintType)
class CARLA_API UCarlaSettingsDelegate : public UObject
{
  GENERATED_BODY()

public:

  UCarlaSettingsDelegate();

  ///����������ΪĬ��ֵ
  void Reset();

  /// CreΪ���������ɵĲ����ߴ����¼��������������˴�ʹ���Զ��庯�����д���
  ///����ʴ˴����������ɵĲ����ߵ��¼����������������ʹ�ô˴��Ķ��ƺ������д���
  void RegisterSpawnHandler(UWorld *World);

  /// ���عؿ���Ӧ�õ�ǰ����
  UFUNCTION(BlueprintCallable, Category = "CARLA Settings", meta = (HidePin = "InWorld"))
  void ApplyQualityLevelPostRestart();

  /// ���عؿ�֮ǰ��Ӧ�õ�ǰ����
  UFUNCTION(BlueprintCallable, Category = "CARLA Settings", meta = (HidePin = "InWorld"))
  void ApplyQualityLevelPreRestart();

  void SetAllActorsDrawDistance(UWorld *world, float max_draw_distance) const;

private:

  UWorld *GetLocalWorld();

  ///����Ӧ�����������ɵĲ����ߣ���Ӧ�õ�ǰ����
  void OnActorSpawned(AActor *Actor);

  /// ������硢ʵ���������Ƿ���Ч��������CarlaSettingsʵ��
  ///@param world���ڻ�ȡCarlaSettings��ʵ��
  void CheckCarlaSettings(UWorld *world);

  ///ִ�����������������ˮƽӦ��������
  void LaunchLowQualityCommands(UWorld *world) const;

  void SetAllRoads(
      UWorld *world,
      float max_draw_distance,
      const TArray<FStaticMaterial> &road_pieces_materials) const;

  void SetActorComponentsDrawDistance(AActor *actor, float max_draw_distance) const;

  void SetPostProcessEffectsEnabled(UWorld *world, bool enabled) const;

  ///ִ�����������ʷʫ�������ˮƽӦ��������
  void LaunchEpicQualityCommands(UWorld *world) const;

  void SetAllLights(
      UWorld *world,
      float max_distance_fade,
      bool cast_shadows,
      bool hide_non_directional) const;

private:

  /// �������������ǰӦ�õ���������
  static EQualityLevel AppliedLowPostResetQualityLevel;

  UCarlaSettings *CarlaSettings = nullptr;

  FOnActorSpawned::FDelegate ActorSpawnedDelegate;
};
