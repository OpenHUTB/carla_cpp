// 版权所有 (c) 2023 巴塞罗那自治大学 (UAB) 计算机视觉中心 (CVC)。
//
// 本作品根据 MIT 许可证的条款授权。
// 如需副本，请访问 <https://opensource.org/licenses/MIT>。

#pragma once

// 引擎头文件
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MeshDescription.h"
#include "ProceduralMeshComponent.h"
// Carla C++ 头文件

// Carla 插件头文件
#include "Carla/Util/ProceduralCustomMesh.h"

#include "MapGenFunctionLibrary.generated.h"

// 声明日志类别
DECLARE_LOG_CATEGORY_EXTERN(LogCarlaMapGenFunctionLibrary, Log, All);

// 声明 UMapGenFunctionLibrary 类
UCLASS(BlueprintType)
class CARLA_API UMapGenFunctionLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()
public:
  // 创建网格的蓝图可调用函数
  UFUNCTION(BlueprintCallable)
  static UStaticMesh* CreateMesh(
      const FProceduralCustomMesh& Data,
      const TArray<FProcMeshTangent>& ParamTangents,
      UMaterialInstance* MaterialInstance,
      FString MapName,
      FString FolderName,
      FName MeshName);

  // 从数据构建网格描述的静态函数
  static FMeshDescription BuildMeshDescriptionFromData(
      const FProceduralCustomMesh& Data,
      const TArray<FProcMeshTangent>& ParamTangents,
      UMaterialInstance* MaterialInstance );

  // 获取横轴墨卡托投影的蓝图可调用函数
  UFUNCTION(BlueprintCallable)
  static FVector2D GetTransversemercProjection(float lat, float lon, float lat0, float lon0);

  // 使线程休眠的蓝图可调用函数
  UFUNCTION(BlueprintCallable)
  static void SetThreadToSleep(float seconds);

  // 在蓝图中刷新渲染命令的蓝图可调用函数
  UFUNCTION(BlueprintCallable)
  static void FlushRenderingCommandsInBlueprint();

  // 清理GEngine的蓝图可调用函数
  UFUNCTION(BlueprintCallable)
  static void CleanupGEngine();
};