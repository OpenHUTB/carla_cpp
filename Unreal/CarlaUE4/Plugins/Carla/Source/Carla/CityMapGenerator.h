// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "MapGen/CityMapMeshHolder.h"

#include "MapGen/DoublyConnectedEdgeList.h"
#include "MapGen/GraphParser.h"

#include "CityMapGenerator.generated.h"

class URoadMap;

UCLASS(HideCategories=(Input,Rendering,Actor))
class CARLA_API ACityMapGenerator : public ACityMapMeshHolder
{
    GENERATED_BODY()

    // ===========================================================================
    /// @name 构造函数和析构函数
    // ===========================================================================
    /// @{
public:
    // ACityMapGenerator类的构造函数，接收一个FObjectInitializer类型的参数，用于初始化对象相关的属性、子对象等内容
    ACityMapGenerator(const FObjectInitializer& ObjectInitializer);

    // ACityMapGenerator类的析构函数，用于释放类所占用的资源等清理操作，目前为空实现，后续若有资源管理需求可在此添加相应代码
    ~ACityMapGenerator();

    /// @}
    // ===========================================================================
    /// @name 重写自UObject的方法
    // ===========================================================================
    /// @{
public:
    // 重写自UObject类的PreSave函数，在对象保存前被调用，用于执行一些保存前的特定操作，比如根据一定条件生成道路地图等相关逻辑
    virtual void PreSave(const ITargetPlatform *TargetPlatform) override;

    /// @}
    // ===========================================================================
    /// @name 重写自ACityMapMeshHolder的方法
    // ===========================================================================
    /// @{
private:
    // 重写自ACityMapMeshHolder类的UpdateMap函数，用于更新地图相关状态，比如更新种子、重新生成图、添加道路网格等操作，根据不同条件执行不同的地图更新步骤
    virtual void UpdateMap() override;

    /// @}
    // ===========================================================================
    /// @name 道路地图相关方法
    // ===========================================================================
    /// @{
public:
    // 一个可以在蓝图中被调用的函数（UFUNCTION宏标记），用于获取当前对象关联的URoadMap对象指针，方便外部获取道路地图对象进行后续操作
    UFUNCTION(BlueprintCallable)
    URoadMap *GetRoadMap()
    {
        return RoadMap;
    }

    /// @}
    // ===========================================================================
    /// @name 地图构建和更新相关的方法
    // ===========================================================================
    /// @{
private:
    // 更新随机种子的函数，如果没有使用固定种子（bUseFixedSeed为假），则生成一个随机种子，用于地图生成中需要随机化的部分，比如地图布局等
    void UpdateSeeds();

    // 重新生成DCEL（Doubly Connected Edge List，双向连通边列表）的函数，用于构建或更新表示地图结构的图数据结构，是地图构建过程中的关键步骤
    void GenerateGraph();

    // 根据当前的DCEL结构，将道路网格添加到场景中的函数，基于地图的图结构信息在场景中创建和放置实际的道路相关的网格模型
    void GenerateRoads();

    // 生成道路地图图像的函数，如果有需求还会将其保存到磁盘，用于生成用于诸如越野和对向车道入侵检测等功能所需要的道路地图图像表示
    void GenerateRoadMap();

    /// @}
    // ===========================================================================
    /// @name 地图生成属性
    // ===========================================================================
    /// @{
private:
    /**
     * 地图在地图单位下的X尺寸。地图单位是基于道路的瓦片网格来计算的（详见地图比例相关说明）。
     */
    UPROPERTY(Category = "Map Generation", EditAnywhere, meta = (ClampMin = "10", ClampMax = "200"))
    uint32 MapSizeX = 20u;

    /**
     * 地图在地图单位下的Y尺寸。地图单位是基于道路的瓦片网格来计算的（详见地图比例相关说明）。
     */
    UPROPERTY(Category = "Map Generation", EditAnywhere, meta = (ClampMin = "10", ClampMax = "200"))
    uint32 MapSizeY = 20u;

    /**
     * 如果为假，则不添加网格，仅生成道路的内部表示形式。
     */
    UPROPERTY(Category = "Map Generation", EditAnywhere)
    bool bGenerateRoads = true;

    /**
     * 如果为假，则每次都会生成一个随机种子。
     */
    UPROPERTY(Category = "Map Generation", EditAnywhere)
    bool bUseFixedSeed = true;

    /**
     * 生成的随机地图的种子。
     */
    UPROPERTY(Category = "Map Generation", EditAnywhere, meta = (EditCondition = bUseFixedSeed))
    int32 Seed = 123456789;

    /// @}
    // ===========================================================================
    /// @name 道路地图相关属性
    // ===========================================================================
    /// @{
private:
    /**
     * 触发生成当前布局的道路地图图像（用于越野和对向车道入侵检测）。
     */
    UPROPERTY(Category = "Road Map", EditAnywhere)
    bool bTriggerRoadMapGeneration = false;

    /**
     * 道路地图中每地图单位对应的像素分辨率。
     */
    UPROPERTY(Category = "Road Map", EditAnywhere, meta = (ClampMin = "1", ClampMax = "500"))
    uint32 PixelsPerMapUnit = 50u;

    /**
     * 道路地图是否应基于靠左行驶交通规则来生成。
     */
    UPROPERTY(Category = "Road Map", EditAnywhere)
    bool bLeftHandTraffic = false;

    /**
     * 如果为真，将编码为图像的道路地图保存到磁盘。图像会保存到项目的“Saved”文件夹中。
     */
    UPROPERTY(Category = "Road Map", EditAnywhere)
    bool bSaveRoadMapToDisk = true;

    /**
     * 如果为真，会在关卡中为道路地图的每个像素绘制一个调试点。
     */
    UPROPERTY(Category = "Road Map", EditAnywhere)
    bool bDrawDebugPixelsToLevel = false;

    /**
     * 道路地图在保存时会重新计算，所以我们总是存储最新版本。仅出于测试目的才取消勾选此选项，因为道路地图可能会与当前道路布局不同步。
     */
    UPROPERTY(Category = "Road Map", EditAnywhere, AdvancedDisplay)
    bool bGenerateRoadMapOnSave = true;

    /**
     * 如果为真，激活关卡中每个被标记的演员的自定义深度通道。此通道对于渲染语义分割是必要的。然而，由于似乎不会对具有此值激活的对象应用遮挡，所以可能会增加性能开销。
     */
    UPROPERTY(Category = "Road Map", EditAnywhere, AdvancedDisplay)
    bool bTagForSemanticSegmentation = false;

    UPROPERTY()
    URoadMap *RoadMap;

    /// @}
    // ===========================================================================
    /// @name 其他私有成员
    // ===========================================================================
    /// @{
private:
    // 一个唯一指针（TUniquePtr）指向的MapGen::DoublyConnectedEdgeList类型对象，用于存储表示地图结构的双向连通边列表数据，方便管理内存并确保唯一性
    TUniquePtr<MapGen::DoublyConnectedEdgeList> Dcel;

    // 一个唯一指针（TUniquePtr）指向的MapGen::GraphParser类型对象，用于解析Dcel所表示的图结构，提取相关有用信息，同样便于内存管理和保证唯一性
    TUniquePtr<MapGen::GraphParser> DcelParser;
    /// @}
};
