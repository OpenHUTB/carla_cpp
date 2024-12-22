// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Util/BoundingBox.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GameFramework/Actor.h"

#include "BoundingBoxCalculator.generated.h"

UCLASS()
class CARLA_API UBoundingBoxCalculator : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  /// 计算给定 Carla actor 的边界框。
  ///
  /// @warning 如果不支持 actor 类型，则默认初始化边界
  /// 返回box
  ///
  /// @warning 交通标志会返回其触发框。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FBoundingBox GetActorBoundingBox(
    const AActor *Actor,
    uint8 InTagQueried = 0xFF);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FBoundingBox GetVehicleBoundingBox(
    const ACarlaWheeledVehicle* Vehicle,
    uint8 InTagQueried = 0xFF);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FBoundingBox GetCharacterBoundingBox(
    const ACharacter* Character,
    uint8 InTagQueried = 0xFF);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void GetTrafficLightBoundingBox(
    const ATrafficLightBase* TrafficLight,
    TArray<FBoundingBox>& OutBB,
    TArray<uint8>& OutTag,
    uint8 InTagQueried = 0xFF);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static FBoundingBox GetSkeletalMeshBoundingBox(const USkeletalMesh* SkeletalMesh);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static FBoundingBox GetStaticMeshBoundingBox(const UStaticMesh* StaticMesh);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static void GetISMBoundingBox(
    UInstancedStaticMeshComponent* ISMComp,
    TArray<FBoundingBox>& OutBoundingBox);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static void GetBBsOfStaticMeshComponents(
    const TArray<UStaticMeshComponent*>& StaticMeshComps,
    TArray<FBoundingBox>& OutBB,
    TArray<uint8>& OutTag,
    uint8 InTagQueried = 0xFF);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static void GetBBsOfSkeletalMeshComponents(
    const TArray<USkeletalMeshComponent*>& SkeletalMeshComps,
    TArray<FBoundingBox>& OutBB,
    TArray<uint8>& OutTag,
    uint8 InTagQueried = 0xFF);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static TArray<FBoundingBox> GetBoundingBoxOfActors(
    const TArray<AActor*>& Actors,
    uint8 InTagQueried = 0xFF);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static TArray<FBoundingBox> GetBBsOfActor(
    const AActor* Actor,
    uint8 InTagQueried = 0xFF);

  // 根据 BB 的距离和类型组合对象的 BB
  // 未组合的 BB 也包括在内（即：TL BB 和极点）
  // DistanceThreshold 为 BB 之间要组合的最大距离，如果为 0.0，则忽略该距离
  // TagToCombine 定义要组合的 BB 的类型，如果为 0，则忽略该类型
  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static void CombineBBsOfActor(
    const AActor* Actor,
    TArray<FBoundingBox>& OutBB,
    TArray<uint8>& OutTag,
    const float DistanceThreshold = 0.0f,
    uint8 TagToCombine = 0xFF);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static FBoundingBox CombineBBs(const TArray<FBoundingBox>& BBsToCombine);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static FBoundingBox CombineBoxes(const TArray<UBoxComponent *>& BBsToCombine);

  // 返回生成 Actor 的 InBB 的静态网格体组件
  // 即：创建 TL 灯箱 BB 的 SMComps
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void GetMeshCompsFromActorBoundingBox(
    const AActor* Actor,
    const FBoundingBox& InBB,
    TArray<UStaticMeshComponent*>& OutStaticMeshComps);

};
