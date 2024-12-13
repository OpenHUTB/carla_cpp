# Copyright (c) # Copyright (c) 2018-2020 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

""" 该模块包含PID控制器，用于执行横向和纵向控制。 """

from collections import deque
import math
import numpy as np
import carla
from agents.tools.misc import get_speed


class VehiclePIDController:
    """
    VehiclePIDController是两个PID控制器（横向和纵向）的组合
    用于从客户端执行车辆的低层次控制
    """


    def __init__(self, vehicle, args_lateral, args_longitudinal, offset=0, max_throttle=0.75, max_brake=0.3,
                 max_steering=0.8):
        """
        构造方法。

        :param vehicle: 应用局部规划逻辑的actor
        :param args_lateral: 设置横向PID控制器的参数字典
            使用以下语义：
                K_P -- 比例项
                K_D -- 微分项
                K_I -- 积分项
        :param args_longitudinal: 设置纵向PID控制器的参数字典
            使用以下语义：
                K_P -- 比例项
                K_D -- 微分项
                K_I -- 积分项
        :param offset: 如果不为零，车辆将从中心线偏移驾驶。
            正值意味着向右偏移，而负值意味着向左偏移。数值足够大
            以至于导致车辆穿过其他车道可能会破坏控制器。
        """

        self.max_brake = max_brake
        self.max_throt = max_throttle
        self.max_steer = max_steering

        self._vehicle = vehicle
        self._world = self._vehicle.get_world()
        self.past_steering = self._vehicle.get_control().steer
        self._lon_controller = PIDLongitudinalController(self._vehicle, **args_longitudinal)
        self._lat_controller = PIDLateralController(self._vehicle, offset, **args_lateral)

    def run_step(self, target_speed, waypoint):
        """
       执行一步控制，调用横向和纵向PID控制器
        以达到给定目标速度的目标路点。

            :param target_speed: 期望的车辆速度
            :param waypoint: 编码为目标位置的路点
            :return: 到路点的距离（以米为单位）
        """

        acceleration = self._lon_controller.run_step(target_speed)
        current_steering = self._lat_controller.run_step(waypoint)
        control = carla.VehicleControl()
        if acceleration >= 0.0:
            control.throttle = min(acceleration, self.max_throt)
            control.brake = 0.0
        else:
            control.throttle = 0.0
            control.brake = min(abs(acceleration), self.max_brake)

        #方向盘调节：不能突然变化，不能转得太多。

        if current_steering > self.past_steering + 0.1:
            current_steering = self.past_steering + 0.1
        elif current_steering < self.past_steering - 0.1:
            current_steering = self.past_steering - 0.1

        if current_steering >= 0:
            steering = min(self.max_steer, current_steering)
        else:
            steering = max(-self.max_steer, current_steering)

        control.steer = steering
        control.hand_brake = False
        control.manual_gear_shift = False
        self.past_steering = steering

        return control


    def change_longitudinal_PID(self, args_longitudinal):
        """更改PIDLongitudinalController的参数"""
        self._lon_controller.change_parameters(**args_longitudinal)

    def change_lateral_PID(self, args_lateral):
        """更改PIDLateralController的参数"""
        self._lat_controller.change_parameters(**args_lateral)

    def set_offset(self, offset):
        """更改偏移量"""
        self._lat_controller.set_offset(offset)


class PIDLongitudinalController:
    """
    PIDLongitudinalController使用PID实现纵向控制。
    """

    def __init__(self, vehicle, K_P=1.0, K_I=0.0, K_D=0.0, dt=0.03):
        """
       构造方法。

            :param vehicle: 应用局部规划逻辑的actor
            :param K_P: 比例项
            :param K_D: 微分项
            :param K_I: 积分项
            :param dt: 时间微分，以秒为单位
        """
        self._vehicle = vehicle
        self._k_p = K_P
        self._k_i = K_I
        self._k_d = K_D
        self._dt = dt
        self._error_buffer = deque(maxlen=10)

    def run_step(self, target_speed, debug=False):
        """
      执行一步纵向控制以达到给定的目标速度。

            :param target_speed: 目标速度，单位为Km/h
            :param debug: 调试布尔值
            :return: 油门控制
        """
        current_speed = get_speed(self._vehicle)

        if debug:
            print('Current speed = {}'.format(current_speed))

        return self._pid_control(target_speed, current_speed)

    def _pid_control(self, target_speed, current_speed):
        """
        基于PID方程估计车辆的油门/刹车

            :param target_speed: 目标速度，单位为Km/h
            :param current_speed: 车辆当前速度，单位为Km/h
            :return: 油门/刹车控制
        """

        error = target_speed - current_speed
        self._error_buffer.append(error)

        if len(self._error_buffer) >= 2:
            _de = (self._error_buffer[-1] - self._error_buffer[-2]) / self._dt
            _ie = sum(self._error_buffer) * self._dt
        else:
            _de = 0.0
            _ie = 0.0

        return np.clip((self._k_p * error) + (self._k_d * _de) + (self._k_i * _ie), -1.0, 1.0)

    def change_parameters(self, K_P, K_I, K_D, dt):
        """更改PID参数"""
        self._k_p = K_P
        self._k_i = K_I
        self._k_d = K_D
        self._dt = dt


class PIDLateralController:
    """
    PIDLateralController使用PID实现横向控制。
    """

    def __init__(self, vehicle, offset=0, K_P=1.0, K_I=0.0, K_D=0.0, dt=0.03):
        """
        构造方法。

            :param vehicle: 应用局部规划逻辑的actor
            :param offset: 距离中心线的距离。如果值足够大，可能会导致车辆侵入其他车道。
            :param K_P: 比例项
            :param K_D: 微分项
            :param K_I: 积分项
            :param dt: 时间微分，以秒为单位
        """
        self._vehicle = vehicle
        self._k_p = K_P
        self._k_i = K_I
        self._k_d = K_D
        self._dt = dt
        self._offset = offset
        self._e_buffer = deque(maxlen=10)

    def run_step(self, waypoint):
        """
        执行一步横向控制，使车辆朝某个路点转向。

            :param waypoint: 目标路点
            :return: 方向盘控制，在[-1, 1]范围内，其中：
            -1 表示最大左转
            +1 表示最大右转
        """
        return self._pid_control(waypoint, self._vehicle.get_transform())

    def set_offset(self, offset):
        """更改偏移量"""
        self._offset = offset

    def _pid_control(self, waypoint, vehicle_transform):
        """
        基于PID方程估计车辆的转向角度

            :param waypoint: 目标路点
            :param vehicle_transform: 车辆当前的变换
            :return: 方向盘控制，在[-1, 1]范围内
        """
        # 获取自车的定位和前向向量
        ego_loc = vehicle_transform.location
        v_vec = vehicle_transform.get_forward_vector()
        v_vec = np.array([v_vec.x, v_vec.y, 0.0])

        # 获取车辆-目标路点的向量
        if self._offset != 0:
            # 将路点侧移
            w_tran = waypoint.transform
            r_vec = w_tran.get_right_vector()
            w_loc = w_tran.location + carla.Location(x=self._offset*r_vec.x,
                                                         y=self._offset*r_vec.y)
        else:
            w_loc = waypoint.transform.location

        w_vec = np.array([w_loc.x - ego_loc.x,
                          w_loc.y - ego_loc.y,
                          0.0])

        wv_linalg = np.linalg.norm(w_vec) * np.linalg.norm(v_vec)
        if wv_linalg == 0:
            _dot = 1
        else:
            _dot = math.acos(np.clip(np.dot(w_vec, v_vec) / (wv_linalg), -1.0, 1.0))
        _cross = np.cross(v_vec, w_vec)
        if _cross[2] < 0:
            _dot *= -1.0

        self._e_buffer.append(_dot)
        if len(self._e_buffer) >= 2:
            _de = (self._e_buffer[-1] - self._e_buffer[-2]) / self._dt
            _ie = sum(self._e_buffer) * self._dt
        else:
            _de = 0.0
            _ie = 0.0

        return np.clip((self._k_p * _dot) + (self._k_d * _de) + (self._k_i * _ie), -1.0, 1.0)

    def change_parameters(self, K_P, K_I, K_D, dt):
        """更改PID参数"""
        self._k_p = K_P
        self._k_i = K_I
        self._k_d = K_D
        self._dt = dt
