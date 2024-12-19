// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//版权声明部分，清晰表明这段代码的版权归属于巴塞罗那自治大学的计算机视觉中心（CVC），明确了代码的版权归属主体。
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 说明该作品依据MIT许可证的条款进行授权，同时给出了获取许可证副本的网址，方便使用者查看具体的许可要求，
// 意味着使用这段代码需要遵循MIT许可证所规定的相关规则，比如保留版权声明等内容。
#pragma once
// 这是一个预处理器指令，其作用是确保该头文件在整个编译过程中只会被编译一次，避免因多次被包含而出现重复定义等编译问题，
// 以此保证编译流程的顺利进行以及代码逻辑的正确性。
#include "CoreMinimal.h"// 引入UE4的核心基础头文件，这个头文件包含了大量UE4开发中常用的基础定义，例如基本的数据类型（像FString、FVector等）、
// 常用的宏以及一些底层的、通用的功能函数等，是构建UE4项目里其他各类模块和类的重要基础支撑，很多功能实现都会依赖于此。
#include "EditorUtilityWidget.h"// 引入编辑器实用工具Widget相关的头文件，表明这个类将会继承或使用到该头文件所定义的编辑器Widget相关的功能、特性等，
// 比如可能涉及到在编辑器环境下特定的界面展示、交互功能的实现等，会基于此类基础来构建自身相关的功能逻辑。
#include "Carla/MapGen/LargeMapManager.h"// 引入与Carla项目中地图生成相关的LargeMapManager类的头文件，推测后续类中的函数可能会与LargeMapManager类进行交互，
// 利用其提供的功能来处理大地图相关的操作，比如对大地图资源的管理、操作等方面的功能调用。
#include "Components/HierarchicalInstancedStaticMeshComponent.h"// 引入层级实例化静态网格组件相关的头文件，虽然在当前展示的类中暂时未明显体现其使用情况，但有可能在相关函数的实现中，
// 会涉及到对这种组件的操作，例如在地图场景中利用该组件来高效地渲染大量相同的静态网格物体等情况。
#include "Engine/LevelStreaming.h"// 引入关卡流相关的头文件，关卡流技术常用于在UE4中实现动态加载和卸载关卡等功能，
// 可以推测类中的部分函数可能会借助该头文件所定义的功能来实现对关卡的加载、卸载以及移动相关操作等。

#include "HoudiniImporterWidget.generated.h"// 这是UE4代码生成相关的头文件，配合UE4的反射机制等，用于生成一些必要的代码来支持类的序列化、蓝图相关操作等功能，
// 使得定义的类可以在UE4的蓝图环境下方便地进行属性编辑、函数调用以及与其他蓝图或C++代码进行交互等操作。

UCLASS(BlueprintType)
class CARLATOOLS_API UHoudiniImporterWidget : public UEditorUtilityWidget
{
  GENERATED_BODY()// 这是UE4中用于标记类的可生成代码部分的宏，借助UE4的代码生成机制，通过这个宏实现类的反射等相关特性，
// 进而让类能够在蓝图环境下展现出可编辑属性、可调用函数等功能，方便开发人员进行可视化编程和功能拓展。
public:// 以下是类的公有成员函数，公有成员函数可以被外部的其他类或者蓝图进行访问，用于向外提供类的功能接口。
  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  static void CreateSubLevels(ALargeMapManager* LargeMapManager);
// 声明一个静态函数（通过static修饰），可以在蓝图中调用（通过BlueprintCallable修饰），属于“HoudiniImporterWidget”分类。
    // 函数接收一个ALargeMapManager类型的指针参数LargeMapManager，推测其功能是根据传入的大地图管理器对象来创建子关卡，
    // 可能涉及到关卡创建的相关配置、资源分配等操作，具体实现会在对应的.cpp文件中完成，外部蓝图或其他代码可以调用该函数来创建特定的子关卡。
  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  static void MoveActorsToSubLevelWithLargeMap(TArray<AActor*> Actors, ALargeMapManager* LargeMapManager);
// 同样是一个可在蓝图中调用（BlueprintCallable修饰）的静态函数，属于“HoudiniImporterWidget”分类。
    // 接收两个参数，一个是AActor类型的数组Actors，表示要移动的一系列演员（在UE4中，Actor可以指代场景中的各种对象），
    // 另一个是ALargeMapManager类型的指针LargeMapManager，推测其功能是将给定的这些演员移动到与大地图管理器相关联的子关卡中，
    // 具体移动过程可能涉及到对演员位置、所属关卡等属性的调整，其具体实现逻辑在对应的.cpp文件中定义，外部可通过该函数来进行相关的演员移动操作。
  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  static void ForceStreamingLevelsToUnload( ALargeMapManager* LargeMapManager );
// 可在蓝图中调用（BlueprintCallable修饰）的静态函数，分类为“HoudiniImporterWidget”。
    // 接收一个ALargeMapManager类型的指针参数LargeMapManager，从函数名推测其功能是强制卸载与大地图管理器相关联的正在流加载的关卡，
    // 可能会调用关卡流相关的功能来实现关卡的卸载操作，具体的卸载流程和相关资源释放等操作在对应的.cpp文件中实现，
  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  static void MoveActorsToSubLevel(TArray<AActor*> Actors, ULevelStreaming* Level);
// 这是一个能在蓝图中调用（BlueprintCallable修饰）的静态函数，归类在“HoudiniImporterWidget”分类下。
    // 函数接收两个参数，一个是AActor类型的数组Actors（代表要移动的演员对象集合），另一个是ULevelStreaming类型的指针Level，
    // 推测其功能是将给定的演员移动到由ULevelStreaming对象所关联的子关卡中，可能涉及到关卡相关属性设置以及演员的位置更新等操作，
    // 具体实现会在对应的.cpp文件中呈现，外部可利用该函数实现演员在不同关卡间的移动操作。
  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  static void UpdateGenericActorCoordinates(AActor* Actor, FVector TileOrigin);
// 声明一个可在蓝图中调用（BlueprintCallable修饰）的静态函数，属于“HoudiniImporterWidget”分类。
    // 接收两个参数，一个是AActor类型的单个演员对象Actor，另一个是FVector类型的TileOrigin（可能表示瓦片原点相关的位置信息），
    // 推测其功能是根据传入的瓦片原点位置信息来更新给定演员的坐标，具体更新方式以及相关的坐标计算等操作会在对应的.cpp文件中定义，
    // 外部代码（如蓝图脚本）可以调用该函数来调整演员在场景中的位置。
  static void UpdateInstancedMeshCoordinates(
      UHierarchicalInstancedStaticMeshComponent* Component, FVector TileOrigin);

  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")// 这是一个使用 UFUNCTION 宏修饰的函数声明，表示该函数可以在UE4蓝图中被调用（通过 BlueprintCallable 修饰），
// 并且所属的分类为 "HoudiniImporterWidget"，方便在蓝图编辑器中按照分类找到该函数进行使用。
  static void UseCOMasActorLocation(TArray<AActor*> Actors);
// 声明一个静态函数，接收一个 AActor 类型的数组参数 Actors，在UE4中，Actor 可以指代场景中的各种游戏对象（如角色、道具等）。
// 函数名为 UseCOMasActorLocation，从名称推测其功能是将传入的这些演员（Actors）的位置设置为它们的质心（Center of Mass，简称 COM）位置，
// 具体实现可能涉及到计算每个演员的质心坐标，并将其位置属性更新为对应的质心坐标值，详细的计算和更新操作应该在对应的.cpp文件中编写。
  // Gets the total number of cluster from the actor name following the following scheme
  // b{builsing}c{cluster}of{clustersize}// 函数注释说明了该函数的功能，即从演员（Actor）的名称中按照特定的命名规则获取集群（cluster）的总数。
// 具体命名规则是类似 "b{builsing}c{cluster}of{clustersize}" 的格式，意味着演员名称中包含了关于建筑（building）、当前集群编号（cluster）以及集群总大小（clustersize）等相关信息，
// 函数会解析这些信息来确定集群的总数。
  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  static bool GetNumberOfClusters(TArray<AActor*> ActorList, int& OutNumClusters);
// 声明一个静态函数，可在蓝图中调用（BlueprintCallable 修饰），属于 "HoudiniImporterWidget" 分类。
// 该函数接收两个参数，一个是 AActor 类型的数组 ActorList，表示包含多个演员对象的列表；
// 另一个是 int 类型的引用参数 OutNumClusters，用于将获取到的集群总数传出函数外部（通过引用传递，函数内对该参数的修改会反映到函数外部）。
// 函数返回一个 bool 类型的值，用于表示是否成功获取到集群的总数，比如如果演员名称不符合预期的命名规则等情况可能导致获取失败，返回 false，
// 而如果成功解析并获取到集群总数，则返回 true，同时将总数赋值给 OutNumClusters 参数供外部使用。
};
