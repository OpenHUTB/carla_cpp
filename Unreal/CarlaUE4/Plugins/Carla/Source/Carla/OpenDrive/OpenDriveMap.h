// 版权所有(c) 2019 Universitat Autonoma 计算机视觉中心 （cvc）
// 巴塞罗那 (UAB)
//
// 本作品根据 MIT 许可证的条款进行许可
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>

#pragma once

// 引入禁用和启用 UE4 宏的头文件
#include <compiler/disable-ue4-macros.h>
// 引入 CARLA 道路地图模块的头文件
#include <carla/road/Map.h>
#include <compiler/enable-ue4-macros.h>

#include "OpenDriveMap.generated.h"

// 用于表示道路上的航路点的结构体
// ‘FWaypoint’ 是一个 BlueprintType 结构体，允许它在蓝图中被使用
USTRUCT(BlueprintType)
struct CARLA_API FWaypoint
{
  GENERATED_BODY()

// ‘Waypoint’ 保存实际的 CARLA 道路航路点数据
  carla::road::element::Waypoint Waypoint;
};

// 用于表示航路点之间连接的结构体
// 该结构体包含起点和终点航路点信息
USTRUCT(BlueprintType)
struct CARLA_API FWaypointConnection
{
  GENERATED_BODY()

// 起点航路点，供蓝图读写
  UPROPERTY(BlueprintReadWrite)
  FWaypoint Start;

// 终点航路点，供蓝图读写
  UPROPERTY(BlueprintReadWrite)
  FWaypoint End;
};

/// 将 CARLA OpenDrive API 公开给蓝图的辅助类。
// 该类代表一张地图，并提供与道路航点、拓扑生成等相关的函数
UCLASS(BlueprintType, Blueprintable)
class CARLA_API UOpenDriveMap : public UObject
{
  GENERATED_BODY()

public:

 // 构造函数，初始化 OpenDrive 地图对象
  UOpenDriveMap(const FObjectInitializer &ObjectInitializer);

  /// 返回此映射是否已初始化。
  /// \return 如果地图已初始化返回 true，否则返回 false
  UFUNCTION(BlueprintCallable)
  bool HasMap() const
  {
    return Map.IsSet();
  }

  /// 使用OpenDrive（XODR）文件加载此映射。
   /// 给定位置，返回距离该位置最近的道路上的航路点
  /// \param Location 查询位置
  /// \param Success 如果操作成功，‘Success’ 被设置为 true，否则为 false
  /// \return 距离该位置最近的航路点
  UFUNCTION(BlueprintCallable)
  bool Load(const FString &XODRContent);

  /// 给定一个位置，返回车道中心最近的点。
   /// \param ApproxDistance 航路点之间的近似距离（默认为 100.0f）
  /// \return 生成的航路点数组
  UFUNCTION(BlueprintCallable)
  FWaypoint GetClosestWaypointOnRoad(FVector Location, bool &Success) const;

  /// 在地图上以近似距离生成航路点。
  /// \return 生成的航路点连接数组，表示车道入口的拓扑结构
  UFUNCTION(BlueprintCallable)
  TArray<FWaypoint> GenerateWaypoints(float ApproxDistance = 100.0f) const;

  /// 生成定义此拓扑的最小航路点集地图。航点位于每条车道的入口处。
  UFUNCTION(BlueprintCallable)
  TArray<FWaypointConnection> GenerateTopology() const;

  /// 在每条道路的起点，在每条车道上生成航点。
  UFUNCTION(BlueprintCallable)
  TArray<FWaypoint> GenerateWaypointsOnRoadEntries() const;

  /// 计算航点的位置。
  UFUNCTION(BlueprintCallable)
  FVector ComputeLocation(FWaypoint Waypoint) const;

  /// 计算航点数组的位置。
  UFUNCTION(BlueprintCallable)
  TArray<FVector> ComputeLocations(const TArray<FWaypoint> &Waypoints) const;

  /// 计算航点的变换。x轴指向该航点的道路方向。
  UFUNCTION(BlueprintCallable)
  FTransform ComputeTransform(FWaypoint Waypoint) const;

  /// 计算航点数组的变换。
  UFUNCTION(BlueprintCallable)
  TArray<FTransform> ComputeTransforms(const TArray<FWaypoint> &Waypoints) const;

  /// 返回给定距离上的航点列表，以便位于航点的车辆可以行驶得到。
  UFUNCTION(BlueprintCallable)
  TArray<FWaypoint> GetNext(FWaypoint Waypoint, float Distance = 100.0f) const;

private:

 // 存储地图的对象，采用 Optional 类型表示地图可能未初始化
  TOptional<carla::road::Map> Map;
};
