// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 头文件保护指令，确保该头文件内容在一个编译单元中只被包含一次
#pragma once  

#include "WeatherParameters.generated.h"

// 使用USTRUCT宏定义了一个名为FWeatherParameters的结构体，并且标记为BlueprintType，意味着这个结构体可以在蓝图中使用
USTRUCT(BlueprintType)
struct CARLA_API FWeatherParameters
{
    GENERATED_BODY()

    // 使用UPROPERTY宏声明一个公有成员变量Cloudiness（云量），具有以下属性：
    // EditAnywhere表示可以在编辑器的任何地方进行编辑（比如蓝图编辑器或者C++代码中对应的属性面板等）。
    // BlueprintReadWrite表示在蓝图中既可以读取该属性的值，也可以对其进行修改。
    // meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0") 表示对该属性值进行范围限制，
    // 其最小值为0.0，最大值为100.0，在UI界面上显示的最小值和最大值也分别是0.0和100.0，这里表示云量占比的取值范围是0%到100%，默认值设为0.0f（即0%）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
    float Cloudiness = 0.0f;

    // 类似地，声明Precipitation（降水量）属性，其取值范围也是0.0到100.0（可能表示降水量的某种相对占比之类的概念，具体取决于应用场景），默认值为0.0f，同样可在编辑器多处编辑且能在蓝图中读写。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
    float Precipitation = 0.0f;

    // PrecipitationDeposits（降水堆积量）属性，取值范围在0.0到100.0之间，默认值0.0f，具备可编辑与蓝图读写特性，用于描述降水在地面等地方堆积情况的相关度量（同样依具体应用而定）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
    float PrecipitationDeposits = 0.0f;

    // WindIntensity（风强度）属性，取值范围是0.0到100.0，默认值设为0.35f，可编辑且能在蓝图中读写，用于衡量风的强度大小（具体数值对应的实际强度意义需结合游戏或模拟场景来确定）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
    float WindIntensity = 0.35f;

    // SunAzimuthAngle（太阳方位角）属性，取值范围从0.0到360.0度（覆盖了整个圆周角度范围），默认值为0.0f，可编辑且支持蓝图读写，用于确定太阳在水平面上的方位角度。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "360.0", UIMin = "0.0", UIMax = "360.0"))
    float SunAzimuthAngle = 0.0f;

    // SunAltitudeAngle（太阳高度角）属性，取值范围是 -90.0到90.0度（从地平面下到正上方垂直角度范围），默认值设为75.0f，可编辑且能在蓝图中操作，用于表示太阳相对于地平面的高度角度。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "-90.0", ClampMax = "90.0", UIMin = "-90.0", UIMax = "90.0"))
    float SunAltitudeAngle = 75.0f;

    // FogDensity（雾密度）属性，取值范围在0.0到100.0之间，默认值0.0f，可编辑并能在蓝图里读写，用来描述雾的浓密程度（比如数值越高雾越浓等情况）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
    float FogDensity = 0.0f;

    // FogDistance（雾距离）属性，取值范围从0.0开始（可能表示雾从某个参考点开始出现的距离等含义），最大值到1000.0，默认值为0.0f，具备可编辑与蓝图读写特性，用于确定雾在场景中的有效影响距离相关度量。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", UIMin = "0.0", UIMax = "1000.0"))
    float FogDistance = 0.0f;

    // FogFalloff（雾消散系数）属性，取值范围在0.0到10.0之间，默认值0.2f，可编辑且能在蓝图中操作，可能用于控制雾随着距离增加而消散的速度等相关效果参数。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", UIMin = "0.0", UIMax = "10.0"))
    float FogFalloff = 0.2f;

    // Wetness（湿度）属性，取值范围是0.0到100.0，默认值0.0f，可编辑且支持蓝图读写，用于表示环境表面等的潮湿程度相关度量（比如地面、物体表面等的湿度情况）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
    float Wetness = 0.0f;

    // ScatteringIntensity（散射强度）属性，取值范围从0.0到100.0，默认值0.0f，可编辑且能在蓝图中读写，可能用于控制光线在大气等环境中的散射效果强度相关参数。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
    float ScatteringIntensity = 0.0f;

    // MieScatteringScale（米氏散射比例）属性，取值范围在0.0到5.0之间，默认值0.0f，可编辑且支持蓝图读写，用于调整米氏散射相关效果的缩放比例（在涉及光散射模拟等场景中起作用）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "5.0", UIMin = "0.0", UIMax = "5.0"))
    float MieScatteringScale = 0.0f;

    // RayleighScatteringScale（瑞利散射比例）属性，取值范围是0.0到2.0，默认值0.0331f，可编辑且能在蓝图中操作，用于控制瑞利散射相关效果的缩放程度（同样在光散射相关模拟场景中有意义）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
    float RayleighScatteringScale = 0.0331f;

    // DustStorm（尘暴强度）属性，取值范围在0.0到100.0之间，默认值0.0f，可编辑且支持蓝图读写，用于衡量尘暴现象在场景中的强度大小（若有尘暴相关模拟场景的话）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
    float DustStorm = 0.0f;
};
