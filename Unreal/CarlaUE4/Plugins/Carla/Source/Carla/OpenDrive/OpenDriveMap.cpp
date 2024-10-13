// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/OpenDrive/OpenDriveMap.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/rpc/String.h>
#include <compiler/enable-ue4-macros.h>

namespace UOpenDriveMap_Private {

  template <typename RangeT>
  static auto GetSize(const RangeT &Range)
  {
    return Range.size();
  }

  template <typename T>
  static auto GetSize(const TArray<T> &Array)
  {
    return Array.Num();
  }


  template <typename T, typename RangeT, typename FuncT>
  static TArray<T> TransformToTArray(RangeT &&Range, FuncT &&TransformFunction)
  {
    TArray<T> Result;
    Result.Reserve(GetSize(Range));
    for (auto &&Item : Range)
    {
      Result.Emplace(TransformFunction(Item));
    }
    return Result;
  }

  template <typename T, typename RangeT>
  static TArray<T> TransformToTArray(RangeT &&Range)
  {
    return TransformToTArray<T>(
        std::forward<RangeT>(Range),
        [](auto &&Item) { return T{Item}; });
  }

} // 以UOpenDriveMap_Private命名的命名空间，命名空间作用是防止同名函数的冲突，并且使程序员更好的区分同名函数。

UOpenDriveMap::UOpenDriveMap(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer) {}

bool UOpenDriveMap::Load(const FString &XODRContent)
{
  auto ResultMap = carla::opendrive::OpenDriveParser::Load(
      carla::rpc::FromLongFString(XODRContent));
  if (ResultMap)
  {
    Map = std::move(*ResultMap);
  }
  return HasMap();
}

FWaypoint UOpenDriveMap::GetClosestWaypointOnRoad(FVector Location, bool &Success) const
{
  check(HasMap());
  auto Result = Map->GetClosestWaypointOnRoad(Location);
  Success = Result.has_value();
  return Result.has_value() ? FWaypoint{*Result} : FWaypoint{};
}

TArray<FWaypoint> UOpenDriveMap::GenerateWaypoints(float ApproxDistance) const
{
  if (ApproxDistance < 1.0f)
  {
    UE_LOG(LogCarla, Error, TEXT("GenerateWaypoints: Please provide an ApproxDistance greater than 1 centimetre."));
    return {};
  }
  check(HasMap());
  using namespace UOpenDriveMap_Private;
  return TransformToTArray<FWaypoint>(Map->GenerateWaypoints(ApproxDistance / 1e2f));
}

/// 根据OpenDrive生成地图的拓扑
TArray<FWaypointConnection> UOpenDriveMap::GenerateTopology() const
{
  check(HasMap());
  using namespace UOpenDriveMap_Private;
  return TransformToTArray<FWaypointConnection>(Map->GenerateTopology(), [](auto &&Item) {
    return FWaypointConnection{FWaypoint{Item.first}, FWaypoint{Item.second}};
  });
}

/// 基于道路的入口生成航点
TArray<FWaypoint> UOpenDriveMap::GenerateWaypointsOnRoadEntries() const
{
  check(HasMap());
  using namespace UOpenDriveMap_Private;
  return TransformToTArray<FWaypoint>(Map->GenerateWaypointsOnRoadEntries());
}

/// 计算航点的位置。
FVector UOpenDriveMap::ComputeLocation(FWaypoint Waypoint) const
{
  return ComputeTransform(Waypoint).GetLocation();
}

/// 计算航点数组的位置。
TArray<FVector> UOpenDriveMap::ComputeLocations(const TArray<FWaypoint> &Waypoints) const
{
  using namespace UOpenDriveMap_Private;
  return TransformToTArray<FVector>(Waypoints, [this](auto &&Waypoint) {
    return ComputeLocation(Waypoint);
  });
}

/// 计算航点的变换。x轴指向该航点的道路方向。
FTransform UOpenDriveMap::ComputeTransform(FWaypoint Waypoint) const
{
  check(HasMap());
  using namespace UOpenDriveMap_Private;
  return Map->ComputeTransform(Waypoint.Waypoint);
}

/// 计算航点数组的变换。
TArray<FTransform> UOpenDriveMap::ComputeTransforms(const TArray<FWaypoint> &Waypoints) const
{
  using namespace UOpenDriveMap_Private;
  return TransformToTArray<FTransform>(Waypoints, [this](auto &&Waypoint) {
    return ComputeTransform(Waypoint);
  });
}

/// 返回给定距离上的航点列表，以便位于航点的车辆可以行驶得到。
TArray<FWaypoint> UOpenDriveMap::GetNext(FWaypoint Waypoint, float Distance) const
{
  // 距离必须大于1厘米
  if (Distance < 1.0f)
  {
    UE_LOG(LogCarla, Error, TEXT("GetNext: Please provide a Distance greater than 1 centimetre."));
    return {};
  }
  check(HasMap());
  using namespace UOpenDriveMap_Private;
  return TransformToTArray<FWaypoint>(Map->GetNext(Waypoint.Waypoint, Distance / 1e2f));
}
