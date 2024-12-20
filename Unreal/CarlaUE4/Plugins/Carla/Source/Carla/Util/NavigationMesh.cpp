// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Util/NavigationMesh.h"
#include "Misc/FileHelper.h"

// FNavigationMesh类的作用可能是用于加载导航网格相关的数据，这里定义了Load函数用于加载特定地图名称对应的导航网格数据
// 参数MapName表示要加载导航网格数据对应的地图名称
TArray<uint8> FNavigationMesh::Load(FString MapName)
{
#if WITH_EDITOR
    {
        // 当在编辑器中运行时，地图名称会有一个额外的前缀，这里的目的是移除这个前缀
        // 先将传入的地图名称赋值给CorrectedMapName，方便后续操作且不改变原始传入的MapName
        FString CorrectedMapName = MapName;
        // 定义了编辑器中地图名称额外前缀的常量字符串
        constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
        // 从CorrectedMapName字符串的开头移除PIEPrefix这个前缀
        CorrectedMapName.RemoveFromStart(PIEPrefix);
        // 在日志中输出地图名称校正的相关信息，方便调试查看，告知从原始的地图名称校正为新的名称
        UE_LOG(LogCarla, Log, TEXT("FNavigationMesh: Corrected map name from %s to %s"), *MapName, *CorrectedMapName);
        // 将校正后的地图名称重新赋值给MapName，后续使用校正后的名称来查找对应的文件等操作
        MapName = CorrectedMapName;
    }
#endif // WITH_EDITOR

    // 根据地图名称构建对应的文件名，这里假设导航网格数据是以.bin为后缀存储的
    const auto FileName = MapName + ".bin";

    // 用于存储查找到的符合条件的文件路径列表，后续会查找与导航网格数据相关的文件
    TArray<FString> Files;
    // 调用文件管理器的方法，递归地查找符合条件的文件，查找的范围是项目内容目录（*FPaths::ProjectContentDir()）
    // 查找的文件名是之前构建的FileName，后面的参数分别表示是否递归查找、是否查找目录、是否查找隐藏文件等
    IFileManager::Get().FindFilesRecursive(Files, *FPaths::ProjectContentDir(), *FileName, true, false, false);

    // 用于存储最终要返回的导航网格文件内容，初始化为空数组
    TArray<uint8> Content;

    // 如果没有找到任何符合条件的文件（即Files数组中元素个数为0），则在日志中输出错误信息，提示找不到对应地图的导航网格文件
    if (!Files.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *MapName);
    }
    // 如果找到了文件，并且能够成功将文件内容加载到Content数组中（通过FFileHelper::LoadFileToArray函数来加载）
    // 则在日志中输出加载文件的相关信息，告知正在加载的导航网格文件名
    else if (FFileHelper::LoadFileToArray(Content, *Files[0], 0))
    {
        UE_LOG(LogCarla, Log, TEXT("Loading Navigation Mesh file '%s'"), *Files[0]);
    }
    // 如果找到了文件，但是加载文件内容到Content数组失败了，则在日志中输出错误信息，提示加载文件失败以及对应的文件名
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Navigation Mesh file '%s'"), *Files[0]);
    }

    // 最后将加载到的（可能为空，加载失败的情况）导航网格文件内容返回
    return Content;
}
