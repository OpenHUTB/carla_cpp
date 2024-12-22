// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "FrameData.h" // 包含FFrameData类的头文件，定义了帧数据的结构和操作
#include "Carla/Game/CarlaEpisode.h" // 包含Carla游戏环节的头文件，表示仿真会话
#include "Carla/Actor/CarlaActor.h" // 包含Carla Actor的头文件，表示仿真中的Actor
#include "Carla/Game/CarlaEngine.h" // 包含Carla游戏引擎的头文件，管理Carla仿真的核心组件
#include "Carla/Traffic/TrafficLightController.h" // 包含交通灯控制器的头文件，管理交通灯的行为
#include "Carla/Traffic/TrafficLightGroup.h" // 包含交通灯组的头文件，表示一组交通灯
#include "Carla/MapGen/LargeMapManager.h" // 包含大地图管理器的头文件，管理大型开放世界地图
#include "Carla/Game/CarlaStatics.h" // 包含Carla静态变量和函数的头文件，提供全局访问点
#include "Carla/Settings/CarlaSettings.h" // 包含Carla设置的头文件，定义仿真的配置参数
#include "Carla/Lights/CarlaLightSubsystem.h" // 包含Carla灯光子系统的头文件，管理游戏世界的灯光

#include <compiler/disable-ue4-macros.h> // 禁用Unreal Engine的宏，防止与Carla代码冲突
#include "carla/rpc/VehicleLightState.h" // 包含Carla RPC车辆灯光状态的头文件，定义车辆灯光状态的RPC结构
#include <compiler/enable-ue4-macros.h> // 启用Unreal Engine的宏

// FFrameData::GetFrameData函数，用于收集当前帧的数据
void FFrameData::GetFrameData(UCarlaEpisode *ThisEpisode, bool bAdditionalData, bool bIncludeActorsAgain)
{
  Episode = ThisEpisode; // 将传入的游戏环节赋值给成员变量Episode
  // PlatformTime.UpdateTime(); // 更新平台时间，此行代码被注释，可能用于性能监测
  const FActorRegistry &Registry = Episode->GetActorRegistry(); // 获取游戏环节中的Actor注册表

  if (bIncludeActorsAgain) // 如果需要再次包括Actor
  {
    AddExistingActors(); // 添加已存在的Actor到帧数据中
  }

  // 遍历Actor注册表中的所有Actor
  for (auto It = Registry.begin(); It != Registry.end(); ++It)
  {
    FCarlaActor* View = It.Value().Get(); // 获取Actor的指针

    // 根据Actor的类型进行不同的数据处理
    switch (View->GetActorType())
    {
      // 对于其他类型和传感器Actor，保存其位置变换
      case FCarlaActor::ActorType::Other:
      case FCarlaActor::ActorType::Sensor:
        AddActorPosition(View); // 添加Actor的位置到帧数据
        break;

      // 对于车辆Actor，保存其位置变换、动画、灯光和车轮动画
      case FCarlaActor::ActorType::Vehicle:
        AddActorPosition(View); // 添加Actor的位置
        AddVehicleAnimation(View); // 添加车辆动画
        AddVehicleLight(View); // 添加车辆灯光状态
        AddVehicleWheelsAnimation(View); // 添加车辆车轮动画
        if (bAdditionalData) // 如果需要额外的数据
        {
          AddActorKinematics(View); // 添加Actor的运动学数据
        }
        break;

      // 对于行人Actor，保存其位置变换和动画
      case FCarlaActor::ActorType::Walker:
        AddActorPosition(View); // 添加Actor的位置
        AddWalkerAnimation(View); // 添加行人动画
        if (bAdditionalData) // 如果需要额外的数据
        {
          AddActorKinematics(View); // 添加Actor的运动学数据
        }
        break;

      // 对于交通灯Actor，保存其状态
      case FCarlaActor::ActorType::TrafficLight:
        AddTrafficLightState(View); // 添加交通灯状态
        break;
    }
  }
  GetFrameCounter(); // 获取当前帧计数器的值
}

// FFrameData::PlayFrameData函数，用于播放存储的帧数据
void FFrameData::PlayFrameData(
    UCarlaEpisode *ThisEpisode,
    std::unordered_map<uint32_t, uint32_t>& MappedId)
{
  // 此函数的具体实现未提供，可能包含将帧数据应用到游戏环节的逻辑
  // ThisEpisode参数可能用于访问游戏环节的状态和功能
  // MappedId参数可能用于处理Actor ID的映射，以确保数据的正确应用
}


  for(const CarlaRecorderEventAdd &EventAdd : EventsAdd.GetEvents())
  {
    uint32_t OldId = EventAdd.DatabaseId;
    // Todo: 检查 EventAdd.DatabaseId 的内存损坏
    auto Result = ProcessReplayerEventAdd(
        EventAdd.Location,
        EventAdd.Rotation,
        EventAdd.Description,
        EventAdd.DatabaseId,
        false,
        true,
        MappedId);
    switch (Result.first)
    {
      //未创建角色
      case 0:
        UE_LOG(LogCarla, Log, TEXT("actor could not be created"));
        break;

      // 已创建 Actor 但具有不同的 ID
      case 1:
        // mapping id （记录的 Id 是 replayer 中的新 Id）
        MappedId[OldId] = Result.second;
        UE_LOG(LogCarla, Log, TEXT("actor created"));
        break;

      // 从现有 actor 重用
      case 2:
        // 映射 ID（假设所需的 Id 映射到什么）
        MappedId[OldId] = Result.second;
        UE_LOG(LogCarla, Log, TEXT("actor reused"));
        break;
    }
  }

  for (const CarlaRecorderEventDel &EventDel : EventsDel.GetEvents())
  {
    ProcessReplayerEventDel(MappedId[EventDel.DatabaseId]);
    MappedId.erase(EventDel.DatabaseId);
  }

  for (const CarlaRecorderPosition &Position : Positions.GetPositions())
  {
    CarlaRecorderPosition Pos = Position;
    auto NewId = MappedId.find(Pos.DatabaseId);
    if (NewId != MappedId.end())
    {
      Pos.DatabaseId = NewId->second;
      ProcessReplayerPosition(Pos, Pos, 0.0, 0.0);
    }
  }

  for (const CarlaRecorderStateTrafficLight &State : States.GetStates())
  {
    CarlaRecorderStateTrafficLight StateTrafficLight = State;
    StateTrafficLight.DatabaseId = MappedId[StateTrafficLight.DatabaseId];
    ProcessReplayerStateTrafficLight(StateTrafficLight);
  }

  for (const CarlaRecorderAnimVehicle &AnimVehicle : Vehicles.GetVehicles())
  {
    CarlaRecorderAnimVehicle Vehicle = AnimVehicle;
    Vehicle.DatabaseId = MappedId[Vehicle.DatabaseId];
    ProcessReplayerAnimVehicle(Vehicle);
  }

  for (const CarlaRecorderAnimWheels &AnimWheel : Wheels.GetVehicleWheels())
  {
    CarlaRecorderAnimWheels Wheels = AnimWheel;
    Wheels.DatabaseId = MappedId[Wheels.DatabaseId];
    ProcessReplayerAnimVehicleWheels(Wheels);
  }

  for (const CarlaRecorderAnimWalker &AnimWalker : Walkers.GetWalkers())
  {
    CarlaRecorderAnimWalker Walker = AnimWalker;
    Walker.DatabaseId = MappedId[Walker.DatabaseId];
    ProcessReplayerAnimWalker(Walker);
  }

  for (const CarlaRecorderAnimBiker &AnimBiker : Bikers.GetBikers())
  {
    CarlaRecorderAnimBiker Biker = AnimBiker;
    Biker.DatabaseId = MappedId[Biker.DatabaseId];
    ProcessReplayerAnimBiker(Biker);
  }

  for (const CarlaRecorderLightVehicle &LightVehicle : LightVehicles.GetLightVehicles())
  {
    CarlaRecorderLightVehicle Light = LightVehicle;
    Light.DatabaseId = MappedId[Light.DatabaseId];
    ProcessReplayerLightVehicle(Light);
  }

  for (const CarlaRecorderLightScene &Light : LightScenes.GetLights())
  {
    ProcessReplayerLightScene(Light);
  }

  SetFrameCounter();
}

void FFrameData::Clear()
{
  EventsAdd.Clear();
  EventsDel.Clear();
  EventsParent.Clear();
  Collisions.Clear();
  Positions.Clear();
  States.Clear();
  Vehicles.Clear();
  Wheels.Clear();
  Walkers.Clear();
  Bikers.Clear();
  LightVehicles.Clear();
  LightScenes.Clear();
  Kinematics.Clear();
  BoundingBoxes.Clear();
  TriggerVolumes.Clear();
  PhysicsControls.Clear();
  TrafficLightTimes.Clear();
  FrameCounter.FrameCounter = 0;
}

void FFrameData::Write(std::ostream& OutStream)
{
  EventsAdd.Write(OutStream);
  EventsDel.Write(OutStream);
  EventsParent.Write(OutStream);
  Positions.Write(OutStream);
  States.Write(OutStream);
  Vehicles.Write(OutStream);
  Wheels.Write(OutStream);
  Walkers.Write(OutStream);
  Bikers.Write(OutStream);
  LightVehicles.Write(OutStream);
  LightScenes.Write(OutStream);
  TrafficLightTimes.Write(OutStream);
  FrameCounter.Write(OutStream);
}

void FFrameData::Read(std::istream& InStream)
{
  Clear();
  while(!InStream.eof())
  {
    Header header;
    ReadValue<char>(InStream, header.Id);
    ReadValue<uint32_t>(InStream, header.Size);
    switch (header.Id)
    {
      // 事件添加
      case static_cast<char>(CarlaRecorderPacketId::EventAdd):
        EventsAdd.Read(InStream);
        break;

      // 事件删除
      case static_cast<char>(CarlaRecorderPacketId::EventDel):
        EventsDel.Read(InStream);
        break;

      // 事件父级
      case static_cast<char>(CarlaRecorderPacketId::EventParent):
        EventsParent.Read(InStream);
        break;

      // 位置
      case static_cast<char>(CarlaRecorderPacketId::Position):
        Positions.Read(InStream);
        break;

      // states
      case static_cast<char>(CarlaRecorderPacketId::State):
        States.Read(InStream);
        break;

      // StatesVehicle 动画
      case static_cast<char>(CarlaRecorderPacketId::AnimVehicle):
        Vehicles.Read(InStream);
        break;

      // walker动画
      case static_cast<char>(CarlaRecorderPacketId::AnimWalker):
        Walkers.Read(InStream);
        break;

      // walker animation
      case static_cast<char>(CarlaRecorderPacketId::AnimVehicleWheels):
        Wheels.Read(InStream);
        break;

      // walker 动画
      case static_cast<char>(CarlaRecorderPacketId::AnimBiker):
        Bikers.Read(InStream);
        break;

      //Vehicle Light 动画
      case static_cast<char>(CarlaRecorderPacketId::VehicleLight):
        LightVehicles.Read(InStream);
        break;

      // 场景灯光动画
      case static_cast<char>(CarlaRecorderPacketId::SceneLight):
        LightScenes.Read(InStream);
        break;

      case static_cast<char>(CarlaRecorderPacketId::FrameCounter):
        FrameCounter.Read(InStream);
        break;

      // 未知数据包，只需跳过
      default:
        // 跳过数据包
        InStream.seekg(header.Size, std::ios::cur);
        break;

    }
  }
}

void FFrameData::CreateRecorderEventAdd(
    uint32_t DatabaseId,
    uint8_t Type,
    const FTransform &Transform,
    FActorDescription ActorDescription,
    bool bAddOtherRelatedInfo)
{
  CarlaRecorderActorDescription Description;
  Description.UId = ActorDescription.UId;
  Description.Id = ActorDescription.Id;

  //属性
  Description.Attributes.reserve(ActorDescription.Variations.Num());
  for (const auto &item : ActorDescription.Variations)
  {
    CarlaRecorderActorAttribute Attr;
    Attr.Type = static_cast<uint8_t>(item.Value.Type);
    Attr.Id = item.Value.Id;
    Attr.Value = item.Value.Value;
    //检查空属性
    if (!Attr.Id.IsEmpty())
    {
      Description.Attributes.emplace_back(std::move(Attr));
    }
  }

  // 记录器事件
  CarlaRecorderEventAdd RecEvent
  {
    DatabaseId,
    Type,
    Transform.GetTranslation(),
    Transform.GetRotation().Euler(),
    std::move(Description)
  };
  AddEvent(std::move(RecEvent));

  if (!bAddOtherRelatedInfo)
  {
    return;
  }

  // 与生成 Actor 相关的其他事件
  FCarlaActor* CarlaActor = Episode->FindCarlaActor(DatabaseId);
  if (!CarlaActor)
  {
    return;
  }
  
  //检查它是否是获得初始物理控制的车辆
  ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(CarlaActor->GetActor());
  if (Vehicle)
  {
    AddPhysicsControl(*Vehicle);
  }

  ATrafficLightBase* TrafficLight = Cast<ATrafficLightBase>(CarlaActor->GetActor());
  if (TrafficLight)
  {
    AddTrafficLightTime(*TrafficLight);
  }

  ATrafficSignBase* TrafficSign = Cast<ATrafficSignBase>(CarlaActor->GetActor());
  if (TrafficSign)
  {
    // 在全局坐标中触发体积
    AddTriggerVolume(*TrafficSign);
  }
  else
  {
    // 本地坐标中的边界框
    AddActorBoundingBox(CarlaActor);
  }
}


void FFrameData::AddActorPosition(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  FTransform Transform = CarlaActor->GetActorGlobalTransform();
  // 获取车辆的位置
  AddPosition(CarlaRecorderPosition
  {
    CarlaActor->GetActorId(),
    Transform.GetLocation(),
    Transform.GetRotation().Euler()
  });
}

void FFrameData::AddVehicleAnimation(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  if (CarlaActor->IsPendingKill())
  {
    return;
  }

  FVehicleControl Control;
  CarlaActor->GetVehicleControl(Control);

  // save
  CarlaRecorderAnimVehicle Record;
  Record.DatabaseId = CarlaActor->GetActorId();
  Record.Steering = Control.Steer;
  Record.Throttle = Control.Throttle;
  Record.Brake = Control.Brake;
  Record.bHandbrake = Control.bHandBrake;
  Record.Gear = Control.Gear;
  AddAnimVehicle(Record);
}

void FFrameData::AddVehicleWheelsAnimation(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr)
  if (CarlaActor->IsPendingKill())
    return;
  if (CarlaActor->GetActorType() != FCarlaActor::ActorType::Vehicle)
    return;

  ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(CarlaActor->GetActor());
  if (CarlaVehicle == nullptr)
    return;

  USkeletalMeshComponent* SkeletalMesh = CarlaVehicle->GetMesh();
  if (SkeletalMesh == nullptr)
    return;

  UVehicleAnimInstance* VehicleAnim = Cast<UVehicleAnimInstance>(SkeletalMesh->GetAnimInstance());
  if (VehicleAnim == nullptr)
    return;

  const UWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = VehicleAnim->GetWheeledVehicleMovementComponent();
  if (WheeledVehicleMovementComponent == nullptr)
    return;

  CarlaRecorderAnimWheels Record;
  Record.DatabaseId = CarlaActor->GetActorId();
  Record.WheelValues.reserve(WheeledVehicleMovementComponent->Wheels.Num());

  uint8 i = 0;
  for (auto Wheel : WheeledVehicleMovementComponent->Wheels)
  {
    WheelInfo Info;
    Info.Location = static_cast<EVehicleWheelLocation>(i);
    Info.SteeringAngle = CarlaVehicle->GetWheelSteerAngle(Info.Location);
    Info.TireRotation = Wheel->GetRotationAngle();
    Record.WheelValues.push_back(Info);
    ++i;
  }

  AddAnimVehicleWheels(Record);

  if (CarlaVehicle->IsTwoWheeledVehicle())
  {
    AddAnimBiker(CarlaRecorderAnimBiker
    {
      CarlaActor->GetActorId(),
      WheeledVehicleMovementComponent->GetForwardSpeed(),
      WheeledVehicleMovementComponent->GetEngineRotationSpeed() / WheeledVehicleMovementComponent->GetEngineMaxRotationSpeed()
    });
  }
}

void FFrameData::AddWalkerAnimation(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  if (!CarlaActor->IsPendingKill())
  {
    FWalkerControl Control;
    CarlaActor->GetWalkerControl(Control);
    AddAnimWalker(CarlaRecorderAnimWalker
    {
      CarlaActor->GetActorId(),
      Control.Speed
    });
  }
}

void FFrameData::AddTrafficLightState(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  ETrafficLightState LightState = CarlaActor->GetTrafficLightState();
  UTrafficLightController* Controller = CarlaActor->GetTrafficLightController();
  if (Controller)
  {
    ATrafficLightGroup* Group = Controller->GetGroup();
    if (Group)
    {
      AddState(CarlaRecorderStateTrafficLight
      {
        CarlaActor->GetActorId(),
        Group->IsFrozen(),
        Controller->GetElapsedTime(),
        static_cast<char>(LightState)
      });
    }
  }
}

void FFrameData::AddVehicleLight(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  FVehicleLightState LightState;
  CarlaActor->GetVehicleLightState(LightState);
  CarlaRecorderLightVehicle LightVehicle;
  LightVehicle.DatabaseId = CarlaActor->GetActorId();
  LightVehicle.State = carla::rpc::VehicleLightState(LightState).light_state;
  AddLightVehicle(LightVehicle);
}

void FFrameData::AddActorKinematics(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  FVector Velocity, AngularVelocity;
  constexpr float TO_METERS = 1e-2;
  Velocity = TO_METERS* CarlaActor->GetActorVelocity();
  AngularVelocity = CarlaActor->GetActorAngularVelocity();
  CarlaRecorderKinematics Kinematic =
  {
    CarlaActor->GetActorId(),
    Velocity,
    AngularVelocity
   };
   AddKinematics(Kinematic);
}

void FFrameData::AddActorBoundingBox(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  const auto &Box = CarlaActor->GetActorInfo()->BoundingBox;
  CarlaRecorderActorBoundingBox BoundingBox =
  {
    CarlaActor->GetActorId(),
    {Box.Origin, Box.Extent}
  };

  AddBoundingBox(BoundingBox);
}

void FFrameData::AddTriggerVolume(const ATrafficSignBase &TrafficSign)
{
  TArray<UBoxComponent*> Triggers = TrafficSign.GetTriggerVolumes();
  if(!Triggers.Num())
  {
    return;
  }
  UBoxComponent* Trigger = Triggers.Top();
  auto VolumeOrigin = Trigger->GetComponentLocation();
  auto VolumeExtent = Trigger->GetScaledBoxExtent();
  CarlaRecorderActorBoundingBox TriggerVolume =
  {
    Episode->GetActorRegistry().FindCarlaActor(&TrafficSign)->GetActorId(),
    {VolumeOrigin, VolumeExtent}
  };
  TriggerVolumes.Add(TriggerVolume);
}

void FFrameData::AddPhysicsControl(const ACarlaWheeledVehicle& Vehicle)
{
  CarlaRecorderPhysicsControl Control;
  Control.DatabaseId = Episode->GetActorRegistry().FindCarlaActor(&Vehicle)->GetActorId();
  Control.VehiclePhysicsControl = Vehicle.GetVehiclePhysicsControl();
  PhysicsControls.Add(Control);
}

void FFrameData::AddTrafficLightTime(const ATrafficLightBase& TrafficLight)
{
  auto DatabaseId = Episode->GetActorRegistry().FindCarlaActor(&TrafficLight)->GetActorId();
  CarlaRecorderTrafficLightTime TrafficLightTime{
    DatabaseId,
    TrafficLight.GetGreenTime(),
    TrafficLight.GetYellowTime(),
    TrafficLight.GetRedTime()
  };
  TrafficLightTimes.Add(TrafficLightTime);
}

void FFrameData::AddPosition(const CarlaRecorderPosition &Position)
{
  Positions.Add(Position);
}

void FFrameData::AddEvent(const CarlaRecorderEventAdd &Event)
{
  EventsAdd.Add(std::move(Event));
}

void FFrameData::AddEvent(const CarlaRecorderEventDel &Event)
{
  EventsDel.Add(std::move(Event));
}

void FFrameData::AddEvent(const CarlaRecorderEventParent &Event)
{
  EventsParent.Add(std::move(Event));
}

void FFrameData::AddCollision(AActor *Actor1, AActor *Actor2)
{
  CarlaRecorderCollision Collision;

  // // some inits
  // Collision.Id = NextCollisionId++;
  // Collision.IsActor1Hero = false;
  // Collision.IsActor2Hero = false;

  // // check actor 1
  // FCarlaActor *FoundActor1 = Episode->GetActorRegistry().FindCarlaActor(Actor1);
  // if (FoundActor1 != nullptr) {
  //   if (FoundActor1->GetActorInfo() != nullptr)
  //   {
  //     auto Role = FoundActor1->GetActorInfo()->Description.Variations.Find("role_name");
  //     if (Role != nullptr)
  //       Collision.IsActor1Hero = (Role->Value == "hero");
  //   }
  //   Collision.DatabaseId1 = FoundActor1->GetActorId();
  // }
  // else {
  //   Collision.DatabaseId1 = uint32_t(-1); // actor1 is not a registered Carla actor
  // }

  // // check actor 2
  // FCarlaActor *FoundActor2 = Episode->GetActorRegistry().FindCarlaActor(Actor2);
  // if (FoundActor2 != nullptr) {
  //   if (FoundActor2->GetActorInfo() != nullptr)
  //   {
  //     auto Role = FoundActor2->GetActorInfo()->Description.Variations.Find("role_name");
  //     if (Role != nullptr)
  //       Collision.IsActor2Hero = (Role->Value == "hero");
  //   }
  //   Collision.DatabaseId2 = FoundActor2->GetActorId();
  // }
  // else {
  //   Collision.DatabaseId2 = uint32_t(-1); // actor2 is not a registered Carla actor
  // }

  Collisions.Add(std::move(Collision));
}

void FFrameData::AddState(const CarlaRecorderStateTrafficLight &State)
{
  States.Add(State);
}

void FFrameData::AddAnimVehicle(const CarlaRecorderAnimVehicle &Vehicle)
{
  Vehicles.Add(Vehicle);
}

void FFrameData::AddAnimVehicleWheels(const CarlaRecorderAnimWheels &VehicleWheels)
{
  Wheels.Add(VehicleWheels);
}

void FFrameData::AddAnimBiker(const CarlaRecorderAnimBiker &Biker)
{
  Bikers.Add(Biker);
}

void FFrameData::AddAnimWalker(const CarlaRecorderAnimWalker &Walker)
{
  Walkers.Add(Walker);
}

void FFrameData::AddLightVehicle(const CarlaRecorderLightVehicle &LightVehicle)
{
  LightVehicles.Add(LightVehicle);
}

void FFrameData::AddEventLightSceneChanged(const UCarlaLight* Light)
{
  CarlaRecorderLightScene LightScene =
  {
    Light->GetId(),
    Light->GetLightIntensity(),
    Light->GetLightColor(),
    Light->GetLightOn(),
    static_cast<uint8>(Light->GetLightType())
  };

  LightScenes.Add(LightScene);
}

void FFrameData::AddKinematics(const CarlaRecorderKinematics &ActorKinematics)
{
  Kinematics.Add(ActorKinematics);
}

void FFrameData::AddBoundingBox(const CarlaRecorderActorBoundingBox &ActorBoundingBox)
{
  BoundingBoxes.Add(ActorBoundingBox);
}

void FFrameData::GetFrameCounter()
{
  FrameCounter.FrameCounter = FCarlaEngine::GetFrameCounter();
}

// create or reuse an actor for replaying
std::pair<int, FCarlaActor*> FFrameData::CreateOrReuseActor(
    FVector &Location,
    FVector &Rotation,
    FActorDescription &ActorDesc,
    uint32_t DesiredId,
    bool SpawnSensors,
    std::unordered_map<uint32_t, uint32_t>& MappedId)
{
  check(Episode != nullptr);

  // 检查我们需要的 actor 类型
  if (ActorDesc.Id.StartsWith("traffic."))
  {
    FCarlaActor* CarlaActor = FindTrafficLightAt(Location);
    if (CarlaActor != nullptr)
    {
      // 重用该 Actor
      UE_LOG(LogCarla, Log, TEXT("TrafficLight found"));
      return std::pair<int, FCarlaActor*>(2, CarlaActor);
    }
    else
    {
      // 未找到执行组件
      UE_LOG(LogCarla, Log, TEXT("TrafficLight not found"));
      return std::pair<int, FCarlaActor*>(0, nullptr);
    }
  }
  else if (SpawnSensors || !ActorDesc.Id.StartsWith("sensor."))
  {
    // 检查是否已经存在具有相同 ID 的该类型的 Actor
    if (Episode->GetActorRegistry().Contains(DesiredId))
    {
      auto* CarlaActor = Episode->FindCarlaActor(DesiredId);
      const FActorDescription *desc = &CarlaActor->GetActorInfo()->Description;
      if (desc->Id == ActorDesc.Id)
      {
        // 我们不需要创建，相同类型的 Actor 已经存在
        //搬迁
        FRotator Rot = FRotator::MakeFromEuler(Rotation);
        FTransform Trans2(Rot, Location, FVector(1, 1, 1));
        CarlaActor->SetActorGlobalTransform(Trans2);
        return std::pair<int, FCarlaActor*>(2, CarlaActor);
      }
    }
    else if (MappedId.find(DesiredId) != MappedId.end() && Episode->GetActorRegistry().Contains(MappedId[DesiredId]))
    {
      auto* CarlaActor = Episode->FindCarlaActor(MappedId[DesiredId]);
      const FActorDescription *desc = &CarlaActor->GetActorInfo()->Description;
      if (desc->Id == ActorDesc.Id)
      {
        // 我们不需要创建，相同类型的 Actor 已经存在
        //搬迁
        FRotator Rot = FRotator::MakeFromEuler(Rotation);
        FTransform Trans2(Rot, Location, FVector(1, 1, 1));
        CarlaActor->SetActorGlobalTransform(Trans2);
        return std::pair<int, FCarlaActor*>(2, CarlaActor);
      }
    }
    // 创建新角色
    // 创建转换
    FRotator Rot = FRotator::MakeFromEuler(Rotation);
    FTransform Trans(Rot, FVector(0, 0, 100000), FVector(1, 1, 1));
    // 创建新角色
    TPair<EActorSpawnResultStatus, FCarlaActor*> Result = Episode->SpawnActorWithInfo(Trans, ActorDesc, DesiredId);
    if (Result.Key == EActorSpawnResultStatus::Success)
    {
      // 搬迁
      FTransform Trans2(Rot, Location, FVector(1, 1, 1));
      Result.Value->SetActorGlobalTransform(Trans2);
      ALargeMapManager * LargeMapManager = UCarlaStatics::GetLargeMapManager(Episode->GetWorld());
      if (LargeMapManager)
      {
        LargeMapManager->OnActorSpawned(*Result.Value);
      }
      return std::pair<int, FCarlaActor*>(1, Result.Value);
    }
    else
    {
      UE_LOG(LogCarla, Log, TEXT("Actor could't be created"));
      return std::pair<int, FCarlaActor*>(0, Result.Value);
    }
  }
  else
  {
    // Actor 已忽略
    return std::pair<int, FCarlaActor*>(0, nullptr);
  }
}

// 用于创建 Actor 的 Replay 事件
std::pair<int, uint32_t> FFrameData::ProcessReplayerEventAdd(
    FVector Location,
    FVector Rotation,
    CarlaRecorderActorDescription Description,
    uint32_t DesiredId,
    bool bIgnoreHero,
    bool ReplaySensors,
    std::unordered_map<uint32_t, uint32_t>& MappedId)
{
  check(Episode != nullptr);
  FActorDescription ActorDesc;
  bool IsHero = false;

  //准备角色描述
  ActorDesc.UId = Description.UId;
  ActorDesc.Id = Description.Id;
  for (const auto &Item : Description.Attributes)
  {
    FActorAttribute Attr;
    Attr.Type = static_cast<EActorAttributeType>(Item.Type);
    Attr.Id = Item.Id;
    Attr.Value = Item.Value;
    ActorDesc.Variations.Add(Attr.Id, std::move(Attr));
    // check for hero
    if (Item.Id == "role_name" && Item.Value == "hero")
      IsHero = true;
  }

  auto result = CreateOrReuseActor(
      Location,
      Rotation,
      ActorDesc,
      DesiredId,
      ReplaySensors,
      MappedId);

  if (result.first != 0)
  {
    //在车辆上禁用 Physics 和 Autopilot
    if (result.second->GetActorType() == FCarlaActor::ActorType::Vehicle)
    {
      // ignore hero ?
      if (!(bIgnoreHero && IsHero))
      {
        // disable physics
        SetActorSimulatePhysics(result.second, false);
        // disable autopilot
        // SetActorAutopilot(result.second, false, false);
      }
      else
      {
        // 重新启用物理学以防万一
        SetActorSimulatePhysics(result.second, true);
      }
    }
    return std::make_pair(result.first, result.second->GetActorId());
  }
  return std::make_pair(result.first, 0);
}

//用于删除 Actor 的 replay 事件
bool FFrameData::ProcessReplayerEventDel(uint32_t DatabaseId)
{
  check(Episode != nullptr);
  FCarlaActor* CarlaActor = Episode->FindCarlaActor(DatabaseId);
  if (CarlaActor == nullptr)
  {
    UE_LOG(LogCarla, Log, TEXT("Actor %d not found to destroy"), DatabaseId);
    return false;
  }
  Episode->DestroyActor(CarlaActor->GetActorId());
  return true;
}

// Replay 事件
bool FFrameData::ProcessReplayerEventParent(uint32_t ChildId, uint32_t ParentId)
{
  check(Episode != nullptr);
  FCarlaActor * Child = Episode->FindCarlaActor(ChildId);
  FCarlaActor * Parent = Episode->FindCarlaActor(ParentId);
  if(!Child)
  {
    UE_LOG(LogCarla, Log, TEXT("Parenting Child actors not found"));
    return false;
  }
  if(!Parent)
  {
    UE_LOG(LogCarla, Log, TEXT("Parenting Parent actors not found"));
    return false;
  }
  Child->SetParent(ParentId);
  Child->SetAttachmentType(carla::rpc::AttachmentType::Rigid);
  Parent->AddChildren(Child->GetActorId());
  if(!Parent->IsDormant())
  {
    if(!Child->IsDormant())
    {
      Episode->AttachActors(
          Child->GetActor(),
          Parent->GetActor(),
          static_cast<EAttachmentType>(carla::rpc::AttachmentType::Rigid));
    }
  }
  else
  {
    Episode->PutActorToSleep(Child->GetActorId());
  }
  return true;
}

// reposition actors
bool FFrameData::ProcessReplayerPosition(CarlaRecorderPosition Pos1, CarlaRecorderPosition Pos2, double Per, double DeltaTime)
{
  check(Episode != nullptr);
  FCarlaActor* CarlaActor = Episode->FindCarlaActor(Pos1.DatabaseId);
  FVector Location;
  FRotator Rotation;
  if(CarlaActor)
  {
    // check to assign first position or interpolate between both
    if (Per == 0.0)
    {
      // assign position 1
      Location = FVector(Pos1.Location);
      Rotation = FRotator::MakeFromEuler(Pos1.Rotation);
    }
    else
    {
      // interpolate positions
      Location = FMath::Lerp(FVector(Pos1.Location), FVector(Pos2.Location), Per);
      Rotation = FMath::Lerp(FRotator::MakeFromEuler(Pos1.Rotation), FRotator::MakeFromEuler(Pos2.Rotation), Per);
    }
    // set new transform
    FTransform Trans(Rotation, Location, FVector(1, 1, 1));
    CarlaActor->SetActorGlobalTransform(Trans, ETeleportType::None);
    return true;
  }
  return false;
}

// reposition the camera
bool FFrameData::SetCameraPosition(uint32_t Id, FVector Offset, FQuat Rotation)
{
  check(Episode != nullptr);

  // get the actor to follow
  FCarlaActor* CarlaActor = Episode->FindCarlaActor(Id);
  if (!CarlaActor)
    return false;
  // get specator pawn
  APawn *Spectator = Episode->GetSpectatorPawn();
  if (!Spectator)
   return false;

  FCarlaActor* CarlaSpectator = Episode->FindCarlaActor(Spectator);
  if (!CarlaSpectator)
    return false;

  FTransform ActorTransform = CarlaActor->GetActorGlobalTransform();
  // set the new position
  FQuat ActorRot = ActorTransform.GetRotation();
  FVector Pos = ActorTransform.GetTranslation() + (ActorRot.RotateVector(Offset));
  CarlaSpectator->SetActorGlobalTransform(FTransform(ActorRot * Rotation, Pos, FVector(1,1,1)));

  return true;
}

bool FFrameData::ProcessReplayerStateTrafficLight(CarlaRecorderStateTrafficLight State)
{
  check(Episode != nullptr);
  FCarlaActor* CarlaActor = Episode->FindCarlaActor(State.DatabaseId);
  if(CarlaActor)
  {
    CarlaActor->SetTrafficLightState(static_cast<ETrafficLightState>(State.State));
    UTrafficLightController* Controller = CarlaActor->GetTrafficLightController();
    if(Controller)
    {
      Controller->SetElapsedTime(State.ElapsedTime);
      ATrafficLightGroup* Group = Controller->GetGroup();
      if (Group)
      {
        Group->SetFrozenGroup(State.IsFrozen);
      }
    }
     return true;
  }
  return false;
}

// set the animation for Vehicles
void FFrameData::ProcessReplayerAnimVehicle(CarlaRecorderAnimVehicle Vehicle)
{
  check(Episode != nullptr);
  FCarlaActor *CarlaActor = Episode->FindCarlaActor(Vehicle.DatabaseId);
  if (CarlaActor)
  {
    FVehicleControl Control;
    Control.Throttle = Vehicle.Throttle;
    Control.Steer = Vehicle.Steering;
    Control.Brake = Vehicle.Brake;
    Control.bHandBrake = Vehicle.bHandbrake;
    Control.bReverse = (Vehicle.Gear < 0);
    Control.Gear = Vehicle.Gear;
    Control.bManualGearShift = false;
    CarlaActor->ApplyControlToVehicle(Control, EVehicleInputPriority::User);
  }
}

void FFrameData::ProcessReplayerAnimVehicleWheels(CarlaRecorderAnimWheels VehicleAnimWheels)
{
  check(Episode != nullptr)
  FCarlaActor *CarlaActor = Episode->FindCarlaActor(VehicleAnimWheels.DatabaseId);
  if (CarlaActor == nullptr)
    return;
  if (CarlaActor->GetActorType() != FCarlaActor::ActorType::Vehicle)
    return;
  ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(CarlaActor->GetActor());
  check(CarlaVehicle != nullptr)
  USkeletalMeshComponent* SkeletalMesh = CarlaVehicle->GetMesh();
  check(SkeletalMesh != nullptr)
  UVehicleAnimInstance* VehicleAnim = Cast<UVehicleAnimInstance>(SkeletalMesh->GetAnimInstance());
  check(VehicleAnim != nullptr)

  for (uint32_t i = 0; i < VehicleAnimWheels.WheelValues.size(); ++i)
  {
    const WheelInfo& Element = VehicleAnimWheels.WheelValues[i];
    VehicleAnim->SetWheelRotYaw(static_cast<uint8>(Element.Location), Element.SteeringAngle);
    VehicleAnim->SetWheelPitchAngle(static_cast<uint8>(Element.Location), Element.TireRotation);
  }
}

// 为车辆设置灯光
void FFrameData::ProcessReplayerLightVehicle(CarlaRecorderLightVehicle LightVehicle)
{
  check(Episode != nullptr);
  FCarlaActor * CarlaActor = Episode->FindCarlaActor(LightVehicle.DatabaseId);
  if (CarlaActor)
  {
    carla::rpc::VehicleLightState LightState(LightVehicle.State);
    CarlaActor->SetVehicleLightState(FVehicleLightState(LightState));
  }
}

void FFrameData::ProcessReplayerLightScene(CarlaRecorderLightScene LightScene)
{
  check(Episode != nullptr);
  UWorld* World = Episode->GetWorld();
  if(World)
  {
    UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
    if (!CarlaLightSubsystem)
    {
      return;
    }
    auto* CarlaLight = CarlaLightSubsystem->GetLight(LightScene.LightId);
    if (CarlaLight)
    {
      CarlaLight->SetLightIntensity(LightScene.Intensity);
      CarlaLight->SetLightColor(LightScene.Color);
      CarlaLight->SetLightOn(LightScene.bOn);
      CarlaLight->SetLightType(static_cast<ELightType>(LightScene.Type));
    }
  }
}

// 设置行走者的动画
void FFrameData::ProcessReplayerAnimWalker(CarlaRecorderAnimWalker Walker)
{
  SetWalkerSpeed(Walker.DatabaseId, Walker.Speed);
}

void FFrameData::ProcessReplayerAnimBiker(CarlaRecorderAnimBiker Biker)
{
  check(Episode != nullptr);
  FCarlaActor * CarlaActor = Episode->FindCarlaActor(Biker.DatabaseId);
  if (CarlaActor == nullptr)
    return;
  ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(CarlaActor->GetActor());
  check(CarlaVehicle != nullptr)
  CarlaVehicle->SetSpeedAnim(Biker.ForwardSpeed);
  CarlaVehicle->SetRotationAnim(Biker.EngineRotation);
}


//重播结束
bool FFrameData::ProcessReplayerFinish(bool bApplyAutopilot, bool bIgnoreHero, std::unordered_map<uint32_t, bool> &IsHero)
{
  // 为所有 AI 车辆设置 Autopilot 和 Physics
  const FActorRegistry& Registry = Episode->GetActorRegistry();
  for (auto& It : Registry)
  {
    FCarlaActor* CarlaActor = It.Value.Get();

    //仅在载具上启用物理
    switch (CarlaActor->GetActorType())
    {

      // vehicles
      case FCarlaActor::ActorType::Vehicle:
        // check for hero
        if (!(bIgnoreHero && IsHero[CarlaActor->GetActorId()]))
        {
            // stop all vehicles
            SetActorSimulatePhysics(CarlaActor, true);
            SetActorVelocity(CarlaActor, FVector(0, 0, 0));
            FVehicleControl Control;
            Control.Throttle = 0.0f;
            Control.Steer = 0.0f;
            Control.Brake = 0.0f;
            Control.bHandBrake = false;
            Control.bReverse = false;
            Control.Gear = 1;
            Control.bManualGearShift = false;
            CarlaActor->ApplyControlToVehicle(Control, EVehicleInputPriority::User);
        }
        break;

      // walkers
      case FCarlaActor::ActorType::Walker:
        // stop walker
        SetWalkerSpeed(CarlaActor->GetActorId(), 0.0f);
        break;
    }
  }
  return true;
}

void FFrameData::SetActorVelocity(FCarlaActor *CarlaActor, FVector Velocity)
{
  if (!CarlaActor)
  {
    return;
  }
  CarlaActor->SetActorTargetVelocity(Velocity);
}

//设置行走者的动画速度
void FFrameData::SetWalkerSpeed(uint32_t ActorId, float Speed)
{
  check(Episode != nullptr);
  FCarlaActor * CarlaActor = Episode->FindCarlaActor(ActorId);
  if (!CarlaActor)
  {
    return;
  }
  FWalkerControl Control;
  Control.Speed = Speed;
  CarlaActor->ApplyControlToWalker(Control);
}

// 启用/禁用 Actor 的物理特性）
bool FFrameData::SetActorSimulatePhysics(FCarlaActor* CarlaActor, bool bEnabled)
{
  if (!CarlaActor)
  {
    return false;
  }
  ECarlaServerResponse Response =
      CarlaActor->SetActorSimulatePhysics(bEnabled);
  if (Response != ECarlaServerResponse::Success)
  {
    return false;
  }
  return true;
}

void FFrameData::SetFrameCounter()
{
  FCarlaEngine::ResetFrameCounter(FrameCounter.FrameCounter);
}

FCarlaActor *FFrameData::FindTrafficLightAt(FVector Location)
{
  check(Episode != nullptr);
  auto World = Episode->GetWorld();
  check(World != nullptr);

  // 获取其位置 （截断为 int）
  int x = static_cast<int>(Location.X);
  int y = static_cast<int>(Location.Y);
  int z = static_cast<int>(Location.Z);

  const FActorRegistry &Registry = Episode->GetActorRegistry();
  // 通过 Registry 中的所有参与者
  for (auto It = Registry.begin(); It != Registry.end(); ++It)
  {
    FCarlaActor* CarlaActor = It.Value().Get();
    if(CarlaActor->GetActorType() == FCarlaActor::ActorType::TrafficLight)
    {
      FVector vec = CarlaActor->GetActorGlobalLocation();
      int x2 = static_cast<int>(vec.X);
      int y2 = static_cast<int>(vec.Y);
      int z2 = static_cast<int>(vec.Z);
      if ((x2 == x) && (y2 == y) && (z2 == z))
      {
        // 找到actor
        return CarlaActor;
      }
    }
  }
  // 丢失actor
  return nullptr;
}

void FFrameData::AddExistingActors(void)
{
  // reginstring 第一帧中的所有现有 Actor
  FActorRegistry Registry = Episode->GetActorRegistry();
  for (auto& It : Registry)
  {
    const FCarlaActor* CarlaActor = It.Value.Get();
    if (CarlaActor != nullptr)
    {
      // 创建事件
      CreateRecorderEventAdd(
          CarlaActor->GetActorId(),
          static_cast<uint8_t>(CarlaActor->GetActorType()),
          CarlaActor->GetActorGlobalTransform(),
          CarlaActor->GetActorInfo()->Description,
          false);
    }
  }
}
