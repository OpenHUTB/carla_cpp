// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "WalkerBoneControlIn.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FWalkerBoneControlIn
{
  GENERATED_BODY()

  // 定义一个 UPROPERTY，表示一个可以在编辑器中查看和修改的属性
  // Category: 指定该属性所在的类别，用于编辑器中的组织
  // EditAnywhere: 表示该属性可以在任何地方编辑
  // BlueprintReadWrite: 表示该属性可以在蓝图中读写
  UPROPERTY(Category = "Walker Bone Control", EditAnywhere, BlueprintReadWrite)
  
  // 该属性表示一个键值对映射（TMap），键为 FString 类型，值为 FTransform 类型
  // 这个映射用于存储与骨骼相关的变换信息
  TMap<FString, FTransform> BoneTransforms;

};
