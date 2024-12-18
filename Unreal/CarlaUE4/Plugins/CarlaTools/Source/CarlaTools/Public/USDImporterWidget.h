// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once//用于防止头文件被重复包含，确保头文件内容在编译时只被处理一次。

#include "CoreMinimal.h"//包含核心基础类型和功能声明
#include "Blueprint/UserWidget.h"//用于创建用户界面相关的蓝图类，这里的 UUSDImporterWidget 继承自它
#include "Animation/Skeleton.h"//与骨骼相关操作的头文件，涉及到骨骼网格相关功能
#include "VehicleWheel.h"//可能用于处理车辆轮子相关的数据和操作
#include "Materials/MaterialInterface.h"//处理材质相关功能
#include "PhysicsEngine/PhysicsAsset.h"//针对物理资产相关的操作，比如物理碰撞等物理属性设置

#include "USDImporterWidget.generated.h"//是由虚幻引擎的代码生成工具生成的头文件，包含了与代码生成相关的宏等内容，用于支持当前类和结构体的反射等机制，使得它们能够在虚幻引擎的蓝图系统、序列化等功能中正常工作。

USTRUCT(BlueprintType)//宏进行标记，表明它可以在蓝图中使用，并且通过 GENERATED_BODY() 宏来支持虚幻引擎的代码生成机制（如反射等）。
struct FWheelTemplates
{
  GENERATED_BODY() // 前右轮对应的轮子类模板，可在蓝图中读写、编辑，属于“USD Wheel”类别。
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheel")
  TSubclassOf<UVehicleWheel> WheelFR;// 前左轮对应的轮子类模板，可在蓝图中读写、编辑，属于“USD Wheel”类别。
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheel")
  TSubclassOf<UVehicleWheel> WheelFL;// 后右轮对应的轮子类模板，可在蓝图中读写、编辑，属于“USD Wheel”类别。
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheel")
  TSubclassOf<UVehicleWheel> WheelRR;// 后左轮对应的轮子类模板，可在蓝图中读写、编辑，属于“USD Wheel”类别。
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheel")
  TSubclassOf<UVehicleWheel> WheelRL;
};

USTRUCT(BlueprintType)
struct FVehicleLight
{
  GENERATED_BODY()
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FString Name;//用于存储灯光的名称
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FVector Location;//用于表示灯光在三维空间中的位置
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FLinearColor Color;//用于定义灯光的颜色
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FVehicleMeshAnchorPoints
{
  GENERATED_BODY();

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector DoorFR;//前右轮
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector DoorFL;//前左轮
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector DoorRR;//后右轮
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector DoorRL;//后左轮
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector WheelFR;//前右门
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector WheelFL;//前左门
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector WheelRR;//后右门
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector WheelRL;//后左门
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector Hood;//引擎盖位置
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector Trunk;//后备箱位置

};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FVehicleMeshParts
{
  GENERATED_BODY();

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> DoorFR;//前右门
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> DoorFL;//前左门
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> DoorRR;//后右门
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> DoorRL;//后左门
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> Trunk;//后备箱
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> Hood;//引擎盖
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> WheelFR;//前右轮
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> WheelFL;//前左轮
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> WheelRR;//后右轮
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> WheelRL;//后左轮
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> Body;车身
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVehicleMeshAnchorPoints Anchors;//部件位置
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<FVehicleLight> Lights;//灯光
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FMergedVehicleMeshParts
{
  GENERATED_BODY();

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* DoorFR;//前右门
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* DoorFL;//前左门
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* DoorRR;//后右门
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* DoorRL;//后左门
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* Trunk;//后备箱
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* Hood;//引擎盖
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* WheelFR;//前右轮
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* WheelFL;//前左轮
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* WheelRR;//后右轮
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* WheelRL;//后左轮
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* Body;车身
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVehicleMeshAnchorPoints Anchors;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<FVehicleLight> Lights;
};

UCLASS()//宏标记该类可以被虚幻引擎的反射系统识别，使其能够在蓝图中使用、序列化等。
class CARLATOOLS_API UUSDImporterWidget : public UUserWidget//意味着这个类可以作为一个用户界面组件在虚幻引擎的编辑器中进行展示和交互，并且可以利用 UUserWidget 类提供的各种用户界面相关的基础功能，如布局管理、事件处理等。
{
  GENERATED_BODY()//宏支持虚幻引擎的代码生成机制，保证类的相关功能（如反射、序列化等）正常实现。

public:

  UFUNCTION(BlueprintCallable, Category="USD Importer")
  void ImportUSDProp(const FString& USDPath, const FString& DestinationAssetPath, bool bAsBlueprint = true);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  void ImportUSDVehicle(
      const FString& USDPath,
      const FString& DestinationAssetPath,
      FWheelTemplates BaseWheelData,
      TArray<FVehicleLight>& LightList,
      FWheelTemplates& WheelObjects,
      bool bAsBlueprint = true);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static AActor* GetGeneratedBlueprint(UWorld* World, const FString& USDPath);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static bool MergeStaticMeshComponents(TArray<AActor*> Actors, const FString& DestMesh);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static TArray<UObject*> MergeMeshComponents(TArray<UPrimitiveComponent*> Components, const FString& DestMesh);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static FVehicleMeshParts SplitVehicleParts(
      AActor* BlueprintActor,
      const TArray<FVehicleLight>& LightList,
      UMaterialInterface* GlassMaterial);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static FMergedVehicleMeshParts GenerateVehicleMeshes(const FVehicleMeshParts& VehicleMeshParts, const FString& DestPath);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static AActor* GenerateNewVehicleBlueprint(
      UWorld* World,
      UClass* BaseClass,
      USkeletalMesh* NewSkeletalMesh,
      UPhysicsAsset* NewPhysicsAsset,
      const FString &DestPath,
      const FMergedVehicleMeshParts& VehicleMeshes,
      const FWheelTemplates& WheelTemplates);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static bool EditSkeletalMeshBones(
      USkeletalMesh* Skeleton,
      const TMap<FString, FTransform> &NewBoneTransforms);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static void CopyCollisionToPhysicsAsset(
      UPhysicsAsset* PhysicsAssetToEdit, UStaticMesh* StaticMesh);
// ImportUSDProp 函数用于将指定路径的通用 USD 资源导入到目标资产路径，并可选择是否创建为蓝图资产。
// @param USDPath 要导入的 USD 资源的文件系统路径，必须是有效的路径字符串，指向一个合法的 USD 文件。
// @param DestinationAssetPath 导入后目标资产在虚幻引擎项目中的存储路径，
// 用于确定资源在项目资产体系中的存放位置，方便后续管理和使用。
// @param bAsBlueprint 可选参数，默认值为 true，表示是否将导入的 USD 资源创建为蓝图资产。
// 如果为 true，则会创建对应的蓝图，便于在蓝图系统中进一步编辑和扩展；如果为 false，则仅导入为普通资产。
// @return 该函数无返回值，主要完成导入操作，导入结果可通过虚幻引擎的资产浏览器等工具查看。
UFUNCTION(BlueprintCallable, Category="USD Importer")
void ImportUSDProp(const FString& USDPath, const FString& DestinationAssetPath, bool bAsBlueprint = true);
};
