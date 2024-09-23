// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

// This file is included before any other file in every compile unit within the
// plugin.
#pragma once // 防止头文件被多次包含

#include "Util/NonCopyable.h" // 引入非拷贝able类
#include "Logging/LogMacros.h" // 引入日志宏定义
#include "Modules/ModuleInterface.h" // 引入模块接口定义

// 声明日志类别，用于Carla的日志记录
DECLARE_LOG_CATEGORY_EXTERN(LogCarla, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCarlaServer, Log, All); // 声明Carla服务器日志类别

// 声明统计组，用于Carla传感器统计
DECLARE_STATS_GROUP(TEXT("CarlaSensor"), STATGROUP_CarlaSensor, STATCAT_Advanced);

// 声明循环统计，记录不同操作的性能数据
DECLARE_CYCLE_STAT(TEXT("Read RT"), STAT_CarlaSensorReadRT, STATGROUP_CarlaSensor); // 读取实时数据的统计
DECLARE_CYCLE_STAT(TEXT("Copy Text"), STAT_CarlaSensorCopyText, STATGROUP_CarlaSensor); // 文本复制的统计
DECLARE_CYCLE_STAT(TEXT("Buffer Copy"), STAT_CarlaSensorBufferCopy, STATGROUP_CarlaSensor); // 缓冲区复制的统计
DECLARE_CYCLE_STAT(TEXT("Stream Send"), STAT_CarlaSensorStreamSend, STATGROUP_CarlaSensor); // 数据流发送的统计

// 编译时启用额外调试日志的选项
#if WITH_EDITOR
// 车辆AI额外日志
// 行人AI额外日志
// 道路生成器额外日志
// 服务器额外日志
// 标签器额外日志
// 天气相关的额外日志
#endif // WITH_EDITOR

// 定义FCarlaModule类，继承自IModuleInterface
class FCarlaModule : public IModuleInterface
{
	void RegisterSettings(); // 注册设置的方法
	void UnregisterSettings(); // 取消注册设置的方法
	bool HandleSettingsSaved(); // 处理设置保存的方法
	void LoadChronoDll(); // 加载Chrono DLL的方法

public:

	/** IModuleInterface实现 */
	virtual void StartupModule() override; // 启动模块的实现
	virtual void ShutdownModule() override; // 关闭模块的实现

};
