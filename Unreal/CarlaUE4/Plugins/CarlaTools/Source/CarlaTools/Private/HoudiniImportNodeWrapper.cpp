// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "HoudiniImportNodeWrapper.h"
// 构造函数，使用 FObjectInitializer 进行初始化
UHoudiniImportNodeWrapper::UHoudiniImportNodeWrapper(const FObjectInitializer& ObjectInitializer)
{
  // 若该对象不是类的默认对象，则将其添加到根节点
  if ( HasAnyFlags(RF_ClassDefaultObject) == false )
	{
		AddToRoot();
	}
}
// 静态函数，用于导入建筑物，接受多个参数，包括 Houdini 对象、变换信息、
//世界上下文对象、地图名称、OSM 文件路径、经纬度、
//簇大小、当前簇编号和是否使用质心标志
UHoudiniImportNodeWrapper* UHoudiniImportNodeWrapper::ImportBuildings(
    UObject* InHoudiniObject,
    const FTransform& InInstantiateAt,
    UObject* InWorldContextObject,
    const FString& MapName, const FString& OSMFilePath,
    float Latitude, float Longitude,
    int ClusterSize, int CurrentCluster,
    bool bUseCOM)
{
  // 输出错误日志，表明 Houdini 资产无效
  UE_LOG(LogCarlaTools, Error, TEXT("Houdini asset not valid"));
  // 返回空指针
  return nullptr;
}
// 激活函数，目前函数体为空
void UHoudiniImportNodeWrapper::Activate()
{
}

// 处理完成的函数，当任务完成时调用
void UHoudiniImportNodeWrapper::HandleCompleted(
    bool bCookSuccess, bool bBakeSuccess)
{
  // 输出日志，表明生成已完成
  UE_LOG(LogCarlaTools, Log, TEXT("Generation Finished"));
  // 若 Completed 事件已绑定，则广播完成信息
  if (Completed.IsBound())
  {
    Completed.Broadcast(bCookSuccess, bBakeSuccess);
  }
  // 从根节点移除该对象
  RemoveFromRoot();
}
// 处理失败的函数，当任务失败时调用
void UHoudiniImportNodeWrapper::HandleFailed(
    bool bCookSuccess, bool bBakeSuccess)
{
  // 输出日志，表明生成失败
  UE_LOG(LogCarlaTools, Log, TEXT("Generation failed"));
  // 若 Failed 事件已绑定，则广播失败信息
  if (Failed.IsBound())
  {
    Failed.Broadcast(bCookSuccess, bBakeSuccess);
  }
  // 从根节点移除该对象
  RemoveFromRoot();
}
