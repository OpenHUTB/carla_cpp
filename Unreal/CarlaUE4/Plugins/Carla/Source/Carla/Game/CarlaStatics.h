// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入Carla相关的头文件，包含游戏的剧集（Episode）、游戏实例（GameInstance）、游戏模式基类（GameModeBase）以及游戏设置（Settings）相关的定义
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Settings/CarlaSettings.h"

// 引入虚幻引擎的游戏玩法静态函数相关头文件，用于获取游戏中的一些通用对象等操作
#include "Kismet/GameplayStatics.h"
// 引入蓝图函数库相关头文件，表明这个类是用于给蓝图提供函数调用功能的
#include "Kismet/BlueprintFunctionLibrary.h"

// 声明这个类是生成代码的一部分，由虚幻引擎的代码生成系统处理
#include "CarlaStatics.generated.h"

// =============================================================================
// -- UCarlaStatics declaration ------------------------------------------------
// =============================================================================

// UCLASS() 宏声明这是一个可以被虚幻引擎反射系统识别的类，CARLA_API 可能是该项目自定义的导出相关的宏定义（用于在不同模块间正确访问这个类）
// UCarlaStatics类继承自UBlueprintFunctionLibrary，意味着这个类主要用于向蓝图脚本暴露一些函数供其调用
UCLASS()
class CARLA_API UCarlaStatics : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // UFUNCTION宏用于将下面的函数暴露给蓝图脚本使用，BlueprintPure表示这个函数是纯函数（不改变类的状态，只是返回值）
    // Category="CARLA" 表示在蓝图编辑器中这个函数会出现在CARLA分类下
    // meta=(WorldContext="WorldContextObject") 表示这个函数需要一个世界上下文对象（通常用于获取所在游戏世界相关的信息）作为参数
    // 函数功能：获取当前世界上下文对应的游戏模式（ACarlaGameModeBase类型），返回值是指向游戏模式对象的指针，如果获取失败则返回nullptr
    UFUNCTION(BlueprintPure, Category="CARLA", meta=(WorldContext="WorldContextObject"))
    static ACarlaGameModeBase *GetGameMode(const UObject *WorldContextObject);

    // 类似上述GetGameMode函数的说明，功能是获取当前世界上下文对应的游戏实例（UCarlaGameInstance类型），返回值是指向游戏实例对象的指针，如果获取失败则返回nullptr
    UFUNCTION(BlueprintPure, Category="CARLA", meta=(WorldContext="WorldContextObject"))
    static UCarlaGameInstance *GetGameInstance(const UObject *WorldContextObject);

    // 获取当前世界上下文对应的游戏剧集（UCarlaEpisode类型），返回值是指向游戏剧集对象的指针，如果获取失败（比如游戏实例不存在等情况）则返回nullptr
    UFUNCTION(BlueprintPure, Category="CARLA", meta=(WorldContext="WorldContextObject"))
    static UCarlaEpisode *GetCurrentEpisode(const UObject *WorldContextObject);

    // 获取当前世界上下文对应的游戏设置（UCarlaSettings类型），返回值是指向游戏设置对象的指针，如果获取失败（比如游戏实例不存在等情况）则返回nullptr
    UFUNCTION(BlueprintPure, Category="CARLA", meta=(WorldContext="WorldContextObject"))
    static UCarlaSettings *GetCarlaSettings(const UObject *WorldContextObject);

    // 获取所有地图的名称，返回一个FString类型的数组，该函数没有依赖特定的世界上下文对象来获取地图名称（可能是从全局配置等地方获取）
    UFUNCTION(BlueprintPure, Category="CARLA")
    static TArray<FString> GetAllMapNames();

    // 获取当前世界上下文对应的游戏记录器（ACarlaRecorder类型），返回值是指向游戏记录器对象的指针，如果当前剧集不存在等情况获取失败则返回nullptr
    UFUNCTION(BlueprintPure, Category="CARLA", meta=(WorldContext="WorldContextObject"))
    static ACarlaRecorder* GetRecorder(const UObject *WorldContextObject);

    // 获取当前世界上下文对应的重放器（CarlaReplayer类型），返回值是指向重放器对象的指针，如果当前剧集不存在等情况获取失败则返回nullptr
    // 注意：这个函数没有BlueprintPure宏，可能在实现或者调用上有一些特殊情况（比如可能会修改某些状态等，需要查看具体实现和使用场景）
    static CarlaReplayer* GetReplayer(const UObject *WorldContextObject);

    // 获取当前世界上下文对应的大地图管理器（ALargeMapManager类型），返回值是指向大地图管理器对象的指针，如果获取游戏模式失败等情况则返回nullptr
    UFUNCTION(BlueprintPure, Category="CARLA", meta=(WorldContext="WorldContextObject"))
    static ALargeMapManager* GetLargeMapManager(const UObject *WorldContextObject);
};

// =============================================================================
// -- UCarlaStatics implementation ---------------------------------------------
// =============================================================================

// 以下是UCarlaStatics类中各个函数的具体实现部分

// GetGameMode函数的实现，通过UGameplayStatics的GetGameMode函数获取游戏模式对象，然后使用Cast进行类型转换，确保返回的是ACarlaGameModeBase类型的指针
inline ACarlaGameModeBase *UCarlaStatics::GetGameMode(const UObject *WorldContext)
{
    return Cast<ACarlaGameModeBase>(UGameplayStatics::GetGameMode(WorldContext));
}

// GetGameInstance函数的实现，类似GetGameMode函数，通过UGameplayStatics的GetGameInstance函数获取游戏实例对象，再用Cast进行类型转换，返回UCarlaGameInstance类型的指针
inline UCarlaGameInstance *UCarlaStatics::GetGameInstance(const UObject *WorldContext)
{
    return Cast<UCarlaGameInstance>(UGameplayStatics::GetGameInstance(WorldContext));
}

// GetCurrentEpisode函数的实现，首先调用GetGameInstance函数获取游戏实例，然后判断游戏实例是否为空，如果不为空则通过游戏实例获取其对应的游戏剧集并返回，否则返回nullptr
inline UCarlaEpisode *UCarlaStatics::GetCurrentEpisode(const UObject *WorldContext)
{
    auto GameInstance = GetGameInstance(WorldContext);
    return GameInstance!= nullptr? GameInstance->GetCarlaEpisode() : nullptr;
}

// GetCarlaSettings函数的实现，同样先获取游戏实例，然后判断游戏实例是否为空，不为空时通过游戏实例获取对应的游戏设置并返回，为空则返回nullptr
inline UCarlaSettings *UCarlaStatics::GetCarlaSettings(const UObject *WorldContext)
{
    auto GameInstance = GetGameInstance(WorldContext);
    return GameInstance!= nullptr? GameInstance->GetCARLASettings() : nullptr;
}

// GetRecorder函数的实现，先调用GetCurrentEpisode函数获取当前剧集对象，然后判断剧集对象是否存在，如果存在则获取该剧集对应的记录器并返回，不存在则返回nullptr
inline ACarlaRecorder* UCarlaStatics::GetRecorder(const UObject *WorldContextObject)
{
    auto* Episode = UCarlaStatics::GetCurrentEpisode(WorldContextObject);
    if (Episode)
    {
        return Episode->GetRecorder();
    }
    return nullptr;
}

// GetReplayer函数的实现，先获取当前剧集对象，判断其是否存在，若存在则获取该剧集对应的重放器并返回，不存在则返回nullptr
inline CarlaReplayer* UCarlaStatics::GetReplayer(const UObject *WorldContextObject)
{
    auto* Episode = UCarlaStatics::GetCurrentEpisode(WorldContextObject);
    if (Episode)
    {
        return Episode->GetReplayer();
    }
    return nullptr;
}

// GetLargeMapManager函数的实现，先调用GetGameMode函数获取游戏模式对象，判断其是否存在，若存在则获取游戏模式对应的大地图管理器并返回，不存在则返回nullptr
inline ALargeMapManager* UCarlaStatics::GetLargeMapManager(const UObject *WorldContextObject)
{
    ACarlaGameModeBase* GameMode = GetGameMode(WorldContextObject);
    if (GameMode)
    {
        return GameMode->GetLMManager();
    }
    return nullptr;
}
