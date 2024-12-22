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

    O            : open/close all doors of vehicle
    T            : toggle vehicle's telemetry

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


# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================


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
    from pygame.locals import K_f
    from pygame.locals import K_g
    from pygame.locals import K_h
    from pygame.locals import K_i
    from pygame.locals import K_l
    from pygame.locals import K_m
    from pygame.locals import K_n
    from pygame.locals import K_o
    from pygame.locals import K_p
    from pygame.locals import K_q
    from pygame.locals import K_r
    from pygame.locals import K_s
    from pygame.locals import K_t
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

# 定义一个函数，用于查找天气预设
def find_weather_presets():
    # 编译一个正则表达式，用于分割命名的字符串
    rgx = re.compile('.+?(?:(?<=[a-z])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])|$)')
    # 定义一个lambda函数，用于将字符串转换为带空格的标题格式
    name = lambda x: ' '.join(m.group(0) for m in rgx.finditer(x))
    # 获取carla.WeatherParameters中所有以大写字母开头的属性名
    presets = [x for x in dir(carla.WeatherParameters) if re.match('[A-Z].+', x)]
    # 返回一个列表，包含天气预设的值和它们的名称
    return [(getattr(carla.WeatherParameters, x), name(x)) for x in presets]

# 定义一个函数，用于actor获取的显示名称
def get_actor_display_name(actor, truncate=250):
    # 将actor的type_id属性从下划线分隔转换为点分隔，并转换为标题格式
    name = ' '.join(actor.type_id.replace('_', '.').title().split('.')[1:])
    # 如果名称长度超过截断值，则截断并添加省略号
    return (name[:truncate - 1] + u'\u2026') if len(name) > truncate else name

# 定义一个函数，用于获取actor蓝图
def get_actor_blueprints(world, filter, generation):
    # 从word中获取蓝图库，并根据过滤器过滤蓝图
    bps = world.get_blueprint_library().filter(filter)

    # 如果generation参数为"all"，则返回所有过滤后的蓝图
    if generation.lower() == "all":
        return bps

    # If the filter returns only one bp, we assume that this one needed 如果过滤后的蓝图只有一个，我们假设这是需要的蓝图，忽略generation参数
    # and therefore, we ignore the generation
    if len(bps) == 1:
        return bps
#如果bps的长度等于1，就返回bps
    try:
        int_generation = int(generation)
        # Check if generation is in available generations检查generation是否在可用的代数中
        if int_generation in [1, 2, 3]:
            # 过滤出对应代数的蓝图
            bps = [x for x in bps if int(x.get_attribute('generation')) == int_generation]
            return bps
        else:
            # 如果generation无效，打印警告信息并返回空列表
            print("   Warning! Actor Generation is not valid. No actor will be spawned.")
            return []
    except:
        # 如果generation转换为整数失败，打印警告信息并返回空列表
        print("   Warning! Actor Generation is not valid. No actor will be spawned.")
        return []


# ==============================================================================
# -- World ---------------------------------------------------------------------
# ==============================================================================


# 定义一个名为 World 的类，用于封装和操作CARLA仿真世界
class World(object):
    # 类的构造函数，初始化 World 类的实例
    def __init__(self, carla_world, hud, args):
        # 将传入的 CARLA 世界对象保存为实例变量
        self.world = carla_world
        # 从参数中获取同步模式设置，并保存为实例变量
        self.sync = args.sync
        # 从参数中获取角色名称，并保存为实例变量
        self.actor_role_name = args.rolename
        try:
            # 尝试从 CARLA 世界对象中获取地图对象，并保存为实例变量
            self.map = self.world.get_map()
        except RuntimeError as error:
            # 如果在获取地图时发生运行时错误，打印错误信息
            print('RuntimeError: {}'.format(error))
            # 提示用户检查 OpenDRIVE (.xodr) 文件是否存在、命名是否正确以及文件是否正确
            print('  The server could not send the OpenDRIVE (.xodr) file:')
            print('  Make sure it exists, has the same name of your town, and is correct.')
            # 退出程序
            sys.exit(1)
        self.hud = hud
        self.player = None
        self.collision_sensor = None
        self.lane_invasion_sensor = None
        self.gnss_sensor = None
        self.imu_sensor = None
        self.radar_sensor = None
        self.camera_manager = None
        self._weather_presets = find_weather_presets()
        self._weather_index = 0
        self._actor_filter = args.filter
        self._actor_generation = args.generation
        self._gamma = args.gamma
        self.restart()
        self.world.on_tick(hud.on_world_tick)
        self.recording_enabled = False
        self.recording_start = 0
        self.constant_velocity_enabled = False
        self.show_vehicle_telemetry = False
        self.doors_are_open = False
        self.current_map_layer = 0
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
        """
           重启相关操作的函数，主要完成以下功能：
           1. 重置玩家最大速度相关属性。
           2. 根据给定条件获取一个随机的蓝图（blueprint），并对蓝图的一些属性进行设置。
           3. 销毁并重新生成玩家角色，设置其位置、旋转等信息，并对其物理属性进行修改。
           4. 设置各类传感器，如碰撞传感器、车道入侵传感器、全球导航卫星系统传感器、惯性测量单元传感器、相机管理相关传感器等。
           5. 根据同步状态决定世界（world）的时间推进方式（tick或wait_for_tick）。
           """
        # 重置玩家常规最大速度属性
        self.player_max_speed = 1.589
        # 重置玩家快速最大速度属性
        self.player_max_speed_fast = 3.713
        # 如果相机管理器（camera_manager）存在，则保留其索引和变换索引，否则使用默认值0
        cam_index = self.camera_manager.index if self.camera_manager is not None else 0
        cam_pos_index = self.camera_manager.transform_index if self.camera_manager is not None else 0
        # 获取符合条件的演员蓝图列表
        blueprint_list = get_actor_blueprints(self.world, self._actor_filter, self._actor_generation)
        if not blueprint_list:
            # 如果没有找到符合过滤器条件的蓝图，则抛出异常
            raise ValueError("Couldn't find any blueprints with the specified filters")
        # 从蓝图列表中随机选择一个蓝图
        blueprint = random.choice(blueprint_list)
        # 设置蓝图的角色名称属性
        blueprint.set_attribute('role_name', self.actor_role_name)
        # 如果蓝图有地形力学（terramechanics）属性，则设置为'true'
        if blueprint.has_attribute('terramechanics'):
            blueprint.set_attribute('terramechanics', 'true')
            # 如果蓝图有颜色（color）属性，则随机选择一个推荐的颜色值并设置
        if blueprint.has_attribute('color'):
            color = random.choice(blueprint.get_attribute('color').recommended_values)
            blueprint.set_attribute('color', color)
            # 如果蓝图有司机ID（driver_id）属性，则随机选择一个推荐的司机ID值并设置
        if blueprint.has_attribute('driver_id'):
            driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
            blueprint.set_attribute('driver_id', driver_id)
            # 如果蓝图有无敌（is_invincible）属性，则设置为'true'
        if blueprint.has_attribute('is_invincible'):
            blueprint.set_attribute('is_invincible', 'true')
        # 如果蓝图有速度（speed）属性，则根据推荐值重置玩家最大速度相关属性
        if blueprint.has_attribute('speed'):
            self.player_max_speed = float(blueprint.get_attribute('speed').recommended_values[1])
            self.player_max_speed_fast = float(blueprint.get_attribute('speed').recommended_values[2])

        # 生成玩家角色相关操作
        if self.player is not None:
            # 获取玩家当前的变换信息（位置、旋转等）
            spawn_point = self.player.get_transform()
            # 将生成点的Z坐标增加2.0，可能是为了调整高度
            spawn_point.location.z += 2.0
            # 重置滚动角（roll）为0.0
            spawn_point.rotation.roll = 0.0
            # 重置俯仰角（pitch）为0.0
            spawn_point.rotation.pitch = 0.0
            # 销毁当前玩家
            self.destroy()
            # 使用新的蓝图和调整后的生成点尝试生成玩家角色
            self.player = self.world.try_spawn_actor(blueprint, spawn_point)
            # 设置车辆遥测相关显示属性为False
            self.show_vehicle_telemetry = False
            # 修改生成的玩家角色（车辆）的物理属性
            self.modify_vehicle_physics(self.player)
            # 如果玩家角色生成失败，则循环尝试在可用的生成点生成玩家
        while self.player is None:
            # 获取地图的生成点列表
            if not self.map.get_spawn_points():
                print('There are no spawn points available in your map/town.')
                print('Please add some Vehicle Spawn Point to your UE4 scene.')
                sys.exit(1)
            spawn_points = self.map.get_spawn_points()
            # 随机选择一个生成点，如果有生成点的话，否则使用默认的变换信息
            spawn_point = random.choice(spawn_points) if spawn_points else carla.Transform()
            self.player = self.world.try_spawn_actor(blueprint, spawn_point)
            self.show_vehicle_telemetry = False
            self.modify_vehicle_physics(self.player)
        # 设置各类传感器
        self.collision_sensor = CollisionSensor(self.player, self.hud)
        self.lane_invasion_sensor = LaneInvasionSensor(self.player, self.hud)
        self.gnss_sensor = GnssSensor(self.player)
        self.imu_sensor = IMUSensor(self.player)
        self.camera_manager = CameraManager(self.player, self.hud, self._gamma)
        self.camera_manager.transform_index = cam_pos_index
        self.camera_manager.set_sensor(cam_index, notify=False)
        actor_type = get_actor_display_name(self.player)
        self.hud.notification(actor_type)
        # 根据是否同步决定世界的时间推进方式
        if self.sync:
            self.world.tick()
        else:
            self.world.wait_for_tick()

    def next_weather(self, reverse=False):
        """
          切换天气的函数，根据传入的reverse参数决定是向前还是向后切换天气预设。
          1. 调整天气索引。
          2. 根据索引获取对应的天气预设。
          3. 在界面显示切换后的天气信息，并在游戏世界中设置对应的天气。
          """
        # 根据reverse参数调整天气索引，实现正向或反向切换
        self._weather_index += -1 if reverse else 1
        # 确保天气索引在预设天气列表的范围内（取模操作）
        self._weather_index %= len(self._weather_presets)
        preset = self._weather_presets[self._weather_index]
        # 在界面显示切换后的天气信息
        self.hud.notification('Weather: %s' % preset[1])
        # 在游戏世界中设置对应的天气
        self.player.get_world().set_weather(preset[0])

    def next_map_layer(self, reverse=False):
        """
           切换地图图层的函数，根据传入的reverse参数决定是向前还是向后切换地图图层。
           1. 调整当前地图图层索引。
           2. 根据索引获取对应的地图图层名称。
           3. 在界面显示切换后的地图图层信息。
           """
        # 根据reverse参数调整当前地图图层索引，实现正向或反向切换
        self.current_map_layer += -1 if reverse else 1
        # 确保地图图层索引在地图图层名称列表的范围内（取模操作）
        self.current_map_layer %= len(self.map_layer_names)
        selected = self.map_layer_names[self.current_map_layer]
        # 在界面显示切换后的地图图层信息
        self.hud.notification('LayerMap selected: %s' % selected)

    def load_map_layer(self, unload=False):
        """
          加载或卸载地图图层的函数，根据传入的unload参数决定是加载还是卸载当前选中的地图图层。
          1. 获取当前选中的地图图层名称。
          2. 根据unload参数决定执行加载还是卸载操作，并在界面显示相应的提示信息。
          """
        selected = self.map_layer_names[self.current_map_layer]
        if unload:
            # 如果是卸载操作，显示卸载提示信息，并执行卸载地图图层的操作
            self.hud.notification('Unloading map layer: %s' % selected)
            self.world.unload_map_layer(selected)
        else:
            # 如果是加载操作，显示加载提示信息，并执行加载地图图层的操作
            self.hud.notification('Loading map layer: %s' % selected)
            self.world.load_map_layer(selected)

    def toggle_radar(self):
        """
           切换雷达传感器的函数，实现雷达传感器的创建和销毁操作，用于控制雷达传感器的启用和禁用。
           如果雷达传感器不存在，则创建一个；如果存在且其传感器实体存在，则销毁该传感器实体，并将雷达传感器设置为None。
           """
        if self.radar_sensor is None:
            self.radar_sensor = RadarSensor(self.player)
        elif self.radar_sensor.sensor is not None:
            self.radar_sensor.sensor.destroy()
            self.radar_sensor = None

    def modify_vehicle_physics(self, actor):
        """
           修改车辆物理属性的函数，尝试获取演员（通常是车辆）的物理控制对象，
           并设置其使用扫掠轮碰撞（use_sweep_wheel_collision）属性为True，然后应用新的物理控制设置到演员上。
           如果出现异常则直接跳过（可能是演员不是车辆等不符合操作条件的情况）。
           """
        try:
            physics_control = actor.get_physics_control()
            physics_control.use_sweep_wheel_collision = True
            actor.apply_physics_control(physics_control)
        except Exception:
            pass

    def tick(self, clock):
        """
          每帧更新相关操作的函数，调用HUD（ Heads-Up Display，通常是游戏界面显示相关）的tick方法，传入自身和时钟参数，
          用于更新游戏界面等相关信息，一般在游戏循环中按帧调用。
          """
        self.hud.tick(self, clock)

    def render(self, display):
        """
            渲染相关操作的函数，调用相机管理器（camera_manager）和HUD的渲染方法，传入显示对象（可能是屏幕等渲染目标），
            用于将游戏画面渲染到指定的显示目标上。
            """
        self.camera_manager.render(display)
        self.hud.render(display)

    def destroy_sensors(self):
        """
           销毁传感器相关操作的函数，主要用于销毁相机管理器中的传感器，
           并将其相关属性（传感器对象、索引等）设置为None，释放相关资源。
           """
        self.camera_manager.sensor.destroy()
        self.camera_manager.sensor = None
        self.camera_manager.index = None

    def destroy(self):
        """
           销毁相关操作的函数，用于销毁游戏中的各种资源，包括雷达传感器（如果存在）、
           各类传感器（如碰撞、车道入侵、全球导航卫星系统、惯性测量单元等传感器）以及玩家角色（如果存在）。
           具体操作是先通过toggle_radar方法处理雷达传感器，然后遍历传感器列表，停止并销毁每个传感器，最后销毁玩家角色。
           """
        if self.radar_sensor is not None:
            self.toggle_radar()
        sensors = [
            self.camera_manager.sensor,
            self.collision_sensor.sensor,
            self.lane_invasion_sensor.sensor,
            self.gnss_sensor.sensor,
            self.imu_sensor.sensor]
        for sensor in sensors:
            if sensor is not None:
                sensor.stop()
                sensor.destroy()
        if self.player is not None:
            self.player.destroy()


# ==============================================================================
# -- KeyboardControl -----------------------------------------------------------
# ==============================================================================


class KeyboardControl(object):
    """Class that handles keyboard input."""
    # 定义了一个名为 `KeyboardControl` 的类，它继承自 `object` 类（在Python 3中，默认继承自 `object`，可不显式写出，但这里明确写出了），从类的文档字符串可知，这个类的主要作用是处理键盘输入相关的操作，用于在程序中响应用户通过键盘进行的各种交互操作。

    def __init__(self, world, start_in_autopilot):
        # 这是类的构造函数（初始化方法），在创建 `KeyboardControl` 类的实例时会被调用，用于初始化实例的各种属性。
        # 参数 `world` 应该是代表整个模拟世界的相关对象，包含了场景中的各种元素（如车辆、角色等）以及相关的状态信息等，通过它可以访问和操作世界中的内容。
        # 参数 `start_in_autopilot` 是一个布尔值，用于指示是否在一开始就启用自动驾驶模式。

        self._autopilot_enabled = start_in_autopilot
        # 将传入的 `start_in_autopilot` 参数值赋给实例属性 `_autopilot_enabled`，用于记录当前是否启用了自动驾驶模式，以下划线开头的属性通常表示是类内部使用的“私有”属性（在Python中其实并没有真正的私有属性概念，只是一种约定俗成的标识）。

        self._ackermann_enabled = False
        # 初始化实例属性 `_ackermann_enabled` 为 `False`，从变量名推测可能用于标记是否启用阿克曼转向相关的某种功能（阿克曼转向常用于车辆转向模型等场景），后续代码应该会根据这个属性的值来决定是否执行相应的阿克曼转向控制逻辑。

        self._ackermann_reverse = 1
        # 初始化实例属性 `_ackermann_reverse` 为 `1`，同样结合名称推测可能与阿克曼转向在倒车等反向操作时的相关参数设置有关，具体作用需结合后续代码中对它的使用来确定。

        if isinstance(world.player, carla.Vehicle):
            # 判断 `world.player` 是否是 `carla.Vehicle` 类型，即判断当前模拟世界中的主角（`player`）是否是车辆，如果是车辆，则进行以下相关的初始化操作。
            self._control = carla.VehicleControl()
            # 创建一个 `carla.VehicleControl` 类的实例，用于后续控制车辆的各种行为（如油门、刹车、转向等操作），这个实例将保存车辆控制相关的参数设置。

            self._ackermann_control = carla.VehicleAckermannControl()
            # 创建一个 `carla.VehicleAckermannControl` 类的实例，用于涉及阿克曼转向相关的更具体的车辆控制操作，可能在更精准的车辆操控场景下会使用到这个实例来设置控制参数。

            self._lights = carla.VehicleLightState.NONE
            # 初始化车辆灯光状态为 `NONE`，也就是所有灯光都关闭的初始状态，后续代码可以根据用户的键盘操作来改变这个灯光状态，开启或关闭不同的车辆灯光。

            world.player.set_autopilot(self._autopilot_enabled)
            # 通过 `world.player`（即模拟世界中的车辆对象）调用 `set_autopilot` 方法，将车辆的自动驾驶模式设置为之前初始化的 `_autopilot_enabled` 所表示的状态，即根据传入的参数决定车辆一开始是否启用自动驾驶。

            world.player.set_light_state(self._lights)
            # 通过 `world.player` 调用 `set_light_state` 方法，将车辆的灯光状态设置为当前初始化的 `_lights` 所表示的状态，也就是一开始将车辆灯光全部关闭。

        elif isinstance(world.player, carla.Walker):
            # 如果 `world.player` 不是车辆，而是 `carla.Walker` 类型（推测 `carla.Walker` 表示模拟世界中的行人等可移动角色），则进行以下针对行人角色的初始化操作。
            self._control = carla.WalkerControl()
            # 创建一个 `carla.WalkerControl` 类的实例，用于控制行人角色的移动等行为，类似车辆的 `carla.VehicleControl`，这个实例会保存行人控制相关的参数。

            self._autopilot_enabled = False
            # 对于行人角色，将自动驾驶模式设置为 `False`，因为通常行人不需要自动驾驶功能，这里明确将其禁用。

            self._rotation = world.player.get_transform().rotation
            # 获取行人当前的旋转角度信息（通过 `get_transform` 方法获取变换信息，再从中获取旋转角度），并保存到实例属性 `_rotation` 中，可能在后续控制行人移动方向等操作时会用到这个初始的旋转角度信息。

        else:
            raise NotImplementedError("Actor type not supported")
            # 如果 `world.player` 既不是车辆也不是行人角色，说明当前代码不支持这种类型的角色作为模拟世界的主角，那么就抛出 `NotImplementedError` 异常，并提示相应的错误信息，表示该角色类型不受支持。

        self._steer_cache = 0.0
        # 初始化一个名为 `_steer_cache` 的实例属性为 `0.0`，从名称推测可能用于缓存车辆或角色转向相关的数据，也许是为了平滑转向操作或者记录上一次的转向状态等，具体作用要看后续代码对它的使用情况。

        world.hud.notification("Press 'H' or '?' for help.", seconds=4.0)
        # 通过 `world.hud`（推测是模拟世界中的抬头显示相关对象）调用 `notification` 方法，在界面上显示一条提示信息，提示用户按下 `H` 或 `?` 键可以获取帮助信息，并且这个提示信息会显示 `4.0` 秒的时间。

    def parse_events(self, client, world, clock, sync_mode):
        if isinstance(self._control, carla.VehicleControl):
            current_lights = self._lights
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return True
            elif event.type == pygame.KEYUP:
                if self._is_quit_shortcut(event.key):
                    return True
                elif event.key == K_BACKSPACE:
                    if self._autopilot_enabled:
                        world.player.set_autopilot(False)
                        world.restart()
                        world.player.set_autopilot(True)
                    else:
                        world.restart()
                elif event.key == K_F1:
                    world.hud.toggle_info()
                elif event.key == K_v and pygame.key.get_mods() & KMOD_SHIFT:
                    world.next_map_layer(reverse=True)
                elif event.key == K_v:
                    world.next_map_layer()
                elif event.key == K_b and pygame.key.get_mods() & KMOD_SHIFT:
                    world.load_map_layer(unload=True)
                elif event.key == K_b:
                    world.load_map_layer()
                elif event.key == K_h or (event.key == K_SLASH and pygame.key.get_mods() & KMOD_SHIFT):
                    world.hud.help.toggle()
                elif event.key == K_TAB:
                    world.camera_manager.toggle_camera()
                elif event.key == K_c and pygame.key.get_mods() & KMOD_SHIFT:
                    world.next_weather(reverse=True)
                elif event.key == K_c:
                    world.next_weather()
                elif event.key == K_g:
                    world.toggle_radar()
                elif event.key == K_BACKQUOTE:
                    world.camera_manager.next_sensor()
                elif event.key == K_n:
                    world.camera_manager.next_sensor()
                elif event.key == K_w and (pygame.key.get_mods() & KMOD_CTRL):
                    if world.constant_velocity_enabled:
                        world.player.disable_constant_velocity()
                        world.constant_velocity_enabled = False
                        world.hud.notification("Disabled Constant Velocity Mode")
                    else:
                        world.player.enable_constant_velocity(carla.Vector3D(17, 0, 0))
                        world.constant_velocity_enabled = True
                        world.hud.notification("Enabled Constant Velocity Mode at 60 km/h")
                elif event.key == K_o:
                    try:
                        if world.doors_are_open:
                            world.hud.notification("Closing Doors")
                            world.doors_are_open = False
                            world.player.close_door(carla.VehicleDoor.All)
                        else:
                            world.hud.notification("Opening doors")
                            world.doors_are_open = True
                            world.player.open_door(carla.VehicleDoor.All)
                    except Exception:
                        pass
                elif event.key == K_t:
                    if world.show_vehicle_telemetry:
                        world.player.show_debug_telemetry(False)
                        world.show_vehicle_telemetry = False
                        world.hud.notification("Disabled Vehicle Telemetry")
                    else:
                        try:
                            world.player.show_debug_telemetry(True)
                            world.show_vehicle_telemetry = True
                            world.hud.notification("Enabled Vehicle Telemetry")
                        except Exception:
                            pass
                elif event.key > K_0 and event.key <= K_9:
                    index_ctrl = 0
                    if pygame.key.get_mods() & KMOD_CTRL:
                        index_ctrl = 9
                    world.camera_manager.set_sensor(event.key - 1 - K_0 + index_ctrl)
                elif event.key == K_r and not (pygame.key.get_mods() & KMOD_CTRL):
                    world.camera_manager.toggle_recording()
                elif event.key == K_r and (pygame.key.get_mods() & KMOD_CTRL):
                    if (world.recording_enabled):
                        client.stop_recorder()
                        world.recording_enabled = False
                        world.hud.notification("Recorder is OFF")
                    else:
                        client.start_recorder("manual_recording.rec")
                        world.recording_enabled = True
                        world.hud.notification("Recorder is ON")
                elif event.key == K_p and (pygame.key.get_mods() & KMOD_CTRL):
                    # stop recorder
                    client.stop_recorder()
                    world.recording_enabled = False
                    # work around to fix camera at start of replaying
                    current_index = world.camera_manager.index
                    world.destroy_sensors()
                    # disable autopilot
                    self._autopilot_enabled = False
                    world.player.set_autopilot(self._autopilot_enabled)
                    world.hud.notification("Replaying file 'manual_recording.rec'")
                    # replayer
                    client.replay_file("manual_recording.rec", world.recording_start, 0, 0)
                    world.camera_manager.set_sensor(current_index)
                elif event.key == K_MINUS and (pygame.key.get_mods() & KMOD_CTRL):
                    if pygame.key.get_mods() & KMOD_SHIFT:
                        world.recording_start -= 10
                    else:
                        world.recording_start -= 1
                    world.hud.notification("Recording start time is %d" % (world.recording_start))
                elif event.key == K_EQUALS and (pygame.key.get_mods() & KMOD_CTRL):
                    if pygame.key.get_mods() & KMOD_SHIFT:
                        world.recording_start += 10
                    else:
                        world.recording_start += 1
                    world.hud.notification("Recording start time is %d" % (world.recording_start))
                if isinstance(self._control, carla.VehicleControl):
                    if event.key == K_f:
                        # Toggle ackermann controller
                        self._ackermann_enabled = not self._ackermann_enabled
                        world.hud.show_ackermann_info(self._ackermann_enabled)
                        world.hud.notification("Ackermann Controller %s" %
                                               ("Enabled" if self._ackermann_enabled else "Disabled"))
                    if event.key == K_q:
                        if not self._ackermann_enabled:
                            self._control.gear = 1 if self._control.reverse else -1
                        else:
                            self._ackermann_reverse *= -1
                            # Reset ackermann control
                            self._ackermann_control = carla.VehicleAckermannControl()
                    elif event.key == K_m:
                        self._control.manual_gear_shift = not self._control.manual_gear_shift
                        self._control.gear = world.player.get_control().gear
                        world.hud.notification('%s Transmission' %
                                               ('Manual' if self._control.manual_gear_shift else 'Automatic'))
                    elif self._control.manual_gear_shift and event.key == K_COMMA:
                        self._control.gear = max(-1, self._control.gear - 1)
                    elif self._control.manual_gear_shift and event.key == K_PERIOD:
                        self._control.gear = self._control.gear + 1
                    elif event.key == K_p and not pygame.key.get_mods() & KMOD_CTRL:
                        if not self._autopilot_enabled and not sync_mode:
                            print("WARNING: You are currently in asynchronous mode and could "
                                  "experience some issues with the traffic simulation")
                        self._autopilot_enabled = not self._autopilot_enabled
                        world.player.set_autopilot(self._autopilot_enabled)
                        world.hud.notification(
                            'Autopilot %s' % ('On' if self._autopilot_enabled else 'Off'))
                    elif event.key == K_l and pygame.key.get_mods() & KMOD_CTRL:
                        current_lights ^= carla.VehicleLightState.Special1
                    elif event.key == K_l and pygame.key.get_mods() & KMOD_SHIFT:
                        current_lights ^= carla.VehicleLightState.HighBeam
                    elif event.key == K_l:
                        # Use 'L' key to switch between lights:
                        # closed -> position -> low beam -> fog
                        if not self._lights & carla.VehicleLightState.Position:
                            world.hud.notification("Position lights")
                            current_lights |= carla.VehicleLightState.Position
                        else:
                            world.hud.notification("Low beam lights")
                            current_lights |= carla.VehicleLightState.LowBeam
                        if self._lights & carla.VehicleLightState.LowBeam:
                            world.hud.notification("Fog lights")
                            current_lights |= carla.VehicleLightState.Fog
                        if self._lights & carla.VehicleLightState.Fog:
                            world.hud.notification("Lights off")
                            current_lights ^= carla.VehicleLightState.Position
                            current_lights ^= carla.VehicleLightState.LowBeam
                            current_lights ^= carla.VehicleLightState.Fog
                    elif event.key == K_i:
                        current_lights ^= carla.VehicleLightState.Interior
                    elif event.key == K_z:
                        current_lights ^= carla.VehicleLightState.LeftBlinker
                    elif event.key == K_x:
                        current_lights ^= carla.VehicleLightState.RightBlinker

        if not self._autopilot_enabled:
            if isinstance(self._control, carla.VehicleControl):
                self._parse_vehicle_keys(pygame.key.get_pressed(), clock.get_time())
                self._control.reverse = self._control.gear < 0
                # Set automatic control-related vehicle lights
                if self._control.brake:
                    current_lights |= carla.VehicleLightState.Brake
                else: # Remove the Brake flag
                    current_lights &= ~carla.VehicleLightState.Brake
                if self._control.reverse:
                    current_lights |= carla.VehicleLightState.Reverse
                else: # Remove the Reverse flag
                    current_lights &= ~carla.VehicleLightState.Reverse
                if current_lights != self._lights: # Change the light state only if necessary
                    self._lights = current_lights
                    world.player.set_light_state(carla.VehicleLightState(self._lights))
                # Apply control
                if not self._ackermann_enabled:
                    world.player.apply_control(self._control)
                else:
                    world.player.apply_ackermann_control(self._ackermann_control)
                    # Update control to the last one applied by the ackermann controller.
                    self._control = world.player.get_control()
                    # Update hud with the newest ackermann control
                    world.hud.update_ackermann_control(self._ackermann_control)

            elif isinstance(self._control, carla.WalkerControl):
                self._parse_walker_keys(pygame.key.get_pressed(), clock.get_time(), world)
                world.player.apply_control(self._control)

    def _parse_vehicle_keys(self, keys, milliseconds):
        if keys[K_UP] or keys[K_w]:
            if not self._ackermann_enabled:
                self._control.throttle = min(self._control.throttle + 0.1, 1.00)
            else:
                self._ackermann_control.speed += round(milliseconds * 0.005, 2) * self._ackermann_reverse
        else:
            if not self._ackermann_enabled:
                self._control.throttle = 0.0

        if keys[K_DOWN] or keys[K_s]:
            if not self._ackermann_enabled:
                self._control.brake = min(self._control.brake + 0.2, 1)
            else:
                self._ackermann_control.speed -= min(abs(self._ackermann_control.speed), round(milliseconds * 0.005, 2)) * self._ackermann_reverse
                self._ackermann_control.speed = max(0, abs(self._ackermann_control.speed)) * self._ackermann_reverse
        else:
            if not self._ackermann_enabled:
                self._control.brake = 0

        steer_increment = 5e-4 * milliseconds
        if keys[K_LEFT] or keys[K_a]:
            if self._steer_cache > 0:
                self._steer_cache = 0
            else:
                self._steer_cache -= steer_increment
        elif keys[K_RIGHT] or keys[K_d]:
            if self._steer_cache < 0:
                self._steer_cache = 0
            else:
                self._steer_cache += steer_increment
        else:
            self._steer_cache = 0.0
        self._steer_cache = min(0.7, max(-0.7, self._steer_cache))
        if not self._ackermann_enabled:
            self._control.steer = round(self._steer_cache, 1)
            self._control.hand_brake = keys[K_SPACE]
        else:
            self._ackermann_control.steer = round(self._steer_cache, 1)

    def _parse_walker_keys(self, keys, milliseconds, world):
        self._control.speed = 0.0
        if keys[K_DOWN] or keys[K_s]:
            self._control.speed = 0.0
        if keys[K_LEFT] or keys[K_a]:
            self._control.speed = .01
            self._rotation.yaw -= 0.08 * milliseconds
        if keys[K_RIGHT] or keys[K_d]:
            self._control.speed = .01
            self._rotation.yaw += 0.08 * milliseconds
        if keys[K_UP] or keys[K_w]:
            self._control.speed = world.player_max_speed_fast if pygame.key.get_mods() & KMOD_SHIFT else world.player_max_speed
        self._control.jump = keys[K_SPACE]
        self._rotation.yaw = round(self._rotation.yaw, 1)
        self._control.direction = self._rotation.get_forward_vector()

    @staticmethod
    def _is_quit_shortcut(key):
        return (key == K_ESCAPE) or (key == K_q and pygame.key.get_mods() & KMOD_CTRL)


# ==============================================================================
# -- HUD -----------------------------------------------------------------------
# ==============================================================================


class HUD(object):                       #定义了一个名为HUD的类，它继承自object类
    def __init__(self, width, height):
    #构造函数，用于初始化对象的属性，接受width和height两个参数，分别表示显示区域的宽度和高度
        self.dim = (width, height)
        #将传入的宽度和高度参数组合成一个元组，存储在self.dim属性中，用于表示显示区域的尺寸
        font = pygame.font.Font(pygame.font.get_default_font(), 20)
        #使用pygame.font.Font()函数创建一个字体对象，该字体使用默认字体，字号为 20
        font_name = 'courier' if os.name == 'nt' else 'mono'
        #根据操作系统类型选择字体名称
        fonts = [x for x in pygame.font.get_fonts() if font_name in x]
        #从pygame可用的字体列表中筛选出包含所选字体名称的字体
        default_font = 'ubuntumono'
        #设置默认字体为ubuntumono
        mono = default_font if default_font in fonts else fonts[0]
        #如果该字体在筛选后的字体列表中，则使用它，否则使用列表中的第一个字体。
        mono = pygame.font.match_font(mono)
        #通过pygame.font.match_font()函数查找与所选字体名称匹配的实际字体文件路径
        self._font_mono = pygame.font.Font(mono, 12 if os.name == 'nt' else 14)
        #用该路径创建一个字号为 12（Windows 系统）或 14（其他系统）的字体对象，存储在self._font_mono属性中
        self._notifications = FadingText(font, (width, 40), (0, height - 40))
        #创建一个FadingText类的实例，传入之前创建的默认字体、一个宽度为width、高度为 40 的尺寸元组以及一个位于显示区域底部左侧的位置元组作为参数
        self.help = HelpText(pygame.font.Font(mono, 16), width, height)
        #创建一个HelpText类的实例，传入字号为 16 的所选字体对象以及显示区域的宽度和高度

        self.server_fps = 0        #用于存储服务器的帧率，初始化为 0
        self.frame = 0             #用于记录当前帧的编号，初始化为 0
        self.simulation_time = 0   #用于记录模拟时间，初始化为 0
        self._show_info = True     #用于控制是否显示信息，初始化为True
        self._info_text = []       #初始化为一个空列表，用于存储要显示的信息文本
        self._server_clock = pygame.time.Clock()
        #创建一个pygame.time.Clock()对象，用于控制服务器端的时间，可用于控制帧率、计时等操作

        self._show_ackermann_info = False  #用于控制是否显示阿克曼转向信息，初始化为False
        self._ackermann_control = carla.VehicleAckermannControl()
        #用于存储和控制车辆的阿克曼转向相关信息 

    def on_world_tick(self, timestamp):    #定义了一个名为on_world_tick的实例方法，用于获取当前世界的时间戳等信息
        self._server_clock.tick()          #调用self._server_clock的tick方法
        self.server_fps = self._server_clock.get_fps()
        #通过self._server_clock的get_fps方法获取服务器的帧率，并将其赋值给self.server_fps属性
        self.frame = timestamp.frame       #将传入的时间戳中的frame属性赋值给self.frame，用于更新当前的帧编号
        self.simulation_time = timestamp.elapsed_seconds
        #将传入的时间戳中的elapsed_seconds属性赋值给self.simulation_time

    def tick(self, world, clock):
        self._notifications.tick(world, clock)
        if not self._show_info:
            return
        t = world.player.get_transform()
        v = world.player.get_velocity()
        c = world.player.get_control()
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
            'Vehicle: % 20s' % get_actor_display_name(world.player, truncate=20),
            'Map:     % 20s' % world.map.name.split('/')[-1],
            'Simulation time: % 12s' % datetime.timedelta(seconds=int(self.simulation_time)),
            '',
            'Speed:   % 15.0f km/h' % (3.6 * math.sqrt(v.x**2 + v.y**2 + v.z**2)),
            u'Compass:% 17.0f\N{DEGREE SIGN} % 2s' % (compass, heading),
            'Accelero: (%5.1f,%5.1f,%5.1f)' % (world.imu_sensor.accelerometer),
            'Gyroscop: (%5.1f,%5.1f,%5.1f)' % (world.imu_sensor.gyroscope),
            'Location:% 20s' % ('(% 5.1f, % 5.1f)' % (t.location.x, t.location.y)),
            'GNSS:% 24s' % ('(% 2.6f, % 3.6f)' % (world.gnss_sensor.lat, world.gnss_sensor.lon)),
            'Height:  % 18.0f m' % t.location.z,
            '']
        if isinstance(c, carla.VehicleControl):
            self._info_text += [
                ('Throttle:', c.throttle, 0.0, 1.0),
                ('Steer:', c.steer, -1.0, 1.0),
                ('Brake:', c.brake, 0.0, 1.0),
                ('Reverse:', c.reverse),
                ('Hand brake:', c.hand_brake),
                ('Manual:', c.manual_gear_shift),
                'Gear:        %s' % {-1: 'R', 0: 'N'}.get(c.gear, c.gear)]
            if self._show_ackermann_info:
                self._info_text += [
                    '',
                    'Ackermann Controller:',
                    '  Target speed: % 8.0f km/h' % (3.6*self._ackermann_control.speed),
                ]
        elif isinstance(c, carla.WalkerControl):
            self._info_text += [
                ('Speed:', c.speed, 0.0, 5.556),
                ('Jump:', c.jump)]
        self._info_text += [
            '',
            'Collision:',
            collision,
            '',
            'Number of vehicles: % 8d' % len(vehicles)]
        if len(vehicles) > 1:
            self._info_text += ['Nearby vehicles:']
            distance = lambda l: math.sqrt((l.x - t.location.x)**2 + (l.y - t.location.y)**2 + (l.z - t.location.z)**2)
            vehicles = [(distance(x.get_location()), x) for x in vehicles if x.id != world.player.id]
            for d, vehicle in sorted(vehicles, key=lambda vehicles: vehicles[0]):
                if d > 200.0:
                    break
                vehicle_type = get_actor_display_name(vehicle, truncate=22)
                self._info_text.append('% 4dm %s' % (d, vehicle_type))

    def show_ackermann_info(self, enabled):
        self._show_ackermann_info = enabled

    def update_ackermann_control(self, ackermann_control):
        self._ackermann_control = ackermann_control

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

        # If the spawn object is not a vehicle, we cannot use the Lane Invasion Sensor
        if parent_actor.type_id.startswith("vehicle."):
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
        bound_x = 0.5 + self._parent.bounding_box.extent.x
        bound_y = 0.5 + self._parent.bounding_box.extent.y
        bound_z = 0.5 + self._parent.bounding_box.extent.z

        self.velocity_range = 7.5 # m/s
        world = self._parent.get_world()
        self.debug = world.debug
        bp = world.get_blueprint_library().find('sensor.other.radar')
        bp.set_attribute('horizontal_fov', str(35))
        bp.set_attribute('vertical_fov', str(20))
        self.sensor = world.spawn_actor(
            bp,
            carla.Transform(
                carla.Location(x=bound_x + 0.05, z=bound_z+0.05),
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
        bound_x = 0.5 + self._parent.bounding_box.extent.x
        bound_y = 0.5 + self._parent.bounding_box.extent.y
        bound_z = 0.5 + self._parent.bounding_box.extent.z
        Attachment = carla.AttachmentType

        if not self._parent.type_id.startswith("walker.pedestrian"):
            self._camera_transforms = [
                (carla.Transform(carla.Location(x=-2.0*bound_x, y=+0.0*bound_y, z=2.0*bound_z), carla.Rotation(pitch=8.0)), Attachment.SpringArmGhost),
                (carla.Transform(carla.Location(x=+0.8*bound_x, y=+0.0*bound_y, z=1.3*bound_z)), Attachment.Rigid),
                (carla.Transform(carla.Location(x=+1.9*bound_x, y=+1.0*bound_y, z=1.2*bound_z)), Attachment.SpringArmGhost),
                (carla.Transform(carla.Location(x=-2.8*bound_x, y=+0.0*bound_y, z=4.6*bound_z), carla.Rotation(pitch=6.0)), Attachment.SpringArmGhost),
                (carla.Transform(carla.Location(x=-1.0, y=-1.0*bound_y, z=0.4*bound_z)), Attachment.Rigid)]
        else:
            self._camera_transforms = [
                (carla.Transform(carla.Location(x=-2.5, z=0.0), carla.Rotation(pitch=-8.0)), Attachment.SpringArmGhost),
                (carla.Transform(carla.Location(x=1.6, z=1.7)), Attachment.Rigid),
                (carla.Transform(carla.Location(x=2.5, y=0.5, z=0.0), carla.Rotation(pitch=-8.0)), Attachment.SpringArmGhost),
                (carla.Transform(carla.Location(x=-4.0, z=2.0), carla.Rotation(pitch=6.0)), Attachment.SpringArmGhost),
                (carla.Transform(carla.Location(x=0, y=-2.5, z=-0.0), carla.Rotation(yaw=90.0)), Attachment.Rigid)]

        self.transform_index = 1
        self.sensors = [
            ['sensor.camera.rgb', cc.Raw, 'Camera RGB', {}],
            ['sensor.camera.depth', cc.Raw, 'Camera Depth (Raw)', {}],
            ['sensor.camera.depth', cc.Depth, 'Camera Depth (Gray Scale)', {}],
            ['sensor.camera.depth', cc.LogarithmicDepth, 'Camera Depth (Logarithmic Gray Scale)', {}],
            ['sensor.camera.semantic_segmentation', cc.Raw, 'Camera Semantic Segmentation (Raw)', {}],
            ['sensor.camera.semantic_segmentation', cc.CityScapesPalette, 'Camera Semantic Segmentation (CityScapes Palette)', {}],
            ['sensor.camera.instance_segmentation', cc.CityScapesPalette, 'Camera Instance Segmentation (CityScapes Palette)', {}],
            ['sensor.camera.instance_segmentation', cc.Raw, 'Camera Instance Segmentation (Raw)', {}],
            ['sensor.lidar.ray_cast', None, 'Lidar (Ray-Cast)', {'range': '50'}],
            ['sensor.camera.dvs', cc.Raw, 'Dynamic Vision Sensor', {}],
            ['sensor.camera.rgb', cc.Raw, 'Camera RGB Distorted',
                {'lens_circle_multiplier': '3.0',
                'lens_circle_falloff': '3.0',
                'chromatic_aberration_intensity': '0.5',
                'chromatic_aberration_offset': '0'}],
            ['sensor.camera.optical_flow', cc.Raw, 'Optical Flow', {}],
            ['sensor.camera.normals', cc.Raw, 'Camera Normals', {}],
        ]
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
        elif self.sensors[self.index][0].startswith('sensor.camera.optical_flow'):
            image = image.get_color_coded_flow()
            array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
            array = np.reshape(array, (image.height, image.width, 4))
            array = array[:, :, :3]
            array = array[:, :, ::-1]
            self.surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))
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
    original_settings = None

    try:
        client = carla.Client(args.host, args.port)
        client.set_timeout(2000.0)

        sim_world = client.get_world()
        if args.sync:
            original_settings = sim_world.get_settings()
            settings = sim_world.get_settings()
            if not settings.synchronous_mode:
                settings.synchronous_mode = True
                settings.fixed_delta_seconds = 0.05
            sim_world.apply_settings(settings)

            traffic_manager = client.get_trafficmanager()
            traffic_manager.set_synchronous_mode(True)

        if args.autopilot and not sim_world.get_settings().synchronous_mode:
            print("WARNING: You are currently in asynchronous mode and could "
                  "experience some issues with the traffic simulation")

        display = pygame.display.set_mode(
            (args.width, args.height),
            pygame.HWSURFACE | pygame.DOUBLEBUF)
        display.fill((0,0,0))
        pygame.display.flip()

        hud = HUD(args.width, args.height)
        world = World(sim_world, hud, args)
        controller = KeyboardControl(world, args.autopilot)

        if args.sync:
            sim_world.tick()
        else:
            sim_world.wait_for_tick()

        clock = pygame.time.Clock()
        while True:
            if args.sync:
                sim_world.tick()
            clock.tick_busy_loop(60)
            if controller.parse_events(client, world, clock, args.sync):
                return
            world.tick(clock)
            world.render(display)
            pygame.display.flip()

    finally:

        if original_settings:
            sim_world.apply_settings(original_settings)

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
        '--generation',
        metavar='G',
        default='2',
        help='restrict to certain actor generation (values: "1","2","All" - default: "2")')
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
    argparser.add_argument(
        '--sync',
        action='store_true',
        help='Activate synchronous mode execution')
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
