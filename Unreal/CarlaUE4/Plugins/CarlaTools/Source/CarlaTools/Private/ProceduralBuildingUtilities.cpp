// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ProceduralBuildingUtilities.h"

#include "AssetRegistryModule.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "IMeshMergeUtilities.h"
#include "Materials/MaterialInstanceConstant.h"
#include "MeshMergeModule.h"
#include "ProceduralMeshComponent.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

void AProceduralBuildingUtilities::GenerateImpostorTexture(const FVector& BuildingSize)
{
  USceneCaptureComponent2D* Camera = NewObject<USceneCaptureComponent2D>(
      this, USceneCaptureComponent2D::StaticClass(), TEXT("ViewProjectionCaptureComponent"));

  Camera->AttachToComponent(
      GetRootComponent(), 
      FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
      FName("ViewProjectionCaptureComponentName"));
  AddInstanceComponent(Camera);
  Camera->OnComponentCreated();
  Camera->RegisterComponent();

  check(Camera!=nullptr);

  Camera->ProjectionType = ECameraProjectionMode::Orthographic;
  Camera->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
  Camera->CaptureSource = ESceneCaptureSource::SCS_BaseColor;

  if(Camera->ShowOnlyActors.Num() == 0)
  {
    Camera->ShowOnlyActors.Add(this);
  }

  SetTargetTextureToSceneCaptureComponent(Camera);

  check(Camera->TextureTarget != nullptr);

  // 前视图
  RenderImpostorView(Camera, BuildingSize, EBuildingCameraView::FRONT);
  // 左视图
  RenderImpostorView(Camera, BuildingSize, EBuildingCameraView::LEFT);
  // 后视图
  RenderImpostorView(Camera, BuildingSize, EBuildingCameraView::BACK);
  // 右视图
  RenderImpostorView(Camera, BuildingSize, EBuildingCameraView::RIGHT);

  Camera->DestroyComponent();
}

UProceduralMeshComponent* AProceduralBuildingUtilities::GenerateImpostorGeometry(const FVector& BuildingSize)
{
  //生成程序网格体 actor（角色或参与者，在游戏开发等情境下可理解为具有特定行为和属性的游戏元素）/组件。
  UProceduralMeshComponent* Mesh = NewObject<UProceduralMeshComponent>(
      this, UProceduralMeshComponent::StaticClass(), TEXT("LOD Impostor Mesh"));

  Mesh->AttachToComponent(
      GetRootComponent(), 
      FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
      FName("LOD Impostor Mesh"));
  AddInstanceComponent(Mesh);
  Mesh->OnComponentCreated();
  Mesh->RegisterComponent();

  check(Mesh != nullptr)

  //前视图
  CreateBuildingImpostorGeometryForView(Mesh, BuildingSize, EBuildingCameraView::FRONT);
  // 左视图
  CreateBuildingImpostorGeometryForView(Mesh, BuildingSize, EBuildingCameraView::LEFT);
  //  后视图
  CreateBuildingImpostorGeometryForView(Mesh, BuildingSize, EBuildingCameraView::BACK);
  //右视图
  CreateBuildingImpostorGeometryForView(Mesh, BuildingSize, EBuildingCameraView::RIGHT);

  return Mesh;

  // 将新的网格体烘焙为静态网格体。

  //指定为细节层次级别（LOD），或者存储在磁盘上以便手动导入。
}

void AProceduralBuildingUtilities::CookProceduralBuildingToMesh(const FString& DestinationPath, const FString& FileName)
{
  TArray<UPrimitiveComponent*> Components;
  TArray<UStaticMeshComponent*> StaticMeshComponents;
  GetComponents<UStaticMeshComponent>(StaticMeshComponents, false); // 类的组件
  for(UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
  {
    if(StaticMeshComponent->GetStaticMesh())
    {
      Components.Add(StaticMeshComponent);
    }
  }

  UWorld* World = GetWorld();

  FMeshMergingSettings MeshMergeSettings;
  TArray<UObject*> AssetsToSync;

  FVector NewLocation;
  const float ScreenAreaSize = TNumericLimits<float>::Max();

  FString PackageName = DestinationPath + FileName;
  UPackage* NewPackage = CreatePackage(*PackageName);
  check(NewPackage);

  const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();
  MeshUtilities.MergeComponentsToStaticMesh(
      Components,
      World,
      MeshMergeSettings,
      nullptr,
      NewPackage,
      FileName,
      AssetsToSync,
      NewLocation,
      ScreenAreaSize,
      true);

  UPackage::SavePackage(NewPackage,
      AssetsToSync[0],
      EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *FileName,
      GError,
      nullptr,
      true,
      true,
      SAVE_NoError);
}

void AProceduralBuildingUtilities::CookProceduralMeshToMesh(
    UProceduralMeshComponent* Mesh,
    const FString& DestinationPath,
    const FString& FileName)
{
  FMeshDescription MeshDescription = BuildMeshDescription(Mesh);

  FString PackageName = DestinationPath + FileName;
  UPackage* NewPackage = CreatePackage(*PackageName);
  check(NewPackage);

  UStaticMesh* StaticMesh = NewObject<UStaticMesh>(NewPackage, *FileName, RF_Public | RF_Standalone);
  StaticMesh->InitResources();
  StaticMesh->LightingGuid = FGuid::NewGuid();

  FStaticMeshSourceModel& SrcModel = StaticMesh->AddSourceModel();
  SrcModel.BuildSettings.bRecomputeNormals = false;
  SrcModel.BuildSettings.bRecomputeTangents = false;
  SrcModel.BuildSettings.bRemoveDegenerates = false;
  SrcModel.BuildSettings.bUseHighPrecisionTangentBasis = false;
  SrcModel.BuildSettings.bUseFullPrecisionUVs = false;
  SrcModel.BuildSettings.bGenerateLightmapUVs = true;
  SrcModel.BuildSettings.SrcLightmapIndex = 0;
  SrcModel.BuildSettings.DstLightmapIndex = 1;
  StaticMesh->CreateMeshDescription(0, MoveTemp(MeshDescription));
  StaticMesh->CommitMeshDescription(0);

  if (!Mesh->bUseComplexAsSimpleCollision )
  {
    StaticMesh->CreateBodySetup();
    UBodySetup* NewBodySetup = StaticMesh->BodySetup;
    NewBodySetup->BodySetupGuid = FGuid::NewGuid();
    NewBodySetup->AggGeom.ConvexElems = Mesh->ProcMeshBodySetup->AggGeom.ConvexElems;
    NewBodySetup->bGenerateMirroredCollision = false;
    NewBodySetup->bDoubleSidedGeometry = true;
    NewBodySetup->CollisionTraceFlag = CTF_UseDefault;
    NewBodySetup->CreatePhysicsMeshes();
  }

  TSet<UMaterialInterface*> UniqueMaterials;
  const int32 NumSections = Mesh->GetNumSections();
  for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
  {
    FProcMeshSection *ProcSection =
      Mesh->GetProcMeshSection(SectionIdx);
    UMaterialInterface *Material = Mesh->GetMaterial(SectionIdx);
    UniqueMaterials.Add(Material);
  }

  for (auto* Material : UniqueMaterials)
  {
    StaticMesh->StaticMaterials.Add(FStaticMaterial(Material));
  }

  StaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;
  StaticMesh->Build(false);
  StaticMesh->PostEditChange();
  FAssetRegistryModule::AssetCreated(StaticMesh);

  UPackage::SavePackage(NewPackage,
      StaticMesh,
      EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *FileName,
      GError,
      nullptr,
      true,
      true,
      SAVE_NoError);
}

UMaterialInstanceConstant* AProceduralBuildingUtilities::GenerateBuildingMaterialAsset(
    const FString& DuplicateParentPath,
    const FString& DestinationPath,
    const FString& FileName)
{
  const FString BaseMaterialSearchPath = DuplicateParentPath;
  const FString PackageName = DestinationPath + FileName;

  UMaterialInstanceConstant* ParentMaterial = LoadObject<UMaterialInstanceConstant>(nullptr, *BaseMaterialSearchPath);

  check(ParentMaterial != nullptr);

  UPackage* NewPackage = CreatePackage(*PackageName);
  FObjectDuplicationParameters Parameters(ParentMaterial, NewPackage);
  Parameters.DestName = FName(FileName);
  Parameters.DestClass = ParentMaterial->GetClass();
  Parameters.DuplicateMode = EDuplicateMode::Normal;
  Parameters.PortFlags = PPF_Duplicate;

  UMaterialInstanceConstant* DuplicatedMaterial = CastChecked<UMaterialInstanceConstant>(StaticDuplicateObjectEx(Parameters));

  const FString PackageFileName = FPackageName::LongPackageNameToFilename(
      PackageName, 
      ".uasset");
  UPackage::SavePackage(NewPackage, DuplicatedMaterial, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName, GError, nullptr, true, true, SAVE_NoError);

  return DuplicatedMaterial;
}

void AProceduralBuildingUtilities::RenderImpostorView(
    USceneCaptureComponent2D* Camera,
    const FVector BuildingSize,
    const EBuildingCameraView View)
{
  MoveCameraToViewPosition(Camera, BuildingSize, View);
  Camera->CaptureScene();
  UTextureRenderTarget2D* CameraRT = Camera->TextureTarget;
  UTexture2D* ViewTexture = CameraRT->ConstructTexture2D(GetWorld(), FString("View_").Append(FString::FromInt(View)), EObjectFlags::RF_NoFlags );
  BakeSceneCaptureRTToTextureAtlas(ViewTexture, View);
}

void AProceduralBuildingUtilities::MoveCameraToViewPosition(
    USceneCaptureComponent2D* Camera,
    const FVector BuildingSize,
    const EBuildingCameraView View)
{
  const float BuildingHeight = BuildingSize.Z;
  float ViewAngle = 0.f;
  float BuildingWidth = 0.f;
  float BuildingDepth = 0.f;

  GetWidthDepthFromView(BuildingSize, View, BuildingWidth, BuildingDepth, ViewAngle);

  /* ORTHO */
  float ViewOrthoWidth = FMath::Max(BuildingWidth, BuildingHeight);
  Camera->OrthoWidth = ViewOrthoWidth;

  float CameraDistance = 0.5f * BuildingDepth + 1000.f;

  FVector NewCameraLocation(
      CameraDistance * FMath::Cos(FMath::DegreesToRadians(ViewAngle)),
      CameraDistance * FMath::Sin(FMath::DegreesToRadians(ViewAngle)),
      BuildingSize.Z / 2.0f);

  FRotator NewCameraRotation(0.0f, ViewAngle + 180.f, 0.0f);

  Camera->SetRelativeLocationAndRotation(NewCameraLocation, NewCameraRotation, false, nullptr, ETeleportType::None);
}

void AProceduralBuildingUtilities::CreateBuildingImpostorGeometryForView(
    UProceduralMeshComponent* Mesh,
    const FVector& BuildingSize,
    const EBuildingCameraView View)
{
  // 根据建筑规模创建顶点。
  TArray<FVector> Vertices;
  TArray<int32> Triangles;

  const float BuildingHeight = BuildingSize.Z;
  float BuildingWidth = 0.f;
  float BuildingDepth = 0.f;
  float ViewAngle = 0.f;

  GetWidthDepthFromView(BuildingSize, View, BuildingWidth, BuildingDepth, ViewAngle);

  FVector RotationAxis(0.0f, 0.0f, 1.0f);
  FVector OriginOffset(- 0.5f * BuildingDepth, - 0.5f * BuildingWidth, 0.0f);

  //顶点是在局部空间中创建的，然后进行偏移以补偿原点位置，最后进行旋转操作。
  //根据视角
  Vertices.Add((FVector(0.0f, 0.0f, 0.0f)                     + OriginOffset).RotateAngleAxis(ViewAngle, RotationAxis));
  Vertices.Add((FVector(0.0f, 0.0f, BuildingHeight)           + OriginOffset).RotateAngleAxis(ViewAngle, RotationAxis));
  Vertices.Add((FVector(0.0f, BuildingWidth, 0.0f)            + OriginOffset).RotateAngleAxis(ViewAngle, RotationAxis));
  Vertices.Add((FVector(0.0f, BuildingWidth, BuildingHeight)  + OriginOffset).RotateAngleAxis(ViewAngle, RotationAxis));

  Triangles.Add(0); Triangles.Add(2); Triangles.Add(1); 
  Triangles.Add(2); Triangles.Add(3); Triangles.Add(1);

  TArray<FVector2D> UVs;
  CalculateViewGeometryUVs(BuildingWidth, BuildingHeight, View, UVs);

  TArray<FVector> Normals;
  Normals.Init(FVector(-1.0f, 0.0f, 0.0f).RotateAngleAxis(ViewAngle, RotationAxis), Vertices.Num());

  TArray<FLinearColor> Colors;
  Colors.Init(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f), Vertices.Num());

  TArray<FProcMeshTangent> Tangents;
  // Tangents.Init(FProcMeshTangent(-1.0f, -1.0f, -1.0f).RotateAngleAxis(ViewAngle, RotationAxis), Vertices.Num());
  Tangents.Init(FProcMeshTangent(FVector(0.0f, 1.0f, 0.0f).RotateAngleAxis(ViewAngle, RotationAxis), false), Vertices.Num());

  Mesh->CreateMeshSection_LinearColor((int32)View, Vertices, Triangles, Normals, UVs, Colors, Tangents, false);
}

void AProceduralBuildingUtilities::GetWidthDepthFromView(
    const FVector& BuildingSize,
    const EBuildingCameraView View,
    float& OutWidth,
    float& OutDepth,
    float& OutViewAngle)
{
  switch(View)
  {
    case EBuildingCameraView::FRONT:
      OutViewAngle = 0.0f;
      OutWidth = FMath::Abs(BuildingSize.Y);
      OutDepth = FMath::Abs(BuildingSize.X);
      break;

    case EBuildingCameraView::LEFT:
      OutViewAngle = 90.0f;
      OutWidth = FMath::Abs(BuildingSize.X);
      OutDepth = FMath::Abs(BuildingSize.Y);
      break;

    case EBuildingCameraView::BACK:
      OutViewAngle = 180.0f;
      OutWidth = FMath::Abs(BuildingSize.Y);
      OutDepth = FMath::Abs(BuildingSize.X);
      break;

    case EBuildingCameraView::RIGHT:
      OutViewAngle = 270.0f;
      OutWidth = FMath::Abs(BuildingSize.X);
      OutDepth = FMath::Abs(BuildingSize.Y);
      break;

    default:
      OutViewAngle = 0.0f;
      OutWidth = 0.0f;
      OutDepth = 0.0f;
  }
}

void AProceduralBuildingUtilities::CalculateViewGeometryUVs(
    const float BuildingWidth,
    const float BuildingHeight,
    const EBuildingCameraView View,
    TArray<FVector2D>& OutUVs)
{
  // 计算从 0 到 1 的 UV 坐标。
  //  ------------
  // |  uv1  uv3  |
  // |  uv0  uv2  |
  //  ------------

  FVector2D OriginOffset;
  switch(View)
  {
    case EBuildingCameraView::FRONT:
      OriginOffset = FVector2D(0.3f, 0.3f);
      break;

    case EBuildingCameraView::LEFT:
      OriginOffset = FVector2D(0.3f, 0.4f);
      break;

    case EBuildingCameraView::BACK:
      OriginOffset = FVector2D(0.4f, 0.3f);
      break;

    case EBuildingCameraView::RIGHT:
      OriginOffset = FVector2D(0.4f, 0.4f);
      break;

    default:
      OriginOffset = FVector2D(0.0f, 0.0f);
  }

  float WidthRatio = BuildingWidth / BuildingHeight;
  float HeightRatio = BuildingHeight / BuildingWidth;

  FVector2D X0, X1, X2, X3;

  if(WidthRatio < 1.0f)
  {
    //垂直适配；纵向适配
    X0 = FVector2D(0.05f - 0.05f * WidthRatio, 0.1f) + OriginOffset;
    X1 = FVector2D(0.05f - 0.05f * WidthRatio, 0.0f) + OriginOffset;
    X2 = FVector2D(0.05f + 0.05f * WidthRatio, 0.1f) + OriginOffset;
    X3 = FVector2D(0.05f + 0.05f * WidthRatio, 0.0f) + OriginOffset;
  }
  else
  {
    //水平适配；横向适配
    X0 = FVector2D(0.0f, 0.05f + 0.05f * HeightRatio) + OriginOffset;
    X1 = FVector2D(0.0f, 0.05f - 0.05f * HeightRatio) + OriginOffset;
    X2 = FVector2D(0.1f, 0.05f + 0.05f * HeightRatio) + OriginOffset;
    X3 = FVector2D(0.1f, 0.05f - 0.05f * HeightRatio) + OriginOffset;
  }

  // 修正偏移量以使其适配于视图象限
  OutUVs.Add(X0);
  OutUVs.Add(X1);
  OutUVs.Add(X2);
  OutUVs.Add(X3);
}
