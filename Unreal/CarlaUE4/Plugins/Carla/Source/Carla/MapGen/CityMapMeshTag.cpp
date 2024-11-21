// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "CityMapMeshTag.h"

#include "Package.h"

#include <array>

// 定义函数CityMapMeshTag::GetBaseMeshTag，该函数返回一个ECityMapMeshTag类型的值
// 用于获取基础的网格标签，这里具体返回的是RoadTwoLanes_LaneLeft这个标签类型
ECityMapMeshTag CityMapMeshTag::GetBaseMeshTag()
{
  return ECityMapMeshTag::RoadTwoLanes_LaneLeft;
}

// 定义函数CityMapMeshTag::GetRoadIntersectionSize，该函数返回一个无符号32位整数
// 用于获取道路交叉路口的大小，这里返回的值是5u（无符号整数5）
uint32 CityMapMeshTag::GetRoadIntersectionSize()
{
  return 5u;
}

// 定义函数CityMapMeshTag::ToString，该函数接受一个ECityMapMeshTag类型的参数Tag
// 用于将ECityMapMeshTag类型的枚举值转换为对应的字符串表示形式
FString CityMapMeshTag::ToString(ECityMapMeshTag Tag)
{
  // 在任何包中查找名为ECityMapMeshTag的枚举类型对象，如果找不到则返回nullptr
  const UEnum* ptr = FindObject<UEnum>(ANY_PACKAGE, TEXT("ECityMapMeshTag"), true);
  // 如果没有找到对应的枚举类型对象
  if(!ptr)
    // 返回一个表示"Invalid"的字符串
    return FString("Invalid");
  // 如果找到了对应的枚举类型对象，则通过该对象根据传入的枚举值Tag获取对应的名称字符串并返回
  return ptr->GetNameStringByIndex(static_cast<int32>(Tag));
}
