# Copyright (c) # Copyright (c) 2018-2020 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module implements an agent that roams around a track following random
waypoints and avoiding other vehicles. The agent also responds to traffic lights.
It can also make use of the global route planner to follow a specified route
"""

# 导入CARLA库，用于与CARLA仿真环境进行交互
import carla
 
# 从shapely.geometry导入Polygon类，用于处理多边形几何形状
from shapely.geometry import Polygon
 
# 从agents.navigation模块导入LocalPlanner和RoadOption类，用于本地路径规划
from agents.navigation.local_planner import LocalPlanner, RoadOption
# 从agents.navigation模块导入GlobalRoutePlanner类，用于全局路径规划
from agents.navigation.global_route_planner import GlobalRoutePlanner
# 从agents.tools.misc模块导入一些实用函数
from agents.tools.misc import (get_speed, is_within_distance,  # 获取速度，判断是否在距离范围内
                               get_trafficlight_trigger_location)  # 获取交通灯触发位置
 
# 从agents.tools.hints模块导入ObstacleDetectionResult和TrafficLightDetectionResult类型提示
from agents.tools.hints import ObstacleDetectionResult, TrafficLightDetectionResult

# 定义一个名为BasicAgent的类
#class BasicAgent:
# """
#BasicAgent类实现了一个能够在场景中导航的代理（Agent）。
#这个代理遵循交通信号灯和其他车辆的规则，但会忽略停车标志（stop signs） 
#该类提供了多个函数，用于指定代理必须遵循的路线，
# 以及根据需求更改其参数，以实现不同的驾驶模式。
class BasicAgent:
    """
    BasicAgent implements an agent that navigates the scene.
    This agent respects traffic lights and other vehicles, but ignores stop signs.
    It has several functions available to specify the route that the agent must follow,
    as well as to change its parameters in case a different driving mode is desired.
    """
    def __init__(self, vehicle, target_speed=20, opt_dict={}, map_inst=None, grp_inst=None):
        """
        Initialization the agent parameters, the local and the global planner.

            :param vehicle: actor to apply to agent logic onto
            :param target_speed: speed (in Km/h) at which the vehicle will move
            :param opt_dict: dictionary in case some of its parameters want to be changed.
                This also applies to parameters related to the LocalPlanner.
            :param map_inst: carla.Map instance to avoid the expensive call of getting it.
            :param grp_inst: GlobalRoutePlanner instance to avoid the expensive call of getting it.

        """
        #将vehicle赋值给自身的_vehicle属性
        self._vehicle = vehicle
        #将自身的_vehicle的get_world对象赋值给自身的_world
        self._world = self._vehicle.get_world()
        #如果存在map_inst
        if map_inst:
            #判断对象是否是某个类的实例
            if isinstance(map_inst, carla.Map):
                #将map_inst赋值给self._map
                self._map = map_inst
            else:
                print("Warning: Ignoring the given map as it is not a 'carla.Map'")
                self._map = self._world.get_map()
        else:
            self._map = self._world.get_map()
        self._last_traffic_light = None

        # Base parameters
        #自身的_ignore_traffic_lights属性被设置为False
        self._ignore_traffic_lights = False
        #自身的_ignore_stop_signs属性被设置为Flase
        self._ignore_stop_signs = False
        #自身的_ignore_vehicles被设置为Flase
        self._ignore_vehicles = False
        #自身的_use_bbs_detection被设置为Flase
        self._use_bbs_detection = False
        #自身的_target_speed被设置为target_speed
        self._target_speed = target_speed
        #自身的_sampling_resolution属性被设置为2.0
        self._sampling_resolution = 2.0
        #自身的_base_tlight_threshold属性被设置为5.0
        self._base_tlight_threshold = 5.0  # meters
        #自身的_base_vehicle_threshold属性被设置为5.0
        self._base_vehicle_threshold = 5.0  # meters
        #自身的_speed_ratio被设置为1
        self._speed_ratio = 1
        #自身的_max_brake被设置为0.5
        self._max_brake = 0.5
        #自身的_offset被设置为0
        self._offset = 0

        # Change parameters according to the dictionary
        opt_dict['target_speed'] = target_speed
        if 'ignore_traffic_lights' in opt_dict:
            self._ignore_traffic_lights = opt_dict['ignore_traffic_lights']
        if 'ignore_stop_signs' in opt_dict:
            self._ignore_stop_signs = opt_dict['ignore_stop_signs']
        if 'ignore_vehicles' in opt_dict:
            self._ignore_vehicles = opt_dict['ignore_vehicles']
        if 'use_bbs_detection' in opt_dict:
            self._use_bbs_detection = opt_dict['use_bbs_detection']
        if 'sampling_resolution' in opt_dict:
            self._sampling_resolution = opt_dict['sampling_resolution']
        if 'base_tlight_threshold' in opt_dict:
            self._base_tlight_threshold = opt_dict['base_tlight_threshold']
        if 'base_vehicle_threshold' in opt_dict:
            self._base_vehicle_threshold = opt_dict['base_vehicle_threshold']
        if 'detection_speed_ratio' in opt_dict:
            self._speed_ratio = opt_dict['detection_speed_ratio']
        if 'max_brake' in opt_dict:
            self._max_brake = opt_dict['max_brake']
        if 'offset' in opt_dict:
            self._offset = opt_dict['offset']

        # Initialize the planners
        self._local_planner = LocalPlanner(self._vehicle, opt_dict=opt_dict, map_inst=self._map)
        if grp_inst:
            if isinstance(grp_inst, GlobalRoutePlanner):
                self._global_planner = grp_inst
            else:
                print("Warning: Ignoring the given map as it is not a 'carla.Map'")
                self._global_planner = GlobalRoutePlanner(self._map, self._sampling_resolution)
        else:
            self._global_planner = GlobalRoutePlanner(self._map, self._sampling_resolution)

        # Get the static elements of the scene
        self._lights_list = self._world.get_actors().filter("*traffic_light*")
        self._lights_map = {}  # Dictionary mapping a traffic light to a wp corresponding to its trigger volume location

    def add_emergency_stop(self, control):
        """
        Overwrites the throttle a brake values of a control to perform an emergency stop.
        The steering is kept the same to avoid going out of the lane when stopping during turns

            :param speed (carl.VehicleControl): control to be modified
        """
        #油门的值设置为0
        control.throttle = 0.0
        #将刹车的值设置为最大
        control.brake = self._max_brake
        #将手刹设置为Flase
        control.hand_brake = False
        #返回修改对象
        return control

    def set_target_speed(self, speed):
        """
        Changes the target speed of the agent
            :param speed (float): target speed in Km/h
        ""
        #将传入的目标速度speed赋值给对象的_target_speed属性
        self._target_speed = speed
        #调用它的set_speed方法并传入speed参数
        self._local_planner.set_speed(speed)

    #接受一个布尔类型参数value，默认值为True
    def follow_speed_limits(self, value=True):
        """
        If active, the agent will dynamically change the target speed according to the speed limits

            #参数value是否激活这个行为
            :param value (bool): whether or not to activate this behavior
        """
        #调用ollow_speed_limits方法并传入value参数
        self._local_planner.follow_speed_limits(value)

    #定义一个方法，获取受保护的成员变量local_planner的值
    def get_local_planner(self):
           #获取受保护成员的方法
        """Get method for protected member local planner"""
        return self._local_planner

    def get_global_planner(self):
        """Get method for protected member local planner"""
        return self._global_planner

    def set_destination(self, end_location, start_location=None, clean_queue=True):
        # type: (carla.Location, carla.Location | None, bool) -> None：#这是一个类型提示，表明函数接受的参数类型和返回类型。这里carla.Location是一个位置对象， 
        """
        This method creates a list of waypoints between a starting and ending location,
        based on the route returned by the global router, and adds it to the local planner.
        If no starting location is passed and `clean_queue` is True, the vehicle local planner's
        target location is chosen, which corresponds (by default), to a location about 5 meters
        in front of the vehicle.
        If `clean_queue` is False the newly planned route will be appended to the current route.

            :param end_location (carla.Location): final location of the route#说明它是一个 carla.Location类型，代表路线的终点。
            :param start_location (carla.Location): starting location of the route#说明它是一个carla.Location类型，代表路线的起点
            :param clean_queue (bool): Whether to clear or append to the currently planned route#说明它是一个布尔值，用于决定是清除当前规划的路线还是将新路线追加到现有路线。
        """
        if not start_location:#这行代码检查是否没有提供起点位置。
            if clean_queue and self._local_planner.target_waypoint:
                # Plan from the waypoint in front of the vehicle onwards#这是一条注释，说明接下来的代码将从车辆前方的路径点开始规划路线。
                start_location = self._local_planner.target_waypoint.transform.location#这行代码将起点位置设置为路径点队列中最后一个路径点的位置。
            elif not clean_queue and self._local_planner._waypoints_queue:
                # Append to the current plan
                start_location = self._local_planner._waypoints_queue[-1][0].transform.location
            else:
                # no target_waypoint or _waypoints_queue empty, use vehicle location#这是一条注释，说明如果没有目标路径点或路径点队列为空，将使用车辆的当前位置作为起点
                start_location = self._vehicle.get_location()#这行代码将起点位置设置为车辆的当前位置。
        start_waypoint = self._map.get_waypoint(start_location)#这行代码获取起点位置对应的路径点（  waypoint  ），这通常用于自动驾驶模拟中，以便规划从起点到终点的路线。
        route_trace = self.trace_route(start_waypoint, end_waypoint)
        self._local_planner.set_global_plan(route_trace, clean_queue=clean_queue)

    def set_global_plan(self, plan, stop_waypoint_creation=True, clean_queue=True):
        """
        Adds a specific plan to the agent.

            :param plan: list of [carla.Waypoint, RoadOption] representing the route to be followed
            :param stop_waypoint_creation: stops the automatic random creation of waypoints
            :param clean_queue: resets the current agent's plan
        """
        self._local_planner.set_global_plan(
            plan,
            stop_waypoint_creation=stop_waypoint_creation,
            clean_queue=clean_queue
        )

    def trace_route(self, start_waypoint, end_waypoint):
        """
        Calculates the shortest route between a starting and ending waypoint.

            :param start_waypoint (carla.Waypoint): initial waypoint
            :param end_waypoint (carla.Waypoint): final waypoint
        """
        start_location = start_waypoint.transform.location
        end_location = end_waypoint.transform.location
        return self._global_planner.trace_route(start_location, end_location)

    def run_step(self):#run_step#方法的定义，它是一个实例方法，属于某个类。
        """Execute one step of navigation."""
        hazard_detected = False#用于标记在导航过程中是否检测到危险。

        # Retrieve all relevant actors
        vehicle_list = self._world.get_actors().filter("*vehicle*")

        vehicle_speed = get_speed(self._vehicle) / 3.6#这行代码调用get_speed函数来获取当前车辆的速度

        # Check for possible vehicle obstacles
        max_vehicle_distance = self._base_vehicle_threshold + self._speed_ratio * vehicle_speed
        affected_by_vehicle, _, _ = self._vehicle_obstacle_detected(vehicle_list, max_vehicle_distance)
        if affected_by_vehicle:
            hazard_detected = True

        # Check if the vehicle is affected by a red traffic light
        max_tlight_distance = self._base_tlight_threshold + self._speed_ratio * vehicle_speed
        affected_by_tlight, _ = self._affected_by_traffic_light(self._lights_list, max_tlight_distance)
        if affected_by_tlight:
            hazard_detected = True

        control = self._local_planner.run_step()
        if hazard_detected:
            control = self.add_emergency_stop(control)

        return control

    def done(self):
        """Check whether the agent has reached its destination."""
        return self._local_planner.done()

    def ignore_traffic_lights(self, active=True):
        """(De)activates the checks for traffic lights"""
        self._ignore_traffic_lights = active

    def ignore_stop_signs(self, active=True):
        """(De)activates the checks for stop signs"""
        self._ignore_stop_signs = active

    def ignore_vehicles(self, active=True):
        """(De)activates the checks for stop signs"""
        self._ignore_vehicles = active

    def set_offset(self, offset):
        """Sets an offset for the vehicle"""
        self._local_planner.set_offset(offset)

    def lane_change(self, direction, same_lane_time=0, other_lane_time=0, lane_change_time=2):
        """
        Changes the path so that the vehicle performs a lane change.
        Use 'direction' to specify either a 'left' or 'right' lane change,
        and the other 3 fine tune the maneuver
        """
        speed = self._vehicle.get_velocity().length()
        path = self._generate_lane_change_path(
            self._map.get_waypoint(self._vehicle.get_location()),
            direction,
            same_lane_time * speed,
            other_lane_time * speed,
            lane_change_time * speed,
            False,
            1,
            self._sampling_resolution
        )
        if not path:
            print("WARNING: Ignoring the lane change as no path was found")

        self.set_global_plan(path)

    def _affected_by_traffic_light(self, lights_list=None, max_distance=None):
        """
        Method to check if there is a red light affecting the vehicle.

            :param lights_list (list of carla.TrafficLight): list containing TrafficLight objects.
                If None, all traffic lights in the scene are used
            :param max_distance (float): max distance for traffic lights to be considered relevant.
                If None, the base threshold value is used
        """
        if self._ignore_traffic_lights:
            return TrafficLightDetectionResult(False, None)

        if not lights_list:
            lights_list = self._world.get_actors().filter("*traffic_light*")

        if not max_distance:
            max_distance = self._base_tlight_threshold

        if self._last_traffic_light:
            if self._last_traffic_light.state != carla.TrafficLightState.Red:
                self._last_traffic_light = None
            else:
                return TrafficLightDetectionResult(True, self._last_traffic_light)

        #获取自行车位置，通过指定方法从车辆对象获取
        ego_vehicle_location = self._vehicle.get_location()
        #根据自行车位置获取对应的路点，借助地图对象的相关方法
        ego_vehicle_waypoint = self._map.get_waypoint(ego_vehicle_location)

        #开始遍历交通信号灯列表
        for traffic_light in lights_list:
            #如果交通信号灯的id在self.light_map中
            if traffic_light.id in self._lights_map:
                #将trigger_wp设置为self.lights_map中对应交通信号灯id的值
                trigger_wp = self._lights_map[traffic_light.id]
            #否则
            else:
                #获取交通信号灯的trigger_location
                trigger_location = get_trafficlight_trigger_location(traffic_light)
                #根据出发位置获取waypoint并赋值给trigger_wap
                trigger_wp = self._map.get_waypoint(trigger_location)
                #将self.lights_map中对应交通信号灯id的值设置为trigger_wp
                self._lights_map[traffic_light.id] = trigger_wp
            #如果触发trigger_wp转换后的位置与自车位置(ego_vehicle_location)的距离大于最大距离
            if trigger_wp.transform.location.distance(ego_vehicle_location) > max_distance:
                #跳过当前循环
                continue
            #如果触发trigger_wp转换后的位置与自车位置(ego_vehicle_location)的距离大于最大距离
            if trigger_wp.road_id != ego_vehicle_waypoint.road_id:
                #跳过当前循环
                continue
            #获取ego_vehicle_waypoint变换后的forward vector并赋值给ve_dir
            ve_dir = ego_vehicle_waypoint.transform.get_forward_vector()
            #获取trgger_wp变换后的forward vector并赋值给wp_dir
            wp_dir = trigger_wp.transform.get_forward_vector()
            #计算ve_dir和wp_dir的点积并赋值给dot_ve_wp
            dot_ve_wp = ve_dir.x * wp_dir.x + ve_dir.y * wp_dir.y + ve_dir.z * wp_dir.z
            #如果点积小于0
            if dot_ve_wp < 0:
                #跳过当前循环
                continue
            #如果traffic_light的状态不等于红色
            if traffic_light.state != carla.TrafficLightState.Red:
                #跳过当前循环
                continue
            #如果触发路点变换后的位置与自车变换后的位置在最大距离内且角度在[90]度范围内
            if is_within_distance(trigger_wp.transform, self._vehicle.get_transform(), max_distance, [0, 90]):
                #将当前交通信号灯设置为最后检测到的交通信号灯
                self._last_traffic_light = traffic_light
                #返回交通信号灯检测结果为True以及对应的交通信号灯对象
                return TrafficLightDetectionResult(True, traffic_light)
        #返回交通信号灯检测结果为False以及None
        return TrafficLightDetectionResult(False, None)

    def _vehicle_obstacle_detected(self, vehicle_list=None, max_distance=None, up_angle_th=90, low_angle_th=0, lane_offset=0):
        """
        Method to check if there is a vehicle in front of the agent blocking its path.

            :param vehicle_list (list of carla.Vehicle): list containing vehicle objects.
                If None, all vehicle in the scene are used
            :param max_distance: max freespace to check for obstacles.
                If None, the base threshold value is used
        """
        def get_route_polygon():
            """
    此函数用于构建表示车辆行驶路线的多边形（Polygon），该多边形基于车辆自身以及规划路径中的路点（waypoint）来确定边界点。
    """
            route_bb = [] # 用于存储构成多边形的边界点列表，初始为空列表
            # 获取车辆包围盒（bounding box）在y轴方向上的范围（extent），包围盒用于表示物体在空间中的大致范围
            extent_y = self._vehicle.bounding_box.extent.y
            # 根据车辆包围盒的y轴范围和一个偏移量（_offset）计算右侧扩展后的距离
            r_ext = extent_y + self._offset
            # 根据车辆包围盒的y轴范围和一个偏移量（_offset）计算左侧扩展后的距离
            l_ext = -extent_y + self._offset
            # 获取车辆自身坐标系下的右方向向量，用于后续计算边界点在空间中的位置
            r_vec = ego_transform.get_right_vector()
            # 根据车辆当前位置（ego_location）和右侧扩展距离（r_ext）以及右方向向量（r_vec）计算多边形的一个边界点p1
            p1 = ego_location + carla.Location(r_ext * r_vec.x, r_ext * r_vec.y)
            # 根据车辆当前位置（ego_location）和左侧扩展距离（l_ext）以及右方向向量（r_vec）计算多边形的另一个边界点p2
            p2 = ego_location + carla.Location(l_ext * r_vec.x, l_ext * r_vec.y)
            # 将计算得到的两个边界点（以坐标列表形式）添加到route_bb列表中，作为多边形的初始边界点
            route_bb.extend([[p1.x, p1.y, p1.z], [p2.x, p2.y, p2.z]])

            # 遍历局部规划器（_local_planner）中的规划路径（plan），每个元素包含路点（wp）和相关信息（这里忽略了后者）
            for wp, _ in self._local_planner.get_plan():
                # 如果当前车辆位置与路点位置的距离大于最大距离（max_distance，此处未定义，应该是外部传入或在更外层定义的变量），则停止遍历
                if ego_location.distance(wp.transform.location) > max_distance:
                    break

                # 获取路点坐标系下的右方向向量，用于计算基于该路点的多边形边界点位置
                r_vec = wp.transform.get_right_vector()
                # 根据路点位置（wp.transform.location）和右侧扩展距离（r_ext）以及右方向向量（r_vec）计算基于该路点的多边形的一个边界点p1
                p1 = wp.transform.location + carla.Location(r_ext * r_vec.x, r_ext * r_vec.y)
                # 根据路点位置（wp.transform.location）和左侧扩展距离（l_ext）以及右方向向量（r_vec）计算基于该路点的多边形的另一个边界点p2
                p2 = wp.transform.location + carla.Location(l_ext * r_vec.x, l_ext * r_vec.y)
                # 将基于该路点计算得到的两个边界点（以坐标列表形式）添加到route_bb列表中，逐步构建多边形的边界点集合
                route_bb.extend([[p1.x, p1.y, p1.z], [p2.x, p2.y, p2.z]])

            # Two points don't create a polygon, nothing to check
            if len(route_bb) < 3:
                return None

            return Polygon(route_bb)

        if self._ignore_vehicles:
            return ObstacleDetectionResult(False, None, -1)

        if vehicle_list is None:
            vehicle_list = self._world.get_actors().filter("*vehicle*")
        if len(vehicle_list) == 0:
            return ObstacleDetectionResult(False, None, -1)

        if not max_distance:
            max_distance = self._base_vehicle_threshold

        ego_transform = self._vehicle.get_transform()
        ego_location = ego_transform.location
        ego_wpt = self._map.get_waypoint(ego_location)

        # Get the right offset
        if ego_wpt.lane_id < 0 and lane_offset != 0:
            lane_offset *= -1

        # Get the transform of the front of the ego
        ego_front_transform = ego_transform
        ego_front_transform.location += carla.Location(
            self._vehicle.bounding_box.extent.x * ego_transform.get_forward_vector())

        opposite_invasion = abs(self._offset) + self._vehicle.bounding_box.extent.y > ego_wpt.lane_width / 2
        use_bbs = self._use_bbs_detection or opposite_invasion or ego_wpt.is_junction

        # Get the route bounding box
        route_polygon = get_route_polygon()

        for target_vehicle in vehicle_list:
            if target_vehicle.id == self._vehicle.id:
                continue

            target_transform = target_vehicle.get_transform()
            if target_transform.location.distance(ego_location) > max_distance:
                continue

            target_wpt = self._map.get_waypoint(target_transform.location, lane_type=carla.LaneType.Any)

            # General approach for junctions and vehicles invading other lanes due to the offset
            if (use_bbs or target_wpt.is_junction) and route_polygon:

                target_bb = target_vehicle.bounding_box
                target_vertices = target_bb.get_world_vertices(target_vehicle.get_transform())
                target_list = [[v.x, v.y, v.z] for v in target_vertices]
                target_polygon = Polygon(target_list)

                if route_polygon.intersects(target_polygon):
                    return ObstacleDetectionResult(True, target_vehicle, target_vehicle.get_location().distance(ego_location))

            # Simplified approach, using only the plan waypoints (similar to TM)
            else:

                if target_wpt.road_id != ego_wpt.road_id or target_wpt.lane_id != ego_wpt.lane_id  + lane_offset:
                    next_wpt = self._local_planner.get_incoming_waypoint_and_direction(steps=3)[0]
                    if not next_wpt:
                        continue
                    if target_wpt.road_id != next_wpt.road_id or target_wpt.lane_id != next_wpt.lane_id  + lane_offset:
                        continue

                target_forward_vector = target_transform.get_forward_vector()
                target_extent = target_vehicle.bounding_box.extent.x
                target_rear_transform = target_transform
                target_rear_transform.location -= carla.Location(
                    x=target_extent * target_forward_vector.x,
                    y=target_extent * target_forward_vector.y,
                )

                if is_within_distance(target_rear_transform, ego_front_transform, max_distance, [low_angle_th, up_angle_th]):
                    return ObstacleDetectionResult(True, target_vehicle, target_transform.location.distance(ego_transform.location))

        return ObstacleDetectionResult(False, None, -1)

    @staticmethod
    def _generate_lane_change_path(waypoint, direction='left', distance_same_lane=10,
                                distance_other_lane=25, lane_change_distance=25,
                                check=True, lane_changes=1, step_distance=2):
        # type: (carla.Waypoint, str, float, float, float, bool, int, float) -> list[tuple[carla.Waypoint, RoadOption]]
        """
        This methods generates a path that results in a lane change.
        Use the different distances to fine-tune the maneuver.
        If the lane change is impossible, the returned path will be empty.
        """
        distance_same_lane = max(distance_same_lane, 0.1)
        distance_other_lane = max(distance_other_lane, 0.1)
        lane_change_distance = max(lane_change_distance, 0.1)

        plan = []
        plan.append((waypoint, RoadOption.LANEFOLLOW))  # start position

        option = RoadOption.LANEFOLLOW

        # Same lane
        distance = 0
        while distance < distance_same_lane:
            next_wps = plan[-1][0].next(step_distance)
            if not next_wps:
                return []
            next_wp = next_wps[0]
            distance += next_wp.transform.location.distance(plan[-1][0].transform.location)
            plan.append((next_wp, RoadOption.LANEFOLLOW))

        if direction == 'left':
            option = RoadOption.CHANGELANELEFT
        elif direction == 'right':
            option = RoadOption.CHANGELANERIGHT
        else:
            # ERROR, input value for change must be 'left' or 'right'
            return []

        lane_changes_done = 0
        lane_change_distance = lane_change_distance / lane_changes

        # Lane change
        while lane_changes_done < lane_changes:

            # Move forward
            next_wps = plan[-1][0].next(lane_change_distance)
            if not next_wps:
                return []
            next_wp = next_wps[0]

            #根据车辆的行驶方向来获取相邻车道的路点，同时统计车道变化的次数
            # Get the side lane
            #检查方向是否为左
            if direction == 'left':
                if check and str(next_wp.lane_change) not in ['Left', 'Both']:
                    return []
                #不满足获取下一个路点的左侧车道的路点，并赋值    
                side_wp = next_wp.get_left_lane()
            else:
                if check and str(next_wp.lane_change) not in ['Right', 'Both']:
                    return []
                side_wp = next_wp.get_right_lane()

            if not side_wp or side_wp.lane_type != carla.LaneType.Driving:
                return []

            # Update the plan
            #将之前定义好的元组添加到plan中
            plan.append((side_wp, option))
            #完成一次车道变换操作
            lane_changes_done += 1

        # Other lane
        #初始化一个变量distance为0
        distance = 0
        #循环条件
        while distance < distance_other_lane:
            #获取路径计划中最后一个元素的第一部分，然后调用这个对象的next方法，并传入一个步长距离
            next_wps = plan[-1][0].next(step_distance)
            #条件判断
            if not next_wps:
                return []
            next_wp = next_wps[0]
            #计算新的距离
            distance += next_wp.transform.location.distance(plan[-1][0].transform.location)
            #将next_wp和LANEFOLLOW的选项作为一个元组添加到plan中
            plan.append((next_wp, RoadOption.LANEFOLLOW))

        #返回最终构建的plan
        return plan
