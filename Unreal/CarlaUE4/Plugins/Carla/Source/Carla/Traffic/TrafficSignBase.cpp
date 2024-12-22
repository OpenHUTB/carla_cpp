// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 引入Carla相关的头文件，可能包含了Carla引擎的一些基础定义、类型等
#include "Carla.h"
// 引入交通标志基础类的头文件，应该定义了交通标志相关的基础功能和属性等
#include "TrafficSignBase.h"
// 引入标志组件的头文件，可能用于处理交通标志的具体组件相关功能
#include "SignComponent.h"

// 定义交通标志基础类ATrafficSignBase的构造函数，接受一个FObjectInitializer类型的常量引用作为参数
// FObjectInitializer通常用于在创建对象时进行一些初始化设置
ATrafficSignBase::ATrafficSignBase(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer) {
  // 设置该Actor的Tick函数是否可以被调用，这里设置为false，表示该Actor不需要每帧进行更新操作
  PrimaryActorTick.bCanEverTick = false;

  // 使用传入的ObjectInitializer创建一个默认的场景组件作为根组件
  // 并将其命名为"SceneRootComponent"，该根组件将作为整个交通标志Actor的基础组件
  RootComponent =
      ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneRootComponent"));
  // 设置根组件的移动性为静态，即该组件在场景中不会移动
  RootComponent->SetMobility(EComponentMobility::Static);
}

// 定义一个函数GetTriggerVolumes，用于获取交通标志的触发体积组件数组
// 函数返回值类型为TArray<UBoxComponent*>，即一个UBoxComponent类型的数组
TArray<UBoxComponent*> ATrafficSignBase::GetTriggerVolumes() const
{
  // 创建一个USignComponent类型的数组，用于存储获取到的标志组件
  TArray<USignComponent*> Components;
  // 通过GetComponents函数获取该交通标志Actor下的所有USignComponent组件
  // 第二个参数false表示不包括已经被标记为待删除的组件
  GetComponents<USignComponent>(Components, false);
  // 判断获取到的标志组件数组的元素数量是否大于0
  if (Components.Num())
  {
    // 如果有标志组件，取出数组中的第一个标志组件
    USignComponent* SignComponent = Components[0];
    // 调用该标志组件的GetEffectTriggerVolume函数获取其触发体积组件数组并返回
    return SignComponent->GetEffectTriggerVolume();
  }
  else
  {
    // 如果没有获取到标志组件，创建一个新的UBoxComponent类型的数组，用于存储触发体积组件
    TArray<UBoxComponent*> TriggerVolumes;
    // 将该交通标志Actor自身的触发体积组件（通过GetTriggerVolume函数获取）添加到数组中
    TriggerVolumes.Add(GetTriggerVolume());
    // 返回包含该交通标志Actor自身触发体积组件的数组
    return TriggerVolumes;
  }
}
