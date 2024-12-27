// 版权所有 (c) 2017 计算机视觉中心 (CVC) 在巴塞罗那自治大学 (UAB)。
//
// 本作品根据 MIT 许可证的条款进行许可。
// 了解更多，请访问 <https://opensource.org/licenses/MIT>。

#pragma once

#include "Carla/Actor/ActorSpawnResult.h"
#include "Carla/Actor/CarlaActorFactory.h"

#include "GameFramework/Actor.h"

#include "CarlaActorFactoryBlueprint.generated.h"

/// 实现 ACarlaActorFactory 接口的蓝图的基础类。
///
/// 从这个类派生的蓝图预期覆盖 GetDefinitions 和 SpawnActor 函数。
UCLASS(Abstract, BlueprintType, Blueprintable)
class CARLA_API ACarlaActorFactoryBlueprint : public ACarlaActorFactory
{
  GENERATED_BODY()

public:

  TArray<FActorDefinition> GetDefinitions() final
  {
    // 调用蓝图实现的 GenerateDefinitions 函数来获取演员定义
    return GenerateDefinitions();
  }

  FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) final
  {
    // 创建一个演员生成结果对象
    FActorSpawnResult Result;
    // 调用蓝图实现的 SpawnActor 函数来生成演员
    SpawnActor(SpawnAtTransform, ActorDescription, Result);
    // 返回生成结果
    return Result;
  }

protected:

  UFUNCTION(BlueprintImplementableEvent)
  TArray<FActorDefinition> GenerateDefinitions();

  UFUNCTION(BlueprintImplementableEvent)
  void SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription,
      FActorSpawnResult &SpawnResult);
};
