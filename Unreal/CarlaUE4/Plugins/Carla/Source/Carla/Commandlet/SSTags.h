 // Copyright (c) 2023计算机视觉中心（CVC），位于巴塞罗那自治大学（UAB）。
 
//
// 本作品依据麻省理工学院许可协议的条款进行授权。
// 如需副本，请访问<https://opensource.org/licenses/MIT>。

#include "Containers/UnrealString.h" 
#pragma once

#if WITH_EDITORONLY_DATA

// 注意：从地图FBX文件导入的资源，在语义分割方面将依据在RoadRunner中定义的资源名称被分类为道路（ROAD）、道路标线（ROADLINES）以及地形（TERRAIN）。这些标签将用于移动网格物体，以及在世界中生成这些网格物体时指定它们的路径。// segmentation as ROAD, ROADLINES AND TERRAIN based on the asset name
// defined in RoadRunner. These tags will be used for moving the meshes
// and for specifying the path to these meshes when spawning them in a world.
namespace SSTags {
  // 卡拉（Carla）语义分割文件夹标签
  static const FString ROAD       = TEXT("道路");
  static const FString ROADLINE   = TEXT("道路标线");
  static const FString TERRAIN    = TEXT("地形");
  static const FString GRASS      = TEXT("地形");
  static const FString SIDEWALK   = TEXT("人行道");
  static const FString CURB       = TEXT("人行道");
  static const FString GUTTER     = TEXT("人行道");

 // RoadRunner标签
  //
  // RoadRunner的网格物体命名约定：
  // 地图名称_网格类型_网格子类型_层数节点
  //
  // 网格类型是一个更宽泛的地理标签（例如“道路”、“地形”）
  // 网格子类型是该标签的细分名称（例如“道路”、“排水沟”、“地面”）
 static const FString R_ROAD1     = TEXT("道路_道路");
  static const FString R_ROAD2     = TEXT("道路们_道路");
  static const FString R_GRASS1    = TEXT("道路_草地");
  static const FString R_GRASS2    = TEXT("道路们_草地");
  static const FString R_MARKING1  = TEXT("道路_标记");
  static const FString R_MARKING2  = TEXT("道路们_标记");
  static const FString R_SIDEWALK1 = TEXT("道路_人行道");
  static const FString R_SIDEWALK2 = TEXT("道路们_人行道");
  static const FString R_CURB1     = TEXT("道路_路缘");
  static const FString R_CURB2     = TEXT("道路们_路缘");
  static const FString R_GUTTER1   = TEXT("道路_排水沟");
  static const FString R_GUTTER2   = TEXT("道路们_排水沟");
  static const FString R_TERRAIN   = TEXT("地形");
}

#endif
