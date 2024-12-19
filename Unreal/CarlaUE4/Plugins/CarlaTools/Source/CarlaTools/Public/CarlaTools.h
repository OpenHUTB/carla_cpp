// Copyright Epic Games, Inc. All Rights Reserved.
// 版权声明，表明这段代码的版权归属于Epic Games公司，保留所有权利，告知使用者代码的版权归属情况。
#pragma once
// 预处理器指令，用于确保该头文件在整个编译过程中只会被编译一次，避免重复定义等问题，提高编译效率并保证编译的正确性。
#include "CoreMinimal.h"// 引入UE4的核心基础头文件，其中包含了许多UE4中基础的数据类型定义、常用的宏以及一些基本的功能函数等，
// 是UE4项目开发中常用的基础依赖，很多其他模块和类都会依赖这里面定义的内容来构建。
#include "Modules/ModuleManager.h"
// 引入UE4模块管理相关的头文件，用于实现模块相关的功能，比如模块的初始化、启动、关闭等操作，
// 当创建自定义的UE4模块时，需要通过这个头文件中定义的接口来与UE4的模块管理系统进行交互。
DECLARE_LOG_CATEGORY_EXTERN(LogCarlaTools, Log, All);
// 这是UE4中用于声明一个日志分类的宏，这里声明了一个名为LogCarlaTools的日志分类，
// 第一个参数是日志分类的名称，后续可以通过这个名称来区分不同模块或者不同功能产生的日志信息；
// 第二个参数指定了日志的级别类型（这里是Log级别，表示常规的日志信息）；
// 第三个参数All表示这个日志分类在所有的构建配置（例如开发、发布等配置）下都可用，方便在不同阶段记录和查看相关模块运行产生的日志。
class FCarlaToolsModule : public IModuleInterface
{
public:
/** IModuleInterface implementation */
    // 以下两个虚函数是实现 IModuleInterface 接口所要求的，用于定义模块在启动和关闭阶段的具体行为。
	/** IModuleInterface implementation */
	virtual void StartupModule() override;// 虚函数声明，用于实现模块启动时的操作，当UE4的模块管理系统启动该模块时，会调用这个函数，
    // 在这个函数内部通常会进行一些初始化资源、注册相关功能、创建必要的对象等操作，
    // 具体的实现需要在对应的.cpp文件中编写，根据模块的功能需求来定制启动逻辑。
	virtual void ShutdownModule() override;// 虚函数声明，用于实现模块关闭时的操作，当UE4的模块管理系统决定关闭该模块时，会调用这个函数，
    // 在这里一般会进行释放资源、注销已注册的功能、销毁相关对象等清理工作，
    // 同样具体的实现要在.cpp文件中根据模块的实际情况来确定关闭时需要执行的操作。
};
};
