// 版权所有 (c) 2017 计算机视觉中心 (CVC) 在巴塞罗那自治大学 (UAB)。
//
// 本作品根据 MIT 许可证的条款进行许可。
// 了解更多，请访问 <https://opensource.org/licenses/MIT>。

#pragma once

#include "GameFramework/Character.h"

#include "PedestrianParameters.generated.h"

// 定义行人性别枚举
UENUM(BlueprintType)
enum class EPedestrianGender : uint8
{
  // 其他性别
  Other       UMETA(DisplayName = "Other"),
  // 女性
  Female      UMETA(DisplayName = "Female"),
  // 男性
  Male        UMETA(DisplayName = "Male"),

  // 大小（隐藏）
  SIZE        UMETA(Hidden),
  // 无效（隐藏）
  INVALID     UMETA(Hidden)
};

// 定义行人年龄枚举
UENUM(BlueprintType)
enum class EPedestrianAge : uint8
{
  // 儿童
  Child       UMETA(DisplayName = "Child"),
  // 青少年
  Teenager    UMETA(DisplayName = "Teenager"),
  // 成人
  Adult       UMETA(DisplayName = "Adult"),
  // 老年人
  Elderly     UMETA(DisplayName = "Elderly"),

  // 大小（隐藏）
  SIZE        UMETA(Hidden),
  // 无效（隐藏）
  INVALID     UMETA(Hidden)
};

// 行人参数结构体
USTRUCT(BlueprintType)
struct CARLA_API FPedestrianParameters
{
  GENERATED_BODY()

  // 行人ID
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Id;

  // 行人角色类
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<ACharacter> Class;

  // 行人性别
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  EPedestrianGender Gender = EPedestrianGender::Other;

  // 行人年龄
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  EPedestrianAge Age = EPedestrianAge::Adult;

  // 行人速度数组
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<float> Speed;

  // 生成次数
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 Generation = 0;

  // 是否可以使用轮椅
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bCanUseWheelChair = false;
};
