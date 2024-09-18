// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"

#include <compiler/disable-ue4-macros.h>
#include <boost/optional.hpp>
#include "carla/road/Map.h"
#include <compiler/enable-ue4-macros.h>

#include "Vehicle/VehicleSpawnPoint.h"

#include "OpenDriveGenerator.generated.h"

UCLASS()
class CARLA_API AProceduralMeshActor : public AActor
{
  GENERATED_BODY()
public:
  AProceduralMeshActor();

  UPROPERTY(Category = "Procedural Mesh Actor", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  UProceduralMeshComponent* MeshComponent;
};

UCLASS()
class CARLA_API AOpenDriveGenerator : public AActor
{
  GENERATED_BODY()

public:

  AOpenDriveGenerator(const FObjectInitializer &ObjectInitializer);

  /// 将OpenDRIVE信息设置为字符串，并生成可查询的映射结构。
  bool LoadOpenDrive(const FString &OpenDrive);

  /// 以字符串形式获取OpenDRIVE信息。
  const FString &GetOpenDrive() const;

  /// 检查是否已加载OpenDrive及其有效性。
  bool IsOpenDriveValid() const;

  /// 基于OpenDRIVE信息生成道路和人行道网格。
  void GenerateRoadMesh();

  /// 基于OpenDRIVE信息生成杆网格。
  void GeneratePoles();

  /// 沿道路生成生成生成点。
  void GenerateSpawnPoints();

  void GenerateAll();

protected:

  virtual void BeginPlay() override;

  /// 确定产卵器相对于每个RoutePlanner的放置高度
  UPROPERTY(Category = "Spawners", EditAnywhere)
  float SpawnersHeight = 300.f;

  UPROPERTY(Category = "Spawners", EditAnywhere)
  TArray<AVehicleSpawnPoint *> VehicleSpawners;

  UPROPERTY(EditAnywhere)
  FString OpenDriveData;

  UPROPERTY(EditAnywhere)
  TArray<AActor *> ActorMeshList;

};
