// 版权所有 (c) 2017 Computer Vision Center (CVC)，巴塞罗那自治大学 (UAB)。
//
// 此作品基于 MIT 许可证授权。
// 如需获取许可证副本，请访问 <https://opensource.org/licenses/MIT>。

#pragma once

#include "Carla/Actor/ActorAttribute.h"

#include "GameFramework/Actor.h"

#include "ActorDefinition.generated.h"

/// 一个 Carla 角色的定义，包含所有变体和属性。
USTRUCT(BlueprintType)
struct FActorDefinition
{
  GENERATED_BODY()

  /// 唯一标识该定义的 ID（无需手动填充）。
  uint32 UId = 0u;

  /// 用于标识角色的显示 ID。
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Id;

  /// 要生成的角色的类（可选）。
  ///
  /// 注意，此参数在客户端不可见，仅由生成器使用。
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<AActor> Class;

  /// 逗号分隔的标签列表。
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Tags;

  /// 变体表示用户可以修改的变量，用于生成角色的不同变体。
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FActorVariation> Variations;

  /// 属性表示角色的不可修改属性，这些属性可能有助于用户识别和过滤角色。
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FActorAttribute> Attributes;
};

/// 一个车辆角色的定义，包含所有变体和属性。
USTRUCT(BlueprintType)
struct FVehicleActorDefinition
{
  GENERATED_BODY()

  /// 逗号分隔的标签列表。
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Tags;

  /// 将静态网格分配给角色。
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UStaticMesh* mesh;

  /// 颜色表示每个角色可用的颜色列表。
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FColor> colors;
};
