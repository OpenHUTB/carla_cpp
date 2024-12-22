// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 预处理指令，用于保证该头文件在编译过程中只会被包含一次，避免因重复包含而引发编译错误，这是C++中常见的防止头文件重复包含的做法。

#include <compiler/disable-ue4-macros.h>
// 引入一个头文件，从文件名推测其作用可能是禁用虚幻引擎4（UE4）相关的一些宏定义，也许是为了避免在后续包含其他文件或进行代码编写时，UE4宏带来的一些潜在冲突或不符合当前需求的行为。
#include <carla/rpc/QualityLevel.h>
// 包含这个头文件，里面应该定义了与carla项目中RPC（远程过程调用）相关的画质等级（QualityLevel）的类型及相关内容，后续代码会基于这个文件里的定义来进行一些操作和关联。
#include <compiler/enable-ue4-macros.h>
// 引入这个头文件，与前面的“disable-ue4-macros.h”相对应，可能是用于重新启用之前被禁用的虚幻引擎4相关宏定义，恢复到正常的UE4宏可用的状态，方便使用UE4特定的功能和语法等。


#include "QualityLevelUE.generated.h"
// 包含这个自动生成代码相关的头文件，在虚幻引擎开发中，这类头文件配合引擎的代码生成工具，会生成诸如反射、序列化等必要的代码，以保障类或者枚举等能在虚幻引擎体系下正常工作，例如能在蓝图中使用、进行对象持久化等操作。

#define CARLA_ENUM_FROM_RPC(e) static_cast<uint8>(carla::rpc::QualityLevel:: e) 
 // 将carla::rpc::QualityLevel类型 转换为 uint8

// 启动时的画质等级，通过命令指定：CarlaUE4.exe -quality-level=Low
// 这是一段注释，用于说明下面定义的枚举类型的用途，即表示启动时的画质等级，并且说明了可以通过在命令行输入“CarlaUE4.exe -quality-level=Low”这样的命令来指定画质等级。
UENUM(BlueprintType)
// 使用“UENUM”宏来定义一个虚幻引擎中的枚举类型，“BlueprintType”参数表示这个枚举类型可以在蓝图（UE4的可视化脚本系统）中使用，方便在蓝图里进行相关的逻辑设置和操作等。

enum class EQualityLevel : uint8
{
  Null = 0, 
// 解决 UE4.24 枚举问题
  Low    = CARLA_ENUM_FROM_RPC(Low)    UMETA(DisplayName = "Low"),
  // Medium = CARLA_ENUM_FROM_RPC(Medium) UMETA(DisplayName = "Medium"),
// 这里是被注释掉的枚举值定义，原本应该是按照和“Low”类似的方式定义“Medium”这个画质等级对应的枚举值，并设置在蓝图中的显示名称，可能是暂时不需要或者后续再完善等原因被注释掉了。
  // High   = CARLA_ENUM_FROM_RPC(High)   UMETA(DisplayName = "High"),
 // 同样是被注释掉的枚举值定义，用于定义“High”画质等级对应的枚举值及蓝图显示名称相关设置，和上面情况类似，暂时未启用。
  Epic   = CARLA_ENUM_FROM_RPC(Epic)   UMETA(DisplayName = "Epic"),
// 定义枚举值“Epic”，通过“CARLA_ENUM_FROM_RPC”宏将“carla::rpc::QualityLevel”中的“Epic”值转换为uint8类型赋值，并用“UMETA”宏设置在蓝图中显示的名称为“Epic”。


  SIZE      UMETA(Hidden),
// 定义一个枚举值“SIZE”，并通过“UMETA”宏将其标记为“Hidden”，意味着在一些常规的可视化界面（比如蓝图属性查看等场景）中这个枚举值是隐藏的，可能它有特殊的内部使用用途，不希望对外展示。
  INVALID   UMETA(Hidden)
// 定义枚举值“INVALID”，同样使用“UMETA”宏标记为“Hidden”，使其在常规可视化界面中不显示，大概也是用于特定的内部逻辑判断或者错误处理等情况。
};

static_assert(
    static_cast<uint8>(EQualityLevel::SIZE) == static_cast<uint8>(carla::rpc::QualityLevel::SIZE),
    "Please keep these two enums in sync.");  
// 确保该类EQualityLevel中画质数量 和 carla::rpc::QualityLevel的画质数量相等

#undef CARLA_ENUM_FROM_RPC
// 取消之前定义的“CARLA_ENUM_FROM_RPC”宏定义，避免这个宏在后续代码中可能产生的意外影响，确保代码的作用域和逻辑清晰性，符合良好的代码编写规范。
