
/// This file contains various constants used in traffic manager
/// arranged into sensible namespaces for re-usability across
/// different files.

#pragma once

#include <limits>
#include <stdint.h>
#include <iostream>
#include <vector>

#define SQUARE(a) ((a) * (a))
#define RATE(MaxY, MinY, DiffX) (((MaxY) - (MinY)) / (DiffX))

namespace carla {
namespace traffic_manager {
namespace constants {

namespace Networking {
static const uint64_t MIN_TRY_COUNT = 20u; // 最小尝试次数
static const unsigned short TM_DEFAULT_PORT = 8000u; // 交通管理默认端口
static const int64_t TM_TIMEOUT = 2000; // 超时时间（毫秒）
} // 命名空间 网络

namespace VehicleRemoval {
static const float STOPPED_VELOCITY_THRESHOLD = 0.8f; // 停止速度阈值
static const double BLOCKED_TIME_THRESHOLD = 90.0; // 被阻塞时间阈值
static const double RED_TL_BLOCKED_TIME_THRESHOLD = 180.0; // 红灯被阻塞时间阈值
static const double DELTA_TIME_BETWEEN_DESTRUCTIONS = 10.0; // 车辆销毁之间的时间间隔
} // namespace VehicleRemoval

namespace HybridMode {
static const float HYBRID_MODE_DT_FL = 0.05f; // 混合模式时的时间步长（浮点数）
static const double HYBRID_MODE_DT = 0.05; // 混合模式时的时间步长（双精度）
static const double INV_HYBRID_DT = 1.0 / HYBRID_MODE_DT; // 混合模式时间步长的倒数
static const float PHYSICS_RADIUS = 50.0f; // 物理半径
} // namespace HybridMode

namespace SpeedThreshold {
static const float HIGHWAY_SPEED = 60.0f / 3.6f; // 高速公路速度（米/秒）
static const float AFTER_JUNCTION_MIN_SPEED = 5.0f / 3.6f; // 交叉口后最小速度（米/秒）
static const float INITIAL_PERCENTAGE_SPEED_DIFFERENCE = 0.0f; // 初始速度百分比差异
} // namespace SpeedThreshold

namespace PathBufferUpdate {
static const float MAX_START_DISTANCE = 20.0f; // 最大起始距离
static const float MINIMUM_HORIZON_LENGTH = 15.0f; // 最小视野长度
static const float HORIZON_RATE = 2.0f; // 视野更新率
static const float HIGH_SPEED_HORIZON_RATE = 4.0f; // 高速视野更新率
} // namespace PathBufferUpdate

namespace WaypointSelection {
static const float TARGET_WAYPOINT_TIME_HORIZON = 0.3f; // 目标路径点时间视野
static const float MIN_TARGET_WAYPOINT_DISTANCE = 3.0f; // 最小目标路径点距离
static const float JUNCTION_LOOK_AHEAD = 5.0f; // 交叉口前瞻距离
static const float SAFE_DISTANCE_AFTER_JUNCTION = 4.0f; // 交叉口后安全距离
static const float MIN_JUNCTION_LENGTH = 8.0f; // 最小交叉口长度
static const float MIN_SAFE_INTERVAL_LENGTH = 0.5f * SAFE_DISTANCE_AFTER_JUNCTION; // 最小安全间隔长度
} // namespace WaypointSelection

namespace LaneChange {
static const float MINIMUM_LANE_CHANGE_DISTANCE = 20.0f; // 最小换道距离
static const float MAXIMUM_LANE_OBSTACLE_DISTANCE = 50.0f; // 最大车道障碍物距离
static const float MAXIMUM_LANE_OBSTACLE_CURVATURE = 0.6f; // 最大车道障碍物曲率
static const float INTER_LANE_CHANGE_DISTANCE = 10.0f; // 车道间换道距离
static const float MIN_WPT_DISTANCE = 5.0f; // 最小路径点距离
static const float MAX_WPT_DISTANCE = 20.0f; // 最大路径点距离
static const float MIN_LANE_CHANGE_SPEED = 5.0f; // 最小换道速度
static const float FIFTYPERC = 50.0f; // 50% 常量
} // namespace LaneChange

namespace Collision {
static const float BOUNDARY_EXTENSION_MINIMUM = 2.5f; // 边界扩展最小值
static const float BOUNDARY_EXTENSION_RATE = 4.35f; // 边界扩展速率
static const float COS_10_DEGREES = 0.9848f; // 10度的余弦值
static const float OVERLAP_THRESHOLD = 0.1f; // 重叠阈值
static const float LOCKING_DISTANCE_PADDING = 4.0f; // 锁定距离填充
static const float COLLISION_RADIUS_STOP = 8.0f; // 碰撞半径停止
static const float COLLISION_RADIUS_MIN = 20.0f; // 最小碰撞半径
static const float COLLISION_RADIUS_RATE = 2.65f; // 碰撞半径速率
static const float MAX_LOCKING_EXTENSION = 10.0f; // 最大锁定扩展
static const float WALKER_TIME_EXTENSION = 1.5f; // 行人时间扩展
static const float SQUARE_ROOT_OF_TWO = 1.414f; // √2
static const float VERTICAL_OVERLAP_THRESHOLD = 4.0f; // 垂直重叠阈值
static const float EPSILON = 2.0f * std::numeric_limits<float>::epsilon(); // 精度值
static const float MIN_REFERENCE_DISTANCE = 0.5f; // 最小参考距离
static const float MIN_VELOCITY_COLL_RADIUS = 2.0f; // 最小速度碰撞半径
static const float VEL_EXT_FACTOR = 0.36f; // 速度扩展因子
} // namespace Collision

namespace FrameMemory {
static const uint64_t INITIAL_SIZE = 50u; // 初始大小
static const uint64_t GROWTH_STEP_SIZE = 50u; // 增长步长
static const float INV_GROWTH_STEP_SIZE = 1.0f / static_cast<float>(GROWTH_STEP_SIZE); // 增长步长的倒数
} // namespace FrameMemory
namespace Map {
static const float INFINITE_DISTANCE = std::numeric_limits<float>::max(); // 无限距离
static const float MAX_GEODESIC_GRID_LENGTH = 20.0f; // 最大地理网格长度
static const float MAP_RESOLUTION = 5.0f; // 地图分辨率
static const float INV_MAP_RESOLUTION = 1.0f / MAP_RESOLUTION; // 地图分辨率的倒数
static const double MAX_WPT_DISTANCE = MAP_RESOLUTION/2.0 + SQUARE(MAP_RESOLUTION); // 最大路径点距离
static const float MAX_WPT_RADIANS = 0.087f;  // 最大路径点弧度（5º）
static float const DELTA = 25.0f; // 增量
static float const Z_DELTA = 500.0f; // Z轴增量
static float const STRAIGHT_DEG = 19.0f; // 直行角度
static const double MIN_LANE_WIDTH = 1.0f; // 最小车道宽度
} // namespace Map

namespace TrafficLight {
static const double MINIMUM_STOP_TIME = 2.0; // 最小停车时间
static const double EXIT_JUNCTION_THRESHOLD = 0;  // 退出交叉口阈值（90º的点积）
} // namespace TrafficLight

namespace MotionPlan {
static const float RELATIVE_APPROACH_SPEED = 12.0f / 3.6f; // 相对接近速度
static const float MIN_FOLLOW_LEAD_DISTANCE = 2.0f; // 最小跟随前车距离
static const float CRITICAL_BRAKING_MARGIN = 0.2f; // 临界刹车余量
static const float EPSILON_RELATIVE_SPEED = 0.001f; // 相对速度的微小值
static const float MAX_JUNCTION_BLOCK_DISTANCE = 1.0f * WaypointSelection::SAFE_DISTANCE_AFTER_JUNCTION; // 最大交叉口阻塞距离
static const float TWO_KM = 2000.0f; // 两公里
static const uint16_t ATTEMPTS_TO_TELEPORT = 5u; // 尝试传送次数
static const float LANDMARK_DETECTION_TIME = 3.5f; // 地标检测时间
static const float TL_TARGET_VELOCITY = 15.0f / 3.6f; // 交通灯目标速度
static const float STOP_TARGET_VELOCITY = 10.0f / 3.6f; // 停止目标速度
static const float YIELD_TARGET_VELOCITY = 10.0f / 3.6f; // 让行目标速度
static const float FRICTION = 0.6f; // 摩擦系数
static const float GRAVITY = 9.81f; // 重力加速度
static const float PI = 3.1415927f; // 圆周率
static const float PERC_MAX_SLOWDOWN = 0.08f; // 最大减速百分比
static const float FOLLOW_LEAD_FACTOR = 2.0f; // 跟随因子
} // namespace MotionPlan

namespace VehicleLight {
static const float SUN_ALTITUDE_DEGREES_BEFORE_DAWN = 15.0f; // 黎明前太阳高度角
static const float SUN_ALTITUDE_DEGREES_AFTER_SUNSET = 165.0f; // 日落后太阳高度角
static const float SUN_ALTITUDE_DEGREES_JUST_AFTER_DAWN = 35.0f; // 黎明后太阳高度角
static const float SUN_ALTITUDE_DEGREES_JUST_BEFORE_SUNSET = 145.0f; // 日落前太阳高度角
static const float HEAVY_PRECIPITATION_THRESHOLD = 80.0f; // 大降水阈值
static const float FOG_DENSITY_THRESHOLD = 20.0f; // 雾密度阈值
static const float MAX_DISTANCE_LIGHT_CHECK = 225.0f; // 最大光照检测距离
} // namespace VehicleLight

namespace PID {
static const float MAX_THROTTLE = 0.85f; // 最大油门
static const float MAX_BRAKE = 0.7f; // 最大刹车
static const float MAX_STEERING = 0.8f; // 最大转向
static const float MAX_STEERING_DIFF = 0.15f; // 最大转向差
static const float DT = 0.05f; // 时间步长
static const float INV_DT = 1.0f / DT; // 时间步长的倒数
static const std::vector<float> LONGITUDIAL_PARAM = {12.0f, 0.05f, 0.02f}; // 纵向参数
static const std::vector<float> LONGITUDIAL_HIGHWAY_PARAM = {20.0f, 0.05f, 0.01f}; // 高速公路纵向参数
static const std::vector<float> LATERAL_PARAM = {4.0f, 0.02f, 0.08f}; // 横向参数
static const std::vector<float> LATERAL_HIGHWAY_PARAM = {2.0f, 0.02f, 0.04f}; // 高速公路横向参数
} // namespace PID

namespace TrackTraffic {
static const uint64_t BUFFER_STEP_THROUGH = 5; // 缓冲步骤
static const float INV_BUFFER_STEP_THROUGH = 1.0f / static_cast<float>(BUFFER_STEP_THROUGH); // 缓冲步骤的倒数
} // namespace TrackTraffic

} // namespace constants
} // namespace traffic_manager
} // namespace carla
