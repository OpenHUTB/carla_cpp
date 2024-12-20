#include "Carla.h"
#include "Game/CarlaGameInstance.h"
#include "Carla/Settings/CarlaSettingsDelegate.h"

#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Settings/CarlaSettings.h"

#include "Async.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/HUD.h"
#include "InstancedFoliageActor.h"
#include "Kismet/GameplayStatics.h"
#include "Landscape.h"
#include "Scalability.h"


static constexpr float CARLA_SETTINGS_MAX_SCALE_SIZE = 50.0f;

/// 运行之间的画质设置配置
EQualityLevel UCarlaSettingsDelegate::AppliedLowPostResetQualityLevel = EQualityLevel::Epic;
// 默认的低画质设置
UCarlaSettingsDelegate::UCarlaSettingsDelegate()
  : ActorSpawnedDelegate(FOnActorSpawned::FDelegate::CreateUObject(// 在构造函数中设置演员生成委托
        this,
        &UCarlaSettingsDelegate::OnActorSpawned)) {}

void UCarlaSettingsDelegate::Reset()// 重置设置代理
{
  AppliedLowPostResetQualityLevel = EQualityLevel::Epic;// 重置画质级别
}

void UCarlaSettingsDelegate::RegisterSpawnHandler(UWorld *InWorld)
{
  CheckCarlaSettings(InWorld);
  InWorld->AddOnActorSpawnedHandler(ActorSpawnedDelegate);// 添加演员生成处理器
}

void UCarlaSettingsDelegate::OnActorSpawned(AActor *InActor)
{
  check(CarlaSettings != nullptr);
  if (InActor != nullptr && IsValid(InActor) && !InActor->IsPendingKill() &&
      !InActor->IsA<AInstancedFoliageActor>() && // 植被剔除由每个实例控制
      !InActor->IsA<ALandscape>() && // 不要触碰景观和道路
      !InActor->ActorHasTag(UCarlaSettings::CARLA_ROAD_TAG) &&
      !InActor->ActorHasTag(UCarlaSettings::CARLA_SKY_TAG))
  {
    TArray<UPrimitiveComponent *> components;
    InActor->GetComponents(components);
    switch (CarlaSettings->GetQualityLevel())
    {
      case EQualityLevel::Low: {
        // 将设置应用于此参与者的当前画质级别
        float dist = CarlaSettings->LowStaticMeshMaxDrawDistance;
        const float maxscale = InActor->GetActorScale().GetMax();
        if (maxscale > CARLA_SETTINGS_MAX_SCALE_SIZE)// 如果缩放尺寸超过最大值
        {
          dist *= 100.0f;
        }
        SetActorComponentsDrawDistance(InActor, dist);
        break;
      }
      default: break;
    }
  }
}

void UCarlaSettingsDelegate::ApplyQualityLevelPostRestart()应用画质级别（重启后）
{
  CheckCarlaSettings(nullptr);
  UWorld *InWorld = CarlaSettings->GetWorld();

  const EQualityLevel QualityLevel = CarlaSettings->GetQualityLevel();

  if (AppliedLowPostResetQualityLevel == QualityLevel)
  {
    return;
  }

  // 启用画质的临时变化（防止将最后的画质设置保存到文件）
  Scalability::ToggleTemporaryQualityLevels(true);

  switch (QualityLevel)
  {
    case EQualityLevel::Low:
    {
      // 执行画质调整
      LaunchLowQualityCommands(InWorld);
      // 迭代遍历所有方向光，停用阴影
      SetAllLights(InWorld, CarlaSettings->LowLightFadeDistance, false, true);
      // 将所有道路设置为低质量材料
      SetAllRoads(InWorld, CarlaSettings->LowRoadPieceMeshMaxDrawDistance, CarlaSettings->LowRoadMaterials);
      // 为所有具有静态网格的参与者设置全局设置中配置的最大距离，以实现低质量画质
      SetAllActorsDrawDistance(InWorld, CarlaSettings->LowStaticMeshMaxDrawDistance);
      // Disable all post process volumes
      SetPostProcessEffectsEnabled(InWorld, false);
      break;
    }
    default:
      UE_LOG(LogCarla, Warning, TEXT("Unknown quality level, falling back to default."));
    case EQualityLevel::Epic:
    {
      LaunchEpicQualityCommands(InWorld);// 启动史诗画质命令
      SetAllLights(InWorld, 0.0f, true, false);// 设置所有灯光
      SetAllRoads(InWorld, 0, CarlaSettings->EpicRoadMaterials);// 设置所有道路
      SetAllActorsDrawDistance(InWorld, 0); // 设置所有演员的绘制距离
      SetPostProcessEffectsEnabled(InWorld, true);// 启用后处理效果
      break;
    }
  }
  AppliedLowPostResetQualityLevel = QualityLevel;
}

void UCarlaSettingsDelegate::ApplyQualityLevelPreRestart()
{
  CheckCarlaSettings(nullptr);
  UWorld *InWorld = CarlaSettings->GetWorld();
  if (!IsValid(InWorld) || InWorld->IsPendingKill())// 如果世界无效或即将销毁
  {
    return;
  }
  // 启用或禁用世界和头显渲染
  APlayerController *playercontroller = UGameplayStatics::GetPlayerController(InWorld, 0);
  if (playercontroller)
  {
    ULocalPlayer *player = playercontroller->GetLocalPlayer();// 获取本地玩家
    if (player) // 如果本地玩家有效
    {
      player->ViewportClient->bDisableWorldRendering = CarlaSettings->bDisableRendering;// 设置是否禁用世界渲染
    }
    // 如果我们已经有一个头显类：
    AHUD *hud = playercontroller->GetHUD();
    if (hud)
    {
      hud->bShowHUD = !CarlaSettings->bDisableRendering;
    }
  }

}

UWorld *UCarlaSettingsDelegate::GetLocalWorld()
{
  return GEngine->GetWorldFromContextObjectChecked(this);// 从上下文对象获取世界
}

void UCarlaSettingsDelegate::CheckCarlaSettings(UWorld *world)
{
  if (IsValid(CarlaSettings))
  {
    return;
  }
  if (world == nullptr || !IsValid(world) || world->IsPendingKill())
  {
    world = GetLocalWorld();
  }
  check(world != nullptr);
  auto GameInstance  = Cast<UCarlaGameInstance>(world->GetGameInstance());// 将游戏实例转换为Carla游戏实例
  check(GameInstance != nullptr); // 确保游戏实例不为空
  CarlaSettings = &GameInstance->GetCarlaSettings();
  check(CarlaSettings != nullptr);
}

void UCarlaSettingsDelegate::LaunchLowQualityCommands(UWorld *world) const// 启动低画质命令
{
  if (!world)
  {
    return;
  }

  // 启动命令以降低画质设置
  GEngine->Exec(world, TEXT("r.DefaultFeature.MotionBlur 0"));
  GEngine->Exec(world, TEXT("r.DefaultFeature.Bloom 0"));
  GEngine->Exec(world, TEXT("r.DefaultFeature.AmbientOcclusion 0"));
  GEngine->Exec(world, TEXT("r.AmbientOcclusionLevels 0"));
  GEngine->Exec(world, TEXT("r.DefaultFeature.AmbientOcclusionStaticFraction 0"));
  GEngine->Exec(world, TEXT("r.RHICmdBypass 0"));
  GEngine->Exec(world, TEXT("r.DefaultFeature.AntiAliasing 1"));
  GEngine->Exec(world, TEXT("r.Streaming.PoolSize 2000"));
  GEngine->Exec(world, TEXT("r.HZBOcclusion 0"));
  GEngine->Exec(world, TEXT("r.MinScreenRadiusForLights 0.01"));
  GEngine->Exec(world, TEXT("r.SeparateTranslucency 0"));
  GEngine->Exec(world, TEXT("r.FinishCurrentFrame 0"));
  GEngine->Exec(world, TEXT("r.MotionBlurQuality 0"));
  GEngine->Exec(world, TEXT("r.PostProcessAAQuality 0"));
  GEngine->Exec(world, TEXT("r.BloomQuality 1"));
  GEngine->Exec(world, TEXT("r.SSR.Quality 0"));
  GEngine->Exec(world, TEXT("r.DepthOfFieldQuality 0"));
  GEngine->Exec(world, TEXT("r.SceneColorFormat 2"));
  GEngine->Exec(world, TEXT("r.TranslucencyVolumeBlur 0"));
  GEngine->Exec(world, TEXT("r.TranslucencyLightingVolumeDim 4"));
  GEngine->Exec(world, TEXT("r.MaxAnisotropy 8"));
  GEngine->Exec(world, TEXT("r.LensFlareQuality 0"));
  GEngine->Exec(world, TEXT("r.SceneColorFringeQuality 0"));
  GEngine->Exec(world, TEXT("r.FastBlurThreshold 0"));
  GEngine->Exec(world, TEXT("r.SSR.MaxRoughness 0.1"));
  GEngine->Exec(world, TEXT("r.AllowOcclusionQueries 1"));
  GEngine->Exec(world, TEXT("r.SSR 0"));
  // GEngine->Exec(world,TEXT("r.StencilForLODDither 1")); // 只读
  GEngine->Exec(world, TEXT("r.EarlyZPass 2")); // 透明先于不透明
  GEngine->Exec(world, TEXT("r.EarlyZPassMovable 1"));
  GEngine->Exec(world, TEXT("Foliage.DitheredLOD 0"));
  // GEngine->Exec(world,TEXT("r.ForwardShading 0")); // 只读
  GEngine->Exec(world, TEXT("sg.PostProcessQuality 0"));
  // GEngine->Exec(world,TEXT("r.ViewDistanceScale 0.1")); //--> 太过极端
  // (far clip too short)
  GEngine->Exec(world, TEXT("sg.ShadowQuality 0"));
  GEngine->Exec(world, TEXT("sg.TextureQuality 0"));
  GEngine->Exec(world, TEXT("sg.EffectsQuality 0"));
  GEngine->Exec(world, TEXT("sg.FoliageQuality 0"));
  GEngine->Exec(world, TEXT("foliage.DensityScale 0"));
  GEngine->Exec(world, TEXT("grass.DensityScale 0"));
  GEngine->Exec(world, TEXT("r.TranslucentLightingVolume 0"));
  GEngine->Exec(world, TEXT("r.LightShaftDownSampleFactor 4"));
  GEngine->Exec(world, TEXT("r.OcclusionQueryLocation 1"));
  // GEngine->Exec(world,TEXT("r.BasePassOutputsVelocity 0")); //--> 只读
  // GEngine->Exec(world,TEXT("r.DetailMode 0")); //-->will change to lods 0
  GEngine->Exec(world, TEXT("r.DefaultFeature.AutoExposure 1"));

}

void UCarlaSettingsDelegate::SetAllRoads(
    UWorld *world,
    const float max_draw_distance,
    const TArray<FStaticMaterial> &road_pieces_materials) const
{
  if (!world || !IsValid(world) || world->IsPendingKill())
  {
    return;
  }
  AsyncTask(ENamedThreads::GameThread, [=]() {
    if (!world || !IsValid(world) || world->IsPendingKill())
    {
      return;
    }
    TArray<AActor *> actors;
    UGameplayStatics::GetAllActorsWithTag(world, UCarlaSettings::CARLA_ROAD_TAG, actors);

    for (int32 i = 0; i < actors.Num(); i++)
    {
      AActor *actor = actors[i];
      if (!IsValid(actor) || actor->IsPendingKill())
      {
        continue;
      }
      TArray<UStaticMeshComponent *> components;
      actor->GetComponents(components);
      for (int32 j = 0; j < components.Num(); j++)
      {
        UStaticMeshComponent *staticmeshcomponent = Cast<UStaticMeshComponent>(components[j]);
        if (staticmeshcomponent)
        {
          staticmeshcomponent->bAllowCullDistanceVolume = (max_draw_distance > 0);
          staticmeshcomponent->bUseAsOccluder = false;
          staticmeshcomponent->LDMaxDrawDistance = max_draw_distance;
          staticmeshcomponent->CastShadow = (max_draw_distance == 0);
          if (road_pieces_materials.Num() > 0)
          {
            TArray<FName> meshslotsnames = staticmeshcomponent->GetMaterialSlotNames();
            for (int32 k = 0; k < meshslotsnames.Num(); k++)
            {
              const FName &slotname = meshslotsnames[k];
              road_pieces_materials.ContainsByPredicate(
              [staticmeshcomponent, slotname](const FStaticMaterial &material)
              {
                if (material.MaterialSlotName.IsEqual(slotname))
                {
                  staticmeshcomponent->SetMaterial(
                  staticmeshcomponent->GetMaterialIndex(slotname),
                  material.MaterialInterface);
                  return true;
                }
                else
                {
                  return false;
                }
              });
            }
          }
        }
      }
    }
  }); // ,DELAY_TIME_TO_SET_ALL_ROADS, false);
}

void UCarlaSettingsDelegate::SetActorComponentsDrawDistance(
    AActor *actor,
    const float max_draw_distance) const
{
  if (!actor)
  {
    return;
  }
  TArray<UPrimitiveComponent *> components;
  actor->GetComponents(components, false);
  float dist = max_draw_distance;
  const float maxscale = actor->GetActorScale().GetMax();
  if (maxscale > CARLA_SETTINGS_MAX_SCALE_SIZE)
  {
    dist *= 100.0f;
  }
  for (int32 j = 0; j < components.Num(); j++)
  {
    UPrimitiveComponent *primitivecomponent = Cast<UPrimitiveComponent>(components[j]);
    if (IsValid(primitivecomponent))
    {
      primitivecomponent->SetCullDistance(dist);
      primitivecomponent->bAllowCullDistanceVolume = dist > 0;
    }
  }
}

void UCarlaSettingsDelegate::SetAllActorsDrawDistance(UWorld *world, const float max_draw_distance) const
{
  /// @TODO: 使用语义按类型（车辆、地面、人物、道具）抓取所有参与者，并设置全局属性中配置的不同距离
  if (!world || !IsValid(world) || world->IsPendingKill())
  {
    return;
  }
  AsyncTask(ENamedThreads::GameThread, [=]() {
    if (!world || !IsValid(world) || world->IsPendingKill())
    {
      return;
    }
    TArray<AActor *> actors;
    // 设置较低画质 - 最大绘制距离
    UGameplayStatics::GetAllActorsOfClass(world, AActor::StaticClass(), actors);
    for (int32 i = 0; i < actors.Num(); i++)
    {
      AActor *actor = actors[i];
      if (!IsValid(actor) || actor->IsPendingKill() ||
      actor->IsA<AInstancedFoliageActor>() ||   // foliage culling is controlled
                                                // per instance
      actor->IsA<ALandscape>() ||   // 不要触碰景观和道路
      actor->ActorHasTag(UCarlaSettings::CARLA_ROAD_TAG) ||
      actor->ActorHasTag(UCarlaSettings::CARLA_SKY_TAG))
      {
        continue;
      }
      SetActorComponentsDrawDistance(actor, max_draw_distance);
    }
  });
}

void UCarlaSettingsDelegate::SetPostProcessEffectsEnabled(UWorld *world, const bool enabled) const
{
  TArray<AActor *> actors;
  UGameplayStatics::GetAllActorsOfClass(world, APostProcessVolume::StaticClass(), actors);
  for (int32 i = 0; i < actors.Num(); i++)
  {
    AActor *actor = actors[i];
    if (!IsValid(actor) || actor->IsPendingKill())
    {
      continue;
    }
    APostProcessVolume *postprocessvolume = Cast<APostProcessVolume>(actor);
    if (postprocessvolume)
    {
      postprocessvolume->bEnabled = enabled;
    }
  }
}

void UCarlaSettingsDelegate::LaunchEpicQualityCommands(UWorld *world) const
{
  if (!world)
  {
    return;
  }

  GEngine->Exec(world, TEXT("r.AmbientOcclusionLevels -1"));
  GEngine->Exec(world, TEXT("r.RHICmdBypass 1"));
  GEngine->Exec(world, TEXT("r.DefaultFeature.AntiAliasing 1"));
  GEngine->Exec(world, TEXT("r.Streaming.PoolSize 2000"));
  GEngine->Exec(world, TEXT("r.MinScreenRadiusForLights 0.03"));
  GEngine->Exec(world, TEXT("r.SeparateTranslucency 1"));
  GEngine->Exec(world, TEXT("r.PostProcessAAQuality 4"));
  GEngine->Exec(world, TEXT("r.BloomQuality 5"));
  GEngine->Exec(world, TEXT("r.SSR.Quality 3"));
  GEngine->Exec(world, TEXT("r.DepthOfFieldQuality 2"));
  GEngine->Exec(world, TEXT("r.SceneColorFormat 4"));
  GEngine->Exec(world, TEXT("r.TranslucencyVolumeBlur 1"));
  GEngine->Exec(world, TEXT("r.TranslucencyLightingVolumeDim 64"));
  GEngine->Exec(world, TEXT("r.MaxAnisotropy 8"));
  GEngine->Exec(world, TEXT("r.LensFlareQuality 2"));
  GEngine->Exec(world, TEXT("r.SceneColorFringeQuality 1"));
  GEngine->Exec(world, TEXT("r.FastBlurThreshold 100"));
  GEngine->Exec(world, TEXT("r.SSR.MaxRoughness -1"));
  // GEngine->Exec(world,TEXT("r.StencilForLODDither 0")); // 只读
  GEngine->Exec(world, TEXT("r.EarlyZPass 3"));
  GEngine->Exec(world, TEXT("r.EarlyZPassMovable 1"));
  GEngine->Exec(world, TEXT("Foliage.DitheredLOD 1"));
  GEngine->Exec(world, TEXT("sg.PostProcessQuality 3"));
  GEngine->Exec(world, TEXT("r.ViewDistanceScale 1")); // --> too extreme (far
                                                       // clip too short)
  GEngine->Exec(world, TEXT("sg.ShadowQuality 3"));
  GEngine->Exec(world, TEXT("sg.TextureQuality 3"));
  GEngine->Exec(world, TEXT("sg.EffectsQuality 3"));
  GEngine->Exec(world, TEXT("sg.FoliageQuality 3"));
  GEngine->Exec(world, TEXT("foliage.DensityScale 1"));
  GEngine->Exec(world, TEXT("grass.DensityScale 1"));
  GEngine->Exec(world, TEXT("r.TranslucentLightingVolume 1"));
  GEngine->Exec(world, TEXT("r.LightShaftDownSampleFactor 2"));
  // GEngine->Exec(world,TEXT("r.OcclusionQueryLocation 0"));
  // GEngine->Exec(world,TEXT("r.BasePassOutputsVelocity 0")); // 只读
  GEngine->Exec(world, TEXT("r.DetailMode 2"));
}

void UCarlaSettingsDelegate::SetAllLights(
    UWorld *world,
    const float max_distance_fade,
    const bool cast_shadows,
    const bool hide_non_directional) const
{
  if (!world || !IsValid(world) || world->IsPendingKill())
  {
    return;
  }
  AsyncTask(ENamedThreads::GameThread, [=]() {
    if (!world || !IsValid(world) || world->IsPendingKill())
    {
      return;
    }
    TArray<AActor *> actors;
    UGameplayStatics::GetAllActorsOfClass(world, ALight::StaticClass(), actors);
    for (int32 i = 0; i < actors.Num(); i++)
    {
      if (!IsValid(actors[i]) || actors[i]->IsPendingKill())
      {
        continue;
      }
      // 调整方向光
      ADirectionalLight *directionallight = Cast<ADirectionalLight>(actors[i]);
      if (directionallight)
      {
        directionallight->SetCastShadows(cast_shadows);
        directionallight->SetLightFunctionFadeDistance(max_distance_fade);
        continue;
      }
      // 禁用任何其他类型的灯
      actors[i]->SetActorHiddenInGame(hide_non_directional);
    }
  });

}
