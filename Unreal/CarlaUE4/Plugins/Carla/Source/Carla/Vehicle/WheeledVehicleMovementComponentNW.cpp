// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "WheeledVehicleMovementComponentNW.h"
#include "PhysicsPublic.h"
#include "PhysXPublic.h"
#include "PhysXVehicleManager.h"
#include "Components/PrimitiveComponent.h"
#include "Logging/MessageLog.h"

// 构造函数：初始化车辆组件
UWheeledVehicleMovementComponentNW::UWheeledVehicleMovementComponentNW(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// 从PhysX获取默认值
	PxVehicleEngineData DefEngineData;
	EngineSetup.MOI = DefEngineData.mMOI;
	EngineSetup.MaxRPM = OmegaToRPM(DefEngineData.mMaxOmega);
	EngineSetup.DampingRateFullThrottle = DefEngineData.mDampingRateFullThrottle;
	EngineSetup.DampingRateZeroThrottleClutchEngaged = DefEngineData.mDampingRateZeroThrottleClutchEngaged;
	EngineSetup.DampingRateZeroThrottleClutchDisengaged = DefEngineData.mDampingRateZeroThrottleClutchDisengaged;

	// 将PhysX曲线转换为我们的曲线
	FRichCurve* TorqueCurveData = EngineSetup.TorqueCurve.GetRichCurve();
	for (PxU32 KeyIdx = 0; KeyIdx < DefEngineData.mTorqueCurve.getNbDataPairs(); ++KeyIdx)
	{
		float Input = DefEngineData.mTorqueCurve.getX(KeyIdx) * EngineSetup.MaxRPM;
		float Output = DefEngineData.mTorqueCurve.getY(KeyIdx) * DefEngineData.mPeakTorque;
		TorqueCurveData->AddKey(Input, Output);
	}

	// 设置离合器数据
	PxVehicleClutchData DefClutchData;
	TransmissionSetup.ClutchStrength = DefClutchData.mStrength;

	// 设置变速箱数据
	PxVehicleGearsData DefGearSetup;
	TransmissionSetup.GearSwitchTime = DefGearSetup.mSwitchTime;
	TransmissionSetup.ReverseGearRatio = DefGearSetup.mRatios[PxVehicleGearsData::eREVERSE];
	TransmissionSetup.FinalRatio = DefGearSetup.mFinalRatio;

	// 设置自动变速箱数据
	PxVehicleAutoBoxData DefAutoBoxSetup;
	TransmissionSetup.NeutralGearUpRatio = DefAutoBoxSetup.mUpRatios[PxVehicleGearsData::eNEUTRAL];
	TransmissionSetup.GearAutoBoxLatency = DefAutoBoxSetup.getLatency();
	TransmissionSetup.bUseGearAutoBox = true;

	// 设置前进档位数据
	for (uint32 i = PxVehicleGearsData::eFIRST; i < DefGearSetup.mNbRatios; ++i)
	{
		FVehicleNWGearData GearData;
		GearData.DownRatio = DefAutoBoxSetup.mDownRatios[i];
		GearData.UpRatio = DefAutoBoxSetup.mUpRatios[i];
		GearData.Ratio = DefGearSetup.mRatios[i];
		TransmissionSetup.ForwardGears.Add(GearData);
	}

	// 初始化转向速度曲线
	FRichCurve* SteeringCurveData = SteeringCurve.GetRichCurve();
	SteeringCurveData->AddKey(0.0f, 1.0f);
	SteeringCurveData->AddKey(20.0f, 0.9f);
	SteeringCurveData->AddKey(60.0f, 0.8f);
	SteeringCurveData->AddKey(120.0f, 0.7f);

	// 初始化轮子设置数组，默认为4个轮子
	const int32 NbrWheels = 4;
	WheelSetups.SetNum(NbrWheels);
	DifferentialSetup.SetNum(NbrWheels);

	IdleBrakeInput = 10;
}

// ... [代码继续，保持不变] ...

// 更新引擎设置
void UWheeledVehicleMovementComponentNW::UpdateEngineSetup(const FVehicleNWEngineData& NewEngineSetup)
{
	if (PVehicleDrive)
	{
		PxVehicleEngineData EngineData;
		GetVehicleEngineSetup(NewEngineSetup, EngineData);

		PxVehicleDriveNW* PVehicleDriveNW = (PxVehicleDriveNW*)PVehicleDrive;
		PVehicleDriveNW->mDriveSimData.setEngineData(EngineData);
	}
}

// 更新差速器设置
void UWheeledVehicleMovementComponentNW::UpdateDifferentialSetup(const TArray<FVehicleNWWheelDifferentialData>& NewDifferentialSetup)
{
	if (PVehicleDrive)
	{
		PxVehicleDifferentialNWData DifferentialData;
		GetVehicleDifferentialNWSetup(NewDifferentialSetup, DifferentialData);

		PxVehicleDriveNW* PVehicleDriveNW = (PxVehicleDriveNW*)PVehicleDrive;
		PVehicleDriveNW->mDriveSimData.setDiffData(DifferentialData);
	}
}

// 更新变速箱设置
void UWheeledVehicleMovementComponentNW::UpdateTransmissionSetup(const FVehicleNWTransmissionData& NewTransmissionSetup)
{
	if (PVehicleDrive)
	{
		PxVehicleGearsData GearData;
		GetVehicleGearSetup(NewTransmissionSetup, GearData);

		PxVehicleAutoBoxData AutoBoxData;
		GetVehicleAutoBoxSetup(NewTransmissionSetup, AutoBoxData);

		PxVehicleDriveNW* PVehicleDriveNW = (PxVehicleDriveNW*)PVehicleDrive;
		PVehicleDriveNW->mDriveSimData.setGearsData(GearData);
		PVehicleDriveNW->mDriveSimData.setAutoBoxData(AutoBoxData);
	}
}

// ... [代码继续，保持不变] ...

// 序列化函数
void UWheeledVehicleMovementComponentNW::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	if (Ar.IsLoading() && Ar.UE4Ver() < VER_UE4_VEHICLES_UNIT_CHANGE)
	{
		PxVehicleEngineData DefEngineData;
		const float DefaultRPM = OmegaToRPM(DefEngineData.mMaxOmega);

		// 从旧单位转换到新单位
		EngineSetup.MaxRPM = EngineSetup.MaxRPM != DefaultRPM ? OmegaToRPM(EngineSetup.MaxRPM) : DefaultRPM;	//需要从rad/s转换为RPM
	}

	if (Ar.IsLoading() && Ar.UE4Ver() < VER_UE4_VEHICLES_UNIT_CHANGE2)
	{
		PxVehicleEngineData DefEngineData;
		PxVehicleClutchData DefClutchData;

		// 从旧单位转换到新单位
		BackwardsConvertCm2ToM2NW(EngineSetup.DampingRateFullThrottle, DefEngineData.mDampingRateFullThrottle);
		BackwardsConvertCm2ToM2NW(EngineSetup.DampingRateZeroThrottleClutchDisengaged, DefEngineData.mDampingRateZeroThrottleClutchDisengaged);
		BackwardsConvertCm2ToM2NW(EngineSetup.DampingRateZeroThrottleClutchEngaged, DefEngineData.mDampingRateZeroThrottleClutchEngaged);
		BackwardsConvertCm2ToM2NW(EngineSetup.MOI, DefEngineData.mMOI);
		BackwardsConvertCm2ToM2NW(TransmissionSetup.ClutchStrength, DefClutchData.mStrength);
	}
}

// 计算常量
void UWheeledVehicleMovementComponentNW::ComputeConstants()
{
	Super::ComputeConstants();
	MaxEngineRPM = EngineSetup.MaxRPM;
}

// 获取轮胎数据
const void* UWheeledVehicleMovementComponentNW::GetTireData(physx::PxVehicleWheels* InWheels, UVehicleWheel* InWheel)
{
	const void* realShaderData = &InWheels->mWheelsSimData.getTireData((PxU32)InWheel->WheelIndex);
	return realShaderData;
}

// 获取轮子形状映射
const int32 UWheeledVehicleMovementComponentNW::GetWheelShapeMapping(physx::PxVehicleWheels* InWheels, uint32 InWheel)
{
	const physx::PxI32 ShapeIndex = InWheels->mWheelsSimData.getWheelShapeMapping((PxU32)InWheel);
	return ShapeIndex;
}

// 获取轮子数据
const physx::PxVehicleWheelData UWheeledVehicleMovementComponentNW::GetWheelData(physx::PxVehicleWheels* InWheels, uint32 InWheel)
{
	const physx::PxVehicleWheelData WheelData = InWheels->mWheelsSimData.getWheelData((physx::PxU32)InWheel);
	return WheelData;
}