// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaExporter.h"// 引入CarlaExporter头文件
#include "CarlaExporterCommands.h"// 引入CarlaExporterCommands头文件
#include "Misc/MessageDialog.h"// 引入MessageDialog头文件，用于显示消息对话框
#include "Framework/MultiBox/MultiBoxBuilder.h"// 引入MultiBoxBuilder头文件，用于构建多功能菜单
#include "SlateBasics.h" // 引入Slate基础组件库
#include "SlateExtras.h" // 引入Slate扩展组件库
#include "Runtime/Engine/Classes/Engine/Selection.h"// 引入Selection头文件，用于获取选中的对象
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"// 引入StaticMeshActor头文件，用于操作静态网格体Actor

#include "Components/InstancedStaticMeshComponent.h"// 引入InstancedStaticMeshComponent头文件，用于操作实例化静态网格组件
#include "PhysicsEngine/BodySetup.h" // 引入BodySetup头文件，用于设置物理体的碰撞体
#include "PhysicsEngine/ConvexElem.h"  // 引入ConvexElem头文件，用于描述凸体碰撞元素
#include "PxTriangleMesh.h" // 引入PxTriangleMesh头文件，用于处理三角网格的物理计算
#include "PxVec3.h"// 引入PxVec3头文件，表示三维向量
#include "LevelEditor.h"// 引入LevelEditor头文件，用于操作关卡编辑器
#include "EngineUtils.h"// 引入EngineUtils头文件，提供引擎工具函数
#include "PhysXPublic.h"// 引入PhysXPublic头文件，提供物理引擎的公共接口
#include "PhysicsPublic.h" // 引入PhysicsPublic头文件，提供物理引擎公共的物理学功能
#include "PhysXIncludes.h"// 引入PhysXIncludes头文件，用于包含PhysX物理引擎的相关文件
#include "PxSimpleTypes.h" // 引入PxSimpleTypes头文件，提供PhysX物理引擎的简单类型
#include <fstream>// 引入fstream头文件，用于文件读写操作
#include <sstream>// 引入sstream头文件，用于字符串流处理

static const FName CarlaExporterTabName("CarlaExporter");//声明了一个名为CarlaExporterTabName的静态常量，类型为FName，并初始化为"CarlaExporter"。

#define LOCTEXT_NAMESPACE "FCarlaExporterModule"
// 模块启动时调用的函数
void FCarlaExporterModule::StartupModule()
{
  // 这段代码将在你的模块被加载到内存中后执行；
  //具体的执行时间在每个模块的.uplugin 文件中指定。

  // 调用FCarlaExporterCommands类的静态函数Register，通常这个函数用于注册相关的命令，
// 可能是将一些自定义的操作命令注册到系统中，以便后续能够被识别和触发执行，比如在编辑器环境下响应特定的用户操作。
  FCarlaExporterCommands::Register();

  PluginCommands = MakeShareable(new FUICommandList);
// 将一个具体的操作（Action）映射（MapAction）到前面创建的命令列表（PluginCommands）中。
// 这里的操作对应的是FCarlaExporterCommands类获取到的PluginActionExportAll动作，也就是定义好的一个具体可执行的导出所有内容的操作。
// 通过FExecuteAction::CreateRaw函数创建一个执行动作的绑定，将这个动作与当前类（this指针所指向的类，应该是FCarlaExporterModule类）中的PluginButtonClicked函数关联起来，
// 意味着当这个动作被触发时，就会执行PluginButtonClicked函数来处理具体的逻辑，最后的FCanExecuteAction()可能是用于定义这个动作是否可执行的相关条件判断等内容（虽然这里暂时没传入具体逻辑）

  PluginCommands->MapAction(
    FCarlaExporterCommands::Get().PluginActionExportAll,
    FExecuteAction::CreateRaw(this, &FCarlaExporterModule::PluginButtonClicked),
    FCanExecuteAction());
// 通过模块管理器（FModuleManager）加载名为"LevelEditor"的模块，并获取其引用（LoadModuleChecked会确保模块加载成功，若加载失败会抛出异常），
// 这个"LevelEditor"模块通常是和游戏引擎等编辑器相关的核心模块，后续操作可能会基于这个模块来扩展编辑器的菜单等功能。

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
// 模块关闭时调用的函数
void FCarlaExporterModule::ShutdownModule()
{
  // 对于支持动态重新加载的模块，
  //这个函数可能会在关闭期间被调用以清理你的模块。
  //在卸载模块之前，我们会调用这个函数。
  FCarlaExporterCommands::Unregister();
}
// 插件按钮点击事件的处理函数
void FCarlaExporterModule::PluginButtonClicked()
{
  UWorld* World = GEditor->GetEditorWorldContext().World();
  if (!World) return;

  //获取所有选中的对象（如果有任何选中对象的话）
  TArray<UObject*> BP_Actors;
  USelection* CurrentSelection = GEditor->GetSelectedActors();
  int32 SelectionNum = CurrentSelection->GetSelectedObjects(AActor::StaticClass(), BP_Actors);

  // 如果没有选中任何对象，那么获取所有对象
  if (SelectionNum == 0)
  {
    for (TActorIterator<AActor> it(World); it; ++it)
      BP_Actors.Add(Cast<UObject>(*it));
  }

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
  constexpr float TO_METERS = 0.01f;

  int offset = 1;
  AreaType areaType;
  for (int round = 0; round < rounds; ++round)
  {
    for (UObject* SelectedObject : BP_Actors)
    {
      AActor* TempActor = Cast<AActor>(SelectedObject);
      if (!TempActor) continue;

      //检查标签（“NoExport”）
      if (TempActor->ActorHasTag(FName("NoExport"))) continue;

      FString ActorName = TempActor->GetName();

      // 通过命名规则检查类型
      // 这段代码主要是根据给定的字符串 `ActorName` 的内容来确定 `areaType` 的值，通过一系列的字符串查找判断，将不同命名模式的 `ActorName` 归类到对应的 `AreaType` 枚举值中。

// 首先判断 `ActorName` 中是否包含 "Road_Road" 或者 "Roads_Road" 字符串，如果包含其中任意一个，说明该 `Actor` 与道路相关，就将 `areaType` 设置为 `AreaType::ROAD`。
if (ActorName.Find("Road_Road")!= -1 || ActorName.Find("Roads_Road")!= -1)
    areaType = AreaType::ROAD;
// 如果不包含上述字符串，接着判断是否包含 "Road_Marking" 或者 "Roads_Marking" 字符串，若包含，则同样认为该 `Actor` 与道路相关（比如道路标线等也算道路部分），将 `areaType` 也设置为 `AreaType::ROAD`。
else if (ActorName.Find("Road_Marking")!= -1 || ActorName.Find("Roads_Marking")!= -1)
    areaType = AreaType::ROAD;
// 再判断 `ActorName` 是否包含 "Road_Curb" 或者 "Roads_Curb" 字符串，若包含，表示该 `Actor` 与道路边缘的路缘石相关，同样归为道路相关类别，将 `areaType` 设置为 `AreaType::ROAD`。
else if (ActorName.Find("Road_Curb")!= -1 || ActorName.Find("Roads_Curb")!= -1)
    areaType = AreaType::ROAD;
// 继续判断 `ActorName` 中是否有 "Road_Gutter" 或者 "Roads_Gutter" 字符串，若存在，意味着该 `Actor` 与道路的排水沟等相关元素有关，也属于道路相关部分，所以 `areaType` 赋值为 `AreaType::ROAD`。
else if (ActorName.Find("Road_Gutter")!= -1 || ActorName.Find("Roads_Gutter")!= -1)
    areaType = AreaType::ROAD;
// 然后判断 `ActorName` 是否包含 "Road_Sidewalk" 或者 "Roads_Sidewalk" 字符串，若包含，说明该 `Actor` 与人行道相关，此时将 `areaType` 设置为 `AreaType::SIDEWALK`，将其归类为人行道类别。
else if (ActorName.Find("Road_Sidewalk")!= -1 || ActorName.Find("Roads_Sidewalk")!= -1)
    areaType = AreaType::SIDEWALK;
// 接着判断 `ActorName` 中是否存在 "Road_Crosswalk" 或者 "Roads_Crosswalk" 字符串，若有，则表明该 `Actor` 与人行横道相关，把 `areaType` 赋值为 `AreaType::CROSSWALK`，将其归到人行横道类别。
else if (ActorName.Find("Road_Crosswalk")!= -1 || ActorName.Find("Roads_Crosswalk")!= -1)
    areaType = AreaType::CROSSWALK;
// 再判断 `ActorName` 是否包含 "Road_Grass" 或者 "Roads_Grass" 字符串，若包含，意味着该 `Actor` 与草地相关，将 `areaType` 设置为 `AreaType::GRASS`，将其归类到草地类别。
else if (ActorName.Find("Road_Grass")!= -1 || ActorName.Find("Roads_Grass")!= -1)
    areaType = AreaType::GRASS;
// 如果以上所有包含特定字符串的条件都不满足，说明该 `Actor` 不属于前面所列举的明确分类情况，那么将 `areaType` 设置为 `AreaType::BLOCK`，可以理解为其他未分类的块状区域之类的默认类别。
else
    areaType = AreaType::BLOCK;

      // 检查是否在这一回合中导出
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
// 写入对象几何体到文件
int32 FCarlaExporterModule::WriteObjectGeom(std::ofstream &f, FString ObjectName, UBodySetup *body, FTransform &CompTransform, AreaType Area, int32 Offset)
{
  // 如果传入的 UBodySetup 指针为空，则直接返回 0，表示没有添加任何顶点
  if (!body) return 0;

  // 定义常量 TO_METERS，用于将 Unreal Engine 的单位转换为米（Unreal Engine 默认使用厘米）
  constexpr float TO_METERS = 0.01f;
  FVector CompLocation = CompTransform.GetTranslation();
  // 初始化计数器 TotalVerticesAdded 为 0，用于记录本次调用中添加的总顶点数
  int TotalVerticesAdded = 0;
  bool Written = false;

  // 尝试写入盒形碰撞体
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
// IMPLEMENT_MODULE 可能是一个宏（macro），用于实现某个模块相关的功能定义或者注册等操作。
// 在这里，它接受两个参数，第一个参数 FCarlaExporterModule 很可能是代表模块的类名或者模块相关的类型名称，
// 第二个参数 CarlaExporter 可能是具体模块的标识或者和模块功能实现相关联的一个名称（比如是模块类的实例名称、模块在系统中的特定代号等）。
// 整体这行代码的作用大概率是借助 IMPLEMENT_MODULE 这个宏机制，将名为 FCarlaExporterModule 的模块相关内容进行实现、注册或者关联到整个系统中，
// 使得该模块能够在对应的程序框架（比如可能是某个游戏引擎、仿真系统等使用了这种模块管理机制的环境里）下正常工作，发挥其特定的导出（Exporter）相关的功能。
