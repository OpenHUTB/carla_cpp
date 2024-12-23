#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Raycast sensor profiler

This script can be used to test, visualize and profile the raycast sensors,
LiDARs and Radar (radar visualization not available, sorry).

By default, the script render one RGB Camera and three LiDARS whose output
can be visualized in a window just running:
  python raycast_sensor_testing.py

For profiling, you can choose the number of LiDARs and Radars and then use
the profiling option to run a series of simulations for points from 100k
to 1.5M per second. In this mode we do not render anything but processing
of the data is done.
For example for profiling one lidar:
  python raycast_sensor_testing.py -ln 1 --profiling
For example for profiling one semantic lidar:
  python raycast_sensor_testing.py -sln 1 --profiling
And for profiling one radar:
  python raycast_sensor_testing.py -rn 1 --profiling

"""

import glob
import os
import sys
# 尝试将 CARLA 库的路径添加到系统路径
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

# 如果无法导入 pygame 库，则抛出错误
try:
    import pygame
    from pygame.locals import K_ESCAPE
    from pygame.locals import K_q
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')
# 定义一个自定义定时器类，用于测量处理时间
class CustomTimer:
    def __init__(self):
        try:
            self.timer = time.perf_counter
        except AttributeError:
            self.timer = time.time

    def time(self):
        return self.timer()
# 显示管理器类，用于管理渲染窗口和传感器的显示
class DisplayManager:
    def __init__(self, grid_size, window_size, show_window=True):
        if show_window:
            pygame.init()
            pygame.font.init()
            self.display = pygame.display.set_mode(window_size, pygame.HWSURFACE | pygame.DOUBLEBUF)
        else:
            self.display = None

        self.grid_size = grid_size
        self.window_size = window_size
        self.sensor_list = []

    def get_window_size(self): """获取窗口的尺寸大小"""
        return [int(self.window_size[0]), int(self.window_size[1])]

    def get_display_size(self): """根据窗口大小和网格大小，获取每个显示区域（按网格划分）的尺寸"""
        return [int(self.window_size[0]/self.grid_size[0]), int(self.window_size[1]/self.grid_size[1])]

    def get_display_offset(self, gridPos):"""根据给定的网格位置，计算显示偏移量"""
        dis_size = self.get_display_size()
        return [int(gridPos[0] * dis_size[0]), int(gridPos[1] * dis_size[1])]

    def add_sensor(self, sensor):"""向传感器列表中添加传感器对象"""
        self.sensor_list.append(sensor)

    def get_sensor_list(self):"""获取传感器列表"""
        return self.sensor_list

    def render(self): """渲染传感器数据到窗口上，如果显示窗口未启用则直接返回不进行渲染操作"""  
        if not self.render_enabled():
            return

        for s in self.sensor_list:
            s.render()

        pygame.display.flip()

    def destroy(self):
        for s in self.sensor_list:
            s.destroy()

    def render_enabled(self):
        return self.display != None
# 传感器管理器类，用于管理各类传感器的创建、数据采集和渲染
class SensorManager:
    def __init__(self, world, display_man, sensor_type, transform, attached, sensor_options, display_pos):
        self.surface = None
        self.world = world
        self.display_man = display_man
        self.display_pos = display_pos
        self.sensor = self.init_sensor(sensor_type, transform, attached, sensor_options)
        self.sensor_options = sensor_options
        self.timer = CustomTimer()

        self.time_processing = 0.0
        self.tics_processing = 0

        self.display_man.add_sensor(self)

    def init_sensor(self, sensor_type, transform, attached, sensor_options):
        if sensor_type == 'RGBCamera':# 如果传感器类型是 'RGBCamera'（彩色摄像头）
            camera_bp = self.world.get_blueprint_library().find('sensor.camera.rgb')
            disp_size = self.display_man.get_display_size()# 获取显示管理器的显示尺寸
            camera_bp.set_attribute('image_size_x', str(disp_size[0]))# 设置摄像头的图像尺寸
            camera_bp.set_attribute('image_size_y', str(disp_size[1]))

            for key in sensor_options:# 遍历传感器选项字典，设置摄像头的属性
                camera_bp.set_attribute(key, sensor_options[key])

            camera = self.world.spawn_actor(camera_bp, transform, attach_to=attached)# 在世界中生成摄像头 actor，并附加到指定的 vehicle 或其他 actor
            camera.listen(self.save_rgb_image)

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
            lidar_bp = self.world.get_blueprint_library().find('sensor.lidar.ray_cast_semantic')
            lidar_bp.set_attribute('range', '100')

            for key in sensor_options:
                lidar_bp.set_attribute(key, sensor_options[key])

            lidar = self.world.spawn_actor(lidar_bp, transform, attach_to=attached)

            lidar.listen(self.save_semanticlidar_image)

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
    #定义一个函数，返回self.sensor
    def get_sensor(self):
        return self.sensor
    #定义一个函数，用于保存 RGB 图像
    def save_rgb_image(self, image):
        记录处理开始的时间
        t_start = self.timer.time()
        #将图像转换为原始格式
        image.convert(carla.ColorConverter.Raw)
        #从图像的原始数据中读取数据，数据类型为无符号 8 位整数
        array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
        #将数组重塑为一个三维数组，形状为(height, width, 4)，其中 4 可能代表 RGBA 通道
        array = np.reshape(array, (image.height, image.width, 4))
        #去除 alpha 通道，只保留 RGB 通道
        array = array[:, :, :3]
        #反转颜色通道的顺序，可能是从 BGR 转换为 RGB
        array = array[:, :, ::-1]
        #如果显示管理器允许渲染
        if self.display_man.render_enabled():
            #将numpy数组转换为pygame表面，可能需要交换轴来正确显示图像
            self.surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))
        #记录处理结束的时间，并更新处理时间和处理次数
        t_end = self.timer.time()
        self.time_processing += (t_end-t_start)
        self.tics_processing += 1
    #定义一个函数，用于保存激光雷达图像
    def save_lidar_image(self, image):
        #记录处理开始时的时间
        t_start = self.timer.time()
        #获取显示尺寸
        disp_size = self.display_man.get_display_size()
        #根据传感器选项中的range参数计算激光雷达的范围
        lidar_range = 2.0*float(self.sensor_options['range'])
        #从图像的原始数据中读取数据
        points = np.frombuffer(image.raw_data, dtype=np.dtype('f4'))
        #对数据进行重塑操作，将数据分成 4 列的矩阵
        points = np.reshape(points, (int(points.shape[0] / 4), 4))
        #提取前两列数据作为激光雷达数据
        lidar_data = np.array(points[:, :2])
        #根据范围和最小显示尺寸对数据进行缩放
        lidar_data *= min(disp_size) / lidar_range
        #对数据进行偏移操作
        lidar_data += (0.5 * disp_size[0], 0.5 * disp_size[1])
        #取数据的绝对值
        lidar_data = np.fabs(lidar_data)  # pylint: disable=E1111
        #将数据转换为 32 位整数类型
        lidar_data = lidar_data.astype(np.int32)
        #再次对数据进行重塑操作
        lidar_data = np.reshape(lidar_data, (-1, 2))
        #定义图像尺寸
        lidar_img_size = (disp_size[0], disp_size[1], 3)
        #创建一个全零的图像数组，类型为无符号 8 位整数
        lidar_img = np.zeros((lidar_img_size), dtype=np.uint8)
        #根据激光雷达数据在图像中绘制白色像素
        lidar_img[tuple(lidar_data.T)] = (255, 255, 255)
        #如果显示管理器允许渲染
        if self.display_man.render_enabled():
            #使用pygame库将激光雷达图像转换为可渲染的表面
            self.surface = pygame.surfarray.make_surface(lidar_img)
        #记录处理结束时的时间
        t_end = self.timer.time()
        #累加处理时间
        self.time_processing += (t_end-t_start)
        #累加处理次数
        self.tics_processing += 1
    #定义一个函数，用于保存语义激光雷达图像
    def save_semanticlidar_image(self, image):
        #记录处理开始时的时间
        t_start = self.timer.time()
        #获取显示尺寸
        disp_size = self.display_man.get_display_size()
        #根据传感器选项中的range参数计算激光雷达的范围
        lidar_range = 2.0*float(self.sensor_options['range'])
        #从图像的原始数据中读取数据，并转换为浮点数类型
        points = np.frombuffer(image.raw_data, dtype=np.dtype('f4'))
        #对数据进行重塑操作，将数据分成 6 列的矩阵
        points = np.reshape(points, (int(points.shape[0] / 6), 6))
        #提取前两列数据作为激光雷达数据
        lidar_data = np.array(points[:, :2])
        #根据范围和最小显示尺寸对数据进行缩放
        lidar_data *= min(disp_size) / lidar_range
        #对数据进行偏移操作
        lidar_data += (0.5 * disp_size[0], 0.5 * disp_size[1])
        #取数据的绝对值
        lidar_data = np.fabs(lidar_data)  # pylint: disable=E1111
        #将数据转换为 32 位整数类型
        lidar_data = lidar_data.astype(np.int32)
        #再次对数据进行重塑操作
        lidar_data = np.reshape(lidar_data, (-1, 2))
        #定义图像尺寸
        lidar_img_size = (disp_size[0], disp_size[1], 3)
        #创建一个全零的图像数组，类型为无符号 8 位整数
        lidar_img = np.zeros((lidar_img_size), dtype=np.uint8)
        #根据激光雷达数据在图像中绘制白色像素
        lidar_img[tuple(lidar_data.T)] = (255, 255, 255)
        #如果显示管理器允许渲染
        if self.display_man.render_enabled():
            #使用pygame库将激光雷达图像转换为可渲染的表面
            self.surface = pygame.surfarray.make_surface(lidar_img)
        #记录处理结束时的时间
        t_end = self.timer.time()
        #累加处理时间
        self.time_processing += (t_end-t_start)
        #累加处理次数
        self.tics_processing += 1

    def save_radar_image(self, radar_data):
        t_start = self.timer.time()
        #print("Hola, saving Radar data!!")
        # To get a numpy [[vel, altitude, azimuth, depth],...[,,,]]:
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

def one_run(args, client):
    """This function performed one test run using the args parameters
    and connecting to the carla client passed.
    """

    display_manager = None
    vehicle = None
    vehicle_list = []
    prof_str = ""
    timer = CustomTimer()

    try:

        # Getting the world and
        world = client.get_world()

        if args.sync:
            traffic_manager = client.get_trafficmanager(8000)
            settings = world.get_settings()
            traffic_manager.set_synchronous_mode(True)
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 0.05
            world.apply_settings(settings)

        if args.profiling:
            settings.no_rendering_mode = True
            world.apply_settings(settings)


        # 启动我们安装传感器的车辆
        bp = world.get_blueprint_library().filter('vehicle')[0]

        if bp.has_attribute('color'):
            color = random.choice(bp.get_attribute('color').recommended_values)
            bp.set_attribute('color', color)

        vehicle = world.spawn_actor(bp, world.get_map().get_spawn_points()[0])
        vehicle_list.append(vehicle)

        vehicle.set_autopilot(True)


        # 显示管理器将所有传感器组织在一个窗口中显示
        display_manager = DisplayManager(grid_size=[2, 2], window_size=[args.width, args.height], show_window=args.render_window)


        # 如果需要，我们会安装 RGB 相机
        if args.render_cam:
            SensorManager(world, display_manager, 'RGBCamera', carla.Transform(carla.Location(x=1.5, z=2.4)), vehicle, {}, [0, 0])


        #如果有，我们会确定所需的激光雷达
        lidar_points_per_second = args.lidar_points

        if args.lidar_number >= 3:
            SensorManager(world, display_manager, 'LiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '50',  'points_per_second': lidar_points_per_second, 'rotation_frequency': '20'}, [1, 0])

        if args.lidar_number >= 2:
            SensorManager(world, display_manager, 'LiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '100', 'points_per_second': lidar_points_per_second, 'rotation_frequency': '20'}, [0, 1])

        if args.lidar_number >= 1:
            SensorManager(world, display_manager, 'LiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '200', 'points_per_second': lidar_points_per_second, 'rotation_frequency': '20'}, [1, 1])


        # 如果有，我们会确定所需的语义激光雷达
        semanticlidar_points_per_second = args.semanticlidar_points

        if args.semanticlidar_number >= 3:
            SensorManager(world, display_manager, 'SemanticLiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '50', 'points_per_second': semanticlidar_points_per_second, 'rotation_frequency': '20'}, [1, 0])

        if args.semanticlidar_number >= 2:
            SensorManager(world, display_manager, 'SemanticLiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '100', 'points_per_second': semanticlidar_points_per_second, 'rotation_frequency': '20'}, [0, 1])

        if args.semanticlidar_number >= 1:
            SensorManager(world, display_manager, 'SemanticLiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '200', 'points_per_second': semanticlidar_points_per_second, 'rotation_frequency': '20'}, [1, 1])


        # 如果有，我们会安装所需的雷达
        radar_points_per_second = args.radar_points

        if args.radar_number >= 3:
            SensorManager(world, display_manager, 'Radar', carla.Transform(carla.Location(x=0, z=2.4), carla.Rotation(pitch=5, yaw=90)), vehicle, {'points_per_second': radar_points_per_second}, [2, 2])

        if args.radar_number >= 2:
            SensorManager(world, display_manager, 'Radar', carla.Transform(carla.Location(x=0, z=2.4), carla.Rotation(pitch=5, yaw=-90)), vehicle, {'points_per_second': radar_points_per_second}, [2, 2])

        if args.radar_number >= 1:
            SensorManager(world, display_manager, 'Radar', carla.Transform(carla.Location(x=0, z=2.4), carla.Rotation(pitch=5)), vehicle, {'points_per_second': radar_points_per_second}, [2, 2])


        call_exit = False
        time_init_sim = timer.time()

        frame = 0
        time0 = timer.time()

        while True:
            frame += 1

            # Carla Tick
            if args.sync:
                world.tick()
            else:
                world.wait_for_tick()

            # 渲染接收的数据
            display_manager.render()

            # 用于分析或输出的时间测量
            if not args.profiling:
                if frame == 30:
                    time_frames = timer.time() - time0
                    time_procc = 0
                    for sensor in display_manager.sensor_list:
                        time_procc += sensor.time_processing

                    print("FPS: %.3f %.3f %.3f" % (time_frames, 1.0/time_frames * 30, time_procc/time_frames))
                    frame = 0
                    for sensor in display_manager.sensor_list:
                        sensor.time_processing = 0
                        sensor.tics_processing = 0
                    time0 = timer.time()

                if args.render_window:
                    for event in pygame.event.get():
                        if event.type == pygame.QUIT:
                            call_exit = True
                        elif event.type == pygame.KEYDOWN:
                            if event.key == K_ESCAPE or event.key == K_q:
                                call_exit = True
                                break
            else:
                if (timer.time() - time_init_sim) < 5.0:
                    frame = 0
                    for sensor in display_manager.sensor_list:
                        sensor.time_processing = 0
                        sensor.tics_processing = 0
                    time0 = timer.time()

                if (timer.time() - time0) > 10.0:
                    time_frames = timer.time() - time0

                    time_procc = 0
                    for sensor in display_manager.sensor_list:
                        time_procc += sensor.time_processing
                    prof_str = "%-10s %-9s  %-9s %-15s %-7.2f %-20.3f" % (args.lidar_number, args.semanticlidar_number, args.radar_number, lidar_points_per_second, float(frame) / time_frames, time_procc/time_frames)
                    break

            if call_exit:
                break
              
    #用于确保某些操作在程序结束时一定会被执行，无论是否发生异常
    finally:
        #检查display_manager是否存在
        if display_manager:
            #如果存在，则调用display_manager.destroy()
            #用于销毁显示管理器相关资源的操作，通常在图形界面程序中用于释放资源
            display_manager.destroy()
        #销毁vehicle_list中的所有车辆
        client.apply_batch([carla.command.DestroyActor(x) for x in vehicle_list])
        #检查args.sync是否为True
        #args通常是通过命令行参数传入的参数对象
        if args.sync:
            #获取当前世界的设置
            settings = world.get_settings()
            #将同步模式设置为False
            settings.synchronous_mode = False
            #将固定时间步长设置为None
            settings.fixed_delta_seconds = None
            #应用新的设置到世界中
            world.apply_settings(settings)
        #检查args.profiling是否为True
        if args.profiling:
            #将无渲染模式设置为False
            settings.no_rendering_mode = False
            #应用新的设置到世界中
            world.apply_settings(settings)

    
    return prof_str


#定义一个函数，通常是程序的主入口点
def main():
    #创建了一个 ArgumentParser 对象，用于解析命令行参数
    #description 参数提供了对这个参数解析器用途的描述
    argparser = argparse.ArgumentParser(
        description='CARLA Sensor tutorial')
    #添加参数--host
    argparser.add_argument(
        #命令行参数的名称
        '--host',
        #定义了在帮助信息中显示的参数名称
        metavar='H',
        #定义了参数的默认值
        default='127.0.0.1',
        #提供了参数的帮助信息
        help='IP of the host server (default: 127.0.0.1)')
    #添加参数-p 或 --port
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        #type 被指定为 int，表示这个参数应该是一个整数
        type=int,
        #help 提供了参数的帮助信息
        help='TCP port to listen to (default: 2000)')
    #添加参数--sync
    argparser.add_argument(
        #这个参数没有默认值，当在命令行中指定 --sync 时，其值为 True
        '--sync',
        #如果指定了这个参数，其值为 True，否则为 False
        action='store_true',
        #help 提供了参数的帮助信息
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
    argparser.add_argument(
        '-lp', '--lidar_points',
        metavar='LP',
        default='100000',
        help='lidar points per second (default: "100000")')
    argparser.add_argument(
        '-ln', '--lidar_number',
        metavar='LN',
        default=3,
        type=int,
        choices=range(0, 4),
        help='Number of lidars to render (from zero to three)')
    argparser.add_argument(
        '-slp', '--semanticlidar_points',
        metavar='SLP',
        default='100000',
        help='semantic lidar points per second (default: "100000")')
    #添加语义激光雷达（Semantic LiDAR）相关参数
    argparser.add_argument(
        '-sln', '--semanticlidar_number',
        metavar='SLN',
        default=0,
        type=int,
        choices=range(0, 4),
        help='Number of semantic lidars to render (from zero to three)')
    #添加雷达（Radar）相关参数
    argparser.add_argument(
        '-rp', '--radar_points',
        metavar='RP',
        default='100000',
        help='radar points per second (default: "100000")')
    #添加雷达（Radar）相关参数
    argparser.add_argument(
        '-rn', '--radar_number',
        metavar='LN',
        default=0,
        type=int,
        choices=range(0, 4),
        help='Number of radars to render (from zero to three)')
    #添加雷达（Radar）相关参数
    argparser.add_argument(
        '--camera',
        dest='render_cam', action='store_true',
        help='render also RGB camera (camera enable by default)')
    #添加雷达（Radar）相关参数
    argparser.add_argument('--no-camera',
        dest='render_cam', action='store_false',
        help='no render RGB camera (camera disable by default)')
    #添加雷达（Radar）相关参数
    argparser.set_defaults(render_cam=True)
    #添加性能分析和窗口渲染参数
    argparser.add_argument(
        '--profiling',
        action='store_true',
        help='Use the script in profiling mode. It measures the performance of \
         the lidar for different number of points.')
    argparser.set_defaults(profiling=False)
    #添加性能分析和窗口渲染参数
    argparser.add_argument(
        '--no-render-window',
        action='store_false',
        dest='render_window',
        help='Render visualization window.')
    argparser.set_defaults(render_window=True)
    #通过parse_args方法解析命令行参数，并将res参数（窗口分辨率）拆分为宽度和高度的整数
    args = argparser.parse_args()
    args.width, args.height = [int(x) for x in args.res.split('x')]

    try:
        #创建了一个carla.Client对象，用于连接到 CARLA 模拟器
        client = carla.Client(args.host, args.port)
        #设置客户端的超时时间为 5 秒
        client.set_timeout(5.0)
        #如果args.profiling为True
        if args.profiling:
            print("-------------------------------------------------------")
            print("# Running profiling with %s lidars, %s semantic lidars and %s radars." % (args.lidar_number, args.semanticlidar_number, args.radar_number))
            #禁用摄像头渲染和可视化窗口
            args.render_cam = False
            #初始化一些变量，用于存储运行结果和测试的点数范围
            args.render_window = False
            runs_output = []

            points_range = ['100000', '200000', '300000', '400000', '500000',
                            '600000', '700000', '800000', '900000', '1000000',
                            '1100000', '1200000', '1300000', '1400000', '1500000']
            #外层循环遍历points_range中的每个点数
            #内层循环设置args.lidar_points、args.semanticlidar_points和args.radar_points为当前点数
            for points in points_range:
                args.lidar_points = points
                args.semanticlidar_points = points
                args.radar_points = points
                #调用one_run函数进行一次运行，并将结果存储在run_str中
                run_str = one_run(args, client)
                #将运行结果添加到runs_output列表中
                runs_output.append(run_str)
            #打印性能分析相关的信息
            print("-------------------------------------------------------")
            print("# Profiling of parallel raycast sensors (LiDAR and Radar)")
            #尝试导入multiprocessing模块来获取 CPU 核心数信息
            try:
                import multiprocessing
                print("#Number of cores: %d" % multiprocessing.cpu_count())
            except ImportError:
                print("#Hardware information not available, please install the " \
                    "multiprocessing module")

            print("#NumLidars NumSemLids NumRadars PointsPerSecond FPS     PercentageProcessing")
            for o  in runs_output:
                print(o)
        #如果args.profiling为False
        else:
            #直接调用one_run函数进行一次运行
            one_run(args, client)
    #如果用户中断（KeyboardInterrupt），则打印取消信息
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')

#当脚本作为主程序运行时，调用main()函数
if __name__ == '__main__':

    main()
