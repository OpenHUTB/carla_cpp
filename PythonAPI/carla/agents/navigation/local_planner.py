# Copyright (c) # Copyright (c) 2018-2020 CVC.# 版权相关声明
#
# This work is licensed under the terms of the MIT license.# 遵循MIT许可协议
# For a copy, see <https://opensource.org/licenses/MIT>.

""" This module contains a local planner to perform low-level waypoint following based on PID controllers. """# 查看许可协议链接

from enum import IntEnum
from collections import deque
import random

import carla
from agents.navigation.controller import VehiclePIDController
from agents.tools.misc import draw_waypoints, get_speed


class RoadOption(IntEnum):# 定义道路选择的枚举类型
    """
    RoadOption represents the possible topological configurations when moving from a segment of lane to other. 表示从一个车道段移动到其他车道段时可能的拓扑配置。 

    """
    VOID = -1
    LEFT = 1
    RIGHT = 2
    STRAIGHT = 3
    LANEFOLLOW = 4
    CHANGELANELEFT = 5
    CHANGELANERIGHT = 6


class LocalPlanner:
    """
    LocalPlanner implements the basic behavior of following a
    trajectory of waypoints that is generated on-the-fly.

    The low-level motion of the vehicle is computed by using two PID controllers,
    one is used for the lateral control and the other for the longitudinal control (cruise speed).

    When multiple paths are available (intersections) this local planner makes a random choice,
    unless a given global plan has already been specified.
    """
    
    """
    实现跟随实时生成的路点轨迹的基本行为。
    通过两个PID控制器计算车辆的低级运动，一个用于横向控制，一个用于纵向控制（巡航速度）。
    在有多个路径可选（交叉路口）时随机选择，除非已指定全局计划。
    """
    
    def __init__(self, vehicle, opt_dict={}, map_inst=None):
        """
        :param vehicle: actor to apply to local planner logic onto
        :param opt_dict: dictionary of arguments with different parameters:
            dt: time between simulation steps
            target_speed: desired cruise speed in Km/h
            sampling_radius: distance between the waypoints part of the plan
            lateral_control_dict: values of the lateral PID controller
            longitudinal_control_dict: values of the longitudinal PID controller
            max_throttle: maximum throttle applied to the vehicle
            max_brake: maximum brake applied to the vehicle
            max_steering: maximum steering applied to the vehicle
            offset: distance between the route waypoints and the center of the lane
        :param map_inst: carla.Map instance to avoid the expensive call of getting it.
        """
               
        """
        :param vehicle: 应用本地规划逻辑的车辆对象
        :param opt_dict: 包含不同参数的字典，可重载默认参数
        :param map_inst: carla.Map实例，避免重复获取地图的昂贵调用
        """
        
        self._vehicle = vehicle
        self._world = self._vehicle.get_world()
        if map_inst:
            if isinstance(map_inst, carla.Map):
                self._map = map_inst
            else:
                print("Warning: Ignoring the given map as it is not a 'carla.Map'")
                self._map = self._world.get_map()
        else:
            self._map = self._world.get_map()

        self._vehicle_controller = None
        self.target_waypoint = None
        self.target_road_option = None

        self._waypoints_queue = deque(maxlen=10000)
        self._min_waypoint_queue_length = 100
        self._stop_waypoint_creation = False

        # Base parameters 基本参数初始化
        self._dt = 1.0 / 20.0
        self._target_speed = 20.0  # Km/h
        self._sampling_radius = 2.0
        self._args_lateral_dict = {'K_P': 1.95, 'K_I': 0.05, 'K_D': 0.2, 'dt': self._dt}
        self._args_longitudinal_dict = {'K_P': 1.0, 'K_I': 0.05, 'K_D': 0, 'dt': self._dt}
        self._max_throt = 0.75
        self._max_brake = 0.3
        self._max_steer = 0.8
        self._offset = 0
        self._base_min_distance = 3.0
        self._distance_ratio = 0.5
        self._follow_speed_limits = False

        # Overload parameters 根据传入字典重载参数
        if opt_dict:
            if 'dt' in opt_dict:
                self._dt = opt_dict['dt']
            if 'target_speed' in opt_dict:
                self._target_speed = opt_dict['target_speed']
            if 'sampling_radius' in opt_dict:
                self._sampling_radius = opt_dict['sampling_radius']
            if 'lateral_control_dict' in opt_dict:
                self._args_lateral_dict = opt_dict['lateral_control_dict']
            if 'longitudinal_control_dict' in opt_dict:
                self._args_longitudinal_dict = opt_dict['longitudinal_control_dict']
            if 'max_throttle' in opt_dict:
                self._max_throt = opt_dict['max_throttle']
            if 'max_brake' in opt_dict:
                self._max_brake = opt_dict['max_brake']
            if 'max_steering' in opt_dict:
                self._max_steer = opt_dict['max_steering']
            if 'offset' in opt_dict:
                self._offset = opt_dict['offset']
            if 'base_min_distance' in opt_dict:
                self._base_min_distance = opt_dict['base_min_distance']
            if 'distance_ratio' in opt_dict:
                self._distance_ratio = opt_dict['distance_ratio']
            if 'follow_speed_limits' in opt_dict:
                self._follow_speed_limits = opt_dict['follow_speed_limits']

        # initializing controller 初始化控制器
        self._init_controller()

    def reset_vehicle(self):
        """Reset the ego-vehicle 重置自车对象"""
        self._vehicle = None

    def _init_controller(self):
        """Controller initialization 控制器初始化方法"""
        self._vehicle_controller = VehiclePIDController(self._vehicle,
                                                        args_lateral=self._args_lateral_dict,
                                                        args_longitudinal=self._args_longitudinal_dict,
                                                        offset=self._offset,
                                                        max_throttle=self._max_throt,
                                                        max_brake=self._max_brake,
                                                        max_steering=self._max_steer)

        # Compute the current vehicle waypoint 获取当前车辆所在路点等信息并添加到路点队列
        current_waypoint = self._map.get_waypoint(self._vehicle.get_location())
        self.target_waypoint, self.target_road_option = (current_waypoint, RoadOption.LANEFOLLOW)
        self._waypoints_queue.append((self.target_waypoint, self.target_road_option))

    def set_speed(self, speed):
        """
        Changes the target speed

        :param speed: new target speed in Km/h
        :return:
        :param speed: 新的目标速度（单位：Km/h），改变目标速度
        """
        if self._follow_speed_limits:
            print("WARNING: The max speed is currently set to follow the speed limits. "
                  "Use 'follow_speed_limits' to deactivate this")
        self._target_speed = speed

    def follow_speed_limits(self, value=True):
        """
        Activates a flag that makes the max speed dynamically vary according to the speed limits

        :param value: bool
        :return:
        :param value: 布尔值，激活根据速度限制动态改变最大速度的标志
        """
        self._follow_speed_limits = value

    def _compute_next_waypoints(self, k=1):
        """
        Add new waypoints to the trajectory queue.

        :param k: how many waypoints to compute
        :return:
        :param k: 要计算的路点数，添加新的路点到轨迹队列
        """
        # check we do not overflow the queue
        available_entries = self._waypoints_queue.maxlen - len(self._waypoints_queue)
        k = min(available_entries, k)

        for _ in range(k):
            last_waypoint = self._waypoints_queue[-1][0]
            next_waypoints = list(last_waypoint.next(self._sampling_radius))

            if len(next_waypoints) == 0:
                break
            elif len(next_waypoints) == 1:
                # only one option available ==> lanefollowing
                next_waypoint = next_waypoints[0]
                road_option = RoadOption.LANEFOLLOW
            else:
                # random choice between the possible options
                road_options_list = _retrieve_options(
                    next_waypoints, last_waypoint)
                road_option = random.choice(road_options_list)
                next_waypoint = next_waypoints[road_options_list.index(
                    road_option)]

            self._waypoints_queue.append((next_waypoint, road_option))

    def set_global_plan(self, current_plan, stop_waypoint_creation=True, clean_queue=True):
        """
        Adds a new plan to the local planner. A plan must be a list of [carla.Waypoint, RoadOption] pairs
        The 'clean_queue` parameter erases the previous plan if True, otherwise, it adds it to the old one
        The 'stop_waypoint_creation' flag stops the automatic creation of random waypoints

        :param current_plan: list of (carla.Waypoint, RoadOption)
        :param stop_waypoint_creation: bool
        :param clean_queue: bool
        :return:
        :param current_plan: 包含(carla.Waypoint, RoadOption)对的列表，添加新计划到本地规划器
        :param stop_waypoint_creation: 布尔值，停止自动创建随机路点的标志
        :param clean_queue: 布尔值，是否清除之前的计划队列
        """
        if clean_queue:
            self._waypoints_queue.clear()

        # Remake the waypoints queue if the new plan has a higher length than the queue
        new_plan_length = len(current_plan) + len(self._waypoints_queue)
        if new_plan_length > self._waypoints_queue.maxlen:
            new_waypoint_queue = deque(maxlen=new_plan_length)
            for wp in self._waypoints_queue:
                new_waypoint_queue.append(wp)
            self._waypoints_queue = new_waypoint_queue

        for elem in current_plan:
            self._waypoints_queue.append(elem)

        self._stop_waypoint_creation = stop_waypoint_creation

    def set_offset(self, offset):
        """Sets an offset for the vehicle 设置车辆的偏移量"""
        self._vehicle_controller.set_offset(offset)

    def run_step(self, debug=False):
        """
        Execute one step of local planning which involves running the longitudinal and lateral PID controllers to
        follow the waypoints trajectory.

        :param debug: boolean flag to activate waypoints debugging
        :return: control to be applied
        :param debug: 布尔值，是否激活路点调试，执行本地规划的一步，运行PID控制器跟随路点轨迹
        :return: 要应用的车辆控制指令
        """
        if self._follow_speed_limits:
            self._target_speed = self._vehicle.get_speed_limit()

        # Add more waypoints too few in the horizon
        if not self._stop_waypoint_creation and len(self._waypoints_queue) < self._min_waypoint_queue_length:
            self._compute_next_waypoints(k=self._min_waypoint_queue_length)

        # Purge the queue of obsolete waypoints
        veh_location = self._vehicle.get_location()
        vehicle_speed = get_speed(self._vehicle) / 3.6
        self._min_distance = self._base_min_distance + self._distance_ratio * vehicle_speed

        num_waypoint_removed = 0
        for waypoint, _ in self._waypoints_queue:

            if len(self._waypoints_queue) - num_waypoint_removed == 1:
                min_distance = 1  # Don't remove the last waypoint until very close by
            else:
                min_distance = self._min_distance

            if veh_location.distance(waypoint.transform.location) < min_distance:
                num_waypoint_removed += 1
            else:
                break

        if num_waypoint_removed > 0:
            for _ in range(num_waypoint_removed):
                self._waypoints_queue.popleft()

        # Get the target waypoint and move using the PID controllers. Stop if no target waypoint
        if len(self._waypoints_queue) == 0:
            control = carla.VehicleControl()
            control.steer = 0.0
            control.throttle = 0.0
            control.brake = 1.0
            control.hand_brake = False
            control.manual_gear_shift = False
        else:
            self.target_waypoint, self.target_road_option = self._waypoints_queue[0]
            control = self._vehicle_controller.run_step(self._target_speed, self.target_waypoint)

        if debug:
            draw_waypoints(self._vehicle.get_world(), [self.target_waypoint], 1.0)

        return control

    def get_incoming_waypoint_and_direction(self, steps=3):
        """
        Returns direction and waypoint at a distance ahead defined by the user.

            :param steps: number of steps to get the incoming waypoint.
            :param steps: 获取前方路点的步数，返回指定步数后的方向和路点
        """
        if len(self._waypoints_queue) > steps:
            return self._waypoints_queue[steps]

        else:
            try:
                wpt, direction = self._waypoints_queue[-1]
                return wpt, direction
            except IndexError:
                return None, RoadOption.VOID

    def get_plan(self):
        """Returns the current plan of the local planner  返回本地规划器当前的计划（路点队列）"""
        return self._waypoints_queue

    def done(self):
        """
        Returns whether or not the planner has finished

        :return: boolean
        返回规划器是否已完成（路点队列是否为空）
        """
        return len(self._waypoints_queue) == 0


def _retrieve_options(list_waypoints, current_waypoint):
    """
    Compute the type of connection between the current active waypoint and the multiple waypoints present in
    list_waypoints. The result is encoded as a list of RoadOption enums.

    :param list_waypoints: list with the possible target waypoints in case of multiple options
    :param current_waypoint: current active waypoint
    :return: list of RoadOption enums representing the type of connection from the active waypoint to each
             candidate in list_waypoints
    :param list_waypoints: 可能的目标路点列表，计算当前活动路点与多个路点之间的连接类型，返回RoadOption枚举列表
    :param current_waypoint: 当前活动路点
    """
    options = []
    for next_waypoint in list_waypoints:
        # this is needed because something we are linking to
        # the beginning of an intersection, therefore the
        # variation in angle is small
        next_next_waypoint = next_waypoint.next(3.0)[0]
        link = _compute_connection(current_waypoint, next_next_waypoint)
        options.append(link)

    return options

#定义一个函数，其功能是计算两个路点（waypoint）之间的拓扑连接类型
def _compute_connection(current_waypoint, next_waypoint, threshold=35):
    """
    Compute the type of topological connection between an active waypoint (current_waypoint) and a target waypoint
    (next_waypoint).

    :param current_waypoint: active waypoint
    :param next_waypoint: target waypoint
    :return: the type of topological connection encoded as a RoadOption enum:
             RoadOption.STRAIGHT
             RoadOption.LEFT
             RoadOption.RIGHT
    :param current_waypoint: 活动路点
    :param next_waypoint: 目标路点
    :param threshold: 角度阈值，计算活动路点与目标路点之间的拓扑连接类型，返回RoadOption枚举
 
    """
    #获取下一个路点的偏航角（yaw angle）
    n = next_waypoint.transform.rotation.yaw
    #将偏航角转换到 0 到 360 度的范围内
    n = n % 360.0
    #获取当前路点的偏航角
    c = current_waypoint.transform.rotation.yaw
    #将当前路点的偏航角转换到 0 到 360 度的范围内
    c = c % 360.0
    #计算两个路点之间的角度差，并将结果转换到 0 到 180 度的范围内
    diff_angle = (n - c) % 180.0
    #如果角度差小于阈值或者大于 180 度减去阈值
    if diff_angle < threshold or diff_angle > (180 - threshold):
        #返回RoadOption.STRAIGHT，表示直走
        return RoadOption.STRAIGHT
    #如果角度差大于 90 度
    elif diff_angle > 90.0:
        #返回RoadOption.LEFT，表示左转
        return RoadOption.LEFT
    #其他情况
    else:
        #返回RoadOption.RIGHT，表示右转
        return RoadOption.RIGHT
