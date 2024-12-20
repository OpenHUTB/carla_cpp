// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "WheeledVehicleAIController.h"

#include "MapGen/RoadMap.h"
#include "Traffic/RoutePlanner.h"
#include "Vehicle/CarlaWheeledVehicle.h"
#include "Carla/CityMapGenerator.h"
#include "Carla/Util/RandomEngine.h"

#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "WheeledVehicleMovementComponent.h"

// =============================================================================
// -- 静态局部方法 -------------------------------------------------------------
// =============================================================================
 
// 判断从起点到终点的光线投射是否成功击中了一个阻挡物
static bool RayCast(const AActor &Actor, const FVector &Start, const FVector &End)
{
  FHitResult OutHit; // 用于存储光线投射的结果
  static FName TraceTag = FName(TEXT("VehicleTrace")); // 定义光线投射的标签，用于调试或区分不同的光线投射
  FCollisionQueryParams CollisionParams(TraceTag, true); // 设置碰撞查询参数，允许物理碰撞
  CollisionParams.AddIgnoredActor(&Actor); // 添加忽略的Actor，即发起光线投射的Actor本身不会被检测为碰撞对象
 
  // 执行光线投射，检查从Start到End之间是否有物体被击中
  const bool Success = Actor.GetWorld()->LineTraceSingleByObjectType(
      OutHit,
      Start,
      End,
      FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllDynamicObjects), // 查询所有动态对象
      CollisionParams);
 
  // 调试用的代码，根据光线投射是否成功绘制不同颜色的线条，这里被注释掉了
  // DrawDebugLine(Actor.GetWorld(), Start, End,
  //     Success ? FColor(255, 0, 0) : FColor(0, 255, 0), false);
 
  // 返回光线投射是否成功且击中了阻挡物
  return Success && OutHit.bBlockingHit;
}
 
// 判断车辆前方是否有障碍物
static bool IsThereAnObstacleAhead(
    const ACarlaWheeledVehicle &Vehicle,
    const float Speed,
    const FVector &Direction)
{
  // 获取车辆的前向向量和车辆的边界框大小
  const auto ForwardVector = Vehicle.GetVehicleOrientation();
  const auto VehicleBounds = Vehicle.GetVehicleBoundingBoxExtent();
 
  // 获取方向的安全法向量（单位向量）
  FVector NormDirection = Direction.GetSafeNormal();
 
  // 根据速度计算光线投射的距离，至少为50.0f，可能与速度的平方成正比（为什么这样做可能取决于具体的设计考虑）
  const float Distance = std::max(50.0f, Speed * Speed); 
 
  // 计算光线投射的起点和终点，包括中心、右侧和左侧三个方向
  const FVector StartCenter = Vehicle.GetActorLocation() +
      (ForwardVector * (250.0f + VehicleBounds.X / 2.0f)) + FVector(0.0f, 0.0f, 50.0f);
  const FVector EndCenter = StartCenter + NormDirection * (Distance + VehicleBounds.X / 2.0f);
 
  const FVector StartRight = StartCenter +
      (FVector(ForwardVector.Y, -ForwardVector.X, ForwardVector.Z) * 100.0f);
  const FVector EndRight = StartRight + NormDirection * (Distance + VehicleBounds.X / 2.0f);
 
  const FVector StartLeft = StartCenter +
      (FVector(-ForwardVector.Y, ForwardVector.X, ForwardVector.Z) * 100.0f);
  const FVector EndLeft = StartLeft + NormDirection * (Distance + VehicleBounds.X / 2.0f);
 
  // 如果中心、右侧或左侧任意一个方向的光线投射击中了阻挡物，则返回true
  return
    RayCast(Vehicle, StartCenter, EndCenter) ||
    RayCast(Vehicle, StartRight, EndRight) ||
    RayCast(Vehicle, StartLeft, EndLeft);
}
 
// 清空队列中的所有元素
template <typename T>
static void ClearQueue(std::queue<T> &Queue)
{
  std::queue<T> EmptyQueue;
  Queue.swap(EmptyQueue); // 通过交换队列来清空原队列
}
 
// =============================================================================
// -- 构造函数和析构函数 -------------------------------------------------------
// =============================================================================
 
// AWheeledVehicleAIController类的构造函数
AWheeledVehicleAIController::AWheeledVehicleAIController(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine")); // 创建随机数引擎
 
  RandomEngine->Seed(RandomEngine->GenerateRandomSeed()); // 为随机数引擎设置随机种子
 
  PrimaryActorTick.bCanEverTick = true; // 允许该Actor参与Tick
  PrimaryActorTick.TickGroup = TG_PrePhysics; // 设置Tick的优先级为物理计算之前
}
 
// AWheeledVehicleAIController类的析构函数
AWheeledVehicleAIController::~AWheeledVehicleAIController() {}
 
// =============================================================================
// -- AController --------------------------------------------------------------
// =============================================================================
 
// 当控制器接管一个pawn时调用
void AWheeledVehicleAIController::OnPossess(APawn *aPawn)
{
  Super::OnPossess(aPawn); // 调用父类的OnPossess方法
 
  // 如果已经控制了一个车辆，则打印错误日志并返回
  if (IsPossessingAVehicle())
  {
    UE_LOG(LogCarla, Error, TEXT("Controller already possessing a vehicle!"));
    return;
  }
  Vehicle = Cast<ACarlaWheeledVehicle>(aPawn); // 将传入的pawn转换为车辆类型
  check(Vehicle != nullptr); // 确保转换成功
  MaximumSteerAngle = Vehicle->GetMaximumSteerAngle(); // 获取车辆的最大转向角度
  check(MaximumSteerAngle > 0.0f); // 确保最大转向角度大于0
  ConfigureAutopilot(bAutopilotEnabled); // 根据是否启用自动驾驶配置自动驾驶参数
 
  // 如果没有设置路线图，则尝试从世界中获取
  if (RoadMap == nullptr)
  {
    TActorIterator<ACityMapGenerator> It(GetWorld());
    RoadMap = (It ? It->GetRoadMap() : nullptr);
  }
}
 
// 当控制器失去对一个pawn的控制时调用
void AWheeledVehicleAIController::OnUnPossess()
{
  Super::OnUnPossess(); // 调用父类的OnUnPossess方法
 
  Vehicle = nullptr; // 清空车辆指针
}
 
// 每一帧调用一次的函数
void AWheeledVehicleAIController::Tick(const float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AWheeledVehicleAIController::Tick); // 性能分析工具的事件范围
  Super::Tick(DeltaTime); // 调用父类的Tick方法
 
  // 如果没有控制一个车辆，则直接返回
  if (!IsPossessingAVehicle())
  {
    return;
  }
 
  // 如果没有启用自动驾驶且控制不是粘性的，则放松车辆控制
  if (!bAutopilotEnabled && !bControlIsSticky)
  {
    Vehicle->ApplyVehicleControl(FVehicleControl{}, EVehicleInputPriority::Relaxation);
  }
 
  // 刷新车辆控制
  Vehicle->FlushVehicleControl();
}
 
// =============================================================================
// -- 自动驾驶 ----------------------------------------------------------------
// =============================================================================
 
// 根据是否启用自动驾驶和是否保留当前状态来配置自动驾驶参数
void AWheeledVehicleAIController::ConfigureAutopilot(const bool Enable, const bool KeepState)
{
  bAutopilotEnabled = Enable; // 设置自动驾驶启用状态
  if (!KeepState)
  {
    // 重置状态
    Vehicle->SetSteeringInput(0.0f); // 设置转向输入为0
    Vehicle->SetThrottleInput(0.0f); // 设置油门输入为0
    Vehicle->SetBrakeInput(0.0f); // 设置刹车输入为0
    Vehicle->SetReverse(false); // 设置不倒车
    Vehicle->SetHandbrakeInput(false); // 设置不拉手刹
    ClearQueue(TargetLocations); // 清空目标位置队列
    Vehicle->SetAIVehicleState(
        bAutopilotEnabled ?
        ECarlaWheeledVehicleState::FreeDriving : // 启用自动驾驶时设置为自由驾驶状态
        ECarlaWheeledVehicleState::AutopilotOff); // 禁用自动驾驶时设置为自动驾驶关闭状态
  }
 
  TrafficLightState = ETrafficLightState::Green; // 默认交通灯状态为绿灯
}
 
// =============================================================================
// -- 交通 --------------------------------------------------------------------
// =============================================================================
 
// 设置固定的行驶路线，可以覆盖当前的路线
void AWheeledVehicleAIController::SetFixedRoute(
    const TArray<FVector> &Locations,
    const bool bOverwriteCurrent)
{
  if (bOverwriteCurrent)
  {
    ClearQueue(TargetLocations); // 如果需要覆盖当前路线，则清空目标位置队列
  }
  for (auto &Location : Locations)
  {
    TargetLocations.emplace(Location); // 将新的位置添加到目标位置队列中
  }
}
