// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Weather/Weather.h"
#include "Carla/Sensor/SceneCaptureCamera.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"
#include "ConstructorHelpers.h"

// AWeather类的构造函数,接受一个FObjectInitializer对象用于初始化类成员
AWeather::AWeather(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 使用ConstructorHelpers的FObjectFinder在指定路径下查找一个材质资源(用于降水效果的后处理材质)
    // 并将找到的材质对象赋值给PrecipitationPostProcessMaterial成员变量
    PrecipitationPostProcessMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(
        TEXT("Material'/Game/Carla/Static/GenericMaterials/00_MastersOpt/Screen_posProcess/M_screenDrops.M_screenDrops'")).Object;

    // 同样使用ConstructorHelpers的FObjectFinder在指定路径下查找一个材质资源(用于沙尘暴效果的后处理材质)
    // 并将找到的材质对象赋值给DustStormPostProcessMaterial成员变量
    DustStormPostProcessMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(
        TEXT("Material'/Game/Carla/Static/GenericMaterials/00_MastersOpt/Screen_posProcess/M_screenDust_wind.M_screenDust_wind'")).Object;

    // 设置该Actor的Tick函数是否可被调用,这里设置为false,表示不需要每帧更新
    PrimaryActorTick.bCanEverTick = false;
    // 使用传入的ObjectInitializer创建一个默认的场景组件作为根组件,并将其命名为"RootComponent"
    RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

// 检查并应用与天气相关的后处理效果的函数
void AWeather::CheckWeatherPostProcessEffects()
{
    // 如果天气的降水参数大于0.0f
    if (Weather.Precipitation > 0.0f)
        // 将降水效果的后处理材质和降水强度（除以100.0f进行归一化）组成一个元组，添加到ActiveBlendables列表中
        ActiveBlendables.Add(MakeTuple(PrecipitationPostProcessMaterial, Weather.Precipitation / 100.0f));
    else
        // 如果降水参数不大于0.0f，从ActiveBlendables列表中移除降水效果的后处理材质
        ActiveBlendables.Remove(PrecipitationPostProcessMaterial);

    // 如果天气的沙尘暴参数大于0.0f
    if (Weather.DustStorm > 0.0f)
        // 将沙尘暴效果的后处理材质和沙尘暴强度（除以100.0f进行归一化）组成一个元组，添加到ActiveBlendables列表
        ActiveBlendables.Add(MakeTuple(DustStormPostProcessMaterial, Weather.DustStorm / 100.0f));
    else
        // 如果沙尘暴参数不大于0.0f，从ActiveBlendables列表中移除沙尘暴效果的后处理材质
        ActiveBlendables.Remove(DustStormPostProcessMaterial);

    // 创建一个数组用于存储场景中所有的传感器Actor
    TArray<AActor*> SensorActors;
    // 使用UGameplayStatics的GetAllActorsOfClass函数获取场景中所有ASceneCaptureCamera类的Actor，并存储到SensorActors数组中
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASceneCaptureCamera::StaticClass(), SensorActors);
    // 遍历传感器Actor数组
    for (AActor* SensorActor : SensorActors)
    {
        // 将当前遍历到的Actor转换为ASceneCaptureCamera类型的指针
        ASceneCaptureCamera* Sensor = Cast<ASceneCaptureCamera>(SensorActor);
        // 遍历ActiveBlendables列表中的每个元素（即每个后处理材质和强度的元组）
        for (auto& ActiveBlendable : ActiveBlendables)
            // 将当前元组中的后处理材质和强度添加到传感器的场景捕获组件2D的后处理设置中
            Sensor->GetCaptureComponent2D()->PostProcessSettings.AddBlendable(ActiveBlendable.Key, ActiveBlendable.Value);
    }
}

// 应用指定天气参数的函数
void AWeather::ApplyWeather(const FWeatherParameters& InWeather)
{
    // 设置当前天气参数为传入的天气参数
    SetWeather(InWeather);
    // 检查并应用与天气相关的后处理效果
    CheckWeatherPostProcessEffects();

#ifdef CARLA_WEATHER_EXTRA_LOG
    // 如果定义了CARLA_WEATHER_EXTRA_LOG宏，则输出以下日志信息，记录当前天气参数的各项值
    UE_LOG(LogCarla, Log, TEXT("Changing weather:"));
    UE_LOG(LogCarla, Log, TEXT("  - Cloudiness = %.2f"), Weather.Cloudiness);
    UE_LOG(LogCarla, Log, TEXT("  - Precipitation = %.2f"), Weather.Precipitation);
    UE_LOG(LogCarla, Log, TEXT("  - PrecipitationDeposits = %.2f"), Weather.PrecipitationDeposits);
    UE_LOG(LogCarla, Log, TEXT("  - WindIntensity = %.2f"), Weather.WindIntensity);
    UE_LOG(LogCarla, Log, TEXT("  - SunAzimuthAngle = %.2f"), Weather.SunAzimuthAngle);
    UE_LOG(LogCarla, Log, TEXT("  - SunAltitudeAngle = %.2f"), Weather.SunAltitudeAngle);
    UE_LOG(LogCarla, Log, TEXT("  - FogDensity = %.2f"), Weather.FogDensity);
    UE_LOG(LogCarla, Log, TEXT("  - FogDistance = %.2f"), Weather.FogDistance);
    UE_LOG(LogCarla, Log, TEXT("  - FogFalloff = %.2f"), Weather.FogFalloff);
    UE_LOG(LogCarla, Log, TEXT("  - Wetness = %.2f"), Weather.Wetness);
    UE_LOG(LogCarla, Log, TEXT("  - ScatteringIntensity = %.2f"), Weather.ScatteringIntensity);
    UE_LOG(LogCarla, Log, TEXT("  - MieScatteringScale = %.2f"), Weather.MieScatteringScale);
    UE_LOG(LogCarla, Log, TEXT("  - RayleighScatteringScale = %.2f"), Weather.RayleighScatteringScale);
    UE_LOG(LogCarla, Log, TEXT("  - DustStorm = %.2f"), Weather.DustStorm);
#endif // CARLA_WEATHER_EXTRA_LOG

    // 调用能真正改变天气的蓝图。
    RefreshWeather(Weather);
}

// 通知天气相关变化给传感器的函数
void AWeather::NotifyWeather(ASensor* Sensor)
{
    // 检查并应用与天气相关的后处理效果
    CheckWeatherPostProcessEffects();

    // 调用能真正改变天气的蓝图。
    RefreshWeather(Weather);
}

// 设置当前天气参数的函数
void AWeather::SetWeather(const FWeatherParameters& InWeather)
{
    Weather = InWeather;
}

// 设置日夜循环状态的函数
void AWeather::SetDayNightCycle(const bool& active)
{
    DayNightCycle = active;
}
