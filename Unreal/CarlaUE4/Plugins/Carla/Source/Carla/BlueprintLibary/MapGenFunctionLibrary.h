// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这是一个头文件保护指令，确保该头文件内容在一个编译单元中只被包含一次
#pragma once  

// Engine headers
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MeshDescription.h"
#include "ProceduralMeshComponent.h"
// Carla C++ headers

// Carla plugin headers
#include "Carla/Util/ProceduralCustomMesh.h"

#include "MapGenFunctionLibrary.generated.h"

// 声明一个名为LogCarlaMapGenFunctionLibrary的日志类别，用于在代码中输出相关日志信息
// Log表示日志级别，All表示涵盖所有类型的日志消息
DECLARE_LOG_CATEGORY_EXTERN(LogCarlaMapGenFunctionLibrary, Log, All);

// 定义UMapGenFunctionLibrary类，它继承自UBlueprintFunctionLibrary，意味着这个类主要用于在蓝图中提供可调用的函数功能
UCLASS(BlueprintType)
class CARLA_API UMapGenFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 一个可在蓝图中调用的函数（UFUNCTION标记且BlueprintCallable属性），用于创建一个UStaticMesh（静态网格体）对象
    // 参数说明：
    // - Data：包含了自定义网格体相关数据的结构体，用于描述网格体的形状、顶点等信息，类型为FProceduralCustomMesh
    // - ParamTangents：包含了网格体切线相关数据的数组，类型为TArray<FProcMeshTangent>，用于处理网格体的表面法线等相关属性
    // - MaterialInstance：材质实例指针，用于指定要应用到创建的网格体上的材质
    // - MapName：字符串类型，可能用于标识地图相关的名称（具体用途需看代码上下文）
    // - FolderName：字符串类型，可能表示存储相关资源的文件夹名称（同样依赖上下文确定准确用途）
    // - MeshName：FName类型，用于指定要创建的网格体的名称
    UFUNCTION(BlueprintCallable)
    static UStaticMesh* CreateMesh(
        const FProceduralCustomMesh& Data,
        const TArray<FProcMeshTangent>& ParamTangents,
        UMaterialInstance* MaterialInstance,
        FString MapName,
        FString FolderName,
        FName MeshName);

    // 静态函数，用于根据给定的自定义网格体数据以及切线数据等构建一个FMeshDescription对象（用于描述网格信息）
    // 参数含义与CreateMesh函数中对应的参数类似，用于提供网格体形状、切线和材质相关信息来构建描述对象
    static FMeshDescription BuildMeshDescriptionFromData(
        const FProceduralCustomMesh& Data,
        const TArray<FProcMeshTangent>& ParamTangents,
        UMaterialInstance* MaterialInstance );

    // 一个可在蓝图中调用的函数，用于进行横向墨卡托投影转换，将经纬度坐标（lat和lon）基于给定的参考经纬度（lat0和lon0）转换为二维平面坐标（返回值FVector2D类型）
    // 参数说明：
    // - lat：表示要转换的纬度坐标，float类型
    // - lon：表示要转换的经度坐标，float类型
    // - lat0：参考纬度坐标，float类型
    // - lon0：参考经度坐标，float类型
    UFUNCTION(BlueprintCallable)
    static FVector2D GetTransversemercProjection(float lat, float lon, float lat0, float lon0);

    // 可在蓝图中调用的函数，用于设置线程休眠的时间（单位：秒），通过传入的float类型参数seconds指定休眠时长
    UFUNCTION(BlueprintCallable)
    static void SetThreadToSleep(float seconds);

    // 可在蓝图中调用的函数，用于在蓝图环境中刷新渲染命令（具体功能可能与游戏引擎的渲染管线相关，需结合具体使用场景深入理解）
    UFUNCTION(BlueprintCallable)
    static void FlushRenderingCommandsInBlueprint();

    // 可在蓝图中调用的函数，用于清理游戏引擎相关资源（具体清理的内容和机制取决于其内部实现，从函数名大致推测是做一些资源释放等清理工作）
    UFUNCTION(BlueprintCallable)
    static void CleanupGEngine();
};
