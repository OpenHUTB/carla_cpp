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

// 定义一个私有命名空间UOpenDriveMap_Private，用于存放模板函数
namespace UOpenDriveMap_Private {

  // 模板函数，用于获取任意范围类型的尺寸
  template <typename RangeT>
  static auto GetSize(const RangeT &Range)
  {
    return Range.size();
  }

  // 模板函数，专门用于获取TArray的尺寸
  template <typename T>
  static auto GetSize(const TArray<T> &Array)
  {
    return Array.Num();
  }


  // 模板函数，用于将任意范围转换为TArray，应用给定的转换函数
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

  // 模板函数，用于将任意范围转换为TArray，使用默认的转换函数
  template <typename T, typename RangeT>
  static TArray<T> TransformToTArray(RangeT &&Range)
  {
    return TransformToTArray<T>(
        std::forward<RangeT>(Range),
        [](auto &&Item) { return T{Item}; });
  }

} // 以UOpenDriveMap_Private命名的命名空间，命名空间作用是防止同名函数的冲突，并且使程序员更好的区分同名函数。

// UOpenDriveMap类的构造函数
UOpenDriveMap::UOpenDriveMap(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer) {}

// UOpenDriveMap类的成员函数，用于加载OpenDrive内容
bool UOpenDriveMap::Load(const FString &XODRContent)
{
  // 使用carla的OpenDriveParser加载OpenDrive内容
  auto ResultMap = carla::opendrive::OpenDriveParser::Load(
      carla::rpc::FromLongFString(XODRContent));
  if (ResultMap)
  {
    // 如果加载成功，移动结果到Map成员变量
    Map = std::move(*ResultMap);
  }
  // 返回是否有地图数据
  return HasMap();
}

// UOpenDriveMap类的成员函数，用于获取道路上最近的路点
FWaypoint UOpenDriveMap::GetClosestWaypointOnRoad(FVector Location, bool &Success) const
{
  // 检查是否有地图数据
  check(HasMap());
  // 获取最近的路点
  auto Result = Map->GetClosestWaypointOnRoad(Location);
  // 设置是否成功的标记
  Success = Result.has_value();
  // 返回路点，如果没有找到则返回空的FWaypoint
  return Result.has_value() ? FWaypoint{*Result} : FWaypoint{};
}

// UOpenDriveMap类的成员函数，用于生成路点数组
TArray<FWaypoint> UOpenDriveMap::GenerateWaypoints(float ApproxDistance) const
{
  // 如果ApproxDistance小于1.0f，记录错误日志并返回空数组
  if (ApproxDistance < 1.0f)
  {
    UE_LOG(LogCarla, Error, TEXT("GenerateWaypoints: Please provide an ApproxDistance greater than 1 centimetre."));
    return {};
  }
  // 检查是否有地图数据
  check(HasMap());
  // 使用私有命名空间中的模板函数生成路点数组
  using namespace UOpenDriveMap_Private;
  return TransformToTArray<FWaypoint>(Map->GenerateWaypoints(ApproxDistance / 1e2f));
}

/// 根据OpenDrive生成地图的拓扑
TArray<FWaypointConnection> UOpenDriveMap::GenerateTopology() const
{
  check(HasMap());
   // 使用私有命名空间
  using namespace UOpenDriveMap_Private;
  // 将Map的拓扑信息转换为FWaypointConnection数组
  return TransformToTArray<FWaypointConnection>(Map->GenerateTopology(), [](auto &&Item) {
    return FWaypointConnection{FWaypoint{Item.first}, FWaypoint{Item.second}};
  });
}

/// 基于道路的入口生成航点
TArray<FWaypoint> UOpenDriveMap::GenerateWaypointsOnRoadEntries() const
{
  check(HasMap());
  using namespace UOpenDriveMap_Private;
  // 将Map的道路入口信息转换为FWaypoint数组
  return TransformToTArray<FWaypoint>(Map->GenerateWaypointsOnRoadEntries());
}

/// 计算航点的位置。
FVector UOpenDriveMap::ComputeLocation(FWaypoint Waypoint) const
{
  // 返回航点变换的位置
  return ComputeTransform(Waypoint).GetLocation();
}

/// 计算航点数组的位置。
TArray<FVector> UOpenDriveMap::ComputeLocations(const TArray<FWaypoint> &Waypoints) const
{
  using namespace UOpenDriveMap_Private;
  // 将航点数组转换为位置数组
  return TransformToTArray<FVector>(Waypoints, [this](auto &&Waypoint) {
    return ComputeLocation(Waypoint);
  });
}

/// 计算航点的变换。x轴指向该航点的道路方向。
FTransform UOpenDriveMap::ComputeTransform(FWaypoint Waypoint) const
{
  check(HasMap());
  using namespace UOpenDriveMap_Private;
  // 计算航点的变换
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
  // 获取给定距离上的航点列表
  return TransformToTArray<FWaypoint>(Map->GetNext(Waypoint.Waypoint, Distance / 1e2f));
}
