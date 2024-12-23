// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the 
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include <string>
#include <cstring>
#include <algorithm>
#include "CustomV2XSensor.h"
#include "V2X/PathLossModel.h"

std::list<AActor *> ACustomV2XSensor::mV2XActorContainer;
ACustomV2XSensor::ActorV2XDataMap ACustomV2XSensor::mActorV2XDataMap;

ACustomV2XSensor::ACustomV2XSensor(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));

    // 初始路径损耗模型
    PathLossModelObj = new PathLossModel(RandomEngine);
}

void ACustomV2XSensor::SetOwner(AActor *Owner)
{
    UE_LOG(LogCarla, Warning, TEXT("CustomV2XSensor: called setowner with %p"), Owner);
    if (GetOwner() != nullptr)
    {
        ACustomV2XSensor::mV2XActorContainer.remove(GetOwner());
        UE_LOG(LogCarla, Warning, TEXT("CustomV2XSensor: removed old owner %p"), GetOwner());
    }

    Super::SetOwner(Owner);

    // 如果演员详细信息不可用，则将演员存储到静态列表中
    if(Owner != nullptr)
    {
        if (std::find(ACustomV2XSensor::mV2XActorContainer.begin(), ACustomV2XSensor::mV2XActorContainer.end(), Owner) == ACustomV2XSensor::mV2XActorContainer.end())
        {
            ACustomV2XSensor::mV2XActorContainer.push_back(Owner);
            UE_LOG(LogCarla, Warning, TEXT("CustomV2XSensor: added owner, length now %d"), ACustomV2XSensor::mV2XActorContainer.size());
        }

    }

    PathLossModelObj->SetOwner(Owner);
    
    UCarlaEpisode* CarlaEpisode = UCarlaStatics::GetCurrentEpisode(GetWorld());
    FCarlaActor* CarlaActor = CarlaEpisode->FindCarlaActor(Owner);
    if (CarlaActor != nullptr)
    {
        mStationId = static_cast<long>(CarlaActor->GetActorId());
    }
}

FActorDefinition ACustomV2XSensor::GetSensorDefinition()
{
    return UActorBlueprintFunctionLibrary::MakeCustomV2XDefinition();
}

/* 添加可配置参数的功能*/
void ACustomV2XSensor::Set(const FActorDescription &ActorDescription)
{
    UE_LOG(LogCarla, Warning, TEXT("CustomV2XSensor: Set function called"));
    Super::Set(ActorDescription);
    UActorBlueprintFunctionLibrary::SetCustomV2X(ActorDescription, this);
}

void ACustomV2XSensor::SetPropagationParams(const float TransmitPower,
                                      const float ReceiverSensitivity,
                                      const float Frequency,
                                      const float combined_antenna_gain,
                                      const float path_loss_exponent,
                                      const float reference_distance_fspl,
                                      const float filter_distance,
                                      const bool use_etsi_fading,
                                      const float custom_fading_stddev)
{
    // 将参数转发到PathLossModel对象
    PathLossModelObj->SetParams(TransmitPower, ReceiverSensitivity, Frequency, combined_antenna_gain, path_loss_exponent, reference_distance_fspl, filter_distance, use_etsi_fading, custom_fading_stddev);
}

void ACustomV2XSensor::SetPathLossModel(const EPathLossModel path_loss_model){
    PathLossModelObj->SetPathLossModel(path_loss_model);
}

void ACustomV2XSensor::SetScenario(EScenario scenario)
{
    PathLossModelObj->SetScenario(scenario);
}

/*
 * 函数将参与者详细信息存储到静态列表中。
 *调用CaService对象生成CAM消息将消息存储在静态映射中
 */
void ACustomV2XSensor::PrePhysTick(float DeltaSeconds)
{
    Super::PrePhysTick(DeltaSeconds);
    // 清除上一个sim卡周期创建的消息
    if (GetOwner())
    {
        ACustomV2XSensor::mActorV2XDataMap.erase(GetOwner());

        // 步骤0：如果满足触发条件，则创建要发送的消息
        //这需要在预物理滴答中完成，以便在所有其他v2x传感器中实现同步接收
        //检查消息是否生成
        if (mMessageDataChanged)
        {
            // 如果生成了消息，请将其存储
            //制作一对消息和发送功率
            //如果不同的v2x传感器以不同的功率发送，我们需要存储它
            carla::sensor::data::CustomV2XData message_pw;
            message_pw.Message = CreateCustomV2XMessage();
            
            message_pw.Power = PathLossModelObj->GetTransmitPower();
            ACustomV2XSensor::mActorV2XDataMap.insert({GetOwner(), message_pw});
        }
    }
}

CustomV2XM_t ACustomV2XSensor::CreateCustomV2XMessage()
{
    CustomV2XM_t message = CustomV2XM_t();

    CreateITSPduHeader(message);
    std::strcpy(message.message,mMessageData.c_str());
    mMessageDataChanged = false;
    return message;
}

void ACustomV2XSensor::CreateITSPduHeader(CustomV2XM_t &message)
{
    ITSContainer::ItsPduHeader_t& header = message.header;
    header.protocolVersion = mProtocolVersion;
    header.messageID = mMessageId;
    header.stationID = mStationId;
}

/*
*函数负责向当前参与者发送消息。
*首先通过调用LOSComm对象来模拟通信。
*如果存在列表，则将这些列表中的消息发送给当前参与者
*/
void ACustomV2XSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ACustomV2XSensor::PostPhysTick);

    //步骤1：创建一个参与者列表，其中包含要针对此v2x传感器实例发送的消息
    std::vector<ActorPowerPair> ActorPowerList;
    for (const auto &pair : ACustomV2XSensor::mActorV2XDataMap)
    {
        if (pair.first != GetOwner())
        {
            ActorPowerPair actor_power_pair;
            actor_power_pair.first = pair.first;
            //以发射功率发送的演员
            actor_power_pair.second = pair.second.Power;
            ActorPowerList.push_back(actor_power_pair);
        }
    }

    //步骤2：模拟参与者列表中的参与者与当前参与者的通信。
    if (!ActorPowerList.empty())
    {
        UCarlaEpisode *carla_episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
        PathLossModelObj->Simulate(ActorPowerList, carla_episode, GetWorld());
        //步骤3：获取可以向当前参与者发送消息的参与者列表，以及他们的消息的接收能力。
        ActorPowerMap actor_receivepower_map = PathLossModelObj->GetReceiveActorPowerList();
        //步骤4：检索收到的参与者的消息

        //获取注册表以检索carla演员ID
        const FActorRegistry &Registry = carla_episode->GetActorRegistry();

        ACustomV2XSensor::V2XDataList msg_received_power_list;
        for (const auto &pair : actor_receivepower_map)
        {
            carla::sensor::data::CustomV2XData send_msg_and_pw = ACustomV2XSensor::mActorV2XDataMap.at(pair.first);
            carla::sensor::data::CustomV2XData received_msg_and_pw;
            //发送CAM
            received_msg_and_pw.Message = send_msg_and_pw.Message;
            //接收电力
            received_msg_and_pw.Power = pair.second;

            msg_received_power_list.push_back(received_msg_and_pw);
        }

        WriteMessageToV2XData(msg_received_power_list);
    }
    //步骤5：发送消息

    if (mV2XData.GetMessageCount() > 0)
    {
        auto DataStream = GetDataStream(*this);
        DataStream.SerializeAndSend(*this, mV2XData, DataStream.PopBufferFromPool());
    }
    mV2XData.Reset();
}

/*
*函数将消息存储到结构中，以便将其发送到python客户端
*/
void ACustomV2XSensor::WriteMessageToV2XData(const ACustomV2XSensor::V2XDataList &msg_received_power_list)
{
    for (const auto &elem : msg_received_power_list)
    {
        mV2XData.WriteMessage(elem);
    }
}


void ACustomV2XSensor::Send(const FString message)
{
    //注意：这是不安全的！
    //应该固定在某个地方以限制长度
    mMessageData = TCHAR_TO_UTF8(*message);
    mMessageDataChanged = true;
}

