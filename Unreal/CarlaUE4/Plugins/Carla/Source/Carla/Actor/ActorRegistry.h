// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 指示此头文件被包含一次，防止重复包含

#include "Carla/Actor/CarlaActor.h" // 包含CARLA中的Actor类定义
#include "Containers/Map.h" // 包含Unreal Engine的Map容器定义
#include <compiler/disable-ue4-macros.h> // 禁用UE4宏，防止与carla库的宏冲突
#include <carla/Iterator.h> // 包含carla库的迭代器定义
#include <compiler/enable-ue4-macros.h> // 启用UE4宏
#include <unordered_map> // 包含C++标准库的unordered_map容器定义

/// 所有Carla角色的注册表
class FActorRegistry
{
public:
  using IdType = FCarlaActor::IdType; // 使用FCarlaActor的IdType作为注册表的ID类型
  using ValueType = TSharedPtr<FCarlaActor>; // 使用TSharedPtr<FCarlaActor>作为注册表的值类型

private:
  // 使用TMap作为存储Actor的数据库类型，键为ID，值为TSharedPtr<FCarlaActor>
  using DatabaseType = TMap<IdType, TSharedPtr<FCarlaActor>>;
  
  // ===========================================================================
  /// 名称 参与者注册函数
  // ===========================================================================
  /// @{
public:
  /// 在数据库中注册@a Actor。将为此分配一个新的ID
  /// 参与者
  ///
  /// @warning 如果多次注册同一个actor，行为未定义。
  FCarlaActor* Register(AActor &Actor, FActorDescription Description, IdType DesiredId = 0); // 注册Actor并分配ID
  void Deregister(IdType Id); // 通过ID注销Actor
  void Deregister(AActor *Actor); // 通过Actor指针注销Actor
  /// @}

  // ===========================================================================
  /// 名称 查找功能
  // ===========================================================================
  /// @{
  int32 Num() const // 返回当前管理的Actor数量
  {
    return Actors.Num(); // 返回Map容器中的元素数量
  }
  bool IsEmpty() const // 检查当前是否有任何Actor
  {
    return Num() == 0; // 如果Actor数量为0，则返回true
  }
  bool Contains(uint32 Id) const // 检查是否包含指定ID的Actor
  {
    return ActorDatabase.Find(Id) != nullptr; // 如果找到指定ID的Actor，则返回true
  }

  // 四个FindCarlaActor函数是类的成员函数，用于在不同的上下文中查找FCarlaActor对象
  // 这些函数展示了如何在C++中处理const正确性，即如何编写不会修改对象状态的函数，并返回指向const对象的指针以防止调用者修改这些对象
  FCarlaActor* FindCarlaActor(IdType Id) // 通过ID查找FCarlaActor
  {
    ValueType* CarlaActorPtr = ActorDatabase.Find(Id); // 在数据库中查找ID对应的Actor
    return CarlaActorPtr ? CarlaActorPtr->Get() : nullptr; // 如果找到，则返回Actor指针，否则返回nullptr
  }
  const FCarlaActor* FindCarlaActor(IdType Id) const // 通过ID查找FCarlaActor（const版本）
  {
    const ValueType* CarlaActorPtr = ActorDatabase.Find(Id); // 在数据库中查找ID对应的Actor
    return CarlaActorPtr ? CarlaActorPtr->Get() : nullptr; // 如果找到，则返回Actor指针，否则返回nullptr
  }
  FCarlaActor* FindCarlaActor(const AActor *Actor) // 通过AActor指针查找FCarlaActor
  {
    IdType* PtrToId = Ids.Find(Actor); // 在Ids映射中查找Actor对应的ID
    return PtrToId ? FindCarlaActor(*PtrToId) : nullptr; // 如果找到ID，则通过ID查找对应的FCarlaActor，否则返回nullptr
  }
  // @}
};

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
