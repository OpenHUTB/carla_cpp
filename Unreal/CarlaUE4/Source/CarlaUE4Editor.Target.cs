// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;// 引入Unreal Build Tool的命名空间，以便使用UBT的类和功能
using System.Collections.Generic;// 引入集合的命名空间，用于操作集合（如列表）
using System;// 引入基础系统命名空间，提供基础类和方法  
using System.IO; // 引入输入输出命名空间，提供文件操作的功能
// 定义一个继承自TargetRules的类CarlaUE4EditorTarget，用于设置CARLA UE4编辑器版本的构建规则
public class CarlaUE4EditorTarget : TargetRules
{// 构造函数，接收一个TargetInfo对象作为参数，并调用基类构造函数
    public CarlaUE4EditorTarget(TargetInfo Target) : base(Target)
    {// 设置目标类型为编辑器 
        Type = TargetType.Editor;// 将CARLAUE4模块添加到额外模块列表中，表示这个构建目标需要CARLAUE4模块 
        ExtraModuleNames.Add("CarlaUE4");
        // 计算配置文件目录的路径
        string ConfigDir = Path.GetDirectoryName(ProjectFile.ToString()) + "/Config/";// 计算OptionalModules.ini文件的完整路径 
        string OptionalModulesFile = Path.Combine(ConfigDir, "OptionalModules.ini");// 读取OptionalModules.ini文件的所有行到字符串数组中
        string[] text = System.IO.File.ReadAllLines(OptionalModulesFile);
        // 假设Unity构建是开启的
        bool UnityOn = true;
        // 遍历OptionalModules.ini文件的每一行
        foreach (string line in text)
        {// 如果某行包含"Unity OFF"，则将UnityOn设置为false
            if (line.Contains("Unity OFF"))
			{
				UnityOn = false;
			}
		}
        // 如果UnityOn为false，即OptionalModules.ini中指定了关闭Unity构建
        if (!UnityOn)
        {// 打印日志信息，表示正在禁用Unity构建
            Console.WriteLine("Disabling unity");
            // 禁用Unity构建相关的选项  
            bUseUnityBuild = false;// 不使用Unity构建
            bForceUnityBuild = false;// 不强制使用Unity构建
            bUseAdaptiveUnityBuild = false;// 不使用自适应Unity构建 
        }
	}
}
