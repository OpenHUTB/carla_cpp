#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Script that render multiple sensors in the same pygame window

By default, it renders four cameras, one LiDAR and one Semantic LiDAR.
It can easily be configure for any different number of sensors. 
To do that, check lines 290-308.
"""

import glob
import os
import sys

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla
import argparse
import random
import time
import numpy as np


try:
    import pygame
    from pygame.locals import K_ESCAPE
    from pygame.locals import K_q
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

class CustomTimer:
    # 自定义计时器类的构造函数
    def __init__(self):
        try:
            # 尝试使用高精度的性能计数器
            self.timer = time.perf_counter
        except AttributeError:
            # 如果不支持perf_counter，则使用标准时间函数
            self.timer = time.time

    # 获取当前时间的方法
    def time(self):
        return self.timer()

class DisplayManager:
    # 显示管理器类的构造函数
    def __init__(self, grid_size, window_size):
        pygame.init()# 初始化pygame
        pygame.font.init() # 初始化pygame字体模块
        self.display = pygame.display.set_mode(window_size, pygame.HWSURFACE | pygame.DOUBLEBUF)

        self.grid_size = grid_size # 网格大小
        self.window_size = window_size # 窗口大小
        self.sensor_list = [] # 传感器列表

    # 获取窗口大小的方法
    def get_window_size(self):
        return [int(self.window_size[0]), int(self.window_size[1])]

    # 获取显示大小的方法
    def get_display_size(self):
        return [int(self.window_size[0]/self.grid_size[1]), int(self.window_size[1]/self.grid_size[0])]

    # 获取显示偏移量的方法
    def get_display_offset(self, gridPos):
        dis_size = self.get_display_size()
        return [int(gridPos[1] * dis_size[0]), int(gridPos[0] * dis_size[1])]

    # 添加传感器的方法
    def add_sensor(self, sensor):
        self.sensor_list.append(sensor)

    # 获取传感器列表的方法
    def get_sensor_list(self):
        return self.sensor_list

    # 渲染显示的方法
    def render(self):
        if not self.render_enabled(): # 如果渲染不被允许，则直接返回
            return

        for s in self.sensor_list:  # 遍历传感器列表并渲染
            s.render()

        pygame.display.flip() # 交换前后缓冲区，更新显示

    # 销毁显示资源的方法
    def destroy(self):
        for s in self.sensor_list: # 遍历传感器列表并销毁
            s.destroy()

    # 检查渲染是否被允许的方法
    def render_enabled(self):
        return self.display != None # 如果display对象不为空，则渲染被允许

class SensorManager:#定义了一个名为  SensorManager  的类。
    def __init__(self, world, display_man, sensor_type, transform, attached, sensor_options, display_pos):#这是类的构造函数，用于初始化对象。它接受以下参数
        self.surface = None
        self.world = world#仿真世界的对象。
        self.display_man = display_man#管理显示的对象
        self.display_pos = display_pos
        self.sensor = self.init_sensor(sensor_type, transform, attached, sensor_options)#调用  init_sensor  方法初始化传感器，并将其赋值给实例变量。
        self.sensor_options = sensor_options
        self.timer = CustomTimer()

        self.time_processing = 0.0
        self.tics_processing = 0

        self.display_man.add_sensor(self)#将传感器添加到显示管理器中

    def init_sensor(self, sensor_type, transform, attached, sensor_options):#这是  init_sensor  方法的定义，它接受传感器类型、变换（位置和方向）、附着对象和传感器选项作为参数。
        if sensor_type == 'RGBCamera':
            camera_bp = self.world.get_blueprint_library().find('sensor.camera.rgb')#如果传感器类型是  RGBCamera  ，这行代码从仿真世界的蓝图库中找到RGB相机的蓝图
            disp_size = self.display_man.get_display_size()#获取显示管理器的显示大小，这通常用于设置相机捕获图像的分辨率。
            camera_bp.set_attribute('image_size_x', str(disp_size[0]))#设置相机蓝图的  image_size_x  属性，即图像的宽度
            camera_bp.set_attribute('image_size_y', str(disp_size[1]))#设置相机蓝图的  image_size_x  属性，即图像的宽度

            for key in sensor_options:
                camera_bp.set_attribute(key, sensor_options[key])#为相机蓝图设置额外的属性，这些属性由  sensor_options  字典提供。

            camera = self.world.spawn_actor(camera_bp, transform, attach_to=attached)#在仿真世界中生成（spawn）一个相机 actor，使用之前设置的蓝图、变换和附着对象。
            camera.listen(self.save_rgb_image)#设置相机监听器，当相机捕获到图像时，会调用  self.save_rgb_image 方法来保存图像。

            return camera

        elif sensor_type == 'LiDAR':
            lidar_bp = self.world.get_blueprint_library().find('sensor.lidar.ray_cast')
            lidar_bp.set_attribute('range', '100')
            lidar_bp.set_attribute('dropoff_general_rate', lidar_bp.get_attribute('dropoff_general_rate').recommended_values[0])
            lidar_bp.set_attribute('dropoff_intensity_limit', lidar_bp.get_attribute('dropoff_intensity_limit').recommended_values[0])
            lidar_bp.set_attribute('dropoff_zero_intensity', lidar_bp.get_attribute('dropoff_zero_intensity').recommended_values[0])

            for key in sensor_options:
                lidar_bp.set_attribute(key, sensor_options[key])

            lidar = self.world.spawn_actor(lidar_bp, transform, attach_to=attached)

            lidar.listen(self.save_lidar_image)

            return lidar
        
        elif sensor_type == 'SemanticLiDAR':
            lidar_bp = self.world.get_blueprint_library().find('sensor.lidar.ray_cast_semantic')# 从蓝图库中获取语义激光雷达的蓝图
            lidar_bp.set_attribute('range', '100')# 设置激光雷达的属性，例如范围设置为100米

            for key in sensor_options:# 遍历sensor_options字典，为激光雷达设置其他属性
                lidar_bp.set_attribute(key, sensor_options[key])

            lidar = self.world.spawn_actor(lidar_bp, transform, attach_to=attached)#在仿真世界中生成激光雷达对象，并设置其位置和附着对象

            lidar.listen(self.save_semanticlidar_image)# 让激光雷达监听并保存语义激光雷达图像

            return lidar
        
        elif sensor_type == "Radar":
            radar_bp = self.world.get_blueprint_library().find('sensor.other.radar')
            for key in sensor_options:
                radar_bp.set_attribute(key, sensor_options[key])

            radar = self.world.spawn_actor(radar_bp, transform, attach_to=attached)
            radar.listen(self.save_radar_image)

            return radar
        
        else:
            return None


    # 获取传感器对象的方法，直接返回实例的sensor属性（该属性应该在外部被正确赋值过，代表对应的传感器对象）
    def get_sensor(self):
        return self.sensor


    # 用于保存RGB图像数据的方法，从接收到的Carla图像数据进行处理，转换格式、调整通道顺序等，并记录处理时间相关信息
    def save_rgb_image(self, image):
        """
    功能：
        接收Carla的RGB图像数据，对其进行处理以便后续显示等操作，同时记录处理该图像所花费的时间以及处理次数，用于性能统计等用途。
    参数：
        image (carla.Image): 从Carla环境获取的原始RGB图像对象。
    """
        # 记录处理该图像开始的时间
        t_start = self.timer.time()

         # 将图像转换为原始格式（可能是去除了一些默认的编码、色彩空间转换等，获取最原始的数据表示形式）
         image.convert(carla.ColorConverter.Raw)
         # 从图像的原始字节数据创建一个numpy数组，指定数据类型为无符号8位整数（对应图像像素的字节表示）
         array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
         # 将一维的数组重新调整为三维数组，维度分别对应图像的高度、宽度以及颜色通道（这里可能包含了透明度通道，共4个通道）
         array = np.reshape(array, (image.height, image.width, 4))
         # 去除颜色通道中的透明度通道，只保留RGB三个颜色通道的数据，因为通常在显示图像时不需要透明度信息（如果图像本身是带透明度的，这里可能需要根据实际情况处理）
         array = array[:, :, :3]
         # 对图像的颜色通道顺序进行反转，将RGB顺序转换为BGR顺序，可能是因为后续使用的显示库（如Pygame）对颜色通道顺序的要求（不同的图像库或显示系统对颜色通道顺序的默认设置可能不同）
         array = array[:, :, ::-1]


      # 如果显示管理器（display_man）的渲染功能是启用状态（render_enabled方法返回True），则根据处理后的图像数据创建一个Pygame的表面对象，用于后续在窗口中显示
        if self.display_man.render_enabled():
            self.surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))

       # 记录处理该图像结束的时间
        t_end = self.timer.time()
# 将本次处理图像所花费的时间累加到总的处理时间中（self.time_processing用于累计处理时间）
        self.time_processing += (t_end-t_start)
# 将处理次数加1（self.tics_processing用于统计处理图像的次数）
        self.tics_processing += 1

# 用于保存LiDAR（激光雷达）图像数据的方法，对接收到的LiDAR数据进行解析、坐标转换、生成可视化图像等操作，并记录处理时间相关信息
    def save_lidar_image(self, image):
        """
    功能：
        接收Carla的LiDAR图像数据，对其进行处理以生成可视化的二维图像（通常是将点云数据转换为灰度图形式用于显示），同时记录处理该图像所花费的时间以及处理次数，用于性能统计等用途。
    参数：
        image (carla.Image 或对应LiDAR数据结构): 从Carla环境获取的原始LiDAR数据对象（这里假设其数据格式符合后续解析的要求）。
    """
        t_start = self.timer.time()

        # 获取显示管理器中设置的显示尺寸（宽度和高度），用于后续根据尺寸对LiDAR数据进行坐标转换等操作，使其适配显示区域
        disp_size = self.display_man.get_display_size()
        lidar_range = 2.0*float(self.sensor_options['range'])

        # 从LiDAR图像的原始字节数据创建一个numpy数组，指定数据类型为单精度浮点数（f4），用于存储点云数据（每个点可能包含坐标、强度等信息）
        points = np.frombuffer(image.raw_data, dtype=np.dtype('f4'))
    # 将一维的点云数据数组重新调整为二维数组，每一行代表一个点的数据，假设每个点的数据由4个元素组成（具体取决于LiDAR数据格式），这里按照每4个元素一组进行重塑
        points = np.reshape(points, (int(points.shape[0] / 4), 4))
    # 提取点云数据中的前两个元素（通常是x、y坐标信息）作为LiDAR数据用于后续处理，构建二维的坐标数组
        lidar_data = np.array(points[:, :2])
    # 根据显示尺寸和LiDAR探测范围对坐标数据进行缩放，使得点云数据能适配到显示尺寸范围内，min(disp_size)可能是取宽度和高度中的较小值作为统一的缩放基准
        lidar_data *= min(disp_size) / lidar_range
    # 将坐标数据进行偏移，使其中心位于显示区域的中心位置（加上显示尺寸的一半作为偏移量）
        lidar_data += (0.5 * disp_size[0], 0.5 * disp_size[1])
    # 取坐标数据的绝对值，确保坐标值都是非负的（可能是为了符合后续图像索引等操作的要求，避免出现负数索引情况）
        lidar_data = np.fabs(lidar_data)  # pylint: disable=E1111
    # 将坐标数据转换为整数类型（int32），因为图像的像素坐标是整数形式，准备用于在图像上绘制点云对应的像素点
        lidar_data = lidar_data.astype(np.int32)
    # 将二维的坐标数组重新调整为二维形式（这里只是进行形状重塑，方便后续操作，实际数据内容不变），每一行代表一个点的坐标信息
        lidar_data = np.reshape(lidar_data, (-1, 2))
    # 创建一个与显示尺寸对应的三维数组，用于表示空白的LiDAR图像（初始化为全0，数据类型为无符号8位整数，对应图像的像素表示），三个通道可用于后续可能的彩色显示等扩展（这里暂时都设置为白色表示点云位置）
        lidar_img_size = (disp_size[0], disp_size[1], 3)
        lidar_img = np.zeros((lidar_img_size), dtype=np.uint8)
        # 根据处理后的LiDAR坐标数据，将对应的图像像素位置设置为白色（RGB值为 (255, 255, 255)），实现将点云数据可视化在图像上的效果
        lidar_img[tuple(lidar_data.T)] = (255, 255, 255)

        if self.display_man.render_enabled():
            self.surface = pygame.surfarray.make_surface(lidar_img)

        t_end = self.timer.time()
        self.time_processing += (t_end-t_start)
        self.tics_processing += 1

    # 用于保存语义LiDAR（SemanticLiDAR，带有语义信息的激光雷达）图像数据的方法，与save_lidar_image方法类似，但处理的数据格式和具体含义可能有所不同，同样记录处理时间相关信息
    def save_semanticlidar_image(self, image):
        """
    功能：
        接收Carla的语义LiDAR图像数据，对其进行处理以生成可视化的二维图像（将带有语义信息的点云数据转换为可视化形式用于显示），同时记录处理该图像所花费的时间以及处理次数，用于性能统计等用途。
    参数：
        image (carla.Image 或对应语义LiDAR数据结构): 从Carla环境获取的原始语义LiDAR数据对象（这里假设其数据格式符合后续解析的要求）。
    """
        t_start = self.timer.time()

        disp_size = self.display_man.get_display_size()
        lidar_range = 2.0*float(self.sensor_options['range'])

        points = np.frombuffer(image.raw_data, dtype=np.dtype('f4'))
        points = np.reshape(points, (int(points.shape[0] / 6), 6))
        lidar_data = np.array(points[:, :2])
        lidar_data *= min(disp_size) / lidar_range
        lidar_data += (0.5 * disp_size[0], 0.5 * disp_size[1])
        lidar_data = np.fabs(lidar_data)  # pylint: disable=E1111
        lidar_data = lidar_data.astype(np.int32)
        lidar_data = np.reshape(lidar_data, (-1, 2))
        lidar_img_size = (disp_size[0], disp_size[1], 3)
        lidar_img = np.zeros((lidar_img_size), dtype=np.uint8)

        lidar_img[tuple(lidar_data.T)] = (255, 255, 255)

        if self.display_man.render_enabled():
            self.surface = pygame.surfarray.make_surface(lidar_img)

        t_end = self.timer.time()
        self.time_processing += (t_end-t_start)
        self.tics_processing += 1

    def save_radar_image(self, radar_data):
        t_start = self.timer.time()
        points = np.frombuffer(radar_data.raw_data, dtype=np.dtype('f4'))
        points = np.reshape(points, (len(radar_data), 4))

        t_end = self.timer.time()
        self.time_processing += (t_end-t_start)
        self.tics_processing += 1

    def render(self):
        if self.surface is not None:
            offset = self.display_man.get_display_offset(self.display_pos)
            self.display_man.display.blit(self.surface, offset)

    def destroy(self):
        self.sensor.destroy()

def run_simulation(args, client):
    """This function performed one test run using the args parameters
    and connecting to the carla client passed.
    """

    display_manager = None
    vehicle = None
    vehicle_list = []
    timer = CustomTimer()

    try:

        # Getting the world and
        world = client.get_world()
        original_settings = world.get_settings()

        if args.sync:
            traffic_manager = client.get_trafficmanager(8000)
            settings = world.get_settings()
            traffic_manager.set_synchronous_mode(True)
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 0.05
            world.apply_settings(settings)


        # Instanciating the vehicle to which we attached the sensors
        bp = world.get_blueprint_library().filter('charger_2020')[0]
        vehicle = world.spawn_actor(bp, random.choice(world.get_map().get_spawn_points()))
        vehicle_list.append(vehicle)
        vehicle.set_autopilot(True)

        # Display Manager organize all the sensors an its display in a window
        # If can easily configure the grid and the total window size
        display_manager = DisplayManager(grid_size=[2, 3], window_size=[args.width, args.height])

        # Then, SensorManager can be used to spawn RGBCamera, LiDARs and SemanticLiDARs as needed
        # and assign each of them to a grid position, 
        SensorManager(world, display_manager, 'RGBCamera', carla.Transform(carla.Location(x=0, z=2.4), carla.Rotation(yaw=-90)), 
                      vehicle, {}, display_pos=[0, 0])
        SensorManager(world, display_manager, 'RGBCamera', carla.Transform(carla.Location(x=0, z=2.4), carla.Rotation(yaw=+00)), 
                      vehicle, {}, display_pos=[0, 1])
        SensorManager(world, display_manager, 'RGBCamera', carla.Transform(carla.Location(x=0, z=2.4), carla.Rotation(yaw=+90)), 
                      vehicle, {}, display_pos=[0, 2])
        SensorManager(world, display_manager, 'RGBCamera', carla.Transform(carla.Location(x=0, z=2.4), carla.Rotation(yaw=180)), 
                      vehicle, {}, display_pos=[1, 1])

        SensorManager(world, display_manager, 'LiDAR', carla.Transform(carla.Location(x=0, z=2.4)), 
                      vehicle, {'channels' : '64', 'range' : '100',  'points_per_second': '250000', 'rotation_frequency': '20'}, display_pos=[1, 0])
        SensorManager(world, display_manager, 'SemanticLiDAR', carla.Transform(carla.Location(x=0, z=2.4)), 
                      vehicle, {'channels' : '64', 'range' : '100', 'points_per_second': '100000', 'rotation_frequency': '20'}, display_pos=[1, 2])


        #Simulation loop
        call_exit = False
        time_init_sim = timer.time()
        while True:
            # Carla Tick
            if args.sync:
                world.tick()
            else:
                world.wait_for_tick()

            # Render received data
            display_manager.render()

            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    call_exit = True
                elif event.type == pygame.KEYDOWN:
                    if event.key == K_ESCAPE or event.key == K_q:
                        call_exit = True
                        break

            if call_exit:
                break

    finally:
        if display_manager:
            display_manager.destroy()

        client.apply_batch([carla.command.DestroyActor(x) for x in vehicle_list])

        world.apply_settings(original_settings)



def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Sensor tutorial')
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '--sync',
        action='store_true',
        help='Synchronous mode execution')
    argparser.add_argument(
        '--async',
        dest='sync',
        action='store_false',
        help='Asynchronous mode execution')
    argparser.set_defaults(sync=True)
    argparser.add_argument(
        '--res',
        metavar='WIDTHxHEIGHT',
        default='1280x720',
        help='window resolution (default: 1280x720)')

    args = argparser.parse_args()

    args.width, args.height = [int(x) for x in args.res.split('x')]

    try:
        client = carla.Client(args.host, args.port)
        client.set_timeout(5.0)

        run_simulation(args, client)

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')


if __name__ == '__main__':

    main()
