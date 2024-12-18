// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>  /// 提供原子操作，确保线程安全
#include <chrono>  /// 提供时间功能，用于时间计算
#include <random>  /// 提供随机数生成功能
#include <unordered_map> /// 提供无序映射容器，用于快速查找
/// 包含Carla客户端相关的头文件
#include "carla/client/Actor.h"
#include "carla/client/Vehicle.h"
#include "carla/Memory.h"/// 包含Carla内存管理相关的头文件
#include "carla/rpc/ActorId.h"/// 包含Carla Rpc 通信相关的头文件，定义了参与者的唯一标识符

#include "carla/trafficmanager/AtomicActorSet.h"/// 包含Carla交通管理器的相关头文件
#include "carla/trafficmanager/AtomicMap.h"

namespace carla {
    namespace traffic_manager {
        /// 使用别名简化代码中的命名
        namespace cc = carla::client;
        namespace cg = carla::geom;
        using ActorPtr = carla::SharedPtr<cc::Actor>;/// 参与者的智能指针类型
        using ActorId = carla::ActorId;/// 参与者的唯一标识符类型
        using Path = std::vector<cg::Location>;/// 路线类型，由一系列地理位置组成
        using Route = std::vector<uint8_t>;/// 路线类型，由一系列字节组成，表示路线信息
        /// 换道信息结构体
        struct ChangeLaneInfo {
            bool change_lane = false;/// 是否换道
            bool direction = false;/// 换道方向
        };
        /// 交通管理参数
        class Parameters {

        private:
            /// 基于速度限制差异的单个车辆目标速度映射
            AtomicMap<ActorId, float> percentage_difference_from_speed_limit;
            /// 单个车辆的车道偏移映射
            AtomicMap<ActorId, float> lane_offset;
            /// 基于期望速度的单个车辆目标速度映射
            AtomicMap<ActorId, float> exact_desired_speed;
            /// 全局目标速度限制差异百分比
            float global_percentage_difference_from_limit = 0;
            /// 全局车道偏移
            float global_lane_offset = 0;
            /// 在碰撞检测期间要忽略的演员集合映射
            AtomicMap<ActorId, std::shared_ptr<AtomicActorSet>> ignore_collision;
            /// 到前导车辆的距离映射
            AtomicMap<ActorId, float> distance_to_leading_vehicle;
            /// 强制换道命令映射
            AtomicMap<ActorId, ChangeLaneInfo> force_lane_change;
            /// 自动换道命令映射
            AtomicMap<ActorId, bool> auto_lane_change;
            /// 闯交通信号灯百分比映射
            AtomicMap<ActorId, float> perc_run_traffic_light;
            /// 闯交通标志百分比映射
            AtomicMap<ActorId, float> perc_run_traffic_sign;
            /// 忽略行人百分比映射
            AtomicMap<ActorId, float> perc_ignore_walkers;
            /// 忽略车辆百分比映射
            AtomicMap<ActorId, float> perc_ignore_vehicles;
            /// 靠右行驶规则百分比映射
            AtomicMap<ActorId, float> perc_keep_right;
            /// 随机左换道百分比映射
            AtomicMap<ActorId, float> perc_random_left;
            /// 随机右换道百分比映射
            AtomicMap<ActorId, float> perc_random_right;
            /// 车辆灯光自动更新标志映射
            AtomicMap<ActorId, bool> auto_update_vehicle_lights;
            /// 同步开关
            std::atomic<bool> synchronous_mode{ false };
            /// 距离边距
            std::atomic<float> distance_margin{ 2.0 };
            /// 混合物理模式开关
            std::atomic<bool> hybrid_physics_mode{ false };
            /// 自动重生模式开关
            std::atomic<bool> respawn_dormant_vehicles{ false };
            /// 相对于主角车辆的最小重生距离
            std::atomic<float> respawn_lower_bound{ 100.0 };
            /// 相对于主角车辆的最大重生距离
            std::atomic<float> respawn_upper_bound{ 1000.0 };
            /// 相对于主角车辆的最小可能重生距离
            float min_lower_bound;
            /// 相对于主角车辆的最大可能重生距离
            float max_upper_bound;
            /// 混合物理半径
            std::atomic<float> hybrid_physics_radius{ 70.0 };
            /// Open Street Map模式参数
            std::atomic<bool> osm_mode{ true };
            /// 是否导入自定义路径的参数映射
            AtomicMap<ActorId, bool> upload_path;
            /// 存储所有自定义路径的结构
            AtomicMap<ActorId, Path> custom_path;
            /// 是否导入自定义路线的参数映射
            AtomicMap<ActorId, bool> upload_route;
            /// 存储所有自定义路线的结构
            AtomicMap<ActorId, Route> custom_route;

        public:
            /// 构造函数
            Parameters();
            /// 析构函数
            ~Parameters();

            ////////////////////////////////// SETTERS /////////////////////////////////////

            /// 设置车辆相对于速度限制的速度降低百分比
            /// 如果小于0，则表示速度增加百分比
            void SetPercentageSpeedDifference(const ActorPtr& actor, const float percentage);

            /// 设置车道偏移量，从中心线开始的位移
            /// 正值表示向右偏移，负值表示向左偏移
            void SetLaneOffset(const ActorPtr& actor, const float offset);

            /// 设置车辆的精确期望速度
            void SetDesiredSpeed(const ActorPtr& actor, const float value);

            /// 设置全局相对于速度限制的速度降低百分比
            /// 如果小于0，则表示速度增加百分比
            void SetGlobalPercentageSpeedDifference(float const percentage);

            /// 设置全局车道偏移量，从中心线开始的位移
            /// 正值表示向右偏移，负值表示向左偏移
            void SetGlobalLaneOffset(float const offset);

            /// 设置车辆之间的碰撞检测规则的方法
            void SetCollisionDetection(
                const ActorPtr& reference_actor, ///<参考车辆指针
                const ActorPtr& other_actor,     ///<另一车辆指针
                const bool detect_collision);    ///<是否碰撞的布尔值

            /// 强制车辆换道的方法
            /// 方向标志可以设置为true表示向左，false表示向右
            void SetForceLaneChange(const ActorPtr& actor, const bool direction);///<车辆指针和方向布尔值

            /// 启用/禁用车辆的自动换道功能
            void SetAutoLaneChange(const ActorPtr& actor, const bool enable);///<车辆指针和启用或禁用的布尔值

            /// 设置车辆应保持与前车距离的方法
            void SetDistanceToLeadingVehicle(const ActorPtr& actor, const float distance);///<车辆指针和应保持的距离值

            /// 设置无视交通标志的概率的方法
            void SetPercentageRunningSign(const ActorPtr& actor, const float perc);///<车辆指针和无视标志的概率值

            /// 设置无视交通信号灯的概率的方法
            void SetPercentageRunningLight(const ActorPtr& actor, const float perc);///<车辆指针和无视信号灯的概率值

            /// 设置无视其他车辆的概率的方法
            void SetPercentageIgnoreVehicles(const ActorPtr& actor, const float perc);///<车辆指针和无视车辆的概率值

            /// 设置无视其他车辆的概率的方法
            void SetPercentageIgnoreWalkers(const ActorPtr& actor, const float perc);///<车辆指针和无视车辆的概率值

            /// 设置强制靠右行驶的概率的方法
            void SetKeepRightPercentage(const ActorPtr& actor, const float percentage);///<车辆指针和保持靠右行驶的概率值

            /// 设置随机向左换道的概率的方法
            void SetRandomLeftLaneChangePercentage(const ActorPtr& actor, const float percentage);///<车辆指针和随机向左换道的概率值

            /// 设置随机向右换道的概率的方法
            void SetRandomRightLaneChangePercentage(const ActorPtr& actor, const float percentage);///<车辆指针和随机向右换道的概率值

            /// 设置是否自动更新车辆灯光状态的方法
            void SetUpdateVehicleLights(const ActorPtr& actor, const bool do_update);///<车辆指针和是否更新的布尔值

            /// 设置所有注册车辆应保持与前车的距离的方法
            void SetGlobalDistanceToLeadingVehicle(const float dist);///< 所有车辆应保持的距离值

            /// 设置同步模式超时时间
            void SetSynchronousModeTimeOutInMiliSecond(const double time);///< 超时时间值

            /// 设置混合物理模式的方法
            void SetHybridPhysicsMode(const bool mode_switch);///< 是否启用混合物理模式的布尔值

            /// 设置同步模式的方法
            void SetSynchronousMode(const bool mode_switch = true);///< 是否启用同步模式的布尔值，默认启用

            /// 设置混合物理半径的方法
            void SetHybridPhysicsRadius(const float radius);///< 混合物理半径值

            /// 设置Open Street Map模式的方法
            void SetOSMMode(const bool mode_switch);///< 是否启用OSM模式的布尔值

            /// 设置是否自动重生休眠车辆的方法
            void SetRespawnDormantVehicles(const bool mode_switch); ///< 是否启用的布尔值

            /// 设置重生休眠车辆的边界的方法
            void SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound);///< 下边界值和下边界值

            /// 设置重生休眠车辆时的边界限制的方法
            void SetMaxBoundaries(const float lower, const float upper);///< 下限值和下限值

            /// 设置自定义路径的方法
            void SetCustomPath(const ActorPtr& actor, const Path path, const bool empty_buffer);///< 车辆指针，路径数据和是否清空缓冲区的布尔值

            /// 移除一组点的方法
            void RemoveUploadPath(const ActorId& actor_id, const bool remove_path);///< 车辆ID和是否移除的布尔值

            /// 更新已设置的点列表的方法
            void UpdateUploadPath(const ActorId& actor_id, const Path path);///< 车辆ID和新的路径数据

            /// 设置自定义路线的方法
            void SetImportedRoute(const ActorPtr& actor, const Route route, const bool empty_buffer);///< 车辆指针，路线数据和是否清空缓冲区的布尔值

            /// 移除路线的方法
            void RemoveImportedRoute(const ActorId& actor_id, const bool remove_path);///<车辆ID和是否移除的布尔值

            /// 更新已设置路线的方法
            void UpdateImportedRoute(const ActorId& actor_id, const Route route);///< 车辆ID和新的路线数据

            ///////////////////////////////// 获取器 /////////////////////////////////////

            /// 获取混合物理半径的方法
            float GetHybridPhysicsRadius() const;

            /// 查询车辆目标速度的方法
            float GetVehicleTargetVelocity(const ActorId& actor_id, const float speed_limit) const;

            /// 查询车辆车道偏移量的方法
            float GetLaneOffset(const ActorId& actor_id) const;

            /// 查询一对车辆之间避碰规则的方法
            bool GetCollisionDetection(const ActorId& reference_actor_id, const ActorId& other_actor_id) const;

            ///查询车辆变道指令的方法
            ChangeLaneInfo GetForceLaneChange(const ActorId& actor_id);

            /// 查询车辆保持右侧规则的百分比概率的方法
            float GetKeepRightPercentage(const ActorId& actor_id);

            /// 查询车辆随机右变道百分比概率的方法
            float GetRandomLeftLaneChangePercentage(const ActorId& actor_id);

            ///查询车辆随机向左变道百分比概率的方法
            float GetRandomRightLaneChangePercentage(const ActorId& actor_id);

            /// 查询车辆自动变道规则的方法
            bool GetAutoLaneChange(const ActorId& actor_id) const;

            /// 查询给定车辆与前方车辆之间距离的方法
            float GetDistanceToLeadingVehicle(const ActorId& actor_id) const;

            /// 获取百分比以运行任何交通灯的方法
            float GetPercentageRunningSign(const ActorId& actor_id) const;

            /// 获取百分比以运行任何交通灯的方法
            float GetPercentageRunningLight(const ActorId& actor_id) const;

            /// 方法获取百分比以忽略任何车辆
            float GetPercentageIgnoreVehicles(const ActorId& actor_id) const;

            ///获取百分比以忽略任何步行者的方法
            float GetPercentageIgnoreWalkers(const ActorId& actor_id) const;

            /// 获取车辆灯光是否应自动更新的方法
            bool GetUpdateVehicleLights(const ActorId& actor_id) const;

            /// 获取同步模式的方法
            bool GetSynchronousMode() const;

            /// 获取同步模式超时
            double GetSynchronousModeTimeOutInMiliSecond() const;

            /// 获取混合物理模式的方法
            bool GetHybridPhysicsMode() const;

            /// 获取是否自动重生载具的方法
            bool GetRespawnDormantVehicles() const;

            /// 获取车辆重生时与英雄车辆之间最小距离的方法
            float GetLowerBoundaryRespawnDormantVehicles() const;

            /// 获取车辆重生时与英雄车辆之间最大距离的方法
            float GetUpperBoundaryRespawnDormantVehicles() const;

            /// 获取Open Street Map模式的方法
            bool GetOSMMode() const;

            /// 获取是否正在上传路径的方法
            bool GetUploadPath(const ActorId& actor_id) const;

            /// 获取自定义路径的方法
            Path GetCustomPath(const ActorId& actor_id) const;

            /// 获取是否正在上传路线的方法
            bool GetUploadRoute(const ActorId& actor_id) const;

            /// 获取自定义路由的方法
            Route GetImportedRoute(const ActorId& actor_id) const;

            /// 同步模式超时变量
            std::chrono::duration<double, std::milli> synchronous_time_out;
        };

    } // namespace traffic_manager
} // namespace carla
