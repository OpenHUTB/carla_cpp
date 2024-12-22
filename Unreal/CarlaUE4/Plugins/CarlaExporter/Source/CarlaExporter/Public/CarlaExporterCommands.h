// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once//这是一种防止头文件被重复包含的预处理指令，确保在编译过程中，该头文件的内容只会被编译一次，避免出现重复定义等编译错误。

#include "CoreMinimal.h"//CoreMinimal.h 是 Unreal Engine 中的一个基础头文件，包含了很多核心的类型定义、宏以及常用的基础功能相关的声明等，几乎所有的 Unreal Engine 模块都需要包含它来获取这些基础支持。
#include "Framework/Commands/Commands.h"//引入了 Unreal Engine 中用于定义和管理命令相关的框架头文件，提供了创建命令类、命令绑定等功能的基础类和接口，这里的 TCommands 模板类等相关功能就在此头文件中定义，是创建自定义编辑器命令的关键依赖。
#include "EditorStyleSet.h"//用于获取编辑器样式相关的功能，比如可以通过它来设置与命令相关的图标、按钮样式等外观呈现方面的内容，使得创建的命令在编辑器界面上能够按照期望的风格展示出来。

class FCarlaExporterCommands : public TCommands<FCarlaExporterCommands>//定义了一个名为 FCarlaExporterCommands 的类，并且它继承自 TCommands<FCarlaExporterCommands>。继承这个基类意味着该类要遵循 TCommands 类所定义的命令相关的框架规范，来实现具体的命令创建、注册等功能，<FCarlaExporterCommands> 是模板参数，指定了当前命令类自身的类型，方便在基类内部进行一些类型相关的操作和关联。
{
public:

  FCarlaExporterCommands()//构造函数
    : TCommands<FCarlaExporterCommands>//在构造函数的初始化列表中调用了基类 TCommands<FCarlaExporterCommands> 的构造函数(
      TEXT("CarlaExporter"),//这是命令集（Command Set）的名称，用于在 Unreal Engine 的命令系统中唯一标识这个命令相关的集合，通常是一个简短且具有代表性的字符串，方便后续对整个命令集进行查找、管理等操作。
      NSLOCTEXT("Contexts", "CarlaExporter", "CarlaExporter Plugin"),//这是一个本地化文本相关的宏，用于创建一个可以根据不同语言环境进行本地化显示的文本资源。在这里，它定义了在某个上下文（"Contexts"）中，键为 "CarlaExporter" 的文本内容为 "CarlaExporter Plugin"，大概率会用于在编辑器界面上作为命令相关的菜单名称、工具提示等显示给用户，并且可以方便地实现多语言支持，让不同语言版本的编辑器显示相应语言的命令描述信息。
      NAME_None,//表示这个命令集没有特定的父级命令集，即它是相对独立的一个层级，具体的命令层级结构在复杂的编辑器命令系统中可以根据实际需求灵活设置。
      FEditorStyle::GetStyleSetName())//调用了 FEditorStyle 类的静态方法来获取当前编辑器样式集的名称，这个名称会关联到前面提到的编辑器样式相关功能，确保命令相关的界面元素（如按钮等）能使用正确的样式配置。
  {
  }

  // TCommands<> interface
  virtual void RegisterCommands() override;//声明了一个虚函数，用于注册具体的命令。这个函数重写了基类 TCommands 中定义的同名虚函数，在派生类（也就是这里的 FCarlaExporterCommands 类）中需要实现这个函数来定义具体要注册哪些命令以及它们的相关属性（如命令对应的执行函数、快捷键绑定、显示名称等），只有经过注册的命令才能在编辑器中被正确识别和使用。

public:
  TSharedPtr< FUICommandInfo > PluginActionExportAll;//定义了一个共享指针类型（TSharedPtr）的成员变量，指向 FUICommandInfo 类型的对象。FUICommandInfo 通常用于保存一个具体的用户界面命令的详细信息，比如命令的名称、图标、执行的操作回调函数等。这里的 PluginActionExportAll 变量很可能是用于保存一个特定的命令信息，从名称推测可能是和 “导出所有（相关内容）” 这个操作对应的命令，后续会在 RegisterCommands() 函数中对其进行详细配置和初始化。
};
