// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "EpisodeSettings.generated.h"

// FEpisodeSettings结构体用于配置游戏或模拟场景中的相关剧集设置参数。
// 它可以通过蓝图进行编辑和读写操作，方便在不同的项目中根据需求灵活调整各种设置。
USTRUCT(BlueprintType)
struct CARLA_API FEpisodeSettings
{
    GENERATED_BODY()

    // bSynchronousMode用于控制是否启用同步模式。
    // 当设置为true时，可能意味着游戏或模拟按照固定的节奏或同步机制来推进帧更新等操作；
    // 设置为false则可能采用异步等其他不同的更新逻辑。
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSynchronousMode = false;

    // bNoRenderingMode用于指定是否进入无渲染模式。
    // 如果为true，表示不进行画面渲染，可能用于某些不需要可视化输出的情况，比如仅进行后台数据模拟等；
    // 若为false则正常进行渲染操作。
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNoRenderingMode = false;

    // bSubstepping用于决定是否启用子步长机制。
    // 当设置为true时，会按照一定的子步长策略来细分每一步的更新操作，常用于更精细的物理模拟等场景；
    // 设为false则不采用这种细分方式。
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSubstepping = true;

    // FixedDeltaSeconds是一个可选的双精度浮点数类型，用于指定固定的时间步长（如果有需要的话）。
    // 例如可以用来精确控制每一次更新的时间间隔，若没有设置具体值则为未初始化的可选状态。
    TOptional<double> FixedDeltaSeconds;

    // MaxSubstepDeltaTime表示最大的子步长时间间隔，单位是秒。
    // 它限制了在启用子步长机制时，每个子步长所允许的最大时间跨度，默认值为0.01秒。
    double MaxSubstepDeltaTime = 0.01;

    // MaxSubsteps定义了最大的子步数。
    // 即在一次完整的更新操作中，最多允许划分的子步数量，这里默认设置为10步。
    int MaxSubsteps = 10;

    // MaxCullingDistance用于指定最大的剔除距离，单位是米（这里是float类型，推测是距离相关，通常用于场景裁剪等操作）。
    // 超出这个距离的物体等可能会被从渲染或其他处理流程中剔除，以节省性能开销，初始值为0.0f表示默认不进行剔除或者还未设置具体的剔除距离。
    float MaxCullingDistance = 0.0f;

    // bDeterministicRagdolls用于控制布娃娃系统是否具有确定性。
    // 当设置为true时，布娃娃相关的物理表现等可能会按照固定的、可预测的方式进行，常用于需要复现相同效果的场景，比如测试等；
    // 设为false则布娃娃物理表现可能会有更多的随机性等情况。
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDeterministicRagdolls = true;

    // TileStreamingDistance用于设定瓦片流送的距离，单位是米，这里设置为300000.f也就是3km。
    // 通常在一些大型场景中，用于控制距离玩家多远的瓦片资源进行加载等流送操作，以平衡性能和场景完整性。
    float TileStreamingDistance = 300000.f; // 3km

    // ActorActiveDistance表示角色激活的距离，单位是米，此处设为200000.f即2km。
    // 意味着在这个距离范围内的角色等相关对象会被激活参与游戏逻辑等处理，超出这个距离可能进入休眠等状态，以节省资源。
    float ActorActiveDistance = 200000.f; // 2km

    // SpectatorAsEgo用于确定是否将旁观者（观察者角色）视为自我主体。
    // 如果为true，可能旁观者相关的视角、行为等会被当作主要的关注对象来进行一些特殊处理；
    // 设为false则旁观者和自我主体有不同的处理逻辑。
    bool SpectatorAsEgo = true;

};
