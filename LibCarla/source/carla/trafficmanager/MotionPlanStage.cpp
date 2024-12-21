// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <limits>

#include "carla/client/TrafficSign.h"
#include "carla/client/TrafficLight.h"
#include "carla/rpc/TrafficLightState.h"

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/PIDController.h"

#include "carla/trafficmanager/MotionPlanStage.h"
// 定义carla空间下的traffic_manage空间
namespace carla {
namespace traffic_manager {
// 引入相关的元素
using namespace constants::MotionPlan;
using namespace constants::WaypointSelection;
using namespace constants::SpeedThreshold;
// 引入HybridMode相关的常量到当前作用域
using constants::HybridMode::HYBRID_MODE_DT;
using constants::HybridMode::HYBRID_MODE_DT_FL;
using constants::Collision::EPSILON;
//初始化类的各个成员变量
MotionPlanStage::MotionPlanStage(
  const std::vector<ActorId> &vehicle_id_list,// 车辆ID列表，标识车辆
  SimulationState &simulation_state,// 模拟状态对象
  const Parameters &parameters,//交通管理相关的参数
  const BufferMap &buffer_map,
  TrackTraffic &track_traffic,//跟踪交通状况
  const std::vector<float> &urban_longitudinal_parameters,//一个存储浮点数的常量引用类型的向量，推测是用于表示城市环境下纵向相关参数的数据（例如车辆在城市道路行驶时的加速度、减速度等纵向控制参数）
  const std::vector<float> &highway_longitudinal_parameters,//同样是存储浮点数的常量引用类型的向量，用于表示高速公路环境下纵向相关参数的数据（高速公路行驶时对应的纵向控制相关参数）
  const std::vector<float> &urban_lateral_parameters,// 存储浮点数的常量引用类型的向量，应该是涉及城市环境下车辆横向相关参数的数据（比如车辆在城市道路转弯时的相关横向控制参数等）
  const std::vector<float> &highway_lateral_parameters,//存储浮点数的常量引用类型的向量，代表高速公路环境下车辆横向相关参数的数据（高速公路场景下车辆横向操作相关参数）
  const LocalizationFrame &localization_frame,//一个 LocalizationFrame 类型的参数，可能用于提供车辆的定位相关信息（例如车辆在地图中的位置坐标、姿态等定位数据结构）
  const CollisionFrame&collision_frame,//类型的参数，推测是用于获取车辆碰撞相关信息的数据结构（比如是否检测到碰撞风险、碰撞方向等信息）
  const TLFrame &tl_frame,//类型的参数，可能与交通信号灯（Traffic Light）相关信息有关，例如当前车辆是否需要根据信号灯状态做出停车、启动等操作判断的数据来源
  const cc::World &world,//应该是表示整个模拟世界的对象，通过它可以获取世界的各种状态信息（如时间、全局地图等）
  ControlFrame &output_array,// 用于存储后续计算得到的控制输出结果（例如车辆的转向角度、油门刹车控制量等信息，以便应用到车辆模拟控制中）
  RandomGenerator &random_device,// ，可能用于生成随机数，在一些涉及随机性的模拟决策场景（比如随机选择路径、随机应对突发情况等）中会用到
  const LocalMapPtr &local_map)// - local_map：LocalMapPtr 类型的常量引用，应该是指向局部地图信息的指针，用于获取车辆周围局部区域的地图数据（例如周边道路情况、障碍物分布等信息辅助决策）
    : vehicle_id_list(vehicle_id_list),
    simulation_state(simulation_state),
    parameters(parameters),
    buffer_map(buffer_map),
    track_traffic(track_traffic),
    urban_longitudinal_parameters(urban_longitudinal_parameters),
    highway_longitudinal_parameters(highway_longitudinal_parameters),
    urban_lateral_parameters(urban_lateral_parameters),
    highway_lateral_parameters(highway_lateral_parameters),
    localization_frame(localization_frame),
    collision_frame(collision_frame),
    tl_frame(tl_frame),
    world(world),
    output_array(output_array),
    random_device(random_device),
    local_map(local_map) {}
// 定义名为 Update 的成员函数，它属于 MotionPlanStage 类，用于更新相关状态信息或者执行一些基于当前状态的计算操作
// 参数 index：一个无符号长整型参数，可能用于在一些容器（比如存储车辆相关信息的数组或向量等）中定位特定车辆对应的索引位置，从而获取该车辆的相关信息进行后续处理
void MotionPlanStage::Update(const unsigned long index) {    
  const ActorId actor_id = vehicle_id_list.at(index); // 根据传入的索引 index，从 vehicle_id_list 中获取对应的车辆 ID（ActorId 类型，可能是用于唯一标识模拟中的车辆等角色的类型）
  const cg::Location vehicle_location = simulation_state.GetLocation(actor_id); // 通过 simulation_state 对象，根据获取到的车辆 ID（actor_id）获取车辆当前的位置信息（cg::Location 类型，可能包含了车辆在三维空间中的坐标等位置相关数据）
    const cg::Location vehicle_location = simulation_state.GetLocation(actor_id);    // 同样通过 simulation_state 对象，依据车辆 ID 获取车辆当前的速度信息（cg::Vector3D 类型，以三维向量形式表示速度的大小和方向）
  const cg::Vector3D vehicle_velocity = simulation_state.GetVelocity(actor_id); 
// 通过 simulation_state 对象，按照车辆 ID 获取车辆当前的旋转状态信息（cg::Rotation 类型，可能涉及车辆在空间中的朝向角度等旋转相关数据）
  const cg::Rotation vehicle_rotation = simulation_state.GetRotation(actor_id);// 通过 simulation_state 对象，按照车辆 ID 获取车辆当前的旋转状态信息（cg::Rotation 类型，可能涉及车辆在空间中的朝向角度等旋转相关数据）
  const float vehicle_speed = vehicle_velocity.Length();// 计算车辆当前的速度大小（标量值），通过调用 vehicle_velocity 的 Length 函数获取其长度（即速度大小），这里的速度单位可能根据具体模拟场景设定（比如米/秒等）
  const cg::Vector3D vehicle_heading = simulation_state.GetHeading(actor_id);// 通过 simulation_state 对象，依据车辆 ID 获取车辆当前的行驶方向信息（cg::Vector3D 类型，以三维向量形式表示车辆车头的朝向方向）
  const bool vehicle_physics_enabled = simulation_state.IsPhysicsEnabled(actor_id); // 通过 simulation_state 对象，根据车辆 ID 判断车辆的物理模拟是否启用（返回布尔值，例如在某些模拟场景中车辆可能处于暂停物理模拟或者只做轨迹演示等情况时物理模拟是关闭的）
  const float vehicle_speed_limit = simulation_state.GetSpeedLimit(actor_id);    // 通过 simulation_state 对象，按照车辆 ID 获取车辆当前所在位置的速度限制信息（返回浮点数，例如该路段规定的最大行驶速度，单位可能根据模拟场景设定）
  const Buffer &waypoint_buffer = buffer_map.at(actor_id); // 根据车辆 ID，从 buffer_map 中获取对应的缓冲区数据（Buffer 类型，具体缓冲区的作用可能与车辆的路径规划、临时存储一些周边环境信息等相关，取决于具体实现）
  const LocalizationData &localization = localization_frame.at(index);    
  // 根据传入的索引 index，从 localization_frame 中获取对应的车辆定位数据（LocalizationData 类型，包含更详细的车辆定位相关信息，比如定位精度、定位方式等补充数据）
  const CollisionHazardData &collision_hazard = collision_frame.at(index);  // 根据传入的索引 index，从 collision_frame 中获取对应的车辆碰撞危险数据（CollisionHazardData 类型，包含车辆周围是否存在碰撞风险、碰撞危险程度等相关详细信息）
  const bool &tl_hazard = tl_frame.at(index);// 根据传入的索引 index，从 tl_frame 中获取对应的交通信号灯相关危险信息（返回布尔值，用于判断当前车辆是否面临因交通信号灯产生的危险情况，比如即将闯红灯等）
  current_timestamp = world.GetSnapshot().GetTimestamp();  // 获取当前世界的时间戳信息，通过 world 对象调用 GetSnapshot 函数获取世界的一个快照（可能包含了当前世界的各种状态信息），再从快照中获取时间戳（用于记录当前操作或者决策所处的时间点）
  StateEntry current_state;// 这里声明了一个 StateEntry 类型的变量 current_state，但后续代码缺失，不清楚具体用途，可能用于记录当前车辆或者整个模拟系统的某种状态信息，等待进一步赋值和使用

  // 实例化传送变换为当前载具变换
  cg::Transform teleportation_transform = cg::Transform(vehicle_location, vehicle_rotation);

  // 从actor_id状态中获取英雄位置信息
  cg::Location hero_location = track_traffic.GetHeroLocation();
  bool is_hero_alive = hero_location != cg::Location(0, 0, 0);

  if (simulation_state.IsDormant(actor_id) && parameters.GetRespawnDormantVehicles() && is_hero_alive) {
    // 冲洗车辆的控制器状态
    current_state = {current_timestamp,
                    0.0f, 0.0f,
                    0.0f};

    // 如果表中不存在，则将条目添加到传送持续时间时钟表中
    if (teleportation_instance.find(actor_id) == teleportation_instance.end()) {
      teleportation_instance.insert({actor_id, current_timestamp});
    }

    // 获取传送载具的下限和上限
    float lower_bound = parameters.GetLowerBoundaryRespawnDormantVehicles();
    float upper_bound = parameters.GetUpperBoundaryRespawnDormantVehicles();
    float dilate_factor = (upper_bound-lower_bound)/100.0f;

    // 测量车辆自上次传送以来所经过的时间
    double elapsed_time = current_timestamp.elapsed_seconds - teleportation_instance.at(actor_id).elapsed_seconds;

    if (parameters.GetSynchronousMode() || elapsed_time > HYBRID_MODE_DT) {
      float random_sample = (static_cast<float>(random_device.next())*dilate_factor) + lower_bound;
      NodeList teleport_waypoint_list = local_map->GetWaypointsInDelta(hero_location, ATTEMPTS_TO_TELEPORT, random_sample);
      if (!teleport_waypoint_list.empty()) {
        for (auto &teleport_waypoint : teleport_waypoint_list) {
          GeoGridId geogrid_id = teleport_waypoint->GetGeodesicGridId();
          if (track_traffic.IsGeoGridFree(geogrid_id)) {
            teleportation_transform = teleport_waypoint->GetTransform();
            teleportation_transform.location.z += 0.5f;
            track_traffic.AddTakenGrid(geogrid_id, actor_id);
            break;
          }
        }
      }
    }
    output_array.at(index) = carla::rpc::Command::ApplyTransform(actor_id, teleportation_transform);

    // 在传送车辆后，使用新的变换更新模拟状态
    KinematicState kinematic_state{teleportation_transform.location,
                                   teleportation_transform.rotation,
                                   vehicle_velocity, vehicle_speed_limit,
                                   vehicle_physics_enabled, simulation_state.IsDormant(actor_id),
                                   teleportation_transform.location};
    simulation_state.UpdateKinematicState(actor_id, kinematic_state);
  }

  else {

    // 目标车速
    float max_target_velocity = parameters.GetVehicleTargetVelocity(actor_id, vehicle_speed_limit) / 3.6f;

    // 接近地标时减速的算法
    float max_landmark_target_velocity = GetLandmarkTargetVelocity(*(waypoint_buffer.at(0)), vehicle_location, actor_id, max_target_velocity);

    // 转弯处减速算法
    float max_turn_target_velocity = GetTurnTargetVelocity(waypoint_buffer, max_target_velocity);
    max_target_velocity = std::min(std::min(max_target_velocity, max_landmark_target_velocity), max_turn_target_velocity);

    // 碰撞处理与目标速度修正
    std::pair<bool, float> collision_response = CollisionHandling(collision_hazard, tl_hazard, vehicle_velocity,
                                                                  vehicle_heading, max_target_velocity);
    bool collision_emergency_stop = collision_response.first;
    float dynamic_target_velocity = collision_response.second;

    // 如果路口后没有足够的空间，请勿进入路口
    bool safe_after_junction = SafeAfterJunction(localization, tl_hazard, collision_emergency_stop);

    // 遇到碰撞或交通灯危险时
    bool emergency_stop = tl_hazard || collision_emergency_stop || !safe_after_junction;

    if (vehicle_physics_enabled && !simulation_state.IsDormant(actor_id)) {// 判断车辆的物理模拟是否启用（vehicle_physics_enabled为true表示启用），并且车辆是否处于休眠状态（!simulation_state.IsDormant(actor_id)表示非休眠状态）
      ActuationSignal actuation_signal{0.0f, 0.0f, 0.0f};// 创建一个ActuationSignal类型的结构体（或类）对象actuation_signal，并初始化为{0.0f, 0.0f, 0.0f}

      const float target_point_distance = std::max(vehicle_speed * TARGET_WAYPOINT_TIME_HORIZON,
                                                  MIN_TARGET_WAYPOINT_DISTANCE);// 计算目标点距离，取车辆速度乘以TARGET_WAYPOINT_TIME_HORIZON（目标路点时间范围，可能表示预测的未来某个时间段）
      const SimpleWaypointPtr &target_waypoint = GetTargetWaypoint(waypoint_buffer, target_point_distance).first;// 调用GetTargetWaypoint函数，传入路点缓冲区（waypoint_buffer）和刚计算出的目标点距离（target_point_distance），
      cg::Location target_location = target_waypoint->GetLocation();    // 获取目标路点的位置信息（cg::Location类型，可能包含三维坐标等位置相关数据），赋值给target_location变量

      float offset = parameters.GetLaneOffset(actor_id); // 获取车辆在车道上的偏移量，通过parameters对象调用GetLaneOffset函数，传入车辆ID（actor_id）获取对应的车道偏移量，
      auto right_vector = target_waypoint->GetTransform().GetRightVector();    // 获取目标路点的右方向向量（GetRightVector函数返回的可能是表示路点所在位置的右侧方向的三维向量，用于确定横向方向），
      auto offset_location = cg::Location(cg::Vector3D(offset*right_vector.x, offset*right_vector.y, 0.0f));// 根据车道偏移量和右方向向量计算出偏移后的位置向量，通过将偏移量与右方向向量的各分量相乘构建一个新的三维向量，
      target_location = target_location + offset_location;// 将之前获取的目标位置（target_location）加上计算出的偏移位置（offset_location），得到考虑车道偏移后的实际目标位置
    // 这样车辆后续的行驶目标就会根据车道偏移情况进行相应调整

      float dot_product = DeviationDotProduct(vehicle_location, vehicle_heading, target_location);// 调用DeviationDotProduct函数，传入车辆当前位置（vehicle_location）、车辆行驶方向（vehicle_heading）和调整后的目标位置（target_location），
      float cross_product = DeviationCrossProduct(vehicle_location, vehicle_heading, target_location);// 调用DeviationCrossProduct函数，传入同样的三个参数，计算它们之间的叉积（叉积可以得到一个与两个输入向量都垂直的向量，其大小等信息在某些场景下可用于判断方向关系等），
      dot_product = acos(dot_product) / PI; // 对计算出的点积结果进行处理，通过调用acos函数（反余弦函数）将点积转换为对应的夹角弧度值（因为点积与向量夹角余弦值相关）
      if (cross_product < 0.0f) {  
// 如果叉积小于0.0f，说明车辆相对目标位置在某个特定的方向关系上（可能是根据叉积正负判断的左右方向等情况，具体取决于向量定义和坐标系设定）
    // 则将点积乘以 -1.0f，可能是为了统一方向偏差的表示方式，使得点积值能更准确地反映符合预期逻辑的角度偏差情况
        dot_product *= -1.0f;
      }
      const float angular_deviation = dot_product; // 将处理后的点积值赋值给angular_deviation变量，从变量名推测它表示车辆与目标位置之间的角度偏差
      const float velocity_deviation = (dynamic_target_velocity - vehicle_speed) / dynamic_target_velocity; 
// 计算速度偏差，用动态目标速度（dynamic_target_velocity，可能是根据路况、规划等因素设定的车辆期望达到的目标速度）减去车辆当前速度（vehicle_speed）
      // 如果未找到车辆的上一个状态，则初始化状态条目
      if (pid_state_map.find(actor_id) == pid_state_map.end()) {
        const auto initial_state = StateEntry{current_timestamp, 0.0f, 0.0f, 0.0f};
        pid_state_map.insert({actor_id, initial_state});
      }

      // 检索先前状态
      traffic_manager::StateEntry previous_state;
      previous_state = pid_state_map.at(actor_id);

      // 选择PID参数
      std::vector<float> longitudinal_parameters;
      std::vector<float> lateral_parameters;
      if (vehicle_speed > HIGHWAY_SPEED) {
        longitudinal_parameters = highway_longitudinal_parameters;
        lateral_parameters = highway_lateral_parameters;
      } else {
        longitudinal_parameters = urban_longitudinal_parameters;
        lateral_parameters = urban_lateral_parameters;
      }

      //如果为车辆启用了物理效果，请使用PID控制器
      // 车辆状态更新
      current_state = {current_timestamp, angular_deviation, velocity_deviation, 0.0f};

      // 控制器驱动
      actuation_signal = PID::RunStep(current_state, previous_state,
                                      longitudinal_parameters, lateral_parameters);

      if (emergency_stop) {
        actuation_signal.throttle = 0.0f;
        actuation_signal.brake = 1.0f;
      }

      // 构建执行信号

      carla::rpc::VehicleControl vehicle_control;
      vehicle_control.throttle = actuation_signal.throttle;
      vehicle_control.brake = actuation_signal.brake;
      vehicle_control.steer = actuation_signal.steer;

      output_array.at(index) = carla::rpc::Command::ApplyVehicleControl(actor_id, vehicle_control);

      // 更新PID状态
      current_state.steer = actuation_signal.steer;
      StateEntry &state = pid_state_map.at(actor_id);
      state = current_state;
    }
    // 对于无物理特性的载具，确定传送时的位置和方向
    else {
      // 冲洗车辆的控制器状态
      current_state = {current_timestamp,
                      0.0f, 0.0f,
                      0.0f};

      // 如果不在表中，则将条目添加到传送持续时间时钟表中
      if (teleportation_instance.find(actor_id) == teleportation_instance.end()) {
        teleportation_instance.insert({actor_id, current_timestamp});
      }

      // 测量车辆自上次传送以来的时间
      double elapsed_time = current_timestamp.elapsed_seconds - teleportation_instance.at(actor_id).elapsed_seconds;

      // 在车辆前方找到一个传送位置，以实现预期的速度
      if (!emergency_stop && (parameters.GetSynchronousMode() || elapsed_time > HYBRID_MODE_DT)) {

        // 目标位移量以达到目标速度
        const float target_displacement = dynamic_target_velocity * HYBRID_MODE_DT_FL;
        SimpleWaypointPtr teleport_target = waypoint_buffer.front();
        cg::Transform target_base_transform = teleport_target->GetTransform();
        cg::Location target_base_location = target_base_transform.location;
        cg::Vector3D target_heading = target_base_transform.GetForwardVector();
        cg::Vector3D correct_heading = (target_base_location - vehicle_location).MakeSafeUnitVector(EPSILON);

        if (vehicle_location.Distance(target_base_location) < target_displacement) {
          cg::Location teleportation_location = vehicle_location + cg::Location(target_heading.MakeSafeUnitVector(EPSILON) * target_displacement);
          teleportation_transform = cg::Transform(teleportation_location, target_base_transform.rotation);
        }
        else {
          cg::Location teleportation_location = vehicle_location + cg::Location(correct_heading * target_displacement);
          teleportation_transform = cg::Transform(teleportation_location, target_base_transform.rotation);
        }
      // 在紧急停止的情况下，请保持在同一位置
      // 此外，在异步模式下，每 dt 时间仅传送一次
      } else {
        teleportation_transform = cg::Transform(vehicle_location, simulation_state.GetRotation(actor_id));
      }
      // 构建执行信号
      output_array.at(index) = carla::rpc::Command::ApplyTransform(actor_id, teleportation_transform);
      simulation_state.UpdateKinematicHybridEndLocation(actor_id, teleportation_transform.location);
    }
  }
}

bool MotionPlanStage::SafeAfterJunction(const LocalizationData &localization,
                                        const bool tl_hazard,
                                        const bool collision_emergency_stop) {// MotionPlanStage类中的成员函数SafeAfterJunction，用于判断车辆在经过路口后是否处于安全状态

  SimpleWaypointPtr junction_end_point = localization.junction_end_point; 
// 获取LocalizationData中的路口端点指针（SimpleWaypointPtr类型，可能指向表示路口结束位置的路点对象，用于后续位置相关的判断和计算）
  SimpleWaypointPtr safe_point = localization.safe_point;// 获取LocalizationData中的安全点指针（SimpleWaypointPtr类型，可能是经过路口后被认定为安全的一个位置对应的路点对象）

  bool safe_after_junction = true; // 初始化一个布尔变量，表示车辆经过路口后是否安全，初始值设为true，后续会根据各种条件判断来更新这个值
  if (!tl_hazard && !collision_emergency_stop
      && localization.is_at_junction_entrance
      && junction_end_point != nullptr && safe_point != nullptr
      && junction_end_point->DistanceSquared(safe_point) > SQUARE(MIN_SAFE_INTERVAL_LENGTH)) {
		  // 判断以下多个条件是否同时满足：
    // 1. 不存在交通信号灯危险（!tl_hazard为true）
    // 2. 没有因碰撞触发紧急停车（!collision_emergency_stop为true）
    // 3. 车辆当前处于路口入口（localization.is_at_junction_entrance为true）
    // 4. 路口端点指针和安全点指针都不为空（junction_end_point!= nullptr && safe_point!= nullptr）
    // 5. 路口端点与安全点之间的距离平方大于最小安全间隔长度的平方（MIN_SAFE_INTERVAL_LENGTH应该是预定义的一个最小安全距离值，这里通过比较距离平方避免开方运算，提高效率）

    ActorIdSet passing_safe_point = track_traffic.GetPassingVehicles(safe_point->GetId()); 
// 获取经过安全点的车辆ID集合，通过track_traffic对象调用GetPassingVehicles函数，传入安全点的ID（safe_point->GetId()）来获取
    ActorIdSet passing_junction_end_point = track_traffic.GetPassingVehicles(junction_end_point->GetId()); 
// 获取经过路口端点的车辆ID集合，同样通过track_traffic对象调用GetPassingVehicles函数，传入路口端点的ID（junction_end_point->GetId()）来获取
    cg::Location mid_point = (junction_end_point->GetLocation() + safe_point->GetLocation())/2.0f;
	// 计算路口端点位置和安全点位置的中点，通过将两个位置对应的cg::Location类型向量相加后除以2.0f得到
    // 只检查那些在其通过路径点中具有安全点的车辆，但不连接端点
    ActorIdSet difference;// 定义一个ActorIdSet类型的变量difference，用于存储只经过安全点但不经过路口端点的车辆ID集合
    std::set_difference(passing_safe_point.begin(), passing_safe_point.end(),// 使用std::set_difference算法，找出在passing_safe_point集合中但不在passing_junction_end_point集合中的元素
                        passing_junction_end_point.begin(), passing_junction_end_point.end(),
                        std::inserter(difference, difference.begin()));
						 
    if (difference.size() > 0) {// 如果difference集合的大小大于0，说明存在这样的车辆（只经过安全点但不经过路口端点），需要进一步检查它们是否会影响本车安全
      for (const ActorId &blocking_id: difference) {// 遍历difference集合中的每一个车辆ID（ActorId类型）
        cg::Location blocking_actor_location = simulation_state.GetLocation(blocking_id);// 获取该车辆（blocking_id对应的车辆）的位置信息（cg::Location类型），通过simulation_state对象调用GetLocation函数来获取
        if (cg::Math::DistanceSquared(blocking_actor_location, mid_point) < SQUARE(MAX_JUNCTION_BLOCK_DISTANCE) 
            && simulation_state.GetVelocity(blocking_id).SquaredLength() < SQUARE(AFTER_JUNCTION_MIN_SPEED)) {// 判断该车辆与前面计算出的中点位置之间的距离平方是否小于最大路口阻塞距离的平方（MAX_JUNCTION_BLOCK_DISTANCE是预定义的一个距离阈值
          safe_after_junction = false; // 如果满足上述距离和速度条件，说明该车辆可能会对本车安全通过路口后造成阻碍，将safe_after_junction设为false，表示不安全
          break;  // 一旦发现存在可能影响安全的车辆，就跳出循环，因为只要有一辆这样的车存在就足以判定不安全了
        }
      }
    }
  }

  return safe_after_junction;  
// 返回表示车辆经过路口后是否安全的布尔值，外部调用者可以根据这个返回值来决定后续的操作（比如是否调整车辆行驶计划等）
}

std::pair<bool, float> MotionPlanStage::CollisionHandling(const CollisionHazardData &collision_hazard,
                                                          const bool tl_hazard,
                                                          const cg::Vector3D vehicle_velocity,
                                                          const cg::Vector3D vehicle_heading,
                                                          const float max_target_velocity) {// MotionPlanStage类中的成员函数CollisionHandling，用于处理车辆碰撞相关情况，并返回关于碰撞处理的相关信息（是否触发紧急停车以及动态目标速度等）
  bool collision_emergency_stop = false; // 初始化一个布尔变量，表示是否因碰撞危险触发紧急停车，初始值设为false，后续会根据碰撞相关条件判断来更新这个值
  float dynamic_target_velocity = max_target_velocity; 
// 初始化一个浮点数变量，表示动态目标速度，初始值设为最大目标速度（max_target_velocity），后续会根据碰撞情况等因素进行调整
  const float vehicle_speed = vehicle_velocity.Length();// 获取车辆当前速度的大小（标量值），通过调用vehicle_velocity的Length函数获取其长度，即当前的速度大小，单位可能根据具体模拟场景设定（比如米/秒等）

  if (collision_hazard.hazard && !tl_hazard) {// 判断是否存在碰撞危险（collision_hazard.hazard为true）且不存在交通信号灯危险（!tl_hazard为true），如果满足这两个条件，则进入后续的碰撞处理相关计算和判断
    const ActorId other_actor_id = collision_hazard.hazard_actor_id;     // 获取涉及碰撞危险的其他车辆的ID（ActorId类型），从collision_hazard对象的hazard_actor_id成员变量获取
    const cg::Vector3D other_velocity = simulation_state.GetVelocity(other_actor_id);// 获取其他车辆（other_actor_id对应的车辆）的速度向量（cg::Vector3D类型），通过simulation_state对象调用GetVelocity函数来获取
    const float vehicle_relative_speed = (vehicle_velocity - other_velocity).Length();  
// 计算本车与其他车辆的相对速度大小，通过将两车的速度向量相减（vehicle_velocity - other_velocity）后取其长度（Length函数）得到相对速度大小
        // 相对速度在判断碰撞的严重程度、采取何种应对措施等方面可能会起到重要作用
    const float available_distance_margin = collision_hazard.available_distance_margin;// 获取可用的距离余量信息（可能是距离碰撞还有多远的余量等相关信息，具体含义取决于CollisionHazardData结构体中该成员变量的定义）

    const float other_speed_along_heading = cg::Math::Dot(other_velocity, vehicle_heading);

    //只有在存在正相对速度的情况下，才考虑避撞决策
    // 自车（即，自车正在缩小与前车的间距）
    if (vehicle_relative_speed > EPSILON_RELATIVE_SPEED) {
      // 如果其他车辆正在接近领头车辆，并且领头车辆距离更远
      // 跟随距离：0公里/小时 -> 5米，100公里/小时 -> 10米
      float follow_lead_distance = FOLLOW_LEAD_FACTOR * vehicle_speed + MIN_FOLLOW_LEAD_DISTANCE;
      if (available_distance_margin > follow_lead_distance) {
        //然后缩小车辆之间的距离，直到达到跟随前车的距离
        // 通过保持与其他_沿航向_速度的相对速度
        dynamic_target_velocity = other_speed_along_heading;
      }
      // 如果车辆正在接近前车，而前车距离较远
      // 但比CRITICAL_BRAKING_MARGIN更近，且比FOLLOW_LEAD_DISTANCE更近
      else if (available_distance_margin > CRITICAL_BRAKING_MARGIN) {
        // 然后跟随前导车辆，获取其沿当前航向的速度
        dynamic_target_velocity = std::max(other_speed_along_heading, RELATIVE_APPROACH_SPEED);
      } else {
        // 如果前车距离小于紧急制动距离，则启动紧急制动
        collision_emergency_stop = true;
      }
    }
    if (available_distance_margin < CRITICAL_BRAKING_MARGIN) {
      collision_emergency_stop = true;
    }
  }

  float max_gradual_velocity = PERC_MAX_SLOWDOWN * vehicle_speed;
  if (dynamic_target_velocity < vehicle_speed - max_gradual_velocity) {
    // 不要每帧减速超过PERC_MAX_SLOWDOWN
    dynamic_target_velocity = vehicle_speed - max_gradual_velocity;
  }
  dynamic_target_velocity = std::min(max_target_velocity, dynamic_target_velocity);

  return {collision_emergency_stop, dynamic_target_velocity};
}

float MotionPlanStage::GetLandmarkTargetVelocity(const SimpleWaypoint& waypoint,
                                                 const cg::Location vehicle_location,
                                                 const ActorId actor_id,
                                                 float max_target_velocity) {// MotionPlanStage类中的成员函数GetLandmarkTargetVelocity，用于根据路点信息、车辆位置以及其他相关条件，获取基于地标（landmark）的目标速度

    auto const max_distance = LANDMARK_DETECTION_TIME * max_target_velocity; 
// 计算地标检测的最大距离，通过将地标检测时间（LANDMARK_DETECTION_TIME，可能是预设的一个用于检测地标有效范围的时间值）乘以最大目标速度得到，

    float landmark_target_velocity = std::numeric_limits<float>::max(); 
// 初始化地标目标速度为正无穷大（std::numeric_limits<float>::max()），后续会在循环中根据具体地标情况更新为合理的值

    auto all_landmarks = waypoint.GetWaypoint()->GetAllLandmarksInDistance(max_distance, false); 
// 初始化地标目标速度为正无穷大（std::numeric_limits<float>::max()），后续会在循环中根据具体地标情况更新为合理的值


    for (auto &landmark: all_landmarks) {// 遍历获取到的所有地标

      auto landmark_location = landmark->GetWaypoint()->GetTransform().location;// 获取地标所在位置信息（cg::Location类型），通过先获取地标对应的路点（landmark->GetWaypoint()），再获取其变换信息中的位置信息（GetTransform().location）得到
      auto landmark_type = landmark->GetType();// 获取地标类型信息（可能是一个字符串标识，用于区分不同种类的地标，比如交通信号灯、停止标志等），通过调用landmark的GetType函数获取
      auto distance = landmark_location.Distance(vehicle_location);// 计算地标位置与车辆当前位置之间的距离，调用Distance函数来获取，单位可能根据具体场景设定（比如米等）

      if (distance > max_distance) { 
// 如果地标与车辆的距离大于最大距离，说明该地标不在本次考虑的有效范围内，直接跳过本次循环，继续检查下一个地标
        continue;
      }

      float minimum_velocity = max_target_velocity;// 初始化一个最小速度值为最大目标速度，后续会根据地标类型进行调整，用于表示基于当前地标情况下车辆应达到的最小允许速度
      if (landmark_type == "1000001") {  // 交通信号灯
        minimum_velocity = TL_TARGET_VELOCITY;  // 将最小速度设置为交通信号灯对应的目标速度（TL_TARGET_VELOCITY，应该是预定义的一个针对交通信号灯场景下车辆合适速度的常量值）
      } else if (landmark_type == "206") {  // 停止
        minimum_velocity = STOP_TARGET_VELOCITY; // 将最小速度设置为停止标志对应的目标速度（STOP_TARGET_VELOCITY，同样是预定义的适合在停止标志场景下的车辆速度常量值）
      } else if (landmark_type == "205") {  // 产出
        minimum_velocity = YIELD_TARGET_VELOCITY;// 将最小速度设置为让行标志对应的目标速度（YIELD_TARGET_VELOCITY，预定义的在让行场景下的车辆合适速度常量值）
      } else if (landmark_type == "274") {  // 速度限制
        float value = static_cast<float>(landmark->GetValue()) / 3.6f;
        value = parameters.GetVehicleTargetVelocity(actor_id, value);// 根据车辆ID（actor_id）和获取到的速度值，调用parameters对象的GetVehicleTargetVelocity函数进一步调整速度值，
        minimum_velocity = (value < max_target_velocity) ? value : max_target_velocity;// 取调整后的速度值和最大目标速度中的较小值作为最小速度，确保不超过最大目标速度限制
      } else { 
// 如果地标类型不属于上述已知的类型，直接跳过本次循环，不考虑该地标对目标速度的影响
        continue;
      }

      float v = std::max(((max_target_velocity - minimum_velocity) / max_distance) * distance + minimum_velocity, minimum_velocity);// 根据距离、最大目标速度以及最小速度计算一个基于当前地标的速度值v，计算公式可能基于某种线性关系或者项目特定的规则
      landmark_target_velocity = std::min(landmark_target_velocity, v); 
// 取计算出的基于当前地标的速度值v和之前记录的地标目标速度（landmark_target_velocity）中的较小值，更新地标目标速度
    }

    return landmark_target_velocity;// 返回最终确定的基于地标情况的目标速度，外部调用者可以根据这个速度值来调整车辆的行驶速度计划等操作
}

float MotionPlanStage::GetTurnTargetVelocity(const Buffer &waypoint_buffer,// MotionPlanStage类中的成员函数GetTurnTargetVelocity，用于根据路点缓冲区信息计算车辆在转弯处的目标速度
                                             float max_target_velocity) {

  if (waypoint_buffer.size() < 3) { // 判断路点缓冲区中的路点数量是否小于3，如果小于3，说明可能路点信息不足，无法准确计算转弯相关参数，直接返回最大目标速度
    return max_target_velocity;
  }
  else {
    const SimpleWaypointPtr first_waypoint = waypoint_buffer.front();// 获取路点缓冲区中的第一个路点指针（SimpleWaypointPtr类型，指向表示路点的对象，可能包含路点位置、方向等相关信息）
    const SimpleWaypointPtr last_waypoint = waypoint_buffer.back();  // 获取路点缓冲区中的最后一个路点指针
    const SimpleWaypointPtr middle_waypoint = waypoint_buffer.at(static_cast<uint16_t>(waypoint_buffer.size() / 2)); 
// 获取路点缓冲区中间位置的路点指针，通过将缓冲区大小除以2（转换为合适的无符号16位整数类型）并以此索引获取路点，

    float radius = GetThreePointCircleRadius(first_waypoint->GetLocation(),
                                             middle_waypoint->GetLocation(),
                                             last_waypoint->GetLocation());

    // 返回转弯处的最大速度
    return std::sqrt(radius * FRICTION * GRAVITY);
  }
}

float MotionPlanStage::GetThreePointCircleRadius(cg::Location first_location,// MotionPlanStage类中的成员函数GetThreePointCircleRadius，用于根据给定的三个点的位置信息，计算这三个点所确定的圆的半径（在二维平面几何场景下）
                                                 cg::Location middle_location,
                                                 cg::Location last_location) {

    float x1 = first_location.x;// 获取第一个点的x坐标值
    float y1 = first_location.y;  // 获取第一个点的y坐标值
    float x2 = middle_location.x;  // 获取中间点的x坐标值
    float y2 = middle_location.y; // 获取中间点的y坐标值
    float x3 = last_location.x;    // 获取最后一个点的x坐标值
    float y3 = last_location.y;// 获取最后一个点的y坐标值

    float x12 = x1 - x2;  // 计算第一个点与中间点在x坐标方向上的差值
    float x13 = x1 - x3;// 计算第一个点与最后一个点在x坐标方向上的差值
    float y12 = y1 - y2; // 计算第一个点与中间点在y坐标方向上的差值
    float y13 = y1 - y3;   // 计算第一个点与最后一个点在y坐标方向上的差值
    float y31 = y3 - y1;// 计算最后一个点与第一个点在y坐标方向上的差值（与y13互为相反数，但后续计算可能需要单独表示）
    float y21 = y2 - y1; // 计算中间点与第一个点在y坐标方向上的差值（与y12互为相反数，但后续计算可能需要单独表示）
    float x31 = x3 - x1; // 计算最后一个点与第一个点在x坐标方向上的差值（与x13互为相反数，但后续计算可能需要单独表示）
    float x21 = x2 - x1;// 计算中间点与第一个点在x坐标方向上的差值（与x12互为相反数，但后续计算可能需要单独表示）

    float sx13 = x1 * x1 - x3 * x3;// 计算一个中间变量sx13，用于后续圆半径计算公式中的一部分，其计算方式基于坐标差值和坐标平方等运算
    float sy13 = y1 * y1 - y3 * y3; // 类似地，计算另一个中间变量sy13，同样参与圆半径的计算推导过程
    float sx21 = x2 * x2 - x1 * x1;    // 计算中间变量sx21，用于后续计算
    float sy21 = y2 * y2 - y1 * y1; // 计算中间变量sy21，用于后续计算

    float f_denom = 2 * (y31 * x12 - y21 * x13);// 计算分母f_denom，用于后续计算圆半径公式中的一部分，它基于前面计算出的坐标差值等信息
    if (f_denom == 0) {
      return std::numeric_limits<float>::max();
    }
    float f = (sx13 * x12 + sy13 * x12 + sx21 * x13 + sy21 * x13) / f_denom;// 根据前面计算出的变量，按照圆半径计算公式计算出变量f的值，f是用于后续确定圆心坐标等计算的中间变量，其具体数学含义基于几何推导而来

    float g_denom = 2 * (x31 * y12 - x21 * y13);  // 计算分母g_denom，同样用于圆半径计算公式的一部分，基于坐标差值等信息构建，如果分母为0，同样返回正无穷大表示无法正常计算圆半径
    if (g_denom == 0) {
      return std::numeric_limits<float>::max();
    }
    float g = (sx13 * y12 + sy13 * y12 + sx21 * y13 + sy21 * y13) / g_denom;// 根据相关变量按照公式计算出变量g的值，g也是确定圆心坐标等计算的中间变量，与f一起用于最终圆半径的计算

    float c = - (x1 * x1 + y1 * y1) - 2 * g * x1 - 2 * f * y1;// 计算一个中间变量c，同样基于坐标和前面计算出的f、g等变量，用于后续计算圆半径，其数学含义来源于圆的方程以及通过三点确定圆的推导过程
    float c = - (x1 * x1 + y1 * y1) - 2 * g * x1 - 2 * f * y1; // 计算圆心的横坐标h，根据几何推导，它与前面计算出的g有关（这里是特定的数学关系体现）
    float h = -g; // 计算圆心的横坐标h，根据几何推导，它与前面计算出的g有关（这里是特定的数学关系体现）
    float k = -f;    // 计算圆心的纵坐标k，与前面计算出的f有关（体现了三点确定圆的数学原理中的对应关系）

  return std::sqrt(h * h + k * k - c);// 根据圆心坐标（h、k）以及中间变量c，按照圆的半径计算公式（基于圆的标准方程推导而来）计算并返回圆的半径
}

void MotionPlanStage::RemoveActor(const ActorId actor_id) {// MotionPlanStage类中的成员函数RemoveActor，用于从相关数据结构中移除指定ID的角色（actor，可能是车辆等模拟对象）信息
  pid_state_map.erase(actor_id); 
// 从pid_state_map数据结构中删除与指定actor_id对应的元素，pid_state_map可能是一个存储了角色ID与某些状态（比如PID控制相关状态等，具体取决于项目定义）映射关系的容器
  teleportation_instance.erase(actor_id); 
// 从teleportation_instance数据结构中删除与指定actor_id对应的元素，teleportation_instance可能是存储了角色瞬移（teleportation，可能在模拟场景中有特殊移动情况相关）实例相关信息的容器
}

void MotionPlanStage::Reset() {// MotionPlanStage类中的成员函数Reset，用于重置（清空）相关的数据结构，通常在需要重新初始化或者开始新的模拟阶段等场景下使用
  pid_state_map.clear();
  teleportation_instance.clear();
}

} // namespace traffic_manager
} // namespace carla
