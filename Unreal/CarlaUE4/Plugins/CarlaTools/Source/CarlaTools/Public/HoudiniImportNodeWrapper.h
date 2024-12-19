// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//// 版权声明，清晰表明这段代码的版权归属于巴塞罗那自治大学的计算机视觉中心（CVC），明确了代码的版权归属主体。
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 说明该作品依据MIT许可证的条款进行授权，并给出了获取许可证副本的网址，方便使用者查看具体许可要求，
// 意味着使用此代码需要遵循MIT许可证所规定的相关规则，例如保留版权声明等内容。
#pragma once
// 这是一个预处理器指令，作用是确保该头文件在整个编译过程中只会被编译一次，避免因重复包含而引发重复定义等编译问题，
// 保证编译流程的顺利进行以及代码结构的合理性。
#include "CoreMinimal.h"// 引入UE4的核心基础头文件，其中涵盖了众多UE4开发常用的基础定义，比如基本的数据类型（像FVector、FString等）、常用的宏以及一些底层通用的功能函数等，
// 是构建UE4项目内其他各类模块和类的重要基础支撑，许多功能的实现都会间接或直接依赖于它里面所定义的内容。
#include "Kismet/BlueprintAsyncActionBase.h"// 引入与蓝图异步操作相关的头文件，表明这个类将会继承或使用到该头文件所定义的蓝图异步操作相关的功能和特性，
// 例如用于在蓝图中方便地执行一些耗时操作（如网络请求、文件加载等）并通过异步方式来避免阻塞主线程，基于其提供的框架来实现自定义的异步功能。
#include "HoudiniImportNodeWrapper.generated.h"
// 这是UE4代码生成相关的头文件，配合UE4的反射机制等，用于生成必要的代码来支持类的序列化、蓝图相关操作等功能，
// 使得定义的类可以在UE4的蓝图环境下进行诸如属性编辑、函数调用以及与其他蓝图或C++代码交互等操作。
// Delegate type for output pins on the node.// 声明一个动态多播委托类型（使用DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams宏），用于表示节点上输出引脚的相关情况。
// 这个委托可以绑定多个函数，当满足特定条件时会触发所有绑定的函数执行。
// 它有两个参数，分别为const bool类型的bCookSuccess（表示烹饪操作是否成功）和const bool类型的bBakeSuccess（表示烘焙操作是否成功）。
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProcessImporterOutputPinDelegate, const bool, bCookSuccess, const bool, bBakeSuccess);

UCLASS()
class CARLATOOLS_API UHoudiniImportNodeWrapper : public UBlueprintAsyncActionBase
{
  GENERATED_BODY()
// 这是UE4中用于标记类的可生成代码部分的宏，借助UE4的代码生成系统，通过这个宏实现类的反射等相关特性，
  // 进而使类在蓝图环境下具备可编辑属性、可调用函数等功能，方便在UE4的可视化编程环境（蓝图）中进行功能拓展和使用。
public:
// 以下是类的公有成员部分，公有成员可以被外部的其他类或者蓝图访问，用于向外提供类的功能接口。
  UHoudiniImportNodeWrapper(const FObjectInitializer& ObjectInitializer);
// 构造函数声明，接收一个FObjectInitializer类型的参数，用于初始化对象相关的一些属性等，
    // 具体的初始化逻辑可能涉及到调用父类构造函数以及根据传入的初始化器来设置类自身特定的初始状态，
    // 其具体实现应该在对应的.cpp文件中完成。
  UFUNCTION(BlueprintCallable, meta=(AdvancedDisplay=5,AutoCreateRefTerm="InInstantiateAt",BlueprintInternalUseOnly="true", WorldContext="WorldContextObject"), Category="Houdini Importer")
  static UHoudiniImportNodeWrapper* ImportBuildings(
      UObject* InHoudiniAsset,
      const FTransform& InInstantiateAt,
      UObject* InWorldContextObject,
      const FString& MapName, const FString& OSMFilePath,
      float Latitude, float Longitude,
      int ClusterSize, int CurrentCluster,
      bool bUseCOM);
// 声明一个静态函数（通过static修饰），可在蓝图中调用（BlueprintCallable修饰），用于导入建筑物相关操作。
    // meta标签内包含了一些额外的元数据信息，例如AdvancedDisplay=5可能与在蓝图编辑器中的显示相关设置有关；
    // AutoCreateRefTerm="InInstantiateAt"表示会自动为参数InInstantiateAt创建引用项；
    // BlueprintInternalUseOnly="true"说明这个函数主要是供蓝图内部使用；
    // WorldContext="WorldContextObject"表明函数执行可能依赖于给定的世界上下文对象。
    // 函数接收多个参数，包括Houdini资产对象（InHoudiniAsset）、实例化位置变换信息（InInstantiateAt）、世界上下文对象（InWorldContextObject）、
    // 地图名称（MapName）、OSM文件路径（OSMFilePath）、纬度（Latitude）、经度（Longitude）、集群大小（ClusterSize）、当前集群（CurrentCluster）以及是否使用质心（bUseCOM）等参数，
    // 函数返回一个指向UHoudiniImportNodeWrapper对象的指针，推测其功能是根据传入的这些参数进行建筑物导入相关的一系列操作，并返回对应的操作包装对象。
  // Fires on task completed
    // 当任务完成时触发的委托属性，使用UPROPERTY宏并通过BlueprintAssignable修饰，表明这个委托可以在蓝图中被绑定函数，
    // 属于“Houdini|Public API”分类，意味着它是对外公开的接口部分，供外部蓝图或其他代码订阅任务完成的相关事件，以便执行后续操作。
  UPROPERTY(BlueprintAssignable, Category="Houdini|Public API")
	FOnProcessImporterOutputPinDelegate Completed;
// Fires if the task fails
    // 当任务失败时触发的委托属性，同样使用UPROPERTY宏和BlueprintAssignable修饰，可在蓝图中绑定函数，
    // 也属于“Houdini|Public API”分类，用于外部代码订阅任务失败的相关事件，从而可以针对性地处理任务失败后的情况，比如进行错误提示等操作。
	
	UPROPERTY(BlueprintAssignable, Category="Houdini|Public API")
	FOnProcessImporterOutputPinDelegate Failed;

  virtual void Activate() override;
// 重写父类（UBlueprintAsyncActionBase）的Activate函数，这个函数在异步操作启动时会被调用，
    // 具体到这个类中，应该是在这里面实现启动导入建筑物相关的异步操作流程，例如发起网络请求、开始文件读取等实际操作，
    // 其具体实现逻辑需要在对应的.cpp文件中编写，以完成整个异步任务的启动过程。
protected:
// 以下是类的受保护成员部分，受保护成员通常可以被派生类访问，用于在类的继承体系中实现一些内部的、不对外公开的功能逻辑。
  UFUNCTION()
  void HandleCompleted(
      bool bCookSuccess, 
      bool bBakeSuccess);
// 声明一个无返回值的函数（UFUNCTION修饰），函数参数为表示烹饪和烘焙操作是否成功的两个布尔值，
    // 推测这个函数是用于处理任务完成时的后续操作，比如根据烹饪和烘焙的成功与否来更新相关状态、触发Completed委托等，
    // 具体的处理逻辑会在对应的.cpp文件中实现，且一般是在内部使用，不直接对外暴露。
  UFUNCTION()
  void HandleFailed(
      bool bCookSuccess,
      bool bBakeSuccess);
// 同样是一个无返回值的函数声明，参数也是烹饪和烘焙操作是否成功的布尔值，
    // 用于处理任务失败时的相关操作，可能会根据失败情况进行错误记录、触发Failed委托等处理，
    // 其具体实现也是在对应的.cpp文件中完成，供类内部处理任务失败场景使用，不直接供外部调用。

};
