// ��Ȩ���� (c) 2022 �����������δ�ѧ (UAB) ������Ӿ����� (CVC)��
//
// ����Ʒ���� MIT ���֤��������Ȩ��
// ���踱��������� <https://opensource.org/licenses/MIT>��

#pragma once
#include "WheeledVehicleMovementComponent.h"
#include "Curves/CurveFloat.h"
#include "WheeledVehicleMovementComponentNW.generated.h"

namespace physx
{
    class PxVehicleWheelData;
}

// ���ֲ��������ݽṹ
USTRUCT()
struct FVehicleNWWheelDifferentialData
{
    GENERATED_USTRUCT_BODY()

    /** ���Ϊ True����Ըó���ʩ��Ť�� */
    UPROPERTY(EditAnywhere, Category = Setup)
    bool bDriven;

    FVehicleNWWheelDifferentialData()
        : bDriven(true)
    { }
};

// �����������ݽṹ
USTRUCT()
struct FVehicleNWEngineData
{
    GENERATED_USTRUCT_BODY()

    /** ���� RPM �µ�Ť�أ�Nm��*/
    UPROPERTY(EditAnywhere, Category = Setup)
    FRuntimeFloatCurve TorqueCurve;

    /** ��������ÿ����ת�� */
    UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.01", UIMin = "0.01"))
    float MaxRPM;

    /** ��������ת���ת��������Kgm^2��*/
    UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.01", UIMin = "0.01"))
    float MOI;

    /** ȫ����ʱ����������ʣ�Kgm^2/s��*/
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float DampingRateFullThrottle;

    /** ������Ӻ�ʱ������������������ʣ�Kgm^2/s��*/
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float DampingRateZeroThrottleClutchEngaged;

    /** ���������ʱ���յ���������������������ʣ�Kgm^2/s��*/
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float DampingRateZeroThrottleClutchDisengaged;

    /** ���� TorqueCurve �����ķ�ֵŤ�� */
    float FindPeakTorque() const;
};

// �����������ݽṹ
USTRUCT()
struct FVehicleNWGearData
{
    GENERATED_USTRUCT_BODY()

    /** ����Ť�ر����� */
    UPROPERTY(EditAnywhere, Category = Setup)
    float Ratio;

    /** engineRevs/maxEngineRevs ��ֵ�����ڴ�ֵʱ���� */
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = Setup)
    float DownRatio;

    /** engineRevs/maxEngineRevs ��ֵ�����ڴ�ֵʱ���� */
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = Setup)
    float UpRatio;
};

// �������������ݽṹ
USTRUCT()
struct FVehicleNWTransmissionData
{
    GENERATED_USTRUCT_BODY()
    /** �Ƿ�ʹ���Զ������� */
    UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (DisplayName = "Automatic Transmission"))
    bool bUseGearAutoBox;

    /** ��������ʱ�䣨�룩*/
    UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float GearSwitchTime;

    /** �Զ����������������������Сʱ�䣨�룩*/
    UPROPERTY(EditAnywhere, Category = Setup, meta = (editcondition = "bUseGearAutoBox", ClampMin = "0.0", UIMin = "0.0"))
    float GearAutoBoxLatency;

    /** ���ճ��ֱȳ��Ա��������ֱ� */
    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup)
    float FinalRatio;

    /** ǰ����λ���ֱȣ����30����*/
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay)
    TArray<FVehicleNWGearData> ForwardGears;

    /** �������ֱ� */
    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup)
    float ReverseGearRatio;

    /** engineRevs/maxEngineRevs ��ֵ�����ڴ�ֵʱ���ӵ�λ */
    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
    float NeutralGearUpRatio;

    /** �����ǿ�ȣ�Kgm^2/s��*/
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float ClutchStrength;
};

// ��ʽ�����˶����NW��
UCLASS(ClassGroup = (Physics), meta = (BlueprintSpawnableComponent), hidecategories = (PlanarMovement, "Components|Movement|Planar", Activation, "Components|Activation"))
class CARLA_API UWheeledVehicleMovementComponentNW : public UWheeledVehicleMovementComponent
{
    GENERATED_UCLASS_BODY()

    /** ���� */
    UPROPERTY(EditAnywhere, Category = MechanicalSetup)
    FVehicleNWEngineData EngineSetup;

    /** ������ */
    UPROPERTY(EditAnywhere, Category = MechanicalSetup)
    TArray<FVehicleNWWheelDifferentialData> DifferentialSetup;

    /** ���������� */
    UPROPERTY(EditAnywhere, Category = MechanicalSetup)
    FVehicleNWTransmissionData TransmissionSetup;

    /** ���ת����ǰ���ٶȣ�km/h���Ĺ�ϵ���� */
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

    /** ���䲢���� PhysX ���� */
    virtual void SetupVehicle() override;

    virtual int32 GetCustomGearBoxNumForwardGears() const;

    virtual void UpdateSimulation(float DeltaTime) override;

    /** ����ģ�����ݣ����� */
    virtual void UpdateEngineSetup(const FVehicleNWEngineData& NewEngineSetup);

    /** ����ģ�����ݣ������� */
    virtual void UpdateDifferentialSetup(const TArray<FVehicleNWWheelDifferentialData>& NewDifferentialSetup);

    /** ����ģ�����ݣ������� */
    virtual void UpdateTransmissionSetup(const FVehicleNWTransmissionData& NewGearSetup);
};