// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.
// 代码采用MIT开源协议授权，详情可参考 https://opensource.org/licenses/MIT
#pragma once
// 引入UE4的核心最小化头文件，包含了UE4引擎基础类型等定义
#include "CoreMinimal.h"
// 引入UE4的用户界面Widget相关头文件，用于创建和操作UI元素
#include "Blueprint/UserWidget.h"
// 引入UE4的过程化网格组件头文件，用于动态生成网格模型相关操作
#include "ProceduralMeshComponent.h"
// 引入UE4的二维向量相关头文件，用于表示二维空间中的向量
#include "Math/Vector2D.h"
// 引入UE4的编辑器工具Actor相关头文件，可能用于在编辑器环境下创建特殊Actor
#include "EditorUtilityActor.h"
// 引入UE4的编辑器工具对象相关头文件，表明此类是一个可在编辑器中使用的工具类对象
#include "EditorUtilityObject.h"
// 引入编译器相关头文件，用于禁用UE4特定宏（可能在特定编译环境下避免宏冲突等情况）
#include <compiler/disable-ue4-macros.h>
// 引入boost库中的可选类型头文件，用于表示可能存在或不存在的值，常用于函数返回值等情况避免空指针等问题
#include <boost/optional.hpp>
// 引入Carla相关的地图头文件，应该是和Carla仿真环境中的地图数据结构相关
#include <carla/road/Map.h>
// 引入编译器相关头文件，用于启用UE4特定宏（与上面的禁用对应，恢复正常的UE4宏定义使用）
#include <compiler/enable-ue4-macros.h>
// 包含生成的头文件（通常是UE4的反射机制相关，用于自动生成类的一些额外代码）
#include "OpenDriveToMap.generated.h"
// 前置声明一些UE4中的类类型，告知编译器这些类在后续会被使用到，但具体定义在别处

class UProceduralMeshComponent;
class UMeshComponent;
class UCustomFileDownloader;
class UMaterialInstance;
// UCLASS宏用于将此类注册到UE4的反射系统中，使其可以在蓝图中使用、进行序列化等操作
// Blueprintable表示此类可以在蓝图中继承扩展，BlueprintType表示此类的实例可以作为蓝图中的变量类型
/**	
 *
	
 */
UCLASS(Blueprintable, BlueprintType)
class CARLATOOLS_API UOpenDriveToMap : public UEditorUtilityObject
{
  GENERATED_BODY()
#if WITH_EDITOR
public:
// 默认构造函数，用于创建UOpenDriveToMap类的实例时进行初始化操作
  UOpenDriveToMap();
// 析构函数，用于在对象销毁时进行资源清理等操作
  ~UOpenDriveToMap();
// UFUNCTION宏将函数标记为可在蓝图中调用的函数
  UFUNCTION()
  void ConvertOSMInOpenDrive();
// UFUNCTION宏将函数标记为可在蓝图中调用的函数，用于创建地图（具体创建的地图类型和方式要看内部实现逻辑）
  UFUNCTION( BlueprintCallable )
  void CreateMap();
 // UFUNCTION宏将函数标记为可在蓝图中调用的函数，用于创建地形
 // 参数MeshGridSize可能表示地形网格的尺寸大小（比如网格的行数或列数等），
 //MeshGridSectionSize可能表示每个网格单元的尺寸大小
  UFUNCTION(BlueprintCallable)
  void CreateTerrain(const int MeshGridSize, const float MeshGridSectionSize);
// 参数MeshIndex可能表示网格的索引，Offset表示偏移量，
 //GridSize表示网格整体大小相关参数，GridSectionSize表示网格单元尺寸相关参数
  UFUNCTION(BlueprintCallable)
  void CreateTerrainMesh(const int MeshIndex, const FVector2D Offset, const int GridSize, const float GridSectionSize);
// UFUNCTION宏将函数标记为可在蓝图中调用的函数，用于获取指定位置的高度值
 // 参数PosX、PosY表示平面位置坐标，bDrivingLane表示是否是行车道（默认值为false），
 //具体获取高度的计算逻辑在函数内部实现
  UFUNCTION(BlueprintCallable)
  float GetHeight(float PosX, float PosY,bool bDrivingLane = false);
  // UFUNCTION宏将函数标记为可在蓝图中调用的函数，用于在检查无碰撞的情况下生成一个Actor实例
    // 参数ActorClassToSpawn表示要生成的Actor类类型，Transform表示生成时的初始变换信息（位置、旋转、缩放等）
  UFUNCTION(BlueprintCallable)
  static AActor* SpawnActorWithCheckNoCollisions(UClass* ActorClassToSpawn, FTransform Transform);
  // UFUNCTION宏将函数标记为可在蓝图中调用的函数，用于获取到行车道边界的距离
    // 参数Location表示空间中的位置，函数内部调用DistanceToLaneBorder函数来计算距离
  UFUNCTION(BlueprintCallable)
  float GetDistanceToDrivingLaneBorder(FVector Location) const{
    return DistanceToLaneBorder(CarlaMap, Location);
  }
// UFUNCTION宏将函数标记为可在蓝图中调用的函数，用于判断指定位置是否在道路范围内
    // 参数Location表示空间中的位置，函数内部调用IsInRoad函数来判断
  UFUNCTION(BlueprintCallable)
  bool GetIsInRoad(FVector Location) const {
    return IsInRoad(CarlaMap, Location);
  }
// 可在蓝图调用，独立生成瓦片
  UFUNCTION(BlueprintCallable)
  void GenerateTileStandalone();
// 可在蓝图调用，生成瓦片
  UFUNCTION(BlueprintCallable)
  void GenerateTile();
// 可在蓝图调用，切换到下一个瓦片
  UFUNCTION(BlueprintCallable)
  bool GoNextTile();
// 可在蓝图调用，返回到主层级
  UFUNCTION(BlueprintCallable)
  void ReturnToMainLevel();
// 可在蓝图调用，校正当前瓦片内Actor位置
  UFUNCTION(BlueprintCallable)
  void CorrectPositionForAllActorsInCurrentTile();
// 可在蓝图调用，获取当前瓦片对应字符串
  UFUNCTION(BlueprintCallable)
  FString GetStringForCurrentTile();
// 可在蓝图调用，在编辑器世界生成Actor实例，参数指定类、位置、旋转
  UFUNCTION(BlueprintCallable)
  AActor* SpawnActorInEditorWorld(UClass* Class, FVector Location, FRotator Rotation);
// 可在编辑器编辑、蓝图读写，存储文件路径字符串
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="File")
  FString FilePath;
// 可在编辑器编辑、蓝图读写，存储地图名称字符串
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="File")
  FString MapName;
// 可在编辑器编辑、蓝图读写，存储URL字符串
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Settings" )
  FString Url;
// 可在编辑器编辑、蓝图读写，存储本地文件路径字符串
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Settings" )
  FString LocalFilePath;
// 可在编辑器编辑、蓝图读写，存储地理坐标原点二维向量
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
  FVector2D OriginGeoCoordinates;
// 可在编辑器编辑、蓝图读写，存储默认道路材质实例指针
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
  UMaterialInstance* DefaultRoadMaterial;
// 可在编辑器编辑、蓝图读写，存储白色车道标线材质实例指针
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
  UMaterialInstance* DefaultLaneMarksWhiteMaterial;
// 可在编辑器编辑、蓝图读写，存储黄色车道标线材质实例指针
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
  UMaterialInstance* DefaultLaneMarksYellowMaterial;
// 可在编辑器编辑、蓝图读写，存储人行道材质实例指针
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
  UMaterialInstance* DefaultSidewalksMaterial;
// 可在编辑器编辑、蓝图读写，存储地形材质实例指针
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
  UMaterialInstance* DefaultLandscapeMaterial;


// 可在编辑器编辑、蓝图读写，树木间距，默认值50.0f
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Settings" )
  float DistanceBetweenTrees = 50.0f;
// 可在编辑器编辑、蓝图读写，距道路边缘距离，默认值3.0f
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Settings" )
  float DistanceFromRoadEdge = 3.0f;
// 可在编辑器编辑、蓝图读写，标记是否开始，默认false
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Stage" )
  bool bHasStarted = false;
// 可在编辑器编辑、蓝图读写，标记道路相关操作是否完成，默认false
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Stage" )
  bool bRoadsFinished = false;
// 可在编辑器编辑、蓝图读写，标记地图是否加载完成，默认false
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Stage" )
  bool bMapLoaded = false;
// 可在编辑器编辑、蓝图读写，瓦片生成最小位置向量
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  FVector MinPosition;
// 可在编辑器编辑、蓝图读写，瓦片生成最大位置向量
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  FVector MaxPosition;
// 可在编辑器编辑、蓝图读写，瓦片尺寸大小
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  float TileSize;
// 可在编辑器编辑、蓝图读写，瓦片0偏移量向量
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  FVector Tile0Offset;
// 可在编辑器编辑、蓝图读写，标记瓦片是否生成完成，默认false
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  bool bTileFinished;
// 可在编辑器编辑、蓝图读写，XY平面瓦片数量
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  FIntVector NumTilesInXY;
// 可在编辑器编辑、蓝图读写，当前瓦片在XY平面的索引
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  FIntVector CurrentTilesInXY;
// 可在编辑器编辑、蓝图读写，基础层级名称
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="TileGeneration" )
  FString BaseLevelName;
// 可在编辑器编辑、蓝图读写，默认高度图纹理
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
  UTexture2D* DefaultHeightmap;
// 可在编辑器编辑、蓝图读写，世界空间结束位置向量
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Heightmap" )
  FVector WorldEndPosition;
// 可在编辑器编辑、蓝图读写，世界空间原点位置向量
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Heightmap" )
  FVector WorldOriginPosition;
// 可在编辑器编辑、蓝图读写，最小高度值
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Heightmap" )
  float MinHeight;
// 可在编辑器编辑、蓝图读写，最大高度值
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Heightmap" )
  float MaxHeight;


protected:
// 可在蓝图调用，生成一些杂项Actor，参数指定偏移量和位置范围
  UFUNCTION(BlueprintCallable)
  TArray<AActor*> GenerateMiscActors(float Offset, FVector MinLocation, FVector MaxLocation );
// 蓝图可实现事件函数，生成完成时触发，参数指定位置范围
  UFUNCTION( BlueprintImplementableEvent )
  void GenerationFinished(FVector MinLocation, FVector MaxLocation);
// 蓝图可实现事件函数，下载完成时触发
  UFUNCTION( BlueprintImplementableEvent )
  void DownloadFinished();

// 蓝图可实现事件函数，执行瓦片相关命令
  UFUNCTION( BlueprintImplementableEvent )
  void ExecuteTileCommandlet();
// 可在蓝图调用，移动Actor到子层级，参数指定要移动的Actor数组
  UFUNCTION( BlueprintCallable )
  void MoveActorsToSubLevels(TArray<AActor*> ActorsToMove);

private:
// 可在蓝图调用，移动Actor到子层级，参数指定要移动的Actor数组
  UFUNCTION()
  void OpenFileDialog();
// 可在蓝图调用，加载地图（具体加载逻辑在函数内实现）
  UFUNCTION(BlueprintCallable)
  void LoadMap();
// 内部函数，生成所有相关内容，参数指定地图和位置范围
  void GenerateAll(const boost::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation);
// 内部函数，生成道路网格，参数指定地图和位置范围
  void GenerateRoadMesh(const boost::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation);
// 内部函数，生成生成点，参数指定地图和位置范围
  void GenerateSpawnPoints(const boost::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation);
// 内部函数，生成树木位置，参数指定地图和位置范围
  void GenerateTreePositions(const boost::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation);
// 内部函数，生成车道标线，参数指定地图和位置范围
  void GenerateLaneMarks(const boost::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation);
// 用于OpenDrive生成的参数结构体
  carla::rpc::OpendriveGenerationParameters opg_parameters;
// 可选的Carla地图对象，可能存储地图相关数据
  boost::optional<carla::road::Map> CarlaMap;
// 获取对齐后的位置变换信息，参数指定原始变换
  FTransform GetSnappedPosition(FTransform Origin);
// 获取地形高度相关值，参数指定原点位置
  float GetHeightForLandscape(FVector Origin);
// 计算到车道边界距离，参数指定地图和位置，可选车道类型
  float DistanceToLaneBorder(const boost::optional<carla::road::Map>& CarlaMap,
      FVector &location,
      int32_t lane_type = static_cast<int32_t>(carla::road::Lane::LaneType::Driving)) const;
// 判断位置是否在道路内，参数指定地图和位置
  bool IsInRoad(const boost::optional<carla::road::Map>& ParamCarlaMap,
        FVector &location) const;
// 初始化纹理数据（具体初始化内容在函数内实现）
  void InitTextureData();
// 导入XODR格式数据（具体导入逻辑在函数内实现）
  void ImportXODR();
// 导入OSM格式数据（具体导入逻辑在函数内实现）
  void ImportOSM();
// 用于文件下载的自定义类指针，可能负责下载地图等资源
  UPROPERTY()
  UCustomFileDownloader* FileDownloader;
// 存储地形相关的Actor数组，可能用于管理地形相关元素
  UPROPERTY()
  TArray<AActor*> Landscapes;
// 高度图纹理指针，用于地形等高度相关可视化
  UPROPERTY()
  UTexture2D* Heightmap;
#endif
};
