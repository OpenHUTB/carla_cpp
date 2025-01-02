#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
#在CARLA仿真环境中控制一辆车辆，并使用pygame库显示车辆的RGB和语义分割图像。


import glob
import os
import sys

# 尝试将CARLA的.egg文件路径添加到sys.path，如果失败则忽略。
# 目的是确保能够正确导入CARLA相关模块，不同系统下对应的文件名格式有所不同（win-amd64或linux-x86_64）
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

import random

# 尝试导入pygame模块，用于创建图形用户界面,如果导入失败，抛出运行时错误。
# pygame是用于显示图像等创建可视化界面的重要库，这里确保其已安装能被正确导入
try:
    import pygame
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')

# 尝试导入queue模块,如果导入失败（在Python 2中），从Queue模块导入。
# 用于后续处理传感器数据等时进行数据排队的相关操作
try:
    import queue
except ImportError:
    import Queue as queue


class CarlaSyncMode(object):# 定义一个名为CarlaSyncMode的上下文管理器类。
    """
    Context manager to synchronize output from different sensors. Synchronous
    mode is enabled as long as we are inside this context

        with CarlaSyncMode(world, sensors) as sync_mode:
            while True:
                data = sync_mode.tick(timeout=1.0)

    """
    
 # 类构造函数，初始化world对象和传感器列表。
    def __init__(self, world, *sensors, **kwargs):
        self.world = world
        self.sensors = sensors
        self.frame = None
        self.delta_seconds = 1.0 / kwargs.get('fps', 20)# 默认FPS为20。
        self._queues = []                             # 用于存储传感器数据的队列。
        self._settings = None                         # 用于存储world的当前设置。
        
# 进入上下文管理器时执行的操作。
    def __enter__(self):
        self._settings = self.world.get_settings()  # 获取当前world设置。
        # 应用新的world设置，开启同步模式、设置是否渲染、固定每帧时间间隔等
        self.frame = self.world.apply_settings(carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=self.delta_seconds))

           # 创建队列并注册事件。
        def make_queue(register_event):
            q = queue.Queue()
            register_event(q.put)
            self._queues.append(q)

        make_queue(self.world.on_tick)        # 为world的tick事件创建队列。
        for sensor in self.sensors:           # 为每个传感器创建队列。
            make_queue(sensor.listen)       
        return self

    def tick(self, timeout):                  # 从传感器队列中获取数据。
        self.frame = self.world.tick()                                
        data = [self._retrieve_data(q, timeout) for q in self._queues]
        assert all(x.frame == self.frame for x in data)         # 确保所有数据帧相同。
        return data

    def __exit__(self, *args, **kwargs):                    # 退出上下文管理器时执行的操作。
        self.world.apply_settings(self._settings)            # 恢复之前的world设置。

    def _retrieve_data(self, sensor_queue, timeout):        # 从传感器队列中检索数据。
        while True:
            data = sensor_queue.get(timeout=timeout)
            if data.frame == self.frame:
                return data


def draw_image(surface, image, blend=False):                        # 函数用于在pygame表面绘制图像。
    array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))  # 从图像的原始数据创建numpy数组，指定数据类型为无符号8位整数（对应图像像素值类型）
    array = np.reshape(array, (image.height, image.width, 4))         # 调整数组形状为(height, width, 4)，对应图像的高、宽和包含alpha通道的4个通道
    array = array[:, :, :3]                                         # 去除alpha通道，只保留RGB三个通道。
    array = array[:, :, ::-1]                                       # 反转颜色通道，从CARLA格式转换为pygame能正确显示的格式。
    image_surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))# 创建pygame表面，交换轴顺序以适配pygame显示要求
    if blend:                                                       # 如果需要混合，则设置alpha值，用于特定的图像显示效果。
        image_surface.set_alpha(100)
    surface.blit(image_surface, (0, 0))                             # 将图像绘制到pygame表面。


def get_font():                                                     # 函数用于获取字体。
    fonts = [x for x in pygame.font.get_fonts()]
    default_font = 'ubuntumono'
    font = default_font if default_font in fonts else fonts[0]
    font = pygame.font.match_font(font)
    return pygame.font.Font(font, 14)


def should_quit():# 函数用于检测是否应该退出程序
    for event in pygame.event.get():
        if event.type == pygame.QUIT:# 如果event事件种类等于pygame.QUIT（点击关闭窗口等操作），就返回True
def should_quit():                                                   #函数用于检测是否应该退出程序
    for event in pygame.event.get():
        if event.type == pygame.QUIT:                                #如果event事件种类等于pygame.QUIT，就返回True
            return True
        elif event.type == pygame.KEYUP:
            if event.key == pygame.K_ESCAPE:# 如果按下了ESC键，返回True表示要退出程序
                return True
    return False


def main():
    """
    主函数，用于初始化游戏（或仿真）环境，创建窗口，
    并与CARLA仿真器建立连接，以获取和控制仿真世界。
    """
    # 初始化一个空列表，用于存储游戏（或仿真）中的参与者（actor）
    actor_list = []
    
    # 初始化pygame库，准备进行图形显示
    pygame.init()

    # 创建pygame显示窗口，设置窗口大小为800x600像素，
    # 使用硬件加速表面（HWSURFACE）和双缓冲（DOUBLEBUF）模式，提高显示性能和画面流畅度
    display = pygame.display.set_mode(
        (800, 600),
        pygame.HWSURFACE | pygame.DOUBLEBUF)
    
    # 设置pygame的显示模式，创建一个窗口
    # (800, 600)是窗口的大小，单位是像素
    # pygame.HWSURFACE和pygame.DOUBLEBUF是显示模式的标志
    # HWSURFACE表示使用硬件加速，DOUBLEBUF表示使用双缓冲以减少画面撕裂
    display = pygame.display.set_mode((800, 600),pygame.HWSURFACE | pygame.DOUBLEBUF)   
    # 调用一个假设存在的函数get_font()，用于获取字体对象
    # 该函数在代码段中未定义，可能是在其他地方定义的
    font = get_font()
    
    # 创建一个pygame时钟对象，用于控制帧率
    clock = pygame.time.Clock()

    # 创建CARLA客户端对象，连接到本地（'localhost'）的CARLA服务器，端口号为2000，
    # 并设置客户端操作的超时时间为2.0秒，避免长时间等待无响应
    # 与CARLA仿真器建立连接
    # 'localhost'是仿真器运行的主机地址，2000是仿真器监听的端口号
    client = carla.Client('localhost', 2000)
    
    # 设置与CARLA仿真器交互时的超时时间（秒）
    # 如果在指定时间内没有收到仿真器的响应，将抛出异常
    client.set_timeout(2.0)

    # 通过客户端获取CARLA世界对象，后续所有的实体创建、场景设置等操作都基于这个世界对象进行
    world = client.get_world()

    try:
        # 获取CARLA世界中的地图对象，地图包含了道路、路口、出生点等信息
        m = world.get_map()
        # 从地图的出生点列表中随机选择一个作为车辆的初始位置姿态（start_pose）
        start_pose = random.choice(m.get_spawn_points())
        # 根据初始位置获取对应的路点（waypoint），路点可用于导航、定位等操作
        waypoint = m.get_waypoint(start_pose.location)

        # 获取世界中的蓝图库，蓝图库包含了可以创建的各种实体（如车辆、传感器等）的模板信息
        blueprint_library = world.get_blueprint_library()

        # 在世界中生成一辆车辆，从蓝图库中筛选出车辆类型（'vehicle.*'）的蓝图，
        # 随机选择一个作为车辆的蓝图，然后按照初始位置姿态（start_pose）进行创建，
        # 并将创建的车辆添加到actor_list列表中，方便后续统一管理和销毁
        vehicle = world.spawn_actor(
            random.choice(blueprint_library.filter('vehicle.*')),
            start_pose)
        actor_list.append(vehicle)
        # 暂时关闭车辆的物理模拟，可能是为了方便后续对车辆位置等进行手动控制设置
        vehicle.set_simulate_physics(False)

        # 在世界中生成一个RGB相机传感器，先从蓝图库中找到'sensor.camera.rgb'类型的相机蓝图，
        # 设置相机的位置和旋转姿态（相对车辆的位置偏移和角度），然后将其挂载到之前创建的车辆上，
        # 最后把相机传感器也添加到actor_list列表中
        camera_rgb = world.spawn_actor(
            blueprint_library.find('sensor.camera.rgb'),
            carla.Transform(carla.Location(x=-5.5, z=2.8), carla.Rotation(pitch=-15)),
            attach_to=vehicle)
        actor_list.append(camera_rgb)

         # 同样的方式创建一个语义分割相机传感器，用于获取场景的语义分割图像信息，
        # 其位置和姿态与RGB相机相同，也挂载到车辆上，并添加到actor_list列表
        camera_semseg = world.spawn_actor(
            blueprint_library.find('sensor.camera.semantic_segmentation'),
            carla.Transform(carla.Location(x=-5.5, z=2.8), carla.Rotation(pitch=-15)),
            attach_to=vehicle)
        actor_list.append(camera_semseg)
    
    # 通过客户端对象获取当前的仿真世界
    # 该世界对象包含了仿真中的所有参与者（如车辆、行人等）
    # 以及用于控制这些参与者的方法和属性
    world = client.get_world()

try:
    # 从仿真世界对象中获取地图
    m = world.get_map()
    
    # 从地图的随机生成点中选择一个作为车辆的起始位置
    # 这些生成点通常是预先定义在地图上的，适合车辆安全出现的位置
    start_pose = random.choice(m.get_spawn_points())
    
    # 根据起始位置获取该位置的道路信息（如方向、交通规则等）
    waypoint = m.get_waypoint(start_pose.location)
    
    # 从仿真世界的蓝图库中获取所有车辆蓝图
    # 蓝图定义了车辆的类型、外观、性能等属性
    blueprint_library = world.get_blueprint_library()
    
    # 从所有车辆蓝图中随机选择一个，并在起始位置生成对应的车辆参与者
    # start_pose包含了位置和旋转信息，用于确定车辆在游戏世界中的初始状态
    vehicle = world.spawn_actor(
        random.choice(blueprint_library.filter('vehicle.*')),  # 匹配所有车辆蓝图
        start_pose)
    
    # 将生成的车辆参与者添加到actor_list列表中，以便后续管理
    actor_list.append(vehicle)
    
    # 禁用车辆的物理模拟，以减少计算负担并提高仿真效率
    # 在某些情况下，你可能希望车辆按照物理规律移动；但在其他情况下，你可能希望直接控制车辆
    vehicle.set_simulate_physics(False)
    
    # 生成并附加一个RGB相机传感器到车辆上
    # 该传感器用于捕获车辆周围环境的彩色图像
    camera_rgb = world.spawn_actor(
        blueprint_library.find('sensor.camera.rgb'),  # 查找RGB相机蓝图
        carla.Transform(carla.Location(x=-5.5, z=2.8), carla.Rotation(pitch=-15)),  # 传感器的位置和旋转
        attach_to=vehicle)  # 将传感器附加到车辆上
    
    # 将生成的RGB相机传感器添加到actor_list列表中
    actor_list.append(camera_rgb)
    
    # 生成并附加一个语义分割相机传感器到车辆上
    # 该传感器用于捕获车辆周围环境的语义分割图像
    # 语义分割图像中的每个像素都标记了对应物体的类别（如道路、车辆、行人等）
    camera_semseg = world.spawn_actor(
        blueprint_library.find('sensor.camera.semantic_segmentation'),  # 查找语义分割相机蓝图
        carla.Transform(carla.Location(x=-5.5, z=2.8), carla.Rotation(pitch=-15)),  # 传感器的位置和旋转
        attach_to=vehicle)  # 将传感器附加到车辆上
    
    # 将生成的语义分割相机传感器添加到actor_list列表中
    actor_list.append(camera_semseg)

        # 创建一个同步模式上下文，传入世界对象、RGB相机和语义分割相机，设置帧率为30，
        # 在这个上下文环境中，不同传感器的数据获取将保持同步
        with CarlaSyncMode(world, camera_rgb, camera_semseg, fps=30) as sync_mode:
            while True:
                if should_quit():
                    return
                clock.tick()

                # Advance the simulation and wait for the data.

                snapshot, image_rgb, image_semseg = sync_mode.tick(timeout=2.0)  
                #使用sync_mode.tick函数来进行snapshot（快照），image_rgb（RGB图像数据）和image_semseg（语义分割图像数据）的模拟推进，并设置了一个时间期限2秒

                # 从当前路点的下一组路点（距离当前路点1.5米范围内）中随机选择一个作为新的路点，
                # 然后根据新的路点更新车辆的位置姿态，实现车辆在场景中的移动
                waypoint = random.choice(waypoint.next(1.5))
                vehicle.set_transform(waypoint.transform)

                 # 将语义分割图像的数据转换为CityScapesPalette颜色格式，方便后续可视化展示等操作
                image_semseg.convert(carla.ColorConverter.CityScapesPalette)
                # 计算模拟的帧率，通过当前快照的时间间隔来计算每秒的帧数，取整后得到帧率数值
                fps = round(1.0 / snapshot.timestamp.delta_seconds)

                # 将RGB图像绘制到pygame显示窗口上
                draw_image(display, image_rgb)
                # 将语义分割图像绘制到pygame显示窗口上，设置混合模式（透明度等效果）为开启
                draw_image(display, image_semseg, blend=True)
                # 在显示窗口上绘制实时的实际帧率信息（通过pygame时钟获取），设置文字颜色为白色
                display.blit(
                    font.render('% 5d FPS (real)' % clock.get_fps(), True, (255, 255, 255)),
                    (8, 10))
                 # 在显示窗口上绘制模拟的帧率信息（通过计算得到），设置文字颜色为白色
                display.blit(
                    font.render('% 5d FPS (simulated)' % fps, True, (255, 255, 255)),
                    (8, 28))
                # 更新整个pygame显示窗口，使绘制的图像和文字等内容显示出来
                pygame.display.flip()

    finally:

        """
        finally块中的代码无论程序是正常结束还是出现异常都会执行，
        主要用于清理创建的各种对象，释放资源等操作。
        """
        print('destroying actors.')
        for actor in actor_list:
            actor.destroy()

        pygame.quit()
        print('done.')


if __name__ == '__main__':

    try:

        main()

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
