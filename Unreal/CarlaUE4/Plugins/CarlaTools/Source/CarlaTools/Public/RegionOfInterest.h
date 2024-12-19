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
  FRoiTile() : X(-1), Y(-1)
  {};

  FRoiTile(int X, int Y)
  {
    this->X = X;
    this->Y = Y;
  };

  FRoiTile(const FRoiTile& Other)
    : FRoiTile(Other.X, Other.Y)
  {}

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

USTRUCT(BlueprintType)
struct CARLATOOLS_API FVegetationROI : public FCarlaRegionOfInterest
{
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite)
  TArray<UProceduralFoliageSpawner*> FoliageSpawners;

  FVegetationROI() : FCarlaRegionOfInterest()
  {
    this->FoliageSpawners.Empty();
  }

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

  TArray<UProceduralFoliageSpawner*> GetFoliageSpawners()
  {
    return this->FoliageSpawners;
  }
};

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

USTRUCT(BlueprintType)
struct CARLATOOLS_API FMiscSpreadedActorsROI : public FCarlaRegionOfInterest
{
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite)
  TSubclassOf<AActor> ActorClass;

  UPROPERTY(BlueprintReadWrite)
  float Probability;

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

  UPROPERTY(BlueprintReadWrite)
  TSubclassOf<AActor> ActorClass;

  UPROPERTY(BlueprintReadWrite)
  FVector ActorLocation;

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

USTRUCT(BlueprintType)
struct CARLATOOLS_API FSoilTypeROI : public FCarlaRegionOfInterest
{
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite)
  FSoilTerramechanicsProperties SoilProperties;

  FSoilTypeROI() : SoilProperties()
  {}
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FMiscWidgetState
{
  GENERATED_USTRUCT_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary|Misc")
  bool IsPersistentState;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MapGenerator|JsonLibrary|Misc")
  FIntPoint InTileCoordinates;
};
