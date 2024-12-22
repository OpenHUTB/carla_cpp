"""
CARLA自动驾驶代理模块

该模块实现了一个基础的自动驾驶代理，可以在地图中自主导航。
代理会遵守交通信号灯和避让其他车辆，但会忽略停车标志。
"""

import carla
from shapely.geometry import Polygon
from agents.navigation.local_planner import LocalPlanner, RoadOption
from agents.navigation.global_route_planner import GlobalRoutePlanner
from agents.tools.misc import (
    get_speed,
    is_within_distance,
    get_trafficlight_trigger_location
)
from agents.tools.hints import ObstacleDetectionResult, TrafficLightDetectionResult

class BasicAgent:
    """
    基础自动驾驶代理类
    
    该代理可以在CARLA环境中自主导航，具有以下特性：
    - 遵守交通信号灯
    - 避让其他车辆
    - 自适应速度控制
    - 动态路径规划
    """

    def __init__(self, vehicle, target_speed=20, opt_dict=None, map_inst=None, grp_inst=None):
        """
        初始化代理
        
        Args:
            vehicle: 要控制的车辆Actor
            target_speed: 目标速度(km/h)
            opt_dict: 配置选项字典
            map_inst: 地图实例(避免重复获取)
            grp_inst: 全局路径规划器实例(避免重复获取)
        """
        # 基础组件初始化
        self._setup_base_components(vehicle, map_inst)
        
        # 配置参数初始化
        self._init_parameters(target_speed, opt_dict or {})
        
        # 规划器初始化
        self._init_planners(opt_dict or {}, grp_inst)
        
        # 场景元素初始化
        self._init_scene_elements()

    def _setup_base_components(self, vehicle, map_inst):
        """初始化基础组件"""
        self._vehicle = vehicle
        self._world = vehicle.get_world()
        self._map = (map_inst if isinstance(map_inst, carla.Map)
                    else self._world.get_map())
        self._last_traffic_light = None

    def _init_parameters(self, target_speed, opt_dict):
        """初始化配置参数"""
        # 基础行为标志
        self._ignore_traffic_lights = opt_dict.get('ignore_traffic_lights', False)
        self._ignore_stop_signs = opt_dict.get('ignore_stop_signs', False)
        self._ignore_vehicles = opt_dict.get('ignore_vehicles', False)
        self._use_bbs_detection = opt_dict.get('use_bbs_detection', False)
        
        # 运动控制参数
        self._target_speed = target_speed
        self._speed_ratio = opt_dict.get('detection_speed_ratio', 1)
        self._max_brake = opt_dict.get('max_brake', 0.5)
        self._offset = opt_dict.get('offset', 0)
        
        # 距离阈值
        self._sampling_resolution = opt_dict.get('sampling_resolution', 2.0)
        self._base_tlight_threshold = opt_dict.get('base_tlight_threshold', 5.0)
        self._base_vehicle_threshold = opt_dict.get('base_vehicle_threshold', 5.0)

    def _init_planners(self, opt_dict, grp_inst):
        """初始化路径规划器"""
        # 本地规划器
        self._local_planner = LocalPlanner(
            self._vehicle,
            opt_dict=opt_dict,
            map_inst=self._map
        )
        
        # 全局规划器
        self._global_planner = (grp_inst if isinstance(grp_inst, GlobalRoutePlanner)
                               else GlobalRoutePlanner(self._map, self._sampling_resolution))

    def _init_scene_elements(self):
        """初始化场景元素"""
        self._lights_list = self._world.get_actors().filter("*traffic_light*")
        self._lights_map = {}  # 交通灯触发器位置映射

    def set_destination(self, end_location, start_location=None, clean_queue=True):
        """
        设置导航目标点
        
        Args:
            end_location: 终点位置
            start_location: 起点位置（默认为当前位置）
            clean_queue: 是否清除现有路径
        """
        start_location = self._determine_start_location(start_location, clean_queue)
        start_waypoint = self._map.get_waypoint(start_location)
        end_waypoint = self._map.get_waypoint(end_location)
        
        route_trace = self.trace_route(start_waypoint, end_waypoint)
        self._local_planner.set_global_plan(route_trace, clean_queue=clean_queue)

    def _determine_start_location(self, start_location, clean_queue):
        """确定起始位置"""
        if not start_location:
            if clean_queue and self._local_planner.target_waypoint:
                return self._local_planner.target_waypoint.transform.location
            elif not clean_queue and self._local_planner._waypoints_queue:
                return self._local_planner._waypoints_queue[-1][0].transform.location
            else:
                return self._vehicle.get_location()
        return start_location

    def run_step(self):
        """
        执行一步导航控制
        
        Returns:
            carla.VehicleControl: 车辆控制指令
        """
        vehicle_speed = get_speed(self._vehicle) / 3.6

        # 检查障碍物
        hazard_detected = self._check_hazards(vehicle_speed)

        # 生成控制指令
        control = self._local_planner.run_step()
        if hazard_detected:
            control = self.add_emergency_stop(control)

        return control

    def _check_hazards(self, vehicle_speed):
        """检查周围环境中的危险"""
        # 检查车辆障碍物
        max_vehicle_distance = (self._base_vehicle_threshold + 
                              self._speed_ratio * vehicle_speed)
        vehicle_state = self._vehicle_obstacle_detected(max_distance=max_vehicle_distance)
        if vehicle_state.is_hazard:
            return True

        # 检查交通信号灯
        max_tlight_distance = (self._base_tlight_threshold + 
                             self._speed_ratio * vehicle_speed)
        tlight_state = self._affected_by_traffic_light(max_distance=max_tlight_distance)
        if tlight_state.is_hazard:
            return True

        return False

    def add_emergency_stop(self, control):
        """
        应用紧急制动
        
        Args:
            control: 原始控制指令
            
        Returns:
            修改后的控制指令
        """
        control.throttle = 0.0
        control.brake = self._max_brake
        control.hand_brake = False
        return control

    def _vehicle_obstacle_detected(self, vehicle_list=None, max_distance=None, 
                                 up_angle_th=90, low_angle_th=0, lane_offset=0):
        """
        检测前方车辆障碍物
        
        使用包围盒或简化方法检测前方车辆，支持路口和车道变换场景。
        """
        if self._ignore_vehicles:
            return ObstacleDetectionResult(False, None, -1)

        # 获取场景中的所有车辆
        vehicle_list = vehicle_list or self._world.get_actors().filter("*vehicle*")
        if not vehicle_list:
            return ObstacleDetectionResult(False, None, -1)

        max_distance = max_distance or self._base_vehicle_threshold
        ego_transform = self._vehicle.get_transform()
        ego_location = ego_transform.location
        ego_waypoint = self._map.get_waypoint(ego_location)

        # 处理特殊情况
        if ego_waypoint.lane_id < 0 and lane_offset != 0:
            lane_offset *= -1

        use_bbs = (self._use_bbs_detection or 
                   abs(self._offset) + self._vehicle.bounding_box.extent.y > ego_waypoint.lane_width / 2 or 
                   ego_waypoint.is_junction)

        # 获取路径多边形
        route_polygon = self._get_route_polygon(max_distance)

        # 检查每个车辆
        for target_vehicle in vehicle_list:
            if target_vehicle.id == self._vehicle.id:
                continue

            if self._is_vehicle_hazard(target_vehicle, ego_location, ego_waypoint,
                                     max_distance, use_bbs, route_polygon, lane_offset):
                return ObstacleDetectionResult(True, target_vehicle, 
                                             target_vehicle.get_location().distance(ego_location))

        return ObstacleDetectionResult(False, None, -1)

    def _is_vehicle_hazard(self, target_vehicle, ego_location, ego_waypoint,
                          max_distance, use_bbs, route_polygon, lane_offset):
        """判断目标车辆是否构成危险"""
        target_transform = target_vehicle.get_transform()
        if target_transform.location.distance(ego_location) > max_distance:
            return False

        target_waypoint = self._map.get_waypoint(target_transform.location, 
                                                lane_type=carla.LaneType.Any)

        if use_bbs or target_waypoint.is_junction:
            return self._check_hazard_by_bbox(target_vehicle, route_polygon)
        else:
            return self._check_hazard_by_waypoint(target_waypoint, ego_waypoint,
                                                target_transform, lane_offset)

    @staticmethod
    def _generate_lane_change_path(waypoint, direction='left', distance_same_lane=10,
                                 distance_other_lane=25, lane_change_distance=25,
                                 check=True, lane_changes=1, step_distance=2):
        """
        生成车道变换路径
        
        Args:
            waypoint: 起始路点
            direction: 变道方向('left'或'right')
            distance_same_lane: 同车道行驶距离
            distance_other_lane: 目标车道行驶距离
            lane_change_distance: 变道距离
            check: 是否检查变道可行性
            lane_changes: 变道次数
            step_distance: 路径点间距
            
        Returns:
            list[tuple[carla.Waypoint, RoadOption]]: 变道路径点列表
        """
        # 参数验证
        if direction not in ['left', 'right']:
            return []
        
        distance_same_lane = max(distance_same_lane, 0.1)
        distance_other_lane = max(distance_other_lane, 0.1)
        lane_change_distance = max(lane_change_distance, 0.1)

        # 初始化路径
        plan = [(waypoint, RoadOption.LANEFOLLOW)]
        
        # 同车道直行段
        plan.extend(BasicAgent._generate_straight_path(plan[-1][0], distance_same_lane, step_distance))
        
        # 变道段
        option = (RoadOption.CHANGELANELEFT if direction == 'left' 
                 else RoadOption.CHANGELANERIGHT)
        
        lane_changes_done = 0
        single_change_dist = lane_change_distance / lane_changes
        
        while lane_changes_done < lane_changes:
            next_wps = plan[-1][0].next(single_change_dist)
            if not next_wps:
                return []
            
            next_wp = next_wps[0]
            side_wp = (next_wp.get_left_lane() if direction == 'left' 
                      else next_wp.get_right_lane())
            
            if not side_wp or side_wp.lane_type != carla.LaneType.Driving:
                return []
            
            if check:
                if direction == 'left' and str(next_wp.lane_change) not in ['Left', 'Both']:
                    return []
                if direction == 'right' and str(next_wp.lane_change) not in ['Right', 'Both']:
                    return []
            
            plan.append((side_wp, option))
            lane_changes_done += 1
        
        # 目标车道直行段
        plan.extend(BasicAgent._generate_straight_path(plan[-1][0], distance_other_lane, step_distance))
        
        return plan

    @staticmethod
    def _generate_straight_path(start_wp, distance, step_distance):
        """生成直线路径段"""
        path = []
        current_distance = 0
        current_wp = start_wp
        
        while current_distance < distance:
            next_wps = current_wp.next(step_distance)
            if not next_wps:
                break
                
            current_wp = next_wps[0]
            current_distance += current_wp.transform.location.distance(path[-1][0].transform.location if path 
                                                                    else start_wp.transform.location)
            path.append((current_wp, RoadOption.LANEFOLLOW))
            
        return path
