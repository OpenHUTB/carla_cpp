// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"

#include "Carla/MapGen/SoilTypeManager.h"
#include "Containers/Array.h"
#include "Containers/EnumAsByte.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/IntPoint.h"
#include "ProceduralFoliageSpawner.h"
#include "Templates/UnrealTypeTraits.h"
#include "UObject/NoExportTypes.h"

#include "RegionOfInterest.generated.h"

// 定义一个可以用于蓝图系统的枚举类型 ERegionOfInterestType
// 用来表示不同类型的感兴趣区域（Region of Interest, ROI）
UENUM(BlueprintType)
enum ERegionOfInterestType
{
  NONE_REGION,
  TERRAIN_REGION,
  WATERBODIES_REGION,  // 尚未支持
  VEGETATION_REGION,
  MISC_SPREADED_ACTORS_REGION,
  MISC_SPECIFIC_LOCATION_ACTORS_REGION,
  SOIL_TYPE_REGION
};
// 定义一个可以用于蓝图系统的枚举类型 ESpreadedActorsDensity
// 用来表示区域内分散演员（spreaded actors）的密度级别
UENUM(BlueprintType)
enum ESpreadedActorsDensity
{
  LOW,
  MEDIUM,
  HIGH
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FRoiTile
{
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite)
  int X;
  UPROPERTY(BlueprintReadWrite)
  int Y;

public:
// 默认构造函数，初始化 X 和 Y 为 -1，表示无效或未指定的瓦片
  FRoiTile() : X(-1), Y(-1)
  {};
  // 构造函数，接受两个整数参数 X 和 Y，用来初始化瓦片的具体位置
  FRoiTile(int X, int Y)
  {
    this->X = X;// 使用 this 指针来区分成员变量和参数名称
    this->Y = Y;
  };
  // 拷贝构造函数，使用另一个 FRoiTile 对象的 X 和 Y 来初始化新的对象
  // 这里调用了带参数的构造函数来进行初始化
  FRoiTile(const FRoiTile& Other)
    : FRoiTile(Other.X, Other.Y)
  {}
  // 重载等号运算符 (==)，允许直接比较两个 FRoiTile 对象
  // 它通过调用 Equals 方法来进行比较，并返回比较结果
  bool operator==(const FRoiTile& Other) const
  {
    return Equals(Other);
  }

  bool Equals(const FRoiTile& Other) const
  {
    return (this->X == Other.X) && (this->Y == Other.Y);
  }

  /// 一个返回位于当前瓦片上方的瓦片的函数。
  FORCEINLINE FRoiTile Up()
  {
    // return FRoiTile(X, Y-1);
    return FRoiTile(X-1, Y);
  }

  /// 一个返回位于当前瓦片下方的瓦片的函数。
  FORCEINLINE FRoiTile Down()
  {
    // return FRoiTile(X, Y+1);
    return FRoiTile(X+1, Y);
  }

  ///一个返回位于当前瓦片左侧的瓦片的函数。
  FORCEINLINE FRoiTile Left()
  {
    // return FRoiTile(X-1, Y);
    return FRoiTile(X, Y+1);
  }

  ///一个返回位于当前瓦片右侧的瓦片的函数。
  FORCEINLINE FRoiTile Right()
  {
    // return FRoiTile(X+1, Y);
    return FRoiTile(X, Y-1);
  }
};

///一个用于对FRoiTile结构体进行哈希处理的函数。
/// 它被用于对该结构体进行哈希处理，以便其能在TMap中用作键。
FORCEINLINE uint32 GetTypeHash(const FRoiTile& Thing)
{
  uint32 Hash = FCrc::MemCrc32(&Thing, sizeof(FRoiTile));
  return Hash;
}

/**
 *
 */
USTRUCT(BlueprintType)
struct CARLATOOLS_API FCarlaRegionOfInterest
{
	GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite)
  TArray<FRoiTile> TilesList;

  UPROPERTY(BlueprintReadWrite)
  TEnumAsByte<ERegionOfInterestType> RegionType = ERegionOfInterestType::NONE_REGION;

  FCarlaRegionOfInterest()
  {
    TilesList.Empty();
  }

  void AddTile(int X, int Y)
  {
    FRoiTile Tile(X,Y);
    TilesList.Add(Tile);
  }

  TEnumAsByte<ERegionOfInterestType> GetRegionType()
  {
    return this->RegionType;
  }

  //一个模板函数，用于检查一个瓦片是否在区域映射图中。
  template <typename R>
  static FORCEINLINE bool IsTileInRegionsSet(FRoiTile RoiTile, TMap<FRoiTile, R> RoisMap)
  {
    static_assert(TIsDerivedFrom<R, FCarlaRegionOfInterest>::IsDerived,
        "ROIs Map Value type is not an URegionOfInterest derived type.");
    return RoisMap.Contains(RoiTile);
  }

  /// 检查两个感兴趣的区域是否相等。
  bool Equals(const FCarlaRegionOfInterest& Other)
  {
    // 检查这两个区域中的瓦片数量是否相同。
    if(this->TilesList.Num() != Other.TilesList.Num())
    {
      return false;
    }

    // 检查这两个区域是否具有相同的瓦片。
    TMap<FRoiTile, int> TileCount;
    for(FRoiTile Tile : Other.TilesList)
    {
      if(TileCount.Contains(Tile))
        TileCount[Tile]++;
      else
        TileCount.Add(Tile, 1);
    }

    for(FRoiTile Tile : TilesList)
    {
      if(!TileCount.Contains(Tile))
        return false;

      TileCount[Tile]--;

      if(TileCount[Tile] == 0)
        TileCount.Remove(Tile);
    }

    if(TileCount.Num() == 0)
      return true;
    else
      return false;
  }

};

// 定义了一个用于表示植被感兴趣区域（Region of Interest, ROI）的结构体，继承自FCarlaRegionOfInterest。
USTRUCT(BlueprintType)
struct CARLATOOLS_API FVegetationROI : public FCarlaRegionOfInterest
{
  GENERATED_BODY()

// 一个数组，用于存储指向植被生成器的指针。
  UPROPERTY(BlueprintReadWrite)
  TArray<UProceduralFoliageSpawner*> FoliageSpawners;

  // FVegetationROI的构造函数，初始化FoliageSpawners为空。
  FVegetationROI() : FCarlaRegionOfInterest()
  {
    this->FoliageSpawners.Empty();
  }

// 一个函数，用于将一个植被生成器添加到FoliageSpawners数组中。
  void AddFoliageSpawner(UProceduralFoliageSpawner* Spawner)
  {
    FoliageSpawners.Add(Spawner);
  }

  // 一个将一组生成器添加到感兴趣区域（ROI）的生成器列表中的函数。
void AddFoliageSpawners(TArray<UProceduralFoliageSpawner*> Spawners)
  {
    for(UProceduralFoliageSpawner* Spawner : Spawners)
    {
      AddFoliageSpawner(Spawner);
    }
  }

// 一个函数，用于获取FoliageSpawners数组。
  TArray<UProceduralFoliageSpawner*> GetFoliageSpawners()
  {
    return this->FoliageSpawners;
  }
};

// 定义了一个用于表示地形感兴趣区域（ROI）的结构体，继承自FCarlaRegionOfInterest。
USTRUCT(BlueprintType)
struct CARLATOOLS_API FTerrainROI : public FCarlaRegionOfInterest
{
  GENERATED_BODY()

  // 一个指向材质实例的指针，该材质实例用于更改感兴趣区域（ROI）的高度图材质。
UPROPERTY(BlueprintReadWrite)
  UMaterialInstanceDynamic* RoiMaterialInstance;

  // 一个用于存储感兴趣区域（ROI）高度图的渲染目标。
 UPROPERTY(BlueprintReadWrite)
  UTextureRenderTarget2D* RoiHeightmapRenderTarget;

// FTerrainROI的构造函数，初始化RoiMaterialInstance和RoiHeightmapRenderTarget。
  FTerrainROI() : FCarlaRegionOfInterest(), RoiMaterialInstance(), RoiHeightmapRenderTarget()
  {}

  /**
 * This function checks if a tile is on the boundary of a region of interest
 *
 * @param RoiTile The tile we're checking
 * @param RoisMap The map of RoiTiles to Rois.
 * @param OutUp Is there a tile above this one?
 * @param OutRight Is there a ROI to the right of this tile?
 * @param OutDown Is there a ROI tile below this one?
 * @param OutLeft Is the tile to the left of the current tile in the RoiMap?
 *
 * return true if the tile is in a boundary
 */
 // 一个静态函数，用于检查一个瓦片是否在ROI的边界上。
  template <typename R>
  static bool IsTileInRoiBoundary(FRoiTile RoiTile, TMap<FRoiTile, R> RoisMap, bool& OutUp, bool& OutRight, bool& OutDown, bool& OutLeft)
  {
    FTerrainROI ThisRoi = RoisMap[RoiTile];
    OutUp   = RoisMap.Contains(RoiTile.Up()) && ThisRoi.Equals(RoisMap[RoiTile.Up()]);
    OutDown = RoisMap.Contains(RoiTile.Down()) && ThisRoi.Equals(RoisMap[RoiTile.Down()]);
    OutLeft = RoisMap.Contains(RoiTile.Left()) && ThisRoi.Equals(RoisMap[RoiTile.Left()]);
    OutRight = RoisMap.Contains(RoiTile.Right()) && ThisRoi.Equals(RoisMap[RoiTile.Right()]);

    return !OutUp || !OutDown || !OutLeft || !OutRight;
  }
};

// 定义了一个用于表示散布演员感兴趣区域（ROI）的结构体，继承自FCarlaRegionOfInterest。
USTRUCT(BlueprintType)
struct CARLATOOLS_API FMiscSpreadedActorsROI : public FCarlaRegionOfInterest
{
  GENERATED_BODY()

// 一个指向演员类的指针，表示在ROI中生成的演员类型。
  UPROPERTY(BlueprintReadWrite)
  TSubclassOf<AActor> ActorClass;

// 一个浮点数，表示在ROI中生成演员的概率。
  UPROPERTY(BlueprintReadWrite)
  float Probability;
 
// 一个枚举值，表示在ROI中散布演员的密度。
  UPROPERTY(BlueprintReadWrite)
  TEnumAsByte<ESpreadedActorsDensity> ActorsDensity;

  FMiscSpreadedActorsROI() : FCarlaRegionOfInterest(), ActorClass(), Probability(0.0f), ActorsDensity(ESpreadedActorsDensity::LOW)
  {}
};

/// 一个用于存储特定位置生成演员的感兴趣区域相关信息的结构体。
USTRUCT(BlueprintType)
struct CARLATOOLS_API FMiscSpecificLocationActorsROI : public FCarlaRegionOfInterest
{
  GENERATED_BODY()

// 一个指向演员类的指针，表示在特定位置生成的演员类型。
  UPROPERTY(BlueprintReadWrite)
  TSubclassOf<AActor> ActorClass;

// 一个向量，表示演员在世界中的生成位置。
  UPROPERTY(BlueprintReadWrite)
  FVector ActorLocation;

// 一个浮点数，表示演员最小旋转范围。
  UPROPERTY(BlueprintReadWrite)
  float MinRotationRange;

  UPROPERTY(BlueprintReadWrite)
  float MaxRotationRange;

  FMiscSpecificLocationActorsROI() : FCarlaRegionOfInterest(),
    ActorClass(),
    ActorLocation(0.0f),
    MinRotationRange(0.0f),
    MaxRotationRange(0.0f)
  {}
};

// 定义了一个用于表示土壤类型感兴趣区域（ROI）的结构体，继承自FCarlaRegionOfInterest。
USTRUCT(BlueprintType)
struct CARLATOOLS_API FSoilTypeROI : public FCarlaRegionOfInterest
{
  GENERATED_BODY()

// 一个土壤类型属性结构体，包含土壤的物理属性。
  UPROPERTY(BlueprintReadWrite)
  FSoilTerramechanicsProperties SoilProperties;

// FSoilTypeROI的构造函数，初始化SoilProperties。
  FSoilTypeROI() : SoilProperties()
  {}
};

// 使用 USTRUCT 宏定义一个可以用于蓝图系统的结构体 FMiscWidgetState
// CARLATOOLS_API 是一个宏，通常用于导出类或结构体以便在其他模块中使用
USTRUCT(BlueprintType)
struct CARLATOOLS_API FMiscWidgetState
{
  GENERATED_USTRUCT_BODY()
	
// IsPersistentState 是一个布尔值属性，表示小部件状态是否持久化
// 可以在编辑器中编辑，并且可以在蓝图中读取和修改
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary|Misc")
  bool IsPersistentState;
	
// InTileCoordinates 是一个 FIntPoint 类型的属性，表示小部件在瓦片坐标系中的位置
  // FIntPoint 是一个包含两个整数坐标的结构体，通常用于表示二维平面上的点
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary|Misc")
  FIntPoint InTileCoordinates;
};
