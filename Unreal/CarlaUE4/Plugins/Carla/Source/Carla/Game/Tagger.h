// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/ObjectLabel.h>
#include <compiler/enable-ue4-macros.h>

#include "Tagger.generated.h"

namespace crp = carla::rpc;

/// 根据语义分割设置 actor 的自定义深度模板值
/// 到他们的网格中。
///
/// 存在非静态函数，因此可以将其放入场景中进行测试
///目的。
UCLASS()
class CARLA_API ATagger : public AActor
{
  GENERATED_BODY()

public:

  /// 设置角色的标签。
  ///
  /// 如果 bTagForSemanticSegmentation 为 true，则激活自定义深度通道。
  /// pass 是渲染语义分割所必需的。
  ///添加性能损失，因为遮挡似乎未应用于
  ///具有 active 此值的对象。
  
  static void TagActor(const AActor &Actor, bool bTagForSemanticSegmentation);


  /// 设置 level 中每个 actor 的标签。
  ///
  /// 如果 bTagForSemanticSegmentation 为 true，则激活自定义深度通道。这
  /// pass 是渲染语义分割所必需的。但是，它可能会
  ///添加性能损失，因为遮挡似乎未应用于
   ///具有 active 此值的对象。
  static void TagActorsInLevel（UWorld &World， bool bTagForSemanticSegmentation）;

  static void TagActorsInLevel(ULevel &Level, bool bTagForSemanticSegmentation);

  /// 检索已标记组件的标记。
  static crp::CityObjectLabel GetTagOfTaggedComponent(const UPrimitiveComponent &Component)
  {
    return static_cast<crp::CityObjectLabel>(Component.CustomDepthStencilValue);
  }

  /// 检索已标记的角色的标记。CityObjectLabel：：None 为
 /// 未添加到数组中。
  static void GetTagsOfTaggedActor(const AActor &Actor, TSet<crp::CityObjectLabel> &Tags);

  /// 如果 @a Component 已使用给定的 @a 标记进行标记，则返回 true。
  static bool MatchComponent(const UPrimitiveComponent &Component, crp::CityObjectLabel Tag)
  {
    return (Tag == GetTagOfTaggedComponent(Component));
  }

  /// 检索已标记的角色的标记。CityObjectLabel：：None 为
  ///未添加到数组中。
  static FString GetTagAsString(crp::CityObjectLabel Tag);

  
  
  ///使用存储它的文件夹路径
  template <typename T>
  static crp::CityObjectLabel GetLabelByPath(const T *Object) {
    const FString Path = Object->GetPathName();
    TArray<FString> StringArray;
    Path.ParseIntoArray(StringArray, TEXT("/"), false);
    return (StringArray.Num() > 4 ? GetLabelByFolderName(StringArray[4]) : crp::CityObjectLabel::None);
  }

  static void SetStencilValue(UPrimitiveComponent &Component,
    const crp::CityObjectLabel &Label, const bool bSetRenderCustomDepth);

  static FLinearColor GetActorLabelColor(const AActor &Actor, const crp::CityObjectLabel &Label);

  static bool IsThing(const crp::CityObjectLabel &Label);

  ATagger();

protected:

#if WITH_EDITOR
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif 

private:

  UPROPERTY(Category = "Tagger", EditAnywhere)
  bool bTriggerTagObjects = false;

  UPROPERTY(Category = "Tagger", EditAnywhere)
  bool bTagForSemanticSegmentation = false;
};
