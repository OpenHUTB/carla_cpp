// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "UObject/NoExportTypes.h"
#include "MapGen/CityMapMeshTag.h"
#include "RoadMap.generated.h"

/// 路线图交叉路口结果。请参阅 URoadMap。
USTRUCT(BlueprintType)
struct CARLA_API FRoadMapIntersectionResult
{
  GENERATED_BODY()

  /// 箱子在越野时的百分比。
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  float OffRoad;

  ///框侵入对面车道的百分比（方向错误）。
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  float OppositeLane;
};

/// 数据存储在路线图像素中。请参阅 URoadMap。
struct FRoadMapPixelData
{
  friend class URoadMap;

  constexpr static int IsRoadRow = 15;

  constexpr static int HasDirectionRow = 14;

  constexpr static uint16 MaximumEncodedAngle = (1 << 14) - 1;

  constexpr static uint16 AngleMask = (0xFFFF >> 2);

public:

  explicit FRoadMapPixelData(uint16 inValue) : Value(inValue) {}

  ///此像素是否位于道路上。
  bool IsRoad() const
  {
    return (Value & (1 << IsRoadRow)) != 0;
  }

  ///此像素是否定义了方向（例如，道路交叉口为
  ///不是越野，但都没有明确的方向）。
  bool HasDirection() const
  {
    return (Value & (1 << HasDirectionRow)) != 0;
  }

  /// 获取道路方向的方位角 [-PI， PI]（球面
  ///坐标）的 S Package。
  ///
  /// 未定义 if ！HasDirection（）.
  float GetDirectionAzimuthalAngle() const
  {
    const float Angle = AngleMask & Value;
    // 在内部，角度由 PI 旋转。
    return (Angle * 2.0f * PI / MaximumEncodedAngle) - PI;
  }

  ///获取此像素处的道路方向。
  ///
  /// 为定义 if !HasDirection().
  FVector GetDirection() const
  {
    const FVector2D SphericalCoords(HALF_PI, GetDirectionAzimuthalAngle());
    return SphericalCoords.SphericalToUnitCartesian();
  }

  FColor EncodeAsColor() const;

private:

  static uint16 Encode(bool IsRoad, bool HasDirection, const FVector &Direction);

  uint16 Value;
};

/// 关卡路线图。包含哪些区域是道路的 2D 信息
///和车道方向。
UCLASS()
class CARLA_API URoadMap : public UObject
{
  GENERATED_BODY()

public:

  /// 创建有效的空地图（每个点都是越野的）。
  URoadMap(const FObjectInitializer& ObjectInitializer);

  /// 重置当前映射 - 初始化给定大小的空映射。
  void Reset(
      uint32 Width,
      uint32 Height,
      float PixelsPerCentimeter,
      const FTransform &WorldToMap,
      const FVector &MapOffset);

  void SetPixelAt(
      uint32 PixelX,
      uint32 PixelY,
      ECityMapMeshTag Tag,
      const FTransform &Transform,
      bool bInvertDirection = false);

  uint32 GetWidth() const
  {
    return Width;
  }

  uint32 GetHeight() const
  {
    return Height;
  }

  /// 返回给定像素的世界位置。
  FVector GetWorldLocation(uint32 PixelX, uint32 PixelY) const;

  /// 检索存储在给定像素处的数据。
  FRoadMapPixelData GetDataAt(uint32 PixelX, uint32 PixelY) const
  {
    check(IsValid());
    return FRoadMapPixelData(RoadMapData[GetIndex(PixelX, PixelY)]);
  }

  /// 如果位于映射限制之外，则钳制值。
  FRoadMapPixelData GetDataAt(const FVector &WorldLocation) const;

  /// 将角色边界与地图相交。
  ///
  ///边界框将投影到地图上，并尽可能地对照它进行检查
  ///与越野区域和对面车道的交叉路口.
  FRoadMapIntersectionResult Intersect(
      const FTransform &BoxTransform,
      const FVector &BoxExtent,
      float ChecksPerCentimeter) const;

  ///将当前地图另存为 PNG，并将像素数据编码为颜色。
  bool SaveAsPNG(const FString &Folder, const FString &MapName) const;

#if WITH_EDITOR

  /// 将映射的状态记录到控制台。
  void Log() const;

  /// 将图像的每个像素绘制为调试点。
  void DrawDebugPixelsToLevel(UWorld *World, bool bJustFlushDoNotDraw = false) const;

#endif // WITH_EDITOR

private:

  int32 GetIndex(uint32 PixelX, uint32 PixelY) const
  {
    return PixelX + Width * PixelY;
  }

  bool IsValid() const
  {
    return ((RoadMapData.Num() > 0) && (RoadMapData.Num() == Height * Width));
  }

  /// 世界到地图的转换。
  UPROPERTY(VisibleAnywhere)
  FTransform WorldToMap;

  /// 地图坐标中的偏移量。
  UPROPERTY(VisibleAnywhere)
  FVector MapOffset;

  /// 每厘米像素数。
  UPROPERTY(VisibleAnywhere)
  float PixelsPerCentimeter;

  /// 地图的宽度（以像素为单位）。
  UPROPERTY(VisibleAnywhere)
  uint32 Width;

  /// 地图的高度（以像素为单位）。
  UPROPERTY(VisibleAnywhere)
  uint32 Height;

  UPROPERTY()
  TArray<uint16> RoadMapData;
};
