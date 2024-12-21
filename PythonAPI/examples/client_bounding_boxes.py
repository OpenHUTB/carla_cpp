#!/usr/bin/env python

# Copyright (c) 2019 Aptiv
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
An example of client-side bounding boxes with basic car controls.

Controls:

    W            : throttle
    S            : brake
    AD           : steer
    Space        : hand-brake

    ESC          : quit
"""

# ==============================================================================
# -- find carla module ---------------------------------------------------------
# ==============================================================================

import glob
import os
import sys

# 尝试将Carla模块所在路径添加到系统路径中，以便后续能够正确导入Carla相关模块
# 根据Python版本和操作系统类型来确定要添加的Carla模块的.egg文件路径
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass


# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import carla
import weakref
import random

# 尝试导入pygame模块，用于创建游戏界面、处理用户输入等相关操作
# 如果导入失败，抛出运行时错误提示安装pygame包
try:
    import pygame
    from pygame.locals import K_ESCAPE
    from pygame.locals import K_SPACE
    from pygame.locals import K_a
    from pygame.locals import K_d
    from pygame.locals import K_s
    from pygame.locals import K_w
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

# 尝试导入numpy模块，用于数值计算和数组操作等
# 如果导入失败，抛出运行时错误提示安装numpy包
try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')

# 定义相机视图的宽度（进行了减半处理）
VIEW_WIDTH = 1920 // 2
# 定义相机视图的高度（进行了减半处理）
VIEW_HEIGHT = 1080 // 2
# 定义相机的视野角度
VIEW_FOV = 90

# 定义用于绘制边界框的颜色（RGB格式）
BB_COLOR = (248, 64, 24)


# ==============================================================================
# -- ClientSideBoundingBoxes ---------------------------------------------------
# ==============================================================================

class ClientSideBoundingBoxes(object):
    """
    This is a module responsible for creating 3D bounding boxes and drawing them
    client-side on pygame surface.
    """

    @staticmethod
    def get_bounding_boxes(vehicles, camera):
        """
        Creates 3D bounding boxes based on carla vehicle list and camera.

        Args:
            vehicles (list): 包含Carla中车辆对象的列表，用于生成对应的边界框
            camera (carla.Camera): 相机对象，用于确定边界框在相机视角下的位置和形状

        Returns:
            list: 经过筛选后的车辆边界框列表，过滤掉了位于相机后方的车辆对应的边界框
        """

        # 为每辆车生成对应的边界框
        bounding_boxes = [ClientSideBoundingBoxes.get_bounding_box(vehicle, camera) for vehicle in vehicles]
        # 过滤掉位于相机后方（z坐标小于等于0）的车辆对应的边界框
        bounding_boxes = [bb for bb in bounding_boxes if all(bb[:, 2] > 0)]
        return bounding_boxes

    @staticmethod
    def draw_bounding_boxes(display, bounding_boxes):
        """
        Draws bounding boxes on pygame display.

        Args:
            display (pygame.Surface): 要在其上绘制边界框的Pygame显示表面对象
            bounding_boxes (list): 包含车辆边界框坐标信息的列表，用于绘制显示
        """

        # 创建一个用于绘制边界框的透明表面，大小与视图尺寸相同
        bb_surface = pygame.Surface((VIEW_WIDTH, VIEW_HEIGHT))
        bb_surface.set_colorkey((0, 0, 0))
        for bbox in bounding_boxes:
            # 将边界框的坐标转换为整数坐标点列表，方便后续绘制线条
            points = [(int(bbox[i, 0]), int(bbox[i, 1])) for i in range(8)]
            # 绘制边界框的底面线条
            pygame.draw.line(bb_surface, BB_COLOR, points[0], points[1])
            pygame.draw.line(bb_surface, BB_COLOR, points[0], points[1])
            pygame.draw.line(bb_surface, BB_COLOR, points[1], points[2])
            pygame.draw.line(bb_surface, BB_COLOR, points[2], points[3])
            pygame.draw.line(bb_surface, BB_COLOR, points[3], points[0])
            # 绘制边界框的顶面线条
            pygame.draw.line(bb_surface, BB_COLOR, points[4], points[5])
            pygame.draw.line(bb_surface, BB_COLOR, points[5], points[6])
            pygame.draw.line(bb_surface, BB_COLOR, points[6], points[7])
            pygame.draw.line(bb_surface, BB_COLOR, points[7], points[4])
            # 绘制连接底面和顶面的线条
            pygame.draw.line(bb_surface, BB_COLOR, points[0], points[4])
            pygame.draw.line(bb_surface, BB_COLOR, points[1], points[5])
            pygame.draw.line(bb_surface, BB_COLOR, points[2], points[6])
            pygame.draw.line(bb_surface, BB_COLOR, points[3], points[7])
        # 将绘制好边界框的表面绘制到显示表面上
        display.blit(bb_surface, (0, 0))

    @staticmethod
    def get_bounding_box(vehicle, camera):
        """
        Returns 3D bounding box for a vehicle based on camera view.

        Args:
            vehicle (carla.Vehicle): 车辆对象，用于获取其边界框信息
            camera (carla.Camera): 相机对象，用于确定车辆边界框在该相机视角下的坐标

        Returns:
            numpy.ndarray: 经过转换后的车辆边界框在相机坐标系下的坐标信息
        """

        # 获取车辆的边界框坐标点（在车辆本地坐标系下）
        bb_cords = ClientSideBoundingBoxes._create_bb_points(vehicle)
        # 将车辆边界框坐标从车辆坐标系转换到传感器（相机）坐标系，并取前3维（去除齐次坐标维度）
        cords_x_y_z = ClientSideBoundingBoxes._vehicle_to_sensor(bb_cords, vehicle, camera)[:3, :]
        # 调整坐标顺序，变为 [y, -z, x] 的顺序，符合后续计算和绘制要求
        cords_y_minus_z_x = np.concatenate([cords_x_y_z[1, :], -cords_x_y_z[2, :], cords_x_y_z[0, :]])
        # 通过相机校准矩阵对坐标进行变换
        bbox = np.transpose(np.dot(camera.calibration, cords_y_minus_z_x))
        # 进行透视除法，将坐标转换到归一化设备坐标（NDC）空间，方便后续绘制
        camera_bbox = np.concatenate([bbox[:, 0] / bbox[:, 2], bbox[:, 1] / bbox[:, 2], bbox[:, 2]], axis=1)
        return camera_bbox

    @staticmethod
    def _create_bb_points(vehicle):
        """
        Returns 3D bounding box for a vehicle.

        Args:
            vehicle (carla.Vehicle): 车辆对象，基于其尺寸等信息生成边界框坐标点

        Returns:
            numpy.ndarray: 车辆边界框在车辆本地坐标系下的坐标信息，形状为 (8, 4)，其中4维表示齐次坐标 (x, y, z, 1)
        """

        cords = np.zeros((8, 4))
        extent = vehicle.bounding_box.extent
        cords[0, :] = np.array([extent.x, extent.y, -extent.z, 1])
        cords[1, :] = np.array([-extent.x, extent.y, -extent.z, 1])
        cords[2, :] = np.array([-extent.x, -extent.y, -extent.z, 1])
        cords[3, :] = np.array([extent.x, -extent.y, -extent.z, 1])
        cords[4, :] = np.array([extent.x, extent.y, extent.z, 1])
        cords[5, :] = np.array([-extent.x, extent.y, extent.z, 1])
        cords[6, :] = np.array([-extent.x, -extent.y, extent.z, 1])
        cords[7, :] = np.array([extent.x, -extent.y, extent.z, 1])
        return cords

    @staticmethod
    def _vehicle_to_sensor(cords, vehicle, sensor):
        """
        Transforms coordinates of a vehicle bounding box to sensor.

        Args:
            cords (numpy.ndarray): 车辆边界框在车辆本地坐标系下的坐标信息
            vehicle (carla.Vehicle): 车辆对象，用于确定其在世界坐标系中的位置和姿态
            sensor (carla.Sensor): 传感器（如相机）对象，用于将坐标转换到传感器坐标系

        Returns:
            numpy.ndarray: 车辆边界框在传感器坐标系下的坐标信息
        """

        world_cord = ClientSideBoundingBoxes._vehicle_to_world(cords, vehicle)
        sensor_cord = ClientSideBoundingBoxes._world_to_sensor(world_cord, sensor)
        return sensor_cord

    @staticmethod
    def _vehicle_to_world(cords, vehicle):
        """
        Transforms coordinates of a vehicle bounding box to world.

        Args:
            cords (numpy.ndarray): 车辆边界框在车辆本地坐标系下的坐标信息
            vehicle (carla.Vehicle): 车辆对象，用于获取其在世界坐标系中的位置和姿态信息

        Returns:
            numpy.ndarray: 车辆边界框在世界坐标系下的坐标信息
        """

        bb_transform = carla.Transform(vehicle.bounding_box.location)
        bb_vehicle_matrix = ClientSideBoundingBoxes.get_matrix(bb_transform)
        vehicle_world_matrix = ClientSideBoundingBoxes.get_matrix(vehicle.get_transform())
        bb_world_matrix = np.dot(vehicle_world_matrix, bb_vehicle_matrix)
        world_cords = np.dot(bb_world_matrix, np.transpose(cords))
        return world_cords

    @staticmethod
    def _world_to_sensor(cords, sensor):
        """
        Transforms world coordinates to sensor.

        Args:
            cords (numpy.ndarray): 车辆边界框在世界坐标系下的坐标信息
            sensor (carla.Sensor): 传感器（如相机）对象，用于将世界坐标系坐标转换到传感器坐标系

        Returns:
            numpy.ndarray: 车辆边界框在传感器坐标系下的坐标信息
        """

        sensor_world_matrix = ClientSideBoundingBoxes.get_matrix(sensor.get_transform())
        world_sensor_matrix = np.linalg.inv(sensor_world_matrix)
        sensor_cords = np.dot(world_sensor_matrix, cords)
        return sensor_cords

    @staticmethod
    def get_matrix(transform):
        """
        Creates matrix from carla transform.

        Args:
            transform (carla.Transform): Carla中的变换对象，包含位置和旋转信息

        Returns:
            numpy.matrix: 对应的齐次变换矩阵，用于坐标变换等操作
        """

        rotation = transform.rotation
        location = transform.location
        c_y = np.cos(np.radians(rotation.yaw))
        s_y = np.sin(np.radians(rotation.yaw))
        c_r = np.cos(np.radians(rotation.roll))
        s_r = np.sin(np.radians(rotation.roll))
        c_p = np.cos(np.radians(rotation.pitch))
        s_p = np.sin(np.radians(rotation.pitch))
        matrix = np.matrix(np.identity(4))
        matrix[0, 3] = location.x
        matrix[1, 3] = location.y
        matrix[2, 3] = location.z
        matrix[0, 0] = c_p * c_y
        matrix[0, 1] = c_y * s_p * s_r - s_y * c_r
        matrix[0, 2] = -c_y * s_p * c_r - s_y * s_r
        matrix[1, 0] = s_y * c_p
        matrix[1, 1] = s_y * s_p * s_r + c_y * c_r
        matrix[1, 2] = -s_y * s_p * c_r + c_y * s_r
        matrix[2, 0] = s_p
        matrix[2, 1] = -c_p * s_r
        matrix[2, 2] = c_p * c_r
        return matrix


# ==============================================================================
# -- BasicSynchronousClient ----------------------------------------------------
# ==============================================================================

class BasicSynchronousClient(object):
    """
    Basic implementation of a synchronous client.
    """

    def __init__(self):
        # Carla客户端对象，用于与Carla服务器通信
        self.client = None
        # Carla世界对象，包含场景中的所有实体和环境信息
        self.world = None
        # 相机对象，用于获取场景图像
        self.camera = None
        # 车辆对象，代表可控制的车辆
        self.car = None

        # Pygame显示表面对象，用于显示图像和绘制内容
        self.display = None
        # 当前从相机获取的图像数据
        self.image = None
        # 用于控制图像捕获的标志，实现同步机制
        self.capture = True

    def camera_blueprint(self):
        """
        Returns camera blueprint.

        Returns:
            carla.ActorBlueprint: 相机的蓝图对象，可用于设置相机的各种属性并生成相机实例
        """

        camera_bp = self.world.get_blueprint_library().find('sensor.camera.rgb')
        camera_bp.set_attribute('image_size_x', str(VIEW_WIDTH))
        camera_bp.set_attribute('image_size_y', str(VIEW_HEIGHT))
        camera_bp.set_attribute('fov', str(VIEW_FOV))
        return camera_bp

    def set_synchronous_mode(self, synchronous_mode):
        """
        Sets synchronous mode.

        Args:
            synchronous_mode (bool): 是否开启同步模式，控制世界的更新和传感器数据获取等是否同步进行
        """

        settings = self.world.get_settings()
        settings.synchronous_mode = synchronous_mode
        self.world.apply_settings(settings)

    def setup_car(self):
        """
        Spawns actor-vehicle to be controled.

        从车辆蓝图库中随机选择一个车辆蓝图，并在世界中随机选择一个生成点来生成车辆实例，作为可控制的车辆。
        """

        car_bp = self.world.get_blueprint_library().filter('vehicle.*')[0]
        location = random.choice(self.world.get_map().get_spawn_points())
        self.car = self.world.spawn_actor(car_bp, location)

    def setup_camera(self):
        """
        Spawns actor-camera to be used to render view.
        Sets calibration for client-side boxes rendering.

        生成相机实例，将其挂载到车辆上，并设置相机的位置和姿态，同时为相机设置校准矩阵，用于后续客户端边界框的渲染。
        """

        camera_transform = carla.Transform(carla.Location(x=-5.5, z=2.8), carla.Rotation(pitch=-15))
        self.camera = self.world.spawn_actor(self.camera_blueprint(), camera_transform, attach_to=self.car)
        weak_self = weakref.ref(self)
        self.camera.listen(lambda image: weak_self().set_image(weak_self, image))

        calibration = np.identity(3)
        calibration[0, 2] = VIEW_WIDTH / 2.0
        calibration[1, 2] = VIEW_HEIGHT / 2.0
        calibration[0, 0] = calibration[1, 1] = VIEW_WIDTH / (2.0 * np.tan(VIEW_FOV * np.pi / 360.0))
        self.camera.calibration = calibration

    def control(self, car):
        """
        Applies control to main car based on pygame pressed keys.
        Will return True If ESCAPE is hit, otherwise False to end main loop.

        根据Pygame中按下的按键来设置车辆的控制参数（油门、刹车、转向、手刹等），如果按下ESC键则返回True表示退出主循环，否则返回False。

        Args:
            car (carla.Vehicle): 要控制的车辆对象

        Returns:
            bool: 是否按下ESC键的标志
        """

        keys = pygame.key.get_pressed()
        if keys[K_ESCAPE]:
            return True

        control = car.get_control()
        control.throttle = 0
        if keys[K_w]:
            control.throttle = 1
            control.reverse = False
        elif keys[K_s]:
            control.throttle = 1
            control.reverse = True
        if keys[K_a]:
            control.steer = max(-1., min(control.steer - 0.05, 0))
        elif keys[K
