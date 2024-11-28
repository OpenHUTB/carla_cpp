// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "USDImporterActor.h"
// 引入名为"USDImporterActor.h"的头文件，通常该头文件中包含了与当前类AUSDImporterActor相关的类型定义、函数声明等内容

AUSDImporterActor::AUSDImporterActor(const FObjectInitializer &ObjectInitializer)
  // AUSDImporterActor类的构造函数，接受一个FObjectInitializer对象用于初始化类成员
  // 通过调用父类的构造函数，传入ObjectInitializer对象，以完成父类部分的初始化
  : Super(ObjectInitializer)
{
  // 设置该Actor的Tick函数是否可被调用，这里设置为false，表示不需要每帧更新
  PrimaryActorTick.bCanEverTick = false;
  // 创建一个默认的场景组件作为根场景组件，并将其命名为"SceneRootComponent"
  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComponent"));
  // 将创建的根场景组件RootSceneComponent设置为Actor的根组件
  RootComponent = RootSceneComponent;
  // 设置根组件的移动性为可移动（Movable），与静态（Static）相对，意味着该组件在场景中可以移动位置等操作
  RootComponent->SetMobility(EComponentMobility::Movable);
}


void AUSDImporterActor::LoadUSDFile()
{
  // 此函数用于加载USD文件，但目前函数体为空，可能在后续的开发中会补充具体的加载USD文件的实现逻辑

}