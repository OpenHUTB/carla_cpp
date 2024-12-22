// Copyright (c) Your Company/Organization Name.
// Licensed under [License Name]. See LICENSE file in the project root for full license information.

using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;

/// <summary>
/// CarlaUE4的构建目标配置类，用于定义项目的构建规则和属性
/// </summary>
public class CarlaUE4Target : TargetRules
{
    // 配置文件相关常量
    private const string CONFIG_FOLDER = "Config";
    private const string OPTIONAL_MODULES_FILE = "OptionalModules.ini";
    private const string UNITY_OFF_FLAG = "Unity OFF";

    /// <summary>
    /// 构建目标配置的构造函数
    /// </summary>
    /// <param name="Target">包含目标平台等信息的对象</param>
    public CarlaUE4Target(TargetInfo Target) : base(Target)
    {
        InitializeTargetSettings();
        ConfigureUnityBuildSettings();
    }

    /// <summary>
    /// 初始化基本目标设置
    /// </summary>
    private void InitializeTargetSettings()
    {
        // 设置为游戏类型构建目标
        Type = TargetType.Game;
        
        // 添加核心模块
        ExtraModuleNames.Add("CarlaUE4");
    }

    /// <summary>
    /// 配置Unity构建设置
    /// </summary>
    private void ConfigureUnityBuildSettings()
    {
        string configFilePath = GetOptionalModulesFilePath();
        
        // 如果配置文件指示禁用Unity构建，则更新相关设置
        if (ShouldDisableUnityBuild(configFilePath))
        {
            DisableUnityBuild();
        }
    }

    /// <summary>
    /// 获取可选模块配置文件的完整路径
    /// </summary>
    private string GetOptionalModulesFilePath()
    {
        string configDir = Path.Combine(
            Path.GetDirectoryName(ProjectFile.ToString()),
            CONFIG_FOLDER
        );
        
        return Path.Combine(configDir, OPTIONAL_MODULES_FILE);
    }

    /// <summary>
    /// 检查是否应该禁用Unity构建
    /// </summary>
    /// <param name="configFilePath">配置文件路径</param>
    /// <returns>是否应该禁用Unity构建</returns>
    private bool ShouldDisableUnityBuild(string configFilePath)
    {
        try
        {
            // 读取并检查配置文件
            string[] configLines = File.ReadAllLines(configFilePath);
            return configLines.Any(line => line.Contains(UNITY_OFF_FLAG));
        }
        catch (Exception ex)
        {
            // 记录错误但继续执行（保持Unity构建启用）
            Console.WriteLine($"Error reading config file: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// 禁用所有Unity构建相关设置
    /// </summary>
    private void DisableUnityBuild()
    {
        Console.WriteLine("Disabling Unity build settings");
        
        bUseUnityBuild = false;
        bForceUnityBuild = false;
        bUseAdaptiveUnityBuild = false;
    }
}
