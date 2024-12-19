// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 本作品根据 MIT 许可证的条款进行许可。
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#指示 一次

#包括 "Carla/Util/BoundingBox.h"

#包括 "Kismet/BlueprintFunctionLibrary.h"

#包括 "GameFramework/Actor.h"

#包括 "BoundingBoxCalculator.generated.h"

UCLASS()
class CARLA_API UBoundingBoxCalculator : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  /// Compute the bounding box of the given Carla actor.
  ///
  /// @warning If the actor type is not supported a default initialized bounding
  /// box is returned.
  ///
  /// @warning Traffic signs return its trigger box instead.
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

  // 根据 BB 的距离和类型组合角色的 BB
  // 未组合的 BB 也包括在内（即：TL BB 和极点）
  // DistanceThreshold 是 BB 之间要组合的最大距离，如果为 0.0，则忽略该距离
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
