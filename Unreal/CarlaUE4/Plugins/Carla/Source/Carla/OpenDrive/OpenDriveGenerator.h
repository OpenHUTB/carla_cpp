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
//sd
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

  /// ��OpenDRIVE��Ϣ����Ϊ�ַ����������ɿɲ�ѯ��ӳ��ṹ��
  bool LoadOpenDrive(const FString &OpenDrive);

  /// ���ַ�����ʽ��ȡOpenDRIVE��Ϣ��
  const FString &GetOpenDrive() const;

  /// ����Ƿ��Ѽ���OpenDrive������Ч�ԡ�
  bool IsOpenDriveValid() const;

  /// ����OpenDRIVE��Ϣ���ɵ�·�����е�����
  void GenerateRoadMesh();

  /// ����OpenDRIVE��Ϣ���ɸ�����
  void GeneratePoles();

  /// �ص�·�����������ɵ㡣
  void GenerateSpawnPoints();

  void GenerateAll();

protected:

  virtual void BeginPlay() override;

  /// ȷ�������������ÿ��RoutePlanner�ķ��ø߶�
  UPROPERTY(Category = "Spawners", EditAnywhere)
  float SpawnersHeight = 300.f;

  UPROPERTY(Category = "Spawners", EditAnywhere)
  TArray<AVehicleSpawnPoint *> VehicleSpawners;

  UPROPERTY(EditAnywhere)
  FString OpenDriveData;

  UPROPERTY(EditAnywhere)
  TArray<AActor *> ActorMeshList;

};
