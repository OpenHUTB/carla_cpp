// 以下是针对提供的代码段的详细注释，解释了每个部分的功能和目的：

// 在项目的描述页面中，应准确填写版权相关的声明信息，以明确项目的版权归属。
// 注意：此注释通常位于项目文档或特定文件中，而不是源代码文件内。

// 引入必要的命名空间，这些命名空间提供了构建Unreal Engine项目所需的类型、方法和工具。
using UnrealBuildTool; // 提供用于构建Unreal Engine项目的工具、类型和方法。
using System.Collections.Generic; // 提供集合类的支持，如List<T>等。
using System; // 提供基础系统类和方法，如字符串、日期等。
using System.IO; // 提供文件操作的功能，如读取、写入文件等。

// 定义一个名为CarlaUE4Target的类，它继承自TargetRules类。
// 通过自定义此类，可以配置与CarlaUE4项目相关的构建规则和属性。
public class CarlaUE4Target : TargetRules
{
    // CarlaUE4Target类的构造函数，用于初始化构建目标的相关配置。
    // Target参数包含了目标平台等信息，通过调用基类的构造函数进行初始化。
    public CarlaUE4Target(TargetInfo Target) : base(Target)
    {
        // 设置构建目标类型为Game，表示此构建目标是用于生成可运行的游戏项目。
        Type = TargetType.Game;
        
        // 将"CarlaUE4"模块添加到构建的额外模块列表中，该模块包含项目特有的功能代码和资源。
        ExtraModuleNames.Add("CarlaUE4");
        
        // 根据项目文件的位置确定配置目录的路径。
        // 首先获取项目文件所在的目录，然后在其后添加"/Config/"以得到配置文件目录。
        string ConfigDir = Path.GetDirectoryName(ProjectFile.ToString()) + "/Config/";
        
        // 生成OptionalModules.ini配置文件的完整路径。
        // 该配置文件包含可选模块的相关配置信息。
        string OptionalModulesFile = Path.Combine(ConfigDir, "OptionalModules.ini");
        
        // 读取OptionalModules.ini文件的所有行，并将其存储到字符串数组中。
        string[] text = System.IO.File.ReadAllLines(OptionalModulesFile);
        
        // 初始化一个布尔变量UnityOn，用于检查是否启用Unity构建。
        // 初始化为true表示默认情况下启用Unity构建。
        bool UnityOn = true;
        
        // 遍历配置文件的每一行，检查是否存在"Unity OFF"字符串。
        // 如果存在，则将UnityOn设置为false，表示禁用Unity构建。
        foreach (string line in text)
        {
            if (line.Contains("Unity OFF"))
            {
                UnityOn = false;
            }
        }
        
        // 如果UnityOn为false，即配置文件中指定了关闭Unity构建，
        // 则执行以下操作来禁用Unity构建配置。
        if (!UnityOn)
        {
            // 输出消息到控制台，提示用户当前将禁用Unity构建。
            Console.WriteLine("Disabling unity");
            
            // 禁用Unity构建相关的配置属性。
            // 这些属性控制Unity构建的行为，将它们设置为false表示禁用Unity构建。
            bUseUnityBuild = false; // 禁用Unity构建。
            bForceUnityBuild = false; // 强制禁用Unity构建（通常不需要，除非有特殊需求）。
            bUseAdaptiveUnityBuild = false; // 禁用自适应Unity构建（根据模块大小自动选择是否使用Unity构建）。
        }
    }
}

// 注意：
// 1. 上述代码中的bUseUnityBuild、bForceUnityBuild和bUseAdaptiveUnityBuild属性
//    是TargetRules类或其基类中定义的，用于控制Unity构建的行为。
//    在Unreal Engine的上下文中，Unity构建是一种将多个模块合并到一个动态链接库(DLL)中的技术，
//    以减少加载时间和内存占用。但是，在某些情况下（如模块间存在不兼容的依赖项时），
//    可能需要禁用Unity构建。
// 2. ProjectFile属性是TargetRules类或其基类中定义的，通常用于表示当前项目的项目文件路径。
//    在这个上下文中，它被用来确定配置文件的路径。
// 3. 这段代码是特定于CarlaUE4项目的构建配置，Carla是一个开源的自动驾驶模拟器，
//    它基于Unreal Engine构建，用于测试和开发自动驾驶系统。
