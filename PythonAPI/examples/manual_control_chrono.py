#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Allows controlling a vehicle with a keyboard. For a simpler and more
# documented example, please take a look at tutorial.py.

"""
Welcome to CARLA manual control.

Use ARROWS or WASD keys for control.

    W            : throttle
    S            : brake
    A/D          : steer left/right
    Q            : toggle reverse
    Space        : hand-brake
    P            : toggle autopilot
    M            : toggle manual transmission
    ,/.          : gear up/down
    CTRL + W     : toggle constant velocity mode at 60 km/h

    L            : toggle next light type
    SHIFT + L    : toggle high beam
    Z/X          : toggle right/left blinker
    I            : toggle interior light

    TAB          : change sensor position
    ` or N       : next sensor
    [1-9]        : change to sensor [1-9]
    G            : toggle radar visualization
    C            : change weather (Shift+C reverse)
    Backspace    : change vehicle

    V            : Select next map layer (Shift+V reverse)
    B            : Load current selected map layer (Shift+B to unload)

    R            : toggle recording images to disk

    CTRL + R     : toggle recording of simulation (replacing any previous)
    CTRL + P     : start replaying last recorded simulation
    CTRL + +     : increments the start time of the replay by 1 second (+SHIFT = 10 seconds)
    CTRL + -     : decrements the start time of the replay by 1 second (+SHIFT = 10 seconds)

    F1           : toggle HUD
    H/?          : toggle help
    ESC          : quit
"""

from __future__ import print_function

# ==============================================================================
# -- find carla module ---------------------------------------------------------
# ==============================================================================
# 以下部分主要是尝试将CARLA模块添加到系统路径中，以便后续能够正确导入和使用CARLA相关的功能
import glob
import os
import sys

try:
    # 根据当前文件的路径，拼接出CARLA模块所在的.egg文件路径，并添加到系统路径中
    # 根据Python版本和操作系统类型来确定具体的文件名格式
    sys.path.append(glob.glob(os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + '/carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass


# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================
# 导入CARLA相关模块以及其他一些常用的Python库
import carla
from carla import ColorConverter as cc

import argparse
import collections
import datetime
import logging
import math
import random
import re
import weakref

try:
    import pygame
    from pygame.locals import KMOD_CTRL
    from pygame.locals import KMOD_SHIFT
    from pygame.locals import K_0
    from pygame.locals import K_9
    from pygame.locals import K_BACKQUOTE
    from pygame.locals import K_BACKSPACE
    from pygame.locals import K_COMMA
    from pygame.locals import K_DOWN
    from pygame.locals import K_ESCAPE
    from pygame.locals import K_F1
    from pygame.locals import K_LEFT
    from pygame.locals import K_PERIOD
    from pygame.locals import K_RIGHT
    from pygame.locals import K_SLASH
    from pygame.locals import K_SPACE
    from pygame.locals import K_TAB
    from pygame.locals import K_UP
    from pygame.locals import K_a
    from pygame.locals import K_b
    from pygame.locals import K_c
    from pygame.locals import K_d
    from pygame.locals import K_g
    from pygame.locals import K_h
    from pygame.locals import K_i
    from pygame.locals import K_l
    from pygame.locals import K_m
    from pygame.locals import K_n
    from pygame.locals import K_p
    from pygame.locals import K_k
    from pygame.locals import K_o
    from pygame.locals import K_j
    from pygame.locals import K_q
    from pygame.locals import K_r
    from pygame.locals import K_s
    from pygame.locals import K_v
    from pygame.locals import K_w
    from pygame.locals import K_x
    from pygame.locals import K_z
    from pygame.locals import K_MINUS
    from pygame.locals import K_EQUALS
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')


# ==============================================================================
# -- Global functions ----------------------------------------------------------
# ==============================================================================

# 查找CARLA中天气预设的函数
def find_weather_presets():
    # 用于将驼峰命名法的字符串分割成单词形式的正则表达式
    rgx = re.compile('.+?(?:(?<=[a-z])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])|$)')
    # 定义一个lambda函数，用于根据正则表达式处理后的结果拼接出更易读的名称
    name = lambda x: ' '.join(m.group(0) for m in rgx.finditer(x))
    # 获取CARLA中WeatherParameters类里所有符合特定命名规则（以大写字母开头）的属性名
    presets = [x for x in dir(carla.WeatherParameters) if re.match('[A-Z].+', x)]
    # 返回包含天气预设参数对象和对应易读名称的元组列表
    return [(getattr(carla.WeatherParameters, x), name(x)) for x in presets]


# 获取Actor显示名称的函数，可对名称进行截断处理
def get_actor_display_name(actor, truncate=250):
    # 将Actor的类型ID中的下划线替换为点，然后分割并取后面部分，再转换为标题格式（首字母大写）来获取名称
    name = ' '.join(actor.type_id.replace('_', '.').title().split('.')[1:])
    # 如果名称长度超过截断长度，则截断并添加省略号
    return (name[:truncate - 1] + u'\u2026') if len(name) > truncate else name


# ==============================================================================
# -- World ---------------------------------------------------------------------
# ==============================================================================

# World类，用于管理整个模拟世界相关的操作和状态
class World(object):
    def __init__(self, carla_world, hud, args):
        # 保存传入的CARLA世界对象
        self.world = carla_world
        # 保存角色名称参数
        self.actor_role_name = args.rolename
        try:
            # 获取地图对象，如果获取失败会抛出运行时错误并进行相应处理
            self.map = self.world.get_map()
        except RuntimeError as error:
            print('RuntimeError: {}'.format(error))
            print('  The server could not send the OpenDRIVE (.xodr) file:')
            print('  Make sure it exists, has the same name of your town, and is correct.')
            sys.exit(1)
        # 保存HUD（ Heads-Up Display，通常用于显示游戏中的各种信息）对象
        self.hud = hud
        # 玩家角色对应的Actor对象，初始化为None
        self.player = None
        # 碰撞传感器对象，初始化为None
        self.collision_sensor = None
        # 车道入侵传感器对象，初始化为None
        self.lane_invasion_sensor = None
        # GNSS（全球导航卫星系统）传感器对象，初始化为None
        self.gnss_sensor = None
        # IMU（惯性测量单元）传感器对象，初始化为None
        self.imu_sensor = None
        # 雷达传感器对象，初始化为None
        self.radar_sensor = None
        # 相机管理器对象，用于管理和操作各种相机传感器，初始化为None
        self.camera_manager = None
        # 获取天气预设列表
        self._weather_presets = find_weather_presets()
        # 当前天气预设的索引，初始化为0
        self._weather_index = 0
        # 用于筛选Actor蓝图的过滤器参数
        self._actor_filter = args.filter
        # 图像显示的伽马值参数
        self._gamma = args.gamma
        # 初始化世界，调用restart方法
        self.restart()
        # 注册每帧更新时要执行的HUD更新函数
        self.world.on_tick(hud.on_world_tick)
        # 记录功能是否启用的标志，初始化为False
        self.recording_enabled = False
        # 记录开始时间，初始化为0
        self.recording_start = 0
        # 恒定速度功能是否启用的标志，初始化为False
        self.constant_velocity_enabled = False
        # 当前地图图层的索引，初始化为0
        self.current_map_layer = 0
        # 地图图层名称列表，包含各种不同类型的地图图层
        self.map_layer_names = [
            carla.MapLayer.NONE,
            carla.MapLayer.Buildings,
            carla.MapLayer.Decals,
            carla.MapLayer.Foliage,
            carla.MapLayer.Ground,
            carla.MapLayer.ParkedVehicles,
            carla.MapLayer.Particles,
            carla.MapLayer.Props,
            carla.MapLayer.StreetLights,
            carla.MapLayer.Walls,
            carla.MapLayer.All
        ]

    def restart(self):
        # 设置玩家的最大速度，普通速度和快速速度的初始值
        self.player_max_speed = 1.589
        self.player_max_speed_fast = 3.713
        # 如果相机管理器存在，获取当前相机索引和相机位置索引，否则初始化为0
        cam_index = self.camera_manager.index if self.camera_manager is not None else 0
        cam_pos_index = self.camera_manager.transform_index if self.camera_manager is not None else 0
        # 从世界蓝图库中根据过滤器筛选出一个随机的蓝图
        blueprint = random.choice(self.world.get_blueprint_library().filter(self._actor_filter))
        # 设置蓝图的角色名称属性
        blueprint.set_attribute('role_name', self.actor_role_name)
        # 如果蓝图有颜色属性，随机选择一个推荐的颜色值并设置
        if blueprint.has_attribute('color'):
            color = random.choice(blueprint.get_attribute('color').recommended_values)
            blueprint.set_attribute('color', color)
        # 如果蓝图有司机ID属性，随机选择一个推荐的司机ID值并设置
        if blueprint.has_attribute('driver_id'):
            driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
            blueprint.set_attribute('driver_id', driver_id)
        # 如果蓝图有无敌属性，设置为无敌（'true'）
        if blueprint.has_attribute('is_invincible'):
            blueprint.set_attribute('is_invincible', 'true')
        # 如果蓝图有速度属性，获取推荐的速度值并设置玩家的最大速度和快速速度
        if blueprint.has_attribute('speed'):
            self.player_max_speed = float(blueprint.get_attribute('speed').recommended_values[1])
            self.player_max_speed_fast = float(blueprint.get_attribute('speed').recommended_values[2])
        else:
            print("No recommended values for 'speed' attribute")
        # 如果玩家对象已经存在，获取其当前位置，升高一定高度，重置旋转角度，然后销毁玩家对象
        if self.player is not None:
            spawn_point = self.player.get_transform()
            spawn_point.location.z += 2.0
            spawn_point.rotation.roll = 0.0
            spawn_point.rotation.pitch = 0.0
            self.destroy()
            self.player = self.world.try_spawn_actor(blueprint, spawn_point)
        # 如果玩家对象还未生成，循环尝试生成玩家
        while self.player is None:
            # 如果地图没有可用的生成点，打印提示信息并退出程序
            if not self.map.get_spawn_points():
                print('There are no spawn points available in your map/town.')
                print('Please add some Vehicle Spawn Point to your UE4 scene.')
                sys.exit(1)
            # 获取地图的生成点列表，随机选择一个生成点（如果有），否则使用默认的变换
            spawn_points = self.map.get_spawn_points()
            spawn_point = random.choice(spawn_points) if spawn_points else carla.Transform()
            self.player = self.world.try_spawn_actor(blueprint, spawn_point)
        # 设置各种传感器
        self.collision_sensor = CollisionSensor(self.player, self.hud)
        self.lane_invasion_sensor = LaneInvasionSensor(self.player, self.hud)
        self.gnss_sensor = GnssSensor(self.player)
        self.imu_sensor = IMUSensor(self.player)
        self.camera_manager = CameraManager(self.player, self.hud, self._gamma)
        self.camera_manager.transform_index = cam_pos_index
        self.camera_manager.set_sensor(cam_index, notify=False)
        # 获取玩家的显示名称并通过HUD显示通知信息
        actor_type = get_actor_display_name(self.player)
        self.hud.notification(actor_type)

    def next_weather(self, reverse=False):
        # 根据传入的参数（是否反向）更新天气预设索引
        self._weather_index += -1 if reverse else 1
        self._weather_index %= len(self._weather_presets)
        # 获取当前索引对应的天气预设
        preset = self._weather_presets[self._weather_index]
        # 通过HUD显示天气变化的通知信息
        self.hud.notification('Weather: %s' % preset[1])
        # 在世界中设置当前的天气
        self.player.get_world().set_weather(preset[0])

    def next_map_layer(self, reverse=False):
        # 根据传入的参数（是否反向）更新当前地图图层索引
        self.current_map_layer += -1 if reverse else 1
        self.current_map_layer %= len(self.map_layer_names)
        # 获取当前索引对应的地图图层
        selected = self.map_layer_names[self.current_map_layer]
        # 通过HUD显示地图图层变化的通知信息
        self.hud.notification('LayerMap selected: %s' % selected)

    def load_map_layer(self, unload=False):
        # 获取当前索引对应的地图图层
        selected = self.map_layer_names[self.current_map_layer]
        if unload:
            # 如果是卸载图层，通过HUD显示卸载图层的通知信息，并在世界中卸载该图层
            self.hud.notification('Unloading map layer: %s' % selected)
            self.world.unload_map_layer(selected)
        else:
            # 如果是加载图层，通过HUD显示加载图层的通知信息，并在世界中加载该图层
            self.hud.notification('Loading map layer: %s' % selected)
            self.world.load_map_layer(selected)

    def toggle_radar(self):
        # 如果雷达传感器不存在，创建一个雷达传感器对象
        if self.radar_sensor is None:
            self.radar_sensor = RadarSensor(self.player)
        # 如果雷达传感器已存在且传感器对象也存在，销毁传感器对象并将雷达传感器置为None
        elif self.radar_sensor.sensor is not None:
            self.radar_sensor.sensor.destroy()
            self.radar_sensor = None

            def tick(self, clock):
                # 调用HUD（Heads-Up Display，通常用于显示游戏中的各种信息）的tick方法，传入当前世界对象和时钟对象，
                # 用于更新HUD相关的状态和显示信息等，一般每帧调用一次
                self.hud.tick(self, clock)

            def render(self, display):
                # 调用相机管理器的render方法，将相机获取到的画面渲染到指定的显示对象上，用于在界面上显示相应的图像内容
                self.camera_manager.render(display)
                # 调用HUD的render方法，将HUD相关的信息（如文字提示、状态显示等）渲染到指定的显示对象上，在界面上展示出来
                self.hud.render(display)

            def destroy_sensors(self):
                # 销毁相机管理器中的传感器对象，先销毁传感器，再将传感器对象置为None，并将相机管理器中的索引也置为None，
                # 一般用于清理传感器相关资源，比如在不需要传感器时进行释放操作
                self.camera_manager.sensor.destroy()
                self.camera_manager.sensor = None
                self.camera_manager.index = None

            def destroy(self):
                # 如果雷达传感器不为None，调用toggle_radar方法来处理雷达传感器（可能是销毁它）
                if self.radar_sensor is not None:
                    self.toggle_radar()
                sensors = [
                    self.camera_manager.sensor,
                    self.collision_sensor.sensor,
                    self.lane_invasion_sensor.sensor,
                    self.gnss_sensor.sensor,
                    self.imu_sensor.sensor]
                # 遍历所有传感器对象，如果传感器不为None，先停止传感器的运行，然后销毁传感器对象，
                # 用于统一清理各种传感器资源，确保在关闭或重置世界等场景下正确释放资源
                for sensor in sensors:
                    if sensor is not None:
                        sensor.stop()
                        sensor.destroy()
                # 如果玩家对象不为None，销毁玩家对象，通常在结束游戏、重置场景等情况下使用，释放玩家相关的资源
                if self.player is not None:
                    self.player.destroy()

            # ==============================================================================
            # -- KeyboardControl -----------------------------------------------------------
            # ==============================================================================

            # 键盘控制类，用于处理键盘输入相关的操作，实现通过键盘控制游戏中的各种行为
            class KeyboardControl(object):
                """Class that handles keyboard input."""

                def __init__(self, world, start_in_autopilot):
                    # 用于标记是否启用了某种汽车模拟相关功能（具体看代码上下文，这里暂不确定确切含义），初始化为False
                    self._carsim_enabled = False
                    # 同样是和汽车模拟相关的一个标记（具体含义依赖代码上下文），初始化为False
                    self._carsim_road = False
                    # 用于标记是否启用了Chrono相关的物理模拟功能（同样需结合代码上下文确定具体用途），初始化为False
                    self._chrono_enabled = False
                    # 标记是否启用自动驾驶功能，传入的参数决定初始值
                    self._autopilot_enabled = start_in_autopilot
                    # 根据世界中的玩家对象类型来初始化相应的控制对象
                    if isinstance(world.player, carla.Vehicle):
                        # 如果玩家是车辆类型，创建车辆控制对象
                        self._control = carla.VehicleControl()
                        # 初始化车辆灯光状态为无灯光状态
                        self._lights = carla.VehicleLightState.NONE
                        # 设置玩家车辆的自动驾驶状态
                        world.player.set_autopilot(self._autopilot_enabled)
                        # 设置玩家车辆的灯光状态
                        world.player.set_light_state(self._lights)
                    elif isinstance(world.player, carla.Walker):
                        # 如果玩家是行人类型，创建行人控制对象
                        self._control = carla.WalkerControl()
                        # 行人不支持自动驾驶，将自动驾驶标记设为False
                        self._autopilot_enabled = False
                        # 获取行人当前的旋转状态
                        self._rotation = world.player.get_transform().rotation
                    else:
                        # 如果玩家对象类型不支持（既不是车辆也不是行人），抛出未实现错误
                        raise NotImplementedError("Actor type not supported")
                    # 用于缓存转向值，初始化为0.0
                    self._steer_cache = 0.0
                    # 通过HUD显示帮助提示信息，显示4秒钟
                    world.hud.notification("Press 'H' or '?' for help.", seconds=4.0)

                def parse_events(self, client, world, clock):
                    # 如果控制对象是车辆控制类型，获取当前车辆灯光状态，用于后续根据键盘操作来更新灯光状态等操作
                    if isinstance(self._control, carla.VehicleControl):
                        current_lights = self._lights
                    # 遍历所有的 pygame 事件（如键盘按键按下、松开，鼠标操作等，这里主要关注键盘事件）
                    for event in pygame.event.get():
                        if event.type == pygame.QUIT:
                            # 如果是退出事件（比如关闭窗口），返回True，一般用于告知主程序可以退出了
                            return True
                        elif event.type == pygame.KEYUP:
                            if self._is_quit_shortcut(event.key):
                                # 如果按下的键是退出快捷键（具体快捷键由 _is_quit_shortcut 方法判断，这里未展示其实现），返回True，用于退出程序
                                return True
                            elif event.key == K_BACKSPACE:
                                if self._autopilot_enabled:
                                    # 如果处于自动驾驶状态，先关闭自动驾驶
                                    world.player.set_autopilot(False)
                                    # 重启世界（可能会重新生成角色、重置场景等相关操作，具体看 world.restart 方法实现）
                                    world.restart()
                                    # 再重新开启自动驾驶
                                    world.player.set_autopilot(True)
                                else:
                                    # 如果未处于自动驾驶状态，直接重启世界
                                    world.restart()
                            elif event.key == K_F1:
                                # 切换HUD显示的信息显示与否（比如显示或隐藏一些详细的状态信息等，具体看 world.hud.toggle_info 方法实现）
                                world.hud.toggle_info()
                            elif event.key == K_v and pygame.key.get_mods() & KMOD_SHIFT:
                                # 如果按下了Shift+V键，反向切换地图图层（调用 world.next_map_layer 方法并传入反向参数，具体功能看该方法实现）
                                world.next_map_layer(reverse=True)
                            elif event.key == K_v:
                                # 如果按下V键，切换地图图层（调用 world.next_map_layer 方法，实现切换到下一个地图图层等相关功能）
                                world.next_map_layer()
                            elif event.key == K_b and pygame.key.get_mods() & KMOD_SHIFT:
                                # 如果按下Shift+B键，卸载地图图层（调用 world.load_map_layer 方法并传入卸载参数，实现从世界中卸载当前选中的地图图层等功能）
                                world.load_map_layer(unload=True)
                            elif event.key == K_b:
                                # 如果按下B键，加载地图图层（调用 world.load_map_layer 方法，实现将指定的地图图层加载到世界中等功能）
                                world.load_map_layer()
                            elif event.key == K_h or (event.key == K_SLASH and pygame.key.get_mods() & KMOD_SHIFT):
                                # 如果按下H键或者Shift+/键，切换帮助信息显示与否（调用 world.hud.help.toggle 方法，实现显示或隐藏帮助文档之类的功能）
                                world.hud.help.toggle()
                            elif event.key == K_TAB:
                                # 如果按下Tab键，切换相机（调用 world.camera_manager.toggle_camera 方法，实现切换不同视角相机等相关功能）
                                world.camera_manager.toggle_camera()
                            elif event.key == K_c and pygame.key.get_mods() & KMOD_SHIFT:
                                # 如果按下Shift+C键，反向切换天气（调用 world.next_weather 方法并传入反向参数，实现切换到上一个天气预设等功能）
                                world.next_weather(reverse=True)
                            elif event.key == K_c:
                                # 如果按下C键，切换天气（调用 world.next_weather 方法，实现切换到下一个天气预设等功能）
                                world.next_weather()
                            elif event.key == K_g:
                                # 如果按下G键，切换雷达（调用 world.toggle_radar 方法，实现开启或关闭雷达传感器等相关功能）
                                world.toggle_radar()
                            elif event.key == K_BACKQUOTE:
                                # 如果按下 ` 键，切换到下一个相机传感器（调用 world.camera_manager.next_sensor 方法，实现切换相机传感器来获取不同视角画面等功能）
                                world.camera_manager.next_sensor()
                            elif event.key == K_n:
                                # 如果按下N键，切换到下一个相机传感器（和按下 ` 键功能类似，也是调用 world.camera_manager.next_sensor 方法来切换相机传感器）
                                world.camera_manager.next_sensor()
                            elif event.key == K_w and (pygame.key.get_mods() & KMOD_CTRL):
                                if world.constant_velocity_enabled:
                                    # 如果恒定速度模式已启用，禁用玩家的恒定速度模式，并更新相应的标记和通过HUD显示提示信息
                                    world.player.disable_constant_velocity()
                                    world.constant_velocity_enabled = False
                                    world.hud.notification("Disabled Constant Velocity Mode")
                                else:
                                    # 如果恒定速度模式未启用，启用玩家的恒定速度模式（设置速度向量等，这里设置为沿X轴方向速度为17，对应大约60km/h，具体转换关系看代码逻辑），
                                    # 更新相应的标记并通过HUD显示提示信息
                                    world.player.enable_constant_velocity(carla.Vector3D(17, 0, 0))
                                    world.constant_velocity_enabled = True
                                    world.hud.notification("Enabled Constant Velocity Mode at 60 km/h")
                            elif event.key > K_0 and event.key <= K_9:
                                # 如果按下数字键0-9，根据按下的数字设置相机传感器（调用 world.camera_manager.set_sensor 方法，传入对应数字索引来切换到指定的相机传感器）
                                world.camera_manager.set_sensor(event.key - 1 - K_0)
                            elif event.key == K_r and not (pygame.key.get_mods() & KMOD_CTRL):
                                # 如果按下普通的R键，切换相机记录功能（调用 world.camera_manager.toggle_recording 方法，实现开始或停止录制相机画面等相关功能）
                                world.camera_manager.toggle_recording()
                            elif event.key == K_r and (pygame.key.get_mods() & KMOD_CTRL):
                                if (world.recording_enabled):
                                    # 如果录制功能已启用，停止录制器，更新录制标记并通过HUD显示提示信息
                                    client.stop_recorder()
                                    world.recording_enabled = False
                                    world.hud.notification("Recorder is OFF")
                                else:
                                    # 如果录制功能未启用，启动录制器（指定录制文件名，这里是"manual_recording.rec"），更新录制标记并通过HUD显示提示信息
                                    client.start_recorder("manual_recording.rec")
                                    world.recording_enabled = True
                                    world.hud.notification("Recorder is ON")
                            elif event.key == K_p and (pygame.key.get_mods() & KMOD_CTRL):
                                # 按下Ctrl+P键，执行以下一系列操作：
                                # 1. 停止录制器
                                client.stop_recorder()
                                world.recording_enabled = False
                                # 2. 记录当前相机传感器索引，然后销毁所有传感器（可能是为了重置传感器相关状态等）
                                current_index = world.camera_manager.index
                                world.destroy_sensors()
                                # 3. 禁用自动驾驶
                                self._autopilot_enabled = False
                                world.player.set_autopilot(self._autopilot_enabled)
                                # 4. 通过HUD显示正在重放文件的提示信息
                                world.hud.notification("Replaying file 'manual_recording.rec'")
                                # 5. 使用客户端对象重放指定的录制文件（"manual_recording.rec"），并传入一些相关参数（如起始时间等，具体看 client.replay_file 方法实现）
                                client.replay_file("manual_recording.rec", world.recording_start, 0, 0)
                                # 6. 重新设置相机传感器为之前记录的索引对应的传感器
                                world.camera_manager.set_sensor(current_index)
                            elif event.key == K_k and (pygame.key.get_mods() & KMOD_CTRL):
                                print("k pressed")
                                if not self._carsim_enabled:
                                    # 如果未启用carsim相关功能，启用它（调用 world.player.enable_carsim 方法，具体实现看对应方法，可能是开启某种车辆模拟相关物理效果等）
                                    self._carsim_enabled = True
                                    world.player.enable_carsim()
                                else:
                                    # 如果已启用，禁用它（调用 world.player.restore_physx_physics 方法，可能是恢复到默认的物理效果等）
                                    self._carsim_enabled = False
                                    world.player.restore_physx_physics()
                            elif event.key == K_o and (pygame.key.get_mods() & KMOD_CTRL):
                                print("o pressed")
                                if not self._chrono_enabled:
                                    # 如果未启用Chrono相关的物理模拟功能，启用它（调用 world.player.enable_chrono_physics 方法，传入一系列参数来配置相关物理模拟，
                                    # 具体参数含义看对应方法实现，比如车辆配置文件、动力系统配置文件、轮胎配置文件等路径相关信息）
                                    self._chrono_enabled = True
                                    vehicle_json = "sedan/vehicle/Sedan_Vehicle.json"
                                    powertrain_json = "sedan/powertrain/Sedan_SimpleMapPowertrain.json"
                                    tire_json = "sedan/tire/Sedan_TMeasyTire.json"
                                    base_path = "/home/adas/carla/Build/chrono-install/share/chrono/data/vehicle/"
                                    world.player.enable_chrono_physics(5000, 0.002, vehicle_json, powertrain_json,
                                                                       tire_json, base_path)
                                else:
                                    # 如果已启用，禁用它（调用 world.player.restore_physx_physics 方法，恢复到默认物理效果）
                                    self._chrono_enabled = False
                                    world.player.restore_physx_physics()
                            elif event.key == K_j and (pygame.key.get_mods() & KMOD_CTRL):
                                self._carsim_road = not self._carsim_road
                                # 切换carsim道路相关的使用状态（调用 world.player.use_carsim_road 方法，根据状态决定是否使用相关道路模拟等功能，具体看对应方法实现）
                                world.player.use_carsim_road(self._carsim_road)
                                print("j pressed, using carsim road =", self._carsim_road)
                # elif event.key == K_i and (pygame.key.get_mods() & KMOD_CTRL):
                #     print("i pressed")
                #     imp = carla.Location(z=50000)
                #     world.player.add_impulse(imp)
                # 如果按下Ctrl + -键
                elif event.key == K_MINUS and (pygame.key.get_mods() & KMOD_CTRL):
                if pygame.key.get_mods() & KMOD_SHIFT:
                    # 如果同时按下了Shift键，将世界（world）对象中的录制开始时间（recording_start）减少10
                    world.recording_start -= 10
                else:
                    # 如果仅按下Ctrl + -键，将录制开始时间减少1
                    world.recording_start -= 1
                # 通过HUD（ Heads-Up Display，用于显示游戏中的各种信息）显示当前录制开始时间的提示信息
                world.hud.notification("Recording start time is %d" % (world.recording_start))

            # 如果按下Ctrl + =键
            elif event.key == K_EQUALS and (pygame.key.get_mods() & KMOD_CTRL):
            if pygame.key.get_mods() & KMOD_SHIFT:
                # 如果同时按下了Shift键，将世界对象中的录制开始时间增加10
                world.recording_start += 10
            else:
                # 如果仅按下Ctrl + =键，将录制开始时间增加1
                world.recording_start += 1
            # 通过HUD显示当前录制开始时间的提示信息
            world.hud.notification("Recording start time is %d" % (world.recording_start))

        # 如果控制对象（_control）是车辆控制类型（carla.VehicleControl）
        if isinstance(self._control, carla.VehicleControl):
            # 如果按下Q键
            if event.key == K_q:
                # 根据当前是否处于倒车状态（reverse属性）来设置挡位（gear），如果处于倒车状态则设为 -1，否则设为1
                self._control.gear = 1 if self._control.reverse else -1
            # 如果按下M键
            elif event.key == K_m:
                # 切换手动换挡模式（manual_gear_shift），取反其当前状态
                self._control.manual_gear_shift = not self._control.manual_gear_shift
                # 将挡位设置为玩家车辆当前控制对象中的挡位
                self._control.gear = world.player.get_control().gear
                # 通过HUD显示当前的换挡模式（手动或自动）提示信息
                world.hud.notification(
                    '%s Transmission' % ('Manual' if self._control.manual_gear_shift else 'Automatic'))
            # 如果处于手动换挡模式且按下逗号（,）键
            elif self._control.manual_gear_shift and event.key == K_COMMA:
                # 将挡位减1，但挡位最小值限制为 -1
                self._control.gear = max(-1, self._control.gear - 1)
            # 如果处于手动换挡模式且按下句号（.）键
            elif self._control.manual_gear_shift and event.key == K_PERIOD:
                # 将挡位加1
                self._control.gear = self._control.gear + 1
            # 如果按下P键且未同时按下Ctrl键
            elif event.key == K_p and not pygame.key.get_mods() & KMOD_CTRL:
                # 切换自动驾驶（autopilot）状态，取反其当前状态
                self._autopilot_enabled = not self._autopilot_enabled
                # 设置玩家车辆的自动驾驶状态
                world.player.set_autopilot(self._autopilot_enabled)
                # 通过HUD显示自动驾驶是开启还是关闭的提示信息
                world.hud.notification('Autopilot %s' % ('On' if self._autopilot_enabled else 'Off'))
            # 如果按下L键且同时按下Ctrl键
            elif event.key == K_l and pygame.key.get_mods() & KMOD_CTRL:
                # 对车辆特殊灯光状态（Special1）进行异或操作，用于切换该灯光状态
                current_lights ^= carla.VehicleLightState.Special1
            # 如果按下L键且同时按下Shift键
            elif event.key == K_l and pygame.key.get_mods() & KMOD_SHIFT:
                # 对车辆远光灯状态（HighBeam）进行异或操作，用于切换远光灯状态
                current_lights ^= carla.VehicleLightState.HighBeam
            # 如果按下L键
            elif event.key == K_l:
                # 使用 'L' 键来切换车辆灯光状态，按照如下顺序切换：关闭 -> 位置灯 -> 近光灯 -> 雾灯
                if not self._lights & carla.VehicleLightState.Position:
                    # 如果当前位置灯未开启，通过HUD显示提示信息“Position lights”，并将位置灯状态设置为开启
                    world.hud.notification("Position lights")
                    current_lights |= carla.VehicleLightState.Position
                else:
                    # 如果位置灯已开启，通过HUD显示提示信息“Low beam lights”，并将近光灯状态设置为开启
                    world.hud.notification("Low beam lights")
                    current_lights |= carla.VehicleLightState.LowBeam
                if self._lights & carla.VehicleLightState.LowBeam:
                    # 如果近光灯已开启，通过HUD显示提示信息“Fog lights”，并将雾灯状态设置为开启
                    world.hud.notification("Fog lights")
                    current_lights |= carla.VehicleLightState.Fog
                if self._lights & carla.VehicleLightState.Fog:
                    # 如果雾灯已开启，通过HUD显示提示信息“Lights off”，并依次关闭位置灯、近光灯、雾灯
                    world.hud.notification("Lights off")
                    current_lights ^= carla.VehicleLightState.Position
                    current_lights ^= carla.VehicleLightState.LowBeam
                    current_lights ^= carla.VehicleLightState.Fog
            # 如果按下I键
            elif event.key == K_i:
                # 对车辆内饰灯状态（Interior）进行异或操作，用于切换内饰灯状态
                current_lights ^= carla.VehicleLightState.Interior
            # 如果按下Z键
            elif event.key == K_z:
                # 对车辆左转向灯状态（LeftBlinker）进行异或操作，用于切换左转向灯状态
                current_lights ^= carla.VehicleLightState.LeftBlinker
            # 如果按下X键
            elif event.key == K_x:
                # 对车辆右转向灯状态（RightBlinker）进行异或操作，用于切换右转向灯状态
                current_lights ^= carla.VehicleLightState.RightBlinker

        # 如果自动驾驶未启用
        if not self._autopilot_enabled:
            # 如果控制对象是车辆控制类型
            if isinstance(self._control, carla.VehicleControl):
                # 解析车辆控制相关的按键操作，传入当前按下的键盘按键状态（keys）和时间（milliseconds）
                self._parse_vehicle_keys(pygame.key.get_pressed(), clock.get_time())
                # 根据挡位（gear）判断是否处于倒车状态，小于0则为倒车
                self._control.reverse = self._control.gear < 0
                # 设置与自动控制相关的车辆灯光状态
                if self._control.brake:
                    # 如果刹车被按下，将刹车灯状态（Brake）设置为开启
                    current_lights |= carla.VehicleLightState.Brake
                else:
                    # 如果刹车未被按下，清除刹车灯状态（将其设为关闭）
                    current_lights &= ~carla.VehicleLightState.Brake
                if self._control.reverse:
                    # 如果处于倒车状态，将倒车灯状态（Reverse）设置为开启
                    current_lights |= carla.VehicleLightState.Reverse
                else:
                    # 如果不处于倒车状态，清除倒车灯状态（将其设为关闭）
                    current_lights &= ~carla.VehicleLightState.Reverse
                # 如果当前灯光状态（current_lights）与之前保存的灯光状态（_lights）不同（即有变化）
                if current_lights != self._lights:
                    # 更新灯光状态为当前状态
                    self._lights = current_lights
                    # 设置玩家车辆的灯光状态为更新后的状态
                    world.player.set_light_state(carla.VehicleLightState(self._lights))
            # 如果控制对象是行人控制类型（carla.WalkerControl）
            elif isinstance(self._control, carla.WalkerControl):
                # 解析行人控制相关的按键操作，传入当前按下的键盘按键状态、时间以及世界对象
                self._parse_walker_keys(pygame.key.get_pressed(), clock.get_time(), world)
            # 将最终的控制指令应用到玩家角色上，使相应的控制生效
            world.player.apply_control(self._control)

        # 解析车辆控制按键的方法，用于根据键盘按键状态更新车辆控制对象的属性
        def _parse_vehicle_keys(self, keys, milliseconds):
            if keys[K_UP] or keys[K_w]:
                # 如果按下向上箭头键或W键，逐渐增加油门（throttle）值，最大值限制为1
                self._control.throttle = min(self._control.throttle + 0.01, 1)
            else:
                # 如果未按下上述按键，油门值设为0
                self._control.throttle = 0.0

            if keys[K_DOWN] or keys[K_s]:
                # 如果按下向下箭头键或S键，逐渐增加刹车（brake）值，最大值限制为1
                self._control.brake = min(self._control.brake + 0.2, 1)
            else:
                # 如果未按下上述按键，刹车值设为0
                self._control.brake = 0

            # 根据时间计算转向增量，时间越长，转向增量越大
            steer_increment = 5e-4 * milliseconds
            if keys[K_LEFT] or keys[K_a]:
                if self._steer_cache > 0:
                    # 如果之前的转向缓存值大于0，将其设为0（表示开始向左转向）
                    self._steer_cache = 0
                else:
                    # 否则，减少转向缓存值，实现向左转向的效果
                    self._steer_cache -= steer_increment
            elif keys[K_RIGHT] or keys[K_d]:
                if self._steer_cache < 0:
                    # 如果之前的转向缓存值小于0，将其设为0（表示开始向右转向）
                    self._steer_cache = 0
                else:
                    # 否则，增加转向缓存值，实现向右转向的效果
                    self._steer_cache += steer_increment
            else:
                # 如果没有左右转向按键按下，将转向缓存值设为0
                self._steer_cache = 0.0
            # 将转向缓存值限制在 -0.7到0.7之间，防止转向幅度过大
            self._steer_cache = min(0.7, max(-0.7, self._steer_cache))
            # 将车辆控制对象的转向（steer）属性设置为四舍五入后的转向缓存值，保留一位小数
            self._control.steer = round(self._steer_cache, 1)
            # 根据是否按下空格键来设置手刹（hand_brake）状态
            self._control.hand_brake = keys[K_SPACE]

        # 解析行人控制按键的方法，用于根据键盘按键状态更新行人控制对象的属性
        def _parse_walker_keys(self, keys, milliseconds, world):
            self._control.speed = 0.0
            if keys[K_DOWN] or keys[K_s]:
                # 如果按下向下箭头键或S键，行人速度设为0
                self._control.speed = 0.0
            if keys[K_LEFT] or keys[K_a]:
                # 如果按下向左箭头键或A键，设置行人速度为一个较小的值（表示缓慢向左移动），并减少行人的朝向角度（yaw）
                self._control.speed = .01
                self._rotation.yaw -= 0.08 * milliseconds
            if keys[K_RIGHT] or keys[K_d]:
                # 如果按下向右箭头键或D键，设置行人速度为一个较小的值（表示缓慢向右移动），并增加行人的朝向角度
                self._control.speed = .01
                self._rotation.yaw += 0.08 * milliseconds
            if keys[K_UP] or keys[K_w]:
                # 如果按下向上箭头键或W键，根据是否按下Shift键来设置行人的速度（快速或普通速度）
                self._control.speed = world.player_max_speed_fast if pygame.key.get_mods() & KMOD_SHIFT else world.player_max_speed
            # 根据是否按下空格键来设置行人的跳跃（jump）状态
            self._control.jump = keys[K_SPACE]
            # 将行人的朝向角度（yaw）四舍五入保留一位小数
            self._rotation.yaw = round(self._rotation.yaw, 1)
            # 设置行人的移动方向为当前朝向对应的前向向量
            self._control.direction = self._rotation.get_forward_vector()

        # 静态方法，用于判断按下的键是否是退出快捷键（Escape键或者Ctrl + Q键）
        @staticmethod
        def _is_quit_shortcut(key):
            return (key == K_ESCAPE) or (key == K_q and pygame.key.get_mods() & KMOD_CTRL)

        # ==============================================================================
        # -- HUD（Heads-Up Display，用于显示游戏中的各种信息）相关类定义----------------------------------
        # ==============================================================================

        class HUD(object):
            def __init__(self, width, height):
                # 保存显示区域的尺寸（宽度和高度）
                self.dim = (width, height)
                # 创建一个默认字体的字体对象，字号为20
                font = pygame.font.Font(pygame.font.get_default_font(), 20)
                font_name = 'courier' if os.name == 'nt' else 'mono'
                fonts = [x for x in pygame.font.get_fonts() if font_name in x]
                default_font = 'ubuntumono'
                mono = default_font if default_font in fonts else fonts[0]
                mono = pygame.font.match_font(mono)
                # 创建用于显示单字节字体的字体对象，字号根据操作系统不同设置（Windows为12，其他为14）
                self._font_mono = pygame.font.Font(mono, 12 if os.name == 'nt' else 14)
                # 创建一个用于显示渐隐提示信息的对象，传入字体、显示区域位置等参数
                self._notifications = FadingText(font, (width, 40), (0, height - 40))
                # 创建一个用于显示帮助信息的对象，传入字体、显示区域宽度和高度等参数
                self.help = HelpText(pygame.font.Font(mono, 16), width, height)
                # 服务器端的帧率，初始化为0
                self.server_fps = 0
                # 当前帧的序号，初始化为0
                self.frame = 0
                # 模拟时间（从开始到现在经过的时间），初始化为0
                self.simulation_time = 0
                # 标记是否显示详细信息，初始化为True
                self._show_info = True
                # 用于存储要显示的详细信息文本内容，初始化为空列表
                self._info_text = []
                # 创建一个用于控制服务器端时间的时钟对象
                self._server_clock = pygame.time.Clock()

            def on_world_tick(self, timestamp):
                # 让服务器端时钟对象进行一次滴答更新（用于控制帧率等相关逻辑）
                self._server_clock.tick()
                # 获取服务器端当前的帧率并赋值给相应属性
                self.server_fps = self._server_clock.get_fps()
                # 获取当前帧的序号并赋值给相应属性
                self.frame = timestamp.frame
                # 获取从开始到现在经过的模拟时间并赋值给相应属性
                self.simulation_time = timestamp.elapsed_seconds

            def tick(self, world, clock):
                # 更新渐隐提示信息的状态（比如根据时间判断是否要继续显示、是否要渐隐消失等），传入世界对象和客户端时钟对象
                self._notifications.tick(world, clock)
                if not self._show_info:
                    # 如果不显示详细信息，直接返回，不进行后续详细信息的更新和显示操作
                    return
                # 获取玩家角色的位置和姿态信息（变换信息）
                t = world.player.get_transform()
                # 获取玩家角色的速度信息
                v = world.player.get_velocity()
                # 获取玩家角色当前的控制对象（车辆控制或行人控制等）
                c = world.player.get_control()
                # 获取玩家角色上的惯性测量单元（IMU）传感器的指南针方向数据（角度值）
                compass = world.imu_sensor.compass
                heading = 'N' if compass > 270.5 or compass < 89.5 else ''
                heading += 'S' if 90.5 < compass < 269.5 else ''
                heading += 'E' if 0.5 < compass < 179.5 else ''
                heading += 'W' if 180.5 < compass < 359.5 else ''
                colhist = world.collision_sensor.get_collision_history()
                collision = [colhist[x + self.frame - 200] for x in range(0, 200)]
                max_col = max(1.0, max(collision))
                collision = [x / max_col for x in collision]
                vehicles = world.world.get_actors().filter('vehicle.*')
                self._info_text = [
                    'Server:  % 16.0f FPS' % self.server_fps,
                    'Client:  % 16.0f FPS' % clock.get_fps(),
                    '',
                    'Vehicle: % 20s' % get_actor_display_name(world.player, truncate

    def toggle_info(self):
        self._show_info = not self._show_info

    def notification(self, text, seconds=2.0):
        self._notifications.set_text(text, seconds=seconds)

    def error(self, text):
        self._notifications.set_text('Error: %s' % text, (255, 0, 0))

    def render(self, display):
        if self._show_info:
            info_surface = pygame.Surface((220, self.dim[1]))
            info_surface.set_alpha(100)
            display.blit(info_surface, (0, 0))
            v_offset = 4
            bar_h_offset = 100
            bar_width = 106
            for item in self._info_text:
                if v_offset + 18 > self.dim[1]:
                    break
                if isinstance(item, list):
                    if len(item) > 1:
                        points = [(x + 8, v_offset + 8 + (1.0 - y) * 30) for x, y in enumerate(item)]
                        pygame.draw.lines(display, (255, 136, 0), False, points, 2)
                    item = None
                    v_offset += 18
                elif isinstance(item, tuple):
                    if isinstance(item[1], bool):
                        rect = pygame.Rect((bar_h_offset, v_offset + 8), (6, 6))
                        pygame.draw.rect(display, (255, 255, 255), rect, 0 if item[1] else 1)
                    else:
                        rect_border = pygame.Rect((bar_h_offset, v_offset + 8), (bar_width, 6))
                        pygame.draw.rect(display, (255, 255, 255), rect_border, 1)
                        f = (item[1] - item[2]) / (item[3] - item[2])
                        if item[2] < 0.0:
                            rect = pygame.Rect((bar_h_offset + f * (bar_width - 6), v_offset + 8), (6, 6))
                        else:
                            rect = pygame.Rect((bar_h_offset, v_offset + 8), (f * bar_width, 6))
                        pygame.draw.rect(display, (255, 255, 255), rect)
                    item = item[0]
                if item:  # At this point has to be a str.
                    surface = self._font_mono.render(item, True, (255, 255, 255))
                    display.blit(surface, (8, v_offset))
                v_offset += 18
        self._notifications.render(display)
        self.help.render(display)


# ==============================================================================
# -- FadingText ----------------------------------------------------------------
# ==============================================================================


class FadingText(object):
    def __init__(self, font, dim, pos):
        self.font = font
        self.dim = dim
        self.pos = pos
        self.seconds_left = 0
        self.surface = pygame.Surface(self.dim)

    def set_text(self, text, color=(255, 255, 255), seconds=2.0):
        text_texture = self.font.render(text, True, color)
        self.surface = pygame.Surface(self.dim)
        self.seconds_left = seconds
        self.surface.fill((0, 0, 0, 0))
        self.surface.blit(text_texture, (10, 11))

    def tick(self, _, clock):
        delta_seconds = 1e-3 * clock.get_time()
        self.seconds_left = max(0.0, self.seconds_left - delta_seconds)
        self.surface.set_alpha(500.0 * self.seconds_left)

    def render(self, display):
        display.blit(self.surface, self.pos)


# ==============================================================================
# -- HelpText ------------------------------------------------------------------
# ==============================================================================


class HelpText(object):
    """Helper class to handle text output using pygame"""
    def __init__(self, font, width, height):
        lines = __doc__.split('\n')
        self.font = font
        self.line_space = 18
        self.dim = (780, len(lines) * self.line_space + 12)
        self.pos = (0.5 * width - 0.5 * self.dim[0], 0.5 * height - 0.5 * self.dim[1])
        self.seconds_left = 0
        self.surface = pygame.Surface(self.dim)
        self.surface.fill((0, 0, 0, 0))
        for n, line in enumerate(lines):
            text_texture = self.font.render(line, True, (255, 255, 255))
            self.surface.blit(text_texture, (22, n * self.line_space))
            self._render = False
        self.surface.set_alpha(220)

    def toggle(self):
        self._render = not self._render

    def render(self, display):
        if self._render:
            display.blit(self.surface, self.pos)


# ==============================================================================
# -- CollisionSensor -----------------------------------------------------------
# ==============================================================================


class CollisionSensor(object):
    def __init__(self, parent_actor, hud):
        self.sensor = None
        self.history = []
        self._parent = parent_actor
        self.hud = hud
        world = self._parent.get_world()
        bp = world.get_blueprint_library().find('sensor.other.collision')
        self.sensor = world.spawn_actor(bp, carla.Transform(), attach_to=self._parent)
        # We need to pass the lambda a weak reference to self to avoid circular
        # reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda event: CollisionSensor._on_collision(weak_self, event))

    def get_collision_history(self):
        history = collections.defaultdict(int)
        for frame, intensity in self.history:
            history[frame] += intensity
        return history

    @staticmethod
    def _on_collision(weak_self, event):
        self = weak_self()
        if not self:
            return
        actor_type = get_actor_display_name(event.other_actor)
        self.hud.notification('Collision with %r' % actor_type)
        impulse = event.normal_impulse
        intensity = math.sqrt(impulse.x**2 + impulse.y**2 + impulse.z**2)
        self.history.append((event.frame, intensity))
        if len(self.history) > 4000:
            self.history.pop(0)


# ==============================================================================
# -- LaneInvasionSensor --------------------------------------------------------
# ==============================================================================


class LaneInvasionSensor(object):
    def __init__(self, parent_actor, hud):
        self.sensor = None
        self._parent = parent_actor
        self.hud = hud
        world = self._parent.get_world()
        bp = world.get_blueprint_library().find('sensor.other.lane_invasion')
        self.sensor = world.spawn_actor(bp, carla.Transform(), attach_to=self._parent)
        # We need to pass the lambda a weak reference to self to avoid circular
        # reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda event: LaneInvasionSensor._on_invasion(weak_self, event))

    @staticmethod
    def _on_invasion(weak_self, event):
        self = weak_self()
        if not self:
            return
        lane_types = set(x.type for x in event.crossed_lane_markings)
        text = ['%r' % str(x).split()[-1] for x in lane_types]
        self.hud.notification('Crossed line %s' % ' and '.join(text))


# ==============================================================================
# -- GnssSensor ----------------------------------------------------------------
# ==============================================================================


class GnssSensor(object):
    def __init__(self, parent_actor):
        self.sensor = None
        self._parent = parent_actor
        self.lat = 0.0
        self.lon = 0.0
        world = self._parent.get_world()
        bp = world.get_blueprint_library().find('sensor.other.gnss')
        self.sensor = world.spawn_actor(bp, carla.Transform(carla.Location(x=1.0, z=2.8)), attach_to=self._parent)
        # We need to pass the lambda a weak reference to self to avoid circular
        # reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda event: GnssSensor._on_gnss_event(weak_self, event))

    @staticmethod
    def _on_gnss_event(weak_self, event):
        self = weak_self()
        if not self:
            return
        self.lat = event.latitude
        self.lon = event.longitude


# ==============================================================================
# -- IMUSensor -----------------------------------------------------------------
# ==============================================================================


class IMUSensor(object):
    def __init__(self, parent_actor):
        self.sensor = None
        self._parent = parent_actor
        self.accelerometer = (0.0, 0.0, 0.0)
        self.gyroscope = (0.0, 0.0, 0.0)
        self.compass = 0.0
        world = self._parent.get_world()
        bp = world.get_blueprint_library().find('sensor.other.imu')
        self.sensor = world.spawn_actor(
            bp, carla.Transform(), attach_to=self._parent)
        # We need to pass the lambda a weak reference to self to avoid circular
        # reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(
            lambda sensor_data: IMUSensor._IMU_callback(weak_self, sensor_data))

    @staticmethod
    def _IMU_callback(weak_self, sensor_data):
        self = weak_self()
        if not self:
            return
        limits = (-99.9, 99.9)
        self.accelerometer = (
            max(limits[0], min(limits[1], sensor_data.accelerometer.x)),
            max(limits[0], min(limits[1], sensor_data.accelerometer.y)),
            max(limits[0], min(limits[1], sensor_data.accelerometer.z)))
        self.gyroscope = (
            max(limits[0], min(limits[1], math.degrees(sensor_data.gyroscope.x))),
            max(limits[0], min(limits[1], math.degrees(sensor_data.gyroscope.y))),
            max(limits[0], min(limits[1], math.degrees(sensor_data.gyroscope.z))))
        self.compass = math.degrees(sensor_data.compass)


# ==============================================================================
# -- RadarSensor ---------------------------------------------------------------
# ==============================================================================


class RadarSensor(object):
    def __init__(self, parent_actor):
        self.sensor = None
        self._parent = parent_actor
        self.velocity_range = 7.5 # m/s
        world = self._parent.get_world()
        self.debug = world.debug
        bp = world.get_blueprint_library().find('sensor.other.radar')
        bp.set_attribute('horizontal_fov', str(35))
        bp.set_attribute('vertical_fov', str(20))
        self.sensor = world.spawn_actor(
            bp,
            carla.Transform(
                carla.Location(x=2.8, z=1.0),
                carla.Rotation(pitch=5)),
            attach_to=self._parent)
        # We need a weak reference to self to avoid circular reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(
            lambda radar_data: RadarSensor._Radar_callback(weak_self, radar_data))

    @staticmethod
    def _Radar_callback(weak_self, radar_data):
        self = weak_self()
        if not self:
            return
        # To get a numpy [[vel, altitude, azimuth, depth],...[,,,]]:
        # points = np.frombuffer(radar_data.raw_data, dtype=np.dtype('f4'))
        # points = np.reshape(points, (len(radar_data), 4))

        current_rot = radar_data.transform.rotation
        for detect in radar_data:
            azi = math.degrees(detect.azimuth)
            alt = math.degrees(detect.altitude)
            # The 0.25 adjusts a bit the distance so the dots can
            # be properly seen
            fw_vec = carla.Vector3D(x=detect.depth - 0.25)
            carla.Transform(
                carla.Location(),
                carla.Rotation(
                    pitch=current_rot.pitch + alt,
                    yaw=current_rot.yaw + azi,
                    roll=current_rot.roll)).transform(fw_vec)

            def clamp(min_v, max_v, value):
                return max(min_v, min(value, max_v))

            norm_velocity = detect.velocity / self.velocity_range # range [-1, 1]
            r = int(clamp(0.0, 1.0, 1.0 - norm_velocity) * 255.0)
            g = int(clamp(0.0, 1.0, 1.0 - abs(norm_velocity)) * 255.0)
            b = int(abs(clamp(- 1.0, 0.0, - 1.0 - norm_velocity)) * 255.0)
            self.debug.draw_point(
                radar_data.transform.location + fw_vec,
                size=0.075,
                life_time=0.06,
                persistent_lines=False,
                color=carla.Color(r, g, b))

# ==============================================================================
# -- CameraManager -------------------------------------------------------------
# ==============================================================================


class CameraManager(object):
    def __init__(self, parent_actor, hud, gamma_correction):
        self.sensor = None
        self.surface = None
        self._parent = parent_actor
        self.hud = hud
        self.recording = False
        bound_y = 0.5 + self._parent.bounding_box.extent.y
        Attachment = carla.AttachmentType
        self._camera_transforms = [
            (carla.Transform(carla.Location(x=-5.5, z=2.5), carla.Rotation(pitch=8.0)), Attachment.SpringArmGhost),
            (carla.Transform(carla.Location(x=1.6, z=1.7)), Attachment.Rigid),
            (carla.Transform(carla.Location(x=5.5, y=1.5, z=1.5)), Attachment.SpringArmGhost),
            (carla.Transform(carla.Location(x=-8.0, z=6.0), carla.Rotation(pitch=6.0)), Attachment.SpringArmGhost),
            (carla.Transform(carla.Location(x=-1, y=-bound_y, z=0.5)), Attachment.Rigid)]
        self.transform_index = 1
        self.sensors = [
            ['sensor.camera.rgb', cc.Raw, 'Camera RGB', {}],
            ['sensor.camera.depth', cc.Raw, 'Camera Depth (Raw)', {}],
            ['sensor.camera.depth', cc.Depth, 'Camera Depth (Gray Scale)', {}],
            ['sensor.camera.depth', cc.LogarithmicDepth, 'Camera Depth (Logarithmic Gray Scale)', {}],
            ['sensor.camera.semantic_segmentation', cc.Raw, 'Camera Semantic Segmentation (Raw)', {}],
            ['sensor.camera.semantic_segmentation', cc.CityScapesPalette,
                'Camera Semantic Segmentation (CityScapes Palette)', {}],
            ['sensor.lidar.ray_cast', None, 'Lidar (Ray-Cast)', {'range': '50'}],
            ['sensor.camera.dvs', cc.Raw, 'Dynamic Vision Sensor', {}],
            ['sensor.camera.rgb', cc.Raw, 'Camera RGB Distorted',
                {'lens_circle_multiplier': '3.0',
                'lens_circle_falloff': '3.0',
                'chromatic_aberration_intensity': '0.5',
                'chromatic_aberration_offset': '0'}]]
        world = self._parent.get_world()
        bp_library = world.get_blueprint_library()
        for item in self.sensors:
            bp = bp_library.find(item[0])
            if item[0].startswith('sensor.camera'):
                bp.set_attribute('image_size_x', str(hud.dim[0]))
                bp.set_attribute('image_size_y', str(hud.dim[1]))
                if bp.has_attribute('gamma'):
                    bp.set_attribute('gamma', str(gamma_correction))
                for attr_name, attr_value in item[3].items():
                    bp.set_attribute(attr_name, attr_value)
            elif item[0].startswith('sensor.lidar'):
                self.lidar_range = 50

                for attr_name, attr_value in item[3].items():
                    bp.set_attribute(attr_name, attr_value)
                    if attr_name == 'range':
                        self.lidar_range = float(attr_value)

            item.append(bp)
        self.index = None

    def toggle_camera(self):
        self.transform_index = (self.transform_index + 1) % len(self._camera_transforms)
        self.set_sensor(self.index, notify=False, force_respawn=True)

    def set_sensor(self, index, notify=True, force_respawn=False):
        index = index % len(self.sensors)
        needs_respawn = True if self.index is None else \
            (force_respawn or (self.sensors[index][2] != self.sensors[self.index][2]))
        if needs_respawn:
            if self.sensor is not None:
                self.sensor.destroy()
                self.surface = None
            self.sensor = self._parent.get_world().spawn_actor(
                self.sensors[index][-1],
                self._camera_transforms[self.transform_index][0],
                attach_to=self._parent,
                attachment_type=self._camera_transforms[self.transform_index][1])
            # We need to pass the lambda a weak reference to self to avoid
            # circular reference.
            weak_self = weakref.ref(self)
            self.sensor.listen(lambda image: CameraManager._parse_image(weak_self, image))
        if notify:
            self.hud.notification(self.sensors[index][2])
        self.index = index

    def next_sensor(self):
        self.set_sensor(self.index + 1)

    def toggle_recording(self):
        self.recording = not self.recording
        self.hud.notification('Recording %s' % ('On' if self.recording else 'Off'))

    def render(self, display):
        if self.surface is not None:
            display.blit(self.surface, (0, 0))

    @staticmethod
    def _parse_image(weak_self, image):
        self = weak_self()
        if not self:
            return
        if self.sensors[self.index][0].startswith('sensor.lidar'):
            points = np.frombuffer(image.raw_data, dtype=np.dtype('f4'))
            points = np.reshape(points, (int(points.shape[0] / 4), 4))
            lidar_data = np.array(points[:, :2])
            lidar_data *= min(self.hud.dim) / (2.0 * self.lidar_range)
            lidar_data += (0.5 * self.hud.dim[0], 0.5 * self.hud.dim[1])
            lidar_data = np.fabs(lidar_data)  # pylint: disable=E1111
            lidar_data = lidar_data.astype(np.int32)
            lidar_data = np.reshape(lidar_data, (-1, 2))
            lidar_img_size = (self.hud.dim[0], self.hud.dim[1], 3)
            lidar_img = np.zeros((lidar_img_size), dtype=np.uint8)
            lidar_img[tuple(lidar_data.T)] = (255, 255, 255)
            self.surface = pygame.surfarray.make_surface(lidar_img)
        elif self.sensors[self.index][0].startswith('sensor.camera.dvs'):
            # Example of converting the raw_data from a carla.DVSEventArray
            # sensor into a NumPy array and using it as an image
            dvs_events = np.frombuffer(image.raw_data, dtype=np.dtype([
                ('x', np.uint16), ('y', np.uint16), ('t', np.int64), ('pol', np.bool)]))
            dvs_img = np.zeros((image.height, image.width, 3), dtype=np.uint8)
            # Blue is positive, red is negative
            dvs_img[dvs_events[:]['y'], dvs_events[:]['x'], dvs_events[:]['pol'] * 2] = 255
            self.surface = pygame.surfarray.make_surface(dvs_img.swapaxes(0, 1))
        else:
            image.convert(self.sensors[self.index][1])
            array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
            array = np.reshape(array, (image.height, image.width, 4))
            array = array[:, :, :3]
            array = array[:, :, ::-1]
            self.surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))
        if self.recording:
            image.save_to_disk('_out/%08d' % image.frame)


# ==============================================================================
# -- game_loop() ---------------------------------------------------------------
# ==============================================================================


def game_loop(args):
    pygame.init()
    pygame.font.init()
    world = None

    try:
        client = carla.Client(args.host, args.port)
        client.set_timeout(200.0)

        display = pygame.display.set_mode(
            (args.width, args.height),
            pygame.HWSURFACE | pygame.DOUBLEBUF)

        hud = HUD(args.width, args.height)
        world = World(client.get_world(), hud, args)
        controller = KeyboardControl(world, args.autopilot)

        clock = pygame.time.Clock()
        while True:
            clock.tick_busy_loop(60)
            if controller.parse_events(client, world, clock):
                return
            world.tick(clock)
            world.render(display)
            pygame.display.flip()

    finally:

        if (world and world.recording_enabled):
            client.stop_recorder()

        if world is not None:
            world.destroy()

        pygame.quit()


# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================


def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Manual Control Client')
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='print debug information')
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
        '-a', '--autopilot',
        action='store_true',
        help='enable autopilot')
    argparser.add_argument(
        '--res',
        metavar='WIDTHxHEIGHT',
        default='1280x720',
        help='window resolution (default: 1280x720)')
    argparser.add_argument(
        '--filter',
        metavar='PATTERN',
        default='vehicle.*',
        help='actor filter (default: "vehicle.*")')
    argparser.add_argument(
        '--rolename',
        metavar='NAME',
        default='hero',
        help='actor role name (default: "hero")')
    argparser.add_argument(
        '--gamma',
        default=2.2,
        type=float,
        help='Gamma correction of the camera (default: 2.2)')
    args = argparser.parse_args()

    args.width, args.height = [int(x) for x in args.res.split('x')]

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)

    print(__doc__)

    try:

        game_loop(args)

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')


if __name__ == '__main__':

    main()
