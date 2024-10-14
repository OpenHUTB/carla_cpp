// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Carla.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "Developer/Settings/Public/ISettingsSection.h"
#include "Developer/Settings/Public/ISettingsContainer.h"
#include "Settings/CarlaSettings.h"

#define LOCTEXT_NAMESPACE "FCarlaModule"

DEFINE_LOG_CATEGORY(LogCarla);
DEFINE_LOG_CATEGORY(LogCarlaServer);

void FCarlaModule::StartupModule()
{
	RegisterSettings();
	LoadChronoDll();
}

void FCarlaModule::LoadChronoDll()
{
	#if defined(WITH_CHRONO) && PLATFORM_WINDOWS
	const FString BaseDir = FPaths::Combine(*FPaths::ProjectPluginsDir(), TEXT("Carla"));
	const FString DllDir = FPaths::Combine(*BaseDir, TEXT("CarlaDependencies"), TEXT("dll"));
	FString ChronoEngineDll = FPaths::Combine(*DllDir, TEXT("ChronoEngine.dll"));
	FString ChronoVehicleDll = FPaths::Combine(*DllDir, TEXT("ChronoEngine_vehicle.dll"));
	FString ChronoModelsDll = FPaths::Combine(*DllDir, TEXT("ChronoModels_vehicle.dll"));
	FString ChronoRobotDll = FPaths::Combine(*DllDir, TEXT("ChronoModels_robot.dll"));
	UE_LOG(LogCarla, Log, TEXT("Loading Dlls from: %s"), *DllDir);
	auto ChronoEngineHandle = FPlatformProcess::GetDllHandle(*ChronoEngineDll);
	if (!ChronoEngineHandle)
	{
		UE_LOG(LogCarla, Warning, TEXT("Error: ChronoEngine.dll could not be loaded"));
	}
	auto ChronoVehicleHandle = FPlatformProcess::GetDllHandle(*ChronoVehicleDll);
	if (!ChronoVehicleHandle)
	{
		UE_LOG(LogCarla, Warning, TEXT("Error: ChronoEngine_vehicle.dll could not be loaded"));
	}
	auto ChronoModelsHandle = FPlatformProcess::GetDllHandle(*ChronoModelsDll);
	if (!ChronoModelsHandle)
	{
		UE_LOG(LogCarla, Warning, TEXT("Error: ChronoModels_vehicle.dll could not be loaded"));
	}
	auto ChronoRobotHandle = FPlatformProcess::GetDllHandle(*ChronoRobotDll);
	if (!ChronoRobotHandle)
	{
		UE_LOG(LogCarla, Warning, TEXT("Error: ChronoModels_robot.dll could not be loaded"));
	}
	#endif
}

void FCarlaModule::ShutdownModule()
{
	if (UObjectInitialized())
	{
		UnregisterSettings();
	}
}

void FCarlaModule::RegisterSettings()
{
	// Registering some settings is just a matter of exposing the default UObject of
	// your desired class, add here all those settings you want to expose
	// to your LDs or artists.

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		// Create the new category
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

		SettingsContainer->DescribeCategory("CARLASettings",
			LOCTEXT("RuntimeWDCategoryName", "CARLA Settings"),
			LOCTEXT("RuntimeWDCategoryDescription", "CARLA plugin settings"));

		// Register the settings
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "CARLASettings", "General",
			LOCTEXT("RuntimeGeneralSettingsName", "General"),
			LOCTEXT("RuntimeGeneralSettingsDescription", "General configuration for the CARLA plugin"),
			GetMutableDefault<UCarlaSettings>()
		);

		// Register the save handler to your settings, you might want to use it to
		// validate those or just act to settings changes.
		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FCarlaModule::HandleSettingsSaved);
		}
	}
}

void FCarlaModule::UnregisterSettings()
{
	// Ensure to unregister all of your registered settings here, hot-reload would
	// otherwise yield unexpected results.

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "CustomSettings", "General");
	}
}

bool FCarlaModule::HandleSettingsSaved()
{
	UCarlaSettings* Settings = GetMutableDefault<UCarlaSettings>();
	bool ResaveSettings = false;

	// Put any validation code in here and resave the settings in case an invalid
	// value has been entered

	if (ResaveSettings)
	{
		Settings->SaveConfig();
	}

	return true;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCarlaModule, Carla)

// =============================================================================
// -- Implement carla throw_exception ------------------------------------------
// =============================================================================
// 引入一个头文件，用于禁用Unreal Engine 4（UE4）的宏定义，以避免可能的宏冲突
#include <compiler/disable-ue4-macros.h>
// 引入Carla库中定义的异常处理相关的头文件
#include <carla/Exception.h>
//引入另一个头文件，用于重新启用之前被禁用的UE4宏定义
#include <compiler/enable-ue4-macros.h>
// 引入C++标准库中的异常处理头文件
#include <exception>
// 定义一个命名空间carla，用于封装Carla相关的代码
namespace carla {
//定义一个函数，用于抛出并处理异常
  void throw_exception(const std::exception &e) {
    UE_LOG(LogCarla, Fatal, TEXT("Exception thrown: %s"), UTF8_TO_TCHAR(e.what()));
	// 调用std::terminate()函数立即终止程序
	// It should never reach this part.
    std::terminate();
  }

} // namespace carla
