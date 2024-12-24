// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"  // 引入Carla头文件
#include "Carla/Actor/ActorSpawnResult.h"  // 引入ActorSpawnResult头文件

// 将EActorSpawnResultStatus类型的状态转换为字符串表示
FString FActorSpawnResult::StatusToString(EActorSpawnResultStatus InStatus)
{
  // 静态断言：确保EActorSpawnResultStatus枚举的大小为4字节
  static_assert(
      static_cast<uint8>(EActorSpawnResultStatus::SIZE) == 4u,
      "If you add a new status, please update this function.");  // 如果你添加了新的状态，请更新此函数

  // 根据传入的状态值进行匹配，返回相应的字符串
  switch (InStatus)
  {
    case EActorSpawnResultStatus::Success:  // 如果状态是Success
      return TEXT("Success");  // 返回"Success"字符串
    case EActorSpawnResultStatus::InvalidDescription:  // 如果状态是InvalidDescription
      return TEXT("Spawn failed because of invalid actor description");  // 返回"因无效的Actor描述而生成失败"的字符串
    case EActorSpawnResultStatus::Collision:  // 如果状态是Collision
      return TEXT("Spawn failed because of collision at spawn position");  // 返回"因生成位置发生碰撞而生成失败"的字符串
    case EActorSpawnResultStatus::UnknownError:  // 如果状态是UnknownError
    default:  // 默认处理其他未知的状态
      return TEXT("Unknown error while trying to spawn actor");  // 返回"尝试生成Actor时出现未知错误"的字符串
  }
}
