// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// 上面两行注释意思是版权所有 (c) 2017 巴塞罗那自治大学（UAB）计算机视觉中心（CVC）
//
// This work is licensed under the terms of the MIT license.（这项工作是根据MIT许可证的条款授权的）
// For a copy, see <https://opensource.org/licenses/MIT>.（要获取副本，请前往<https://opensource.org/licenses/MIT>）

#pragma once  // 确保头文件内容在每个编译单元中只被包含一次

// 包含禁用虚幻引擎4（UE4）宏的头文件
#include <compiler/disable-ue4-macros.h>

// 包含CARLA RPC中定义的角色属性类型的头文件
#include <carla/rpc/ActorAttributeType.h> 

// 包含启用虚幻引擎4（UE4）宏的头文件
#include <compiler/enable-ue4-macros.h>

 // 包含自动生成的头文件，通常用于Unreal Engine 4 (UE4) 中的反射系统
#include "ActorAttribute.generated.h" 

// 定义一个宏，用于将carla::rpc::ActorAttributeType枚举值转换为uint8类型
#define CARLA_ENUM_FROM_RPC(e) static_cast<uint8>(carla::rpc::ActorAttributeType::e)

/// List of valid types for actor attributes.（角色属性有效类型的列表）

// 标记这个枚举可以在Unreal Engine的蓝图系统中使用
UENUM(BlueprintType)
enum class EActorAttributeType : uint8
{
  Null = 0, // Workarround for UE4.24 issue with enums
  Bool      = CARLA_ENUM_FROM_RPC(Bool)      UMETA(DisplayName = "Bool"),
  Int       = CARLA_ENUM_FROM_RPC(Int)       UMETA(DisplayName = "Integer"),
  Float     = CARLA_ENUM_FROM_RPC(Float)     UMETA(DisplayName = "Float"),
  String    = CARLA_ENUM_FROM_RPC(String)    UMETA(DisplayName = "String"),
  RGBColor  = CARLA_ENUM_FROM_RPC(RGBColor)  UMETA(DisplayName = "RGB Color (comma separated)"),

  SIZE        UMETA(Hidden),
  INVALID     UMETA(Hidden)
};

#undef CARLA_ENUM_FROM_RPC

/// Definition of an actor variation. Variations represent attributes of the
/// actor that the user can modify in order to generate variations of the same
/// actor.
///
/// A list of recommended values is given. If bRestrictToRecommended is true,
/// only recommended values are accepted as valid.
USTRUCT(BlueprintType)
struct CARLA_API FActorVariation
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Id;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  EActorAttributeType Type = EActorAttributeType::String;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FString> RecommendedValues;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bRestrictToRecommended = false;
};

/// An actor attribute, may be an intrinsic (non-modifiable) attribute of the
/// actor or an user-defined actor variation.
USTRUCT(BlueprintType)
struct CARLA_API FActorAttribute
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Id;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  EActorAttributeType Type = EActorAttributeType::String;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Value;
};
