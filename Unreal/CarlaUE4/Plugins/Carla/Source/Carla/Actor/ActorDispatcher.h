// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 头文件保护宏，确保该头文件在编译过程中只被包含一次，避免重复定义等问题
#pragma once

// 引入各种必要的头文件，用于处理Actor相关的定义、描述、注册以及生成结果等信息，是Carla项目中与Actor相关的基础类定义所在
#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Actor/ActorSpawnResult.h"

// 引入用于处理容器数组以及函数模板相关的头文件，方便进行数据存储和函数式编程相关操作
#include "Containers/Array.h"
#include "Templates/Function.h"

// 引入用于生成代码相关的头文件，可能与UE4的反射系统配合，实现类的动态创建等功能（由UE4的特定宏机制生成相关代码）
#include "ActorDispatcher.generated.h"

// 前置声明类，告知编译器后续会定义该类，此处只是声明，暂时不需要包含其完整头文件定义，ACarlaActorFactory类应该是用于创建Actor的工厂类
class ACarlaActorFactory;

/// 负责将ActorDefinitions绑定到生成函数，以及
/// 维护所有已生成参与者的注册表
UCLASS()
class CARLA_API UActorDispatcher : public UObject
{
  GENERATED_BODY()

public:

  // 定义一个函数类型别名SpawnFunctionType，它表示一个可调用对象类型（通常是函数指针或者可调用的函数对象等），
  // 该可调用对象接收一个FTransform（表示变换信息，如位置、旋转、缩放）和一个FActorDescription（Actor的描述信息）作为参数，
  // 并返回一个FActorSpawnResult（Actor生成结果）类型的结果
  using SpawnFunctionType = TFunction<FActorSpawnResult(const FTransform &, const FActorDescription &)>;

  // 将给定的ActorDefinition（Actor的定义信息）绑定到对应的SpawnFunction（生成函数）上。
  // 当后续需要根据匹配的描述来生成Actor时，对应的生成函数就会被调用。
  // 注意：无效的定义将会被忽略，具体的有效性判断可能在函数内部实现或者遵循相关约定
  void Bind(FActorDefinition Definition, SpawnFunctionType SpawnFunction);

  // 将指定的ACarlaActorFactory对象中所有的Actor定义绑定到其对应的生成函数上。
  // 同样，无效的定义将会被忽略，此函数方便了对一批Actor定义进行统一的绑定操作
  void Bind(ACarlaActorFactory &ActorFactory);

 // 根据给定的变换信息（Transform）和Actor描述信息（ActorDescription）在相应位置生成一个Actor。
 // 创建出来的Actor可以通过调用DestroyActor函数来进行销毁。
 // 函数返回一个包含生成函数结果状态（EActorSpawnResultStatus）和指向生成的FCarlaActor对象指针的键值对（TPair）。
 // 如果生成状态不是成功状态，那么返回的FCarlaActor指针是无效的（可能为nullptr等情况）
  TPair<EActorSpawnResultStatus, FCarlaActor*> SpawnActor(
  const FTransform &Transform,
  FActorDescription ActorDescription,
  FCarlaActor::IdType DesiredId = 0);

  // 在给定的变换位置（Transform）基于给定的Actor描述信息（ActorDescription）重新生成一个Actor。
  // 此函数用于重新生成处于休眠状态的Actor，创建出来的Actor同样可以通过调用DestroyActor函数来销毁
  // 函数返回重新生成的AActor对象指针
  AActor* ReSpawnActor(
      const FTransform &Transform,
      FActorDescription ActorDescription);

  // 将指定Id的Actor设置为休眠状态，需要传入对应的Id以及UCarlaEpisode对象指针（可能用于关联具体的场景等上下文信息），
  // 具体的休眠实现逻辑应该在函数内部处理，比如暂停其更新、减少资源占用等相关操作
  void PutActorToSleep(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode);

  // 将指定Id的处于休眠状态的Actor唤醒，使其恢复正常运行状态，同样需要传入Id和UCarlaEpisode对象指针来确定具体操作的Actor和上下文环境
  void WakeActorUp(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode);

  // 销毁指定的Actor，并将其从Actor注册表中正确移除，以保证注册表数据的准确性和资源的合理释放。
  // 如果传入的Actor已经被销毁或者已经被标记为销毁状态，那么函数返回true；如果传入的Actor不可销毁（比如受保护等情况）或者为空指针，函数返回false
  bool DestroyActor(AActor *Actor);

  bool DestroyActor(FCarlaActor::IdType ActorId);

  // 注册一个并非通过“SpawnActor”函数创建的Actor，但这个Actor需要被保留在Actor注册表中，方便统一管理等操作。
  // 函数返回注册后的FCarlaActor对象指针，需要传入实际的AActor对象引用以及其对应的Actor描述信息，还可以指定一个期望的Id（默认为0）
  FCarlaActor* RegisterActor(
      AActor &Actor,
      FActorDescription ActorDescription,
      FActorRegistry::IdType DesiredId = 0);

  // 获取所有的Actor定义信息，以常量引用的形式返回存储Actor定义的数组（TArray<FActorDefinition>），
  // 外部调用此函数只能读取定义信息，不能修改，保证数据的安全性
  const TArray<FActorDefinition> &GetActorDefinitions() const
  {
    return Definitions;
  }

  // 获取Actor注册表信息，以常量引用的形式返回FActorRegistry对象，外部调用此函数只能读取注册表内容，不能修改，
  // 同样是为了保证数据的安全性，避免外部随意篡改注册表数据
  const FActorRegistry &GetActorRegistry() const
  {
    return Registry;
  }

  // 获取Actor注册表信息，返回FActorRegistry对象的引用，通过此函数获取到的引用可以对注册表进行修改操作，
  // 提供了一种灵活的方式来访问和操作注册表，与上面的常量引用获取函数形成对比，满足不同的使用场景需求
  FActorRegistry &GetActorRegistry()
  {
    return Registry;
  }

private:

  // 定义一个UE4的函数宏（UFUNCTION），表明这是一个可以被UE4反射系统识别和调用的函数，
  // 该函数用于处理Actor被销毁时的相关逻辑，当Actor被销毁时会自动触发这个函数，参数为被销毁的AActor对象指针
  UFUNCTION()
  void OnActorDestroyed(AActor *Actor);

  // 存储所有Actor的定义信息，是一个FActorDefinition类型的数组，通过前面的Bind函数等操作来填充，
  // 用于后续根据定义来生成或者管理Actor等相关操作
  TArray<FActorDefinition> Definitions;

  // 存储所有的Actor生成函数，是一个SpawnFunctionType类型（前面定义的函数类型别名）的数组，
  // 与Definitions数组中的定义信息相对应，用于在生成Actor时根据定义找到对应的生成函数来执行生成操作
  TArray<SpawnFunctionType> SpawnFunctions;

  // 存储所有Actor对应的类信息，是一个TSubclassOf<AActor>类型（用于表示AActor的子类类型）的数组，
  // 可以用于动态创建或者查找具体的Actor类等相关操作，与Actor的生成和管理流程相关
  TArray<TSubclassOf<AActor>> Classes;

  // 存储已生成Actor的注册表信息，是一个FActorRegistry类型的对象，用于记录和管理所有已经生成的Actor，
  // 比如Actor的Id、状态等相关信息，通过这个注册表可以方便地对Actor进行查找、操作以及维护等工作
  FActorRegistry Registry;

};
