// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/CarlaActor.h"

#include "Containers/Map.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Iterator.h>
#include <compiler/enable-ue4-macros.h>

#include <unordered_map>

/// 所有Carla角色的注册表
class FActorRegistry
{
public:

  using IdType = FCarlaActor::IdType;
  using ValueType = TSharedPtr<FCarlaActor>;

private:

  //使用 DatabaseType = std::unordered_map<IdType, FCarlaActor>;
  using DatabaseType = TMap<IdType, TSharedPtr<FCarlaActor>>;

  // ===========================================================================
  /// 名称 参与者注册函数
  // ===========================================================================
  /// @{
public:

  /// 在数据库中注册@a Actor。将为此分配一个新的ID
  /// 参与者
  ///
  /// @warning Undefined if an actor is registered more than once.
  FCarlaActor* Register(AActor &Actor, FActorDescription Description, IdType DesiredId = 0);

  void Deregister(IdType Id);

  void Deregister(AActor *Actor);

  /// @}
  // ===========================================================================
  /// 名称 查找功能
  // ===========================================================================
  /// @{

  int32 Num() const//返回当前管理的演员数量。const关键字表示这个函数不会修改任何成员变量
  {
    return Actors.Num();
  }

  bool IsEmpty() const//检查当前是否有任何演员。如果没有演员（即数量为0），则返回true；否则返回false
  {
    return Num() == 0;
  }

  bool Contains(uint32 Id) const
  {
    return ActorDatabase.Find(Id) != nullptr;
  }
//四个FindCarlaActor函数是类的成员函数，用于在不同的上下文中查找FCarlaActor对象
//这些函数展示了如何在C++中处理const正确性，即如何编写不会修改对象状态的函数，并返回指向const对象的指针以防止调用者修改这些对象
  FCarlaActor* FindCarlaActor(IdType Id)
  {
    ValueType* CarlaActorPtr = ActorDatabase.Find(Id);
    return CarlaActorPtr ? CarlaActorPtr->Get() : nullptr;
  }

  const FCarlaActor* FindCarlaActor(IdType Id) const
  {
    const ValueType* CarlaActorPtr = ActorDatabase.Find(Id);
    return CarlaActorPtr ? CarlaActorPtr->Get() : nullptr;
  }

  FCarlaActor* FindCarlaActor(const AActor *Actor)
  {
    IdType* PtrToId = Ids.Find(Actor);
    return PtrToId ? FindCarlaActor(*PtrToId) : nullptr;
  }

  const FCarlaActor* FindCarlaActor(const AActor *Actor) const
  {
    const IdType* PtrToId = Ids.Find(Actor);
    return PtrToId ? FindCarlaActor(*PtrToId) : nullptr;
  }

  FString GetDescriptionFromStream(carla::streaming::detail::stream_id_type Id);

  void PutActorToSleep(IdType Id, UCarlaEpisode* CarlaEpisode);//用于将指定ID的演员设置为“睡眠”状态。UCarlaEpisode参数用于指定这个操作是在哪个特定的情节或场景中进行的

  void WakeActorUp(IdType Id, UCarlaEpisode* CarlaEpisode);

  /// @}
  // ===========================================================================
  ///名称范围迭代支持
  // ===========================================================================
  /// @{
public:

  auto begin() const noexcept
  {
    return ActorDatabase.begin();
  }

  auto end() const noexcept
  {
    return ActorDatabase.end();
  }

  /// @}
private://类的私有部分，用于管理CARLA模拟环境中的演员
//创建一个FCarlaActor对象，并返回一个智能指针指向它。它接受一个演员ID、一个AActor引用、一个描述对象和一个状态对象作为参数。
  TSharedPtr<FCarlaActor> MakeCarlaActor(
    IdType Id,
    AActor &Actor,
    FActorDescription Description,
    carla::rpc::ActorState InState) const;

  FCarlaActor MakeFakeActor(
    AActor &Actor) const;

  TMap<IdType, AActor *> Actors;//一个映射，将演员ID映射到AActor指针。这允许通过ID快速查找演员对象

  TMap<AActor *, IdType> Ids;//另一个映射，将AActor指针映射到演员ID。这允许通过演员对象快速查找其ID

  DatabaseType ActorDatabase;//数据库类型的成员变量，用于存储演员信息

  static IdType ID_COUNTER;//一个静态成员变量，用于生成唯一的演员ID。每次创建新演员时，这个计数器可能会递增，以确保每个演员都有一个唯一的ID。
};
