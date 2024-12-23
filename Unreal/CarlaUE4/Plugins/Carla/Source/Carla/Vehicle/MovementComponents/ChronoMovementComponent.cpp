// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ChronoMovementComponent.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Vehicle/MovementComponents/DefaultMovementComponent.h"

#include "compiler/disable-ue4-macros.h"
#include <carla/rpc/String.h>
#ifdef WITH_CHRONO
#include "chrono_vehicle/utils/ChUtilsJSON.h"
#endif
#include "compiler/enable-ue4-macros.h"
#include "Carla/Util/RayTracer.h"


void UChronoMovementComponent::CreateChronoMovementComponent(
    ACarlaWheeledVehicle* Vehicle,
    uint64_t MaxSubsteps,
    float MaxSubstepDeltaTime,
    FString VehicleJSON,
    FString PowertrainJSON,
    FString TireJSON,
    FString BaseJSONPath)
{
  #ifdef WITH_CHRONO
  UChronoMovementComponent* ChronoMovementComponent = NewObject<UChronoMovementComponent>(Vehicle);
  if (!VehicleJSON.IsEmpty())
  {
    ChronoMovementComponent->VehicleJSON = VehicleJSON;
  }
  if (!PowertrainJSON.IsEmpty())
  {
    ChronoMovementComponent->PowertrainJSON = PowertrainJSON;
  }
  if (!TireJSON.IsEmpty())
  {
    ChronoMovementComponent->TireJSON = TireJSON;
  }
  if (!BaseJSONPath.IsEmpty())
  {
    ChronoMovementComponent->BaseJSONPath = BaseJSONPath;
  }
  ChronoMovementComponent->MaxSubsteps = MaxSubsteps;
  ChronoMovementComponent->MaxSubstepDeltaTime = MaxSubstepDeltaTime;
  Vehicle->SetCarlaMovementComponent(ChronoMovementComponent);
  ChronoMovementComponent->RegisterComponent();
  #else
  UE_LOG(LogCarla, Warning, TEXT("Error: Chrono is not enabled") );
  #endif
}

#ifdef WITH_CHRONO

using namespace chrono;
using namespace chrono::vehicle;

constexpr double CMTOM = 0.01;
ChVector<> UE4LocationToChrono(const FVector& Location)
{
  return CMTOM*ChVector<>(Location.X, -Location.Y, Location.Z);
}
constexpr double MTOCM = 100;
FVector ChronoToUE4Location(const ChVector<>& position)
{
  return MTOCM*FVector(position.x(), -position.y(), position.z());
}
ChVector<> UE4DirectionToChrono(const FVector& Location)
{
  return ChVector<>(Location.X, -Location.Y, Location.Z);
}
FVector ChronoToUE4Direction(const ChVector<>& position)
{
  return FVector(position.x(), -position.y(), position.z());
}
ChQuaternion<> UE4QuatToChrono(const FQuat& Quat)
{
  return ChQuaternion<>(Quat.W, -Quat.X, Quat.Y, -Quat.Z);
}
FQuat ChronoToUE4Quat(const ChQuaternion<>& quat)
{
  return FQuat(-quat.e1(), quat.e2(), -quat.e3(), quat.e0());
}

UERayCastTerrain::UERayCastTerrain(
    ACarlaWheeledVehicle* UEVehicle,
    chrono::vehicle::ChVehicle* ChrVehicle)
    : CarlaVehicle(UEVehicle), ChronoVehicle(ChrVehicle) {}

std::pair<bool, FHitResult>
    UERayCastTerrain::GetTerrainProperties(const FVector &Location) const
{
  const double MaxDistance = 1000000;
  FVector StartLocation = Location;
  FVector EndLocation = Location + FVector(0,0,-1)*MaxDistance; // 向下搜索
  FHitResult Hit;
  FCollisionQueryParams CollisionQueryParams;
  CollisionQueryParams.AddIgnoredActor(CarlaVehicle);
  bool bDidHit = CarlaVehicle->GetWorld()->LineTraceSingleByChannel(
      Hit,
      StartLocation,
      EndLocation,
      ECC_GameTraceChannel2, // 摄像机（任意碰撞）
      CollisionQueryParams,
      FCollisionResponseParams()
  );
  return std::make_pair(bDidHit, Hit);
}

double UERayCastTerrain::GetHeight(const ChVector<>& loc) const
{
  FVector Location = ChronoToUE4Location(loc + ChVector<>(0,0,0.5)); // 一个小偏移量，用于正确检测地面
  auto point_pair = GetTerrainProperties(Location);
  if (point_pair.first)
  {
    double Height = CMTOM*static_cast<double>(point_pair.second.Location.Z);
    return Height;
  }
  return -1000000.0;
}
ChVector<> UERayCastTerrain::GetNormal(const ChVector<>& loc) const
{
  FVector Location = ChronoToUE4Location(loc);
  auto point_pair = GetTerrainProperties(Location);
  if (point_pair.first)
  {
    FVector Normal = point_pair.second.Normal;
    auto ChronoNormal = UE4DirectionToChrono(Normal);
    return ChronoNormal;
  }
  return UE4DirectionToChrono(FVector(0,0,1));
}
float UERayCastTerrain::GetCoefficientFriction(const ChVector<>& loc) const
{
  return 1;
}

void UChronoMovementComponent::BeginPlay()
{
  Super::BeginPlay();

  DisableUE4VehiclePhysics();

  // // // Chrono System
  Sys.Set_G_acc(ChVector<>(0, 0, -9.81));
  Sys.SetSolverType(ChSolver::Type::BARZILAIBORWEIN);
  Sys.SetSolverMaxIterations(150);
  Sys.SetMaxPenetrationRecoverySpeed(4.0);

  InitializeChronoVehicle();

  // 创建地形
  Terrain = chrono_types::make_shared<UERayCastTerrain>(CarlaVehicle, Vehicle.get());

  CarlaVehicle->OnActorHit.AddDynamic(
      this, &UChronoMovementComponent::OnVehicleHit);
  CarlaVehicle->GetMesh()->OnComponentBeginOverlap.AddDynamic(
      this, &UChronoMovementComponent::OnVehicleOverlap);
  CarlaVehicle->GetMesh()->SetCollisionResponseToChannel(
      ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
}

void UChronoMovementComponent::InitializeChronoVehicle()
{
  // 初始位置设有小偏移量，以防穿透地面
  FVector VehicleLocation = CarlaVehicle->GetActorLocation() + FVector(0,0,25);
  FQuat VehicleRotation = CarlaVehicle->GetActorRotation().Quaternion();
  auto ChronoLocation = UE4LocationToChrono(VehicleLocation);
  auto ChronoRotation = UE4QuatToChrono(VehicleRotation);

  // 为车辆JSON文件设置基本路径
  vehicle::SetDataPath(carla::rpc::FromFString(BaseJSONPath));

  std::string BasePath_string = carla::rpc::FromFString(BaseJSONPath);

  // 为json文件创建完整路径
  //不要使用vehicle::GetDataFile()，因为chrono库中的字符串会与unreal的std库发生冲突
  std::string VehicleJSON_string = carla::rpc::FromFString(VehicleJSON);
  std::string VehiclePath_string = BasePath_string + VehicleJSON_string;
  FString VehicleJSONPath = carla::rpc::ToFString(VehiclePath_string);

  std::string PowerTrainJSON_string = carla::rpc::FromFString(PowertrainJSON);
  std::string PowerTrain_string = BasePath_string + PowerTrainJSON_string;
  FString PowerTrainJSONPath = carla::rpc::ToFString(PowerTrain_string);

  std::string TireJSON_string = carla::rpc::FromFString(TireJSON);
  std::string Tire_string = BasePath_string + TireJSON_string;
  FString TireJSONPath = carla::rpc::ToFString(Tire_string);

  UE_LOG(LogCarla, Log, TEXT("Loading Chrono files: Vehicle: %s, PowerTrain: %s, Tire: %s"),
      *VehicleJSONPath,
      *PowerTrainJSONPath,
      *TireJSONPath);
  // 创建JSON车辆文件
  Vehicle = chrono_types::make_shared<WheeledVehicle>(
      &Sys,
      VehiclePath_string);
  Vehicle->Initialize(ChCoordsys<>(ChronoLocation, ChronoRotation));
  Vehicle->GetChassis()->SetFixed(false);
  // 创建并初始化传动系统
  auto powertrain = ReadPowertrainJSON(
      PowerTrain_string);
  Vehicle->InitializePowertrain(powertrain);
  // 创建并初始化轮胎
  for (auto& axle : Vehicle->GetAxles()) {
      for (auto& wheel : axle->GetWheels()) {
          auto tire = ReadTireJSON(Tire_string);
          Vehicle->InitializeTire(tire, wheel, VisualizationType::MESH);
      }
  }
}

void UChronoMovementComponent::ProcessControl(FVehicleControl &Control)
{
  VehicleControl = Control;
  auto PowerTrain = Vehicle->GetPowertrain();
  if (PowerTrain)
  {
    if (VehicleControl.bReverse)
    {
      PowerTrain->SetDriveMode(ChPowertrain::DriveMode::REVERSE);
    }
    else
    {
      PowerTrain->SetDriveMode(ChPowertrain::DriveMode::FORWARD);
    }
  }
}

void UChronoMovementComponent::TickComponent(float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UChronoMovementComponent::TickComponent);
  if (DeltaTime > MaxSubstepDeltaTime)
  {
    uint64_t NumberSubSteps =
        FGenericPlatformMath::FloorToInt(DeltaTime/MaxSubstepDeltaTime);
    if (NumberSubSteps < MaxSubsteps)
    {
      for (uint64_t i = 0; i < NumberSubSteps; ++i)
      {
        AdvanceChronoSimulation(MaxSubstepDeltaTime);
      }
      float RemainingTime = DeltaTime - NumberSubSteps*MaxSubstepDeltaTime;
      if (RemainingTime > 0)
      {
        AdvanceChronoSimulation(RemainingTime);
      }
    }
    else
    {
      double SubDelta = DeltaTime / MaxSubsteps;
      for (uint64_t i = 0; i < MaxSubsteps; ++i)
      {
        AdvanceChronoSimulation(SubDelta);
      }
    }
  }
  else
  {
    AdvanceChronoSimulation(DeltaTime);
  }

  const auto ChronoPositionOffset = ChVector<>(0,0,-0.25f);
  auto VehiclePos = Vehicle->GetVehiclePos() + ChronoPositionOffset;
  auto VehicleRot = Vehicle->GetVehicleRot();
  double Time = Vehicle->GetSystem()->GetChTime();

  FVector NewLocation = ChronoToUE4Location(VehiclePos);
  FQuat NewRotation = ChronoToUE4Quat(VehicleRot);
  if(NewLocation.ContainsNaN() || NewRotation.ContainsNaN())
  {
    UE_LOG(LogCarla, Warning, TEXT(
        "Error: Chrono vehicle position or rotation contains NaN. Disabling chrono physics..."));
    UDefaultMovementComponent::CreateDefaultMovementComponent(CarlaVehicle);
    return;
  }
  CarlaVehicle->SetActorLocation(NewLocation);
  FRotator NewRotator = NewRotation.Rotator();
  // 添加小幅旋转以补偿chrono偏移量
  const float ChronoPitchOffset = 2.5f;
  NewRotator.Add(ChronoPitchOffset, 0.f, 0.f); 
  CarlaVehicle->SetActorRotation(NewRotator);
}

void UChronoMovementComponent::AdvanceChronoSimulation(float StepSize)
{
  double Time = Vehicle->GetSystem()->GetChTime();
  double Throttle = VehicleControl.Throttle;
  double Steering = -VehicleControl.Steer; // 右舵改为左舵
  double Brake = VehicleControl.Brake + VehicleControl.bHandBrake;
  Vehicle->Synchronize(Time, {Steering, Throttle, Brake}, *Terrain.get());
  Vehicle->Advance(StepSize);
  Sys.DoStepDynamics(StepSize);
}

FVector UChronoMovementComponent::GetVelocity() const
{
  if (Vehicle)
  {
    return ChronoToUE4Location(
        Vehicle->GetVehiclePointVelocity(ChVector<>(0,0,0)));
  }
  return FVector();
}

int32 UChronoMovementComponent::GetVehicleCurrentGear() const
{
  if (Vehicle)
  {
    auto PowerTrain = Vehicle->GetPowertrain();
    if (PowerTrain)
    {
      return PowerTrain->GetCurrentTransmissionGear();
    }
  }
  return 0;
}

float UChronoMovementComponent::GetVehicleForwardSpeed() const
{
  if (Vehicle)
  {
    return GetVelocity().X;
  }
  return 0.f;
}

void UChronoMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  if(!CarlaVehicle)
  {
    return;
  }
  // 重置回调函数以响应碰撞
  CarlaVehicle->OnActorHit.RemoveDynamic(
      this, &UChronoMovementComponent::OnVehicleHit);
  CarlaVehicle->GetMesh()->OnComponentBeginOverlap.RemoveDynamic(
      this, &UChronoMovementComponent::OnVehicleOverlap);
  CarlaVehicle->GetMesh()->SetCollisionResponseToChannel(
      ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
}
#endif

void UChronoMovementComponent::DisableSpecialPhysics()
{
  DisableChronoPhysics();
}

void UChronoMovementComponent::DisableChronoPhysics()
{
  this->SetComponentTickEnabled(false);
  EnableUE4VehiclePhysics(true);
  CarlaVehicle->OnActorHit.RemoveDynamic(this, &UChronoMovementComponent::OnVehicleHit);
  CarlaVehicle->GetMesh()->OnComponentBeginOverlap.RemoveDynamic(
      this, &UChronoMovementComponent::OnVehicleOverlap);
  CarlaVehicle->GetMesh()->SetCollisionResponseToChannel(
      ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
  UDefaultMovementComponent::CreateDefaultMovementComponent(CarlaVehicle);
}

void UChronoMovementComponent::OnVehicleHit(AActor *Actor,
    AActor *OtherActor,
    FVector NormalImpulse,
    const FHitResult &Hit)
{
  carla::log_warning("Chrono physics does not support collisions yet, reverting to default PhysX physics.");
  DisableChronoPhysics();
}

// 在车辆网格重叠时，仅当启用了carsim时才有效
// （此事件在与静态环境重叠时触发）
void UChronoMovementComponent::OnVehicleOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult & SweepResult)
{
  if (OtherComp->GetCollisionResponseToChannel(
      ECollisionChannel::ECC_WorldDynamic) ==
      ECollisionResponse::ECR_Block)
  {
    carla::log_warning("Chrono physics does not support collisions yet, reverting to default PhysX physics.");
    DisableChronoPhysics();
  }
}
