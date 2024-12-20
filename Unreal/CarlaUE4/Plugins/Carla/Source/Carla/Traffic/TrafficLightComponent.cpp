// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


#include "TrafficLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TrafficLightController.h"
#include "TrafficLightGroup.h"
#include "TrafficLightInterface.h"
#include "TrafficLightManager.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "carla/road/element/RoadInfoSignal.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/road/element/RoadInfoSignal.h>
#include <compiler/enable-ue4-macros.h>
// 交通灯组件类的构造函数，调用父类的构造函数
UTrafficLightComponent::UTrafficLightComponent()
  : Super()
{
}
// 初始化交通灯标志
void UTrafficLightComponent::InitializeSign(const carla::road::Map &Map)
{
  // 一个极小的正数，用于后续的数值计算和比较
  const double epsilon = 0.00001;
 // 获取此信号的所有参考信息，函数 GetAllReferencesToThisSignal 未给出具体实现
  auto References = GetAllReferencesToThisSignal(Map);
  // 遍历所有参考信息
  for (auto& Reference : References)
  {
    auto RoadId = Reference.first;
    const auto* SignalReference = Reference.second;
   // 遍历信号参考的有效范围
    for(auto &validity : SignalReference->GetValidities())
    {
     // 遍历车道范围
      for(auto lane : carla::geom::Math::GenerateRange(validity._from_lane, validity._to_lane))
      {
      // 排除车道 0
        if(lane == 0)
          continue;
        // 定义一个道路元素的路点
        carla::road::element::Waypoint signal_waypoint;
        // 尝试获取路点，存储在 optional 中，因为可能获取失败
        boost::optional<carla::road::element::Waypoint> opt_signal_waypoint = Map.GetWaypoint(
            RoadId, lane, SignalReference->GetS());
        if(opt_signal_waypoint){
        // 成功获取路点
          signal_waypoint = opt_signal_waypoint.get();
        }else{
        // 输出错误日志
          UE_LOG(LogCarla, Error, TEXT("signal_waypoint is not valid") );
          continue;
        }
       // 防止在路口内添加包围盒
        if (Map.IsJunction(RoadId)) {
        // 获取路点的前驱
          auto predecessors = Map.GetPredecessors(signal_waypoint);
          if (predecessors.size() == 1) {
            auto predecessor = predecessors.front();
            if (!Map.IsJunction(predecessor.road_id)) {
              signal_waypoint = predecessor;
            }
          }
        }
        // 检查车道类型是否为行车道，不是则跳过
        if(Map.GetLane(signal_waypoint).GetType() != cr::Lane::LaneType::Driving)
          continue;

        // 计算包围盒宽度为车道宽度的四分之一
        float BoxWidth = static_cast<float>(
            0.5f*Map.GetLaneWidth(signal_waypoint)*0.5);
         // 包围盒长度
        float BoxLength = 1.5f;
        // 包围盒高度
        float BoxHeight = 1.0f;

       // 防止道路宽度为 0 的情况
        BoxWidth = std::max(0.01f, BoxWidth);
        // 获取车道长度和距离
        double LaneLength = Map.GetLane(signal_waypoint).GetLength();
        double LaneDistance = Map.GetLane(signal_waypoint).GetDistance();
      // 防止包围盒与路口重叠的安全距离
        float AdditionalDistance = 1.5f;
        if(lane < 0)
        {
           // 根据车道方向调整路点的 s 值
          signal_waypoint.s = FMath::Clamp(signal_waypoint.s - (BoxLength + AdditionalDistance),
              LaneDistance + epsilon, LaneDistance + LaneLength - epsilon);
        }
        else
        {
        // 根据车道方向调整路点的 s 值
          signal_waypoint.s = FMath::Clamp(signal_waypoint.s + (BoxLength + AdditionalDistance),
              LaneDistance + epsilon, LaneDistance + LaneLength - epsilon);
        }
        // 计算包围盒的变换矩阵
        FTransform BoxTransform = Map.ComputeTransform(signal_waypoint);
         // 获取大地图管理器
        ALargeMapManager* LargeMapManager = UCarlaStatics::GetLargeMapManager(GetWorld());
        if (LargeMapManager)
        {
        // 将全局变换转换为局部变换
          BoxTransform = LargeMapManager->GlobalToLocalTransform(BoxTransform);
        }
        // 生成交通灯包围盒
        GenerateTrafficLightBox(BoxTransform, FVector(100*BoxLength, 100*BoxWidth, 100*BoxHeight));
      }
    }
  }
}
// 生成交通灯包围盒
void UTrafficLightComponent::GenerateTrafficLightBox(const FTransform BoxTransform,
    const FVector BoxSize)
{
  // 生成触发盒，函数 GenerateTriggerBox 未给出具体实现
  UBoxComponent* BoxComponent = GenerateTriggerBox(BoxTransform, BoxSize);
  // 为触发盒添加开始重叠事件处理函数
  BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &UTrafficLightComponent::OnBeginOverlapTriggerBox);
  // 为触发盒添加结束重叠事件处理函数
  BoxComponent->OnComponentEndOverlap.AddDynamic(this, &UTrafficLightComponent::OnEndOverlapTriggerBox);
  // 添加触发体的效果，函数 AddEffectTriggerVolume 未给出具体实现
  AddEffectTriggerVolume(BoxComponent);
}
// 设置交通灯状态
void UTrafficLightComponent::SetLightState(ETrafficLightState NewState)
{
  // 更新交通灯状态
  LightState = NewState;
  // 广播状态改变
  LightChangeDispatcher.Broadcast();
  if (GetOwner()->Implements<UTrafficLightInterface>())
  {
   // 通过接口执行光状态改变函数
    ITrafficLightInterface::Execute_LightChanged(GetOwner(), LightState);
  }
  // 将所有者转换为交通灯基础类型
  ATrafficLightBase* OldTrafficLight = Cast<ATrafficLightBase>(GetOwner());
  if (OldTrafficLight)
  {
  // 调用交通灯基础类型的光状态改变兼容函数
    OldTrafficLight->LightChangedCompatibility(NewState);
  }
  // 遍历车辆控制器，更新它们的交通灯状态
  for (auto Controller : Vehicles)
  {
    if (Controller != nullptr)
    {
      Controller->SetTrafficLightState(LightState);
    }
  }
}
// 获取交通灯状态
ETrafficLightState UTrafficLightComponent::GetLightState() const
{
  return LightState;
}
// 设置交通灯组是否冻结
void UTrafficLightComponent::SetFrozenGroup(bool InFreeze)
{
  if (GetGroup())
  {
    // 调用组的冻结函数
    GetGroup()->SetFrozenGroup(InFreeze);
  }
}
// 获取交通灯组，函数 TrafficLightController->GetGroup() 未给出具体实现
ATrafficLightGroup* UTrafficLightComponent::GetGroup()
{
  return TrafficLightController->GetGroup();
}

const ATrafficLightGroup* UTrafficLightComponent::GetGroup() const
{
  return TrafficLightController->GetGroup();
}
// 获取交通灯控制器
UTrafficLightController* UTrafficLightComponent::GetController()
{
  return TrafficLightController;
}

const UTrafficLightController* UTrafficLightComponent::GetController() const
{
  return TrafficLightController;
}
// 处理触发盒开始重叠事件
void UTrafficLightComponent::OnBeginOverlapTriggerBox(UPrimitiveComponent *OverlappedComp,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult &SweepResult)
{
   // 将重叠的 Actor 转换为 Carla 轮式车辆
  ACarlaWheeledVehicle * Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle)
  {
    // 获取车辆的控制器
    AWheeledVehicleAIController* VehicleController =
        Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (VehicleController)
    {
    // 设置车辆控制器的交通灯状态
     VehicleController->SetTrafficLightState(LightState);
      // 将车辆控制器添加到车辆列表
      Vehicles.Add(VehicleController);
     // 设置车辆控制器的交通灯为当前交通灯
      VehicleController->SetTrafficLight(Cast<ATrafficLightBase>(GetOwner()));
    }
  }
}
// 处理触发盒结束重叠事件
void UTrafficLightComponent::OnEndOverlapTriggerBox(UPrimitiveComponent *OverlappedComp,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex)
{
  // 将重叠的 Actor 转换为 Carla 轮式车辆
  ACarlaWheeledVehicle * Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle)
  {
   // 获取车辆的控制器
    AWheeledVehicleAIController* VehicleController =
        Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (VehicleController)
    {
     // 将车辆控制器的交通灯状态设置为绿色
      VehicleController->SetTrafficLightState(ETrafficLightState::Green);
       // 清除车辆控制器的交通灯
      VehicleController->SetTrafficLight(nullptr);
     // 从车辆列表中移除车辆控制器
      Vehicles.Remove(VehicleController);
    }
  }
}
// 设置交通灯控制器

void UTrafficLightComponent::SetController(UTrafficLightController* Controller)
{
  TrafficLightController = Controller;
}
