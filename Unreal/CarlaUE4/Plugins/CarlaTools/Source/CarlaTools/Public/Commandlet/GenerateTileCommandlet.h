// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 这是一个头文件保护宏，防止头文件被重复包含
#pragma once
// 引入UE4中引擎的对象库相关头文件，用于处理游戏中的对象资源等操作
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"// 引入UE4中命令行工具相关的基类头文件，当前类会继承自这个基类来实现特定的命令行功能
#include "Commandlets/Commandlet.h"
// 以下两个头文件可能是用于控制UE4相关宏的启用和禁用，具体要看其内部实现
#include <compiler/disable-ue4-macros.h>

#include <compiler/enable-ue4-macros.h>
// 引入OpenDriveToMap相关的头文件，推测可能是用于将OpenDrive数据转换为地图相关功能的自定义类（具体需看对应实现）
#include "OpenDriveToMap.h"
// 这个宏用于生成与类相关的一些代码，UE4的反射机制相关，用于实现序列化、属性编辑等功能
#include "GenerateTileCommandlet.generated.h"
// 声明一个日志类别，名为LogCarlaToolsMapGenerateTileCommandlet，用于在代码运行时输出相应的日志信息，日志级别有Log等，作用范围是All（可根据实际需求输出各种相关日志）
// Each commandlet should generate only 1 Tile

DECLARE_LOG_CATEGORY_EXTERN(LogCarlaToolsMapGenerateTileCommandlet, Log, All);
// 使用UCLASS宏定义一个UE4中的类，使其能被UE4的反射系统识别并具有相应的功能，CARLATOOLS_API可能是用于控制类的导出等相关设置（具体要看项目的宏定义配置）


UCLASS()// UGenerateTileCommandlet类继承自UCommandlet，意味着它可以作为一个命令行工具在UE4项目中执行特定任务
class CARLATOOLS_API UGenerateTileCommandlet// 由UE4的反射系统生成的必要代码体，用于实现类相关的反射功能等
  : public UCommandlet// 默认构造函数，用于创建UGenerateTileCommandlet类的对象实例，通常在这里可以进行一些成员变量的初始化等基本操作（如果没有在初始化列表中完成的话）
{
  GENERATED_BODY()

public:
// 默认构造函数，用于创建UGenerateTileCommandlet类的对象实例，通常在这里可以进行一些成员变量的初始化等基本操作（如果没有在初始化列表中完成的话）
  /// Default constructor.
  UGenerateTileCommandlet();// 带初始化参数的构造函数，通过传入FObjectInitializer参数，可以更灵活地初始化对象及其相关属性，按照UE4的对象初始化机制来进行操作
  UGenerateTileCommandlet(const FObjectInitializer &);
// 以下代码块只有在编译包含编辑器相关数据（WITH_EDITORONLY_DATA宏定义启用，通常在编辑器相关编译配置下）时才会编译这段代码
#if WITH_EDITORONLY_DATA
// 重写UCommandlet类中的Main函数，这个函数是命令行工具执行的入口点，传入的Params参数包含了命令行传入的相关参数，在这个函数中会实现具体的命令行任务逻辑
  virtual int32 Main(const FString &Params) override;

#endif // WITH_EDITORONLY_DATA// 使用UPROPERTY宏声明一个属性，UOpenDriveToMap*类型的指针，推测是用于指向与OpenDrive转换地图相关的某个对象实例，通过UE4的属性系统可以方便地在编辑器中进行赋值、访问等操作
  UPROPERTY()
  UOpenDriveToMap* OpenDriveMap;
  UPROPERTY()
  UClass* OpenDriveClass;
};
