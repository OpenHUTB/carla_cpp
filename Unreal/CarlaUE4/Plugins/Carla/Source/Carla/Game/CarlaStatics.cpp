// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaStatics.h"

#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"

// UCarlaStatics类作用域下的函数GetAllMapNames的定义，该函数用于获取所有地图名称相关的操作
// 函数返回一个FString类型的数组，里面存放着处理后的地图名称
TArray<FString> UCarlaStatics::GetAllMapNames()
{
    // 用于临时存储字符串列表的数组，比如在后续解析文件路径等操作时可能会临时存放一些分割出来的字符串片段等
    TArray<FString> TmpStrList, 
    // 用于存储最终要返回的地图名称列表
                    MapNameList; 
    
    // 通过文件管理器查找符合条件的文件，递归地查找（第三个参数指定查找的文件后缀为*.umap，表示查找所有的umap类型文件）
    // 第一个参数MapNameList是用于存放查找到的文件路径列表（这里是存放umap文件的路径）
    // 第二个参数*FPaths::ProjectContentDir() 表示从项目内容目录开始查找
    // 后面几个布尔参数分别表示是否递归查找、是否包含目录、是否包含隐藏文件等，这里设置为相应的查找要求
    IFileManager::Get().FindFilesRecursive(
        MapNameList, *FPaths::ProjectContentDir(), TEXT("*.umap"), true, false, false); 
    
    // 移除名称中包含"TestMaps"的元素，可能是不想把测试相关的地图纳入最终的地图名称列表中
    MapNameList.RemoveAll( [](const FString &Name) { return Name.Contains("TestMaps");});
    // 移除名称中包含"OpenDriveMap"的元素，同理也是筛选掉特定的不需要的地图相关内容
    MapNameList.RemoveAll( [](const FString &Name) { return Name.Contains("OpenDriveMap");});
    
    // 遍历找到的地图名称列表（经过前面筛选后的），对每个地图名称进行处理
    for (int i = 0; i < MapNameList.Num(); i++) {
        // 将当前地图名称（文件路径）按照"Content/"进行分割，分割后的结果存放在TmpStrList数组中
        MapNameList[i].ParseIntoArray(TmpStrList, TEXT("Content/"), true);
        // 取分割后的第二个元素作为新的地图名称（这里假设是按照特定格式组织路径，取后半部分作为有效地图名部分）
        MapNameList[i] = TmpStrList[1];
        // 替换掉地图名称中的".umap"后缀，得到一个更简洁纯粹的地图名称（去掉了文件后缀相关的表示）
        MapNameList[i] = MapNameList[i].Replace(TEXT(".umap"), TEXT(""));
        // 在地图名称前面加上 "/Game/" 前缀，可能是为了符合游戏内对于地图资源路径等的格式要求
        MapNameList[i] = "/Game/" + MapNameList[i];
    }
    return MapNameList;
}
