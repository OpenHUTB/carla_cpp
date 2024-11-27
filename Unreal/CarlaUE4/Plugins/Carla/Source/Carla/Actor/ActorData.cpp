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

void FVehicleData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(Actor);
  if (bSimulatePhysics)
  {
    PhysicsControl = Vehicle->GetVehiclePhysicsControl();
  }
  Control = Vehicle->GetVehicleControl();
  AckermannControl = Vehicle->GetVehicleAckermannControl();
  bAckermannControlActive = Vehicle->IsAckermannControlActive();
  AckermannControllerSettings = Vehicle->GetAckermannControllerSettings();
  LightState = Vehicle->GetVehicleLightState();
  auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
  if (Controller)
  {
    SpeedLimit = Controller->GetSpeedLimit();
  }
  FailureState = Vehicle->GetFailureState();
}

void FVehicleData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RestoreActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(Actor);
  Vehicle->SetSimulatePhysics(bSimulatePhysics);
  if (bSimulatePhysics)
  {
    Vehicle->ApplyVehiclePhysicsControl(PhysicsControl);
  }
  Vehicle->ApplyAckermannControllerSettings(AckermannControllerSettings);
  if (!bAckermannControlActive)
  {
    Vehicle->ApplyVehicleControl(Control, EVehicleInputPriority::Client);
  }
  else
  {
    Vehicle->ApplyVehicleAckermannControl(AckermannControl, EVehicleInputPriority::Client);
  }
  Vehicle->SetVehicleLightState(LightState);
  auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
  if (Controller)
  {
    Controller->SetSpeedLimit(SpeedLimit);
  }
  Vehicle->SetFailureState(FailureState);
}

void FWalkerData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  auto Walker = Cast<AWalkerBase>(Actor);
  auto Controller = Walker != nullptr ? Cast<AWalkerController>(Walker->GetController()) : nullptr;
  if (Controller != nullptr)
  {
    WalkerControl = carla::rpc::WalkerControl{Controller->GetWalkerControl()};
  }
  bAlive = Walker->bAlive;
}

void FWalkerData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RestoreActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  auto Walker = Cast<ACharacter>(Actor);
  auto Controller = Walker != nullptr ? Cast<AWalkerController>(Walker->GetController()) : nullptr;
  if (Controller != nullptr)
  {
    Controller->ApplyWalkerControl(WalkerControl);
  }
  auto CharacterMovement = Cast<UCharacterMovementComponent>(Walker->GetCharacterMovement());
  // TODO: Handle death timer
}

AActor* FTrafficSignData::RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info)
{
  FTransform SpawnTransform = GetLocalTransform(CarlaEpisode);
  FActorSpawnParameters SpawnParams;
  SpawnParams.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  return CarlaEpisode->GetWorld()->SpawnActor<ATrafficSignBase>(
        Model,
        SpawnTransform,
        SpawnParams);
}

void FTrafficSignData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  Model = Actor->GetClass();
  ATrafficSignBase* TrafficSign = Cast<ATrafficSignBase>(Actor);
  USignComponent* TrafficSignComponent =
        Cast<USignComponent>(TrafficSign->FindComponentByClass<USignComponent>());
  if (TrafficSignComponent)
  {
    SignModel = TrafficSignComponent->GetClass();
    SignId = TrafficSignComponent->GetSignId();
  }
}

void FTrafficSignData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  AActor* Actor = CarlaActor->GetActor();
  if (SignId.Len())
  {
    USignComponent* SignComponent =
        NewObject<USignComponent>(Actor, SignModel);
    SignComponent->SetSignId(SignId);
    SignComponent->RegisterComponent();
    SignComponent->AttachToComponent(
        Actor->GetRootComponent(),
        FAttachmentTransformRules::KeepRelativeTransform);
    ACarlaGameModeBase *GameMode = UCarlaStatics::GetGameMode(CarlaEpisode->GetWorld());
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

void FActorSensorData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RestoreActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  ASensor* Sensor = Cast<ASensor>(Actor);
  Sensor->SetDataStream(std::move(Stream));
}
