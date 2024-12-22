// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "DigitalTwinsBaseWidget.h"

#include "OpenDriveToMap.h"
// 定义一个静态的 UOpenDriveToMap 指针，并初始化为 nullptr
static UOpenDriveToMap* OpenDriveToMapObject = nullptr;
// 函数：InitializeOpenDriveToMap
// 输入参数：UOpenDriveToMap 的子类类型 TSubclassOf<UOpenDriveToMap> BaseClass
// 功能：根据传入的类类型初始化一个 UOpenDriveToMap 对象
// 首先检查 OpenDriveToMapObject 是否为 nullptr 且 BaseClass 不为 nullptr
// 若是，则输出错误日志并使用 NewObject 函数创建新的 UOpenDriveToMap 对象
UOpenDriveToMap* UDigitalTwinsBaseWidget::InitializeOpenDriveToMap(TSubclassOf<UOpenDriveToMap> BaseClass){
  if( OpenDriveToMapObject == nullptr && BaseClass != nullptr ){
    // 输出错误日志，表示正在创建新对象
    UE_LOG(LogCarlaTools, Error, TEXT("Creating New Object") );
    OpenDriveToMapObject = NewObject<UOpenDriveToMap>(this, BaseClass);
  }
 // 返回创建或已存在的 OpenDriveToMapObject 指针
  return OpenDriveToMapObject;
}

// 函数：GetOpenDriveToMap
// 功能：返回 OpenDriveToMapObject 指针
UOpenDriveToMap* UDigitalTwinsBaseWidget::GetOpenDriveToMap(){
  return OpenDriveToMapObject;
}

// 函数：SetOpenDriveToMap
// 输入参数：UOpenDriveToMap 指针 ToSet
// 功能：将传入的指针设置为 OpenDriveToMapObject 的值
void UDigitalTwinsBaseWidget::SetOpenDriveToMap(UOpenDriveToMap* ToSet){
  OpenDriveToMapObject = ToSet;
}


// 函数：DestroyOpenDriveToMap
// 功能：销毁 OpenDriveToMapObject
// 首先调用 ConditionalBeginDestroy 函数开始销毁对象
// 然后将 OpenDriveToMapObject 置为 nullptr
void UDigitalTwinsBaseWidget::DestroyOpenDriveToMap(){
  OpenDriveToMapObject->ConditionalBeginDestroy();
  OpenDriveToMapObject = nullptr;
}
