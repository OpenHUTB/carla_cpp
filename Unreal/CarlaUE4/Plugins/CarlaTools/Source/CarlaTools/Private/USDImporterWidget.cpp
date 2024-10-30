// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "USDImporterWidget.h"
#ifdef WITH_OMNIVERSE
  #include "USDCARLAInterface.h"
#endif
#include "Kismet/GameplayStatics.h"
#include "Modules/ModuleManager.h"
#include "IMeshMergeUtilities.h"
#include "MeshMergeModule.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Engine/Blueprint.h"
#include "ReferenceSkeleton.h"
#include "Components/SkeletalMeshComponent.h"
#include "PackageHelperFunctions.h"
#include "EditorAssetLibrary.h"
#include <unordered_map>
#include <string>
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "VehicleWheel.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#include "Factories/BlueprintFactory.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintEditor.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "EditorStaticMeshLibrary.h"


void UUSDImporterWidget::ImportUSDProp(
    const FString& USDPath, const FString& DestinationAssetPath, bool bAsBlueprint)
{
#ifdef WITH_OMNIVERSE
  UUSDCARLAInterface::ImportUSD(USDPath, DestinationAssetPath, false, bAsBlueprint);
#else
  UE_LOG(LogCarlaTools, Error, TEXT("Omniverse Plugin is not enabled"));
#endif

}

void UUSDImporterWidget::ImportUSDVehicle(
    const FString& USDPath,
    const FString& DestinationAssetPath,
    FWheelTemplates BaseWheelData,
    TArray<FVehicleLight>& LightList,
    FWheelTemplates& WheelObjects,
    bool bAsBlueprint)
{
#ifdef WITH_OMNIVERSE
  // 导入网格体
  UUSDCARLAInterface::ImportUSD(USDPath, DestinationAssetPath, false, bAsBlueprint);
  //导入灯光
  TArray<FUSDCARLALight> USDLights = UUSDCARLAInterface::GetUSDLights(USDPath);
  LightList.Empty();
  for (const FUSDCARLALight& USDLight : USDLights)
  {
    FVehicleLight Light {USDLight.Name, USDLight.Location, USDLight.Color};
    LightList.Add(Light);
  }
  // 导入车轮及悬架数据。
  TArray<FUSDCARLAWheelData> WheelsData = UUSDCARLAInterface::GetUSDWheelData(USDPath);
  auto CreateVehicleWheel =
      [&](const FUSDCARLAWheelData& WheelData,
         TSubclassOf<UVehicleWheel> TemplateClass,
         const FString &PackagePathName)
      -> TSubclassOf<UVehicleWheel>
  {
    //获取对编辑器子系统的引用。
    constexpr float MToCM = 100.f;
    constexpr float RadToDeg = 360.f/3.14159265359f;
    FString BlueprintName =  FPaths::GetBaseFilename(PackagePathName);
    FString BlueprintPath = FPaths::GetPath(PackagePathName);
    IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
    // 创建一个新的蓝图工厂
    UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
    //为新的蓝图设置父类
    Factory->ParentClass = TemplateClass;
    //使用给定的名称创建一个新的蓝图资产。
    UObject* NewAsset = AssetTools.CreateAsset(BlueprintName, BlueprintPath, UBlueprint::StaticClass(), Factory);
    // 将新创建的资产转换为一个 UBlueprint 类型
    UBlueprint* NewBlueprint = Cast<UBlueprint>(NewAsset);
    //修改新的蓝图。
    NewBlueprint->Modify();
    // 编辑新蓝图的默认对象。
    UVehicleWheel* Result = Cast<UVehicleWheel>(NewBlueprint->GeneratedClass->ClassDefaultObject);
    Result->MaxBrakeTorque = MToCM*WheelData.MaxBrakeTorque;
    if (WheelData.MaxHandBrakeTorque != 0)
    {
      Result->MaxHandBrakeTorque = MToCM*WheelData.MaxHandBrakeTorque;
    }
    Result->SteerAngle = RadToDeg*WheelData.MaxSteerAngle;
    Result->SuspensionMaxDrop = MToCM*WheelData.MaxDroop;
    Result->LatStiffValue = WheelData.LateralStiffnessY;
    Result->LongStiffValue = WheelData.LongitudinalStiffness;
    return Result->GetClass();
  };
  //保存车轮对象。
  FString AssetPath = DestinationAssetPath + FPaths::GetBaseFilename(USDPath);
  FString PathWheelFL = AssetPath + "_Wheel_FLW";
  FString PathWheelFR = AssetPath + "_Wheel_FRW";
  FString PathWheelRL = AssetPath + "_Wheel_RLW";
  FString PathWheelRR = AssetPath + "_Wheel_RRW";
  WheelObjects.WheelFL = CreateVehicleWheel(
      WheelsData[0], BaseWheelData.WheelFL, PathWheelFL);
  WheelObjects.WheelFR = CreateVehicleWheel(
      WheelsData[1], BaseWheelData.WheelFR, PathWheelFR);
  WheelObjects.WheelRL = CreateVehicleWheel(
      WheelsData[2], BaseWheelData.WheelRL, PathWheelRL);
  WheelObjects.WheelRR = CreateVehicleWheel(
      WheelsData[3], BaseWheelData.WheelRR, PathWheelRR);

#else
  UE_LOG(LogCarlaTools, Error, TEXT("Omniverse Plugin is not enabled"));
#endif
}

AActor* UUSDImporterWidget::GetGeneratedBlueprint(UWorld* World, const FString& USDPath)
{
  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
  FString USDFileName = FPaths::GetBaseFilename(USDPath, true);
  UE_LOG(LogCarlaTools, Log, TEXT("Searching for name %s"), *USDFileName);
  for (AActor* Actor : Actors)
  {
    if(Actor->GetName().Contains(USDFileName))
    {
      return Actor;
    }
  }
  return nullptr;
}

bool UUSDImporterWidget::MergeStaticMeshComponents(
    TArray<AActor*> Actors, const FString& DestMesh)
{
  if (Actors.Num() == 0)
  {
    UE_LOG(LogCarlaTools, Error, TEXT("No actors for merge"));
    return false;
  }
  UWorld* World = Actors[0]->GetWorld();
  const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();
  TArray<UPrimitiveComponent*> ComponentsToMerge;
  for(AActor* Actor : Actors)
  {
    TArray<UPrimitiveComponent*> ActorComponents;
    Actor->GetComponents(ActorComponents, false);
    ComponentsToMerge.Append(ActorComponents);
  }
  FMeshMergingSettings MeshMergeSettings;
  TArray<UObject*> AssetsToSync;
  const float ScreenAreaSize = TNumericLimits<float>::Max();
  FVector NewLocation;
  MeshUtilities.MergeComponentsToStaticMesh(ComponentsToMerge, World, MeshMergeSettings, nullptr, nullptr, DestMesh, AssetsToSync, NewLocation, ScreenAreaSize, true);
  return true;
}

TArray<UObject*> UUSDImporterWidget::MergeMeshComponents(
    TArray<UPrimitiveComponent*> ComponentsToMerge,
    const FString& DestMesh)
{
  if(!ComponentsToMerge.Num())
  {
    return {};
  }
  UWorld* World = ComponentsToMerge[0]->GetWorld();
  const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();
  FMeshMergingSettings MeshMergeSettings;
  TArray<UObject*> AssetsToSync;
  const float ScreenAreaSize = TNumericLimits<float>::Max();
  FVector NewLocation;
  MeshUtilities.MergeComponentsToStaticMesh(ComponentsToMerge, World, MeshMergeSettings, nullptr, nullptr, DestMesh, AssetsToSync, NewLocation, ScreenAreaSize, true);
  return AssetsToSync;
}

bool IsChildrenOf(USceneComponent* Component, FString StringInParent)
{
  USceneComponent* CurrentComponent = Component;
  while(CurrentComponent)
  {
    FString ComponentName = UKismetSystemLibrary::GetDisplayName(CurrentComponent);
    if(ComponentName.Contains(StringInParent))
    {
      return true;
    }
    CurrentComponent = CurrentComponent->GetAttachParent();
  }
  return false;
}

FVehicleMeshParts UUSDImporterWidget::SplitVehicleParts(
    AActor* BlueprintActor,
    const TArray<FVehicleLight>& LightList,
    UMaterialInterface* GlassMaterial)
{
  FVehicleMeshParts Result;
  Result.Lights = LightList;
  TArray<UStaticMeshComponent*> MeshComponents;
  BlueprintActor->GetComponents(MeshComponents, false);
  FVector BodyLocation = FVector(0,0,0);
  TArray<UStaticMeshComponent*> GlassComponents;
  for (UStaticMeshComponent* Component : MeshComponents)
  {
    if (!Component->GetStaticMesh())
    {
      continue;
    }
    FString ComponentName = UKismetSystemLibrary::GetDisplayName(Component);
    if (IsChildrenOf(Component, "door_0"))
    {
      Result.DoorFL.Add(Component);
      Result.Anchors.DoorFL = Component->GetComponentTransform().GetLocation();
    }
    else if (IsChildrenOf(Component, "door_1"))
    {
      Result.DoorFR.Add(Component);
      Result.Anchors.DoorFR = Component->GetComponentTransform().GetLocation();
    }
    else if (IsChildrenOf(Component, "door_2"))
    {
      Result.DoorRL.Add(Component);
      Result.Anchors.DoorRL = Component->GetComponentTransform().GetLocation();
    }
    else if (IsChildrenOf(Component, "door_3"))
    {
      Result.DoorRR.Add(Component);
      Result.Anchors.DoorRR = Component->GetComponentTransform().GetLocation();
    }
    else if (ComponentName.Contains("trunk"))
    {
      Result.Trunk.Add(Component);
      Result.Anchors.Trunk = Component->GetComponentTransform().GetLocation();
    }
    else if (ComponentName.Contains("hood"))
    {
      Result.Hood.Add(Component);
      Result.Anchors.Hood = Component->GetComponentTransform().GetLocation();
    }
    else if (IsChildrenOf(Component, "suspension_0"))
    {
      Result.WheelFL.Add(Component);
      if (ComponentName.Contains("wheel"))
      {
        Result.Anchors.WheelFL = Component->GetComponentTransform().GetLocation();
      }
    }
    else if (IsChildrenOf(Component, "suspension_1"))
    {
      Result.WheelFR.Add(Component);
      if (ComponentName.Contains("wheel"))
      {
        Result.Anchors.WheelFR = Component->GetComponentTransform().GetLocation();
      }
    }
    else if (IsChildrenOf(Component, "suspension_2"))
    {
      Result.WheelRL.Add(Component);
      if (ComponentName.Contains("wheel"))
      {
        Result.Anchors.WheelRL = Component->GetComponentTransform().GetLocation();
      }
    }
    else if (IsChildrenOf(Component, "suspension_3"))
    {
      Result.WheelRR.Add(Component);
      if (ComponentName.Contains("wheel"))
      {
        Result.Anchors.WheelRR = Component->GetComponentTransform().GetLocation();
      }
    }
    else if (ComponentName.Contains("Collision"))
    {

    }
    else
    {
      Result.Body.Add(Component);
      if (ComponentName.Contains("body"))
      {
        BodyLocation = Component->GetComponentTransform().GetLocation();
      }
    }

    if(ComponentName.Contains("glass") ||
       IsChildrenOf(Component, "glass"))
    {
      GlassComponents.Add(Component);
    }
  }
  Result.Anchors.DoorFR -= BodyLocation;
  Result.Anchors.DoorFL -= BodyLocation;
  Result.Anchors.DoorRR -= BodyLocation;
  Result.Anchors.DoorRL -= BodyLocation;
  Result.Anchors.WheelFR -= BodyLocation;
  Result.Anchors.WheelFL -= BodyLocation;
  Result.Anchors.WheelRR -= BodyLocation;
  Result.Anchors.WheelRL -= BodyLocation;
  Result.Anchors.Hood -= BodyLocation;
  Result.Anchors.Trunk -= BodyLocation;
  for (FVehicleLight& Light : Result.Lights)
  {
    Light.Location -= BodyLocation;
  }
  // 修复玻璃材质不透明的问题
  for (UStaticMeshComponent* Compopnent : GlassComponents)
  {
    const TArray<UMaterialInterface*>& Materials = Compopnent->GetMaterials();
    for (int32 i = 0; i < Materials.Num(); i++)
    {
      UMaterialInterface* Material = Materials[i];
      if (Material)
      {
        FString MaterialName = Material->GetName();
        if (MaterialName == "DefaultMaterial")
        {
          Compopnent->SetMaterial(i, GlassMaterial);
        }
      }
    }
  }
  return Result;
}

FMergedVehicleMeshParts UUSDImporterWidget::GenerateVehicleMeshes(
    const FVehicleMeshParts& VehicleMeshParts, const FString& DestPath)
{
  FMergedVehicleMeshParts Result;
  auto MergePart =
      [](TArray<UPrimitiveComponent*> Components, const FString& DestMeshPath)
      -> UStaticMesh*
      {
        if (!Components.Num())
        {
          return nullptr;
        }
        TArray<UObject*> Output = MergeMeshComponents(Components, DestMeshPath);
        if (Output.Num())
        {
          return Cast<UStaticMesh>(Output[0]);
        }
        else
        {
          return nullptr;
        }
      };
  Result.DoorFR = MergePart(VehicleMeshParts.DoorFR, DestPath + "_door_fr");
  Result.DoorFL = MergePart(VehicleMeshParts.DoorFL, DestPath + "_door_fl");
  Result.DoorRR = MergePart(VehicleMeshParts.DoorRR, DestPath + "_door_rr");
  Result.DoorRL = MergePart(VehicleMeshParts.DoorRL, DestPath + "_door_rl");
  Result.Trunk = MergePart(VehicleMeshParts.Trunk, DestPath + "_trunk");
  Result.Hood = MergePart(VehicleMeshParts.Hood, DestPath + "_hood");
  Result.WheelFR = MergePart(VehicleMeshParts.WheelFR, DestPath + "_wheel_fr");
  Result.WheelFL = MergePart(VehicleMeshParts.WheelFL, DestPath + "_wheel_fl");
  Result.WheelRR = MergePart(VehicleMeshParts.WheelRR, DestPath + "_wheel_rr");
  Result.WheelRL = MergePart(VehicleMeshParts.WheelRL, DestPath + "_wheel_rl");
  Result.Body = MergePart(VehicleMeshParts.Body, DestPath + "_body");
  Result.Anchors = VehicleMeshParts.Anchors;
  Result.Lights = VehicleMeshParts.Lights;
  return Result;
}

FString GetCarlaLightName(const FString &USDName)
{
  FString LowerCaseUSDName = USDName.ToLower();
  FString LightType = "";
  if (LowerCaseUSDName.Contains("headlight"))
  {
    LightType = "low_beam";
  }
  else if (LowerCaseUSDName.Contains("brakelight"))
  {
    LightType = "brake";
  }
  else if (LowerCaseUSDName.Contains("blinker"))
  {
    LightType = "blinker";
  }
  else if (LowerCaseUSDName.Contains("night"))
  {
    LightType = "high_beam";
  }
  else if (LowerCaseUSDName.Contains("reverse"))
  {
    LightType = "reverse";
  }
  else if (LowerCaseUSDName.Contains("highbeamlight"))
  {
    LightType = "high_beam";
  }
  else if (LowerCaseUSDName.Contains("foglight"))
  {
    LightType = "fog";
  }
  else if (LowerCaseUSDName.Contains("TailLight"))
  {
    LightType = "position";
  }
  else
  {
    LightType = USDName;
  }

  FString FinalName = "-" + LightType + "-";
  if (LowerCaseUSDName.EndsWith("_fr"))
  {
    FinalName = "front" + FinalName + "r-";
  }
  else if (LowerCaseUSDName.EndsWith("_fl"))
  {
    FinalName = "front" + FinalName + "l-";
  }
  else if (LowerCaseUSDName.EndsWith("_rr"))
  {
    FinalName = "back" + FinalName + "r-";
  }
  else if (LowerCaseUSDName.EndsWith("_rl"))
  {
    FinalName = "back" + FinalName + "l-";
  }

  return FinalName;
}

AActor* UUSDImporterWidget::GenerateNewVehicleBlueprint(
    UWorld* World,
    UClass* BaseClass,
    USkeletalMesh* NewSkeletalMesh,
    UPhysicsAsset* NewPhysicsAsset,
    const FString &DestPath, 
    const FMergedVehicleMeshParts& VehicleMeshes,
    const FWheelTemplates& WheelTemplates)
{
  std::unordered_map<std::string, std::pair<UStaticMesh*, FVector>> MeshMap = {
    {"SM_DoorFR", {VehicleMeshes.DoorFR, VehicleMeshes.Anchors.DoorFR}},
    {"SM_DoorFL", {VehicleMeshes.DoorFL, VehicleMeshes.Anchors.DoorFL}},
    {"SM_DoorRR", {VehicleMeshes.DoorRR, VehicleMeshes.Anchors.DoorRR}},
    {"SM_DoorRL", {VehicleMeshes.DoorRL, VehicleMeshes.Anchors.DoorRL}},
    {"Trunk", {VehicleMeshes.Trunk, VehicleMeshes.Anchors.Trunk}},
    {"Hood", {VehicleMeshes.Hood, VehicleMeshes.Anchors.Hood}},
    {"Wheel_FR", {VehicleMeshes.WheelFR, FVector(0,0,0)}},
    {"Wheel_FL", {VehicleMeshes.WheelFL, FVector(0,0,0)}},
    {"Wheel_RR", {VehicleMeshes.WheelRR, FVector(0,0,0)}},
    {"Wheel_RL", {VehicleMeshes.WheelRL, FVector(0,0,0)}},
    {"Body", {VehicleMeshes.Body, FVector(0,0,0)}}
  };

  AActor* TemplateActor = World->SpawnActor<AActor>(BaseClass);
  // 获取并将所有静态网格体替换为合适的网格体
  TArray<UStaticMeshComponent*> MeshComponents;
  TemplateActor->GetComponents(MeshComponents);
  for (UStaticMeshComponent* Component : MeshComponents)
  {
    std::string ComponentName = TCHAR_TO_UTF8(*Component->GetName());
    auto &MapElement = MeshMap[ComponentName];
    UStaticMesh* ComponentMesh = MapElement.first;
    FVector MeshLocation = MapElement.second;
    if(ComponentMesh)
    {
      Component->SetStaticMesh(ComponentMesh);
      Component->SetRelativeLocation(MeshLocation);
    }
    UE_LOG(LogCarlaTools, Log, TEXT("Component name %s, name %s"),
    *UKismetSystemLibrary::GetDisplayName(Component), *Component->GetName());
  }

  // 获取骨骼网格体并且修改以匹配车辆参数
  USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(
      TemplateActor->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
  if(!SkeletalMeshComponent)
  {
    UE_LOG(LogCarlaTools, Log, TEXT("Skeletal mesh component not found"));
    return nullptr;
  }
  USkeletalMesh* SkeletalMesh = SkeletalMeshComponent->SkeletalMesh;
  TMap<FString, FTransform> NewBoneTransform = {
    {"Wheel_Front_Left", FTransform(VehicleMeshes.Anchors.WheelFL)},
    {"Wheel_Front_Right", FTransform(VehicleMeshes.Anchors.WheelFR)},
    {"Wheel_Rear_Right", FTransform(VehicleMeshes.Anchors.WheelRR)},
    {"Wheel_Rear_Left", FTransform(VehicleMeshes.Anchors.WheelRL)}
  };
  if(!SkeletalMesh)
  {
    UE_LOG(LogCarlaTools, Log, TEXT("Mesh not generated, skeletal mesh missing"));
    return nullptr;
  }
  bool bSuccess = EditSkeletalMeshBones(NewSkeletalMesh, NewBoneTransform);
  if (!NewSkeletalMesh || !bSuccess)
  {
    UE_LOG(LogCarlaTools, Log, TEXT("Blueprint generation error"));
    return nullptr;
  }
  SkeletalMeshComponent->SetSkeletalMesh(NewSkeletalMesh);
  UE_LOG(LogCarlaTools, Log, TEXT("Num Lights %d"), VehicleMeshes.Lights.Num());
  for (const FVehicleLight& Light : VehicleMeshes.Lights)
  {
    FString FixedLightName = GetCarlaLightName(Light.Name);
    UClass * LightClass = UPointLightComponent::StaticClass();
    if (FixedLightName.Contains("beam"))
    {
      LightClass = USpotLightComponent::StaticClass();
    }
    ULocalLightComponent * LightComponent = NewObject<ULocalLightComponent>(TemplateActor, LightClass, FName(*FixedLightName));
    LightComponent->RegisterComponent();
    LightComponent->AttachToComponent(
        TemplateActor->GetRootComponent(),
        FAttachmentTransformRules::KeepRelativeTransform);
    LightComponent->SetRelativeLocation(Light.Location); // Set the position of the light relative to the actor
    LightComponent->SetIntensityUnits(ELightUnits::Lumens);
    LightComponent->SetIntensity(5000.f); // Set the brightness of the light
    LightComponent->SetVolumetricScatteringIntensity(0.f);
    if (FixedLightName.Contains("high_beam"))
    {
      USpotLightComponent* SpotLight =
          Cast<USpotLightComponent>(LightComponent);
      SpotLight->SetRelativeRotation(FRotator(-1.5f, 0, 0));
      SpotLight->SetAttenuationRadius(5000.f);
      SpotLight->SetInnerConeAngle(20.f);
      SpotLight->SetOuterConeAngle(30.f);
      LightComponent->SetIntensity(150000.f); // Set the brightness of the light
      LightComponent->SetVolumetricScatteringIntensity(0.025f);
    }
    else if (FixedLightName.Contains("low_beam"))
    {
      USpotLightComponent* SpotLight =
          Cast<USpotLightComponent>(LightComponent);
      LightComponent->SetRelativeRotation(FRotator(-3.f, 0, 0));
      LightComponent->SetAttenuationRadius(3000.f);
      SpotLight->SetInnerConeAngle(50.f);
      SpotLight->SetOuterConeAngle(65.f);
      LightComponent->SetIntensity(15000.f); // Set the brightness of the light
      LightComponent->SetVolumetricScatteringIntensity(0.025f);
    }
    LightComponent->SetLightColor(Light.Color);
    TemplateActor->AddInstanceComponent(LightComponent);
    UE_LOG(LogCarlaTools, Log, TEXT("Spawn Light %s, %s, %s"), *Light.Name, *Light.Location.ToString(), *Light.Color.ToString());
  }
  // 设置车轮半径
  UVehicleWheel* WheelDefault;
  WheelDefault = WheelTemplates.WheelFL->GetDefaultObject<UVehicleWheel>();
  WheelDefault->ShapeRadius = VehicleMeshes.WheelFL->GetBounds().SphereRadius;
  WheelDefault = WheelTemplates.WheelFR->GetDefaultObject<UVehicleWheel>();
  WheelDefault->ShapeRadius = VehicleMeshes.WheelFR->GetBounds().SphereRadius;
  WheelDefault = WheelTemplates.WheelRL->GetDefaultObject<UVehicleWheel>();
  WheelDefault->ShapeRadius = VehicleMeshes.WheelRL->GetBounds().SphereRadius;
  WheelDefault = WheelTemplates.WheelRR->GetDefaultObject<UVehicleWheel>();
  WheelDefault->ShapeRadius = VehicleMeshes.WheelRR->GetBounds().SphereRadius;
  // 分配生成的车轮类型
  TArray<FWheelSetup> WheelSetups;
  FWheelSetup Setup;
  Setup.WheelClass = WheelTemplates.WheelFL;
  Setup.BoneName = "Wheel_Front_Left";
  WheelSetups.Add(Setup);
  Setup.WheelClass = WheelTemplates.WheelFR;
  Setup.BoneName = "Wheel_Front_Right";
  WheelSetups.Add(Setup);
  Setup.WheelClass = WheelTemplates.WheelRL;
  Setup.BoneName = "Wheel_Rear_Left";
  WheelSetups.Add(Setup);
  Setup.WheelClass = WheelTemplates.WheelRR;
  Setup.BoneName = "Wheel_Rear_Right";
  WheelSetups.Add(Setup);
  ACarlaWheeledVehicle* CarlaVehicle =
      Cast<ACarlaWheeledVehicle>(TemplateActor);
  if(CarlaVehicle)
  {
    UWheeledVehicleMovementComponent4W* MovementComponent =
        Cast<UWheeledVehicleMovementComponent4W>(
            CarlaVehicle->GetVehicleMovementComponent());
    MovementComponent->WheelSetups = WheelSetups;
  }
  else
  {
    UE_LOG(LogCarlaTools, Error, TEXT("Null CarlaVehicle"));
  }
  // 在新的物理资产里对象设置车辆碰撞
  UEditorStaticMeshLibrary::AddSimpleCollisions(
      VehicleMeshes.Body, EScriptingCollisionShapeType::NDOP26);
  CopyCollisionToPhysicsAsset(NewPhysicsAsset, VehicleMeshes.Body);
  // 将物理资产分配给骨骼网格体。
  NewSkeletalMesh->PhysicsAsset = NewPhysicsAsset;
  // 创建新的车辆蓝图。
  FKismetEditorUtilities::FCreateBlueprintFromActorParams Params;
  Params.bReplaceActor = false;
  Params.bKeepMobility = true;
  Params.bDeferCompilation = false;
  Params.bOpenBlueprint = false;
  Params.ParentClassOverride = BaseClass;
  FKismetEditorUtilities::CreateBlueprintFromActor(
      DestPath,
      TemplateActor,
      Params);
  return nullptr;
}

bool UUSDImporterWidget::EditSkeletalMeshBones(
    USkeletalMesh* NewSkeletalMesh,
    const TMap<FString, FTransform> &NewBoneTransforms)
{
  if(!NewSkeletalMesh)
  {
    UE_LOG(LogCarlaTools, Log, TEXT("Skeletal mesh invalid"));
    return false;
  }
  FReferenceSkeleton& ReferenceSkeleton = NewSkeletalMesh->RefSkeleton;
  FReferenceSkeletonModifier SkeletonModifier(ReferenceSkeleton, NewSkeletalMesh->Skeleton);
  for (auto& Element : NewBoneTransforms)
  {
    const FString& BoneName = Element.Key;
    const FTransform& BoneTransform = Element.Value;
    int32 BoneIdx = SkeletonModifier.FindBoneIndex(FName(*BoneName));
    if (BoneIdx == INDEX_NONE)
    {
      UE_LOG(LogCarlaTools, Log, TEXT("Bone %s not found"), *BoneName);
    }
    UE_LOG(LogCarlaTools, Log, TEXT("Bone %s corresponds to index %d"), *BoneName, BoneIdx);
    SkeletonModifier.UpdateRefPoseTransform(BoneIdx, BoneTransform);
  }

  NewSkeletalMesh->MarkPackageDirty();
  UPackage* Package = NewSkeletalMesh->GetOutermost();
  return UPackage::SavePackage(
      Package, NewSkeletalMesh,
      EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *(Package->GetName()), GError, nullptr, true, true, SAVE_NoError);
}

void UUSDImporterWidget::CopyCollisionToPhysicsAsset(
    UPhysicsAsset* PhysicsAssetToEdit, UStaticMesh* StaticMesh)
{
  UE_LOG(LogCarlaTools, Log, TEXT("Num bodysetups %d"), PhysicsAssetToEdit->SkeletalBodySetups.Num());
  UBodySetup* BodySetupPhysicsAsset = 
      PhysicsAssetToEdit->SkeletalBodySetups[
          PhysicsAssetToEdit->FindBodyIndex(FName("Vehicle_Base"))];
  UBodySetup* BodySetupStaticMesh = StaticMesh->BodySetup;
  BodySetupPhysicsAsset->AggGeom = BodySetupStaticMesh->AggGeom;

}
