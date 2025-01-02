// 版权所有 1998-2017 Epic Games, Inc. 保留所有权利

#include "Carla.h"
#include "Developer/Settings/Public/ISettingsModule.h" // 包含用于管理插件设置的模块接口
#include "Developer/Settings/Public/ISettingsSection.h" // 包含设置部分的接口
#include "Developer/Settings/Public/ISettingsContainer.h" // 包含设置容器的接口
#include "Settings/CarlaSettings.h" // 包含CARLA设置的具体实现

#define LOCTEXT_NAMESPACE "FCarlaModule" // 定义本地化文本命名空间

// 定义两个日志类别：用于一般日志和服务器相关日志
DEFINE_LOG_CATEGORY(LogCarla);
DEFINE_LOG_CATEGORY(LogCarlaServer);

// 模块启动函数
void FCarlaModule::StartupModule()
{
	RegisterSettings(); // 注册插件设置
	LoadChronoDll();    // 加载Chrono动态链接库（DLL）
}

// 加载Chrono动态链接库的实现
void FCarlaModule::LoadChronoDll()
{
	#if defined(WITH_CHRONO) && PLATFORM_WINDOWS // 仅在Windows平台且启用了Chrono库时执行
	const FString BaseDir = FPaths::Combine(*FPaths::ProjectPluginsDir(), TEXT("Carla")); // 获取插件的基础目录
	const FString DllDir = FPaths::Combine(*BaseDir, TEXT("CarlaDependencies"), TEXT("dll")); // 拼接DLL所在目录路径
	FString ChronoEngineDll = FPaths::Combine(*DllDir, TEXT("ChronoEngine.dll")); // Chrono引擎核心DLL
	FString ChronoVehicleDll = FPaths::Combine(*DllDir, TEXT("ChronoEngine_vehicle.dll")); // Chrono车辆模块DLL
	FString ChronoModelsDll = FPaths::Combine(*DllDir, TEXT("ChronoModels_vehicle.dll")); // Chrono模型模块DLL
	FString ChronoRobotDll = FPaths::Combine(*DllDir, TEXT("ChronoModels_robot.dll")); // Chrono机器人模块DLL
	UE_LOG(LogCarla, Log, TEXT("Loading Dlls from: %s"), *DllDir); // 记录加载路径

	// 加载DLL并检查是否成功
	auto ChronoEngineHandle = FPlatformProcess::GetDllHandle(*ChronoEngineDll);
	if (!ChronoEngineHandle)
	{
		UE_LOG(LogCarla, Warning, TEXT("Error: ChronoEngine.dll could not be loaded")); // 提示加载失败
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

// 模块关闭函数
void FCarlaModule::ShutdownModule()
{
	if (UObjectInitialized()) // 确保对象系统已经初始化
	{
		UnregisterSettings(); // 注销插件设置
	}
}

// 注册设置
void FCarlaModule::RegisterSettings()
{
	// 注册插件设置，以便开发者或艺术家在编辑器中调整
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		// 创建一个新设置类别
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

		SettingsContainer->DescribeCategory("CARLASettings",
			LOCTEXT("RuntimeWDCategoryName", "CARLA Settings"), // 设置类别名称
			LOCTEXT("RuntimeWDCategoryDescription", "CARLA plugin settings")); // 设置类别描述

		// 注册通用设置部分
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "CARLASettings", "General",
			LOCTEXT("RuntimeGeneralSettingsName", "General"), // 设置部分名称
			LOCTEXT("RuntimeGeneralSettingsDescription", "General configuration for the CARLA plugin"), // 设置部分描述
			GetMutableDefault<UCarlaSettings>() // 获取CARLA设置的默认对象
		);

		// 注册保存处理函数，用于验证设置或对修改后的设置执行操作
		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FCarlaModule::HandleSettingsSaved);
		}
	}
}

// 注销设置
void FCarlaModule::UnregisterSettings()
{
	// 确保注销所有已注册的设置，避免热重载导致的意外行为
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "CustomSettings", "General");
	}
}

// 处理设置保存逻辑
bool FCarlaModule::HandleSettingsSaved()
{
	UCarlaSettings* Settings = GetMutableDefault<UCarlaSettings>();
	bool ResaveSettings = false; // 标记是否需要重新保存设置

	// 验证设置值，并在无效时重新保存设置
	if (ResaveSettings)
	{
		Settings->SaveConfig(); // 保存配置
	}

	return true; // 返回保存成功状态
}

#undef LOCTEXT_NAMESPACE // 取消定义本地化文本命名空间

// 实现模块的加载入口
IMPLEMENT_MODULE(FCarlaModule, Carla)

// =============================================================================
// -- 实现carla异常处理 --------------------------------------------------------
// =============================================================================

// 引入一个头文件，用于禁用Unreal Engine 4（UE4）的宏定义，避免宏冲突
#include <compiler/disable-ue4-macros.h>
// 引入Carla库中定义的异常处理相关头文件
#include <carla/Exception.h>
// 引入另一个头文件，用于重新启用之前被禁用的UE4宏定义
#include <compiler/enable-ue4-macros.h>
// 引入C++标准库中的异常处理头文件
#include <exception>

// 定义命名空间carla，封装Carla相关的代码
namespace carla {

// 定义一个函数，用于抛出并处理异常
  void throw_exception(const std::exception &e) {
    // 使用UE_LOG记录异常信息，并以Fatal级别终止程序
    UE_LOG(LogCarla, Fatal, TEXT("Exception thrown: %s"), UTF8_TO_TCHAR(e.what()));
	// 调用std::terminate()立即终止程序
	// 理论上程序不应该到达这里
    std::terminate();
  }

} // 命名空间carla结束
