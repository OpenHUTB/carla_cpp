// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/V2XSensor.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include <string.h>
#include <algorithm>
#include "V2X/CaService.h"
#include "V2XSensor.h"
#include "V2X/PathLossModel.h"
std::list<AActor *> AV2XSensor::mV2XActorContainer;
AV2XSensor::ActorV2XDataMap AV2XSensor::mActorV2XDataMap;

AV2XSensor::AV2XSensor(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));

    // 初始化路径损失模型
    PathLossModelObj = new PathLossModel(RandomEngine);
    CaServiceObj = new CaService(RandomEngine);
}

void AV2XSensor::SetOwner(AActor *Owner)
{
    UE_LOG(LogCarla, Warning, TEXT("V2XSensor: called setowner with %p"), Owner);
    if (GetOwner() != nullptr)
    {
        AV2XSensor::mV2XActorContainer.remove(GetOwner());
        UE_LOG(LogCarla, Warning, TEXT("V2XSensor: removed old owner %p"), GetOwner());
    }

    Super::SetOwner(Owner);

    // 如果 actor 详细信息不可用，则将 actor 存储到静态列表中
    if (Owner != nullptr)
    {
        if (std::find(AV2XSensor::mV2XActorContainer.begin(), AV2XSensor::mV2XActorContainer.end(), Owner) == AV2XSensor::mV2XActorContainer.end())
        {
            AV2XSensor::mV2XActorContainer.push_back(Owner);
            UE_LOG(LogCarla, Warning, TEXT("V2XSensor: added owner, length now %d"), AV2XSensor::mV2XActorContainer.size());
        }
        UWorld *world = GetWorld();
        CaServiceObj->SetOwner(world, Owner);
        PathLossModelObj->SetOwner(Owner);
    }
}

FActorDefinition AV2XSensor::GetSensorDefinition()
{
    return UActorBlueprintFunctionLibrary::MakeV2XDefinition();
}

/* 添加可配置参数的函数*/
void AV2XSensor::Set(const FActorDescription &ActorDescription)
{
    UE_LOG(LogCarla, Warning, TEXT("V2XSensor: Set function called"));
    Super::Set(ActorDescription);
    UActorBlueprintFunctionLibrary::SetV2X(ActorDescription, this);
}

void AV2XSensor::SetCaServiceParams(const float GenCamMin, const float GenCamMax, const bool FixedRate)
{
    // 将参数转发到 CaService Obj
    CaServiceObj->SetParams(GenCamMin, GenCamMax, FixedRate);
}

void AV2XSensor::SetPropagationParams(const float TransmitPower,
                                      const float ReceiverSensitivity,
                                      const float Frequency,
                                      const float combined_antenna_gain,
                                      const float path_loss_exponent,
                                      const float reference_distance_fspl,
                                      const float filter_distance,
                                      const bool use_etsi_fading,
                                      const float custom_fading_stddev)
{
    // 将参数转发到 CaService Obj
    PathLossModelObj->SetParams(TransmitPower, ReceiverSensitivity, Frequency, combined_antenna_gain, path_loss_exponent, reference_distance_fspl, filter_distance, use_etsi_fading, custom_fading_stddev);
}

void AV2XSensor::SetPathLossModel(const EPathLossModel path_loss_model)
{
    PathLossModelObj->SetPathLossModel(path_loss_model);
}

void AV2XSensor::SetScenario(EScenario scenario)
{
    PathLossModelObj->SetScenario(scenario);
}

/*
 * Function 将 actor 详细信息存储在 static 列表中。
 * 调用 CaService 对象生成 CAM 消息
 * 将消息存储在静态 map 中
 */
void AV2XSensor::PrePhysTick(float DeltaSeconds)
{
    Super::PrePhysTick(DeltaSeconds);
    // 清除在上一个 sim 周期中创建的消息
    if (GetOwner())
    {
        AV2XSensor::mActorV2XDataMap.erase(GetOwner());

        // 步骤 0：如果满足触发条件，则创建要发送的消息
        // 这需要在 Pre Phys Tick 中完成，以便在所有其他 V2X 传感器中启用同步接收
        // 检查是否生成了消息
        if (CaServiceObj->Trigger(DeltaSeconds))
        {
            // 如果生成了 message，请存储它
            // 产生一对消息并发送功率
            // 如果不同的 V2X 传感器以不同的功率发送，我们需要将其存储
            carla::sensor::data::CAMData cam_pw;
            cam_pw.Message = CaServiceObj->GetCamMessage();
            cam_pw.Power = PathLossModelObj->GetTransmitPower();
            AV2XSensor::mActorV2XDataMap.insert({GetOwner(), cam_pw});
        }
    }
}

void AV2XSensor::SetAccelerationStandardDeviation(const FVector &Vec)
{
    CaServiceObj->SetAccelerationStandardDeviation(Vec);
}

void AV2XSensor::SetGNSSDeviation(const float noise_lat_stddev,
                                  const float noise_lon_stddev,
                                  const float noise_alt_stddev,
                                  const float noise_head_stddev,
                                  const float noise_lat_bias,
                                  const float noise_lon_bias,
                                  const float noise_alt_bias,
                                  const float noise_head_bias)
{
    CaServiceObj->SetGNSSDeviation(noise_lat_stddev,
                                   noise_lon_stddev,
                                   noise_alt_stddev,
                                   noise_head_stddev,
                                   noise_lat_bias,
                                   noise_lon_bias,
                                   noise_alt_bias,
                                   noise_head_bias);
}

void AV2XSensor::SetVelDeviation(const float noise_vel_stddev)
{
    CaServiceObj->SetVelDeviation(noise_vel_stddev);
}

void AV2XSensor::SetYawrateDeviation(const float noise_yawrate_stddev, const float noise_yawrate_bias)
{
    CaServiceObj->SetYawrateDeviation(noise_yawrate_stddev, noise_yawrate_bias);
}

/*
 * Function 负责向当前 actor 发送消息。
 * First simulates the communication by calling LOSComm object.
 * If there is a list present then messages from those list are sent to the current actor
 */
void AV2XSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(AV2XSensor::PostPhysTick);
    if (GetOwner())
    {
        // 第 1 步：创建一个参与者列表，其中包含要针对此 v2x 传感器实例发送的消息
        std::vector<ActorPowerPair> ActorPowerList;
        for (const auto &pair : AV2XSensor::mActorV2XDataMap)
        {
            if (pair.first != GetOwner())
            {
                ActorPowerPair actor_power_pair;
                actor_power_pair.first = pair.first;
                // actor sending with transmit power
                actor_power_pair.second = pair.second.Power;
                ActorPowerList.push_back(actor_power_pair);
            }
        }

        // 第 2 步：模拟 actor 列表中的 actor 与当前 actor 的通信。
        if (!ActorPowerList.empty())
        {
            UCarlaEpisode *carla_episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
            PathLossModelObj->Simulate(ActorPowerList, carla_episode, GetWorld());
            // 第 3 步：获取可以向当前参与者发送消息的参与者列表，以及对他们消息的接收能力。
            ActorPowerMap actor_receivepower_map = PathLossModelObj->GetReceiveActorPowerList();
            // 第 4 步：检索收到的 actor 的消息

            // get registry to retrieve carla actor IDs
            const FActorRegistry &Registry = carla_episode->GetActorRegistry();

            AV2XSensor::V2XDataList msg_received_power_list;
            for (const auto &pair : actor_receivepower_map)
            {
                // Note: AActor* sender_actor = pair.first;
                carla::sensor::data::CAMData send_msg_and_pw = AV2XSensor::mActorV2XDataMap.at(pair.first);
                carla::sensor::data::CAMData received_msg_and_pw;
                // sent CAM
                received_msg_and_pw.Message = send_msg_and_pw.Message;
                // receive power
                received_msg_and_pw.Power = pair.second;

                msg_received_power_list.push_back(received_msg_and_pw);
            }

            WriteMessageToV2XData(msg_received_power_list);
        }
        // 第 5 步：发送消息

        if (mV2XData.GetMessageCount() > 0)
        {
            auto DataStream = GetDataStream(*this);
            DataStream.SerializeAndSend(*this, mV2XData, DataStream.PopBufferFromPool());
        }
        mV2XData.Reset();
    }
}

/*
 * Function the store the message into the structure so it can be sent to python client
 */
void AV2XSensor::WriteMessageToV2XData(const AV2XSensor::V2XDataList &msg_received_power_list)
{
    for (const auto &elem : msg_received_power_list)
    {
        mV2XData.WriteMessage(elem);
    }
}
