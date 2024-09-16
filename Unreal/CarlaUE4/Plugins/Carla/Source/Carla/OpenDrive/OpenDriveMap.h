// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <compiler/disable-ue4-macros.h>
#include <carla/road/Map.h>
#include <compiler/enable-ue4-macros.h>

#include "OpenDriveMap.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FWaypoint
{
  GENERATED_BODY()

  carla::road::element::Waypoint Waypoint;
};

USTRUCT(BlueprintType)
struct CARLA_API FWaypointConnection
{
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite)
  FWaypoint Start;

  UPROPERTY(BlueprintReadWrite)
  FWaypoint End;
};

/// 将 CARLA OpenDrive API 公开给蓝图的辅助类。
UCLASS(BlueprintType, Blueprintable)
class CARLA_API UOpenDriveMap : public UObject
{
  GENERATED_BODY()

public:

  UOpenDriveMap(const FObjectInitializer &ObjectInitializer);

  /// 返回此映射是否已初始化。
  UFUNCTION(BlueprintCallable)
  bool HasMap() const
  {
    return Map.IsSet();
  }

  /// 使用OpenDrive（XODR）文件加载此映射。
  UFUNCTION(BlueprintCallable)
  bool Load(const FString &XODRContent);

  /// 给定一个位置，返回车道中心最近的点。
  UFUNCTION(BlueprintCallable)
  FWaypoint GetClosestWaypointOnRoad(FVector Location, bool &Success) const;

  /// 在地图上以近似距离生成航路点。
  UFUNCTION(BlueprintCallable)
  TArray<FWaypoint> GenerateWaypoints(float ApproxDistance = 100.0f) const;

  /// 生成定义此拓扑的最小航路点集
  /// 地图。航路点位于每条车道的入口处。
  UFUNCTION(BlueprintCallable)
  TArray<FWaypointConnection> GenerateTopology() const;

  /// Generate waypoints on each lane at the start of each road.
  UFUNCTION(BlueprintCallable)
  TArray<FWaypoint> GenerateWaypointsOnRoadEntries() const;

  /// Compute the location of a waypoint.
  UFUNCTION(BlueprintCallable)
  FVector ComputeLocation(FWaypoint Waypoint) const;

  /// Compute the locations of an array of waypoints.
  UFUNCTION(BlueprintCallable)
  TArray<FVector> ComputeLocations(const TArray<FWaypoint> &Waypoints) const;

  /// Compute the transform of a waypoint. The X-axis is directed towards the
  /// direction of the road at that waypoint.
  UFUNCTION(BlueprintCallable)
  FTransform ComputeTransform(FWaypoint Waypoint) const;

  /// Compute the transforms of an array of waypoints.
  UFUNCTION(BlueprintCallable)
  TArray<FTransform> ComputeTransforms(const TArray<FWaypoint> &Waypoints) const;

  /// Return the list of waypoints at a given distance such that a vehicle at
  /// waypoint could drive to.
  UFUNCTION(BlueprintCallable)
  TArray<FWaypoint> GetNext(FWaypoint Waypoint, float Distance = 100.0f) const;

private:

  TOptional<carla::road::Map> Map;
};
