
#pragma once

#include <memory>  // 引入智能指针头文件

#include "carla/trafficmanager/DataStructures.h"  // 引入数据结构定义
#include "carla/trafficmanager/InMemoryMap.h"  // 引入内存地图相关定义
#include "carla/trafficmanager/LocalizationUtils.h"  // 引入定位工具的定义
#include "carla/trafficmanager/Parameters.h"  // 引入参数设置相关定义
#include "carla/trafficmanager/RandomGenerator.h"  // 引入随机数生成器
#include "carla/trafficmanager/TrackTraffic.h"  // 引入交通跟踪相关定义
#include "carla/trafficmanager/SimulationState.h"  // 引入仿真状态相关定义
#include "carla/trafficmanager/Stage.h"  // 引入阶段类定义

namespace carla {
namespace traffic_manager {

namespace cc = carla::client;  // 定义命名空间别名

using LocalMapPtr = std::shared_ptr<InMemoryMap>;  // 定义指向 InMemoryMap 的智能指针类型
using LaneChangeSWptMap = std::unordered_map<ActorId, SimpleWaypointPtr>;  // 定义车道变更简易路径点映射
using WaypointPtr = carla::SharedPtr<cc::Waypoint>;  // 定义指向 Waypoint 的共享指针类型
using Action = std::pair<RoadOption, WaypointPtr>;  // 定义行动为道路选项与路径点的配对
using ActionBuffer = std::vector<Action>;  // 定义行动缓冲区为动作的向量
using Path = std::vector<cg::Location>;  // 定义路径为坐标位置的向量
using Route = std::vector<uint8_t>;  // 定义路线为无符号字节的向量

/// 此类功能是维护车辆前方的路径点以供跟随。
/// 该类还负责管理车道变更决策并适当地修改路径轨迹。
class LocalizationStage : Stage {
private:
  const std::vector<ActorId> &vehicle_id_list;  // 存储车辆 ID 列表
  BufferMap &buffer_map;  // 引用缓冲区映射
  const SimulationState &simulation_state;  // 引用仿真状态
  TrackTraffic &track_traffic;  // 引用交通跟踪
  const LocalMapPtr &local_map;  // 引用本地地图智能指针
  Parameters &parameters;  // 引用参数设置
  // 被标记为移除的车辆数组。
  std::vector<ActorId>& marked_for_removal;  // 存储待移除车辆的 ID 列表
  LocalizationFrame &output_array;  // 引用输出数组
  LaneChangeSWptMap last_lane_change_swpt;  // 最近的车道变更路径点映射
  ActorIdSet vehicles_at_junction;  // 存储在交叉口的车辆 ID 集合
  using SimpleWaypointPair = std::pair<SimpleWaypointPtr, SimpleWaypointPtr>;  // 定义简易路径点对
  std::unordered_map<ActorId, SimpleWaypointPair> vehicles_at_junction_entrance;  // 存储在交叉口入口的车辆及路径点对
  RandomGenerator &random_device;  // 引用随机数生成器

  // 分配车道变更路径点
  SimpleWaypointPtr AssignLaneChange(const ActorId actor_id,
                                     const cg::Location vehicle_location,
                                     const float vehicle_speed,
                                     bool force, bool direction);

  // 扩展并查找安全空间
  void ExtendAndFindSafeSpace(const ActorId actor_id,
                              const bool is_at_junction_entrance,
                              Buffer &waypoint_buffer);

  // 导入路径
  void ImportPath(Path &imported_path,
                  Buffer &waypoint_buffer,
                  const ActorId actor_id,
                  const float horizon_square);

  // 导入路线
  void ImportRoute(Route &imported_actions,
                  Buffer &waypoint_buffer,
                  const ActorId actor_id,
                  const float horizon_square);

public:
  // 构造函数
  LocalizationStage(const std::vector<ActorId> &vehicle_id_list,
                    BufferMap &buffer_map,
                    const SimulationState &simulation_state,
                    TrackTraffic &track_traffic,
                    const LocalMapPtr &local_map,
                    Parameters &parameters,
                    std::vector<ActorId>& marked_for_removal,
                    LocalizationFrame &output_array,
                    RandomGenerator &random_device);

  // 更新方法
  void Update(const unsigned long index) override;

  // 移除演员方法
  void RemoveActor(const ActorId actor_id) override;

  // 重置方法
  void Reset() override;

  // 计算下一个动作
  Action ComputeNextAction(const ActorId &actor_id);

  // 计算动作缓冲区
  ActionBuffer ComputeActionBuffer(const ActorId& actor_id);

};

} // namespace traffic_manager
} // namespace carla