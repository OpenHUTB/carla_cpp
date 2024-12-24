// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Actor/ActorDispatcher.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/ActorROS2Handler.h"
#include "Carla/Actor/CarlaActorFactory.h"

#include "Carla/Game/Tagger.h"
#include "Carla/Vehicle/VehicleControl.h"

#include "GameFramework/Controller.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/ros2/ROS2.h"
#include <compiler/enable-ue4-macros.h>

// UActorDispatcher类中的Bind函数，用于绑定一个Actor定义和生成函数
void UActorDispatcher::Bind(FActorDefinition Definition, SpawnFunctionType Functor)
{
  // 检查Actor定义是否有效
  if (UActorBlueprintFunctionLibrary::CheckActorDefinition(Definition))
  {
    // 分配一个唯一的ID给这个定义
    Definition.UId = static_cast<uint32>(SpawnFunctions.Num()) + 1u;
    // 将定义添加到定义列表中
    Definitions.Emplace(Definition);
    // 将生成函数添加到生成函数列表中
    SpawnFunctions.Emplace(Functor);
    // 将对应的类添加到类列表中
    Classes.Emplace(Definition.Class);
  }
  else
  {
    // 如果定义无效，记录警告日志
    UE_LOG(LogCarla, Warning, TEXT("Invalid definition '%s' ignored"), *Definition.Id);
  }
}

// UActorDispatcher类中的另一个Bind函数，用于绑定一个Actor工厂
void UActorDispatcher::Bind(ACarlaActorFactory &ActorFactory)
{
   // 遍历Actor工厂中所有定义，并绑定它们
  for (const auto &Definition : ActorFactory.GetDefinitions())
  {
    Bind(Definition, [&](const FTransform &Transform, const FActorDescription &Description) {
       // 返回Actor工厂生成的Actor
      return ActorFactory.SpawnActor(Transform, Description);
    });
  }
}

// UActorDispatcher类中的SpawnActor函数，用于生成Actor
TPair<EActorSpawnResultStatus, FCarlaActor*> UActorDispatcher::SpawnActor(
    const FTransform &Transform,
    FActorDescription Description,
    FCarlaActor::IdType DesiredId)
{
  // 检查Actor描述是否有效
  if ((Description.UId == 0u) || (Description.UId > static_cast<uint32>(SpawnFunctions.Num())))
  {
    // 如果无效，记录错误日志并返回失败结果
    UE_LOG(LogCarla, Error, TEXT("Invalid ActorDescription '%s' (UId=%d)"), *Description.Id, Description.UId);
    return MakeTuple(EActorSpawnResultStatus::InvalidDescription, nullptr);
  }

  // 记录生成Actor的日志
  UE_LOG(LogCarla, Log, TEXT("Spawning actor '%s'"), *Description.Id);

  // 设置Actor描述中的类
  Description.Class = Classes[Description.UId - 1];
  // 调用对应的生成函数生成Actor
  FActorSpawnResult Result = SpawnFunctions[Description.UId - 1](Transform, Description);

  // 如果生成结果状态为成功但未返回Actor，记录警告日志并将状态设置为未知错误
  if ((Result.Status == EActorSpawnResultStatus::Success) && (Result.Actor == nullptr))
  {
    UE_LOG(LogCarla, Warning, TEXT("ActorSpawnResult: Trying to spawn '%s'"), *Description.Id);
    UE_LOG(LogCarla, Warning, TEXT("ActorSpawnResult: Reported success but did not return an actor"));
    Result.Status = EActorSpawnResultStatus::UnknownError;
  }

  // 如果生成结果有效，则注册Actor，否则设置为nullptr
  FCarlaActor* View = Result.IsValid() ?
      RegisterActor(*Result.Actor, std::move(Description), DesiredId) : nullptr;
   // 如果注册失败，记录警告日志并检查结果状态不应为成功
  if (!View)
  {
    UE_LOG(LogCarla, Warning, TEXT("Failed to spawn actor '%s'"), *Description.Id);
    check(Result.Status != EActorSpawnResultStatus::Success);
  }
  else
  {
    // 如果注册成功，对Actor进行标记
    ATagger::TagActor(*View->GetActor(), true);
  }
  // 返回生成结果状态和Actor指针
  return MakeTuple(Result.Status, View);
}

AActor* UActorDispatcher::ReSpawnActor(
    const FTransform &Transform,
    FActorDescription Description)
{
  // 检查Description是否有效，如果UId为0或超出SpawnFunctions数组的大小，则返回nullptr
  if ((Description.UId == 0u) || (Description.UId > static_cast<uint32>(SpawnFunctions.Num())))
  {
    UE_LOG(LogCarla, Error, TEXT("Invalid ActorDescription '%s' (UId=%d)"), *Description.Id, Description.UId);
    return nullptr;
  }

  // 记录日志，表示开始生成指定的Actor
  UE_LOG(LogCarla, Log, TEXT("Spawning actor '%s'"), *Description.Id);

  // 根据UId从Classes数组中获取对应的类，并赋值给Description.Class
  Description.Class = Classes[Description.UId - 1];
  // 使用对应的SpawnFunction生成Actor
  FActorSpawnResult Result = SpawnFunctions[Description.UId - 1](Transform, Description);

  // 如果生成结果状态为成功，但未返回Actor，则记录警告并返回nullptr
  if ((Result.Status == EActorSpawnResultStatus::Success) && (Result.Actor == nullptr))
  {
    UE_LOG(LogCarla, Warning, TEXT("ActorSpawnResult: Trying to spawn '%s'"), *Description.Id);
    UE_LOG(LogCarla, Warning, TEXT("ActorSpawnResult: Reported success but did not return an actor"));
    Result.Status = EActorSpawnResultStatus::UnknownError;
    return nullptr;
  }

  // 如果生成结果状态为成功，则返回生成的Actor
  if (Result.Status == EActorSpawnResultStatus::Success)
  {
    return Result.Actor;
  }

  // 如果生成失败，则返回nullptr
  return nullptr;
}

bool UActorDispatcher::DestroyActor(FCarlaActor::IdType ActorId)
{
  // 检查要销毁的Actor是否在注册表中存在
  FCarlaActor* View = Registry.FindCarlaActor(ActorId);

  // 如果Actor不在注册表中或已经被标记为待销毁，则返回false
  if (!View)
  {
    UE_LOG(LogCarla, Warning, TEXT("Trying to destroy actor that is not in the registry"));
    return false;
  }

  const FString &Id = View->GetActorInfo()->Description.Id;

  // 获取Actor的控制器，如果存在，则尝试销毁
  AActor* Actor = View->GetActor();
  if(Actor)
  {
    APawn* Pawn = Cast<APawn>(Actor);
    AController* Controller = (Pawn != nullptr ? Pawn->GetController() : nullptr);
    if (Controller != nullptr)
    {
      UE_LOG(LogCarla, Log, TEXT("Destroying actor's controller: '%s'"), *Id);
      bool Success = Controller->Destroy();
      if (!Success)
      {
        UE_LOG(LogCarla, Error, TEXT("Failed to destroy actor's controller: '%s'"), *Id);
      }
    }

    // 销毁Actor
    // ...（代码未完整，以下为注释）
    // 如果Actor存在，调用Destroy函数尝试销毁
    // 如果销毁失败，记录错误日志
    // 返回true表示销毁操作已尝试执行
    return true;
  }
  // 如果Actor不存在，返回false表示销毁操作未执行
  return false;
}

    UE_LOG(LogCarla, Log, TEXT("UActorDispatcher::Destroying actor: '%s' %x"), *Id, Actor);
    UE_LOG(LogCarla, Log, TEXT("            %s"), Actor?*Actor->GetName():*FString("None"));
    if (!Actor || !Actor->Destroy())
    {
      UE_LOG(LogCarla, Error, TEXT("Failed to destroy actor: '%s'"), *Id);
      return false;
    }
  }

  Registry.Deregister(ActorId);

  return true;
}

FCarlaActor* UActorDispatcher::RegisterActor(
    AActor &Actor, FActorDescription Description,
    FActorRegistry::IdType DesiredId)
{
  FCarlaActor* View = Registry.Register(Actor, Description, DesiredId);
  if (View)
  {
    // 待办事项：支持外部角色销毁
    Actor.OnDestroyed.AddDynamic(this, &UActorDispatcher::OnActorDestroyed);

    // ROS2 中 actor 到 ros_name 的映射
    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
    {
      // 参与者 ros_name
      std::string RosName;
      // 遍历actor的描述中的所有变体（属性）
      for (auto &&Attr : Description.Variations)
      {// 如果找到一个键为"ros_name"的属性
        if (Attr.Key == "ros_name")
        {// 将该属性的值（假设是TCHAR类型）转换为UTF-8编码的std::string
          RosName = std::string(TCHAR_TO_UTF8(*Attr.Value.Value));
        }
      }
      // 将actor的ID也转换为UTF-8编码的std::string
      const std::string id = std::string(TCHAR_TO_UTF8(*Description.Id));
      // 检查ROS名称是否与actor的ID相同
      if (RosName == id) {
        if(RosName.find("vehicle") != std::string::npos)
        {// 构造一个新的车辆名称，包含"vehicle"和actor的ID（通过View->GetActorId()
          std::string VehicleName = "vehicle" + std::to_string(View->GetActorId());
          // 将这个新的车辆名称添加到ROS 2系统中，与actor关联
          ROS2->AddActorRosName(static_cast<void*>(&Actor), VehicleName);
        }
        else
        { // 如果ROS名称不包含"vehicle"，则查找最后一个'.'字符的位置
          size_t pos = RosName.find_last_of('.');
          if (pos != std::string::npos) {
            std::string lastToken = RosName.substr(pos + 1) + "__";
            ROS2->AddActorRosName(static_cast<void*>(&Actor), lastToken);
          }
        }
      } else {
        ROS2->AddActorRosName(static_cast<void*>(&Actor), RosName);
      }

      // 英雄载具控制器
      for (auto &&Attr : Description.Variations)
      { // 如果找到一个键为"role_name"且值为"hero"或"ego"的属性
        if (Attr.Key == "role_name" && (Attr.Value.Value == "hero" || Attr.Value.Value == "ego"))
        {
          ROS2->AddActorCallback(static_cast<void*>(&Actor), RosName, [RosName](void *Actor, carla::ros2::ROS2CallbackData Data) -> void
          {
            AActor *UEActor = reinterpret_cast<AActor *>(Actor);
            ActorROS2Handler Handler(UEActor, RosName);
            boost::variant2::visit(Handler, Data);
          });
        }
      }
    }
    #endif
  }
  return View;
}
// UActorDispatcher 类是一个用于管理CARLA模拟器中actor（如车辆、行人等）的调度器。
// 它提供了将actor置于休眠状态、唤醒actor以及处理actor销毁事件的方法
void UActorDispatcher::PutActorToSleep(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode)
{
  Registry.PutActorToSleep(Id, CarlaEpisode);
}
// 唤醒指定的actor
void UActorDispatcher::WakeActorUp(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode)
{// 调用Registry的WakeActorUp方法，唤醒指定的actor
  Registry.WakeActorUp(Id, CarlaEpisode);
}

void UActorDispatcher::OnActorDestroyed(AActor *Actor)
{
  FCarlaActor* CarlaActor = Registry.FindCarlaActor(Actor);// 尝试在Registry中找到与该Unreal Engine actor对应的CARLA actor
  if (CarlaActor)
  {
    if (CarlaActor->IsActive())
    {
      Registry.Deregister(CarlaActor->GetActorId());
    }
  }
// 如果定义了WITH_ROS2宏，表示项目集成了ROS 2
  #if defined(WITH_ROS2)
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  if (ROS2->IsEnabled())
  {
    ROS2->RemoveActorRosName(reinterpret_cast<void *>(Actor));
  }
  #endif
}
