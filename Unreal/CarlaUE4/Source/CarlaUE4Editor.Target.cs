// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;// ����Unreal Build Tool�������ռ䣬�Ա�ʹ��UBT����͹���
using System.Collections.Generic;// ���뼯�ϵ������ռ䣬���ڲ������ϣ����б�
using System;// �������ϵͳ�����ռ䣬�ṩ������ͷ���  
using System.IO; // ����������������ռ䣬�ṩ�ļ������Ĺ���
// ����һ���̳���TargetRules����CarlaUE4EditorTarget����������CARLA UE4�༭���汾�Ĺ�������
public class CarlaUE4EditorTarget : TargetRules
{// ���캯��������һ��TargetInfo������Ϊ�����������û��๹�캯��
    public CarlaUE4EditorTarget(TargetInfo Target) : base(Target)
    {// ����Ŀ������Ϊ�༭�� 
        Type = TargetType.Editor;// ��CARLAUE4ģ����ӵ�����ģ���б��У���ʾ�������Ŀ����ҪCARLAUE4ģ�� 
        ExtraModuleNames.Add("CarlaUE4");
        // ���������ļ�Ŀ¼��·��
        string ConfigDir = Path.GetDirectoryName(ProjectFile.ToString()) + "/Config/";// ����OptionalModules.ini�ļ�������·�� 
        string OptionalModulesFile = Path.Combine(ConfigDir, "OptionalModules.ini");// ��ȡOptionalModules.ini�ļ��������е��ַ���������
        string[] text = System.IO.File.ReadAllLines(OptionalModulesFile);
        // ����Unity�����ǿ�����
        bool UnityOn = true;
        // ����OptionalModules.ini�ļ���ÿһ��
        foreach (string line in text)
        {// ���ĳ�а���"Unity OFF"����UnityOn����Ϊfalse
            if (line.Contains("Unity OFF"))
			{
				UnityOn = false;
			}
		}
        // ���UnityOnΪfalse����OptionalModules.ini��ָ���˹ر�Unity����
        if (!UnityOn)
        {// ��ӡ��־��Ϣ����ʾ���ڽ���Unity����
            Console.WriteLine("Disabling unity");
            // ����Unity������ص�ѡ��  
            bUseUnityBuild = false;// ��ʹ��Unity����
            bForceUnityBuild = false;// ��ǿ��ʹ��Unity����
            bUseAdaptiveUnityBuild = false;// ��ʹ������ӦUnity���� 
        }
	}
}
