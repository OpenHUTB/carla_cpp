// 在项目设置的描述页面填写你的版权声明

// 引入必要的命名空间
using UnrealBuildTool;// 用于构建Unreal Engine项目
using System.Collections.Generic;
using System;
using System.IO;

// 定义一个用于构建目标配置的类
public class CarlaUE4Target : TargetRules
{
	// CarlaUE4Target类的构造函数
	public CarlaUE4Target(TargetInfo Target) : base(Target)
	{
 
 		// 设置构建配置的目标类型为'Game'
		Type = TargetType.Game;
		ExtraModuleNames.Add("CarlaUE4");
  
    // 根据项目文件的位置确定配置目录路径
    string ConfigDir = Path.GetDirectoryName(ProjectFile.ToString()) + "/Config/";
    string OptionalModulesFile = Path.Combine(ConfigDir, "OptionalModules.ini");
    string[] text = System.IO.File.ReadAllLines(OptionalModulesFile);

    // 初始化一个布尔标志来检查是否启用Unity构建
    bool UnityOn = true;

    // 遍历配置文件中的每一行
    foreach (string line in text) {
      if (line.Contains("Unity OFF"))
      {
        UnityOn = false;
      }
    }

    if (!UnityOn) {
      // 输出消息到控制台，表示将禁用Unity构建
      Console.WriteLine("Disabling unity");
      // 禁用Unity构建配置
      bUseUnityBuild = false;
      bForceUnityBuild = false;
      bUseAdaptiveUnityBuild = false;
    }
	}
}
