// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 包含禁用UE4宏的编译器指令
#include <compiler/disable-ue4-macros.h>
// 包含carla的rpc LightState
#include <carla/rpc/LightState.h>
// 包含启用UE4宏的编译器指令
#include <compiler/enable-ue4-macros.h>

#include "CoreMinimal.h"//CoreMinimal.h是虚幻引擎的一个基础头文件。
#include "Components/ActorComponent.h"//这表示这个源文件（假设是一个.cpp文件）正在包含ActorComponent.h头文件。
#include "CarlaLight.generated.h"//这是一个自定义的头文件


// 将carla的rpc枚举值转换为UE4的枚举值
#define CARLA_ENUM_FROM_RPC(e) static_cast<uint8>(carla::rpc::LightState::LightGroup:: e)
// 定义一个枚举类型，用于表示不同类型的灯光
UENUM(BlueprintType)
enum class ELightType : uint8
{
  Null      = 0, // UE4.24 枚举问题的解决方法
  Vehicle   = CARLA_ENUM_FROM_RPC(Vehicle)    UMETA(DisplayName = "Vehicle"),
  Street    = CARLA_ENUM_FROM_RPC(Street)     UMETA(DisplayName = "Street"),
  Building  = CARLA_ENUM_FROM_RPC(Building)   UMETA(DisplayName = "Building"),
  Other     = CARLA_ENUM_FROM_RPC(Other)      UMETA(DisplayName = "Other"),
};
// 取消宏定义，以便在其他地方重新使用
#undef CARLA_ENUM_FROM_RPC

// 表示场景中灯光的类
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CARLA_API UCarlaLight : public UActorComponent
{
  GENERATED_BODY()

public:
 // 构造函数
  UCarlaLight();
// 当组件开始播放时调用
  void BeginPlay() override;
 // 当组件结束播放时调用
  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
// 当组件被销毁时调用
  void OnComponentDestroyed(bool bDestroyingHierarchy) override;
// 注册灯光的蓝图可调用函数
  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void RegisterLight();
 // 更新灯光的蓝图可实现事件
  UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Carla Light")
  void UpdateLights();
// 根据天气注册灯光的蓝图可实现事件
  UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Carla Light")
  void RegisterLightWithWeather();
// 设置灯光强度的蓝图可调用函数
  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightIntensity(float Intensity);
// 获取灯光强度的蓝图纯函数
  UFUNCTION(BlueprintPure, Category = "Carla Light")
  float GetLightIntensity() const;
 // 设置灯光颜色的蓝图可调用函数
  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightColor(FLinearColor Color);
// 获取灯光颜色的蓝图纯函数
  UFUNCTION(BlueprintPure, Category = "Carla Light")
  FLinearColor GetLightColor() const;
 // 获取灯光开关状态的蓝图纯函数
  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightOn(bool bOn);
 // 获取灯光开关状态的蓝图纯函数

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  bool GetLightOn() const;
 // 设置灯光类型的蓝图可调用函数
  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightType(ELightType Type);
 // 获取灯光类型的蓝图纯函数
  UFUNCTION(BlueprintPure, Category = "Carla Light")
  ELightType GetLightType() const;
// 获取carla的rpc LightState
  carla::rpc::LightState GetLightState();
// 设置carla的rpc LightState
  void SetLightState(carla::rpc::LightState LightState);

  // 获取灯光位置
  FVector GetLocation() const;
// 获取灯光ID的蓝图纯函数
  UFUNCTION(BlueprintPure, Category = "Carla Light")
  int GetId() const;
// 设置灯光ID的蓝图可调用函数
  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetId(int InId);

protected:
  // 可编辑的属性，灯光类型
  UPROPERTY(EditAnywhere, Category = "Carla Light")
  ELightType LightTyp e = ELightType::Street;
 // 可编辑的属性，灯光强度
  UPROPERTY(EditAnywhere, Category = "Carla Light")
  float LightIntensity;

  // 可编辑的属性，灯光颜色
  UPROPERTY(EditAnywhere, Category = "Carla Light")
  FLinearColor LightColor;
 // 可编辑的属性，灯光开关状态
  UPROPERTY(EditAnywhere, Category = "Carla Light")
  bool bLightOn;
// 可编辑的属性，灯光ID
  UPROPERTY(EditAnywhere, Category = "Carla Light")
  int Id = -1;

  private:
// 记录灯光变化的私有函数
  void RecordLightChange() const;
// 标记灯光是否已注册
  bool bRegistered = false;
};
