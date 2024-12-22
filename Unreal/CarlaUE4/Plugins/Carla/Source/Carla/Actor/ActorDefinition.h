// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 指示此头文件被包含一次，防止重复包含

#include "Carla/Actor/ActorAttribute.h" // 包含CARLA中Actor属性的定义

#include "GameFramework/Actor.h" // 包含Unreal Engine中Actor框架的定义

#include "ActorDefinition.generated.h" // 包含Actor定义的生成文件，用于Unreal Engine的序列化

// CARLA参与者的定义，包含所有变化和属性
USTRUCT(BlueprintType) // 使用Unreal Engine的UStruct宏，标记为蓝图类型，可在Unreal Editor中使用
struct FActorDefinition
{
  GENERATED_BODY() // 由Unreal Engine生成的代码，用于创建结构体的类

  /// 唯一标识定义（无需填写）
  uint32 UId = 0u; // 定义的唯一标识符，通常由系统自动分配

  /// 用于标识参与者的显示ID
  UPROPERTY(EditAnywhere, BlueprintReadWrite) // 属性可以在编辑器中编辑，并且可以在蓝图中读写
  FString Id; // 参与者的显示ID，用于在用户界面中显示

  /// 要生成的参与者类（可选）
  ///
  /// 请注意，此参数未在客户端公开，仅用于
  /// 生成器本身
  UPROPERTY(EditAnywhere, BlueprintReadWrite) // 属性可以在编辑器中编辑，并且可以在蓝图中读写
  TSubclassOf<AActor> Class; // 参与者类，用于指定要生成的Actor类型

  /// 一个逗号分隔的标签列表
  UPROPERTY(EditAnywhere, BlueprintReadWrite) // 属性可以在编辑器中编辑，并且可以在蓝图中读写
  FString Tags; // 参与者的标签，用于分类和搜索

  /// 变体代表了用户可以修改的变量，以生成不同的变体
  /// 关于参与者
  UPROPERTY(EditAnywhere, BlueprintReadWrite) // 属性可以在编辑器中编辑，并且可以在蓝图中读写
  TArray<FActorVariation> Variations; // 参与者的变体数组，用于定义不同的配置选项

  /// 属性表示参与者可能具有的不可修改的特性
  /// 帮助用户识别和过滤参与者
  UPROPERTY(EditAnywhere, BlueprintReadWrite) // 属性可以在编辑器中编辑，并且可以在蓝图中读写
  TArray<FActorAttribute> Attributes; // 参与者的属性数组，用于定义参与者的特性
};

/// 一个包含所有变体和属性的车辆角色定义
USTRUCT(BlueprintType) // 使用Unreal Engine的UStruct宏，标记为蓝图类型，可在Unreal Editor中使用
struct FVehicleActorDefinition
{
  GENERATED_BODY() // 由Unreal Engine生成的代码，用于创建结构体的类

  /// 一个逗号分隔的标签列表
  UPROPERTY(EditAnywhere, BlueprintReadWrite) // 属性可以在编辑器中编辑，并且可以在蓝图中读写
  FString Tags; // 车辆参与者的标签，用于分类和搜索

  /// 将静态网格体分配给该参与者
  UPROPERTY(EditAnywhere, BlueprintReadWrite) // 属性可以在编辑器中编辑，并且可以在蓝图中读写
  UStaticMesh* mesh; // 车辆参与者的静态网格体，用于定义车辆的外观

  /// 颜色代表每个参与者的可用颜色
  UPROPERTY(EditAnywhere, BlueprintReadWrite) // 属性可以在编辑器中编辑，并且可以在蓝图中读写
  TArray<FColor> colors; // 车辆参与者的颜色数组，用于定义车辆的颜色选项
};
