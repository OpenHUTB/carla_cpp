// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这是一个头文件保护指令，确保该头文件内容在一个编译单元中只被包含一次
#pragma once

// 引入用于准备烹饪资产的命令行相关头文件
#include "PrepareAssetsForCookingCommandlet.h"
// 引入道路绘制器包装相关的头文件
#include "Util/RoadPainterWrapper.h"
// 引入用于操作引擎中对象库的相关类的头文件
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
// 引入Carla中OpenDrive相关的头文件，可能用于处理地图等相关数据结构
#include "Carla/OpenDrive/OpenDrive.h"
// 引入OpenDrive解析器相关头文件，用于解析OpenDrive格式的数据
#include <carla/opendrive/OpenDriveParser.h>

// 以下三个头文件的引入和禁用/启用UE4相关宏有关，具体看相关头文件内的实现细节
#include <compiler/disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/geom/Math.h>
#include <carla/rpc/String.h>
#include <compiler/enable-ue4-macros.h>

// 引入用于生成代码相关的头文件（通常和UE4的反射等机制有关）
#include "LoadAssetMaterialsCommandlet.generated.h"

// 定义结构体FLargeMapTileData，使用USTRUCT宏标记，使其能被UE4的反射系统识别
USTRUCT()
struct CARLA_API FLargeMapTileData
{
    // 用于UE4反射系统生成必要的代码体，不需要手动修改
    GENERATED_USTRUCT_BODY()

    // 定义一个属性FirstTileCenterX，用于存储地图瓦片中心的X坐标，可在UE4编辑器中进行设置等操作（通过UPROPERTY宏标记）
    UPROPERTY()
    float FirstTileCenterX;

    // 定义一个属性FirstTileCenterY，用于存储地图瓦片中心的Y坐标，可在UE4编辑器中进行设置等操作（通过UPROPERTY宏标记）
    UPROPERTY()
    float FirstTileCenterY;

    // 定义一个属性Size，用于存储地图瓦片的尺寸大小，可在UE4编辑器中进行设置等操作（通过UPROPERTY宏标记）
    UPROPERTY()
    float Size;
};

// 前置声明结构体FDecalsProperties，具体定义可能在别处（通常用于解决循环包含等问题）
struct FDecalsProperties;

// 定义类ULoadAssetMaterialsCommandlet，继承自UCommandlet，是UE4中命令行工具类的派生类，用于执行特定的命令操作
UCLASS()
class CARLA_API ULoadAssetMaterialsCommandlet : public UCommandlet
{
    // 用于UE4反射系统生成必要的代码体，不需要手动修改
    GENERATED_BODY()

public:
    // 默认构造函数，用于创建该类的对象实例
    ULoadAssetMaterialsCommandlet();

#if WITH_EDITORONLY_DATA
    // 获取指定包名 @a PackageName 中所有要烹饪的资产的路径，返回一个FAssetsPaths类型（具体类型应该在别处定义）
    FAssetsPaths GetAssetsPathFromPackage(const FString &PackageName) const;

    // 解析通过 @a InParams 传入的命令行参数，返回一个FPackageParams类型（具体类型应该在别处定义）
    FPackageParams ParseParams(const FString& InParams) const;

    // 根据包名 @a PackageName 和地图路径数组 @a MapsPaths 加载资产的材料
    void LoadAssetsMaterials(const FString &PackageName, const TArray<FMapData> &MapsPaths);

    // 应用道路绘制器相关材料，根据已加载的地图名 @a LoadedMapName、包名 @a PackageName，可选是否基于瓦片（默认是false）
    void ApplyRoadPainterMaterials(const FString &LoadedMapName, const FString &PackageName, bool IsInTiles = false);

    // 读取贴花配置文件（可能用于处理道路等表面贴花相关的配置），根据包名 @a PackageName 返回FDecalsProperties类型的配置信息
    FDecalsProperties ReadDecalsConfigurationFile(const FString &PackageName);

    // 命令行工具类的主入口方法，重写了父类UCommandlet的Main方法，接受参数 @a Params，执行具体的命令逻辑并返回执行结果（返回值类型为int32）
    virtual int32 Main(const FString &Params) override;
#endif // WITH_EDITORONLY_DATA

private:
    // 存储从任何对象库中加载的资产数据，是一个FAssetData类型的数组，可在UE4编辑器中显示等（通过UPROPERTY宏标记）
    UPROPERTY()
    TArray<FAssetData> AssetDatas;

    // 指向UE4世界对象的指针，可能用于获取或操作世界相关的资源等
    UPROPERTY()
    UWorld *World;

    // 指向要加载的新UE4世界对象的指针，可能用于特定的加载场景等情况
    UPROPERTY()
    UWorld *NewWorldToLoad;

    // 用于在对象库中加载地图的对象库指针，加载的数据会存储在AssetDatas中
    UPROPERTY()
    UObjectLibrary *MapObjectLibrary;

    // 用于获取道路绘制器蓝图的子类类型，指定了ARoadPainterWrapper类的子类（具体子类可能根据实际需求确定）
    UPROPERTY()
    TSubclassOf<ARoadPainterWrapper> RoadPainterSubclass;

    // 一个字典（TMap类型），用于将JSON文件中 "decal_names" 数组里的名称转换为已经加载的材质实例，这些材质实例会被应用到道路上
    UPROPERTY()
    TMap<FString, FString> DecalNamesMap;

    // 以下这行被注释掉了，原本可能是用于存储用于贴花的路点集合（TSet类型，存储FVector类型的路点）
    //TSet<FVector> WaypointsUsedForDecals;

    // 一个可选的carla::road::Map类型，可能用于存储OpenDrive格式解析后的地图数据（具体看相关库的定义）
    boost::optional<carla::road::Map> XODRMap;

    // 获取在虚幻内容目录下，给定包名 @a PackageName 时找到的第一个.Package.json文件的路径，返回FString类型的路径字符串
    FString GetFirstPackagePath(const FString &PackageName) const;

    // 存储地图瓦片数据的结构体实例，用于处理地图瓦片相关的尺寸、坐标等信息
    FLargeMapTileData TileData;

    // 存储贴花属性的结构体实例，应该包含了和贴花相关的各种配置属性等信息
    FDecalsProperties DecalsProperties;

    // 标记是否已填充数据，用于指示相关数据是否已经被正确加载或初始化等情况
    bool FilledData;

    // 标记是否已读取配置文件，用于判断配置文件读取操作是否已经执行过
    bool ReadConfigFile;
};
