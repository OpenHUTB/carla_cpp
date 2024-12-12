#!/usr/bin/env python

# Copyright (c) 2018 Intel Labs.
# authors: German Ros (german.ros@intel.com)
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

""" Module with auxiliary functions. """

import math
import numpy as np
import carla

# 定义一个极小的正数，用于避免除零错误或作为极小的增量
_EPS = np.finfo(float).eps


def draw_waypoints(world, waypoints, z=0.5):
    """
    Draw a list of waypoints at a certain height given in z.

        :param world: carla.world object  # 接收一个 Carla 世界对象
        :param waypoints: list or iterable container with the waypoints to draw  # 接收一个包含待绘制路点的列表或可迭代容器
        :param z: height in meters  # 绘制路点的高度，单位为米
    """
    for wpt in waypoints:
        # 获取路点的变换信息
        wpt_t = wpt.transform
        # 将路点位置提升到指定高度 z
        begin = wpt_t.location + carla.Location(z=z)
        # 将路点的旋转角度从度转换为弧度
        angle = math.radians(wpt_t.rotation.yaw)
        # 计算箭头终点的位置
        end = begin + carla.Location(x=math.cos(angle), y=math.sin(angle))
        # 在世界中绘制箭头，箭头大小为 0.3，生命周期为 1.0 秒
        world.debug.draw_arrow(begin, end, arrow_size=0.3, life_time=1.0)


def get_speed(vehicle):
    """
    Compute speed of a vehicle in Km/h.

        :param vehicle: the vehicle for which speed is calculated  # 接收一个车辆对象
        :return: speed as a float in Km/h  # 以千米/小时为单位的车辆速度
    """
    # 获取车辆的速度向量
    vel = vehicle.get_velocity()
    # 计算车辆的速度大小并转换为千米/小时
    return 3.6 * math.sqrt(vel.x ** 2 + vel.y ** 2 + vel.z ** 2)


def get_trafficlight_trigger_location(traffic_light):
    """
    Calculates the yaw of the waypoint that represents the trigger volume of the traffic light
    """
    def rotate_point(point, radians):
        """
        rotate a given point by a given angle  # 将给定点按给定角度旋转
        """
        # 旋转点的 x 坐标
        rotated_x = math.cos(radians) * point.x - math.sin(radians) * point.y
        # 旋转点的 y 坐标
        rotated_y = math.sin(radians) * point.x - math.cos(radians) * point.y
        # 返回旋转后的点
        return carla.Vector3D(rotated_x, rotated_y, point.z)

    # 获取交通灯的变换信息
    base_transform = traffic_light.get_transform()
    # 获取交通灯的旋转角度
    base_rot = base_transform.rotation.yaw
    # 获取交通灯触发区域的位置
    area_loc = base_transform.transform(traffic_light.trigger_volume.location)
    # 获取交通灯触发区域的范围
    area_ext = traffic_light.trigger_volume.extent
    # 旋转触发区域的点
    point = rotate_point(carla.Vector3D(0, 0, area_ext.z), math.radians(base_rot))
    # 计算旋转后的点的位置
    point_location = area_loc + carla.Location(x=point.x, y=point.y)
    # 返回旋转后的位置
    return carla.Location(point_location.x, point_location.y, point_location.z)


def is_within_distance(target_transform, reference_transform, max_distance, angle_interval=None):
    """
    Check if a location is both within a certain distance from a reference object.
    By using 'angle_interval', the angle between the location and reference transform
    will also be taken into account, being 0 a location in front and 180, one behind.

    :param target_transform: location of the target object  # 目标对象的位置
    :param reference_transform: location of the reference object  # 参考对象的位置
    :param max_distance: maximum allowed distance  # 允许的最大距离
    :param angle_interval: only locations between [min, max] angles will be considered. This isn't checked by default.  # 可选的角度范围，默认为 None
    :return: boolean  # 是否在距离和角度范围内
    """
    # 计算目标位置和参考位置的向量
    target_vector = np.array([
        target_transform.location.x - reference_transform.location.x,
        target_transform.location.y - reference_transform.location.y
    ])
    # 计算目标向量的范数
    norm_target = np.linalg.norm(target_vector)
    # 如果向量长度太小，认为在范围内
    if norm_target < 0.001:
        return True
    # 超出最大距离则不在范围内
    if norm_target > max_distance:
        return False
    # 如果不考虑角度，只检查距离，认为在范围内
    if not angle_interval:
        return True
    # 获取角度范围的上下限
    min_angle = angle_interval[0]
    max_angle = angle_interval[1]
    # 获取参考对象的前向向量
    fwd = reference_transform.get_forward_vector()
    forward_vector = np.array([fwd.x, fwd.y])
    # 计算目标向量和前向向量的夹角
    angle = math.degrees(math.acos(np.clip(np.dot(forward_vector, target_vector) / norm_target, -1., 1.)))
    # 判断夹角是否在指定范围内
    return min_angle < angle < max_angle


def compute_magnitude_angle(target_location, current_location, orientation):
    """
    Compute relative angle and distance between a target_location and a current_location

        :param target_location: location of the target object  # 目标对象的位置
        :param current_location: location of the reference object  # 参考对象的位置
        :param orientation: orientation of the reference object  # 参考对象的朝向
        :return: a tuple composed by the distance to the object and the angle between both objects  # 距离和角度的元组
    """
    # 计算目标位置和参考位置的向量
    target_vector = np.array([target_location.x - current_location.x, target_location.y - current_location.y])
    # 计算目标向量的范数
    norm_target = np.linalg.norm(target_vector)
    # 计算参考对象的前向单位向量
    forward_vector = np.array([math.cos(math.radians(orientation)), math.sin(math.radians(orientation))])
    # 计算目标向量和前向向量的夹角
    d_angle = math.degrees(math.acos(np.clip(np.dot(forward_vector, target_vector) / norm_target, -1., 1.)))
    # 返回距离和夹角
    return (norm_target, d_angle)


def distance_vehicle(waypoint, vehicle_transform):
    """
    Returns the 2D distance from a waypoint to a vehicle

        :param waypoint: actual waypoint  # 实际的路点
        :param vehicle_transform: transform of the target vehicle  # 目标车辆的变换信息
    """
    # 获取车辆的位置
    loc = vehicle_transform.location
    # 计算路点和车辆位置在 x 轴上的距离
    x = waypoint.transform.location.x - loc.x
    # 计算路点和车辆位置在 y 轴上的距离
    y = waypoint.transform.location.y - loc.y
    # 计算路点和车辆的 2D 距离
    return math.sqrt(x * x + y * y)


def vector(location_1, location_2):
    """
    Returns the unit vector from location_1 to location_2

        :param location_1, location_2: carla.Location objects  # 接收两个 Carla 位置对象

   .. note::
        Alternatively you can use:
        `(location_2 - location_1).make_unit_vector()`  # 可使用另一种方式计算单位向量
    """
    # 计算位置差在 x 轴上的分量
    x = location_2.x - location_1.x
    # 计算位置差在 y 轴上的分量
    y = location_2.y - location_1.y
    # 计算位置差在 z 轴上的分量
    z = location_2.z - location_1.z
    # 计算位置差的范数并加上一个极小的增量以避免除零错误
    norm = np.linalg.norm([x, y, z]) + _EPS
    # 计算单位向量
    return [x / norm, y / norm, z / norm]


def compute_distance(location_1, location_2):
    """
    Euclidean distance between 3D points

        :param location_1, location_2: 3D points  # 接收两个 3D 点

   .. deprecated:: 0.9.13
        Use `location_1.distance(location_2)` instead  # 已弃用，建议使用 `location_1.distance(location_2)` 替代
    """
    # 计算位置差在 x 轴上的分量
    x = location_2.x - location_1.x
    # 计算位置差在 y 轴上的分量
    y = location_2.y - location_1.y
    # 计算位置差在 z 轴上的分量
    z = location_2.z - location_2.z
    # 计算位置差的范数并加上一个极小的增量以避免除零错误
    norm = np.linalg.norm([x, y, z]) + _EPS
    # 返回距离
    return norm


def positive(num):
    """
    Return the given number if positive, else 0

        :param num: value to check  # 要检查的数字
    """
    # 如果数字大于 0 则返回该数字，否则返回 0
    return num if num > 0.0 else 0.0
