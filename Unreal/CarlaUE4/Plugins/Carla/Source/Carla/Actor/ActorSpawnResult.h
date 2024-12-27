// 版权声明
// 版权 (c) 2017 计算机视觉中心 (CVC) 巴塞罗那自治大学 (UAB)。
//
// 本作品在 MIT 许可证条款下授权。
// 有关副本，请参见 <https://opensource.org/licenses/MIT>。

#pragma once

#include "ActorSpawnResult.generated.h"

/// 演员生成结果的有效类型列表。
UENUM(BlueprintType)
enum class EActorSpawnResultStatus : uint8
{
  Success UMETA(DisplayName = "成功"),  // 生成成功
  InvalidDescription UMETA(DisplayName = "无效的演员描述"),  // 演员描述无效
  Collision UMETA(DisplayName = "生成位置发生碰撞"),  // 生成位置发生碰撞
  UnknownError UMETA(DisplayName = "未知错误"),  // 未知错误

  SIZE UMETA(Hidden)  // 枚举大小，用于内部计算，隐藏不显示
};

/// 演员生成函数的结果。
USTRUCT(BlueprintType)
struct FActorSpawnResult
{
  GENERATED_BODY()  // 自动生成的 UE4 代码

  FActorSpawnResult() = default;  // 默认构造函数

  // 显式构造函数，根据传入的演员指针设置生成结果
  explicit FActorSpawnResult(AActor *InActor)
    : Actor(InActor),
      Status(Actor != nullptr ?
          EActorSpawnResultStatus::Success :  // 如果演员指针有效，设置为成功
          EActorSpawnResultStatus::UnknownError) {}  // 否则设置为未知错误

  // 将枚举值转换为字符串，便于调试和显示
  static FString StatusToString(EActorSpawnResultStatus Status);

  // 检查生成结果是否有效
  bool IsValid() const
  {
    return (Actor != nullptr) && (Status == EActorSpawnResultStatus::Success);  // 演员指针有效且状态为成功
  }

  // 生成的演员指针
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  AActor *Actor = nullptr;

  // 生成状态
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  EActorSpawnResultStatus Status = EActorSpawnResultStatus::UnknownError;
};
