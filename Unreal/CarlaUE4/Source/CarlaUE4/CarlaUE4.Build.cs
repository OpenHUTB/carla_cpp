// 在项目设置的描述页面中填写自己的版权声明

using UnrealBuildTool;// 引入Unreal Build Tool的命名空间，以便使用UBT的类和功能
// 定义一个继承自ModuleRules的类CarlaUE4，用于设置CARLA UE4模块的构建规则
public class CarlaUE4 : ModuleRules
{// 构造函数，接收一个ReadOnlyTargetRules对象作为参数，并调用基类构造函数
    public CarlaUE4(ReadOnlyTargetRules Target) : base(Target)
    {// 设置私有预编译头文件（PCH）的路径，PCH用于加速编译过程
        PrivatePCHHeaderFile = "CarlaUE4.h";
        // 添加公共依赖模块，这些模块是CARLA UE4模块在编译时需要的其他模块 
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
        // 检查当前构建目标是否为编辑器
        if (Target.Type == TargetType.Editor)
        {// 如果是编辑器构建，则添加对UnrealEd模块的依赖，UnrealEd模块包含编辑器相关的功能
            PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
        }
        // 初始化私有依赖模块列表，这里当前没有添加任何私有依赖模块 
        PrivateDependencyModuleNames.AddRange(new string[] { });

        // 如果您正在使用Slate UI，请取消注释。
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // 如果您正在使用在线功能，请取消注释
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // 如果要包含 OnlineSubsystemSteam，请将其添加到uproject文件中的插件部分，并将Enabled属性设置为true
    }
}
