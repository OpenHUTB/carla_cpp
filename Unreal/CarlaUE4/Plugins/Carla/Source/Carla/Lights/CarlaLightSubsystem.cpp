// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaLightSubsystem.h"
#include "Carla/Weather/Weather.h"
#include "Kismet/GameplayStatics.h"

//using cr = carla::rpc;

void UCarlaLightSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
  // TODO: 订阅地图变化
}

// UCarlaLightSubsystem类的成员函数，用于反初始化
void UCarlaLightSubsystem::Deinitialize()
{
  // 这个函数目前为空，不执行任何操作
}

// UCarlaLightSubsystem类的成员函数，用于注册一个灯光组件
void UCarlaLightSubsystem::RegisterLight(UCarlaLight* CarlaLight)
{
  // 检查传入的灯光组件是否有效
  if(CarlaLight)
  {
     // 获取灯光的ID
    auto LightId = CarlaLight->GetId();
    // 检查是否已经存在具有相同ID的灯光
    if (Lights.Contains(LightId))
    {
      // 如果存在，记录警告日志并返回
      UE_LOG(LogCarla, Warning, TEXT("Light Id overlapping"));
      return;
    }
    // 将灯光添加到集合中
    Lights.Add(LightId, CarlaLight);
  }
  // 设置客户端状态为脏，表示需要更新
  SetClientStatesdirty("");
}

// UCarlaLightSubsystem类的成员函数，用于注销一个灯光组件
void UCarlaLightSubsystem::UnregisterLight(UCarlaLight* CarlaLight)
{
  // 检查传入的灯光组件是否有效
  if(CarlaLight)
  {
    // 从集合中移除灯光
    Lights.Remove(CarlaLight->GetId());
  }
  // 设置客户端状态为脏，表示需要更新
  SetClientStatesdirty("");
}

// UCarlaLightSubsystem类的成员函数，用于检查是否有待更新的灯光
bool UCarlaLightSubsystem::IsUpdatePending() const
{
  // 遍历客户端状态集合
  for (auto ClientPair : ClientStates)
  {
    // 如果有任何客户端状态为真，表示有更新待处理
    if(ClientPair.Value)
    {
      return true;
    }
  }
  // 如果所有客户端状态都为假，表示没有更新待处理
  return false;
}

// UCarlaLightSubsystem类的成员函数，用于获取所有灯光的状态
std::vector<carla::rpc::LightState> UCarlaLightSubsystem::GetLights(FString Client)
{
  // 创建一个用于存储结果的向量
  std::vector<carla::rpc::LightState> result;

  // 为指定客户端设置状态为false
  ClientStates.FindOrAdd(Client) = false;

  // 遍历所有灯光
  for(auto& Light : Lights)
  {
    // 获取灯光组件
    UCarlaLight* CarlaLight = Light.Value;

    // 将灯光的状态添加到结果向量中
    result.push_back(CarlaLight->GetLightState());
  }
  return result;
}

// UCarlaLightSubsystem类的成员函数，用于设置灯光状态
void UCarlaLightSubsystem::SetLights(
  FString Client,
  std::vector<carla::rpc::LightState> LightsToSet,
  bool DiscardClient)
{
  // 查找指定客户端的状态
  bool* ClientState = ClientStates.Find(Client);

  // 如果找到客户端状态
  if(ClientState) {
    // 遍历所有需要设置的灯光状态
    for(auto& LightState : LightsToSet) {
      // 查找对应的灯光组件
      UCarlaLight* CarlaLight = Lights.FindRef(LightState._id);
      // 如果找到灯光组件，设置其状态
      if(CarlaLight) {
        CarlaLight->SetLightState(LightState);
      }
    }
    // 更新客户端状态为已更新
    *ClientState = true;

    // 如果需要丢弃客户端状态
    if(DiscardClient)
    {
      // 从客户端状态集合中移除指定客户端
      ClientStates.Remove(Client);
    }
  }

}

// UCarlaLightSubsystem类的成员函数，用于根据ID获取灯光组件
UCarlaLight* UCarlaLightSubsystem::GetLight(int Id)
{
  // 检查集合中是否包含指定ID的灯光
  if (Lights.Contains(Id))
  {
    // 如果包含，返回对应的灯光组件
    return Lights[Id];
  }
  // 如果不包含，返回nullptr
  return nullptr;
}

// UCarlaLightSubsystem类的成员函数，用于设置日夜循环是否激活
void UCarlaLightSubsystem::SetDayNightCycle(const bool active) {
  // 获取所有天气控制器
  TArray<AActor*> WeatherActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWeather::StaticClass(), WeatherActors);
  // 如果存在天气控制器
  if (WeatherActors.Num())
  {
    // 获取第一个天气控制器
    if (AWeather* WeatherActor = Cast<AWeather>(WeatherActors[0]))
    {
      // 设置日夜循环状态
      WeatherActor->SetDayNightCycle(active);
    }
  }
}

// UCarlaLightSubsystem类的成员函数，用于标记客户端状态为脏
void UCarlaLightSubsystem::SetClientStatesdirty(FString ClientThatUpdate)
{
  for(auto& Client : ClientStates)
  {
    // 如果当前客户端不是更新的客户端
    if(Client.Key != ClientThatUpdate)
    {
      Client.Value = true;
    }

  }
}
