// 版权所有 (c) 2017 计算机视觉中心 (CVC) 在巴塞罗那自治大学 (UAB)。
//
// 本作品根据 MIT 许可证的条款进行许可。
// 了解更多，请访问 <https://opensource.org/licenses/MIT>。

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorSpawnResult.h"

#include "Containers/Array.h"
#include "GameFramework/Actor.h"

#include "CarlaActorFactory.generated.h"

/// 基类，用于 Carla 演员工厂。
UCLASS(Abstract)
class CARLA_API ACarlaActorFactory : public AActor
{
  GENERATED_BODY()

public:

  ACarlaActorFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
  {
    // 设置主演员 tick 不会永远执行
    PrimaryActorTick.bCanEverTick = false;
  }

  /// 获取这个类能够生成的演员定义列表。
  /// @return 演员定义的数组
  virtual TArray<FActorDefinition> GetDefinitions() {
    // 未实现的函数
    unimplemented();
    // 返回空数组
    return {};
  }

  /// 根据 @a ActorDescription 和 @a Transform 生成一个演员。
  ///
  /// @pre ActorDescription 被期望是从 GetDefinitions 中检索到的定义之一衍生而来的。
  /// @param SpawnAtTransform 生成演员时的变换
  /// @param ActorDescription 演员的描述
  /// @return 演员生成结果
  virtual FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) {
    // 未实现的函数
    unimplemented();
    // 返回空的生成结果
    return {};
  }
};
