// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// 确保头文件只被包含一次

#include "CoreMinimal.h"// 包含UE4核心编程环境的基本类型

#include "Components/SplineComponent.h"// 包含SplineComponent组件的定义
#include "EditorUtilityWidget.h"// 包含编辑器实用工具窗口的定义
#include "Math/Vector.h"// 包含数学向量的定义

#include "ProceduralWaterManager.generated.h"// 包含UProceduralWaterManager类的生成代码

// 枚举类型，定义了水体生成的类型
UENUM(BlueprintType)
enum EWaterGenerationType
{
  RIVERS = 0,// 河流
  LAKE = 1// 湖泊
};
// 结构体，存储河流生成的元信息
USTRUCT(BlueprintType)
struct CARLATOOLS_API FProceduralRiversMetaInfo
{
  GENERATED_USTRUCT_BODY();

  UPROPERTY(BlueprintReadWrite)// 可读写属性
  TEnumAsByte<EWaterGenerationType> WaterGenerationType;
// 水体生成类型
  UPROPERTY(BlueprintReadWrite)
  FString WaterInfoPath;// 水体信息路径

  UPROPERTY(BlueprintReadWrite)
  int CustomSampling;// 自定义采样


  UPROPERTY(BlueprintReadWrite)
  float CustomScaleFactor;// 自定义缩放因子

  UPROPERTY(BlueprintReadWrite)
  float CustomRiverWidth;// 自定义河流宽度

  UPROPERTY(BlueprintReadWrite)
  FVector CustomLocationOffset;// 自定义位置偏移

  UPROPERTY(BlueprintReadWrite)
  float CustomHeight;// 自定义高度

  UPROPERTY(BlueprintReadWrite)
  int SizeOfLandscape;// 景观尺寸

  UPROPERTY(BlueprintReadWrite)
  int SizeOfTexture;// 纹理尺寸
};
// 类，负责生成和管理程序化水体（河流和湖泊）
UCLASS(BlueprintType)
class CARLATOOLS_API UProceduralWaterManager : public UEditorUtilityWidget
{
  GENERATED_BODY()
  
public:  
  // 构造函数，设置默认值
  UProceduralWaterManager();

public:  
  /// 河流蓝图类，由用户通过界面设置
  UPROPERTY(BlueprintReadWrite)
  TSubclassOf<class AActor> RiverBlueprintClass;

  /// 湖泊蓝图类，由用户通过界面设置
  UPROPERTY(BlueprintReadWrite)
  TSubclassOf<class AActor> LakeBlueprintClass;

  /// 主函数，从界面调用以启动生成过程
  UFUNCTION(BlueprintCallable)
  FString StartWaterGeneration(const FProceduralRiversMetaInfo MetaInfo);

  /// 从代码中添加河流点到SplinePoint集合
  UFUNCTION(BlueprintImplementableEvent)
  void AddRiverPointFromCode(AActor* RiverActor, FSplinePoint SplinePoint);

  /// // 检查并反转水流方向
  UFUNCTION(BlueprintImplementableEvent)
  void CheckAndReverseWaterFlow(AActor* RiverActor);

private:
  
  /// // 负责河流生成，解析文件，实例化演员和设置其Spline点
  UFUNCTION()
  FString RiverGeneration(const FProceduralRiversMetaInfo MetaInfo);

  /// 负责湖泊生成，解析文件，实例化演员和设置其属性
  UFUNCTION()
  FString LakeGeneration(const FProceduralRiversMetaInfo MetaInfo);

  ///  实例化一个新的河流蓝图演员
  UFUNCTION()
  AActor* SpawnRiverBlueprintActor();

  // 实例化一个新的湖泊蓝图演员
  UFUNCTION()
  AActor* SpawnLakeBlueprintActor();

  // 计算特定2D坐标点的地形高度
  UFUNCTION()
  float GetLandscapeSurfaceHeight(float x, float y, bool bDrawDebugLines = false);


  /************ RIVER PRESETS GENERATOR ************/
public:// 创建河流预设文件
  UFUNCTION(BlueprintCallable)
  bool CreateRiverPresetFiles(TSubclassOf<AActor> RiverParentClass);
};
