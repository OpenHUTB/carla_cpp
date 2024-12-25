// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "StaticMeshCollection.generated.h"

class UInstancedStaticMeshComponent;

/// 保存静态网格实例化器。
UCLASS(Abstract)
class CARLA_API AStaticMeshCollection : public AActor
{
  GENERATED_BODY()

public:

  AStaticMeshCollection(const FObjectInitializer& ObjectInitializer);

protected:

  uint32 GetNumberOfInstantiators() const
  {
    return MeshInstantiators.Num();
  }

  void PushBackInstantiator(UStaticMesh *Mesh);

  void SetStaticMesh(uint32 i, UStaticMesh *Mesh);

  void AddInstance(uint32 i, const FTransform &Transform);

  void ClearInstances();

/// 同时清除实例。
void ClearInstantiators();

private:

  UPROPERTY(Category = "Instanced Static Mesh Collection", VisibleAnywhere)
  TArray<UInstancedStaticMeshComponent *> MeshInstantiators;
};
