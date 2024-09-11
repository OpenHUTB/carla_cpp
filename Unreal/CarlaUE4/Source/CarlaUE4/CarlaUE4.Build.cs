// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class CarlaUE4 : ModuleRules
{
	public CarlaUE4(ReadOnlyTargetRules Target) : base(Target)
	{
    PrivatePCHHeaderFile = "CarlaUE4.h";

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		if (Target.Type == TargetType.Editor)
        {
            PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
        }

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// 如果您正在使用Slate UI，请取消注释。
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// 如果您正在使用在线功能，请取消注释
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// 如果要包含 OnlineSubsystemSteam，请将其添加到uproject文件中的插件部分，并将Enabled属性设置为true
	}
}
