// 版权所有 (c) 2017 Computer Vision Center (CVC)，巴塞罗那自治大学 (UAB)。
//
// 此作品基于 MIT 许可证授权。
// 如需获取许可证副本，请访问 <https://opensource.org/licenses/MIT>。

#pragma once

#include "Carla/Actor/ActorAttribute.h"

#include "ActorDescription.generated.h"

/// Carla角色的描述，包含其所有变体。
USTRUCT(BlueprintType)
struct FActorDescription
{
  GENERATED_BODY()

  /// 基于此描述的定义的UId。
  uint32 UId = 0u;

  /// 用于标识角色的显示ID。
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Id;

  /// 要生成的角色的类。
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<AActor> Class;

  /// 用户选择的角色变体。请注意，在此阶段，这些变体由不可修改的属性表示。
  ///
  ///   Key: 属性的ID。
  ///   Value: 属性。
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TMap<FString, FActorAttribute> Variations;
};
