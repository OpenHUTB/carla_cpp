
#pragma once

#include "carla/rpc/ActorId.h"
/**
 * @namespace carla::traffic_manager
 *
 * @brief carla命名空间中用于交通管理的子命名空间。
 */
namespace carla {
namespace traffic_manager {
    /**
     * @brief 使用carla::rpc命名空间中的ActorId类型。
     */
using ActorId = carla::rpc::ActorId;

/**
 * @class Stage
 * @brief 阶段类型接口。
 *
 * 这是一个抽象基类，定义了交通管理阶段的基本操作。
 */
class Stage {

public:
    /**
     * @brief 默认构造函数。
     */
    Stage() {};
    /**
     * @brief 拷贝构造函数。
     *
     * @param other 要拷贝的Stage对象。
     */
    Stage(const Stage&) {};
    /**
     * @brief 虚析构函数。
     *
     * 确保派生类的正确析构。
     */
    virtual ~Stage() {};

    /**
     * @brief 更新方法。
     *
     * 应在每个更新周期调用此方法，代表阶段的核心操作。
     *
     * @param index 当前更新周期的索引。
     */
    virtual void Update(const unsigned long index) = 0;
    /**
     * @brief 移除参与者方法。
     *
     * 应从阶段的内部状态中移除指定的参与者。
     *
     * @param actor_id 要移除的参与者的ID。
     */
    virtual void RemoveActor(const ActorId actor_id) = 0;
    /**
     * @brief 重置方法。
     *
     * 应清空阶段的所有内部状态。
     */
    virtual void Reset() = 0;
};

} // namespace traffic_manager
} // namespace carla
