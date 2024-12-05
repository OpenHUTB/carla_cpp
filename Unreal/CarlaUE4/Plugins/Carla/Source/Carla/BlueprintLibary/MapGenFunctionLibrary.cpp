// 版权所有 (c) 2023 巴塞罗那自治大学 (UAB) 计算机视觉中心 (CVC)。
//
// 本作品根据 MIT 许可证的条款授权。
// 如需副本，请访问 <https://opensource.org/licenses/MIT>。

#include "MapGenFunctionLibrary.h"

// 引擎头文件
#include "AssetRegistry/AssetRegistryModule.h"
#include "Materials/MaterialInstance.h"
#include "StaticMeshAttributes.h"
#include "RenderingThread.h"
// Carla C++ 头文件

// Carla 插件头文件

// 定义日志类别
DEFINE_LOG_CATEGORY(LogCarlaMapGenFunctionLibrary);
// 定义 OSM 到厘米的比例因子
static const float OSMToCentimetersScaleFactor = 100.0f;

// 从给定数据构建网格描述
FMeshDescription UMapGenFunctionLibrary::BuildMeshDescriptionFromData(
  const FProceduralCustomMesh& Data,
  const TArray<FProcMeshTangent>& ParamTangents,
  UMaterialInstance* MaterialInstance  )
{
  // 获取顶点数量
  int32 VertexCount = Data.Vertices.Num();
  // 获取顶点实例数量
  int32 VertexInstanceCount = Data.Triangles.Num();
  // 计算多边形数量
  int32 PolygonCount = Data.Vertices.Num()/3;

  // 创建网格描述对象
  FMeshDescription MeshDescription;
  // 获取静态网格属性
  FStaticMeshAttributes AttributeGetter(MeshDescription);
  // 注册属性
  AttributeGetter.Register();

  // 获取各种网格属性引用
  TPolygonGroupAttributesRef<FName> PolygonGroupNames = AttributeGetter.GetPolygonGroupMaterialSlotNames();
  TVertexAttributesRef<FVector> VertexPositions = AttributeGetter.GetVertexPositions();
  TVertexInstanceAttributesRef<FVector> Tangents = AttributeGetter.GetVertexInstanceTangents();
  TVertexInstanceAttributesRef<float> BinormalSigns = AttributeGetter.GetVertexInstanceBinormalSigns();
  TVertexInstanceAttributesRef<FVector> Normals = AttributeGetter.GetVertexInstanceNormals();
  TVertexInstanceAttributesRef<FVector4> Colors = AttributeGetter.GetVertexInstanceColors();
  TVertexInstanceAttributesRef<FVector2D> UVs = AttributeGetter.GetVertexInstanceUVs();

  // 为网格描述预留空间
  FPolygonGroupID PolygonGroupForSection;
  MeshDescription.ReserveNewVertices(VertexCount);
  MeshDescription.ReserveNewVertexInstances(VertexInstanceCount);
  MeshDescription.ReserveNewPolygons(PolygonCount);
  MeshDescription.ReserveNewEdges(PolygonCount * 2);
  UVs.SetNumIndices(4);

  // 创建材质
  TMap<UMaterialInterface*, FPolygonGroupID> UniqueMaterials;
  const int32 NumSections = 1;
  UniqueMaterials.Reserve(1);
  FPolygonGroupID NewPolygonGroup = MeshDescription.CreatePolygonGroup();

  if( MaterialInstance != nullptr ){
    UMaterialInterface *Material = MaterialInstance;
    UniqueMaterials.Add(Material, NewPolygonGroup);
    PolygonGroupNames[NewPolygonGroup] = Material->GetFName();
  }else{
    UE_LOG(LogCarla, Error, TEXT("MaterialInstance is nullptr"));
  }
  PolygonGroupForSection = NewPolygonGroup;

  // 创建模型顶点
  int32 NumVertex = Data.Vertices.Num();
  TMap<int32, FVertexID> VertexIndexToVertexID;
  VertexIndexToVertexID.Reserve(NumVertex);
  for (int32 VertexIndex = 0; VertexIndex < NumVertex; ++VertexIndex)
  {
    const FVector &Vert = Data.Vertices[VertexIndex];
    const FVertexID VertexID = MeshDescription.CreateVertex();
    VertexPositions[VertexID] = Vert;
    VertexIndexToVertexID.Add(VertexIndex, VertexID);
  }

  // 创建顶点实例
  int32 NumIndices = Data.Triangles.Num();
  int32 NumTri = NumIndices / 3;
  TMap<int32, FVertexInstanceID> IndiceIndexToVertexInstanceID;
  IndiceIndexToVertexInstanceID.Reserve(NumVertex);
  for (int32 IndiceIndex = 0; IndiceIndex < NumIndices; IndiceIndex++)
  {
    const int32 VertexIndex = Data.Triangles[IndiceIndex];
    const FVertexID VertexID = VertexIndexToVertexID[VertexIndex];
    const FVertexInstanceID VertexInstanceID = MeshDescription.CreateVertexInstance(VertexID);
    IndiceIndexToVertexInstanceID.Add(IndiceIndex, VertexInstanceID);
    Normals[VertexInstanceID] = Data.Normals[VertexIndex];

    if(ParamTangents.Num() == Data.Vertices.Num())
    {
      Tangents[VertexInstanceID] = ParamTangents[VertexIndex].TangentX;
      BinormalSigns[VertexInstanceID] = ParamTangents[VertexIndex].bFlipTangentY ? -1.f : 1.f;
    }
    Colors[VertexInstanceID] = FLinearColor(0,0,0);
    if(Data.UV0.Num() == Data.Vertices.Num())
    {
      UVs.Set(VertexInstanceID, 0, Data.UV0[VertexIndex]);
    }else{
      UVs.Set(VertexInstanceID, 0, FVector2D(0,0));
    }
    UVs.Set(VertexInstanceID, 1, FVector2D(0,0));
    UVs.Set(VertexInstanceID, 2, FVector2D(0,0));
    UVs.Set(VertexInstanceID, 3, FVector2D(0,0));
  }

  // 创建多边形
  for (int32 TriIdx = 0; TriIdx < NumTri; TriIdx++)
  {
    FVertexID VertexIndexes[3];
    TArray<FVertexInstanceID> VertexInstanceIDs;
    VertexInstanceIDs.SetNum(3);

    for (int32 CornerIndex = 0; CornerIndex < 3; ++CornerIndex)
    {
      const int32 IndiceIndex = (TriIdx * 3) + CornerIndex;
      const int32 VertexIndex = Data.Triangles[IndiceIndex];
      VertexIndexes[CornerIndex] = VertexIndexToVertexID[VertexIndex];
      VertexInstanceIDs[CornerIndex] = IndiceIndexToVertexInstanceID[IndiceIndex];
    }

    // 将一个多边形插入到网格中
    MeshDescription.CreatePolygon(NewPolygonGroup, VertexInstanceIDs);
  }

  return MeshDescription;
}

// 创建静态网格
UStaticMesh* UMapGenFunctionLibrary::CreateMesh(
    const FProceduralCustomMesh& Data,
    const TArray<FProcMeshTangent>& ParamTangents,
    UMaterialInstance* MaterialInstance,
    FString MapName,
    FString FolderName,
    FName MeshName)
{
  IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

  UStaticMesh::FBuildMeshDescriptionsParams Params;
  Params.bBuildSimpleCollision = true;

  FString PackageName = "/Game/CustomMaps/" + MapName + "/Static/" + FolderName + "/" + MeshName.ToString();

  if (!PlatformFile.DirectoryExists(*PackageName))
  {
    //PlatformFile.CreateDirectory(*PackageName);
  }

  FMeshDescription Description = BuildMeshDescriptionFromData(Data,ParamTangents, MaterialInstance);

  if (Description.Polygons().Num() > 0)
  {
    UPackage* Package = CreatePackage(*PackageName);
    check(Package);
    UStaticMesh* Mesh = NewObject<UStaticMesh>( Package, MeshName, RF_Public | RF_Standalone);

    Mesh->InitResources();

    Mesh->LightingGuid = FGuid::NewGuid();
    Mesh->StaticMaterials.Add(FStaticMaterial(MaterialInstance));
    Mesh->BuildFromMeshDescriptions({ &Description }, Params);
    Mesh->CreateBodySetup();
    Mesh->BodySetup->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;
    Mesh->BodySetup->CreatePhysicsMeshes();
    // 从源构建网格
    Mesh->NeverStream = false;
    TArray<UObject*> CreatedAssets;
    CreatedAssets.Add(Mesh);

    // 通知资产注册表新资产
    FAssetRegistryModule::AssetCreated(Mesh);
    //UPackage::SavePackage(Package, Mesh, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *(MeshName.ToString()), GError, nullptr, true, true, SAVE_NoError);
    Package->MarkPackageDirty();
    return Mesh;
  }
  return nullptr;
}

// 获取横轴墨卡托投影
FVector2D UMapGenFunctionLibrary::GetTransversemercProjection(float lat, float lon, float lat0, float lon0)
{
  // 地球半径（米）
  const float R = 6373000.0f;
  float latt = FMath::DegreesToRadians(lat);
  float lonn  = FMath::DegreesToRadians(lon - lon0);
  float latt0 = FMath::DegreesToRadians(lat0);
  float eps = atan(tan(latt)/cos(lonn));
  float nab = asinh(sin(lonn)/sqrt(tan(latt)*tan(latt)+cos(lonn)*cos(lonn)));
  float x = R*nab;
  float y = R*eps;
  float eps0 = atan(tan(latt0)/cos(0));
  float nab0 = asinh(sin(0)/sqrt(tan(latt0)*tan(latt0)+cos(0)*cos(0)));
  float x0 = R*nab0;
  float y0 = R*eps0;
  FVector2D Result = FVector2D(x, -(y - y0)) * OSMToCentimetersScaleFactor;

  return Result;
}

// 使线程休眠
void UMapGenFunctionLibrary::SetThreadToSleep(float seconds){
  //FGenericPlatformProcess::Sleep(seconds);
}

// 在蓝图中刷新渲染命令
void UMapGenFunctionLibrary::FlushRenderingCommandsInBlueprint(){
  FlushRenderingCommands(true);
  FlushPendingDeleteRHIResources_GameThread();
}

// 清理GEngine
void UMapGenFunctionLibrary::CleanupGEngine(){
  GEngine->PerformGarbageCollectionAndCleanupActors();
#if WITH_EDITOR
  FText TransResetText(FText::FromString("Clean up after Move actors to sublevels"));
  if ( GEditor->Trans )
  {
    GEditor->Trans->Reset(TransResetText);
    GEditor->Cleanse(true, true, TransResetText);
  }
#endif
}