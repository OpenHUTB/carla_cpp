// Copyright (c) 2017 计算机视觉中心 (CVC) 巴塞罗那自治大学 (UAB)。
//
// 本作品在 MIT 许可证下发布。
// 有关副本，请参见 <https://opensource.org/licenses/MIT>。

#pragma once

#include "Carla/Actor/CarlaActor.h"

#include "Containers/Map.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Iterator.h>
#include <compiler/enable-ue4-macros.h>

#include <unordered_map>

/// Carla 演员的所有注册表。
class FActorRegistry
{
public:

  using IdType = FCarlaActor::IdType;
  using ValueType = TSharedPtr<FCarlaActor>;

private:

  // 使用 TMap 作为数据库类型，而不是 std::unordered_map
  using DatabaseType = TMap<IdType, TSharedPtr<FCarlaActor>>;

  // ===========================================================================
  /// @name 演员注册函数
  // ===========================================================================
  /// @{
public:

  /// 在数据库中注册 @a Actor。将为该演员分配一个新的 ID。
  ///
  /// @warning 如果一个演员被注册多次，结果是未定义的。
  FCarlaActor* Register(AActor &Actor, FActorDescription Description, IdType DesiredId = 0);

  /// 通过 ID 注销演员。
  void Deregister(IdType Id);

  /// 通过指针注销演员。
  void Deregister(AActor *Actor);

  /// @}
  // ===========================================================================
  /// @name 查询函数
  // ===========================================================================
  /// @{

  /// 返回演员的数量。
  int32 Num() const
  {
    return Actors.Num();
  }

  /// 检查注册表是否为空。
  bool IsEmpty() const
  {
    return Num() == 0;
  }

  /// 检查指定的 ID 是否存在于数据库中。
  bool Contains(uint32 Id) const
  {
    return ActorDatabase.Find(Id) != nullptr;
  }

  /// 通过 ID 查找 Carla 演员。
  FCarlaActor* FindCarlaActor(IdType Id)
  {
    ValueType* CarlaActorPtr = ActorDatabase.Find(Id);
    return CarlaActorPtr ? CarlaActorPtr->Get() : nullptr;
  }

  /// 通过 ID 查找 Carla 演员（常量版本）。
  const FCarlaActor* FindCarlaActor(IdType Id) const
  {
    const ValueType* CarlaActorPtr = ActorDatabase.Find(Id);
    return CarlaActorPtr ? CarlaActorPtr->Get() : nullptr;
  }

  /// 通过 AActor 指针查找 Carla 演员。
  FCarlaActor* FindCarlaActor(const AActor *Actor)
  {
    IdType* PtrToId = Ids.Find(Actor);
    return PtrToId ? FindCarlaActor(*PtrToId) : nullptr;
  }

  /// 通过 AActor 指针查找 Carla 演员（常量版本）。
  const FCarlaActor* FindCarlaActor(const AActor *Actor) const
  {
    const IdType* PtrToId = Ids.Find(Actor);
    return PtrToId ? FindCarlaActor(*PtrToId) : nullptr;
  }

  /// 从流 ID 获取描述。
  FString GetDescriptionFromStream(carla::streaming::detail::stream_id_type Id);

  /// 将演员置于休眠状态。
  void PutActorToSleep(IdType Id, UCarlaEpisode* CarlaEpisode);

  /// 唤醒演员。
  void WakeActorUp(IdType Id, UCarlaEpisode* CarlaEpisode);

  /// @}
  // ===========================================================================
  /// @name 范围迭代支持
  // ===========================================================================
  /// @{
public:

  /// 返回注册表的迭代器开始位置。
  auto begin() const noexcept
  {
    return ActorDatabase.begin();
  }

  /// 返回注册表的迭代器结束位置。
  auto end() const noexcept
  {
    return ActorDatabase.end();
  }

  /// @}
private:

  /// 创建一个新的 Carla 演员。
  TSharedPtr<FCarlaActor> MakeCarlaActor(
    IdType Id,
    AActor &Actor,
    FActorDescription Description,
    carla::rpc::ActorState InState) const;

  /// 创建一个假的 Carla 演员。
  FCarlaActor MakeFakeActor(
    AActor &Actor) const;

  /// 存储所有 Carla 演员的 TMap。
  TMap<IdType, AActor *> Actors;

  /// 存储演员指针到 ID 的映射。
  TMap<AActor *, IdType> Ids;

  /// 存储演员 ID 到 Carla 演员的映射。
  DatabaseType ActorDatabase;

  /// 静态变量，用于生成唯一的演员 ID。
  static IdType ID_COUNTER;
};
