// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/OpenDrive/OpenDriveActor.h"

#include "Carla/OpenDrive/OpenDrive.h"

#include "UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"

#include "ConstructorHelpers.h"
#include "Materials/MaterialExpressionTextureSample.h"

#include "DrawDebugHelpers.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/geom/Math.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/element/Waypoint.h>
#include <carla/rpc/String.h>
#include <compiler/enable-ue4-macros.h>

#include <algorithm>
#include <unordered_set>

AOpenDriveActor::AOpenDriveActor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;

  // 用来保存一次性初始化的结构
  static struct FConstructorStatics
  {
    // 用来从资源包里寻找目标渲染纹理图片的工具类
    ConstructorHelpers::FObjectFinderOptional<UTexture2D> TextureObject;
    FName Category;
    FText Name;
    FConstructorStatics()
    // 使用这个静态类(结构体)找到渲染纹理图片的路径
      : TextureObject(TEXT("/Carla/Icons/OpenDriveActorIcon")),
        Category(TEXT("OpenDriveActor")),
        Name(NSLOCTEXT("SpriteCategory", "OpenDriveActor", "OpenDriveActor"))
    {}
  } ConstructorStatics;

  // 我们需要一个场景组件来附加 Icon sprite
  USceneComponent *SceneComponent =
      ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComp"));
  RootComponent = SceneComponent;
  RootComponent->Mobility = EComponentMobility::Static;

#if WITH_EDITORONLY_DATA
  SpriteComponent =
      ObjectInitializer.CreateEditorOnlyDefaultSubobject<UBillboardComponent>(this, TEXT("Sprite"));
  if (SpriteComponent)
  {
    // 从辅助类对象中获取sprite纹理。
    SpriteComponent->Sprite = ConstructorStatics.TextureObject.Get();
    // 分配sprite类别名称。 
    SpriteComponent->SpriteInfo.Category = ConstructorStatics.Category;
    // 设置sprite显示名称
    SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.Name;
    // 将sprite附加到场景组件。
    SpriteComponent->SetupAttachment(RootComponent);
    SpriteComponent->Mobility = EComponentMobility::Static;
    SpriteComponent->SetEditorScale(1.f);
  }
#endif // WITH_EDITORONLY_DATA
}

// 如果编译器配置为使用编辑器（即在编辑器环境中）
#if WITH_EDITOR
// AOpenDriveActor类的成员函数，用于处理属性更改后的事件
void AOpenDriveActor::PostEditChangeProperty(struct FPropertyChangedEvent &Event)
{
  // 调用父类的PostEditChangeProperty函数
  Super::PostEditChangeProperty(Event);

   // 获取发生更改的属性名称
  const FName PropertyName = (Event.Property != NULL ? Event.Property->GetFName() : NAME_None);
   // 如果更改的属性是bGenerateRoutes
  if (PropertyName == GET_MEMBER_NAME_CHECKED(AOpenDriveActor, bGenerateRoutes))
  {
    // 如果bGenerateRoutes被设置为true
    if (bGenerateRoutes)
    {
      bGenerateRoutes = false;

      RemoveRoutes(); //  移除现有的路线，避免OpenDrive重叠避免OpenDrive重叠
      RemoveSpawners(); // 如果OpenDRIVE发生了变化，则重新启动生成器。
      BuildRoutes();// 构建新的路线

      // 如果bAddSpawners为true，则添加生成器
      if (bAddSpawners)
      {
        AddSpawners();
      }
      // 如果bShowDebug为true，则显示调试信息
      if (bShowDebug)
      {
        DebugRoutes();
      }
    }
  }
  // 如果更改的属性是bRemoveRoutes
  if (PropertyName == GET_MEMBER_NAME_CHECKED(AOpenDriveActor, bRemoveRoutes))
  {
    // 如果bRemoveRoutes被设置为true
    if (bRemoveRoutes)
    {
      // 将bRemoveRoutes设置为false，以避免重复执行
      bRemoveRoutes = false;

      RemoveDebugRoutes();// 移除调试路线
      RemoveSpawners();// 移除生成器
      RemoveRoutes(); // 移除路线
    }
  }
  // 如果更改的属性是bShowDebug
  if (PropertyName == GET_MEMBER_NAME_CHECKED(AOpenDriveActor, bShowDebug))
  {
    // 如果bShowDebug被设置为true，则显示调试信息
    if (bShowDebug)
    {
      DebugRoutes();
    }
    // 如果bShowDebug被设置为false，则移除调试信息
    else
    {
      RemoveDebugRoutes();
    }
  }
  // 如果更改的属性是bRemoveCurrentSpawners
  if (PropertyName == GET_MEMBER_NAME_CHECKED(AOpenDriveActor, bRemoveCurrentSpawners))
  {
    // 如果bRemoveCurrentSpawners被设置为true
    if (bRemoveCurrentSpawners)
    {
      // 将bRemoveCurrentSpawners设置为false，以避免重复执行
      bRemoveCurrentSpawners = false;

      // 移除生成器
      RemoveSpawners();
    }
  }
}
#endif // WITH_EDITOR

void AOpenDriveActor::BuildRoutes()
{
  BuildRoutes(GetWorld()->GetMapName());
}

void AOpenDriveActor::BuildRoutes(FString MapName)
{
  using Waypoint = carla::road::element::Waypoint;

  // 由于OpenDRIVE文件与关卡名称相同，因此使用关卡名称和游戏内容目录构建xodr文件的路径。
  const FString XodrContent = UOpenDrive::LoadXODR(MapName);

  auto map = carla::opendrive::OpenDriveParser::Load(carla::rpc::FromLongFString(XodrContent));

  if (!map.has_value())
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to parse OpenDrive file."));
    return;
  }

  // 包含地图每条车道末端路点的一个列表
  const std::vector<Waypoint> LaneWaypoints =
      map->GenerateWaypointsOnRoadEntries();

  std::unordered_map<Waypoint, std::vector<Waypoint>> PredecessorMap;

  for (auto &Wp : LaneWaypoints)
  {
    const auto PredecessorsList = map->GetPredecessors(Wp);
    if (PredecessorsList.empty())
    {
      continue;
    }
    const auto MinRoadId = *std::min_element(
        PredecessorsList.begin(),
        PredecessorsList.end(),
        [](const auto &WaypointA, const auto &WaypointB) {
          return WaypointA.road_id < WaypointB.road_id;
        });
    PredecessorMap[MinRoadId].emplace_back(Wp);
  }

  for (auto &&PredecessorWp : PredecessorMap)
  {
    ARoutePlanner *RoutePlanner = nullptr;

    for (auto &&Wp : PredecessorWp.second)
    {
      std::vector<Waypoint> Waypoints;
      auto CurrentWp = Wp;

      do
      {
        Waypoints.emplace_back(CurrentWp);
        const auto Successors = map->GetNext(CurrentWp, RoadAccuracy);
        if (Successors.empty())
        {
          break;
        }
        if (Successors.front().road_id != Wp.road_id)
        {
          break;
        }
        CurrentWp = Successors.front();
      } while (CurrentWp.road_id == Wp.road_id);

      // 将当前道路的最后一个路点连接到下一条道路的第一个路点。
      const auto FollowingWp = map->GetSuccessors(CurrentWp);
      if (!FollowingWp.empty())
      {
        Waypoints.emplace_back(FollowingWp.front());
      }

      if (Waypoints.size() >= 2)
      {
        TArray<FVector> Positions;
        Positions.Reserve(Waypoints.size());
        for (int i = 0; i < Waypoints.size(); ++i)
        {
          // 添加触发器高度，因为点的Z坐标不影响驾驶员AI，并且在编辑器中易于可视化。
          Positions.Add(map->ComputeTransform(Waypoints[i]).location +
              FVector(0.f, 0.f, TriggersHeight));
        }

        // 如果路线规划器不存在，则创建它。
        if (RoutePlanner == nullptr )
        {
          const auto WpTransform = map->ComputeTransform(Wp);
          RoutePlanner = GetWorld()->SpawnActor<ARoutePlanner>();
          RoutePlanner->bIsIntersection = map->IsJunction(Wp.road_id);
          RoutePlanner->SetBoxExtent(FVector(70.f, 70.f, 50.f));
          RoutePlanner->SetActorRotation(WpTransform.rotation);
          RoutePlanner->SetActorLocation(WpTransform.location +
              FVector(0.f, 0.f, TriggersHeight));
        }

        if (RoutePlanner != nullptr)
        {
          RoutePlanner->AddRoute(1.f, Positions);
          RoutePlanners.Add(RoutePlanner);
        }
      }
    }
  }
}

void AOpenDriveActor::RemoveRoutes()
{
  const int rp_num = RoutePlanners.Num();
  for (int i = 0; i < rp_num; i++)
  {
    if (RoutePlanners[i] != nullptr)
    {
      RoutePlanners[i]->Destroy();
    }
  }
  RoutePlanners.Empty();
}

void AOpenDriveActor::DebugRoutes() const
{
  for (int i = 0; i < RoutePlanners.Num(); ++i)
  {
    if (RoutePlanners[i] != nullptr)
    {
      RoutePlanners[i]->DrawRoutes();
    }
  }
}

void AOpenDriveActor::RemoveDebugRoutes() const
{
#if WITH_EDITOR
  FlushPersistentDebugLines(GetWorld());
#endif // WITH_EDITOR
}

void AOpenDriveActor::AddSpawners()
{
  for (int i = 0; i < RoutePlanners.Num(); ++i)
  {
    if (RoutePlanners[i] != nullptr)
    {
      if (!bOnIntersections && RoutePlanners[i]->bIsIntersection)
      {
        continue;
      }
      else
      {
        FTransform Trans = RoutePlanners[i]->GetActorTransform();
        AVehicleSpawnPoint *Spawner = GetWorld()->SpawnActor<AVehicleSpawnPoint>();
        Spawner->SetActorRotation(Trans.GetRotation());
        Spawner->SetActorLocation(Trans.GetTranslation() + FVector(0.f, 0.f, SpawnersHeight));
        VehicleSpawners.Add(Spawner);
      }
    }
  }
}

void AOpenDriveActor::RemoveSpawners()
{
  const int vs_num = VehicleSpawners.Num();
  for (int i = 0; i < vs_num; i++)
  {
    if (VehicleSpawners[i] != nullptr)
    {
      VehicleSpawners[i]->Destroy();
    }
  }
  VehicleSpawners.Empty();
}
