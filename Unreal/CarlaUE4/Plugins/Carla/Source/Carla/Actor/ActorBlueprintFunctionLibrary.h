// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/PedestrianParameters.h"
#include "Carla/Actor/PropParameters.h"
#include "Carla/Actor/VehicleParameters.h"
#include "Carla/Sensor/GnssSensor.h"
#include "Carla/Sensor/Radar.h"
#include "Carla/Sensor/InertialMeasurementUnit.h"
#include "Carla/Sensor/V2XSensor.h"
#include "Carla/Sensor/CustomV2XSensor.h"
#include "Carla/Sensor/V2XSensor.h"
#include "Carla/Sensor/CustomV2XSensor.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "ActorBlueprintFunctionLibrary.generated.h"

class ASceneCaptureSensor;
class AShaderBasedSensor;
struct FLidarDescription;

UCLASS()
class UActorBlueprintFunctionLibrary : public UBlueprintFunctionLibrary// 继承自UBlueprintFunctionLibrary。
{
  GENERATED_BODY()
// Unreal Engine的宏，用于自动生成类的某些部分。

public:

  /// @}
  /// ==========================================================================
  /// @name 参与者定义验证器
  /// ==========================================================================
  /// @{

  /// 返回角色定义是否有效。打印所有找到的错误
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static bool CheckActorDefinition(const FActorDefinition &ActorDefinitions);

  /// 返回演员定义列表是否有效。打印所有
  /// 发现错误
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static bool CheckActorDefinitions(const TArray<FActorDefinition> &ActorDefinitions);

  /// @}
  /// ==========================================================================
  /// @name 创建角色定义的辅助工具
  /// ==========================================================================
  /// @{
 /// 创建一个通用的参与者定义。
  static FActorDefinition MakeGenericDefinition(
      const FString &Category,
      const FString &Type,
      const FString &Id);
/// 创建一个通用的传感器参与者定义。
  static FActorDefinition MakeGenericSensorDefinition(
      const FString &Type,
      const FString &Id);
/// 创建一个相机参与者定义。
  static FActorDefinition MakeCameraDefinition(
      const FString &Id,
      bool bEnableModifyingPostProcessEffects = false);

/// 创建一个相机参与者定义，并返回操作是否成功。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeCameraDefinition(
      const FString &Id,
      bool bEnableModifyingPostProcessEffects,
      bool &Success,
      FActorDefinition &Definition);
/// 创建一个法线相机参与者定义。
  static FActorDefinition MakeNormalsCameraDefinition();
/// 创建一个法线相机参与者定义，并返回操作是否成功。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeNormalsCameraDefinition(
      bool &Success,
      FActorDefinition &Definition);
/// 创建一个激光雷达参与者定义。
  static FActorDefinition MakeLidarDefinition(
      const FString &Id);
/// 创建一个激光雷达参与者定义，并返回操作是否成功。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeLidarDefinition(
      const FString &Id,
      bool &Success,
      FActorDefinition &Definition);
/// 创建一个全球导航卫星系统传感器参与者定义。
  static FActorDefinition MakeGnssDefinition();
/// 创建一个全球导航卫星系统传感器参与者定义，并返回操作是否成功。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeGnssDefinition(
      bool &Success,
      FActorDefinition &Definition);
/// 创建一个惯性测量单元参与者定义。
  static FActorDefinition MakeIMUDefinition();
/// 创建一个惯性测量单元参与者定义，并返回操作是否成功。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeIMUDefinition(
      bool &Success,
      FActorDefinition &Definition);
/// 创建一个雷达参与者定义。
  static FActorDefinition MakeRadarDefinition();
/// 创建一个雷达参与者定义，并返回操作是否成功。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeRadarDefinition(
      bool &Success,
      FActorDefinition &Definition);
/// 创建一个车对外界信息交换传感器参与者定义。
  static FActorDefinition MakeV2XDefinition();
/// 创建一个车对外界信息交换传感器参与者定义，并返回操作是否成功。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeV2XDefinition(
      bool &Success,
      FActorDefinition &Definition);
/// 创建一个自定义车对外界信息交换传感器参与者定义。
  static FActorDefinition MakeCustomV2XDefinition();
/// 创建一个自定义车对外界信息交换传感器参与者定义，并返回操作是否成功。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeCustomV2XDefinition(
      bool &Success,
      FActorDefinition &Definition);            
/// 根据车辆参数创建车辆参与者定义，并返回操作是否成功。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeVehicleDefinition(
      const FVehicleParameters &Parameters,
      bool &Success,
      FActorDefinition &Definition);
/// 根据车辆参数数组创建多个车辆参与者定义。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeVehicleDefinitions(
      const TArray<FVehicleParameters> &ParameterArray,
      TArray<FActorDefinition> &Definitions);
/// 根据行人参数创建行人参与者定义，并返回操作是否成功。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakePedestrianDefinition(
      const FPedestrianParameters &Parameters,
      bool &Success,
      FActorDefinition &Definition);
/// 根据行人参数数组创建多个行人参与者定义。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakePedestrianDefinitions(
      const TArray<FPedestrianParameters> &ParameterArray,
      TArray<FActorDefinition> &Definitions);
 /// 根据ID数组创建触发器参与者定义。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeTriggerDefinitions(
      const TArray<FString> &ParameterArray,
      TArray<FActorDefinition> &Definitions);
/// 根据ID创建触发器参与者定义，并返回操作是否成功。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeTriggerDefinition(
      const FString &Id,
      bool &Success,
      FActorDefinition &Definition);
/// 根据道具参数创建道具参与者定义，并返回操作是否成功。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakePropDefinition(
      const FPropParameters &Parameters,
      bool &Success,
      FActorDefinition &Definition);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakePropDefinitions(
      const TArray<FPropParameters> &ParameterArray,
      TArray<FActorDefinition> &Definitions);

  UFUNCTION()
  static void MakeObstacleDetectorDefinitions(
      const FString &Type,
      const FString &Id,
      FActorDefinition &Definition);

  /// @}
  /// ==========================================================================
  /// @name 获取属性值的帮助程序
  /// ==========================================================================
  /// @{
  /// 将参与者属性转换为布尔值，如果不存在则返回默认值。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static bool ActorAttributeToBool(const FActorAttribute &ActorAttribute, bool Default);
/// 将参与者属性转换为整数，如果不存在则返回默认值。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static int32 ActorAttributeToInt(const FActorAttribute &ActorAttribute, int32 Default);
/// 将参与者属性转换为浮点数，如果不存在则返回默认值。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static float ActorAttributeToFloat(const FActorAttribute &ActorAttribute, float Default);
/// 将参与者属性转换为字符串，如果不存在则返回默认值。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FString ActorAttributeToString(const FActorAttribute &ActorAttribute, const FString &Default);
/// 将参与者属性转换为颜色，如果不存在则返回默认值。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FColor ActorAttributeToColor(const FActorAttribute &ActorAttribute, const FColor &Default);
/// 从参与者属性映射中检索布尔值，如果不存在则返回默认值。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static bool RetrieveActorAttributeToBool(
      const FString &Id,
      const TMap<FString, FActorAttribute> &Attributes,
      bool Default);
/// 从参与者属性映射中检索整数，如果不存在则返回默认值。
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static int32 RetrieveActorAttributeToInt(
      const FString &Id,
      const TMap<FString, FActorAttribute> &Attributes,
      int32 Default);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static float RetrieveActorAttributeToFloat(
      const FString &Id,
      const TMap<FString, FActorAttribute> &Attributes,
      float Default);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FString RetrieveActorAttributeToString(
      const FString &Id,
      const TMap<FString, FActorAttribute> &Attributes,
      const FString &Default);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FColor RetrieveActorAttributeToColor(
      const FString &Id,
      const TMap<FString, FActorAttribute> &Attributes,
      const FColor &Default);

  /// @}
  /// ==========================================================================
  /// @name 设置角色的辅助工具
  /// ==========================================================================
  /// @{

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void SetCamera(const FActorDescription &Description, ASceneCaptureSensor *Camera);
  static void SetCamera(const FActorDescription &Description, AShaderBasedSensor *Camera);

  static void SetLidar(const FActorDescription &Description, FLidarDescription &Lidar);

  static void SetGnss(const FActorDescription &Description, AGnssSensor *Gnss);

  static void SetIMU(const FActorDescription &Description, AInertialMeasurementUnit *IMU);

  static void SetRadar(const FActorDescription &Description, ARadar *Radar);

  static void SetV2X(const FActorDescription &Description, AV2XSensor *V2X);
  static void SetCustomV2X(const FActorDescription &Description, ACustomV2XSensor *V2X);
};
