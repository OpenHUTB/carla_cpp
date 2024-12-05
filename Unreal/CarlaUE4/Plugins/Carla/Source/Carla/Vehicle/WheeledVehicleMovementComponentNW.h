// 版权所有 (c) 2022 巴塞罗那自治大学 (UAB) 计算机视觉中心 (CVC)。
//
// 本作品根据 MIT 许可证的条款授权。
// 如需副本，请访问 <https://opensource.org/licenses/MIT>。

#pragma once
#include "WheeledVehicleMovementComponent.h"
#include "Curves/CurveFloat.h"
#include "WheeledVehicleMovementComponentNW.generated.h"

namespace physx
{
    class PxVehicleWheelData;
}

// 车轮差速器数据结构
USTRUCT()
struct FVehicleNWWheelDifferentialData
{
    GENERATED_USTRUCT_BODY()

    /** 如果为 True，则对该车轮施加扭矩 */
    UPROPERTY(EditAnywhere, Category = Setup)
    bool bDriven;

    FVehicleNWWheelDifferentialData()
        : bDriven(true)
    { }
};

// 车辆引擎数据结构
USTRUCT()
struct FVehicleNWEngineData
{
    GENERATED_USTRUCT_BODY()

    /** 给定 RPM 下的扭矩（Nm）*/
    UPROPERTY(EditAnywhere, Category = Setup)
    FRuntimeFloatCurve TorqueCurve;

    /** 引擎的最大每分钟转速 */
    UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.01", UIMin = "0.01"))
    float MaxRPM;

    /** 引擎绕旋转轴的转动惯量（Kgm^2）*/
    UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.01", UIMin = "0.01"))
    float MOI;

    /** 全油门时引擎的阻尼率（Kgm^2/s）*/
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float DampingRateFullThrottle;

    /** 离合器接合时零油门下引擎的阻尼率（Kgm^2/s）*/
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float DampingRateZeroThrottleClutchEngaged;

    /** 离合器分离时（空挡）零油门下引擎的阻尼率（Kgm^2/s）*/
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float DampingRateZeroThrottleClutchDisengaged;

    /** 查找 TorqueCurve 产生的峰值扭矩 */
    float FindPeakTorque() const;
};

// 车辆齿轮数据结构
USTRUCT()
struct FVehicleNWGearData
{
    GENERATED_USTRUCT_BODY()

    /** 决定扭矩倍增量 */
    UPROPERTY(EditAnywhere, Category = Setup)
    float Ratio;

    /** engineRevs/maxEngineRevs 的值，低于此值时降档 */
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = Setup)
    float DownRatio;

    /** engineRevs/maxEngineRevs 的值，高于此值时升档 */
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = Setup)
    float UpRatio;
};

// 车辆变速器数据结构
USTRUCT()
struct FVehicleNWTransmissionData
{
    GENERATED_USTRUCT_BODY()
    /** 是否使用自动变速器 */
    UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (DisplayName = "Automatic Transmission"))
    bool bUseGearAutoBox;

    /** 换档所需时间（秒）*/
    UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float GearSwitchTime;

    /** 自动变速器启动换档所需的最小时间（秒）*/
    UPROPERTY(EditAnywhere, Category = Setup, meta = (editcondition = "bUseGearAutoBox", ClampMin = "0.0", UIMin = "0.0"))
    float GearAutoBoxLatency;

    /** 最终齿轮比乘以变速器齿轮比 */
    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup)
    float FinalRatio;

    /** 前进档位齿轮比（最多30个）*/
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay)
    TArray<FVehicleNWGearData> ForwardGears;

    /** 倒档齿轮比 */
    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup)
    float ReverseGearRatio;

    /** engineRevs/maxEngineRevs 的值，高于此值时增加档位 */
    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
    float NeutralGearUpRatio;

    /** 离合器强度（Kgm^2/s）*/
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float ClutchStrength;
};

// 轮式车辆运动组件NW类
UCLASS(ClassGroup = (Physics), meta = (BlueprintSpawnableComponent), hidecategories = (PlanarMovement, "Components|Movement|Planar", Activation, "Components|Activation"))
class CARLA_API UWheeledVehicleMovementComponentNW : public UWheeledVehicleMovementComponent
{
    GENERATED_UCLASS_BODY()

    /** 引擎 */
    UPROPERTY(EditAnywhere, Category = MechanicalSetup)
    FVehicleNWEngineData EngineSetup;

    /** 差速器 */
    UPROPERTY(EditAnywhere, Category = MechanicalSetup)
    TArray<FVehicleNWWheelDifferentialData> DifferentialSetup;

    /** 变速器数据 */
    UPROPERTY(EditAnywhere, Category = MechanicalSetup)
    FVehicleNWTransmissionData TransmissionSetup;

    /** 最大转向与前进速度（km/h）的关系曲线 */
    UPROPERTY(EditAnywhere, Category = SteeringSetup)
    FRuntimeFloatCurve SteeringCurve;

    virtual void Serialize(FArchive& Ar) override;
    virtual void ComputeConstants() override;
#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

    virtual const void* GetTireData(physx::PxVehicleWheels* Wheels, UVehicleWheel* Wheel);
    virtual const int32 GetWheelShapeMapping(physx::PxVehicleWheels* Wheels, uint32 Wheel);
    virtual const physx::PxVehicleWheelData GetWheelData(physx::PxVehicleWheels* Wheels, uint32 Wheel);

    /** 分配并设置 PhysX 车辆 */
    virtual void SetupVehicle() override;

    virtual int32 GetCustomGearBoxNumForwardGears() const;

    virtual void UpdateSimulation(float DeltaTime) override;

    /** 更新模拟数据：引擎 */
    virtual void UpdateEngineSetup(const FVehicleNWEngineData& NewEngineSetup);

    /** 更新模拟数据：差速器 */
    virtual void UpdateDifferentialSetup(const TArray<FVehicleNWWheelDifferentialData>& NewDifferentialSetup);

    /** 更新模拟数据：变速器 */
    virtual void UpdateTransmissionSetup(const FVehicleNWTransmissionData& NewGearSetup);
};