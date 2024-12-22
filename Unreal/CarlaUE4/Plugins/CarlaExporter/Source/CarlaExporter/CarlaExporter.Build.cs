// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

//使用UnrealBuildTool构建系统
using UnrealBuildTool;

//定义CarlaExporter类，继承自ModuleRules
public class CarlaExporter : ModuleRules
{
  // 构造函数，接收一个 ReadOnlyTargetRules 类型的参数 Target，并调用基类的构造函数
  public CarlaExporter(ReadOnlyTargetRules Target) : base(Target)
  {
    // 设置预编译头文件的使用模式为 UseExplicitOrSharedPCHs
    PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

    // 向公共包含路径列表中添加元素
    PublicIncludePaths.AddRange(
      new string[] {
        //... add public include paths required here...
        // 在这里添加所需的公共包含路径
      }
      );


    // 向私有包含路径列表中添加元素
    PrivateIncludePaths.AddRange(
      new string[] {
        //... add other private include paths required here...
        // 在这里添加所需的其他私有包含路径
      }
    );


    // 向公共依赖模块名称列表中添加元素
    PublicDependencyModuleNames.AddRange(
      new string[]
      {
        "Core",
        //... add other public dependencies that you statically link with here...
        // 在这里添加其他需要静态链接的公共依赖模块
      }
    );


    // 向私有依赖模块名称列表中添加元素
    PrivateDependencyModuleNames.AddRange(
      new string[]
      {
        "Projects",
        "InputCore",
        "UnrealEd",
        "LevelEditor",
        "CoreUObject",
        "Engine",
        "Slate",
        "SlateCore",
        "Physx",
        "EditorStyle",
        //添加模块名称
        //... add private dependencies that you statically link with here...
        // 在这里添加其他需要静态链接的私有依赖模块
      }
    );


    // 向动态加载模块名称列表中添加元素
    DynamicallyLoadedModuleNames.AddRange(
      new string[]
      {
        //... add any modules that your module loads dynamically here...
        // 在这里添加此模块需要动态加载的任何模块
      }
    );
  }
}
