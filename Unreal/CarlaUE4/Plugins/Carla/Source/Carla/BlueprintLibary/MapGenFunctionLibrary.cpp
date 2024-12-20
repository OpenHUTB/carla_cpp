// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MapGenFunctionLibrary.h"

// 引擎头文件
#include "AssetRegistry/AssetRegistryModule.h"
#include "Materials/MaterialInstance.h"
#include "StaticMeshAttributes.h"
#include "RenderingThread.h"
// Carla C++ 头文件

// Carla 插件头文件
// 定义一个日志类别，名为LogCarlaMapGenFunctionLibrary。
// 这个类别在程序运行过程中可用于输出特定的日志信息，便于开发者调试代码以及追踪与Carla地图生成功能库相关的执行情况、错误等内容，方便定位问题所在。
DEFINE_LOG_CATEGORY(LogCarlaMapGenFunctionLibrary);
// 声明一个静态的常量，类型为单精度浮点数（float），用于表示从OpenStreetMap（OSM）单位到厘米单位的缩放因子。
// 这里将其值设定为100.0f，意思是在涉及到OSM相关数据转换为厘米单位的操作时，每1个OSM单位对应100厘米，方便后续进行长度、距离等相关的换算。
static const float OSMToCentimetersScaleFactor = 100.0f;

// 这是一个类UMapGenFunctionLibrary的成员函数BuildMeshDescriptionFromData的定义。
// 该函数的目的是根据传入的不同参数构建一个FMeshDescription类型的网格描述信息，这个网格描述会包含诸如顶点、三角形、材质等构建一个完整网格所需要的各种元素信息，下面是具体的函数参数和函数体逻辑。

FMeshDescription UMapGenFunctionLibrary::BuildMeshDescriptionFromData(
  const FProceduralCustomMesh& Data,
  const TArray<FProcMeshTangent>& ParamTangents,
  UMaterialInstance* MaterialInstance  )
{
// 获取传入的自定义网格数据结构（Data）中顶点的数量，存储到名为VertexCount的整型变量中。
    // 通过调用Num()函数获取数组元素个数的方式来得到顶点数量，后续可以基于这个数量进行循环遍历顶点等操作，比如依次设置每个顶点的坐标等信息。
  int32 VertexCount = Data.Vertices.Num();
  int32 VertexInstanceCount = Data.Triangles.Num();
  int32 PolygonCount = Data.Vertices.Num()/3;

	FMeshDescription MeshDescription;
  FStaticMeshAttributes AttributeGetter(MeshDescription);
  AttributeGetter.Register();

  TPolygonGroupAttributesRef<FName> PolygonGroupNames = AttributeGetter.GetPolygonGroupMaterialSlotNames();
  TVertexAttributesRef<FVector> VertexPositions = AttributeGetter.GetVertexPositions();
  TVertexInstanceAttributesRef<FVector> Tangents = AttributeGetter.GetVertexInstanceTangents();
  TVertexInstanceAttributesRef<float> BinormalSigns = AttributeGetter.GetVertexInstanceBinormalSigns();
  TVertexInstanceAttributesRef<FVector> Normals = AttributeGetter.GetVertexInstanceNormals();
  TVertexInstanceAttributesRef<FVector4> Colors = AttributeGetter.GetVertexInstanceColors();
  TVertexInstanceAttributesRef<FVector2D> UVs = AttributeGetter.GetVertexInstanceUVs();

  // 计算每个 ProcMesh 元素类型的总计
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

  // 创建 VertexInstance
  //"VertexInstance" 是三维图形学和计算机图形学中的术语，它指的是模型中一个特定顶点的实例或具体实现。
  int32 NumIndices = Data.Triangles.Num();
  int32 NumTri = NumIndices / 3;
  TMap<int32, FVertexInstanceID> IndiceIndexToVertexInstanceID;
  IndiceIndexToVertexInstanceID.Reserve(NumVertex);
  for (int32 IndiceIndex = 0; IndiceIndex < NumIndices; IndiceIndex++)
  {
    const int32 VertexIndex = Data.Triangles[IndiceIndex];
    const FVertexID VertexID = VertexIndexToVertexID[VertexIndex];
    const FVertexInstanceID VertexInstanceID =
    MeshDescription.CreateVertexInstance(VertexID);
    IndiceIndexToVertexInstanceID.Add(IndiceIndex, VertexInstanceID);
    Normals[VertexInstanceID] = Data.Normals[VertexIndex];

    // 检查传入的参数Tangents数量是否与顶点数量相等，如果相等则设置切线和双法线符号
    if(ParamTangents.Num() == Data.Vertices.Num())
    {
      Tangents[VertexInstanceID] = ParamTangents[VertexIndex].TangentX;
      BinormalSigns[VertexInstanceID] =
        ParamTangents[VertexIndex].bFlipTangentY ? -1.f : 1.f;
    }else{
    // 如果不相等，这里没有代码，可能需要处理不匹配的情况
    }
    // 设置顶点颜色为黑色
    Colors[VertexInstanceID] = FLinearColor(0,0,0);
    // 检查UV0数量是否与顶点数量相等，如果相等则设置UV坐标
    if(Data.UV0.Num() == Data.Vertices.Num())
    {
       // 如果不相等，则设置UV坐标为(0,0)
      UVs.Set(VertexInstanceID, 0, Data.UV0[VertexIndex]);
    }else{
      UVs.Set(VertexInstanceID, 0, FVector2D(0,0));
    }
    // 将其他UV通道设置为(0,0)
    UVs.Set(VertexInstanceID, 1, FVector2D(0,0));
    UVs.Set(VertexInstanceID, 2, FVector2D(0,0));
    UVs.Set(VertexInstanceID, 3, FVector2D(0,0));
  }

  // 遍历三角形，将每个多边形添加到网格描述中
  for (int32 TriIdx = 0; TriIdx < NumTri; TriIdx++)
  {
    FVertexID VertexIndexes[3];
    TArray<FVertexInstanceID> VertexInstanceIDs;
    VertexInstanceIDs.SetNum(3);

     // 获取三角形的每个角的顶点索引和实例ID
    for (int32 CornerIndex = 0; CornerIndex < 3; ++CornerIndex)
    {
      const int32 IndiceIndex = (TriIdx * 3) + CornerIndex;
      const int32 VertexIndex = Data.Triangles[IndiceIndex];
      VertexIndexes[CornerIndex] = VertexIndexToVertexID[VertexIndex];
      VertexInstanceIDs[CornerIndex] =
        IndiceIndexToVertexInstanceID[IndiceIndex];
    }

    // 将一个多边形插入到网格中
    MeshDescription.CreatePolygon(NewPolygonGroup, VertexInstanceIDs);

  }

  return MeshDescription;
}

// UMapGenFunctionLibrary类的静态方法，用于根据给定的数据创建网格
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

  // 构建包的路径
  FString PackageName = "/Game/CustomMaps/" + MapName + "/Static/" + FolderName + "/" + MeshName.ToString();

  // 检查包路径是否存在，如果不存在则创建目录
  if (!PlatformFile.DirectoryExists(*PackageName))
  {
    //PlatformFile.CreateDirectory(*PackageName);
  }


  // 从给定的数据和切线信息构建网格描述
  FMeshDescription Description = BuildMeshDescriptionFromData(Data,ParamTangents, MaterialInstance);

  // 检查构建的网格描述中是否有多边形
  if (Description.Polygons().Num() > 0)
  {
     // 创建一个新的包（Package），用于存储静态网格
    UPackage* Package = CreatePackage(*PackageName);
    // 检查包是否创建成功
    check(Package);
    // 在新建的包中创建一个新的静态网格对象
    UStaticMesh* Mesh = NewObject<UStaticMesh>( Package, MeshName, RF_Public | RF_Standalone);

    // 初始化网格资源
    Mesh->InitResources();
    // 为网格设置一个新的光照Guid
    Mesh->LightingGuid = FGuid::NewGuid();
    // 将材质实例添加到网格的静态材质列表中
    Mesh->StaticMaterials.Add(FStaticMaterial(MaterialInstance));
    // 从网格描述构建网格
    Mesh->BuildFromMeshDescriptions({ &Description }, Params);
    // 为网格创建刚体设置
    Mesh->CreateBodySetup();
    // 设置网格的碰撞追踪标志，使用复杂网格作为简单网格
    Mesh->BodySetup->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;
    // 为网格的刚体设置创建物理网格
    Mesh->BodySetup->CreatePhysicsMeshes();
    // Build mesh from source
    Mesh->NeverStream = false;
    TArray<UObject*> CreatedAssets;
    CreatedAssets.Add(Mesh);

    // Notify asset registry of new asset
    FAssetRegistryModule::AssetCreated(Mesh);
    //UPackage::SavePackage(Package, Mesh, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *(MeshName.ToString()), GError, nullptr, true, true, SAVE_NoError);
    Package->MarkPackageDirty();
    return Mesh;
  }
  return nullptr;
}

FVector2D UMapGenFunctionLibrary::GetTransversemercProjection(float lat, float lon, float lat0, float lon0)
{
  // earth radius in m
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

// UMapGenFunctionLibrary类的方法，用于使当前线程休眠指定的秒数
void UMapGenFunctionLibrary::SetThreadToSleep(float seconds){
  //FGenericPlatformProcess::Sleep(seconds);
}

// UMapGenFunctionLibrary类的方法，用于在蓝图中刷新渲染命令
void UMapGenFunctionLibrary::FlushRenderingCommandsInBlueprint(){
  // 刷新渲染命令，确保所有图形操作都已完成
  FlushRenderingCommands(true);
	// 在游戏线程中刷新待删除的RHI资源
 	FlushPendingDeleteRHIResources_GameThread();
}

// UMapGenFunctionLibrary类的方法，用于清理GEngine
void UMapGenFunctionLibrary::CleanupGEngine(){
  // 执行垃圾回收并清理Actor
  GEngine->PerformGarbageCollectionAndCleanupActors();
// 如果编译器配置为使用编辑器
#if WITH_EDITOR
  // 创建一个文本对象，用于重置操作的描述
  FText TransResetText(FText::FromString("Clean up after Move actors to sublevels"));
  // 如果GEditor的Trans对象存在
  if ( GEditor->Trans )
  {
    // 重置事务
    GEditor->Trans->Reset(TransResetText);
    // 清理编辑器，包括移动Actor到子级别
    GEditor->Cleanse(true, true, TransResetText);
  }
#endif
}
