// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/MapGen/RoadMap.h"

#include "Carla/Sensor/PixelReader.h"

#include "FileHelper.h"
#include "HighResScreenshot.h"

// 仅在编辑器环境下包含相关调试绘制头文件，用于在编辑器中进行一些辅助的可视化调试操作
#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif // WITH_EDITOR

#include <type_traits>

// 定义本地化文本命名空间，这里命名为"CarlaRoadMap"，用于处理与道路地图相关的本地化文本信息
#define LOCTEXT_NAMESPACE "CarlaRoadMap"

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

// 静态函数：将浮点数限制在指定的整数范围内并转换为无符号整数
// @param Value 要进行转换的浮点数
// @param Min 限制范围的下限（整数）
// @param Max 限制范围的上限（整数）
// @return 转换后的无符号32位整数，该整数在[Min, Max]范围内
static uint32 ClampFloatToUInt(const float Value, int32 Min, int32 Max)
// 使用FMath库中的Clamp函数先将浮点数转换为整数并限制在[Min, Max]范围内，然后返回无符号32位整数形式
{
  return FMath::Clamp(FMath::FloorToInt(Value), Min, Max);
}
// 静态函数：返回旋转后的方位角（在球坐标下），使其范围在[0, 2*PI]
// @param Direction 输入的方向向量，用于计算其方位角
// @return 旋转后的方位角（浮点数），范围在[0, 2*PI]

// 返回经过 π 弧度旋转后的方位角（在球坐标系下），使其处于 [0, 2π] 这个区间范围内。
// 处于 [0, 2π] 这个区间范围内。
static float GetRotatedAzimuthAngle(const FVector &Direction)
{
  // 将输入的笛卡尔坐标形式的方向向量转换为球坐标形式，得到球坐标的向量表示
  const FVector2D SphericalCoords = Direction.UnitCartesianToSpherical();
  // 将球坐标中的方位角（Y分量）加上PI，实现旋转效果，并返回旋转后的方位角
  return SphericalCoords.Y + PI;
}

// =============================================================================
// -- FRoadMapPixelData --------------------------------------------------------
// =============================================================================

// FRoadMapPixelData类的成员函数：对道路地图像素数据进行编码
// @param IsRoad 表示该像素是否对应道路的布尔值
// @param HasDirection 表示该像素对应的道路是否有方向信息的布尔值
// @param Direction 若HasDirection为真，则为对应的道路方向向量，否则为零向量
// @return 编码后的无符号16位整数，包含了道路、方向及角度等信息
uint16 FRoadMapPixelData::Encode(bool IsRoad, bool HasDirection, const FVector &Direction)
{
  // 根据输入的方向向量计算对应的角度，并将其转换为无符号整数形式，用于编码
  const uint16 AngleAsUInt = MaximumEncodedAngle * GetRotatedAzimuthAngle(Direction) / (2.0f * PI);
  // 检查编码后的角度是否符合特定条件，即不与表示道路行的标志位冲突（通过位运算判断）
  check(!(AngleAsUInt & (1 << IsRoadRow)));
  // 检查编码后的角度是否符合特定条件，即不与表示方向行的标志位冲突（通过位运算判断）
  check(!(AngleAsUInt & (1 << HasDirectionRow)));
  // 将道路、方向和角度信息通过位运算进行编码组合，返回最终的编码值
  return (IsRoad << IsRoadRow) | (HasDirection << HasDirectionRow) | (AngleAsUInt);
}

// FRoadMapPixelData类的成员函数：将道路地图像素数据编码为颜色值
// @return 编码后的颜色值，用于在可视化等场景中表示像素信息
FColor FRoadMapPixelData::EncodeAsColor() const
{
  // 如果该像素不对应道路
  if (!IsRoad()) {
    // 返回黑色（RGB值都为0，透明度为255，即完全不透明）作为表示非道路像素的颜色
    return FColor(0u, 0u, 0u, 255u);
  } else if (!HasDirection()) {
    // 如果该像素对应道路但没有方向信息
    // 返回白色（RGB值都为255，透明度为255，即完全不透明）作为表示无方向道路像素的颜色
    return FColor(255u, 255u, 255u, 255u);
  } else {
    // 定义一个lambda函数，用于将输入的浮点数转换为适合作为颜色分量的值（范围在0 - 255）
    auto ToColor = [](float X){
      return FMath::FloorToInt(256.0 * (X + PI) / (2.0f * PI)) % 256;
    };
    // 获取道路方向的方位角
    const float Azimuth = GetDirectionAzimuthalAngle();
    // 根据上述转换规则，生成一个表示有方向道路像素的颜色值，其中蓝色分量根据方位角变化，红色为0，绿色为255
    return FColor(0u, 255u, ToColor(Azimuth), 255u);
  }
}

// =============================================================================
// -- URoadMap -----------------------------------------------------------------
// =============================================================================

// URoadMap类的构造函数，用于初始化道路地图对象的一些默认属性
// @param ObjectInitializer 对象初始化参数，用于传递给父类构造函数等初始化操作
URoadMap::URoadMap(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer),
  // 每厘米对应的像素数，初始化为1.0f
  PixelsPerCentimeter(1.0f),
  // 地图的宽度，初始化为1个单位（这里单位未明确指定，可能根据具体应用场景而定）
  Width(1u),
  // 地图的高度，初始化为1个单位（这里单位未明确指定，可能根据具体应用场景而定）
  Height(1u)
{
  // 向道路地图数据容器中添加一个初始值为0的数据元素，可能用于后续的填充或初始化操作
  RoadMapData.Add(0u);
  static_assert(
      // 静态断言，用于在编译时检查FRoadMapPixelData::Value的类型与RoadMapData容器中元素类型是否匹配
      // 如果不匹配，编译器会给出相应的错误提示信息
      std::is_same<decltype(FRoadMapPixelData::Value), typename decltype(RoadMapData)::ElementType>::value,
      "Declaration map of FRoadMapPixelData's value does not match current serialization type");
}

// URoadMap类的成员函数：重置道路地图的相关参数和数据
// @param inWidth 新的地图宽度（无符号32位整数）
// @param inHeight 新的地图高度（无符号32位整数）
// @param inPixelsPerCentimeter 新的每厘米对应的像素数（浮点数）
// @param inWorldToMap 世界坐标到地图坐标的变换信息（FTransform类型）
// @param inMapOffset 地图在世界坐标中的偏移量（FVector类型）
void URoadMap::Reset(
    const uint32 inWidth,
    const uint32 inHeight,
    const float inPixelsPerCentimeter,
    const FTransform &inWorldToMap,
    const FVector &inMapOffset)
{
  // 初始化道路地图数据容器，将其中的元素都设置为0，容器大小为新的宽度和高度的乘积
  RoadMapData.Init(0u, inWidth * inHeight);
  Width = inWidth;
  Height = inHeight;
  PixelsPerCentimeter = inPixelsPerCentimeter;
  WorldToMap = inWorldToMap;
  MapOffset = inMapOffset;
}

// URoadMap类的成员函数：在指定像素位置设置道路地图的像素数据
// @param PixelX 要设置像素数据的像素在X轴上的坐标（无符号32位整数）
// @param PixelY 要设置像素数据的像素在Y轴上的坐标（无符号32位整数）
// @param Tag 城市地图网格标签，用于确定道路类型等相关信息（ECityMapMeshTag类型）
// @param Transform 与该像素相关的变换信息（FTransform类型）
// @param bInvertDirection 是否反转方向的布尔值，用于根据具体情况调整道路方向
void URoadMap::SetPixelAt(
    const uint32 PixelX,
    const uint32 PixelY,
    const ECityMapMeshTag Tag,
    const FTransform &Transform,
    const bool bInvertDirection)
{
  bool bIsRoad = false;
  bool bHasDirection = false;
  FVector Direction(0.0f, 0.0f, 0.0f);

  // 获取输入的变换信息中的旋转部分，并将其转换为旋转器（Rotator）类型，以便后续处理角度等信息
  auto Rotator = Transform.GetRotation().Rotator();

  switch (Tag) {
    default:
      // 如果标签为默认情况（未在下面的具体分支中匹配到），则认为该像素位置不是道路
      break;
    // 以下是各种道路类型对应的处理分支，根据不同的道路类型设置是否为道路、是否有方
    case ECityMapMeshTag::RoadTwoLanes_LaneRight:
    case ECityMapMeshTag::Road90DegTurn_Lane1:
    case ECityMapMeshTag::RoadTIntersection_Lane1:
    case ECityMapMeshTag::RoadTIntersection_Lane9:
    case ECityMapMeshTag::RoadXIntersection_Lane1:
    case ECityMapMeshTag::RoadXIntersection_Lane9:
      bIsRoad = true;
      bHasDirection = true;
      // 对于这些道路类型，将旋转器的偏航角（Yaw）增加180.0f，以调整方向向量的方向
      Rotator.Yaw += 180.0f;
      break;
    case ECityMapMeshTag::RoadTwoLanes_LaneLeft:
    case ECityMapMeshTag::Road90DegTurn_Lane0:
    case ECityMapMeshTag::RoadTIntersection_Lane0:
    case ECityMapMeshTag::RoadTIntersection_Lane2:
    case ECityMapMeshTag::RoadTIntersection_Lane5:
    case ECityMapMeshTag::RoadTIntersection_Lane8:
    case ECityMapMeshTag::RoadXIntersection_Lane0:
    case ECityMapMeshTag::RoadXIntersection_Lane8:
      bIsRoad = true;
      bHasDirection = true;
      break;
    case ECityMapMeshTag::Road90DegTurn_Lane9:
    case ECityMapMeshTag::RoadTIntersection_Lane7:
    case ECityMapMeshTag::RoadXIntersection_Lane7:
    case ECityMapMeshTag::RoadXIntersection_Lane5:
      bIsRoad = true;
      bHasDirection = true;
      // 对于这些道路类型，将旋转器的偏航角（Yaw）增加90.0f，以调整方向向量的方向
      Rotator.Yaw += 90.0f;
      break;
    case ECityMapMeshTag::Road90DegTurn_Lane7:
      bIsRoad = true;
      bHasDirection = true;
      // 对于这种道路类型，将旋转器的偏航角（Yaw）增加90.0f（可能还需要进一步调整，注释中有 + 15.5f 的提示，但未实际应用）
      Rotator.Yaw += 90.0f; //+ 15.5f;
      break;
    case ECityMapMeshTag::Road90DegTurn_Lane5:
      bIsRoad = true;
      bHasDirection = true;
      // 对于这种道路类型，将旋转器的偏航角（Yaw）增加90.0f + 35.0f，以调整方向向量的方向
      Rotator.Yaw += 90.0f + 35.0f;
      break;
    case ECityMapMeshTag::Road90DegTurn_Lane3:
      bIsRoad = true;
      bHasDirection = true;
      // 对于这种道路类型，将旋转器的偏航角（Yaw）增加90.0f + 45.0f + 20.5f，以调整方向向量的方向
      Rotator.Yaw += 90.0f + 45.0f + 20.5f;
      break;
    case ECityMapMeshTag::Road90DegTurn_Lane8:
    case ECityMapMeshTag::RoadTIntersection_Lane4:
    case ECityMapMeshTag::RoadXIntersection_Lane2:
    case ECityMapMeshTag::RoadXIntersection_Lane4:
      bIsRoad = true;
      bHasDirection = true;
      // 对于这些道路类型，将旋转器的偏航角（Yaw）增加270.0f，以调整方向向量的方向
      Rotator.Yaw += 270.0f;
      break;
    case ECityMapMeshTag::Road90DegTurn_Lane6:
      bIsRoad = true;
      bHasDirection = true;
      // 对于这种道路类型，将旋转器的偏航角（Yaw）增加270.0f + 50.0f，以调整方向向量的方向
      Rotator.Yaw += 270.0f + 50.0f;
      break;
    case ECityMapMeshTag::Road90DegTurn_Lane4:
      bIsRoad = true;
      bHasDirection = true;
      // 对于这种道路类型，将旋转器的偏航角（Yaw）增加270.0f + 80.0f，以调整方向向量的方向
      Rotator.Yaw += 270.0f + 80.0f;
      break;
    case ECityMapMeshTag::Road90DegTurn_Lane2:
      bIsRoad = true;
      bHasDirection = true;
      //Rotator.Yaw += 270.0f + 70.0f;
      break;
    case ECityMapMeshTag::RoadTIntersection_Lane3:
    case ECityMapMeshTag::RoadTIntersection_Lane6:
    case ECityMapMeshTag::RoadXIntersection_Lane3:
    case ECityMapMeshTag::RoadXIntersection_Lane6:
      bIsRoad = true;
      bHasDirection = false;
      break;
  }
  if (bHasDirection) {
    // 如果该像素对应的道路有方向信息，则根据旋转器创建一个四元数（Quat）表示的旋转
    FQuat Rotation(Rotator);
    // 获取旋转后的方向向量，即四元数的前向向量
    Direction = Rotation.GetForwardVector();
    if (bInvertDirection) {
      // 如果需要反转方向，则将方向向量取反
      Direction *= -1.0f;
    }
  }
  const auto Value = FRoadMapPixelData::Encode(bIsRoad, bHasDirection, Direction);
  RoadMapData[GetIndex(PixelX, PixelY)] = Value;
}

// URoadMap类的成员函数：根据像素坐标获取对应的世界坐标位置
// @param PixelX 像素在X轴上的坐标（无符号32位整数）
// @param PixelY 像素在Y轴上的坐标（无符号32位整数）
// @return 对应的世界坐标位置（FVector类型）
FVector URoadMap::GetWorldLocation(uint32 PixelX, uint32 PixelY) const
{
  const FVector RelativePosition(
      static_cast<float>(PixelX) / PixelsPerCentimeter,
      static_cast<float>(PixelY) / PixelsPerCentimeter,
      0.0f);
  return WorldToMap.InverseTransformPosition(RelativePosition + MapOffset);
}

// URoadMap类的成员函数：根据世界坐标位置获取对应的道路地图像素数据
// @param WorldLocation 要获取像素数据对应的世界坐标位置（FVector类型）
// @return 对应的道路地图像素数据（FRoadMapPixelData类型）
FRoadMapPixelData URoadMap::GetDataAt(const FVector &WorldLocation) const
{
  check(IsValid());
  const FVector Location = WorldToMap.TransformPosition(WorldLocation) - MapOffset;
  uint32 X = ClampFloatToUInt(PixelsPerCentimeter * Location.X, 0, Width - 1);
  uint32 Y = ClampFloatToUInt(PixelsPerCentimeter * Location.Y, 0, Height - 1);
  return GetDataAt(X, Y);
}

// URoadMap类的成员函数：计算道路地图与一个长方体的相交情况
// @param BoxTransform 长方体的变换信息（FTransform类型），用于确定长方体在世界坐标中的位置和姿态
// @param BoxExtent 长方体的范围（FVector类型），表示长方体在各个坐标轴方向上的尺寸大小
// @param ChecksPerCentimeter 每厘米的检查次数（浮点数），用于控制相交检测的精度
// @return 相交结果结构体
FRoadMapIntersectionResult URoadMap::Intersect(
    const FTransform &BoxTransform,
    const FVector &BoxExtent,
    float ChecksPerCentimeter) const
{
  auto DirectionOfMovement = BoxTransform.GetRotation().GetForwardVector();
  DirectionOfMovement.Z = 0.0f; // Project to XY plane (won't be normalized anymore).
  uint32 CheckCount = 0u;
  FRoadMapIntersectionResult Result = {0.0f, 0.0f};
  const float Step = 1.0f / ChecksPerCentimeter;
  for (float X = -BoxExtent.X; X < BoxExtent.X; X += Step) {
    for (float Y = -BoxExtent.Y; Y < BoxExtent.Y; Y += Step) {
      ++CheckCount;
      auto Location = BoxTransform.TransformPosition(FVector(X, Y, 0.0f));
      const auto &Data = GetDataAt(Location);
      if (!Data.IsRoad()) {
        Result.OffRoad += 1.0f;
      } else if (Data.HasDirection() &&
                 0.0f > FVector::DotProduct(Data.GetDirection(), DirectionOfMovement)) {
        Result.OppositeLane += 1.0f;
      }
    }
  }
  // 如果CheckCount大于0（这里的0u表示无符号的0）
  if (CheckCount > 0u) {
    // 将Result结构体中的OffRoad成员除以CheckCount（先将CheckCount转换为float类型）
    Result.OffRoad /= static_cast<float>(CheckCount);
    // 将Result结构体中的OppositeLane成员除以CheckCount（同样先转换类型）
    Result.OppositeLane /= static_cast<float>(CheckCount);
  } else {
    UE_LOG(LogCarla, Warning, TEXT("URoadMap::Intersect did zero checks"));
  }
  return Result;
}

// 声明一个函数，用于将道路地图数据保存为PNG图片和元数据文件。
bool URoadMap::SaveAsPNG(const FString &Folder, const FString &MapName) const
{
  // 如果道路地图数据无效，则记录错误日志并返回false
  if (!IsValid()) {
    UE_LOG(LogCarla, Error, TEXT("Cannot save invalid road map to disk"));
    return false;
  }

  // 构造PNG图片文件的完整路径。
  const FString ImagePath = FPaths::Combine(Folder, MapName + TEXT(".png"));
  // 构造元数据文件的完整路径。
  const FString MetadataPath = FPaths::Combine(Folder, MapName + TEXT(".txt"));
  
  // 设置目标图像的尺寸。
  const FIntPoint DestSize(Width, Height);
  // 创建一个指向TImagePixelData<FColor>的智能指针，用于存储像素数据。
  TUniquePtr<TImagePixelData<FColor>> PixelData = MakeUnique<TImagePixelData<FColor>>(DestSize);  
  // 为像素数据预留空间。
  PixelData->Pixels.Reserve(RoadMapData.Num());
  // 遍历道路地图数据，将每个数据点转换为颜色，并添加到像素数据中。
  for (auto Value : RoadMapData) {
    PixelData->Pixels.Emplace(FRoadMapPixelData(Value).EncodeAsColor());
  }
  // 将像素数据保存到磁盘上的PNG文件。
  FPixelReader::SavePixelsToDisk(std::move(PixelData), ImagePath);

  // 保存元数据
  FFormatNamedArguments Args;
  Args.Add("MapName", FText::FromString(MapName));
  Args.Add("Width", GetWidth());
  Args.Add("Height", GetHeight());
  Args.Add("CmPerPixel", 1.0f / PixelsPerCentimeter);
  Args.Add("Transform", FText::FromString(WorldToMap.ToString()));
  Args.Add("Offset", FText::FromString(MapOffset.ToString()));
   
  // 使用格式化参数对象和格式化字符串来创建元数据内容。
  const auto Contents = FText::Format(
      LOCTEXT("RoadMapMetadata",
          "Map name = {MapName}\n"
          "Size = {Width}x{Height} pixels\n"
          "Density = {CmPerPixel} cm/pixel\n"
          "World-To-Map Transform (T|R|S) = ({Transform})\n"
          "Map Offset = ({Offset})\n"),
      Args); 
  // 将元数据内容保存到文本文件，如果失败则记录错误日志。
  if (!FFileHelper::SaveStringToFile(Contents.ToString(), *MetadataPath)) {
    UE_LOG(LogCarla, Error, TEXT("Failed to save map metadata"));
  }
 
  // 记录日志，表示道路地图已保存。
  UE_LOG(LogCarla, Log, TEXT("Saved road map to \"%s\""), *ImagePath);
  return true;
}

#if WITH_EDITOR

// 一个成员函数，用于在编辑器中记录道路地图的相关信息。
void URoadMap::Log() const
{
  // 计算地图数据的大小（以MB为单位）。
  const float MapSizeInMB = // Only map data, not the class itself.
      static_cast<float>(sizeof(decltype(RoadMapData)::ElementType) * RoadMapData.Num()) /
      (1024.0f * 1024.0f);
  // 记录地图的尺寸、大小和比例尺信息。
  UE_LOG(
      LogCarla,
      Log,
      TEXT("Generated road map %dx%d (%.2fMB) with %.2f cm/pixel"),
      GetWidth(),
      GetHeight(),
      MapSizeInMB,
      1.0f / PixelsPerCentimeter);

  // 如果道路地图数据无效，则记录错误日志并返回。
  if (!IsValid()) {
    UE_LOG(LogCarla, Error, TEXT("Error generating road map"));
    return;
  }
}

// 一个成员函数，用于在编辑器中绘制调试像素。
void URoadMap::DrawDebugPixelsToLevel(UWorld *World, const bool bJustFlushDoNotDraw) const
{ 
  // 定义Z轴偏移量。
  const FVector ZOffset(0.0f, 0.0f, 50.0f);
  // 清除持久的调试线条。
  FlushPersistentDebugLines(World); 
  // 如果不只刷新而不绘制，则直接返回。
  if (!bJustFlushDoNotDraw) {
    // 遍历地图的每个像素点。
    for (auto X = 0u; X < Width; ++X) {
      for (auto Y = 0u; Y < Height; ++Y) { 
        // 获取当前像素点的世界位置，并添加Z轴偏移量。
        auto Location = GetWorldLocation(X, Y) + ZOffset; 
        // 获取当前像素点的数据。
        const auto &Data = GetDataAt(X, Y);
        // 获取当前像素点的颜色。
        auto Color = Data.EncodeAsColor();
        // 如果数据包含方向信息，则绘制方向箭头，否则绘制点。
        if (Data.HasDirection()) {
          const FVector ArrowEnd = Location + 50.0f * Data.GetDirection();
          DrawDebugDirectionalArrow(World, Location, ArrowEnd, 60.0f, Color, true);
        } else {
          DrawDebugPoint(World, Location, 6.0f, Color, true);
        }
      }
    }
  }
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE
