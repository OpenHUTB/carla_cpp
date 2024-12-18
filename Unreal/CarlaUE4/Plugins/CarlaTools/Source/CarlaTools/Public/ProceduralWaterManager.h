// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"// 引入核心的Unreal引擎头文件

#include "Components/SplineComponent.h"// 引入Spline组件头文件
#include "EditorUtilityWidget.h"// 引入编辑器工具小部件头文件
#include "Math/Vector.h"// 引入向量数学头文件

#include "ProceduralWaterManager.generated.h"// 生成所需的头文件

UENUM(BlueprintType)// 定义一个可在蓝图中使用的枚举类型
enum EWaterGenerationType
{
  RIVERS = 0,
  LAKE = 1
};

USTRUCT(BlueprintType)// 定义一个可在蓝图中使用的结构体
struct CARLATOOLS_API FProceduralRiversMetaInfo
{
  GENERATED_USTRUCT_BODY();

  UPROPERTY(BlueprintReadWrite)
  TEnumAsByte<EWaterGenerationType> WaterGenerationType;

  UPROPERTY(BlueprintReadWrite)
  FString WaterInfoPath;

  UPROPERTY(BlueprintReadWrite)
  int CustomSampling;

  UPROPERTY(BlueprintReadWrite)
  float CustomScaleFactor;

  UPROPERTY(BlueprintReadWrite)
  float CustomRiverWidth;

  UPROPERTY(BlueprintReadWrite)
  FVector CustomLocationOffset;

  UPROPERTY(BlueprintReadWrite)
  float CustomHeight;

  UPROPERTY(BlueprintReadWrite)
  int SizeOfLandscape;

  UPROPERTY(BlueprintReadWrite)
  int SizeOfTexture;
};

UCLASS(BlueprintType)
class CARLATOOLS_API UProceduralWaterManager : public UEditorUtilityWidget
{
  GENERATED_BODY()
  
public:  
  //构造函数，设置默认属性
  UProceduralWaterManager();

public:  
   /// 河流蓝图类，通过小部件接口由用户设置
  /// @a MetaInfo 是此过程的输入数据
  UPROPERTY(BlueprintReadWrite)
  TSubclassOf<class AActor> RiverBlueprintClass;

  /// 湖泊蓝图类，通过小部件接口由用户设置
  /// @a MetaInfo 是此过程的输入数据
  UPROPERTY(BlueprintReadWrite)
  TSubclassOf<class AActor> LakeBlueprintClass;

   /// 从小部件调用的主函数，用于启动所有生成过程
  /// @a MetaInfo 是此过程的输入数据
  UFUNCTION(BlueprintCallable)
  FString StartWaterGeneration(const FProceduralRiversMetaInfo MetaInfo);

   /// 向Spline组件添加河流的控制点 @a riverActor 和 @a splinePoint  
  /// 这些控制点将在后续处理过程中添加到Spline组件中
  /// 该函数在蓝图中实现
  UFUNCTION(BlueprintImplementableEvent)
  void AddRiverPointFromCode(AActor* RiverActor, FSplinePoint SplinePoint);

 /// 根据河流的起点和终点的高度，检查并决定河流的流向
  /// @a riverActor 是河流的Actor实例
  UFUNCTION(BlueprintImplementableEvent)
  void CheckAndReverseWaterFlow(AActor* RiverActor);

private:
  
  ///  负责河流的生成，解析文件，实例化Actor并设置Spline控制点
  UFUNCTION()
  FString RiverGeneration(const FProceduralRiversMetaInfo MetaInfo);

  /// 负责湖泊的生成，解析文件，实例化Actor并设置属性
  UFUNCTION()
  FString LakeGeneration(const FProceduralRiversMetaInfo MetaInfo);

  /// 实例化一个新的河流蓝图Actor
  /// 返回创建的Actor
  AActor* SpawnRiverBlueprintActor();

  /// 实例化一个新的湖泊蓝图Actor
  /// 返回创建的Actor
  UFUNCTION()
  AActor* SpawnLakeBlueprintActor();

  /// 计算特定二维坐标（@a x, @a y）上的地形高度
  /// 通过发射射线并检测碰撞点来实现。@a bDrawDebugLines 参数用于可视化调试射线，
  /// 仅供调试用途。
  /// 返回 Z 坐标值
  UFUNCTION()
  float GetLandscapeSurfaceHeight(float x, float y, bool bDrawDebugLines = false);


  /************ 河流预设生成器 ************/
public:
  UFUNCTION(BlueprintCallable)// 声明该函数可以从蓝图调用
  bool CreateRiverPresetFiles(TSubclassOf<AActor> RiverParentClass); // 创建河流预设文件
};
