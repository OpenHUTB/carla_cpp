// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaExporterCommands.h"

#define LOCTEXT_NAMESPACE "FCarlaExporterModule"

void FCarlaExporterCommands::RegisterCommands() // 定义了一个名为RegisterCommands的函数，它属于FCarlaExporterCommands类的成员函数，
// 从函数名推测其功能大概率是用于注册相关的命令，比如在某个用户界面或者系统中注册可以执行的操作命令
{
  UI_COMMAND(PluginActionExportAll, "Carla Exporter", "Export all or selected meshes into an .OBJ file to be used by Carla (in /saved/ folder)", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
// 取消之前定义的LOCTEXT_NAMESPACE宏，恢复到之前的状态或者避免该宏定义在后续代码中产生不必要的影响，
// 保证命名空间的定义只在合适的代码范围内生效。