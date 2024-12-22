// 在项目设置的描述页面填写你的版权声明，此处应按照相关要求及规范，准确填写版权相关的声明信息，以表明项目的版权归属等情况。

// 引入必要的命名空间
using UnrealBuildTool; // 此命名空间提供了用于构建Unreal Engine项目的各类工具、类型以及方法等，是整个UE4项目构建流程实现的基础，借助它可以完成如模块配置、目标类型设定等构建相关操作。
using System.Collections.Generic;// 引入集合类的命名空间，用于操作集合，如List<T>等
using System;// 引入基础系统命名空间，提供基础类和方法，如字符串、日期等操作
using System.IO;// 引入输入输出命名空间，提供文件操作的功能，如文件读取、写入等

// 定义一个用于构建目标配置的类，继承自TargetRules，通过自定义此类来配置特定项目（这里大概率是和CarlaUE4相关项目）在构建过程中的各项规则和属性。
public class CarlaUE4Target : TargetRules
{
    // CarlaUE4Target类的构造函数，在创建该类实例时会执行此方法，用于初始化构建目标的相关配置。
    // 参数Target包含了目标平台等相关信息，通过调用基类（TargetRules）的构造函数，将这些信息传递给基类进行必要的初始化操作，然后在此基础上进一步配置本类特有的属性。
    public CarlaUE4Target(TargetInfo Target) : base(Target)
    {
        // 设置构建配置的目标类型为'Game'，意味着这个构建目标是用于生成可运行的游戏项目，区别于例如编辑器类型等其他类型的构建目标，确定了项目最终产出的基本性质。
        Type = TargetType.Game;
        // 将"CarlaUE4"模块添加到构建的额外模块列表中，表明在构建过程中需要包含这个模块，该模块应该包含了项目特有的一些功能代码、资源等，对于项目的完整构建是必不可少的部分。
        ExtraModuleNames.Add("CarlaUE4");

        // 根据项目文件的位置确定配置目录路径，先获取项目文件所在的目录路径（通过Path.GetDirectoryName(ProjectFile.ToString())获取），然后拼接上"/Config/"，得到配置文件所在的目录路径，方便后续查找和读取相关配置文件。
        string ConfigDir = Path.GetDirectoryName(ProjectFile.ToString()) + "/Config/";
        // 组合生成OptionalModules.ini配置文件的完整路径，这个配置文件大概率包含了一些可选模块相关的配置信息，后续会读取并依据其内容进行相应的构建配置调整。
        string OptionalModulesFile = Path.Combine(ConfigDir, "OptionalModules.ini");
        // 读取OptionalModules.ini配置文件的所有行内容，并将每行内容存储到一个字符串数组中，方便后续逐行分析其中的配置信息。
        string[] text = System.IO.File.ReadAllLines(OptionalModulesFile);

        // 初始化一个布尔标志来检查是否启用Unity构建，初始化为true表示默认情况下认为是启用Unity构建的，后续会根据配置文件内容来判断是否需要更改此状态。
        bool UnityOn = true;

        // 遍历配置文件中的每一行，对每行内容进行分析，以查找是否存在特定的配置标识来决定是否禁用Unity构建。
        foreach (string line in text)
        {
            // 检查当前行内容是否包含"Unity OFF"字符串，如果包含则说明配置文件中明确指定要禁用Unity构建，需要相应地调整构建配置。
            if (line.Contains("Unity OFF"))
            {
                UnityOn = false;
            }
        }
        //如果UnityOn为false，即OptionalModules.ini中指定了关闭Unity构建

        // 如果根据配置文件判断得出需要禁用Unity构建（UnityOn为false），则执行以下配置调整操作。
        if (!UnityOn)
        {
            // 输出消息到控制台，提示用户当前将禁用Unity构建，方便在构建过程中了解构建配置的变更情况，便于调试和排查问题。
            Console.WriteLine("Disabling unity");
            // 禁用Unity构建配置，这几个属性分别从不同角度控制Unity构建相关的行为，将它们都设置为false，表示完全禁用Unity构建机制，采用其他构建方式来构建项目。
            bUseUnityBuild = false;
            bForceUnityBuild = false;
            bUseAdaptiveUnityBuild = false;
        }
    }
}
