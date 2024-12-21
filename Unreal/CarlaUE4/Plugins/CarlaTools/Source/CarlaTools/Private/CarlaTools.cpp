// Copyright Epic Games, Inc. All Rights Reserved.

#include "CarlaTools.h"
// 定义本地化文本的命名空间，方便对文本进行本地化相关操作
// 这里定义的命名空间为 "FCarlaToolsModule"
#define LOCTEXT_NAMESPACE "FCarlaToolsModule"
// 定义一个日志类别，名为 LogCarlaTools，方便后续在代码中根据这个类别记录不同的日志信息
DEFINE_LOG_CATEGORY(LogCarlaTools);
// FCarlaToolsModule 类的模块启动函数，在模块被加载到内存后执行
// 具体的执行时机在每个模块对应的.uplugin 文件中有指定
void FCarlaToolsModule::StartupModule()// This code will execute after your module is loaded into memory; the exact timing is specified in the.uplugin file per-module
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FCarlaToolsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}// 取消之前定义的本地化文本命名空间

#undef LOCTEXT_NAMESPACE// 实现模块相关的宏，将 FCarlaToolsModule 模块与 "CarlaTools" 名称关联起来，以便系统正确识别和加载该模块
	
IMPLEMENT_MODULE(FCarlaToolsModule, CarlaTools)
// 包含一些可能用于禁用和启用UE4特定宏的头文件，具体作用取决于相关编译器设置和UE4的内部机制
#include <compiler/disable-ue4-macros.h>
#include <carla/Exception.h>
#include <compiler/enable-ue4-macros.h>
// 包含C++标准库中用于处理异常的头文件
#include <exception>
// 定义 carla 命名空间，用于组织与Carla相关的代码逻辑
namespace carla {
// 定义一个函数 throw_exception，用于抛出异常并记录详细的异常信息到日志中
    // 该函数接收一个 std::exception 类型的异常对象作为参数
  void throw_exception(const std::exception &e) {// 使用UE_LOG宏记录一条严重级别的日志（Fatal级别），日志类别为 LogCarlaTools
        // 日志内容包含提示文本 "Exception thrown: " 以及将异常对象的错误信息（通过 e.what() 获取）从UTF8编码转换为TCHAR类型后显示出来
    UE_LOG(LogCarlaTools, Fatal, TEXT("Exception thrown: %s"), UTF8_TO_TCHAR(e.what()));// 理论上在记录完异常信息后，由于是严重错误，程序不应该继续执行到这里，所以直接调用 std::terminate() 终止程序运行
        // It should never reach this part.
    // It should never reach this part.
    std::terminate();
  }

} // namespace carla 
