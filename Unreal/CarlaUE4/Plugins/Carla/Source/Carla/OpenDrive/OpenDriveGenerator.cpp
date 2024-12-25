// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>. 

// 包含头文件，用于访问Carla游戏框架和相关类
#include "Carla.h"
#include "Carla/Game/CarlaStatics.h"
#include "OpenDriveGenerator.h"
#include "Traffic/TrafficLightManager.h"
#include "Util/ProceduralCustomMesh.h"

// 禁用和启用UE4宏，以避免命名冲突
#include <compiler/disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/rpc/String.h>
#include <compiler/enable-ue4-macros.h>

// 包含Unreal Engine的头文件，用于访问游戏引擎的功能
#include "Engine/Classes/Interfaces/Interface_CollisionDataProvider.h"
#include "PhysicsCore/Public/BodySetupEnums.h"

// AProceduralMeshActor类的构造函数，用于初始化ProceduralMeshActor
AProceduralMeshActor::AProceduralMeshActor()
{
    // 设置PrimaryActorTick标志，表明这个Actor不需要每帧进行Tick
    PrimaryActorTick.bCanEverTick = false;

    // 创建一个默认的子对象UProceduralMeshComponent，并命名为"RootComponent"
    MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("RootComponent"));

    // 将这个MeshComponent设置为Actor的根组件
    RootComponent = MeshComponent;
}

// AOpenDriveGenerator类的构造函数，用于初始化OpenDriveGenerator
AOpenDriveGenerator::AOpenDriveGenerator(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false; // 设置PrimaryActorTick标志，表明这个Actor不需要每帧进行Tick
  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent")); // 创建一个默认的子对象USceneComponent，并命名为"SceneComponent"
  SetRootComponent(RootComponent); // 将这个SceneComponent设置为Actor的根组件
  RootComponent->Mobility = EComponentMobility::Static; // 设置组件的移动性为静态
}

// 加载OpenDrive文件
bool AOpenDriveGenerator::LoadOpenDrive(const FString &OpenDrive)
{
  using OpenDriveLoader = carla::opendrive::OpenDriveParser; // 使用carla的OpenDriveParser

  if (OpenDrive.IsEmpty()) // 检查OpenDrive字符串是否为空
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive is empty")); // 如果为空，记录错误日志
    return false;
  }

  OpenDriveData = OpenDrive; // 将OpenDrive字符串存储在成员变量中
  return true;
}

// 获取OpenDrive数据
const FString &AOpenDriveGenerator::GetOpenDrive() const
{
  return OpenDriveData;
}

// 检查OpenDrive是否有效
bool AOpenDriveGenerator::IsOpenDriveValid() const
{
  return UCarlaStatics::GetGameMode(GetWorld())->GetMap().has_value(); // 检查是否有有效的地图数据
}

// 生成道路网格
void AOpenDriveGenerator::GenerateRoadMesh()
{
  if (!IsOpenDriveValid()) // 检查OpenDrive是否有效
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive has not been loaded")); // 如果无效，记录错误日志
    return;
  }

  carla::rpc::OpendriveGenerationParameters Parameters; // 定义OpendriveGenerationParameters
  UCarlaGameInstance * GameInstance = UCarlaStatics::GetGameInstance(GetWorld()); // 获取游戏实例
  if(GameInstance) // 如果游戏实例存在
  {
    Parameters = GameInstance->GetOpendriveGenerationParameters(); // 获取OpendriveGenerationParameters
  }
  else
  {
    carla::log_warning("Missing game instance"); // 如果游戏实例不存在，记录警告日志
  }

  auto& CarlaMap = UCarlaStatics::GetGameMode(GetWorld())->GetMap(); // 获取Carla地图
  const auto Meshes = CarlaMap->GenerateChunkedMesh(Parameters); // 生成网格
  for (const auto &Mesh : Meshes) // 遍历每个网格
  {
    if (!Mesh->GetVertices().size()) // 如果网格顶点数为0，跳过
    {
      continue;
    }
    AProceduralMeshActor* TempActor = GetWorld()->SpawnActor<AProceduralMeshActor>(); // 生成一个新的ProceduralMeshActor
    UProceduralMeshComponent *TempPMC = TempActor->MeshComponent; // 获取ProceduralMeshComponent
    TempPMC->bUseAsyncCooking = true; // 设置异步烘焙
    TempPMC->bUseComplexAsSimpleCollision = true; // 设置复杂碰撞为简单碰撞
    TempPMC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 设置碰撞模式

    const FProceduralCustomMesh MeshData = *Mesh; // 获取网格数据
    TempPMC->CreateMeshSection_LinearColor( // 创建网格部分
        0,
        MeshData.Vertices,
        MeshData.Triangles,
        MeshData.Normals,
        TArray<FVector2D>(), // UV0
        TArray<FLinearColor>(), // VertexColor
        TArray<FProcMeshTangent>(), // Tangents
        true); // Create collision

    ActorMeshList.Add(TempActor); // 将新生成的Actor添加到列表中
  }

  if(!Parameters.enable_mesh_visibility) // 如果不启用网格可见性
  {
    for(AActor * actor : ActorMeshList) // 遍历Actor列表
    {
      actor->SetActorHiddenInGame(true); // 设置Actor在游戏中隐藏
    }
  }
}

// 生成杆子（未实现）
void AOpenDriveGenerator::GeneratePoles()
{
  if (!IsOpenDriveValid()) // 检查OpenDrive是否有效
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive has not been loaded")); // 如果无效，记录错误日志
    return;
  }
  /// TODO: To implement
}

// 生成生成点
void AOpenDriveGenerator::GenerateSpawnPoints()
{
  if (!IsOpenDriveValid()) // 检查OpenDrive是否有效
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive has not been loaded")); // 如果无效，记录错误日志
    return;
  }
  auto& CarlaMap = UCarlaStatics::GetGameMode(GetWorld())->GetMap(); // 获取Carla地图
  const auto Waypoints = CarlaMap->GenerateWaypointsOnRoadEntries(); // 生成路网上的生成点
  for (const auto &Wp : Waypoints) // 遍历每个生成点
  {
    const FTransform Trans = CarlaMap->ComputeTransform(Wp); // 计算生成点的变换
    AVehicleSpawnPoint *Spawner = GetWorld()->SpawnActor<AVehicleSpawnPoint>(); // 生成一个新的VehicleSpawnPoint
    Spawner->SetActorRotation(Trans.GetRotation()); // 设置生成点的旋转
    Spawner->SetActorLocation(Trans.GetTranslation() + FVector(0.f, 0.f, SpawnersHeight)); // 设置生成点的位置
    VehicleSpawners.Add(Spawner); // 将新生成的Spawner添加到列表中
  }
}

// 生成所有内容
void AOpenDriveGenerator::GenerateAll()
{
  GenerateRoadMesh(); // 生成道路网格
  GenerateSpawnPoints(); // 生成生成点
  GeneratePoles(); // 生成杆子
}

// 开始游戏时调用
void AOpenDriveGenerator::BeginPlay()
{
  Super::BeginPlay(); // 调用父类的BeginPlay

  const FString XodrContent = UOpenDrive::GetXODR(GetWorld()); // 获取XODR内容
  LoadOpenDrive(XodrContent); // 加载OpenDrive

  GenerateAll(); // 生成所有内容

  auto World = GetWorld(); // 获取世界
  check(World != nullptr); // 检查世界是否为空

  // Autogenerate signals
  AActor* TrafficLightManagerActor =  UGameplayStatics::GetActorOfClass(World, ATrafficLightManager::StaticClass()); // 获取TrafficLightManagerActor
  if(TrafficLightManagerActor == nullptr) // 如果不存在
  {
    World->SpawnActor<ATrafficLightManager>(); // 生成一个新的TrafficLightManagerActor
  }
}
