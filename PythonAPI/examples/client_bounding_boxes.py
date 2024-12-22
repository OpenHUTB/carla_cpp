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

# 尝试将Carla模块的路径添加到系统路径中，以便后续能够正确导入Carla相关的库
# 根据Python版本（major.minor）以及操作系统类型（win-amd64或linux-x86_64）来查找对应的Carla模块egg文件路径
# 如果查找成功则添加到系统路径，若查找失败（IndexError）则跳过（即假设已经在系统路径中或者不需要添加）
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

# 尝试导入pygame库，用于创建游戏界面、处理用户输入等相关操作
# 如果导入失败则抛出运行时错误，提示确保安装了pygame包
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

# 尝试导入numpy库，用于进行数值计算、数组操作等，在处理坐标变换等操作中会用到
# 如果导入失败则抛出运行时错误，提示确保安装了numpy包
try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')

# 定义相机视图的宽度，这里将1920像素的宽度减半，可根据实际需求调整
VIEW_WIDTH = 1920//2
# 定义相机视图的高度，这里将1080像素的高度减半，可根据实际需求调整
VIEW_HEIGHT = 1080//2
# 定义相机的视野角度，单位为度，用于确定相机可视范围的宽窄程度
VIEW_FOV = 90

# 定义用于绘制边界框的颜色，以RGB格式表示，这里是橙色（可根据喜好修改颜色）
BB_COLOR = (248, 64, 24)

# ==============================================================================
# -- ClientSideBoundingBoxes ---------------------------------------------------
# ==============================================================================


class ClientSideBoundingBoxes(object):
    """
    This is a module responsible for creating 3D bounding boxes and drawing them
    client-side on pygame surface.
    """

    # 为每辆车创建3D边界框，使用列表推导式遍历车辆列表调用get_bounding_box方法创建边界框
    @staticmethod
    def get_bounding_boxes(vehicles, camera):
        """
        Creates 3D bounding boxes based on carla vehicle list and camera.
        """

        # 过滤掉位于相机后方的物体（即边界框在相机坐标系中z坐标小于等于0的
        bounding_boxes = [ClientSideBoundingBoxes.get_bounding_box(vehicle, camera) for vehicle in vehicles]
        # filter objects behind camera
        bounding_boxes = [bb for bb in bounding_boxes if all(bb[:, 2] > 0)]
        return bounding_boxes

    @staticmethod
    def draw_bounding_boxes(display, bounding_boxes):
        """
        Draws bounding boxes on pygame display.
        """

         # 创建一个与显示表面大小相同的透明表面，用于绘制边界框（避免覆盖原图像其他内容）
        bb_surface = pygame.Surface((VIEW_WIDTH, VIEW_HEIGHT))
        bb_surface.set_colorkey((0, 0, 0))
        for bbox in bounding_boxes:
            # 将边界框的每个顶点坐标转换为整数坐标，方便在Pygame中绘制
            points = [(int(bbox[i, 0]), int(bbox[i, 1])) for i in range(8)]
             # 绘制边界框的底边线条
          
            pygame.draw.line(bb_surface, BB_COLOR, points[0], points[1])
            pygame.draw.line(bb_surface, BB_COLOR, points[0], points[1])
            pygame.draw.line(bb_surface, BB_COLOR, points[1], points[2])
            pygame.draw.line(bb_surface, BB_COLOR, points[2], points[3])
            pygame.draw.line(bb_surface, BB_COLOR, points[3], points[0])
            # 绘制边界框的顶边线条
            pygame.draw.line(bb_surface, BB_COLOR, points[4], points[5])
            pygame.draw.line(bb_surface, BB_COLOR, points[5], points[6])
            pygame.draw.line(bb_surface, BB_COLOR, points[6], points[7])
            pygame.draw.line(bb_surface, BB_COLOR, points[7], points[4])

            # 绘制连接底边和顶边的线条
            pygame.draw.line(bb_surface, BB_COLOR, points[0], points[4])
            pygame.draw.line(bb_surface, BB_COLOR, points[1], points[5])
            pygame.draw.line(bb_surface, BB_COLOR, points[2], points[6])
            pygame.draw.line(bb_surface, BB_COLOR, points[3], points[7])
              # 将绘制好边界框的表面复制到主显示表面上，使其显示出来
        display.blit(bb_surface, (0, 0))

    @staticmethod
    def get_bounding_box(vehicle, camera):
        """
        Returns 3D bounding box for a vehicle based on camera view.
        """

         # 获取车辆的边界框顶点坐标（在车辆本地坐标系下
        bb_cords = ClientSideBoundingBoxes._create_bb_points(vehicle)
        # 将车辆本地坐标系下的边界框坐标转换到相机坐标系下
        cords_x_y_z = ClientSideBoundingBoxes._vehicle_to_sensor(bb_cords, vehicle, camera)[:3, :]
        # 调整坐标顺序（将y、-z、x顺序排列，可能是为了符合后续计算或显示的要求）
        cords_y_minus_z_x = np.concatenate([cords_x_y_z[1, :], -cords_x_y_z[2, :], cords_x_y_z[0, :]])
         # 利用相机的校准矩阵对坐标进行变换，得到在相机图像平面上的坐标（齐次坐标形式）
        bbox = np.transpose(np.dot(camera.calibration, cords_y_minus_z_x))
        # 将齐次坐标转换为归一化坐标（除以深度坐标，便于后续绘制等操作），得到最终的边界框坐标
        camera_bbox = np.concatenate([bbox[:, 0] / bbox[:, 2], bbox[:, 1] / bbox[:, 2], bbox[:, 2]], axis=1)
        return camera_bbox

    @staticmethod
    def _create_bb_points(vehicle):
        """
        Returns 3D bounding box for a vehicle.
        """

        cords = np.zeros((8, 4))
         # 获取车辆边界框的范围（长、宽、高的一半）
        extent = vehicle.bounding_box.extent
        # 按照特定顺序设置边界框的8个顶点坐标（以下是构建一个长方体边界框顶点坐标的常见方式）
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
        """

         # 先将车辆本地坐标系下的坐标转换到世界坐标系下
        world_cord = ClientSideBoundingBoxes._vehicle_to_world(cords, vehicle)
         # 再将世界坐标系下的坐标转换到传感器（相机）坐标系下
        sensor_cord = ClientSideBoundingBoxes._world_to_sensor(world_cord, sensor)
        return sensor_cord

    @staticmethod
    def _vehicle_to_world(cords, vehicle):
        """
        Transforms coordinates of a vehicle bounding box to world.
        """

        # 获取车辆边界框的初始变换（位置信息）
        bb_transform = carla.Transform(vehicle.bounding_box.location)
        # 根据车辆边界框的初始变换创建对应的变换矩阵
        bb_vehicle_matrix = ClientSideBoundingBoxes.get_matrix(bb_transform)
        # 获取车辆在世界坐标系中的变换矩阵（包含位置、旋转等信息)
        vehicle_world_matrix = ClientSideBoundingBoxes.get_matrix(vehicle.get_transform())
         # 通过矩阵乘法将车辆边界框从本地坐标系转换到世界坐标系，先将车辆边界框的变换矩阵与车辆在世界坐标系的变换矩阵相乘
        bb_world_matrix = np.dot(vehicle_world_matrix, bb_vehicle_matrix)
        # 再用得到的世界坐标系下的变换矩阵乘以边界框坐标，得到世界坐标系下的坐标
        world_cords = np.dot(bb_world_matrix, np.transpose(cords))
        return world_cords

    @staticmethod
    def _world_to_sensor(cords, sensor):
        """
        Transforms world coordinates to sensor.
        """

        # 获取传感器（相机）在世界坐标系中的变换矩阵
        sensor_world_matrix = ClientSideBoundingBoxes.get_matrix(sensor.get_transform())
        # 求传感器在世界坐标系中的变换矩阵的逆矩阵，用于将世界坐标转换到传感器坐标
        world_sensor_matrix = np.linalg.inv(sensor_world_matrix)
        # 通过矩阵乘法将世界坐标系下的坐标转换到传感器坐标系下
        sensor_cords = np.dot(world_sensor_matrix, cords)
        return sensor_cords

    @staticmethod
    def get_matrix(transform):
        """
        Creates matrix from carla transform.
        """

        rotation = transform.rotation
        location = transform.location
        # 计算偏航角的余弦值
        c_y = np.cos(np.radians(rotation.yaw))
         # 计算偏航角的正弦值
        s_y = np.sin(np.radians(rotation.yaw))
         # 计算翻滚角的余弦值
        c_r = np.cos(np.radians(rotation.roll))
           # 计算翻滚角的正弦值
        s_r = np.sin(np.radians(rotation.roll))
         # 计算俯仰角的余弦值
        c_p = np.cos(np.radians(rotation.pitch))
         # 计算俯仰角的正弦值
        s_p = np.sin(np.radians(rotation.pitch))
        matrix = np.matrix(np.identity(4))
        # 设置变换矩阵中的位置信息（平移部分）
        matrix[0, 3] = location.x
        matrix[1, 3] = location.y
        matrix[2, 3] = location.z
          # 设置变换矩阵中的旋转信息（按照特定的旋转矩阵计算公式填充各元素
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
        # Carla客户端对象，用于与Carla服务器进行通信
        self.client = None
        # Carla世界对象，代表整个模拟世界，包含地图、车辆、传感器等所有元素
        self.world = None
        # Carla相机对象，用于获取场景图像信息以及辅助绘制边界框等
        self.camera = None
        # Carla车辆对象，代表要控制的车辆
        self.car = None

        # Pygame的显示表面对象，用于显示整个游戏画面（包含图像和绘制的边界框等）
        self.display = None
         # 存储从相机获取的图像数据（以特定格式存储，后续会进行处理用于显示）
        self.image = None
        # 用于图像捕获的标志位，用于同步图像获取和处理的流程，初始设为True
        self.capture = True

    def camera_blueprint(self):
        """
        Returns camera blueprint.
        """

        # 从世界的蓝图库中查找名为'sensor.camera.rgb'的相机蓝图
        camera_bp = self.world.get_blueprint_library().find('sensor.camera.rgb')
        # 设置相机图像的宽度属性，使用之前定义的VIEW_WIDTH值
        camera_bp.set_attribute('image_size_x', str(VIEW_WIDTH))
         # 设置相机图像的高度属性，使用之前定义的VIEW_HEIGHT值
        camera_bp.set_attribute('image_size_y', str(VIEW_HEIGHT))
        # 设置相机的视野角度属性，使用之前定义的VIEW_FOV值
        camera_bp.set_attribute('fov', str(VIEW_FOV))
        return camera_bp

    # 设置同步模式的方法，用于控制Carla世界的同步运行特性
    def set_synchronous_mode(self, synchronous_mode):
        """
        Sets synchronous mode.
        """

        # 获取当前世界的设置对象，该对象包含了世界运行相关的各种参数设置
        settings = self.world.get_settings()
        # 将同步模式参数设置为传入的synchronous_mode值，决定世界是否按同步方式运行
        settings.synchronous_mode = synchronous_mode
        # 将更新后的设置应用到Carla世界中，使其生效
        self.world.apply_settings(settings)

    # 用于生成要控制的车辆（actor-vehicle）的方法
    def setup_car(self):
        """
        Spawns actor-vehicle to be controled.
        """

        # 从世界的蓝图库中筛选出所有名称以'vehicle.'开头的蓝图（即车辆相关蓝图），并取第一个作为要创建车辆的蓝图
        car_bp = self.world.get_blueprint_library().filter('vehicle.*')[0]
        # 从世界地图的所有出生点中随机选择一个作为车辆的初始位置
        location = random.choice(self.world.get_map().get_spawn_points())
        # 在选定的位置，使用选定的车辆蓝图在世界中生成车辆实例，并将其赋值给self.car
        self.car = self.world.spawn_actor(car_bp, location)

    # 用于设置相机（actor-camera）相关参数以及绑定到车辆等操作的方法
    def setup_camera(self):
        """
        Spawns actor-camera to be used to render view.
        Sets calibration for client-side boxes rendering.
        """

        # 创建一个相机的变换对象，指定相机在世界坐标系中的位置（x坐标为-5.5，z坐标为2.8）和姿态（俯仰角为-15度），这里没有设置y坐标和偏航、翻滚角，可能采用默认值
        camera_transform = carla.Transform(carla.Location(x=-5.5, z=2.8), carla.Rotation(pitch=-15))
        # 使用之前定义的camera_blueprint方法获取相机蓝图，并基于该蓝图、相机变换信息以及绑定到车辆（self.car）上的设定，在世界中生成相机实例
        self.camera = self.world.spawn_actor(self.camera_blueprint(), camera_transform, attach_to=self.car)
        # 创建一个对当前实例（self）的弱引用，避免循环引用导致内存泄漏等问题，便于在回调函数中访问当前实例的属性和方法
        weak_self = weakref.ref(self)
        # 为相机注册一个监听函数（回调函数），当相机获取到新的图像数据时会触发该函数，将图像数据传递给set_image方法进行处理
        self.camera.listen(lambda image: weak_self().set_image(weak_self, image))

        # 创建一个3x3的单位矩阵，用于初始化相机的校准矩阵，后续会修改其中的元素来完成实际的校准设置
        calibration = np.identity(3)
        # 设置校准矩阵中对应图像宽度方向上的中心坐标，使得后续计算的坐标能正确对应图像中的位置（将图像宽度的一半赋值给对应元素）
        calibration[0, 2] = VIEW_WIDTH / 2.0
        # 根据相机的视野角度（VIEW_FOV）计算图像宽度和高度方向上的缩放因子，以确保边界框绘制等操作在图像平面上的坐标正确对应实际像素位置
        calibration[1, 2] = VIEW_HEIGHT / 2.0
        # 将校准矩阵赋值给相机实例的calibration属性，以便后续使用该校准信息进行相关坐标变换等操作
        calibration[0, 0] = calibration[1, 1] = VIEW_WIDTH / (2.0 * np.tan(VIEW_FOV * np.pi / 360.0))
        self.camera.calibration = calibration

    # 根据Pygame中按下的按键来控制车辆的方法
    def control(self, car):
        """
        Applies control to main car based on pygame pressed keys.
        Will return True If ESCAPE is hit, otherwise False to end main loop.
        """

        # 获取Pygame中当前被按下的所有按键状态
        keys = pygame.key.get_pressed()
        # 判断是否按下了ESCAPE键，如果按下则返回True，表示要结束程序主循环
        if keys[K_ESCAPE]:
            return True

        # 获取车辆当前的控制对象，用于设置车辆的各项控制参数（如油门、刹车、转向等）
        control = car.get_control()
        # 先将油门参数初始化为0，后续根据按键情况再进行调整
        control.throttle = 0
        # 如果按下了W键，将油门设置为1（表示全油门加速），并设置车辆为正向行驶（reverse为False）
        if keys[K_w]:
            control.throttle = 1
            control.reverse = False
        # 如果按下了S键，将油门设置为1（表示全油门，但这里可能是刹车或倒车情况，取决于车辆的具体实现），并设置车辆为反向行驶（reverse为True）
        elif keys[K_s]:
            control.throttle = 1
            control.reverse = True
        # 如果按下了A键，逐渐减小车辆的转向角度，限制转向角度在[-1, 0]范围内，每次减少0.05（实现向左转向的效果，转向角度的调整幅度可根据实际情况调整）
        if keys[K_a]:
            control.steer = max(-1., min(control.steer - 0.05, 0))
        # 如果按下了D键，逐渐增大车辆的转向角度，限制转向角度在[0, 1]范围内，每次增加0.05（实现向右转向的效果，转向角度的调整幅度可根据实际情况调整）
        elif keys[K_d]:
            control.steer = min(1., max(control.steer + 0.05, 0))
        # 如果没有按下A或D键，将转向角度设置为0，表示车辆直线行驶
        else:
            control.steer = 0
        # 根据是否按下空格键来设置手刹状态，按下则手刹拉起（hand_brake为True）
        control.hand_brake = keys[K_SPACE]

        # 将设置好的控制参数应用到车辆上，使车辆按照设定的控制指令进行动作
        car.apply_control(control)
        # 返回False，表示未按下ESCAPE键，程序主循环继续执行
        return False

    # 静态方法，用于设置从相机传感器获取的图像数据，通过self.capture标志位来同步图像获取和处理流程
    @staticmethod
    def set_image(weak_self, img):
        """
        Sets image coming from camera sensor.
        The self.capture flag is a mean of synchronization - once the flag is
        set, next coming image will be stored.
        """

        # 通过弱引用获取实际的实例对象
        self = weak_self()
        # 判断self.capture标志位是否为True，如果是则表示可以存储当前获取到的图像数据
        if self.capture:
            # 将传入的图像数据赋值给实例的image属性，用于后续的显示、处理等操作
            self.image = img
            # 将self.capture标志位设置为False，表示已经存储了当前图像，下次获取图像时需要等待该标志位再次被设置为True
            self.capture = False

    # 用于将从相机获取的图像数据进行处理并显示到Pygame显示表面上的方法
    def render(self, display):
        """
        Transforms image from camera sensor and blits it to main pygame display.
        """

        # 判断是否已经获取到了相机图像数据（self.image不为None），如果有则进行后续处理和显示操作
        if self.image is not None:
            # 从图像数据的原始字节数据中创建一个numpy数组，指定数据类型为无符号8位整数（对应图像像素的字节表示）
            array = np.frombuffer(self.image.raw_data, dtype=np.dtype("uint8"))
            # 将一维的数组重新调整为三维数组，维度分别对应图像的高度、宽度以及颜色通道（这里可能包含了透明度通道，共4个通道
            array = np.reshape(array, (self.image.height, self.image.width, 4))
            # 去除颜色通道中的透明度通道，只保留RGB三个颜色通道的数据，因为通常在显示图像时不需要透明度信息（如果图像本身是带透明度的，这里可能需要根据实际情况处理）
            array = array[:, :, :3]
            # 对图像的颜色通道顺序进行反转，将RGB顺序转换为BGR顺序，可能是因为Pygame显示图像时对颜色通道顺序的要求（不同的图像库或显示系统对颜色通道顺序的默认设置可能不同
            array = array[:, :, ::-1]
             # 根据处理后的数组数据创建一个Pygame的表面对象，用于后续将其绘制到显示表面上
            surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))
            # 将创建好的图像表面绘制到传入的显示表面（display）的左上角（坐标(0, 0)）位置，实现图像的显示
            display.blit(surface, (0, 0))

    # 对图像的颜色通道顺序进行反转，将RGB顺序转换为BGR顺序，可能是因为Pygame显示图像时对颜色通道顺序的要求（不同的图像库或显示系统对颜色通道顺序的默认设置可能不同
    def game_loop(self):
        """
        Main program loop.
        """

        try:
             # 初始化Pygame库，用于后续创建游戏窗口、处理用户输入、显示图像等操作
            pygame.init()

            # 创建一个Carla客户端对象，连接到本地（127.0.0.1）的Carla服务器，端口号为2000，并设置连接超时时间为2秒
            self.client = carla.Client('127.0.0.1', 2000)
            self.client.set_timeout(2.0)
            # 通过客户端获取Carla世界对象，该对象包含了整个模拟世界的各种元素和状态信息
            self.world = self.client.get_world()

             # 调用setup_car方法生成要控制的车辆实例
            self.setup_car()
            # 调用setup_camera方法生成相机实例并进行相关设置
            self.setup_camera()

             # 创建一个Pygame的显示表面，设置其大小为之前定义的VIEW_WIDTH和VIEW_HEIGHT，使用硬件加速（HWSURFACE）和双缓冲（DOUBLEBUF）模式，以提高显示性能和图像显示的流畅度
            self.display = pygame.display.set_mode((VIEW_WIDTH, VIEW_HEIGHT), pygame.HWSURFACE | pygame.DOUBLEBUF)
            # 创建一个Pygame的时钟对象，用于控制游戏循环的帧率等时间相关操作
            pygame_clock = pygame.time.Clock()

            # 调用set_synchronous_mode方法开启Carla世界的同步模式，使得后续的世界更新等操作按同步方式进行
            self.set_synchronous_mode(True)
            # 获取世界中所有的车辆演员（actor）对象，通过过滤名称以'vehicle.'开头的方式筛选出所有车辆，用于后续绘制车辆的边界框等操作
            vehicles = self.world.get_actors().filter('vehicle.*')

            # 进入主循环，只要不按下ESCAPE键就会一直循环执行以下操作
            while True:
                # 执行Carla世界的一次更新步骤，在同步模式下，会等待所有注册的传感器等完成数据更新后再继续执行后续代码
                self.world.tick()

                 # 将图像捕获标志位self.capture设置为True，表示可以接收下一张相机图像进行处理和显示了
                self.capture = True
                # 使用时钟对象控制循环的执行频率，这里设置每秒最多执行20次循环（即帧率上限为20帧/秒），通过忙等待循环来实现帧率控制
                pygame_clock.tick_busy_loop(20)

                # 调用render方法将从相机获取的图像显示到Pygame显示表面上
                self.render(self.display)
                # 调用ClientSideBoundingBoxes类的get_bounding_boxes方法获取所有车辆基于相机视角的边界框信息
                bounding_boxes = ClientSideBoundingBoxes.get_bounding_boxes(vehicles, self.camera)
                ClientSideBoundingBoxes.draw_bounding_boxes(self.display, bounding_boxes)

                 # 更新Pygame的显示表面，将所有绘制的内容（图像和边界框等）显示出来
                pygame.display.flip()

                # 处理Pygame中的事件队列，例如按键按下、鼠标点击等事件，确保事件能被及时响应（虽然这里没有对其他事件做具体处理，但需要调用该方法来保持Pygame事件系统的正常运行）
                pygame.event.pump()
                # 调用control方法根据用户按键情况控制车辆，并判断是否按下了ESCAPE键，如果按下则退出主循环
                if self.control(self.car):
                    return

        finally:
             # 无论主循环是正常结束还是因为异常等原因退出，都会执行以下清理操作
             # 调用set_synchronous_mode方法关闭Carla世界的同步模式
            self.set_synchronous_mode(False)
            # 销毁相机实例，释放相关资源
            self.camera.destroy()
            # 销毁车辆实例，释放相关资源
            self.car.destroy()
            # 退出Pygame库，关闭显示窗口等相关资源
            pygame.quit()


# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================


# 主函数，用于初始化并启动客户端侧边界框演示程序
def main():
    """
    Initializes the client-side bounding box demo.
    """

     # 创建一个BasicSynchronousClient类的实例对象，该对象包含了整个程序的主要逻辑和功能实现相关的方法和属性
    try:
        client = BasicSynchronousClient()
        # 调用实例的game_loop方法，进入主程序循环，开始执行程序的核心逻辑（如连接Carla、生成车辆相机、处理图像、控制车辆等操作
        client.game_loop()
    finally:
        # 无论主程序循环是正常结束还是出现异常退出，都会执行以下代码，打印'EXIT'表示程序执行完毕退出
        print('EXIT')


if __name__ == '__main__':
    main()
