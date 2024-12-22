// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include <vector>

// 禁用Eigen 3.1.0中的警告
#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Wmisleading-indentation"
#  pragma clang diagnostic ignored "-Wint-in-bool-context"
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  pragma clang diagnostic ignored "-Wshadow"
#endif
#include <Eigen/Dense>
#include <Eigen/Cholesky>
#include <Eigen/Eigenvalues>
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

#include "SpringBasedVegetationComponent.generated.h"

USTRUCT(BlueprintType)// 定义一个可在蓝图中使用的结构体
struct FSkeletonBone
{
// GENERATED_BODY()宏用于自动生成Unreal Engine需要的反射代码
// 这使得结构体可以在Unreal Engine的序列化系统中被正确处理
  GENERATED_BODY()
// 使用UPROPERTY宏定义一个属性
// EditAnywhere表示该属性可以在属性窗口中编辑，无论是在C++代码中还是在蓝图编辑器中
// BlueprintReadWrite表示该属性在蓝图编辑器中可读可写
// Category参数为该属性指定了一个分类，便于在属性窗口中查找
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  float Mass = 1.f;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  float Length = 0.5f;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FVector CenterOfMass = FVector(0,0,0);
};

USTRUCT(BlueprintType) // 定义一个可在蓝图中使用的结构体
struct FSkeletonJoint// 骨骼关节的结构体
{
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  int JointId;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  int ParentId;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FString JointName;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  bool bIsStatic = false;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  TArray<int> ChildrenIds;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FTransform Transform; // relative to parent
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FRotator RestingAngles; // resting angle position of springs
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  float Mass = 10.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  float SpringStrength = 1000.f;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FRotator AngularVelocity = FRotator(0,0,0);
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FRotator AngularAcceleration = FRotator(0,0,0);
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FTransform GlobalTransform;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FTransform GolbalInverseTransform;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  TArray<FSkeletonBone> Bones;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FVector ExternalForces = FVector(0,0,0);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  float CollisionForceProportionalFactor = 1.0f;
};

struct FJointCollision// 关节碰撞结构体
{
  bool CanRest = true;
  int Iteration = 1;
};

struct FJointProperties
{
 // 关节的质量，默认为0.0
 // 质量是影响物体惯性和重力效应的物理量
  float Mass = 0.0;
// 关节的惯性张量，初始化为零矩阵
// 惯性张量描述了物体在旋转时的惯性，是质量分布和物体形状的函数
  Eigen::Matrix3d InertiaTensor = Eigen::Matrix3d::Zero();
// 作用在关节上的力，初始化为零向量
// 力是改变物体运动状态的原因，包括线性和旋转运动
  Eigen::Vector3d Force = Eigen::Vector3d::Zero();
// 作用在关节上的力矩，初始化为零向量
// 力矩是改变物体旋转状态的原因，与力和力臂的叉积有关
  Eigen::Vector3d Torque = Eigen::Vector3d::Zero();
// 虚力矩，初始化为零向量
// 虚力矩可能用于计算或校正，通常与刚体动力学中的某些特定问题相关
  Eigen::Vector3d FictitiousTorque = Eigen::Vector3d::Zero();
// 关节的质心位置，初始化为零向量
// 质心是物体质量分布的平均位置，对于计算物体的整体运动很重要
  Eigen::Vector3d CenterOfMass = Eigen::Vector3d::Zero();
// 关节到全局坐标系的变换矩阵，初始化为零矩阵
// 这个矩阵用于将关节的局部坐标转换为全局坐标
  Eigen::Matrix3d JointToGlobalMatrix = Eigen::Matrix3d::Zero();
// 关节的角速度，初始化为零向量
 // 角速度描述了物体旋转的快慢和方向
  Eigen::Vector3d AngularVelocity = Eigen::Vector3d::Zero();
  Eigen::Vector3d LinearVelocity = Eigen::Vector3d::Zero();
  Eigen::Vector3d AngularAcceleration = Eigen::Vector3d::Zero();
  Eigen::Vector3d LinearAcceleration = Eigen::Vector3d::Zero();
  Eigen::Vector3d LocalAngularAcceleration = Eigen::Vector3d::Zero();
};

USTRUCT(BlueprintType)
struct FSkeletonHierarchy
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  TArray<FSkeletonJoint> Joints;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  TArray<int> EndJoints;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  TArray<int> EndToRootOrder;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  TArray<int> RootToEndOrder;
  
  void Clear();
  void ComputeChildrenJointsAndBones();
  void ComputeEndJoints();
  void AddForce(const FString& BoneName, const FVector& Force);
  void ClearExternalForces();
  FSkeletonJoint& GetRootJoint() { return Joints[0]; }
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API USpringBasedVegetationComponent : public UActorComponent
{
  GENERATED_BODY()

public:

  USpringBasedVegetationComponent(const FObjectInitializer& ObjectInitializer);

  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  void TickComponent(float DeltaTime,
      enum ELevelTick TickType,
      FActorComponentTickFunction * ThisTickFunction) override;

  void UpdateSkeletalMesh();

  void UpdateGlobalTransform();

  void GenerateCollisionCapsules();

  void ResetComponent();

  UFUNCTION(CallInEditor, Category = "Spring Based Vegetation Component")
  void ComputeSpringStrengthForBranches();

  UFUNCTION()
  void OnCollisionEvent(
      UPrimitiveComponent* HitComponent,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComponent,
      FVector NormalImpulse,
      const FHitResult& Hit);

  UFUNCTION()
  void OnBeginOverlapEvent(
      UPrimitiveComponent* OverlapComponent,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComponent,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult& SweepResult);

  UFUNCTION()
  void OnEndOverlapEvent(
      UPrimitiveComponent* OverlapComponent,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComponent,
      int32 OtherBodyIndex);

  UFUNCTION(CallInEditor, Category = "Spring Based Vegetation Component")
  void GenerateSkeletonHierarchy();
  // UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spring Based Vegetation Component")
  // TArray<AActor*> OverlappingActors;
  // UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  TMap<AActor*, TArray<UPrimitiveComponent*>> OverlappingActors;

  UFUNCTION(BlueprintCallable)
  void AddForce(const FString& BoneName, const FVector& Force)
  {
    Skeleton.AddForce(BoneName, Force);
  }
  UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  TArray<FString> FixedJointsList = {"joint1"};

  UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  TArray<FString> DebugJointsToSimulate = {};

private:

  void ComputePerJointProperties(
      std::vector<FJointProperties>& JointLocalPropertiesList,
      std::vector<FJointProperties>& JointPropertiesList);
  void ComputeCompositeBodyContribution(
      std::vector<FJointProperties>& JointLocalPropertiesList,
      std::vector<FJointProperties>& JointPropertiesList);
  void ComputeFictitiousForces(
      std::vector<FJointProperties>& JointLocalPropertiesList,
      std::vector<FJointProperties>& JointPropertiesList);
  void ResolveContactsAndCollisions(
      std::vector<FJointProperties>& JointLocalPropertiesList,
      std::vector<FJointProperties>& JointPropertiesList);
  void SolveEquationOfMotion(std::vector<FJointProperties>& JointPropertiesList, float DeltaTime);

  std::vector<FJointCollision> JointCollisionList;

public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  USkeletalMeshComponent* SkeletalMesh;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  TArray<UPrimitiveComponent*> BoneCapsules;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  TMap<UPrimitiveComponent*, int> CapsuleToJointId;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float Beta = 0.5f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float Alpha = 0.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  FVector Gravity = FVector(0,0,-1);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float BaseSpringStrength = 10000.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float MinSpringStrength = 2000.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float HorizontalFallof = 0.1f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float VerticalFallof = 0.1f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float RestFactor = 0.5f;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float DeltaTimeOverride = -1.f;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float CollisionForceParameter = 10.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float CollisionForceMinVel = 1.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float ForceDistanceFalloffExponent = 1.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float ForceMaxDistance = 180.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float MinForceFactor = 0.01;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float LineTraceMaxDistance = 180.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float CapsuleRadius = 6.0f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float MinBoneLength = 10.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  FVector SpringStrengthMulFactor = FVector(1,1,1);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float VehicleCenterZOffset = 120.f;
  


public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  FSkeletonHierarchy Skeleton;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  bool DebugEnableVisualization { false };

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  bool DebugEnableAllCollisions { false };

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  bool bAutoComputeStrength = true;
};
