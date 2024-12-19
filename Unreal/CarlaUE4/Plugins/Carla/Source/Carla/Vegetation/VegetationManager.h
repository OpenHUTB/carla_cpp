// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入相关头文件，这些头文件包含了游戏框架、组件、材质以及 Carla 相关模块等的定义
#include "GameFramework/Actor.h"
#include "ProceduralFoliageVolume.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "InstancedFoliageActor.h"

#include "Containers/Array.h"
#include "Containers/Map.h"

#include "Carla/MapGen/LargeMapManager.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include <memory>

#include "VegetationManager.generated.h"

// 定义结构体 FTileMeshComponent，用于存储与瓦片相关的实例化静态网格组件信息及使用索引等
USTRUCT()
struct FTileMeshComponent
{
    GENERATED_BODY()
    // 指向实例化静态网格组件的指针，初始化为空
    UInstancedStaticMeshComponent* InstancedStaticMeshComponent {nullptr};
    // 记录正在使用的索引数组
    TArray<int32> IndicesInUse {};
    // 表示该瓦片相关组件是否处于活动（存活）状态
    bool bIsAlive = false;
};

// 定义结构体 FTileData，用于存储与瓦片相关的各种数据，如实例化植被 actor、过程化植被体积等
USTRUCT()
struct FTileData
{
    GENERATED_BODY()
    // 指向实例化植被 actor 的指针
    AInstancedFoliageActor* InstancedFoliageActor {nullptr};
    // 指向过程化植被体积的指针
    AProceduralFoliageVolume* ProceduralFoliageVolume {nullptr};
    // 缓存瓦片的网格组件信息
    TArray<std::shared_ptr<FTileMeshComponent>> TileMeshesCache {};
    // 缓存动态材质实例的数组
    TArray<UMaterialInstanceDynamic*> MaterialInstanceDynamicCache {};

    // 判断是否包含指定的实例化静态网格组件
    bool ContainsMesh(const UInstancedStaticMeshComponent*) const;
    // 更新瓦片的网格组件信息
    void UpdateTileMeshComponent(UInstancedStaticMeshComponent* NewInstancedStaticMeshComponent);
    // 根据给定值和调试材质标志更新材质缓存
    void UpdateMaterialCache(const FLinearColor& Value, bool DebugMaterials);
    // 析构函数
    ~FTileData();
};

// 定义结构体 FFoliageBlueprint，用于处理植被蓝图相关信息，如蓝图类名和生成的 actor 类等
USTRUCT()
struct FFoliageBlueprint
{
    GENERATED_BODY()
    // 完整的蓝图类名（字符串形式）
    FString BPFullClassName {};
    // 实际生成的 actor 类（类型为 TSubclassOf<AActor>）
    TSubclassOf<AActor> SpawnedClass { nullptr };

    // 判断当前蓝图是否有效
    bool IsValid() const;
    // 根据给定路径设置蓝图类名
    bool SetBPClassName(const FString& Path);
    // 设置生成的 actor 类
    bool SetSpawnedClass();
};

// 定义结构体 FPooledActor，用于管理池化的 actor，包含 actor 的使用状态、激活状态、指针等信息
USTRUCT()
struct FPooledActor
{
    GENERATED_BODY()
    // 表示该 actor 是否正在被使用
    bool InUse { false };
    // 表示该 actor 是否处于激活状态
    bool IsActive { false };
    // 指向实际的 actor 实例的指针
    AActor* Actor { nullptr };
    // 该 actor 的全局变换信息
    FTransform GlobalTransform {FTransform()};
    // 索引值
    int32 Index {-1};
    // 指向与之关联的瓦片网格组件的智能指针
    std::shared_ptr<FTileMeshComponent> TileMeshComponent {nullptr};

    // 启用 actor，设置其变换、索引以及关联的瓦片网格组件
    void EnableActor(
        const FTransform& Transform,
        int32 NewIndex,
        std::shared_ptr<FTileMeshComponent>& NewTileMeshComponent);
    // 将 actor 设置为激活状态
    void ActiveActor();
    // 禁用 actor
    void DisableActor();
};

// 定义结构体 FElementsToSpawn，用于存储要生成的元素相关信息，包括瓦片网格组件、植被蓝图以及位置和索引信息
USTRUCT()
struct FElementsToSpawn
{
    GENERATED_BODY()
    // 要生成元素对应的瓦片网格组件
    std::shared_ptr<FTileMeshComponent> TileMeshComponent;
    // 植被蓝图相关信息
    FFoliageBlueprint BP;
    // 包含变换和索引的配对数组，表示生成元素的位置和对应索引
    TArray<TPair<FTransform, int32>> TransformIndex;
};

// 定义 AVegetationManager 类，继承自 AActor，用于管理植被相关的各种操作，如添加车辆、更新资源池位置等
UCLASS()
class CARLA_API AVegetationManager : public AActor
{
    GENERATED_BODY()

public:
    // 向管理器中添加车辆（Carla 轮式车辆类型）
    void AddVehicle(ACarlaWheeledVehicle* Vehicle);
    // 从管理器中移除车辆（Carla 轮式车辆类型）
    void RemoveVehicle(ACarlaWheeledVehicle* Vehicle);

    // 蓝图可调用函数，用于更新资源池的基础位置
    UFUNCTION(BlueprintCallable)
    void UpdatePoolBasePosition();

public:
    // 是否开启调试材质的属性，可在编辑器中编辑默认值
    UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
    bool DebugMaterials {false};

    // 隐藏材质的距离属性，可在编辑器中编辑默认值，控制多远的距离开始隐藏材质相关表现
    UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
    float HideMaterialDistance {500.0f};

    // 激活 actor 的距离属性，可在编辑器中编辑默认值，控制多远的距离激活 actor
    UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
    float ActiveActorDistance {500.0f};

    // 用于调试的过滤器，控制是否生成灌木丛，可在编辑器中编辑默认值
    UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
    bool SpawnBushes {true};

    // 用于调试的过滤器，控制是否生成树木，可在编辑器中编辑默认值
    UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
    bool SpawnTrees {true};

    // 用于调试的过滤器，控制是否生成岩石，可在编辑器中编辑默认值
    UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
    bool SpawnRocks {true};

    // 用于调试的过滤器，控制是否生成植物，可在编辑器中编辑默认值
    UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
    bool SpawnPlants {true};

    // 生成的缩放比例属性，可在编辑器中编辑默认值，影响植被生成的缩放效果
    UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
    float SpawnScale {1.0f};

    // 初始资源池大小属性，可在编辑器中编辑默认值，决定初始创建的资源池中的 actor 数量
    UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
    int32 InitialPoolSize {10};

    /// @}
    // ===========================================================================
    /// @name Overriden from AActor
    // ===========================================================================
    /// @{
protected:
    // 重写 AActor 的 BeginPlay 函数，游戏开始时执行相关初始化等操作
    virtual void BeginPlay() override;
    // 重写 AActor 的 Tick 函数，每帧执行相关更新逻辑
    virtual void Tick(float DeltaTime) override;

private:
    // 根据给定的路径（字符串形式）判断对应的植被类型是否启用
    bool IsFoliageTypeEnabled(const FString& Path) const;
    // 检查是否有新的瓦片，用于相关更新逻辑判断
    bool CheckForNewTiles() const;

    // 获取正在使用的瓦片名称数组
    TArray<FString> GetTilesInUse();

    // 更新给定瓦片的材质信息
    void UpdateMaterials(FTileData* Tile);
    // 获取给定瓦片要生成的元素信息数组
    TArray<FElementsToSpawn> GetElementsToSpawn(FTileData* Tile);
    // 根据要生成的元素信息生成骨骼植被（具体实现可能涉及复杂的植被生成逻辑）
    void SpawnSkeletalFoliages(TArray<FElementsToSpawn>& ElementsToSpawn);
    // 销毁骨骼植被（可能涉及释放相关资源等操作）
    void DestroySkeletalFoliages();
    // 激活资源池中的 actor（根据相关距离等条件判断激活哪些 actor）
    void ActivePooledActors();
    // 从资源池中启用指定的 actor，根据给定的变换、索引、瓦片网格组件以及资源池等信息进行操作
    bool EnableActorFromPool(
        const FTransform& Transform,
        int32 Index,
        std::shared_ptr<FTileMeshComponent>& TileMeshComponent,
        TArray<FPooledActor>& Pool);

    // 创建或更新瓦片缓存（可能涉及加载、更新瓦片相关数据等操作）
    void CreateOrUpdateTileCache(ULevel* InLevel);
    // 更新植被蓝图缓存（可能涉及重新加载、验证蓝图相关信息等操作）
    void UpdateFoliageBlueprintCache(ULevel* InLevel);
    // 设置瓦片数据内部相关信息（具体设置内容可能由函数内部逻辑决定）
    void SetTileDataInternals(FTileData& TileData);
    // 设置实例化静态网格组件缓存（可能涉及初始化、更新组件相关缓存数据等操作）
    void SetInstancedStaticMeshComponentCache(FTileData& TileData);
    // 设置材质缓存（根据具体逻辑更新材质相关的缓存信息）
    void SetMaterialCache(FTileData& TileData);

    // 释放瓦片缓存相关资源（可能涉及删除缓存数据、释放内存等操作）
    void FreeTileCache(ULevel* InLevel);

    // 当关卡添加到世界时调用的函数，执行相关初始化等操作
    void OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld);
    // 当关卡从世界移除时调用的函数，执行相关清理等操作
    void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);
    // 世界原点偏移后调用的函数，进行相关数据更新等操作（例如更新植被位置等）
    void PostWorldOriginOffset(UWorld*, FIntVector, FIntVector InDstOrigin);

    // 为给定的植被蓝图类创建对应的资源池
    void CreatePoolForBPClass(const FFoliageBlueprint& BP);
    // 根据给定的植被蓝图和变换信息创建植被 actor（具体创建逻辑根据植被类型等因素而定）
    AActor* CreateFoliage(const FFoliageBlueprint& BP, const FTransform& Transform) const;

    // 获取骨骼模板（具体获取逻辑和用途可能与植被的骨骼相关特性有关）
    void GetSketalTemplates();

private:
    // 用于资源池平移的定时器，控制资源池位置更新的时间间隔
    float PoolTranslationTimer {30.0f};
    // 资源池非激活状态下的变换信息，初始化为默认值
    FTransform InactivePoolTransform { FQuat(1.0f, 1.0f, 1.0f, 1.0f), FVector(1.0f, 1.0f, 1.0f), FVector(1.0f, 1.0f, 1.0f)};
    // 指向大型地图管理器的指针
    ALargeMapManager* LargeMap {nullptr};
    // 指向主角车辆（Carla 轮式车辆类型）的指针
    ACarlaWheeledVehicle* HeroVehicle {nullptr};
    // 缓存植被蓝图信息的映射表，键为字符串（可能是蓝图相关标识），值为 FFoliageBlueprint 结构体
    TMap<FString, FFoliageBlueprint> FoliageBlueprintCache {};
    // 缓存瓦片数据的映射表，键为字符串（可能是瓦片相关标识），值为 FTileData 结构体
    TMap<FString, FTileData> TileCache {};
    // 缓存 actor 资源池的映射表，键为字符串（可能是与 actor 类型相关标识），值为 FPooledActor 数组
    TMap<FString, TArray<FPooledActor>> ActorPool {};

    // 用于更新资源池非激活变换的定时器句柄
    FTimerHandle UpdatePoolInactiveTransformTimer;
};
