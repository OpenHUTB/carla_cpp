// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the
// 卡尔斯鲁厄理工学院
//
// 路径损耗模型
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <vector>


using ActorPowerMap = std::map<AActor *, float>;
using ActorPowerPair = std::pair<AActor *, float>;

enum EPathState
{
    LOS,
    NLOSb,
    NLOSv
};

enum EPathLossModel
{
    Winner,
    Geometric,
};

enum EScenario
{
    Highway,
    Rural,
    Urban
};

class PathLossModel
{
public:
    PathLossModel(URandomEngine *random_engine);
    void SetOwner(AActor *Owner);
    void SetScenario(EScenario scenario);
    void Simulate(const std::vector<ActorPowerPair> ActorList, UCarlaEpisode *CarlaEpisode, UWorld *World);
    ActorPowerMap GetReceiveActorPowerList();
    void SetParams(const float TransmitPower,
                   const float ReceiverSensitivity,
                   const float Frequency,
                   const float combined_antenna_gain,
                   const float path_loss_exponent,
                   const float reference_distance_fspl,
                   const float filter_distance,
                   const bool use_etsi_fading,
                   const float custom_fading_stddev);
    float GetTransmitPower() { return TransmitPower; }
    void SetPathLossModel(const EPathLossModel path_loss_model);

private:
    // 非视距(NLOSv,non-line-of-sight) 衍射
    double CalcVehicleLoss(const double d1, const double d2, const double h);
    // 计算接收功率
    float CalculateReceivedPower(AActor *OtherActor,
                                 const float OtherTransmitPower,
                                 const FVector Source,
                                 const FVector Destination,
                                 const double Distance3d,
                                 const double ht,
                                 const double ht_local,
                                 const double hr,
                                 const double hr_local,
                                 const double reference_z);
    void EstimatePathStateAndVehicleObstacles(AActor *OtherActor, FVector Source, double TxHeight, double RxHeight, double reference_z, EPathState &state, std::vector<FVector> &vehicle_obstacles);
    double MakeVehicleBlockageLoss(double TxHeight, double RxHeight, double obj_height, double obj_distance);
    // 变量
    AActor *mActorOwner;
    UCarlaEpisode *mCarlaEpisode;
    UWorld *mWorld;
    URandomEngine *mRandomEngine;

    ActorPowerMap mReceiveActorPowerList;
    FVector CurrentActorLocation;

    // 常量
    constexpr static float c_speedoflight = 299792458.0;  // 光速（单位为m/s）

    // 完整的双摄像路径损失
    const double epsilon_r = 1.02;

    // 参数
    static double Frequency_GHz;   // 传输频率（GHz）。5.9 GHz 是多个物理信道的标准。 5.9f;//5.9 GHz
    static double Frequency;       // Frequency_GHz * std::pow(10,9);
    static double lambda;          // c_speedoflight/Frequency;
    float reference_distance_fspl; // 对数距离路径损耗模型的参考距离（单位：米m）
    float TransmitPower;           // 发送方传输功率（单位：毫瓦分贝 dBm）
    float ReceiverSensitivity;     // 接收器灵敏度（单位：毫瓦分贝 dBm）
    EScenario scenario;            // 选项：[urban, rustic, highly available]，定义衰落噪声参数
    float path_loss_exponent; // 由于建筑物遮挡导致的非视距损耗参数。没有单位，默认为 2.7;
    float filter_distance;    // 最大传输距离（以米为单位，默认为 500.0），上面的路径损耗计算因模拟速度而略过
    EPathLossModel model;
    bool use_etsi_fading;     // 使用 ETSI 出版物中提到的衰落参数（true），或使用自定义衰落标准偏差
    float custom_fading_stddev;  // 衰减标准偏差的自定义值，仅当use_etsi_fading设置为 false 时才使用
    float combined_antenna_gain; // 10.0 dBi， 发射机和接收机天线的组合增益（以 dBi 为单位），辐射效率和方向性的参数

    // 根据参数设置预先计算的依赖参数
    float m_fspl_d0;

protected:
    /// 如果要追踪光线，则允许预处理的方法。

    float ComputeLoss(AActor *OtherActor, FVector Source, FVector Destination, double Distance3d, double TxHeight, double RxHeight, double reference_z);
    bool IsVehicle(const FHitResult &HitInfo);
    bool GetLocationIfVehicle(const FVector CurrentActorLocation, const FHitResult &HitInfo, const double reference_z, FVector &location);
    bool HitIsSelfOrOther(const FHitResult &HitInfo, AActor *OtherActor);
    float CalculatePathLoss_WINNER(EPathState state, double Distance);
    double CalculateNLOSvLoss(const FVector Source, const FVector Destination, const double TxHeight, const double RxHeight, const double RxDistance3d, std::vector<FVector> &vehicle_obstacles);

    float CalculateShadowFading(EPathState state);

    // 完整的双射线模型
    double CalculateTwoRayPathLoss(double Distance3d, double TxHeight, double RxHeight);
    // 简化的双射线模型
    float CalculateTwoRayPathLossSimple(double Distance3d, double TxHeight, double RxHeight);

    // 预计算功能
    void CalculateFSPL_d0();
    TArray<FHitResult> HitResult;
};
