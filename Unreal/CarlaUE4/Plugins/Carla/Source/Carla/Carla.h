// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

// This file is included before any other file in every compile unit within the
// plugin.
#pragma once // ��ֹͷ�ļ�����ΰ���

#include "Util/NonCopyable.h" // ����ǿ���able��
#include "Logging/LogMacros.h" // ������־�궨��
#include "Modules/ModuleInterface.h" // ����ģ��ӿڶ���

// ������־�������Carla����־��¼
DECLARE_LOG_CATEGORY_EXTERN(LogCarla, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCarlaServer, Log, All); // ����Carla��������־���

// ����ͳ���飬����Carla������ͳ��
DECLARE_STATS_GROUP(TEXT("CarlaSensor"), STATGROUP_CarlaSensor, STATCAT_Advanced);

// ����ѭ��ͳ�ƣ���¼��ͬ��������������
DECLARE_CYCLE_STAT(TEXT("Read RT"), STAT_CarlaSensorReadRT, STATGROUP_CarlaSensor); // ��ȡʵʱ���ݵ�ͳ��
DECLARE_CYCLE_STAT(TEXT("Copy Text"), STAT_CarlaSensorCopyText, STATGROUP_CarlaSensor); // �ı����Ƶ�ͳ��
DECLARE_CYCLE_STAT(TEXT("Buffer Copy"), STAT_CarlaSensorBufferCopy, STATGROUP_CarlaSensor); // ���������Ƶ�ͳ��
DECLARE_CYCLE_STAT(TEXT("Stream Send"), STAT_CarlaSensorStreamSend, STATGROUP_CarlaSensor); // ���������͵�ͳ��

// ����ʱ���ö��������־��ѡ��
#if WITH_EDITOR
// ����AI������־
// ����AI������־
// ��·������������־
// ������������־
// ��ǩ��������־
// ������صĶ�����־
#endif // WITH_EDITOR

// ����FCarlaModule�࣬�̳���IModuleInterface
class FCarlaModule : public IModuleInterface
{
	void RegisterSettings(); // ע�����õķ���
	void UnregisterSettings(); // ȡ��ע�����õķ���
	bool HandleSettingsSaved(); // �������ñ���ķ���
	void LoadChronoDll(); // ����Chrono DLL�ķ���

public:

	/** IModuleInterfaceʵ�� */
	virtual void StartupModule() override; // ����ģ���ʵ��
	virtual void ShutdownModule() override; // �ر�ģ���ʵ��

};
