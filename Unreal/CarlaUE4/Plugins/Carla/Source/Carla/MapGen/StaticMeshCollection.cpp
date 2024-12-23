// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "StaticMeshCollection.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

/*
 * AStaticMeshCollection 的构造函数，初始化静态网格集合类的实例。
 * 该构造函数会禁用 Actor 的 Tick（每帧更新），并设置根组件为一个默认的场景组件。
 * 根组件的移动性被设置为静态，表示该组件在游戏运行时不会移动或变化。
 */
AStaticMeshCollection::AStaticMeshCollection(
        const FObjectInitializer &ObjectInitializer) :
        Super(ObjectInitializer) {
    PrimaryActorTick.bCanEverTick = false;
    RootComponent =
            ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComponent"));
    RootComponent->SetMobility(EComponentMobility::Static);
}

/*
 * PushBackInstantiator 函数用于向静态网格集合中添加一个新的实例化静态网格组件。
 * 该函数创建一个新的 UInstancedStaticMeshComponent，并将其附加到根组件上。
 * 然后设置其网格为传入的 Mesh，注册组件并将其添加到 MeshInstantiators 列表中。
 */
void AStaticMeshCollection::PushBackInstantiator(UStaticMesh *Mesh) {
    auto Instantiator = NewObject<UInstancedStaticMeshComponent>(this);
    check(Instantiator != nullptr);
    Instantiator->SetMobility(EComponentMobility::Static);
    Instantiator->SetupAttachment(RootComponent);
    Instantiator->SetStaticMesh(Mesh);
    Instantiator->RegisterComponent();
    MeshInstantiators.Add(Instantiator);
}

/*
 * SetStaticMesh 函数用于根据索引 i 设置静态网格实例化组件的网格。
 * 如果给定的索引有效且对应的实例化组件存在，则更新该组件的静态网格为传入的 Mesh。
 */
void AStaticMeshCollection::SetStaticMesh(uint32 i, UStaticMesh *Mesh) {
    if ((GetNumberOfInstantiators() > i) && (MeshInstantiators[i] != nullptr)) {
        MeshInstantiators[i]->SetStaticMesh(Mesh);
    }
}

/*
 * AddInstance 函数用于向指定索引的静态网格实例化组件添加一个实例。
 * 根据传入的索引 i 和变换信息 Transform，将新的实例添加到对应的实例化组件中。
 * 如果索引有效且对应的实例化组件存在，则调用 AddInstance 方法进行添加。
 */
void AStaticMeshCollection::AddInstance(uint32 i, const FTransform &Transform) {
    if ((GetNumberOfInstantiators() > i) && (MeshInstantiators[i] != nullptr)) {
        MeshInstantiators[i]->AddInstance(Transform);
    }
}

/*
 * ClearInstances 函数用于清除所有静态网格实例化组件中的实例。
 * 遍历所有实例化组件，如果组件不为空，则调用 ClearInstances 方法来移除所有实例。
 */
void AStaticMeshCollection::ClearInstances() {
    for (auto *Instantiator: MeshInstantiators) {
        if (Instantiator != nullptr) {
            Instantiator->ClearInstances();
        }
    }
}

/*
 * ClearInstantiators 函数用于清除所有静态网格实例化组件（Instantiators）。
 * 首先调用 ClearInstances() 清除所有实例，然后清空 MeshInstantiators 数组。
 */
void AStaticMeshCollection::ClearInstantiators() {
    ClearInstances();
    MeshInstantiators.Empty();
}
