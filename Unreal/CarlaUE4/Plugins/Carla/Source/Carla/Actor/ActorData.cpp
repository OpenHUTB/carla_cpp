// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 包含ActorData类的头文件
#include "ActorData.h"
 
// 包含Carla库中关于游戏集、交通灯、交通标志、交通灯组件、交通灯控制器等相关的头文件
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Traffic/TrafficLightBase.h"
#include "Carla/Traffic/SignComponent.h"
#include "Carla/Traffic/TrafficLightComponent.h"
#include "Carla/Traffic/TrafficLightController.h"
 
// 包含Carla库中关于游戏模式基础类和静态类的头文件
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaStatics.h"
 
// 包含Carla库中关于车辆的头文件
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
 
// 包含游戏框架中角色移动组件的头文件
#include "GameFramework/CharacterMovementComponent.h"
 
// 包含Carla库中关于行人控制器和行人基础类的头文件
#include "Carla/Walker/WalkerController.h"
#include "Carla/Walker/WalkerBase.h"
 
// 包含Carla库中关于传感器的头文件
#include "Carla/Sensor/Sensor.h"
 
// 包含CarlaActor类的头文件
#include "CarlaActor.h"
 
// FActorData类的成员函数，用于重生一个Actor
AActor* FActorData::RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info)
{
  // 获取局部变换，并向上移动15个单位以避免车轮与几何体相交
  FTransform SpawnTransform = GetLocalTransform(CarlaEpisode);
  SpawnTransform.AddToTranslation(FVector(0,0,15));
  // 使用给定的信息和变换重生Actor
  return CarlaEpisode->ReSpawnActorWithInfo(SpawnTransform, Info.Description);
}
 
// FActorData类的成员函数，用于记录Actor的数据
void FActorData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  // 获取Actor及其变换
  AActor* Actor = CarlaActor->GetActor();
  FTransform Transform = Actor->GetTransform();
  // 计算位置（考虑到当前地图的原点）
  Location = FDVector(Transform.GetLocation()) + CarlaEpisode->GetCurrentMapOrigin();
  Rotation = Transform.GetRotation();
  Scale = Transform.GetScale3D();
  // 获取Actor的根组件，并检查是否为UPrimitiveComponent类型
  UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
  if (Component)
  {
    // 记录物理模拟状态和角速度
    bSimulatePhysics = Component->IsSimulatingPhysics();
    AngularVelocity = Component->GetPhysicsAngularVelocityInDegrees();
  }
  // 记录速度
  Velocity = Actor->GetVelocity();
}
 
// FActorData类的成员函数，用于恢复Actor的数据
void FActorData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  // 获取Actor并设置其变换
  AActor* Actor = CarlaActor->GetActor();
  Actor->SetActorTransform(GetLocalTransform(CarlaEpisode));
  // 获取Actor的根组件，并检查是否为UPrimitiveComponent类型
  UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
  if (Component)
  {
    // 恢复线性和角速度
    Component->SetPhysicsLinearVelocity(
        Velocity, false, "None");
    Component->SetPhysicsAngularVelocityInDegrees(
        AngularVelocity, false, "None");
  }
  // 尝试将Actor转换为ACharacter或ACarlaWheeledVehicle类型，以特别处理车辆和行人的物理
  auto* Character = Cast<ACharacter>(Actor);
  auto* CarlaVehicle = Cast<ACarlaWheeledVehicle>(Actor);
  if (CarlaVehicle != nullptr){
    // 车辆物理处理方式不同，因此设置物理模拟状态
    CarlaVehicle->SetSimulatePhysics(bSimulatePhysics);
  }
  else if (Character != nullptr)
  {
    // 行人物理处理方式也不同，但此处未实现具体逻辑
  }
  // In the rest of actors, the physics is controlled with the UPrimitiveComponent, so we use
  // that for disable it.
  else
  {
    // 对于其他Actor，使用UPrimitiveComponent控制物理
    auto RootComponent = Component;
    if (RootComponent != nullptr)
    {
      RootComponent->SetSimulatePhysics(bSimulatePhysics);
      RootComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
  }
}
 
// FActorData类的成员函数，用于获取相对于当前地图原点的局部变换
FTransform FActorData::GetLocalTransform(UCarlaEpisode* CarlaEpisode) const
{
  // 计算局部位置（考虑到当前地图的原点）
  FVector LocalLocation = (Location - CarlaEpisode->GetCurrentMapOrigin()).ToFVector();
  // 返回包含位置、旋转和缩放的变换
  return FTransform(Rotation, LocalLocation, Scale);
}


// 记录车辆相关数据
void FVehicleData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);// 继承自基类的记录数据方法
  AActor* Actor = CarlaActor->GetActor(); // 获取Actor对象
  ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(Actor); // 将Actor转换为车辆类型
  if (bSimulatePhysics) // 如果需要模拟物理
  {
    PhysicsControl = Vehicle->GetVehiclePhysicsControl();// 获取车辆物理控制数据
  }
  Control = Vehicle->GetVehicleControl(); // 获取车辆控制数据
  AckermannControl = Vehicle->GetVehicleAckermannControl();// 获取Ackermann控制数据
  bAckermannControlActive = Vehicle->IsAckermannControlActive(); // 检查Ackermann控制是否激活
  AckermannControllerSettings = Vehicle->GetAckermannControllerSettings();// 获取Ackermann控制器设置
  LightState = Vehicle->GetVehicleLightState();// 获取车辆灯光状态
  auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());// 获取车辆AI控制器
  if (Controller)
  {
    SpeedLimit = Controller->GetSpeedLimit();// 获取速度限制
  }
  FailureState = Vehicle->GetFailureState();// 获取车辆故障状态
}

// 恢复车辆相关数据
void FVehicleData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RestoreActorData(CarlaActor, CarlaEpisode);// 继承自基类的恢复数据方法
  AActor* Actor = CarlaActor->GetActor();// 获取Actor对象
  ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(Actor);// 将Actor转换为车辆类型
  Vehicle->SetSimulatePhysics(bSimulatePhysics);// 设置是否模拟物理
  if (bSimulatePhysics)// 如果需要模拟物理
  {
    Vehicle->ApplyVehiclePhysicsControl(PhysicsControl);// 应用车辆物理控制数据
  }
  Vehicle->ApplyAckermannControllerSettings(AckermannControllerSettings);// 应用Ackermann控制器设置
  if (!bAckermannControlActive)// 如果Ackermann控制不激活
  {
    Vehicle->ApplyVehicleControl(Control, EVehicleInputPriority::Client);// 应用车辆控制数据
  }
  else
  {
    Vehicle->ApplyVehicleAckermannControl(AckermannControl, EVehicleInputPriority::Client);// 应用Ackermann控制数据
  }
  Vehicle->SetVehicleLightState(LightState);// 设置车辆灯光状态
  auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());/ 获取车辆AI控制器
  if (Controller)// 如果控制器存在
  {
    Controller->SetSpeedLimit(SpeedLimit);// 设置速度限制
  }
  Vehicle->SetFailureState(FailureState);// 设置车辆故障状态
}

// 记录行人相关数据
void FWalkerData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);// 继承自基类的记录数据方法
  AActor* Actor = CarlaActor->GetActor();// 获取Actor对象
  auto Walker = Cast<AWalkerBase>(Actor);// 将Actor转换为行人类型
  auto Controller = Walker != nullptr ? Cast<AWalkerController>(Walker->GetController()) : nullptr;// 获取行人控制器
  if (Controller != nullptr)
  {
    WalkerControl = carla::rpc::WalkerControl{Controller->GetWalkerControl()};// 获取行人控制数据
  }
  bAlive = Walker->bAlive;// 获取行人是否存活的状态
}

// 恢复行人相关数据
void FWalkerData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RestoreActorData(CarlaActor, CarlaEpisode);// 继承自基类的恢复数据方法
  AActor* Actor = CarlaActor->GetActor();
  auto Walker = Cast<ACharacter>(Actor); // 将Actor转换为角色类型
  auto Controller = Walker != nullptr ? Cast<AWalkerController>(Walker->GetController()) : nullptr;
  if (Controller != nullptr)
  {
    Controller->ApplyWalkerControl(WalkerControl);
  }
  auto CharacterMovement = Cast<UCharacterMovementComponent>(Walker->GetCharacterMovement());// 获取角色运动组件
  // TODO: Handle death timer
}

// 重新生成交通标志Actor
AActor* FTrafficSignData::RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info)// 获取本地变换
{
  FTransform SpawnTransform = GetLocalTransform(CarlaEpisode);// 创建Actor生成参数
  FActorSpawnParameters SpawnParams;
  SpawnParams.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;// 设置碰撞处理方式为总是生成
  return CarlaEpisode->GetWorld()->SpawnActor<ATrafficSignBase>(// 在世界中生成交通标志Actor
        Model,
        SpawnTransform,
        SpawnParams);
}
 // 记录交通标志的actor数据
void FTrafficSignData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{// 调用基类方法记录基本的actor数据
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);
  // 获取CARLA actor对应的Unreal Engine actor
  AActor* Actor = CarlaActor->GetActor();
 // 记录交通标志的模型（类）
  Model = Actor->GetClass();
 // 尝试将actor转换为交通标志基类
  ATrafficSignBase* TrafficSign = Cast<ATrafficSignBase>(Actor);
 // 在交通标志中查找交通标志组件
  USignComponent* TrafficSignComponent =
        Cast<USignComponent>(TrafficSign->FindComponentByClass<USignComponent>());
  if (TrafficSignComponent)
  {
    SignModel = TrafficSignComponent->GetClass();
    SignId = TrafficSignComponent->GetSignId();
  }
}
// 恢复交通标志的actor数据
void FTrafficSignData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  AActor* Actor = CarlaActor->GetActor();
 // 如果SignId不为空，说明有交通标志数据需要恢复
  if (SignId.Len())
  {// 在actor上创建新的交通标志组件，使用之前记录的模型和ID
    USignComponent* SignComponent =
        NewObject<USignComponent>(Actor, SignModel);
    SignComponent->SetSignId(SignId);
    // 注册组件，使其生效
    SignComponent->RegisterComponent();
   // 将组件附加到actor的根组件上，保持相对变换
    SignComponent->AttachToComponent(
        Actor->GetRootComponent(),
        FAttachmentTransformRules::KeepRelativeTransform);
    // 获取游戏模式，用于初始化交通标志
    ACarlaGameModeBase *GameMode = UCarlaStatics::GetGameMode(CarlaEpisode->GetWorld());
   // 初始化交通标志，需要传入地图对象
    SignComponent->InitializeSign(GameMode->GetMap().get());
  }
}

AActor* FTrafficLightData::RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info)
{
  FTransform SpawnTransform = GetLocalTransform(CarlaEpisode);
  FActorSpawnParameters SpawnParams;
  SpawnParams.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  return CarlaEpisode->GetWorld()->SpawnActor<ATrafficLightBase>(
        Model,
        SpawnTransform,
        SpawnParams);
}

void FTrafficLightData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  Model = Actor->GetClass();
  ATrafficLightBase* TrafficLight = Cast<ATrafficLightBase>(Actor);
  UTrafficLightComponent* Component = TrafficLight->GetTrafficLightComponent();
  SignId = Component->GetSignId();
  Controller = Component->GetController();
  Controller->RemoveTrafficLight(Component);
  Controller->AddCarlaActorTrafficLight(CarlaActor);
  LightState = TrafficLight->GetTrafficLightState();
  PoleIndex = TrafficLight->GetPoleIndex();
}

void FTrafficLightData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  AActor* Actor = CarlaActor->GetActor();
  ATrafficLightBase* TrafficLight = Cast<ATrafficLightBase>(Actor);
  UTrafficLightComponent* Component = TrafficLight->GetTrafficLightComponent();
  Component->SetSignId(SignId);
  Controller->RemoveCarlaActorTrafficLight(CarlaActor);
  Controller->AddTrafficLight(Component);
  ACarlaGameModeBase *GameMode = UCarlaStatics::GetGameMode(CarlaEpisode->GetWorld());
  Component->InitializeSign(GameMode->GetMap().get());
  Component->SetLightState(Controller->GetCurrentState().State);
  TrafficLight->SetPoleIndex(PoleIndex);
  TrafficLight->SetTrafficLightState(LightState);
}

void FActorSensorData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  ASensor* Sensor = Cast<ASensor>(Actor);
  Stream = Sensor->MoveDataStream();
}
 // 恢复传感器的actor数据
void FActorSensorData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RestoreActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
 // 获取传感器，并设置其数据流
  ASensor* Sensor = Cast<ASensor>(Actor);
  Sensor->SetDataStream(std::move(Stream));
}
