// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Settings/CarlaSettings.h"

#include "Carla/Util/IniFile.h"

#include "CommandLine.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Engine.h"
#include "Engine/PointLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstance.h"
#include "Package.h"
#include "UnrealMathUtility.h"

// 初始化设置文件（INI file）段
#define S_CARLA_SERVER          TEXT("CARLA/Server")
#define S_CARLA_QUALITYSETTINGS TEXT("CARLA/QualitySettings")

// =============================================================================
// -- 静态变量 & 常量 -----------------------------------------------------------
// =============================================================================

const FName UCarlaSettings::CARLA_ROAD_TAG = FName("CARLA_ROAD");
const FName UCarlaSettings::CARLA_SKY_TAG = FName("CARLA_SKY");

// =============================================================================
// -- 静态方法 ------------------------------------------------------------------
// =============================================================================
// 定义一个静态函数，用于将字符串转换为质量等级枚举值  
// 参数：  
// - SQualitySettingsLevel：一个FString类型的字符串，表示质量等级的设置  
// - Default：一个EQualityLevel类型的默认值，当字符串不匹配任何已知质量等级时返回此值，默认为EQualityLevel::INVALID 
static EQualityLevel QualityLevelFromString(
    const FString &SQualitySettingsLevel,
    const EQualityLevel Default = EQualityLevel::INVALID)
{
  // 如果输入字符串等于"Low"，则返回EQualityLevel::Low 
  if (SQualitySettingsLevel.Equals("Low"))
  {
    return EQualityLevel::Low;
  }
  // 如果输入字符串等于"Epic"，则返回EQualityLevel::Epic
  if (SQualitySettingsLevel.Equals("Epic"))
  {
    return EQualityLevel::Epic;
  }
  // 如果字符串不匹配"Low"或"Epic"，则返回传入的默认值
  return Default;
}
// 定义一个函数，用于将质量等级枚举值转换为字符串  
// 参数：  
// - QualitySettingsLevel：一个EQualityLevel类型的质量等级枚举值 
FString QualityLevelToString(EQualityLevel QualitySettingsLevel)
{
   // 尝试在项目的任何包中找到名为"EQualityLevel"的枚举类型对象
  const UEnum *ptr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EQualityLevel"), true);
  // 如果未找到该枚举类型对象，则返回"Invalid"字符串
  if (!ptr)
  {
    return FString("Invalid");
  }
  // 返回与传入的质量等级枚举值对应的枚举项名称字符串  
  // 这里使用static_cast<int32>将枚举值转换为索引值
  return ptr->GetNameStringByIndex(static_cast<int32>(QualitySettingsLevel));
}

static void LoadSettingsFromConfig(
    const FIniFile &ConfigFile,
    UCarlaSettings &Settings,
    const bool bLoadCarlaServerSection)
{
  // CarlaServer.
  if (bLoadCarlaServerSection)
  {
    ConfigFile.GetInt(S_CARLA_SERVER,    TEXT("WorldPort"), Settings.RPCPort);
    ConfigFile.GetInt(S_CARLA_SERVER,    TEXT("RPCPort"), Settings.RPCPort);
    Settings.StreamingPort = Settings.RPCPort + 1u;
    Settings.SecondaryPort = Settings.RPCPort + 2u;
    ConfigFile.GetInt(S_CARLA_SERVER,    TEXT("StreamingPort"), Settings.StreamingPort);
    ConfigFile.GetInt(S_CARLA_SERVER,    TEXT("SecondaryPort"), Settings.SecondaryPort);
    FString Tmp;
    ConfigFile.GetString(S_CARLA_SERVER, TEXT("PrimaryIP"), Tmp);
    Settings.PrimaryIP = TCHAR_TO_UTF8(*Tmp);
    ConfigFile.GetInt(S_CARLA_SERVER,    TEXT("PrimaryPort"), Settings.PrimaryPort);
  }
  ConfigFile.GetBool(S_CARLA_SERVER, TEXT("SynchronousMode"), Settings.bSynchronousMode);
  ConfigFile.GetBool(S_CARLA_SERVER, TEXT("DisableRendering"), Settings.bDisableRendering);
  // 画质配置 QualitySettings.
  FString sQualityLevel;
  ConfigFile.GetString(S_CARLA_QUALITYSETTINGS, TEXT("QualityLevel"), sQualityLevel);
  Settings.SetQualityLevel(QualityLevelFromString(sQualityLevel));
}

static bool GetSettingsFilePathFromCommandLine(FString &Value)
{
  if (FParse::Value(FCommandLine::Get(), TEXT("-carla-settings="), Value))
  {
    if (FPaths::IsRelative(Value))
    {
      Value = FPaths::ConvertRelativePathToFull(FPaths::LaunchDir(), Value);
    }
    return true;
  }
  return false;
}

// =============================================================================
// -- UCarlaSettings -----------------------------------------------------------
// =============================================================================

void UCarlaSettings::LoadSettings()
{
  CurrentFileName = TEXT("");
  // 如果存在，则从项目配置文件夹加载设置。
  LoadSettingsFromFile(FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("CarlaSettings.ini")), false);
  // 如果提供，则加载命令行参数给出的设置。
  {
    FString FilePath;
    if (GetSettingsFilePathFromCommandLine(FilePath))
    {
      LoadSettingsFromFile(FilePath, true);
    }
  }
  // 从命令行覆盖设置。
  {
    uint32 Value;
    if (FParse::Value(FCommandLine::Get(), TEXT("-world-port="), Value) ||
        FParse::Value(FCommandLine::Get(), TEXT("-carla-port="), Value) ||
        FParse::Value(FCommandLine::Get(), TEXT("-carla-rpc-port="), Value) ||
        FParse::Value(FCommandLine::Get(), TEXT("-carla-world-port="), Value))
    {
      // 默认情况下，流媒体服务的端口是RPC端口+1，辅助服务器端口是RPC端口+2
      RPCPort = Value;
      StreamingPort = Value + 1u;
      SecondaryPort = Value + 2u;
    }
    if (FParse::Value(FCommandLine::Get(), TEXT("-carla-streaming-port="), Value))
    {
      StreamingPort = Value;
    }
    if (FParse::Value(FCommandLine::Get(), TEXT("-carla-secondary-port="), Value))
    {
      SecondaryPort = Value;
    }
    FString Tmp;
    if (FParse::Value(FCommandLine::Get(), TEXT("-carla-primary-host="), Tmp))
    {
      PrimaryIP = TCHAR_TO_UTF8(*Tmp);
    }
    if (FParse::Value(FCommandLine::Get(), TEXT("-carla-primary-port="), Value))
    {
      PrimaryPort = Value;
    }
    FString StringQualityLevel;
    if (FParse::Value(FCommandLine::Get(), TEXT("-quality-level="), StringQualityLevel))
    {
      QualityLevel = QualityLevelFromString(StringQualityLevel, EQualityLevel::Epic);
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("-no-rendering")))
    {
      bDisableRendering = true;
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("-ros2")))
    {
      ROS2 = true;
    }
  }
}

void UCarlaSettings::LoadSettingsFromString(const FString &INIFileContents)
{
  UE_LOG(LogCarla, Log, TEXT("Loading CARLA settings from string"));
  FIniFile ConfigFile;
  ConfigFile.ProcessInputFileContents(INIFileContents);
  constexpr bool bLoadCarlaServerSection = false;
  LoadSettingsFromConfig(ConfigFile, *this, bLoadCarlaServerSection);
  CurrentFileName = TEXT("<string-provided-by-client>");
}

void UCarlaSettings::LogSettings() const
{
  auto EnabledDisabled = [](bool bValue) { return (bValue ? TEXT("Enabled") : TEXT("Disabled")); };
  UE_LOG(LogCarla, Log,
      TEXT("== CARLA Settings =============================================================="));
  UE_LOG(LogCarla, Log, TEXT("Last settings file loaded: %s"), *CurrentFileName);
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_SERVER);
  UE_LOG(LogCarla, Log, TEXT("RPC Port = %d"), RPCPort);
  UE_LOG(LogCarla, Log, TEXT("Streaming Port = %d"), StreamingPort);
  UE_LOG(LogCarla, Log, TEXT("Secondary Port = %d"), SecondaryPort);
  UE_LOG(LogCarla, Log, TEXT("Synchronous Mode = %s"), EnabledDisabled(bSynchronousMode));
  UE_LOG(LogCarla, Log, TEXT("Rendering = %s"), EnabledDisabled(!bDisableRendering));
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_QUALITYSETTINGS);
  UE_LOG(LogCarla, Log, TEXT("Quality Level = %s"), *QualityLevelToString(QualityLevel));
  UE_LOG(LogCarla, Log,
      TEXT("================================================================================"));
}

#undef S_CARLA_QUALITYSETTINGS
#undef S_CARLA_SERVER

void UCarlaSettings::LoadSettingsFromFile(const FString &FilePath, const bool bLogOnFailure)
{
  if (FPaths::FileExists(FilePath))
  {
    UE_LOG(LogCarla, Log, TEXT("Loading CARLA settings from \"%s\""), *FilePath);
    const FIniFile ConfigFile(FilePath);
    constexpr bool bLoadCarlaServerSection = true;
    LoadSettingsFromConfig(ConfigFile, *this, bLoadCarlaServerSection);
    CurrentFileName = FilePath;
  }
  else if (bLogOnFailure)
  {
    UE_LOG(LogCarla, Error, TEXT("Unable to find settings file \"%s\""), *FilePath);
  }
}
