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

void UCarlaLightSubsystem::Deinitialize()
{

}

void UCarlaLightSubsystem::RegisterLight(UCarlaLight* CarlaLight)
{
  if(CarlaLight)
  {
    auto LightId = CarlaLight->GetId();
    if (Lights.Contains(LightId))
    {
      UE_LOG(LogCarla, Warning, TEXT("Light Id overlapping"));
      return;
    }
    Lights.Add(LightId, CarlaLight);
  }
  SetClientStatesdirty("");
}

void UCarlaLightSubsystem::UnregisterLight(UCarlaLight* CarlaLight)//用于从灯光子系统中注销一个灯光对象。UCarlaLight* CarlaLight - 指向要注销的灯光对象的指针。
{
  if(CarlaLight)
  {
    Lights.Remove(CarlaLight->GetId());
  }
  SetClientStatesdirty("");
}

bool UCarlaLightSubsystem::IsUpdatePending() const
{
  for (auto ClientPair : ClientStates)
  {
    if(ClientPair.Value)
    {
      return true;
    }
  }
  return false;
}

std::vector<carla::rpc::LightState> UCarlaLightSubsystem::GetLights(FString Client)
{
  std::vector<carla::rpc::LightState> result;

  ClientStates.FindOrAdd(Client) = false;

  for(auto& Light : Lights)
  {
    UCarlaLight* CarlaLight = Light.Value;

    result.push_back(CarlaLight->GetLightState());
  }
  return result;
}

void UCarlaLightSubsystem::SetLights(
  FString Client,
  std::vector<carla::rpc::LightState> LightsToSet,//一个std::vector容器，包含carla::rpc::LightState类型的对象，这些对象表示要设置的灯光状态。
  bool DiscardClient)
{
  bool* ClientState = ClientStates.Find(Client);//在ClientStates容器中查找与给定Client标识符相关联的客户端状态。返回的是一个指向布尔值的指针，表示该客户端是否有待处理的更新。

  if(ClientState) {
    for(auto& LightState : LightsToSet) {
      UCarlaLight* CarlaLight = Lights.FindRef(LightState._id);
      if(CarlaLight) {
        CarlaLight->SetLightState(LightState);
      }
    }
    *ClientState = true;//最后，将找到的客户端状态设置为true，表示有灯光状态更新待处理。这里直接修改了指针指向的值，而不是通过引用或返回值来更新状态。

    if(DiscardClient)
    {
      ClientStates.Remove(Client);
    }
  }

}

UCarlaLight* UCarlaLightSubsystem::GetLight(int Id)
{
  if (Lights.Contains(Id))
  {
    return Lights[Id];
  }
  return nullptr;
}

void UCarlaLightSubsystem::SetDayNightCycle(const bool active) {
  TArray<AActor*> WeatherActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWeather::StaticClass(), WeatherActors);
  if (WeatherActors.Num())
  {
    if (AWeather* WeatherActor = Cast<AWeather>(WeatherActors[0]))
    {
      WeatherActor->SetDayNightCycle(active);
    }
  }
}

void UCarlaLightSubsystem::SetClientStatesdirty(FString ClientThatUpdate)
{
  for(auto& Client : ClientStates)
  {
    if(Client.Key != ClientThatUpdate)
    {
      Client.Value = true;
    }

  }
}
