# Copyright (c) # Copyright (c) 2018-2020 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


""" 这个模块实现了一个代理（agent），它在赛道上随机漫游，遵循路径点并避开其他车辆。
该代理还响应交通信号灯、交通标志，并具有不同的可能配置。
 """

import numpy as np
import carla
from agents.navigation.basic_agent import BasicAgent
from agents.navigation.local_planner import RoadOption
from agents.navigation.behavior_types import Cautious, Aggressive, Normal

from agents.tools.misc import get_speed, positive

# 定义行为代理类，继承自BasicAgent，用于实现具有不同行为特性的车辆代理控制逻辑
class BehaviorAgent(BasicAgent):
    """
  BehaviorAgent实现了一个代理，它能够在场景中导航以到达给定的目标目的地，通过计算到达目的地的最短可能路径。
  这个代理能够正确地遵循交通标志、速度限制和交通信号灯，同时还会考虑到附近车辆的情况。
  通过分析周围环境，例如避免被紧跟，可以做出车道变换决策。
  除此之外，代理还可以保持与前方车辆的安全距离，通过跟踪即时的碰撞时间并将其保持在一定范围内。
  最后，代理中编码了不同的行为集合，从谨慎到更具侵略性的行为都有。
    """

    def __init__(self, vehicle, behavior='normal', opt_dict={}, map_inst=None, grp_inst=None):
        """
        Constructor method.

            :param vehicle: actor to apply to local planner logic onto
            :param behavior: type of agent to apply
        """

       # 调用父类（BasicAgent）的构造函数来初始化一些基础属性和功能
        super().__init__(vehicle, opt_dict=opt_dict, map_inst=map_inst, grp_inst=grp_inst)
        self._look_ahead_steps = 0

        # 车辆信息
        self._speed = 0  # 当前车辆速度
        self._speed_limit = 0 # 当前道路的限速
        self._direction = None  # 车辆行驶的目标道路选项（例如车道跟随、变道等）
        self._incoming_direction = None  # 即将进入路段的行驶方向
        self._incoming_waypoint = None  # 即将进入路段对应的路点
        self._min_speed = 5 # 最小速度设定
        self._behavior = None # 代理行为对象，具体类型根据传入的behavior参数确定
        self._sampling_resolution = 4.5 # 采样分辨率，可能用于路径规划等采样相关操作

        # 代理行为参数
        if behavior == 'cautious':
            self._behavior = Cautious()

        elif behavior == 'normal':
            self._behavior = Normal()

        elif behavior == 'aggressive':
            self._behavior = Aggressive()

    def _update_information(self):
        """
        这个方法更新了关于自车（ego vehicle）基于周围世界的信息。
        """
        # 获取当前车辆的速度
        self._speed = get_speed(self._vehicle)
        # 获取当前车辆所在道路的限速，并设置到局部规划器中
        self._speed_limit = self._vehicle.get_speed_limit()
        self._local_planner.set_speed(self._speed_limit)
        # 获取车辆当前的目标道路选项，如果为None则默认为车道跟随
        self._direction = self._local_planner.target_road_option
        if self._direction is None:
            self._direction = RoadOption.LANEFOLLOW

        # 根据限速计算前瞻的步数，这里简单地以限速除以10来估算（具体数值可能需要根据实际情况调整）
        self._look_ahead_steps = int((self._speed_limit) / 10)
     
        # 获取即将进入路段的路点和行驶方向
        self._incoming_waypoint, self._incoming_direction = self._local_planner.get_incoming_waypoint_and_direction(
            steps=self._look_ahead_steps)
        if self._incoming_direction is None:
            self._incoming_direction = RoadOption.LANEFOLLOW

    def traffic_light_manager(self):
        """
        这个方法负责处理红灯的行为。
        """
        actor_list = self._world.get_actors()
        lights_list = actor_list.filter("*traffic_light*")
        affected, _ = self._affected_by_traffic_light(lights_list)

        return affected

    def _tailgating(self, waypoint, vehicle_list):
        """
        这个方法负责处理尾随行为。

            :param location: current location of the agent
            :param waypoint: current waypoint of the agent
            :param vehicle_list: list of all the nearby vehicles
        """

        # 获取当前路径点（waypoint）的左车道标记中的换道信息
    left_turn = waypoint.left_lane_marking.lane_change
    # 获取当前路径点的右车道标记中的换道信息
    right_turn = waypoint.right_lane_marking.lane_change
    # 获取当前路径点的左车道信息
    left_wpt = waypoint.get_left_lane()
    # 获取当前路径点的右车道信息
    right_wpt = waypoint.get_right_lane()
    # 检测当前车辆后方是否有其他车辆接近，并获取相关信息
    # vehicle_list 是车辆列表，min_proximity_threshold 是最小接近阈值，self._speed_limit 是速度限制
    # up_angle_th 和 low_angle_th 是检测角度的阈值
    behind_vehicle_state, behind_vehicle, _ = self._vehicle_obstacle_detected(vehicle_list, max(
        self._behavior.min_proximity_threshold, self._speed_limit / 2), up_angle_th=180, low_angle_th=160)
 
    # 如果检测到后方有车辆且当前车辆速度低于后方车辆速度，则执行换道逻辑
    if behind_vehicle_state and self._speed < get_speed(behind_vehicle):
    # 如果可以向右换道（右换道或双向换道）且右车道是有效的驾驶车道
    if (right_turn == carla.LaneChange.Right or right_turn ==
            carla.LaneChange.Both) and waypoint.lane_id * right_wpt.lane_id > 0 and right_wpt.lane_type == carla.LaneType.Driving:
        # 在右车道上检测是否有新的障碍物
        # lane_offset=1 表示在右侧车道进行检测
        new_vehicle_state, _, _ = self._vehicle_obstacle_detected(vehicle_list, max(
            self._behavior.min_proximity_threshold, self._speed_limit / 2), up_angle_th=180, lane_offset=1)
        # 如果没有新的障碍物，则执行向右换道的动作
        if not new_vehicle_state:
            print("尾随车辆，向右换道！")
            # 获取目标路径点和右车道路径点的位置
            end_waypoint = self._local_planner.target_waypoint
            # 设置一个尾随计数器（可能用于后续的逻辑处理）
            self._behavior.tailgate_counter = 200
            # 设置新的目的地为右车道路径点的位置
            self.set_destination(end_waypoint.transform.location,
                                 right_wpt.transform.location)
    # 如果可以向左换道且左车道是有效的驾驶车道
    elif left_turn == carla.LaneChange.Left and waypoint.lane_id * left_wpt.lane_id > 0 and left_wpt.lane_type == carla.LaneType.Driving:
        # 在左车道上检测是否有新的障碍物
        # lane_offset=-1 表示在左侧车道进行检测
        new_vehicle_state, _, _ = self._vehicle_obstacle_detected(vehicle_list, max(
            self._behavior.min_proximity_threshold, self._speed_limit / 2), up_angle_th=180, lane_offset=-1)
        # 如果没有新的障碍物，则执行向左换道的动作
        if not new_vehicle_state:
            print("尾随车辆，向左换道！")
            # 获取目标路径点和左车道路径点的位置
            end_waypoint = self._local_planner.target_waypoint
            # 设置尾随计数器
            self._behavior.tailgate_counter = 200
            # 设置新的目的地为左车道路径点的位置
            self.set_destination(end_waypoint.transform.location,
                                 left_wpt.transform.location)
    def collision_and_car_avoid_manager(self, waypoint):
        """
        这个模块负责在发生碰撞的情况下发出警告，并管理可能的尾随机会。

            :param location: current location of the agent
            :param waypoint: current waypoint of the agent
            :return vehicle_state: True if there is a vehicle nearby, False if not
            :return vehicle: nearby vehicle
            :return distance: distance to nearby vehicle
        """

        vehicle_list = self._world.get_actors().filter("*vehicle*")
        def dist(v): return v.get_location().distance(waypoint.transform.location)
        vehicle_list = [v for v in vehicle_list if dist(v) < 45 and v.id != self._vehicle.id]

        if self._direction == RoadOption.CHANGELANELEFT:
            vehicle_state, vehicle, distance = self._vehicle_obstacle_detected(
                vehicle_list, max(
                    self._behavior.min_proximity_threshold, self._speed_limit / 2), up_angle_th=180, lane_offset=-1)
        elif self._direction == RoadOption.CHANGELANERIGHT:
            vehicle_state, vehicle, distance = self._vehicle_obstacle_detected(
                vehicle_list, max(
                    self._behavior.min_proximity_threshold, self._speed_limit / 2), up_angle_th=180, lane_offset=1)
        else:
            vehicle_state, vehicle, distance = self._vehicle_obstacle_detected(
                vehicle_list, max(
                    self._behavior.min_proximity_threshold, self._speed_limit / 3), up_angle_th=30)

            # Check for tailgating检查跟车过近（的情况）。
            if not vehicle_state and self._direction == RoadOption.LANEFOLLOW \
                    and not waypoint.is_junction and self._speed > 10 \
                    and self._behavior.tailgate_counter == 0:
                self._tailgating(waypoint, vehicle_list)

        return vehicle_state, vehicle, distance

    def pedestrian_avoid_manager(self, waypoint):
        """
        这个模块负责在与任何行人发生碰撞的情况下发出警告。

            :param location: current location of the agent
            :param waypoint: current waypoint of the agent
            :return vehicle_state: True if there is a walker nearby, False if not
            :return vehicle: nearby walker
            :return distance: distance to nearby walker
        """

        walker_list = self._world.get_actors().filter("*walker.pedestrian*")
        def dist(w): return w.get_location().distance(waypoint.transform.location)
        walker_list = [w for w in walker_list if dist(w) < 10]

        if self._direction == RoadOption.CHANGELANELEFT:
            walker_state, walker, distance = self._vehicle_obstacle_detected(walker_list, max(
                self._behavior.min_proximity_threshold, self._speed_limit / 2), up_angle_th=90, lane_offset=-1)
        elif self._direction == RoadOption.CHANGELANERIGHT:
            walker_state, walker, distance = self._vehicle_obstacle_detected(walker_list, max(
                self._behavior.min_proximity_threshold, self._speed_limit / 2), up_angle_th=90, lane_offset=1)
        else:
            walker_state, walker, distance = self._vehicle_obstacle_detected(walker_list, max(
                self._behavior.min_proximity_threshold, self._speed_limit / 3), up_angle_th=60)

        return walker_state, walker, distance

    def car_following_manager(self, vehicle, distance, debug=False):
        """
        这个模块负责管理当我们前方有车辆时的跟车行为。

            :param vehicle: car to follow
            :param distance: distance from vehicle
            :param debug: boolean for debugging
            :return control: carla.VehicleControl
        """

        vehicle_speed = get_speed(vehicle)
        delta_v = max(1, (self._speed - vehicle_speed) / 3.6)
        ttc = distance / delta_v if delta_v != 0 else distance / np.nextafter(0., 1.)

        # 在安全时间距离下，减速。
        if self._behavior.safety_time > ttc > 0.0:
            target_speed = min([
                positive(vehicle_speed - self._behavior.speed_decrease),
                self._behavior.max_speed,
                self._speed_limit - self._behavior.speed_lim_dist])
            self._local_planner.set_speed(target_speed)
            control = self._local_planner.run_step(debug=debug)

        # 在实际安全距离范围内，尝试跟随前方车辆的速度。
        elif 2 * self._behavior.safety_time > ttc >= self._behavior.safety_time:
            target_speed = min([
                max(self._min_speed, vehicle_speed),
                self._behavior.max_speed,
                self._speed_limit - self._behavior.speed_lim_dist])
            self._local_planner.set_speed(target_speed)
            control = self._local_planner.run_step(debug=debug)

        # 正常行为。
        else:
            target_speed = min([
                self._behavior.max_speed,
                self._speed_limit - self._behavior.speed_lim_dist])
            self._local_planner.set_speed(target_speed)
            control = self._local_planner.run_step(debug=debug)

        return control

    def run_step(self, debug=False):
        """
       执行导航的一步。

            :param debug: boolean for debugging
            :return control: carla.VehicleControl
        """
        self._update_information()

        control = None
        if self._behavior.tailgate_counter > 0:
            self._behavior.tailgate_counter -= 1

        ego_vehicle_loc = self._vehicle.get_location()
        ego_vehicle_wp = self._map.get_waypoint(ego_vehicle_loc)

        # 1: 处理红灯和停车行为。
        if self.traffic_light_manager():
            return self.emergency_stop()

        # 2.1: 行人避让行为。
        walker_state, walker, w_distance = self.pedestrian_avoid_manager(ego_vehicle_wp)

        if walker_state:
            # 距离是从两辆车的中心计算得出的
            # 我们使用边界框来计算实际距离。
            distance = w_distance - max(
                walker.bounding_box.extent.y, walker.bounding_box.extent.x) - max(
                    self._vehicle.bounding_box.extent.y, self._vehicle.bounding_box.extent.x)

            # 如果车辆非常接近，则紧急刹车。
            if distance < self._behavior.braking_distance:
                return self.emergency_stop()

        #2.2：跟车行为
        vehicle_state, vehicle, distance = self.collision_and_car_avoid_manager(ego_vehicle_wp)

        if vehicle_state:
            # 距离是从两辆车的中心计算的。
            # 我们使用边界框来计算实际距离。
            distance = distance - max(
                vehicle.bounding_box.extent.y, vehicle.bounding_box.extent.x) - max(
                    self._vehicle.bounding_box.extent.y, self._vehicle.bounding_box.extent.x)

            # 如果车辆非常接近，就紧急刹车。
            if distance < self._behavior.braking_distance:
                return self.emergency_stop()
            else:
                control = self.car_following_manager(vehicle, distance)

        # 3: Intersection behavior
        #条件判断即将到达路口方向是左还是右
        elif self._incoming_waypoint.is_junction and (self._incoming_direction in [RoadOption.LEFT, RoadOption.RIGHT]):
            #确定目标速度，获取较小值
            target_speed = min([
                self._behavior.max_speed,
                self._speed_limit - 5])
            #计算得到的目标速度target_speed设置到self._local_planner中
            self._local_planner.set_speed(target_speed)
            #计算出车辆的控制指令
            control = self._local_planner.run_step(debug=debug)

        # 4: Normal behavior
        else:
            #目标速度取最小值
            target_speed = min([
                self._behavior.max_speed,
                self._speed_limit - self._behavior.speed_lim_dist])
            self._local_planner.set_speed(target_speed)
            control = self._local_planner.run_step(debug=debug)

        return control

    #目的是对车辆控制进行修改，以实现紧急制动
    def emergency_stop(self):
        """
        Overwrites the throttle a brake values of a control to perform an emergency stop.
        The steering is kept the same to avoid going out of the lane when stopping during turns

            :param speed (carl.VehicleControl): control to be modified
        """
        #创建一个新的对象VehicleControl
        control = carla.VehicleControl()
        #将油门值设置为0.0
        control.throttle = 0.0
        #将刹车值设置为最大
        control.brake = self._max_brake
        #将手刹设置为Flase
        control.hand_brake = False
        #将修改后的车辆控制对象返回
        return control
