// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

/// 用于标识ProceduralMapGenerator所使用的网格的标签
///
///只要我们拥有的网格数少于255个，它就能正常工作，目前是蓝图阶段
/// 类型枚举仅支持 uint8
UENUM(BlueprintType)
enum class ECityMapMeshTag : uint8
{
  RoadTwoLanes_LaneLeft               UMETA(DisplayName = "Road: Two Lanes - Lane Left"),
  RoadTwoLanes_LaneRight              UMETA(DisplayName = "Road: Two Lanes - Lane Right"),
  RoadTwoLanes_SidewalkLeft           UMETA(DisplayName = "Road: Two Lanes - Sidewalk Left"),
  RoadTwoLanes_SidewalkRight          UMETA(DisplayName = "Road: Two Lanes - Sidewalk Right"),
  RoadTwoLanes_LaneMarkingSolid       UMETA(DisplayName = "Road: Two Lanes - Lane Marking Solid"),
  RoadTwoLanes_LaneMarkingBroken      UMETA(DisplayName = "Road: Two Lanes - Lane Marking Broken"),

  Road90DegTurn_Lane0                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 0"),
  Road90DegTurn_Lane1                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 1"),
  Road90DegTurn_Lane2                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 2"),
  Road90DegTurn_Lane3                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 3"),
  Road90DegTurn_Lane4                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 4"),
  Road90DegTurn_Lane5                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 5"),
  Road90DegTurn_Lane6                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 6"),
  Road90DegTurn_Lane7                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 7"),
  Road90DegTurn_Lane8                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 8"),
  Road90DegTurn_Lane9                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 9"),
  Road90DegTurn_Sidewalk0             UMETA(DisplayName = "Road: 90 Degree Turn - Sidewalk 0"),
  Road90DegTurn_Sidewalk1             UMETA(DisplayName = "Road: 90 Degree Turn - Sidewalk 1"),
  Road90DegTurn_Sidewalk2             UMETA(DisplayName = "Road: 90 Degree Turn - Sidewalk 2"),
  Road90DegTurn_Sidewalk3             UMETA(DisplayName = "Road: 90 Degree Turn - Sidewalk 3"),
  Road90DegTurn_LaneMarking           UMETA(DisplayName = "Road: 90 Degree Turn - Lane Marking"),

  RoadTIntersection_Lane0             UMETA(DisplayName = "Road: T-Intersection - Lane 0"),
  RoadTIntersection_Lane1             UMETA(DisplayName = "Road: T-Intersection - Lane 1"),
  RoadTIntersection_Lane2             UMETA(DisplayName = "Road: T-Intersection - Lane 2"),
  RoadTIntersection_Lane3             UMETA(DisplayName = "Road: T-Intersection - Lane 3"),
  RoadTIntersection_Lane4             UMETA(DisplayName = "Road: T-Intersection - Lane 4"),
  RoadTIntersection_Lane5             UMETA(DisplayName = "Road: T-Intersection - Lane 5"),
  RoadTIntersection_Lane6             UMETA(DisplayName = "Road: T-Intersection - Lane 6"),
  RoadTIntersection_Lane7             UMETA(DisplayName = "Road: T-Intersection - Lane 7"),
  RoadTIntersection_Lane8             UMETA(DisplayName = "Road: T-Intersection - Lane 8"),
  RoadTIntersection_Lane9             UMETA(DisplayName = "Road: T-Intersection - Lane 9"),
  RoadTIntersection_Sidewalk0         UMETA(DisplayName = "Road: T-Intersection - Sidewalk 0"),
  RoadTIntersection_Sidewalk1         UMETA(DisplayName = "Road: T-Intersection - Sidewalk 1"),
  RoadTIntersection_Sidewalk2         UMETA(DisplayName = "Road: T-Intersection - Sidewalk 2"),
  RoadTIntersection_Sidewalk3         UMETA(DisplayName = "Road: T-Intersection - Sidewalk 3"),
  RoadTIntersection_LaneMarking       UMETA(DisplayName = "Road: T-Intersection - Lane Marking"),

  RoadXIntersection_Lane0             UMETA(DisplayName = "Road: X-Intersection - Lane 0"),
  RoadXIntersection_Lane1             UMETA(DisplayName = "Road: X-Intersection - Lane 1"),
  RoadXIntersection_Lane2             UMETA(DisplayName = "Road: X-Intersection - Lane 2"),
  RoadXIntersection_Lane3             UMETA(DisplayName = "Road: X-Intersection - Lane 3"),
  RoadXIntersection_Lane4             UMETA(DisplayName = "Road: X-Intersection - Lane 4"),
  RoadXIntersection_Lane5             UMETA(DisplayName = "Road: X-Intersection - Lane 5"),
  RoadXIntersection_Lane6             UMETA(DisplayName = "Road: X-Intersection - Lane 6"),
  RoadXIntersection_Lane7             UMETA(DisplayName = "Road: X-Intersection - Lane 7"),
  RoadXIntersection_Lane8             UMETA(DisplayName = "Road: X-Intersection - Lane 8"),
  RoadXIntersection_Lane9             UMETA(DisplayName = "Road: X-Intersection - Lane 9"),
  RoadXIntersection_Sidewalk0         UMETA(DisplayName = "Road: X-Intersection - Sidewalk 0"),
  RoadXIntersection_Sidewalk1         UMETA(DisplayName = "Road: X-Intersection - Sidewalk 1"),
  RoadXIntersection_Sidewalk2         UMETA(DisplayName = "Road: X-Intersection - Sidewalk 2"),
  RoadXIntersection_Sidewalk3         UMETA(DisplayName = "Road: X-Intersection - Sidewalk 3"),
  RoadXIntersection_LaneMarking       UMETA(DisplayName = "Road: X-Intersection - Lane Marking"),

  NUMBER_OF_TAGS         UMETA(Hidden),
  INVALID                UMETA(Hidden)
};

///用于处理 ECityMapMeshTag 的辅助类
class CityMapMeshTag
{
public:

  ///返回标签的数量
  static constexpr uint8 GetNumberOfTags() {
    return ToUInt(ECityMapMeshTag::NUMBER_OF_TAGS);
  }

  /// 返回基础网格。基础网格定义了地图缩放的单位图块
  static ECityMapMeshTag GetBaseMeshTag();

  /// 获取道路交叉口一侧的图块大小。即，返回N，使得NxN
  /// 大小相当于一个道路交叉口的部分
  static uint32 GetRoadIntersectionSize();

  /// @名称 标签转换
  /// @{

  /// Convert @a Tag to an unsigned integer type.
  static constexpr uint8 ToUInt(ECityMapMeshTag Tag) {
    return static_cast<uint8>(Tag);
  }

  /// 将@a标签转换为无符号整数类型
  static ECityMapMeshTag FromUInt(uint8 Value) {
    check(Value < GetNumberOfTags());
    return static_cast<ECityMapMeshTag>(Value);
  }

  /// 获取一个标签名称作为 FString
  static FString ToString(ECityMapMeshTag Tag);

  /// 将 @a Value 转换为 ECityMapMeshTag 并获取其名称作为 FString
  static FString ToString(uint8 Value) {
    return ToString(FromUInt(Value));
  }

  /// @}
};
