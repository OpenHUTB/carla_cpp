// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
//这是预处理指令
#include "Carla/OpenDrive/OpenDriveActor.h"//这是将Carla/OpenDrive/OpenDriveActor.h头文件包含进来。这意味着当前文件可能会使用到OpenDriveActor.h中定义的类、函数或者其他类型。
#include "Commandlets/Commandlet.h"//这表明MoveAssetsCommandlet类可能是从Commandlet类派生而来或者使用了Commandlet类中的功能。
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"//包含ObjectLibrary.h头文件，这可能是因为MoveAssetsCommandlet类需要使用ObjectLibrary类的功能。

#if WITH_EDITORONLY_DATA
#include "AssetRegistry/Public/AssetRegistryModule.h"//AssetRegistryModule可能用于在编辑器中管理资产注册表相关的功能。
#include "Developer/AssetTools/Public/AssetToolsModule.h"//AssetToolsModule可能提供了一些在编辑器中操作资产的工具函数或者类。
#endif // WITH_EDITORONLY_DATA//表示条件编译块结束
#include "MoveAssetsCommandlet.generated.h"//这是一个自动生成的头文件（通常由虚幻引擎的代码生成工具生成）

///包含 “包参数” 的结构体，
///用于在调用此命令行工具时存储已解析的参数。
USTRUCT()
struct CARLA_API FMovePackageParams
{
  GENERATED_USTRUCT_BODY()

  FString Name;

  TArray<FString> MapNames;
};

UCLASS()
class UMoveAssetsCommandlet : public UCommandlet
{
  GENERATED_BODY()

public:

  /// 默认构造函数.
  UMoveAssetsCommandlet();
#if WITH_EDITORONLY_DATA

  /// 解析通过 @a InParams 提供的命令行参数。
  ///要解析的参数是包名称
  ///和以字符串形式连接在一起的地图名称列表。
  FMovePackageParams ParseParams(const FString &InParams) const;

  /// 将地图中包含的所有资产从 @a SrcPath 移动到 @a DestPath
  void MoveAssetsFromMapForSemanticSegmentation(const FString &PackageName, const FString &MapName);

  /// 将在一个 “包参数” 中列出的所有地图的网格移动
  void MoveAssets(const FMovePackageParams &PackageParams);

public:

  ///主方法和命令行工具的入口，
  ///以 @a Params 作为输入参数
  virtual int32 Main(const FString &Params) override;

#endif // WITH_EDITORONLY_DATA

private:

  /// 以下数据结构被声明为类成员，
  ///并使用 UPROPERTY 宏来避免被虚幻引擎 4 进行垃圾回收。

介绍一下 UPROPERTY 宏的作用和使用方法
数据结构如何避免被 UE4 进行垃圾回收
推荐一些有关 UE4 数据结构的教程
  /// 从任何对象库中加载资产
  UPROPERTY()
  TArray<FAssetData> AssetDatas;

  /// 从任何对象库中加载地图
  UPROPERTY()
  TArray<FAssetData> MapContents;
  /// 用于在对象库中加载资产。
  ///已加载的数据存储在 “资产数据” 中。
  UPROPERTY()
  UObjectLibrary *AssetsObjectLibrary;
};
