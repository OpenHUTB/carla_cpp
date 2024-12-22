
// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


// 引入名为"UncenteredPivotPointMesh.h"的头文件，通常该头文件中包含了与当前类相关的类型定义、函数声明等内容
#include "UncenteredPivotPointMesh.h"

AUncenteredPivotPointMesh::AUncenteredPivotPointMesh(const FObjectInitializer& ObjectInitializer)
  // AUncenteredPivotPointMesh类的构造函数，接受一个FObjectInitializer对象用于初始化类成员
  // 通过调用父类的构造函数，传入ObjectInitializer对象，以完成父类部分的初始化
  : Super(ObjectInitializer)
{
  // 设置该Actor的Tick函数是否可被调用，这里设置为false，表示不需要每帧更新
  PrimaryActorTick.bCanEverTick = false;

  // 创建一个默认的场景组件作为根组件，并将其命名为"SceneRoot"
  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
  // 创建一个默认的静态网格组件，并将其命名为"StaticMeshComp"
  SMComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
  // 将创建的静态网格组件SMComp附加到根组件RootComponent上，建立父子关系
  SMComp->SetupAttachment(RootComponent);
}