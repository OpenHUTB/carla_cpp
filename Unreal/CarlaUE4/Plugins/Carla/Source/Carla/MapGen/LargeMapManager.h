// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Actor/CarlaActor.h"
#include "GameFramework/Actor.h"

#include "Engine/LevelStreamingDynamic.h"

#include "Math/DVector.h"

#include "LargeMapManager.generated.h"


//待办事项：缓存Carla剧集。

USTRUCT()
struct FActiveActor
{
  GENERATED_BODY()

  FActiveActor() {}

  FActiveActor(
    const FCarlaActor* InCarlaActor,
    const FTransform& InTransform)
    : CarlaActor(InCarlaActor),
      WorldLocation(FDVector(InTransform.GetTranslation())),
      Rotation(InTransform.GetRotation()) {}

  const FCarlaActor* CarlaActor;

  FDVector WorldLocation;

  FQuat Rotation;
};

USTRUCT(BlueprintType)
struct FCarlaMapTile
{
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Carla Map Tile")
  FString Name; // Tile_{TileID_X}_{TileID_Y}

  // 绝对位置不依赖于重新定基准。
  UPROPERTY(VisibleAnywhere,  BlueprintReadWrite, Category = "Carla Map Tile")
  FVector Location{0.0f};
  // 待办事项：不是 F 向量

  UPROPERTY(VisibleAnywhere,  BlueprintReadWrite, Category = "Carla Map Tile")
  ULevelStreamingDynamic* StreamingLevel = nullptr;

  bool TilesSpawned = false;
};

UCLASS()
class CARLA_API ALargeMapManager : public AActor
{
  GENERATED_BODY()

public:

  using TileID = uint64;

  //为该角色的属性设置默认值。

  ALargeMapManager();

  ~ALargeMapManager();

protected:
  //在游戏开始或生成时被调用。
  virtual void BeginPlay() override;

  void PreWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin);
  void PostWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin);

  void OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld);
  void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

public:

  void RegisterInitialObjects();

  void OnActorSpawned(const FCarlaActor& CarlaActor);

  UFUNCTION(Category="Large Map Manager")
  void OnActorDestroyed(AActor* DestroyedActor);

  // 每帧被调用。
  void Tick(float DeltaTime) override;

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void GenerateMap(FString InAssetsPath);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void GenerateLargeMap();

  void GenerateMap(TArray<TPair<FString, FIntVector>> MapPathsIds);

  UFUNCTION(BlueprintCallable, CallInEditor, Category = "Large Map Manager")
  void ClearWorldAndTiles();

  UFUNCTION(BlueprintCallable, CallInEditor, Category = "Large Map Manager")
  void GenerateMap_Editor()
  {
    if (!LargeMapTilePath.IsEmpty()) GenerateMap(LargeMapTilePath);
  }

  void AddActorToUnloadedList(const FCarlaActor& CarlaActor, const FTransform& Transform);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  FIntVector GetNumTilesInXY() const;

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  int GetNumTiles() const
  {
    return MapTiles.Num();
  }

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  bool IsLevelOfTileLoaded(FIntVector InTileID) const;

  bool IsTileLoaded(TileID TileId) const
  {
    return CurrentTilesLoaded.Contains(TileId);
  }

  bool IsTileLoaded(FVector Location) const
  {
    return IsTileLoaded(GetTileID(Location));
  }

  bool IsTileLoaded(FDVector Location) const
  {
    return IsTileLoaded(GetTileID(Location));
  }

  FTransform GlobalToLocalTransform(const FTransform& InTransform) const;
  FVector GlobalToLocalLocation(const FVector& InLocation) const;

  FTransform LocalToGlobalTransform(const FTransform& InTransform) const;
  FVector LocalToGlobalLocation(const FVector& InLocation) const;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Large Map Manager")
  FString LargeMapTilePath = "/Game/Carla/Maps/testmap";
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Large Map Manager")
  FString LargeMapName = "testmap";

  void SetTile0Offset(const FVector& Offset);

  void SetTileSize(float Size);

  float GetTileSize();

  FVector GetTile0Offset();

  void SetLayerStreamingDistance(float Distance);

  void SetActorStreamingDistance(float Distance);

  float GetLayerStreamingDistance() const;

  float GetActorStreamingDistance() const;

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  FIntVector GetTileVectorID(FVector TileLocation) const;

  FIntVector GetTileVectorID(FDVector TileLocation) const;

  FIntVector GetTileVectorID(TileID TileID) const;

  FVector GetTileLocation(TileID TileID) const;
  
  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  FVector GetTileLocation(FIntVector TileVectorID) const;

  FDVector GetTileLocationD(TileID TileID) const;

  FDVector GetTileLocationD(FIntVector TileVectorID) const;

  /// 从给定的位置检索覆盖该区域的图块ID。
  TileID GetTileID(FVector TileLocation) const;

  TileID GetTileID(FDVector TileLocation) const;

  TileID GetTileID(FIntVector TileVectorID) const;
public:
  FCarlaMapTile* GetCarlaMapTile(FVector Location);

  FCarlaMapTile& GetCarlaMapTile(ULevel* InLevel);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  FCarlaMapTile& GetCarlaMapTile(FIntVector TileVectorID);
  
  FCarlaMapTile* GetCarlaMapTile(TileID TileID);

  FCarlaMapTile& LoadCarlaMapTile(FString TileMapPath, TileID TileId);

  ACarlaWheeledVehicle* GetHeroVehicle();

  // 默认情况下，当周围没有其他自车时，旁观者会被当作自车来对待。
  //但这可以通过Python应用程序编程接口（API）来更改，最终会在这个函数中体现出来。
  bool SpectatorAsEgo = false;
  void ConsiderSpectatorAsEgo(bool _SpectatorAsEgo);

protected:

  void RemoveLandscapeCollisionIfHaveTerraMechanics(ULevel* InLevel);

  void UpdateTilesState();

  void RemovePendingActorsToRemove();

  //检查是否有任何处于活动状态的角色需要转换为休眠状态的角色。
  //因为它超出了范围（角色流送距离）
 // 仅存储所选角色的数组
  void CheckActiveActors();

  // 将超出范围的活动角色转换为休眠角色。
  void ConvertActiveToDormantActors();

  // 检查是否有任何休眠角色需要转换为活动角色。
// 因为它进入了范围（角色流送距离）
// 仅存储所选角色的数组
  void CheckDormantActors();

  // 将超出范围的活动角色转换为休眠角色。
  void ConvertDormantToActiveActors();

  void CheckIfRebaseIsNeeded();

  void GetTilesToConsider(
    const AActor* ActorToConsider,
    TSet<TileID>& OutTilesToConsider);

  void GetTilesThatNeedToChangeState(
    const TSet<TileID>& InTilesToConsider,
    TSet<TileID>& OutTilesToBeVisible,
    TSet<TileID>& OutTilesToHidde);

  void UpdateTileState(
    const TSet<TileID>& InTilesToUpdate,
    bool InShouldBlockOnLoad,
    bool InShouldBeLoaded,
    bool InShouldBeVisible);

  void UpdateCurrentTilesLoaded(
    const TSet<TileID>& InTilesToBeVisible,
    const TSet<TileID>& InTilesToHidde);

  UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  TMap<uint64, FCarlaMapTile> MapTiles;

  // 所有要考虑用于瓦片加载的角色（所有主车）
// 数组中的第一个角色是被选中用于重新定基准的那个角色
// 待办事项：支持对多个主车进行重新定基准操作
  UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  TArray<AActor*> ActorsToConsider;

  UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  AActor* Spectator = nullptr;
  //UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  TArray<FCarlaActor::IdType> ActiveActors;
  TArray<FCarlaActor::IdType> DormantActors;

  //临时集合用于移除角色。这样做只是为了避免在更新循环中移除它们。
  TSet<AActor*> ActorsToRemove;
  TSet<FCarlaActor::IdType> ActivesToRemove;
  TSet<FCarlaActor::IdType> DormantsToRemove;

  // 用于将角色从一个数组移动到另一个数组的辅助工具。.
  TSet<FCarlaActor::IdType> ActiveToDormantActors;
  TSet<FCarlaActor::IdType> DormantToActiveActors;

  UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  TSet<uint64> CurrentTilesLoaded;

  // 重新定基准后的当前原点。
  UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  FIntVector CurrentOriginInt{ 0 };

  FDVector CurrentOriginD;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Large Map Manager")
  FVector Tile0Offset = FVector(0,0,0);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Large Map Manager")
  float TickInterval = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Large Map Manager")
  float LayerStreamingDistance = 3.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Large Map Manager")
  float ActorStreamingDistance = 2.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Large Map Manager")
  float RebaseOriginDistance = 2.0f * 1000.0f * 100.0f;

  float LayerStreamingDistanceSquared = LayerStreamingDistance * LayerStreamingDistance;
  float ActorStreamingDistanceSquared = ActorStreamingDistance * ActorStreamingDistance;
  float RebaseOriginDistanceSquared = RebaseOriginDistance * RebaseOriginDistance;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Large Map Manager")
  float TileSide = 2.0f * 1000.0f * 100.0f; // 2km

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Large Map Manager")
  FVector LocalTileOffset = FVector(0,0,0);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Large Map Manager")
  bool ShouldTilesBlockOnLoad = false;


  void RegisterTilesInWorldComposition();

  FString GenerateTileName(TileID TileID);

  FString TileIDToString(TileID TileID);

  void DumpTilesTable() const;

  void PrintMapInfo();

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Large Map Manager")
  FString AssetsPath = "";

  FString BaseTileMapPath = "/Game/Carla/Maps/LargeMap/EmptyTileBase";

  FColor PositonMsgColor = FColor::Purple;

  const int32 TilesDistMsgIndex = 100;
  const int32 MaxTilesDistMsgIndex = TilesDistMsgIndex + 10;

  const int32 ClientLocMsgIndex = 200;
  const int32 MaxClientLocMsgIndex = ClientLocMsgIndex + 10;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float MsgTime = 1.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool bPrintMapInfo = true;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool bPrintErrors = false;

  UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  bool bHasTerramechanics = false;
};
