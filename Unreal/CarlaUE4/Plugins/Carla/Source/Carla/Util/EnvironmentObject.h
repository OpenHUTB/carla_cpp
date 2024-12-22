// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 预处理指令，用于确保该头文件在整个编译过程中只会被包含一次

#include "Carla/Util/BoundingBox.h"
// 引入Carla项目中Util目录下的BoundingBox.h头文件用于表示物体的包围盒信息

#include <compiler/disable-ue4-macros.h>
// 引入一个用于禁用UE4（虚幻引擎4）相关宏定义的头文件
#include <carla/rpc/ObjectLabel.h>
// 引入carla项目中rpc模块下的ObjectLabel.h头文件

#include <compiler/enable-ue4-macros.h>
// 引入一个用于启用UE4（虚幻引擎4）相关宏定义的头文件

#include "BoundingBox.h"
// 再次引入一个名为BoundingBox.h的头文件

#include "EnvironmentObject.generated.h"
// 引入由虚幻引擎的代码生成工具生成的头文件（EnvironmentObject.generated.h）

namespace crp = carla::rpc;
// 使用namespace别名定义，将carla::rpc命名空间简称为crp，方便后续代码中引用该命名空间下的类型、函数等内容，使代码更简洁易读。

enum EnvironmentObjectType
{
  Invalid,
  Vehicle,
  Character,
  TrafficLight,
  ISMComp,
  SMComp,
  SKMComp
};
// 定义一个枚举类型EnvironmentObjectType，用于列举不同种类的环境对象类型。
// 其中Invalid表示无效或未定义的类型，Vehicle表示车辆类型，Character表示角色（如行人等）类型，
// TrafficLight表示交通信号灯类型

// Name is under discussion
USTRUCT(BlueprintType)
struct CARLA_API FEnvironmentObject
{
// 使用USTRUCT宏定义一个结构体，名为FEnvironmentObject，并标记为BlueprintType类型

  GENERATED_BODY()
 // 这是一个虚幻引擎代码生成相关的宏，用于指示编译器在这里生成必要的代码

  AActor* Actor = nullptr;
// 定义一个成员变量Actor，类型为指向AActor类（AActor通常是虚幻引擎中表示游戏中角色、物体等的基类）的指针，初始化为nullptr

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Name;
// 使用UPROPERTY宏声明一个成员变量Name

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString IdStr;
// 同样使用UPROPERTY宏声明一个成员变量IdStr，类型为FString

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FTransform Transform;
// 使用UPROPERTY宏声明一个成员变量Transform，类型为FTransform（FTransform通常用于表示物体在三维空间中的变换信息，如位置、旋转、缩放等）用于存储环境对象在游戏世界中的空间变换

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FBoundingBox BoundingBox;
// 使用UPROPERTY宏声明一个成员变量BoundingBox，类型为FBoundingBox（从之前引入的相关头文件推测用于表示包围盒信息），
  // 具有相应的编辑和蓝图读写属性，用于存储环境对象的包围盒相关信息，比如界定该对象在三维空间中的范围大小等情况。
  uint64 Id = 0;

  EnvironmentObjectType Type = EnvironmentObjectType::Invalid;
// 定义一个成员变量Type，类型为之前定义的EnvironmentObjectType枚举类型，初始化为Invalid，表示默认情况下环境对象的类型为无效或未定义状态，

  crp::CityObjectLabel ObjectLabel;
// 定义一个成员变量ObjectLabel，类型为crp（即carla::rpc命名空间下的）CityObjectLabel类型，

  bool CanTick = false;
// 定义一个成员变量CanTick，类型为布尔型（bool），初始化为false，
// 可能用于表示该环境对象是否能够进行每帧更新（Tick）相关的操作，用于控制对象在游戏运行过程中的更新逻辑。
};
