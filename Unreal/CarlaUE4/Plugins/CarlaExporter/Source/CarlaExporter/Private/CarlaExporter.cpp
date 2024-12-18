// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaExporter.h"
#include "CarlaExporterCommands.h"
#include "Misc/MessageDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "Runtime/Engine/Classes/Engine/Selection.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConvexElem.h"
#include "PxTriangleMesh.h"
#include "PxVec3.h"
#include "LevelEditor.h"
#include "EngineUtils.h"
#include "PhysXPublic.h"
#include "PhysicsPublic.h"
#include "PhysXIncludes.h"
#include "PxSimpleTypes.h"
#include <fstream>
#include <sstream>

static const FName CarlaExporterTabName("CarlaExporter");

#define LOCTEXT_NAMESPACE "FCarlaExporterModule"

void FCarlaExporterModule::StartupModule()
{
<<<<<<< HEAD
  // 该代码将在模块加载到内存后执行，具体的执行时间由 .uplugin 文件中的 per-module 指定
=======
  // 这段代码将在你的模块被加载到内存中后执行；
  //具体的执行时间在每个模块的.uplugin 文件中指定。
>>>>>>> 8d1254daeddeb0654f073858a955d3f26695c945

  FCarlaExporterCommands::Register(); // 注册插件命令

  PluginCommands = MakeShareable(new FUICommandList);// 创建一个新的命令列表对象

  PluginCommands->MapAction(
    FCarlaExporterCommands::Get().PluginActionExportAll,// 获取插件命令
    FExecuteAction::CreateRaw(this, &FCarlaExporterModule::PluginButtonClicked),
    FCanExecuteAction());// 判断是否可以执行命令

  FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

  {
    TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
    MenuExtender->AddMenuExtension("FileActors",
      EExtensionHook::After,
      PluginCommands,
      FMenuExtensionDelegate::CreateRaw(this, &FCarlaExporterModule::AddMenuExtension));

    LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
  }
}

void FCarlaExporterModule::ShutdownModule()
{
<<<<<<< HEAD
  // 在模块卸载时调用此函数进行清理
  FCarlaExporterCommands::Unregister();// 注销插件命令
=======
  // 对于支持动态重新加载的模块，
  //这个函数可能会在关闭期间被调用以清理你的模块。
  //在卸载模块之前，我们会调用这个函数。
  FCarlaExporterCommands::Unregister();
>>>>>>> 8d1254daeddeb0654f073858a955d3f26695c945
}

void FCarlaExporterModule::PluginButtonClicked()
{
  UWorld* World = GEditor->GetEditorWorldContext().World();
  if (!World) return;

<<<<<<< HEAD
  // 获取当前选中的所有对象（如果有的话）
=======
  //获取所有选中的对象（如果有任何选中对象的话）
>>>>>>> 8d1254daeddeb0654f073858a955d3f26695c945
  TArray<UObject*> BP_Actors;
  USelection* CurrentSelection = GEditor->GetSelectedActors();
  int32 SelectionNum = CurrentSelection->GetSelectedObjects(AActor::StaticClass(), BP_Actors);

<<<<<<< HEAD
  //如果没有选中任何对象，则获取所有对象
=======
  // 如果没有选中任何对象，那么获取所有对象
>>>>>>> 8d1254daeddeb0654f073858a955d3f26695c945
  if (SelectionNum == 0)
  {
    for (TActorIterator<AActor> it(World); it; ++it)
      BP_Actors.Add(Cast<UObject>(*it));
  }

<<<<<<< HEAD
  // 获取目标路径
  FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());
  // 构建文件名
  std::ostringstream name;
  name << TCHAR_TO_UTF8(*Path) << "/" << TCHAR_TO_UTF8(*World->GetMapName()) << ".obj";
  // 创建文件
  std::ofstream f(name.str());

  // 定义导出的轮次
  int rounds;
  rounds = 5;

  // Unreal 使用的是 'cm'（厘米），Recast 使用的是 'm'（米），需要做缩放
=======
  // 得到目标路径
  FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());
  //构建最终名称。
  std::ostringstream name;
  name << TCHAR_TO_UTF8(*Path) << "/" << TCHAR_TO_UTF8(*World->GetMapName()) << ".obj";
  //创建文件。
  std::ofstream f(name.str());

  // 定义回合数
  int rounds;
  rounds = 5;

  // 我们需要缩放网格（虚幻引擎使用 “厘米”，Recast 使用 “米”）
>>>>>>> 8d1254daeddeb0654f073858a955d3f26695c945
  constexpr float TO_METERS = 0.01f;

  int offset = 1;
  AreaType areaType;
  for (int round = 0; round < rounds; ++round)
  {
    for (UObject* SelectedObject : BP_Actors)
    {
      AActor* TempActor = Cast<AActor>(SelectedObject);
      if (!TempActor) continue;

<<<<<<< HEAD
      /检查该 Actor 是否有 "NoExport" 标签
=======
      //检查标签（“NoExport”）
>>>>>>> 8d1254daeddeb0654f073858a955d3f26695c945
      if (TempActor->ActorHasTag(FName("NoExport"))) continue;

      FString ActorName = TempActor->GetName();

<<<<<<< HEAD
      // 根据命名规则检查 Actor 类型
=======
      // 通过命名规则检查类型
>>>>>>> 8d1254daeddeb0654f073858a955d3f26695c945
      if (ActorName.Find("Road_Road") != -1 || ActorName.Find("Roads_Road") != -1)
        areaType = AreaType::ROAD;
      else if (ActorName.Find("Road_Marking") != -1 || ActorName.Find("Roads_Marking") != -1)
        areaType = AreaType::ROAD;
      else if (ActorName.Find("Road_Curb") != -1 || ActorName.Find("Roads_Curb") != -1)
        areaType = AreaType::ROAD;
      else if (ActorName.Find("Road_Gutter") != -1 || ActorName.Find("Roads_Gutter") != -1)
        areaType = AreaType::ROAD;
      else if (ActorName.Find("Road_Sidewalk") != -1 || ActorName.Find("Roads_Sidewalk") != -1)
        areaType = AreaType::SIDEWALK;
      else if (ActorName.Find("Road_Crosswalk") != -1 || ActorName.Find("Roads_Crosswalk") != -1)
        areaType = AreaType::CROSSWALK;
      else if (ActorName.Find("Road_Grass") != -1 || ActorName.Find("Roads_Grass") != -1)
        areaType = AreaType::GRASS;
      else
        areaType = AreaType::BLOCK;

<<<<<<< HEAD
      // 根据导出轮次决定是否导出
=======
      // 检查是否在这一回合中导出
>>>>>>> 8d1254daeddeb0654f073858a955d3f26695c945
      if (rounds > 1)
      {
        if (areaType == AreaType::BLOCK && round != 0)
          continue;
        else if (areaType == AreaType::ROAD && round != 1)
          continue;
        else if (areaType == AreaType::GRASS && round != 2)
          continue;
        else if (areaType == AreaType::SIDEWALK && round != 3)
          continue;
        else if (areaType == AreaType::CROSSWALK && round != 4)
          continue;
      }

      f << "g " << TCHAR_TO_ANSI(*(ActorName)) << "\n";

      TArray<UActorComponent*> Components = TempActor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
      for (auto *Component : Components)
      {

        // 检查是否是实例化静态网格。
        UInstancedStaticMeshComponent* comp2 = Cast<UInstancedStaticMeshComponent>(Component);
        if (comp2)
        {
          UBodySetup *body = comp2->GetBodySetup();
          if (!body) continue;

          for (int i=0; i<comp2->GetInstanceCount(); ++i)
          {
            //将“instance_”加上索引值 i 写入文件并换行。
            FString ObjectName = ActorName +"_"+FString::FromInt(i);

            // 获取组件的位置和变换
            FTransform InstanceTransform;
            comp2->GetInstanceTransform(i, InstanceTransform, true);
            FVector InstanceLocation = InstanceTransform.GetTranslation();

            offset += WriteObjectGeom(f, ObjectName, body, InstanceTransform, areaType, offset);
          }
        }
        else
        {
          // 尝试作为静态网格。
          UStaticMeshComponent* comp = Cast<UStaticMeshComponent>(Component);
          if (!comp) continue;

          UBodySetup *body = comp->GetBodySetup();
          if (!body)
            continue;

          //将字母“o”以及经过转换后的组件名称写入文件并换行。
          FString ObjectName = ActorName +"_"+comp->GetName();

          // 获取组件的位置和变换。
          FTransform CompTransform = comp->GetComponentTransform();
          FVector CompLocation = CompTransform.GetTranslation();

          offset += WriteObjectGeom(f, ObjectName, body, CompTransform, areaType, offset);
        }
      }
    }
  }
  f.close();
}

int32 FCarlaExporterModule::WriteObjectGeom(std::ofstream &f, FString ObjectName, UBodySetup *body, FTransform &CompTransform, AreaType Area, int32 Offset)
{
  if (!body) return 0;

  constexpr float TO_METERS = 0.01f;
  FVector CompLocation = CompTransform.GetTranslation();
  int TotalVerticesAdded = 0;
  bool Written = false;

  // try to write the box collision if any
  for (const auto &box: body->AggGeom.BoxElems)
  {
    // 得到数据
    const int32 nbVerts = 8;
    TArray<FVector> boxVerts;
    TArray<int32> indexBuffer;

    FVector HalfExtent(box.X / 2.0f, box.Y / 2.0f, box.Z / 2.0f);

    f << "o " << TCHAR_TO_ANSI(*(ObjectName +"_box")) << "\n";

    // 定义 8 个顶点。
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(-HalfExtent.X, -HalfExtent.Y, -HalfExtent.Z)));
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(-HalfExtent.X, -HalfExtent.Y, +HalfExtent.Z)));
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(-HalfExtent.X, +HalfExtent.Y, -HalfExtent.Z)));
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(-HalfExtent.X, +HalfExtent.Y, +HalfExtent.Z)));
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(+HalfExtent.X, -HalfExtent.Y, -HalfExtent.Z)));
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(+HalfExtent.X, -HalfExtent.Y, +HalfExtent.Z)));
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(+HalfExtent.X, +HalfExtent.Y, -HalfExtent.Z)));
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(+HalfExtent.X, +HalfExtent.Y, +HalfExtent.Z)));

    // 定义 12 个面（36 个索引）。
    indexBuffer.Add(0); indexBuffer.Add(1); indexBuffer.Add(3);
    indexBuffer.Add(0); indexBuffer.Add(3); indexBuffer.Add(2);
    indexBuffer.Add(2); indexBuffer.Add(3); indexBuffer.Add(7);
    indexBuffer.Add(2); indexBuffer.Add(7); indexBuffer.Add(6);
    indexBuffer.Add(6); indexBuffer.Add(7); indexBuffer.Add(5);
    indexBuffer.Add(6); indexBuffer.Add(5); indexBuffer.Add(4);
    indexBuffer.Add(4); indexBuffer.Add(5); indexBuffer.Add(1);
    indexBuffer.Add(4); indexBuffer.Add(1); indexBuffer.Add(0);
    indexBuffer.Add(2); indexBuffer.Add(6); indexBuffer.Add(4);
    indexBuffer.Add(2); indexBuffer.Add(4); indexBuffer.Add(0);
    indexBuffer.Add(7); indexBuffer.Add(3); indexBuffer.Add(1);
    indexBuffer.Add(7); indexBuffer.Add(1); indexBuffer.Add(5);

    // 写入所有顶点。
    for (int32 j=0; j<nbVerts; j++)
    {
      const FVector &v = boxVerts[j];
      FVector vec(v.X, v.Y, v.Z);
      FVector vec3 = CompTransform.TransformVector(vec);
      FVector world(CompLocation.X + vec3.X, CompLocation.Y + vec3.Y, CompLocation.Z + vec3.Z);

      f << "v " << std::fixed << world.X * TO_METERS << " " << world.Z * TO_METERS << " " << world.Y * TO_METERS << "\n";
    }
    //根据区域类型设置材质。
    switch (Area)
    {
      case AreaType::ROAD:      f << "usemtl road"      << "\n"; break;
      case AreaType::GRASS:     f << "usemtl grass"     << "\n"; break;
      case AreaType::SIDEWALK:  f << "usemtl sidewalk"  << "\n"; break;
      case AreaType::CROSSWALK: f << "usemtl crosswalk" << "\n"; break;
      case AreaType::BLOCK:     f << "usemtl block"     << "\n"; break;
    }
    //写入所有面。
    int k = 0;
    for (int32 i=0; i<indexBuffer.Num()/3; ++i)
    {
      // 对于左手坐标系，使用逆序。
      f << "f " << Offset + indexBuffer[k+2] << " " << Offset + indexBuffer[k+1] << " " << Offset + indexBuffer[k] << "\n";
      k += 3;
    }
    TotalVerticesAdded += nbVerts;
    Offset += nbVerts;
    Written = true;
  }

  // 尝试写入简单碰撞网格。
  for (const auto &convex : body->AggGeom.ConvexElems)
  {
    //获取数据。
    PxConvexMesh *mesh = convex.GetConvexMesh();
    if (!mesh) continue;
    int32 nbVerts = mesh->getNbVertices();
    const PxVec3* convexVerts = mesh->getVertices();
    const PxU8* indexBuffer = (PxU8 *) mesh->getIndexBuffer();

    f << "o " << TCHAR_TO_ANSI(*(ObjectName +"_convex")) << "\n";

    //写入所有顶点。
    for (int32 j=0; j<nbVerts; j++)
    {
      const PxVec3 &v = convexVerts[j];
      FVector vec(v.x, v.y, v.z);
      FVector vec3 = CompTransform.TransformVector(vec);
      FVector world(CompLocation.X + vec3.X, CompLocation.Y + vec3.Y, CompLocation.Z + vec3.Z);

      f << "v " << std::fixed << world.X * TO_METERS << " " << world.Z * TO_METERS << " " << world.Y * TO_METERS << "\n";
    }
    // 根据区域类型设置材质。
    switch (Area)
    {
      case AreaType::ROAD:      f << "usemtl road"      << "\n"; break;
      case AreaType::GRASS:     f << "usemtl grass"     << "\n"; break;
      case AreaType::SIDEWALK:  f << "usemtl sidewalk"  << "\n"; break;
      case AreaType::CROSSWALK: f << "usemtl crosswalk" << "\n"; break;
      case AreaType::BLOCK:     f << "usemtl block"     << "\n"; break;
    }
    // 写入所有面。
    for (PxU32 i=0; i<mesh->getNbPolygons(); ++i)
    {
      PxHullPolygon face;
      bool status = mesh->getPolygonData(i, face);
      const PxU8* faceIndices = indexBuffer + face.mIndexBase;
      int faceNbVerts = face.mNbVerts;
      for(int32 j=2; j<faceNbVerts; j++)
      {
        // 对于左手坐标系，使用逆序。
        f << "f " << Offset + faceIndices[j-1] << " " << Offset + faceIndices[j] << " " << Offset + faceIndices[0] << "\n";
      }
    }
    TotalVerticesAdded += nbVerts;
    Offset += nbVerts;
    Written = true;
  }

  if (!Written)
  {
    //写入网格。
    for (const auto &mesh : body->TriMeshes)
    {
      // 得到数据
      PxU32 nbVerts = mesh->getNbVertices();
      const PxVec3* convexVerts = mesh->getVertices();

      f << "o " << TCHAR_TO_ANSI(*(ObjectName +"_mesh")) << "\n";

      // 写入所有顶点。
      for (PxU32 j=0; j<nbVerts; j++)
      {
        const PxVec3 &v = convexVerts[j];
        FVector vec(v.x, v.y, v.z);
        FVector vec3 = CompTransform.TransformVector(vec);
        FVector world(CompLocation.X + vec3.X, CompLocation.Y + vec3.Y, CompLocation.Z + vec3.Z);

        f << "v " << std::fixed << world.X * TO_METERS << " " << world.Z * TO_METERS << " " << world.Y * TO_METERS << "\n";
      }
      // 根据区域类型设置材质。
      switch (Area)
      {
        case AreaType::ROAD:      f << "usemtl road"      << "\n"; break;
        case AreaType::GRASS:     f << "usemtl grass"     << "\n"; break;
        case AreaType::SIDEWALK:  f << "usemtl sidewalk"  << "\n"; break;
        case AreaType::CROSSWALK: f << "usemtl crosswalk" << "\n"; break;
        case AreaType::BLOCK:     f << "usemtl block"     << "\n"; break;
      }
      // 写入所有面
      int k = 0;
      //三角形索引可以是 16 位或 32 位。
      if (mesh->getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES)
      {
        PxU16 *Indices16 = (PxU16 *) mesh->getTriangles();
        for (PxU32 i=0; i<mesh->getNbTriangles(); ++i)
        {
          //  对于左手坐标系，使用逆序。
          f << "f " << Offset + Indices16[k+2] << " " << Offset + Indices16[k+1] << " " << Offset + Indices16[k] << "\n";
          k += 3;
        }
      }
      else
      {
        PxU32 *Indices32 = (PxU32 *) mesh->getTriangles();
        for (PxU32 i=0; i<mesh->getNbTriangles(); ++i)
        {
          //  对于左手坐标系，使用逆序。
          f << "f " << Offset + Indices32[k+2] << " " << Offset + Indices32[k+1] << " " << Offset + Indices32[k] << "\n";
          k += 3;
        }
      }
      TotalVerticesAdded += nbVerts;
      Offset += nbVerts;
    }
  }
  return TotalVerticesAdded;
}

void FCarlaExporterModule::AddMenuExtension(FMenuBuilder& Builder)
{
  Builder.AddMenuEntry(FCarlaExporterCommands::Get().PluginActionExportAll);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCarlaExporterModule, CarlaExporter)
