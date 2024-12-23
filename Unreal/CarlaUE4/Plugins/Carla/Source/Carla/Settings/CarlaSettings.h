// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Engine/StaticMesh.h"

#include "Carla/Settings/QualityLevelUE.h"

#include "CarlaSettings.generated.h"

/// Carla 的全局设置
///
/// 设置对象用于将配置设置和可编辑设置保存在一个位置。为确保设置保存到指定的配置文件，请确保使用全局配置或配置元数据添加属性。
UCLASS(BlueprintType, Blueprintable, config = Game, defaultconfig)
class CARLA_API UCarlaSettings : public UObject
{
  GENERATED_BODY()

public:

  /// 设置新的质量设置级别并在游戏中进行与之相关的更改。
  ///
  /// @note 这不会应用质量设置。请改用 ApplyQualitySettings 函数
  /// @param InQualityLevel Store the new quality.
  UFUNCTION(BlueprintCallable, Category = "CARLA Settings")
  void SetQualityLevel(EQualityLevel InQualityLevel)
  {
    QualityLevel = InQualityLevel;
  }

  /// @return 当前质量设置级别（尚未应用）。
  UFUNCTION(BlueprintCallable, Category = "CARLA Settings")
  EQualityLevel GetQualityLevel() const
  {
    return QualityLevel;
  }

  /// 根据命令行参数和 INI 文件（如果提供）加载设置。
  void LoadSettings();

  /// 从给定的字符串（格式为 INI）加载设置。CarlaServer 部分被忽略。
  void LoadSettingsFromString(const FString &INIFileContents);

  /// 记录设置值。
  void LogSettings() const;

public:

  /// CARLA_ROAD 名称用于标记道路网格参与者。
  static const FName CARLA_ROAD_TAG;

  /// CARLA_SKY 名称用于标记场景中的天空球（sky sphere, BPS）参与者。
  static const FName CARLA_SKY_TAG;

private:

  void LoadSettingsFromFile(const FString &FilePath, bool bLogOnFailure);

  /// 用于加载此设置的设置文件的文件名。如果没有使用则为空。
  UPROPERTY(Category = "CARLA Settings|Debug", VisibleAnywhere)
  FString CurrentFileName;

  // ===========================================================================
  /// @name CARLA 服务端
  // ===========================================================================
  /// @{

public:

  /// 用于监听客户端连接的世界端口。
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere, meta = (EditCondition = bUseNetworking))
  uint32 RPCPort = 2000u;

  /// 流媒体端口的设置。
  uint32 StreamingPort = 2001u;

  /// 辅助服务器端口的设置。
  uint32 SecondaryPort = 2002u;

  /// 设置要连接的主服务器的IP和端口。
  std::string PrimaryIP = "";
  uint32      PrimaryPort = 2002u;

  /// 在同步模式下，CARLA 会等待每个节拍信号，直到收到来自客户端的控制。
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere, meta = (EditCondition = bUseNetworking))
  bool bSynchronousMode = false;

  /// 启用或禁用世界的视窗渲染。默认情况下禁用。
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere)
  bool bDisableRendering = false;

  // ===========================================================================
  /// @name 画质设置
  // ===========================================================================
  /// @{

private:

  /// 画质设置等级
  UPROPERTY(Category = "Quality Settings", VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
  EQualityLevel QualityLevel = EQualityLevel::Epic;

public:

  /// 低质量道路材料。使用插槽名称为道路的每个部分设置低质量的材料。
  ///
  /// @todo Move Low quality vars to a generic map of structs with the quality
  /// level as key.
  UPROPERTY(Category = "Quality Settings/Low",
      BlueprintReadOnly,
      EditAnywhere,
      config,
      DisplayName = "Road Materials List for Low Quality")
  TArray<FStaticMaterial> LowRoadMaterials;

  /// 光照函数应完全淡化为 禁用亮度（DisabledBrightness）的距离。
  /// 这对于隐藏在远处应用的光照函数的混叠很有用。
  UPROPERTY(Category = "Quality Settings/Low", BlueprintReadOnly, EditAnywhere, config)
  float LowLightFadeDistance  = 1000.0f;

  /// 所有原始组件的默认 low 距离。 
  UPROPERTY(Category = "Quality Settings/Low", BlueprintReadOnly, EditAnywhere, config,
      meta = (ClampMin = "5000.0", ClampMax = "20000.0", UIMin = "5000.0", UIMax = "20000.0"))
  float LowStaticMeshMaxDrawDistance = 10000.0f;

  /// 道路网格的默认 low 距离。
  UPROPERTY(Category = "Quality Settings/Low", BlueprintReadOnly, EditAnywhere, config,
      meta = (ClampMin = "5000.0", ClampMax = "20000.0", UIMin = "5000.0", UIMax = "20000.0"))
  float LowRoadPieceMeshMaxDrawDistance = 15000.0f;

  /// 史诗级质量道路材料。
  /// 使用插槽名称为道路的每个部分设置质量的材料。
  UPROPERTY(Category = "Quality Settings/Epic",
      BlueprintReadOnly,
      EditAnywhere,
      config,
      DisplayName = "Road Materials List for EPIC Quality")
  TArray<FStaticMaterial> EpicRoadMaterials;

  /// Enable ROS2
  UPROPERTY(Category = "Quality Settings/ROS2",
      BlueprintReadOnly,
      EditAnywhere,
      config,
      DisplayName = "Enable ROS2")
  bool ROS2 = false;

  /// @}
};
