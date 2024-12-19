// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).// 版权声明，表明这段代码的版权归属于巴塞罗那自治大学的计算机视觉中心（CVC），明确版权归属主体。
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 说明该作品遵循MIT许可证的条款进行授权，并且给出了获取许可证副本的网址，方便查看具体许可内容，
// 意味着使用者需按照MIT许可证要求来使用该代码，比如保留版权声明等相关要求。
#pragma once
// 预处理器指令，作用是确保该头文件在整个编译过程中只会被编译一次，避免因多次包含导致的重复定义等编译错误，
// 保证编译的顺利进行以及代码结构的合理性。
#include "CoreMinimal.h"// 引入UE4的核心基础头文件，其包含了众多UE4开发中常用的基础定义，例如基本的数据类型、常用的宏以及一些底层的功能函数等，
// 是构建UE4项目其他模块和类的重要基础依赖，很多功能的实现都会间接或直接用到这里面的内容。
#include "EditorUtilityWidget.h"// 引入编辑器实用工具Widget相关的头文件，表明这个类可能会继承或使用到该头文件所定义的编辑器Widget相关的功能、特性等，
// 比如可能涉及到在编辑器环境下特定的界面展示、交互功能的实现等，会基于此类基础来构建。
#include "CarlaTools.h"// 引入CarlaTools相关的头文件，虽然不清楚具体里面的内容，但推测可能包含了与CarlaTools这个项目或功能模块相关的类型定义、函数声明等，
// 为当前类提供与之相关的功能支持或者依赖其内部的一些实现来完成自身功能逻辑。

#include "DigitalTwinsBaseWidget.generated.h"
// 这是UE4代码生成相关的头文件，配合UE4的反射机制等，用于生成一些必要的代码来支持类的序列化、蓝图相关操作等功能，
// 是实现UE4类在蓝图环境下可操作、可拓展等特性的重要组成部分。
class UOpenDriveToMap;
// 前向声明UOpenDriveToMap类，在不需要包含该类完整定义的情况下告知编译器后续代码中会用到这个类，
// 解决编译时可能因类定义先后顺序导致的找不到类定义等问题，常用于类之间存在关联但暂不需要完整类细节的场景。
UCLASS(BlueprintType)
class CARLATOOLS_API UDigitalTwinsBaseWidget : public UEditorUtilityWidget
{
  GENERATED_BODY()// 这是UE4中用于标记类的可生成代码部分的宏，借助UE4的代码生成系统，通过这个宏来实现诸如反射等相关特性，
  // 以支持类在蓝图环境下的使用、属性编辑、函数调用等功能。
public:
// 以下是类的公有成员函数，这些函数可以被外部代码（如其他类或者蓝图）访问，用于提供类的对外接口功能。
  UFUNCTION(BlueprintCallable)
  UOpenDriveToMap* InitializeOpenDriveToMap(TSubclassOf<UOpenDriveToMap> BaseClass);
// 声明一个可在蓝图中调用的函数（通过BlueprintCallable修饰），名为InitializeOpenDriveToMap，
  // 它接收一个参数BaseClass，参数类型是TSubclassOf<UOpenDriveToMap>，表示UOpenDriveToMap类的子类类型，
  // 函数的返回值是UOpenDriveToMap* 指针类型，推测其功能是根据传入的子类类型来初始化一个UOpenDriveToMap相关的对象，
  // 并返回指向该对象的指针，可能涉及到对象的创建、配置等初始化操作。
  UFUNCTION(BlueprintPure)
  UOpenDriveToMap* GetOpenDriveToMap();
// 声明一个在蓝图中可调用且为纯函数（通过BlueprintPure修饰）的函数GetOpenDriveToMap，
  // 函数返回值是UOpenDriveToMap* 指针类型，意味着该函数不会修改类的成员状态，只是返回与UOpenDriveToMap相关的对象指针，
  // 推测用于获取之前已经初始化或者关联好的UOpenDriveToMap对象，供外部代码使用其功能或者获取相关信息。
  UFUNCTION(BlueprintCallable)
  void SetOpenDriveToMap(UOpenDriveToMap* ToSet);
// 声明一个可在蓝图中调用的函数SetOpenDriveToMap，接收一个UOpenDriveToMap* 类型的参数ToSet，
  // 推测其功能是将传入的UOpenDriveToMap对象指针赋值给类内部对应的成员变量或者进行相关关联操作，
  // 用于更新类中所管理的UOpenDriveToMap对象相关信息。
  UFUNCTION(BlueprintCallable)
  void DestroyOpenDriveToMap();
// 声明一个可在蓝图中调用的函数DestroyOpenDriveToMap，从函数名推测其功能是用于销毁已经存在的UOpenDriveToMap对象，
  // 可能会进行释放内存、清理相关资源以及解除与该对象关联等操作，确保对象被正确地清理掉，避免资源泄漏等问题。
};
