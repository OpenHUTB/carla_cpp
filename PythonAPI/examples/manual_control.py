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
    """
    函数功能：用于解析通过 `pygame` 接收到的各类事件，根据不同的键盘按键操作（松开事件 `KEYUP`），
    执行与模拟世界（`world`）相关的各种交互行为，例如控制车辆行驶状态、切换地图、改变天气、操作车辆灯光等，
    同时依据当前是否处于自动驾驶模式（`_autopilot_enabled`）以及是否为同步模式（`sync_mode`）等条件，
    决定是否将相应控制操作应用到模拟世界中的角色（车辆或行人）上。

    参数说明：
    - `self`：类的实例对象本身，通过它可以访问类的实例属性等信息。
    - `client`：与CARLA模拟器进行通信的客户端对象，用于执行一些需要和模拟器服务端交互的操作（如启动/停止录制等）。
    - `world`：代表整个模拟世界的对象，包含了场景中的各种元素（车辆、行人、地图、天气等）以及相关的操作方法，用于操作和改变世界中的各种状态。
    - `clock`：可能是用于记录时间相关信息的对象，例如获取当前时间等，用于一些操作中基于时间的判断或者控制频率等方面的需求。
    - `sync_mode`：一个布尔值，表示是否处于同步模式，在某些操作（如切换自动驾驶状态）时可能需要根据这个模式来给出相应的提示或进行不同的处理。
    """
    if isinstance(self._control, carla.VehicleControl):
        current_lights = self._lights
        # 如果当前的控制对象是车辆控制类型（`carla.VehicleControl`），先将当前的车辆灯光状态保存到 `current_lights` 变量中，
        // 后续会根据用户操作来更新这个灯光状态，并在必要时应用到车辆上，以实现车辆灯光状态的动态变化。

    for event in pygame.event.get():
        # 遍历通过 `pygame` 获取到的所有事件，根据事件的类型和具体的按键信息来执行相应的操作。

        if event.type == pygame.QUIT:
            return True
            // 如果事件类型是 `pygame.QUIT`，表示用户关闭了图形界面窗口，此时直接返回 `True`，通常意味着程序应该结束运行，
            // 可能会由外部调用该函数的代码根据这个返回值来决定后续的处理逻辑，比如退出主循环等。

        elif event.type == pygame.KEYUP:
            // 如果事件类型是键盘按键松开（`KEYUP`），则根据松开的具体按键执行以下不同的操作，以响应用户的键盘交互。

            if self._is_quit_shortcut(event.key):
                return True
                // 如果按下的键是定义为退出的快捷键（通过 `_is_quit_shortcut` 方法判断，具体实现未在这段代码中展示），
                // 则返回 `True`，同样可能用于告知程序应该结束运行，作用与接收到 `pygame.QUIT` 事件类似。

            elif event.key == K_BACKSPACE:
                if self._autopilot_enabled:
                    world.player.set_autopilot(False)
                    world.restart()
                    world.player.set_autopilot(True)
                else:
                    world.restart()
                // 如果按下的是退格键（`K_BACKSPACE`）：
                // - 若当前处于自动驾驶模式（`_autopilot_enabled` 为 `True`），先关闭车辆的自动驾驶功能，
                // 然后调用 `world.restart()` 方法重启整个模拟世界，最后再重新开启自动驾驶功能。这样做可能是为了重置一些与自动驾驶相关的状态或者场景元素等，
                // 比如重新初始化交通流、车辆位置等情况，确保模拟环境处于一个合适的初始状态。
                // - 若当前未处于自动驾驶模式，则直接重启模拟世界，同样用于重新初始化场景等操作，使模拟世界回到初始状态，方便重新进行模拟操作等。

            elif event.key == K_F1:
                world.hud.toggle_info()
                // 如果按下的是 `F1` 键，调用模拟世界的抬头显示（`hud`）相关方法 `toggle_info` 来切换信息显示的状态，
                // 比如显示或隐藏某些辅助信息（具体显示内容由 `toggle_info` 方法内部逻辑决定），方便用户查看或隐藏一些帮助性的界面元素。

            elif event.key == K_v and pygame.key.get_mods() & KMOD_SHIFT:
                world.next_map_layer(reverse=True)
                // 如果按下的是 `v` 键并且同时按下了 `Shift` 键（通过 `pygame.key.get_mods()` 和 `KMOD_SHIFT` 判断），
                // 则调用模拟世界的 `next_map_layer` 方法来切换地图图层，且传入 `reverse=True` 参数表示是反向切换。
                // 具体如何反向切换由 `next_map_layer` 方法的内部逻辑决定，可能是按照某种特定顺序倒序切换地图的不同显示层级等。

            elif event.key == K_v:
                world.next_map_layer()
                // 如果只按下了 `v` 键，调用模拟世界的 `next_map_layer` 方法按照正常顺序切换地图图层，
                // 用于在不同地图图层之间进行切换浏览，方便用户查看不同层级的地图信息或者切换不同风格、细节程度的地图显示效果。

            elif event.key == K_b and pygame.key.get_mods() & KMOD_SHIFT:
                world.load_map_layer(unload=True)
                // 如果按下的是 `b` 键并且同时按下了 `Shift` 键，调用模拟世界的 `load_map_layer` 方法来加载地图图层，
                // 同时传入 `unload=True` 参数，可能表示先卸载当前已加载的图层再进行加载操作（具体逻辑由 `load_map_layer` 方法内部决定），
                // 这样做也许是为了更新地图数据或者切换到不同的地图配置等情况。

            elif event.key == K_b:
                world.load_map_layer()
                // 如果只按下了 `b` 键，调用模拟世界的 `load_map_layer` 方法来正常加载地图图层，
                // 用于将指定的地图图层加载到模拟世界中显示，使得相应的地图内容在模拟场景中呈现出来，供用户查看和操作。

            elif event.key == K_h or (event.key == K_SLASH and pygame.key.get_mods() & KMOD_SHIFT):
                world.hud.help.toggle()
                // 如果按下的是 `h` 键或者按下的是 `/` 键并且同时按下了 `Shift` 键，调用模拟世界抬头显示中帮助信息相关的 `help.toggle` 方法
                // 来切换帮助信息的显示状态，方便用户查看或隐藏操作帮助内容，使用户可以按需获取关于操作的详细说明等信息。

            elif event.key == K_TAB:
                world.camera_manager.toggle_camera()
                // 如果按下的是 `TAB` 键，调用模拟世界的相机管理对象（`camera_manager`）的 `toggle_camera` 方法来切换相机视角或者相机模式等，
                // 具体切换内容由 `toggle_camera` 方法内部逻辑决定，例如可能在不同车辆摄像头视角（如前视、后视、环视等）之间切换，或者切换不同的渲染模式等。

            elif event.key == K_c and pygame.key.get_mods() & KMOD_SHIFT:
                world.next_weather(reverse=True)
                // 如果按下的是 `c` 键并且同时按下了 `Shift` 键，调用模拟世界的 `next_weather` 方法来切换天气状态，且是按照反向顺序切换，
                // 比如从晴转阴的反向切换（具体反向切换逻辑由 `next_weather` 方法内部决定），可以让用户方便地在不同天气情况之间来回切换查看效果。

            elif event.key == K_c:
                world.next_weather()
                // 如果只按下了 `c` 键，调用模拟世界的 `next_weather` 方法按照正常顺序切换天气状态，用于改变模拟世界中的天气情况，
                // 比如从晴天切换到雨天等，以模拟不同天气环境下的场景表现，为模拟测试等提供多样化的环境条件。

            elif event.key == K_g:
                world.toggle_radar()
                // 如果按下的是 `g` 键，调用模拟世界的 `toggle_radar` 方法来切换雷达的显示状态，比如显示或隐藏雷达相关的可视化界面，
                // 具体操作由 `toggle_radar` 方法内部决定，方便用户根据需求查看或隐藏雷达探测信息等内容。

            elif event.key == K_BACKQUOTE:
                world.camera_manager.next_sensor()
                // 如果按下的是反引号（`K_BACKQUOTE`）键，调用模拟世界相机管理对象的 `next_sensor` 方法来切换到下一个传感器，
                // 可能用于切换不同的车辆传感器（如摄像头、激光雷达等）视角或数据源，便于用户查看不同传感器获取到的信息。

            elif event.key == K_n:
                world.camera_manager.next_sensor()
                // 如果按下的是 `n` 键，同样调用模拟世界相机管理对象的 `next_sensor` 方法来切换到下一个传感器，功能与按下反引号键类似，
                // 可能是提供了另一种切换传感器的操作方式，增加用户操作的便利性和灵活性。

            elif event.key == K_w and (pygame.key.get_mods() & KMOD_CTRL):
                if world.constant_velocity_enabled:
                    world.player.disable_constant_velocity()
                    world.constant_velocity_enabled = False
                    world.hud.notification("Disabled Constant Velocity Mode")
                else:
                    world.player.enable_constant_velocity(carla.Vector3D(17, 0, 0))
                    world.constant_velocity_enabled = True
                    world.hud.notification("Enabled Constant Velocity Mode at 60 km/h")
                // 如果按下的是 `w` 键并且同时按下了 `Ctrl` 键：
                // - 若当前已经启用了恒定速度模式（`world.constant_velocity_enabled` 为 `True`），则调用车辆对象（`world.player`）的 `disable_constant_velocity` 方法
                // 来禁用恒定速度模式，同时将 `world.constant_velocity_enabled` 状态变量设为 `False`，并在抬头显示上给出提示信息，表示已禁用该模式，
                // 告知用户当前车辆不再以恒定速度行驶。
                // - 若当前未启用恒定速度模式，则调用车辆对象的 `enable_constant_velocity` 方法来启用恒定速度模式，
                // 设置速度向量为 `(17, 0, 0)`（可能对应每小时60公里的速度，具体取决于速度单位的设定），更新 `world.constant_velocity_enabled` 为 `True`，
                // 并在抬头显示上给出相应的提示信息，表示已启用该模式及对应的速度，方便用户了解车辆行驶速度模式的变化情况。

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
                // 如果按下的是 `o` 键：
                // - 先尝试执行以下操作，若模拟世界中的车辆门当前是打开状态（`world.doors_are_open` 为 `True`），则在抬头显示上给出提示信息表示正在关门，
                // 将开门状态变量设为 `False`，然后调用车辆对象的 `close_door` 方法关闭所有车门（`carla.VehicleDoor.All` 表示所有车门），实现关闭车辆门的操作。
                // - 若车辆门当前是关闭状态，则在抬头显示上给出提示信息表示正在开门，将开门状态变量设为 `True`，并调用车辆对象的 `open_door` 方法打开所有车门。
                // 若在执行过程中出现异常（比如车门操作不被允许等情况），则直接忽略异常（`pass`），程序继续执行，避免因个别异常情况导致整个程序崩溃。

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
                // 如果按下的是 `t` 键：
                // - 若当前已经显示车辆遥测信息（`world.show_vehicle_telemetry` 为 `True`），则调用车辆对象的 `show_debug_telemetry` 方法并传入 `False`
                // 来关闭车辆的调试遥测信息显示功能，同时更新相应的状态变量 `world.show_vehicle_telemetry` 为 `False`，并在抬头显示上给出提示信息，
                // 表示已禁用车辆遥测信息显示，减少不必要的信息展示，提高界面简洁性。
                // - 若当前未显示车辆遥测信息，则尝试调用车辆对象的 `show_debug_telemetry` 方法并传入 `True` 来启用车辆的调试遥测信息显示功能，
                // 若成功则更新状态变量 `world.show_vehicle_telemetry` 为 `True`，并在抬头显示上给出提示信息，表示已启用车辆遥测信息显示，方便用户获取车辆相关的详细运行数据。
                // 若在启用过程中出现异常，则忽略异常（`pass`），程序继续执行，确保不会因异常而中断程序运行。

            elif event.key > K_0 and event.key <= K_9:
                index_ctrl = 0
                if pygame.key.get_mods() & KMOD_CTRL:
                    index_ctrl = 9
                world.camera_manager.set_sensor(event.key - 1 - K_0 + index_ctrl)
                // 如果按下的键是数字键（`K_0` 到 `K_9` 之间）：
                // - 先判断是否同时按下了 `Ctrl` 键，如果按下了则将 `index_ctrl` 设为 `9`，否则设为 `0`。然后调用模拟世界相机管理对象的 `set_sensor` 方法，
                // 根据按下的数字键以及 `index_ctrl` 的值来设置当前要使用的传感器，具体如何根据这些值设置传感器由 `set_sensor` 方法内部逻辑决定，
                // 这样可以方便用户通过数字键快速切换到指定的传感器视角或数据源，便于查看不同传感器采集的数据。

            elif event.key == K_r and not (pygame.key.get_mods() & KMOD_CTRL):
                world.camera_manager.toggle_recording()
                // 如果按下的是 `r` 键并且没有同时按下 `Ctrl` 键，调用模拟世界相机管理对象的 `toggle_recording` 方法来切换图像录制功能的状态，
                // 比如开始或停止录制传感器获取到的图像数据（具体切换逻辑由 `toggle_recording` 方法内部决定），方便用户按需进行图像数据的录制操作。

            elif event.key == K_r and (pygame.key.get_mods() & KMOD_CTRL):
                if (world.recording_enabled):
                    client.stop_recorder()
                    world.recording_enabled = False
                    world.hud.notification("Recorder is OFF")
                else:
                    client.start_recorder("manual_recording.rec")
                    world.recording_enabled = True
                    world.hud.notification("Recorder is ON")
                // 如果按下的是 `r` 键并且同时按下了 `Ctrl` 键：
                // - 若当前已经启用了录制功能（`world.recording_enabled` 为 `True`），则通过客户端对象（`client`）调用 `stop_recorder` 方法来停止录制，
                // 同时更新录制状态变量 `world.recording_enabled` 为 `False`，并在抬头显示上给出提示信息，表示录制器已关闭，告知用户当前不再进行数据录制。
                // - 若当前未启用录制功能，则通过客户端对象调用 `start_recorder` 方法来启动录制，将录制文件命名为 `manual_recording.rec`，
                // 更新录制状态变量 `world.recording_enabled` 为 `True`，并在抬头显示上给出提示信息，表示录制器已开启，方便用户开始进行数据录制操作。

            elif event.key == K_p and (pygame.key.get_mods() & KMOD_CTRL):
                // stop recorder
                client.stop_recorder()
                world.recording_enabled = False
                // work around to fix camera at start of replaying
                current_index = world.camera_manager.index
                world.destroy_sensors()
                // disable autopilot
                self._autopilot_enabled = False
                world.player.set_autopilot(self._autopilot_enabled)
                world.hud.notification("Replaying file 'manual_recording.rec'")
                // replayer
                client.replay_file("manual_recording.rec", world.recording_start, 0, 0)
                world.camera_manager.set_sensor(current_index)
                // 如果按下的是 `p` 键并且同时按下了 `Ctrl` 键：
                // - 首先通过客户端对象停止录制功能，将录制状态变量 `world.recording_enabled` 设为 `False`。
                // - 为了解决在开始回放时相机相关的问题，先获取当前相机管理对象中的相机索引（`world.camera_manager.index`），
                //
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
    """
    函数功能：根据传入的键盘按键状态（`keys`）以及时间间隔（`milliseconds`）信息，解析并更新车辆的控制参数，
    针对是否启用阿克曼转向控制（`_ackermann_enabled`）有不同的计算和赋值逻辑，以实现对车辆油门、刹车、转向等操作的控制。

    参数说明：
    - `self`：类的实例对象本身，通过它可以访问类的实例属性等信息，这里用于获取和更新实例中与车辆控制相关的属性。
    - `keys`：一个表示键盘按键状态的对象（可能是 `pygame` 相关的数据结构），通过其元素（如 `keys[K_UP]` 等）可以判断各个特定按键是否被按下，用于确定用户的操作意图。
    - `milliseconds`：表示时间间隔的数值，单位可能是毫秒，用于在一些控制参数的计算中，结合时间因素来实现更平滑、合理的控制效果，例如根据时间来调整速度变化量等。
    """
    if keys[K_UP] or keys[K_w]:
        # 判断向上箭头键（`K_UP`）或者 `w` 键是否被按下，如果按下则执行以下操作，用于控制车辆加速（油门操作）。
        if not self._ackermann_enabled:
            # 如果当前未启用阿克曼转向控制（车辆采用常规控制方式）。
            self._control.throttle = min(self._control.throttle + 0.1, 1.00)
            # 将车辆控制对象（`self._control`，类型为 `carla.VehicleControl`）中的油门（`throttle`）参数增加 `0.1`，
            // 但同时通过 `min` 函数限制其最大值为 `1.00`，确保油门值在合理的范围 [0, 1] 内，避免超出正常的油门控制范围。
        else:
            # 如果当前启用了阿克曼转向控制，则按照以下逻辑调整阿克曼控制相关的速度参数。
            self._ackermann_control.speed += round(milliseconds * 0.005, 2) * self._ackermann_reverse
            # 根据时间间隔（`milliseconds`）来计算速度的增加量，计算公式为 `milliseconds * 0.005` 并四舍五入保留两位小数，
            // 然后再乘以 `_ackermann_reverse`（这个值可能与车辆行驶方向相关，比如倒车时取反速度变化方向等），最后将计算得到的速度变化量累加到阿克曼控制对象（`self._ackermann_control`）的 `speed` 属性上，实现速度的动态调整。
    else:
        if not self._ackermann_enabled:
            self._control.throttle = 0.0
            # 如果向上箭头键和 `w` 键都未被按下（即用户没有加速意图），且车辆采用常规控制方式，将油门参数设置为 `0.0`，表示车辆停止加速。

    if keys[K_DOWN] or keys[K_s]:
        # 判断向下箭头键（`K_DOWN`）或者 `s` 键是否被按下，如果按下则执行以下操作，用于控制车辆减速（刹车操作）。
        if not self._ackermann_enabled:
            # 如果当前未启用阿克曼转向控制（车辆采用常规控制方式）。
            self._control.brake = min(self._control.brake + 0.2, 1)
            # 将车辆控制对象（`self._control`）中的刹车（`brake`）参数增加 `0.2`，同时通过 `min` 函数限制其最大值为 `1`，
            // 确保刹车值在合理的范围 [0, 1] 内，模拟刹车力度逐渐增加且不超过最大刹车强度的效果。
        else:
            # 如果当前启用了阿克曼转向控制，则按照以下逻辑调整阿克曼控制相关的速度参数，实现减速操作。
            self._ackermann_control.speed -= min(abs(self._ackermann_control.speed), round(milliseconds * 0.005, 2)) * self._ackermann_reverse
            # 首先计算本次刹车操作对应的速度减少量，取 `self._ackermann_control.speed` 的绝对值与 `milliseconds * 0.005` 四舍五入保留两位小数后的较小值，
            // 再乘以 `_ackermann_reverse`（考虑行驶方向因素），从当前的 `_ackermann_control.speed` 中减去这个减少量，实现速度的降低。
            self._ackermann_control.speed = max(0, abs(self._ackermann_control.speed)) * self._ackermann_reverse
            # 然后确保速度值不小于 `0`，通过取 `0` 和当前速度绝对值的较大值再乘以 `_ackermann_reverse` 来更新速度，防止速度变为负数（符合实际物理情况）。
    else:
        if not self._ackermann_enabled:
            self._control.brake = 0
            # 如果向下箭头键和 `s` 键都未被按下（即用户没有刹车意图），且车辆采用常规控制方式，将刹车参数设置为 `0`，表示车辆不进行刹车操作。

    steer_increment = 5e-4 * milliseconds
    # 根据时间间隔（`milliseconds`）计算转向增量，计算公式为 `5e-4 * milliseconds`，意味着随着时间的增加，转向的调整幅度会相应增大，
    // 这样可以使转向操作在不同的时间间隔下更加平滑、合理，避免瞬间转向幅度过大。

    if keys[K_LEFT] or keys[K_a]:
        # 判断向左箭头键（`K_LEFT`）或者 `a` 键是否被按下，如果按下则执行以下转向操作，用于控制车辆向左转向。
        if self._steer_cache > 0:
            self._steer_cache = 0
            # 如果之前缓存的转向值（`_steer_cache`）大于 `0`（表示之前可能处于向右转向或者转向回正的状态），则将其重置为 `0`，
            // 即优先响应新的向左转向操作，清除之前相反方向的转向缓存。
        else:
            self._steer_cache -= steer_increment
            # 如果之前的转向缓存值小于等于 `0`，则按照计算得到的转向增量（`steer_increment`）减少 `_steer_cache` 的值，实现向左转向角度的累积增加，模拟逐渐向左打方向盘的操作。
    elif keys[K_RIGHT] or keys[K_d]:
        # 判断向右箭头键（`K_RIGHT`）或者 `d` 键是否被按下，如果按下则执行以下转向操作，用于控制车辆向右转向。
        if self._steer_cache < 0:
            self._steer_cache = 0
            # 如果之前缓存的转向值（`_steer_cache`）小于 `0`（表示之前可能处于向左转向或者转向回正的状态），则将其重置为 `0`，
            // 即优先响应新的向右转向操作，清除之前相反方向的转向缓存。
        else:
            self._steer_cache += steer_increment
            # 如果之前的转向缓存值大于等于 `0`，则按照计算得到的转向增量（`steer_increment`）增加 `_steer_cache` 的值，实现向右转向角度的累积增加，模拟逐渐向右打方向盘的操作。
    else:
        self._steer_cache = 0.0
        # 如果左右方向键都未被按下（即用户没有转向意图），则将转向缓存值重置为 `0.0`，表示车辆转向回正，停止转向操作。

    self._steer_cache = min(0.7, max(-0.7, self._steer_cache))
    # 对转向缓存值（`_steer_cache`）进行限制，确保其在合理的范围 [-0.7, 0.7] 内，避免转向角度过大导致不合理的车辆行驶状态，模拟实际车辆转向角度的合理限制。

    if not self._ackermann_enabled:
        self._control.steer = round(self._steer_cache, 1)
        self._control.hand_brake = keys[K_SPACE]
        # 如果当前未启用阿克曼转向控制（车辆采用常规控制方式）：
        // - 将车辆控制对象（`self._control`）中的转向（`steer`）参数设置为经过四舍五入保留一位小数后的转向缓存值（`_steer_cache`），实现将计算好的转向角度应用到车辆控制上。
        // - 根据空格键（`K_SPACE`）是否被按下，来设置车辆控制对象中的手刹（`hand_brake`）参数，若空格键被按下则手刹拉起（值为 `True`，此处虽未显式写布尔值转换，但根据 `keys` 的逻辑可推测如此），否则手刹松开（值为 `False`）。
    else:
        self._ackermann_control.steer = round(self._steer_cache, 1)
        // 如果当前启用了阿克曼转向控制，则将阿克曼控制对象（`self._ackermann_control`）中的转向（`steer`）参数设置为经过四舍五入保留一位小数后的转向缓存值（`_steer_cache`），
        // 按照阿克曼转向控制的逻辑来应用转向角度，实现对车辆转向的精确控制。

   def _parse_walker_keys(self, keys, milliseconds, world):
    """
    函数功能：根据传入的键盘按键状态（`keys`）、时间间隔（`milliseconds`）以及模拟世界（`world`）相关信息，解析并更新行人角色（`walker`）的控制参数，
    包括行人的移动速度、旋转角度、跳跃状态以及移动方向等，以此来实现对行人在模拟世界中行为的控制。

    参数说明：
    - `self`：类的实例对象本身，通过它可以访问类的实例属性等信息，这里用于获取和更新实例中与行人控制相关的属性。
    - `keys`：一个表示键盘按键状态的对象（可能是 `pygame` 相关的数据结构），通过其元素（如 `keys[K_UP]` 等）可以判断各个特定按键是否被按下，用于确定用户的操作意图。
    - `milliseconds`：表示时间间隔的数值，单位可能是毫秒，用于在一些控制参数的计算中，结合时间因素来实现更平滑、合理的控制效果，例如根据时间来调整旋转角度变化量等。
    - `world`：代表整个模拟世界的对象，这里主要用于获取行人的最大速度相关信息（`world.player_max_speed` 和 `world.player_max_speed_fast`），以根据用户操作来设置合适的行人移动速度。
    """
    self._control.speed = 0.0
    # 首先将行人控制对象（`self._control`，类型为 `carla.WalkerControl`）中的速度（`speed`）参数初始化为 `0.0`，表示行人初始状态为静止。

    if keys[K_DOWN] or keys[K_s]:
        self._control.speed = 0.0
        # 判断向下箭头键（`K_DOWN`）或者 `s` 键是否被按下，如果按下同样将行人速度设置为 `0.0`，即按下这两个键时行人保持静止，
        // 可能意味着这两个键在当前设计中没有赋予实际的移动功能或者用于停止行人当前的移动操作等。

    if keys[K_LEFT] or keys[K_a]:
        self._control.speed =.01
        self._rotation.yaw -= 0.08 * milliseconds
        # 判断向左箭头键（`K_LEFT`）或者 `a` 键是否被按下，如果按下：
        // - 将行人控制对象中的速度参数设置为 `0.01`，表示行人开始以一个相对较慢的固定速度向左移动（这里 `0.01` 应该是经过设定的合适的移动速度值，具体单位可能由模拟环境定义）。
        // - 根据时间间隔（`milliseconds`）来减少行人的旋转角度（`yaw`），计算公式为 `0.08 * milliseconds`，意味着随着时间的推移，行人会逐渐向左转身，
        // 转身的角度变化量与时间成正比，实现更平滑自然的转向效果，模拟行人向左行走时身体自然转向的动作。

    if keys[K_RIGHT] or keys[K_d]:
        self._control.speed =.01
        self._rotation.yaw += 0.08 * milliseconds
        # 判断向右箭头键（`K_RIGHT`）或者 `d` 键是否被按下，如果按下：
        // - 将行人控制对象中的速度参数设置为 `0.01`，表示行人开始以一个相对较慢的固定速度向右移动（与向左移动时速度值相同，保持对称的操作逻辑）。
        // - 根据时间间隔（`milliseconds`）来增加行人的旋转角度（`yaw`），计算公式为 `0.08 * milliseconds`，意味着随着时间的推移，行人会逐渐向右转身，
        // 同样实现了更平滑自然的转向效果，模拟行人向右行走时身体自然转向的动作。

    if keys[K_UP] or keys[K_w]:
        self._control.speed = world.player_max_speed_fast if pygame.key.get_mods() & KMOD_SHIFT else world.player_max_speed
        # 判断向上箭头键（`K_UP`）或者 `w` 键是否被按下，如果按下：
        // - 通过判断是否同时按下了 `Shift` 键（`pygame.key.get_mods() & KMOD_SHIFT`）来决定行人的移动速度。
        // 如果按下了 `Shift` 键，则将行人速度设置为 `world.player_max_speed_fast`（可能表示行人的快速移动速度，具体值由模拟世界对象 `world` 中的相关属性定义），
        // 否则设置为 `world.player_max_speed`（可能表示行人的正常移动速度），以此实现通过不同按键组合来控制行人以不同速度向前移动的功能。

    self._control.jump = keys[K_SPACE]
    # 根据空格键（`K_SPACE`）是否被按下，来设置行人控制对象中的跳跃（`jump`）参数，若空格键被按下则 `jump` 参数为 `True`（此处虽未显式写布尔值转换，但根据逻辑可推测如此），表示行人执行跳跃动作，否则为 `False`，表示行人不跳跃。

    self._rotation.yaw = round(self._rotation.yaw, 1)
    # 对行人的旋转角度（`yaw`）进行四舍五入保留一位小数的操作，这样可以避免旋转角度出现过于精确或不必要的小数位，使角度值更加简洁合理，同时也可能符合模拟环境中对角度精度的实际要求。

    self._control.direction = self._rotation.get_forward_vector()
    # 将行人控制对象中的方向（`direction`）参数设置为通过行人当前旋转角度（`self._rotation`）获取到的前向向量，
    // 以此确定行人实际的移动方向，使得行人按照其当前朝向所对应的前方方向进行移动，保证移动方向与旋转角度的一致性。

@staticmethod
def _is_quit_shortcut(key):
    """
    静态方法功能：判断传入的键盘按键（`key`）是否为定义的退出快捷键组合。

    参数说明：
    - `key`：表示一个键盘按键的代码（可能是 `pygame` 中定义的按键常量），用于判断是否符合退出快捷键的定义。
    """
    return (key == K_ESCAPE) or (key == K_q and pygame.key.get_mods() & KMOD_CTRL)
    // 返回一个布尔值，表示传入的按键是否满足退出快捷键的条件。具体判断逻辑为：如果按键是 `K_ESCAPE`（通常对应 `Esc` 键）或者
    // 按键是 `K_q` 并且同时按下了 `Ctrl` 键（通过 `pygame.key.get_mods() & KMOD_CTRL` 判断），则认为是退出快捷键，返回 `True`，否则返回 `False`。
    // 这样在其他地方调用该方法时，可以方便地判断用户是否按下了特定的退出快捷键组合，以决定是否执行相应的退出程序等操作。


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
    """
    函数功能：在每一帧更新时执行相关操作，用于收集和整理模拟世界（`world`）中与角色（车辆或行人）相关的各种信息，并将这些信息整理成特定格式存储在 `_info_text` 属性中，
    例如角色的位置、速度、传感器数据、控制参数以及周围车辆情况等信息，可能后续用于在界面上显示这些状态信息给用户查看。

    参数说明：
    - `self`：类的实例对象本身，通过它可以访问类的实例属性等信息，用于更新和操作实例中与信息展示相关的属性及数据结构。
    - `world`：代表整个模拟世界的对象，包含了场景中的各种元素（车辆、行人、传感器等）以及相关的获取状态信息的方法，用于获取角色及世界的各种实时状态数据。
    - `clock`：可能是用于记录时间相关信息的对象，例如获取当前帧率等，用于展示帧率相关信息以及一些基于时间的计算（虽然在本函数中未体现明显的时间计算依赖于 `clock`，但作为整体逻辑的一部分传入进来）。
    """
    self._notifications.tick(world, clock)
    # 调用 `self._notifications` 对象的 `tick` 方法，传入模拟世界（`world`）和时间对象（`clock`），可能用于更新通知相关的状态信息（具体功能取决于 `_notifications` 对象的实现，此处未展示其代码细节），比如清除过期的通知等操作。

    if not self._show_info:
        return
    # 如果 `_show_info` 属性为 `False`，表示不需要显示相关信息，直接返回，不执行后续的信息收集和整理操作，可能用于根据用户设置或者程序状态来决定是否展示详细信息。

    t = world.player.get_transform()
    # 获取模拟世界中当前主角（`world.player`，可能是车辆或者行人角色）的变换信息（`get_transform` 方法），包含了位置、旋转等信息，并存储到变量 `t` 中，后续会从中提取具体的位置等数据用于信息展示。

    v = world.player.get_velocity()
    # 获取模拟世界中当前主角的速度信息（`get_velocity` 方法），存储到变量 `v` 中，用于后续计算并展示主角的移动速度相关情况。

    c = world.player.get_control()
    # 获取模拟世界中当前主角的控制参数信息（`get_control` 方法），存储到变量 `c` 中，根据主角是车辆还是行人，其控制参数结构不同，后续会根据这个参数的类型来展示不同的控制相关信息（如油门、刹车、转向等对于车辆，速度、跳跃等对于行人）。

    compass = world.imu_sensor.compass
    # 从模拟世界的惯性测量单元（`imu_sensor`）中获取指南针方向数据（`compass`），该数据可能表示当前主角的朝向角度，用于后续判断并展示具体的方位信息（如东南西北方向）。

    heading = 'N' if compass > 270.5 or compass < 89.5 else ''
    heading += 'S' if 90.5 < compass < 269.5 else ''
    heading += 'E' if 0.5 < compass < 179.5 else ''
    heading += 'W' if 180.5 < compass < 359.5 else ''
    // 根据获取到的指南针方向数据（`compass`）来确定并拼接方位字符串（`heading`），按照角度范围判断当前主角大致朝向哪个方向（北、南、东、西），例如角度大于 270.5 度或者小于 89.5 度则认为是朝北（添加 `N` 到 `heading` 字符串中），以此类推，最后 `heading` 字符串就表示了当前主角的方位信息。

    colhist = world.collision_sensor.get_collision_history()
    # 从模拟世界的碰撞传感器（`collision_sensor`）获取碰撞历史数据（`get_collision_history` 方法），这个数据可能是一个记录了过去一段时间内碰撞情况的序列，用于后续分析和展示碰撞相关信息。

    collision = [colhist[x + self.frame - 200] for x in range(0, 200)]
    // 通过列表推导式从碰撞历史数据（`colhist`）中提取最近 200 个时间步（此处 `self.frame` 可能与当前帧数相关，具体逻辑未完全展示，但大概是基于当前帧往前推 200 帧对应的碰撞数据）的碰撞信息，存储到 `collision` 列表中，方便后续对这段时间内的碰撞情况进行处理和展示。

    max_col = max(1.0, max(collision))
    collision = [x / max_col for x in collision]
    // 先找出 `collision` 列表中的最大值（与 `1.0` 取较大值作为 `max_col`），然后将 `collision` 列表中的每个元素都除以 `max_col`，这样做可能是为了对碰撞数据进行归一化处理，使其数值范围更便于展示或者后续的比较分析等操作。

    vehicles = world.world.get_actors().filter('vehicle.*')
    // 通过模拟世界对象（`world.world`，这里可能是对整个世界场景中所有对象的一个顶层表示）的 `get_actors` 方法获取所有的演员（`actors`，在这个场景下可能主要是指车辆和行人等可移动对象），然后使用 `filter` 方法筛选出所有名称匹配 `vehicle.*` 模式的对象，也就是获取所有的车辆对象，存储到 `vehicles` 列表中，用于后续展示周围车辆相关信息。

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
        ''
    ]
    // 初始化 `self._info_text` 列表，用于存储要展示的各种信息文本内容，按照一定的格式进行组织，依次添加服务器帧率、客户端帧率、当前主角名称（车辆或行人，通过 `get_actor_display_name` 方法获取并截断显示长度为 20 个字符）、地图名称（取 `world.map.name` 分割后的最后一部分，可能是地图的具体名称或标识）、模拟时间（格式化为时间间隔形式）、主角速度（通过速度向量计算并转换为千米每小时单位）、指南针方向及方位信息、加速度计数据、陀螺仪数据、主角位置（提取 `x` 和 `y` 坐标展示）、全球导航卫星系统（`GNSS`）的经纬度信息、主角高度（`z` 坐标）等信息，每个信息占一行，方便后续整体展示。

    if isinstance(c, carla.VehicleControl):
        self._info_text += [
            ('Throttle:', c.throttle, 0.0, 1.0),
            ('Steer:', c.steer, -1.0, 1.0),
            ('Brake:', c.brake, 0.0, 1.0),
            ('Reverse:', c.reverse),
            ('Hand brake:', c.hand_brake),
            ('Manual:', c.manual_gear_shift),
            'Gear:        %s' % {-1: 'R', 0: 'N'}.get(c.gear, c.gear)
        ]
        if self._show_ackermann_info:
            self._info_text += [
                '',
                'Ackermann Controller:',
                '  Target speed: % 8.0f km/h' % (3.6*self._ackermann_control.speed)
            ]
        // 如果当前主角的控制参数（`c`）类型是 `carla.VehicleControl`，说明主角是车辆，则在 `_info_text` 列表中添加车辆特有的控制参数信息，包括油门（展示油门值，并标注取值范围是 0.0 到 1.0）、转向（展示转向值，并标注取值范围是 -1.0 到 1.0）、刹车（展示刹车值及取值范围）、是否倒车、是否手刹拉起、是否手动换挡以及当前挡位信息（通过字典映射将挡位值转换为对应的挡位表示，如 -1 表示倒车挡 `R`，0 表示空挡 `N` 等）。
        // 如果 `_show_ackermann_info` 属性为 `True`，表示需要展示阿克曼控制器相关信息，则继续添加阿克曼控制器的目标速度信息（将速度值转换为千米每小时单位展示），用于给用户提供更详细的车辆控制相关状态。

    elif isinstance(c, carla.WalkerControl):
        self._info_text += [
            ('Speed:', c.speed, 0.0, 5.556),
            ('Jump:', c.jump)
        ]
        // 如果当前主角的控制参数（`c`）类型是 `carla.WalkerControl`，说明主角是行人，则在 `_info_text` 列表中添加行人特有的控制参数信息，包括行人的移动速度（展示速度值，并标注取值范围是 0.0 到 5.556，这里的取值范围应该是根据模拟环境中行人速度的合理范围设定的）以及是否跳跃（展示跳跃状态），方便用户了解行人的当前行为控制情况。

    self._info_text += [
        '',
        'Collision:',
        collision,
        '',
        'Number of vehicles: % 8d' % len(vehicles)
    ]
    // 在 `_info_text` 列表中继续添加碰撞信息相关内容，先是添加一个空行用于分隔，然后添加 `Collision:` 作为标识，接着添加之前处理好的归一化后的碰撞数据列表（`collision`），再添加一个空行，最后添加当前场景中车辆的数量信息（通过 `len(vehicles)` 获取车辆列表长度即车辆数量，并按照格式 `% 8d` 进行格式化展示），用于展示碰撞历史以及周围车辆的数量情况。

    if len(vehicles) > 1:
        self._info_text += ['Nearby vehicles:']
        distance = lambda l: math.sqrt((l.x - t.location.x)**2 + (l.y - t.location.y)**2 + (l.z - t.location.z)**2)
        vehicles = [(distance(x.get_location()), x) for x in vehicles if x.id!= world.player.id]
        for d, vehicle in sorted(vehicles, key=lambda vehicles: vehicles[0]):
            if d > 200.0:
                break
            vehicle_type = get_actor_display_name(vehicle, truncate=22)
            self._info_text.append('% 4dm %s' % (d, vehicle_type))
        // 如果场景中车辆数量大于 1（即除了主角车辆外还有其他车辆），则在 `_info_text` 列表中添加 `Nearby vehicles:` 作为标识，表示后续展示周围附近车辆的信息。
        // 定义一个匿名函数（`lambda` 表达式） `distance`，用于计算给定位置（`l`）与主角位置（`t.location`）之间的距离，通过坐标差值的平方和开根号来计算欧几里得距离。
        // 使用列表推导式遍历所有车辆（过滤掉主角车辆，通过 `x.id!= world.player.id` 判断），计算每辆车与主角的距离，并将距离和车辆对象组成元组，存储到新的 `vehicles` 列表中。
        // 然后对这个新的 `vehicles` 列表按照距离（元组的第一个元素）进行排序（通过 `sorted` 函数和指定 `key` 参数为提取元组第一个元素的匿名函数），遍历排序后的列表，对于距离小于等于 200.0 米的车辆（超出这个距离则跳出循环，认为是较远不需要展示），获取车辆的显示名称（通过 `get_actor_display_name` 方法并截断长度为 22 个字符），并按照 `% 4dm %s` 的格式将距离（单位米）和车辆名称添加到 `_info_text` 列表中，用于展示主角周围附近车辆的距离及类型信息。

    def show_ackermann_info(self, enabled):
    """
    函数功能：用于设置是否显示阿克曼控制器相关信息的标志位。

    参数说明：
    - `self`：类的实例对象本身，通过它可以访问类的实例属性等信息，这里用于更新实例中表示阿克曼信息显示状态的属性。
    - `enabled`：一个布尔值，用于指定是否要显示阿克曼控制器相关信息，若为 `True` 则表示要显示，`False` 则表示不显示。
    """
    self._show_ackermann_info = enabled
    // 将实例属性 `_show_ackermann_info` 的值设置为传入的 `enabled` 参数值，以此来控制后续在信息展示环节是否呈现阿克曼控制器相关的详细信息。

def update_ackermann_control(self, ackermann_control):
    """
    函数功能：更新实例中保存的阿克曼控制对象（`_ackermann_control`），使其与传入的阿克曼控制参数保持一致。

    参数说明：
    - `self`：类的实例对象本身，用于更新实例中与阿克曼控制相关的属性。
    - `ackermann_control`：一个 `carla.VehicleAckermannControl` 类型的对象（推测，根据函数上下文及常见的使用场景），包含了阿克曼转向控制相关的各种参数信息，用于替换当前实例中保存的旧的阿克曼控制对象，实现控制参数的更新。
    """
    self._ackermann_control = ackermann_control
    // 将实例属性 `_ackermann_control` 的值更新为传入的 `ackermann_control` 参数所代表的阿克曼控制对象，这样在后续涉及阿克曼控制的操作中，会使用新传入的控制参数进行相应的计算和处理。

def toggle_info(self):
    """
    函数功能：切换是否显示信息的状态，即对当前的信息显示标志位（`_show_info`）取反，用于控制相关信息在界面上的显示与隐藏。
    """
    self._show_info = not self._show_info
    // 将实例属性 `_show_info` 的值进行取反操作，若原来为 `True`（表示显示信息）则变为 `False`（隐藏信息），反之亦然，方便用户通过操作（比如按下某个对应的按键）来切换信息展示与否的状态。

def notification(self, text, seconds=2.0):
    """
    函数功能：设置要显示的通知信息内容以及显示时长。

    参数说明：
    - `self`：类的实例对象本身，通过它调用 `_notifications` 对象的相关方法来设置通知信息。
    - `text`：一个字符串，表示要显示的通知内容，告知用户一些相关的提示、状态等信息。
    - `seconds`：一个浮点数，默认值为 `2.0`，表示通知信息在界面上显示的时长，单位为秒，用于控制通知显示的时间长度，过后通知自动消失。
    """
    self._notifications.set_text(text, seconds=seconds)
    // 通过实例中的 `_notifications` 对象调用其 `set_text` 方法，传入要显示的通知文本内容（`text`）以及显示时长（`seconds`）参数，来设置并触发显示相应的通知信息，让用户能够看到对应的提示内容。

def error(self, text):
    """
    函数功能：用于显示错误信息通知，将传入的文本内容包装成特定格式（添加 `Error:` 前缀）并设置为红色字体显示，以突出错误提示。

    参数说明：
    - `self`：类的实例对象本身，通过它调用 `_notifications` 对象的相关方法来设置错误通知信息。
    - `text`：一个字符串，表示具体的错误描述内容，告知用户出现了什么错误情况。
    """
    self._notifications.set_text('Error: %s' % text, (255, 0, 0))
    // 通过实例中的 `_notifications` 对象调用其 `set_text` 方法，将传入的错误文本内容（`text`）包装成带有 `Error:` 前缀的格式，同时传入一个表示颜色的元组 `(255, 0, 0)`（代表红色，用于设置错误信息的文本颜色，使其在界面上更醒目），来设置并显示错误通知信息，提醒用户发生了错误情况。

def render(self, display):
    """
    函数功能：负责将各种信息（如角色状态信息、通知信息、帮助信息等）渲染到给定的显示界面（`display`）上，根据不同的信息类型和格式进行相应的绘制操作，
    例如绘制文本、绘制进度条（用于展示一些数值范围的参数情况）等，以可视化的方式呈现给用户。

    参数说明：
    - `self`：类的实例对象本身，通过它可以访问类的实例属性等信息，用于获取要渲染的各种信息内容以及相关的显示设置（如字体、显示区域尺寸等）。
    - `display`：一个 `pygame` 中的显示对象（推测，根据常见的 `pygame` 渲染相关代码结构），代表了整个要进行绘制显示的界面，所有的渲染操作都会作用到这个对象上，最终将呈现出完整的可视化界面效果。
    """
    if self._show_info:
        info_surface = pygame.Surface((220, self.dim[1]))
        info_surface.set_alpha(100)
        display.blit(info_surface, (0, 0))
        // 如果当前设置为显示信息（`_show_info` 为 `True`），则进行以下渲染操作：
        // - 创建一个新的 `pygame.Surface` 对象，它代表一个二维的图像表面，大小设置为宽度 `220` 像素，高度为实例属性 `self.dim[1]` 所指定的值（可能是整个显示区域在垂直方向上的尺寸等相关设定），这个表面将用于承载要显示的信息内容。
        // - 设置这个表面的透明度（`alpha` 值）为 `100`，使得后续在上面绘制的信息会呈现出一定的半透明效果，可能用于使信息显示不那么突兀，与背景更好地融合或者突出显示的层次感等。
        // - 使用 `display.blit` 方法将这个半透明的表面绘制到给定的显示界面（`display`）的左上角坐标 `(0, 0)` 位置，作为信息显示的背景区域。

        v_offset = 4
        bar_h_offset = 100
        bar_width = 106
        for item in self._info_text:
            if v_offset + 18 > self.dim[1]:
                break
            // 初始化垂直偏移量（`v_offset`）为 `4`，用于控制每行信息在垂直方向上的绘制位置；初始化水平偏移量（`bar_h_offset`）为 `100` 和进度条宽度（`bar_width`）为 `106`，这两个参数可能用于后续绘制一些类似进度条形式来展示数值范围的信息时的位置和尺寸设定。
            // 然后遍历 `self._info_text` 列表（这个列表在其他函数中已经填充了各种要显示的信息内容，如角色状态信息等），对于每一项信息进行相应的绘制操作，当垂直偏移量加上每行信息的大致高度（这里取 `18` 像素，根据后续代码推测是一个合适的高度值，用于控制行间距等）超过了显示区域的垂直尺寸（`self.dim[1]`）时，停止遍历，避免超出显示范围进行绘制。

            if isinstance(item, list):
                if len(item) > 1:
                    points = [(x + 8, v_offset + 8 + (1.0 - y) * 30) for x, y in enumerate(item)]
                    pygame.draw.lines(display, (255, 136, 0), False, points, 2)
                item = None
                v_offset += 18
                // 如果当前遍历到的信息项（`item`）是一个列表类型，并且列表长度大于 `1`，则进行以下操作：
                // - 通过列表推导式构建一个坐标点列表 `points`，用于绘制线条。对于列表中的每个元素，其 `x` 坐标是索引值加上 `8`（可能是为了在水平方向上有一定的偏移，使其绘制位置更合适），`y` 坐标是垂直偏移量 `v_offset` 加上 `8` 再加上根据元素值 `y` 计算得到的垂直方向上的偏移量（通过 `(1.0 - y) * 30` 计算，这里的 `30` 可能是一个根据显示效果调整好的垂直方向上的长度系数，用于根据数值大小合理分布线条的位置等）。
                // - 使用 `pygame.draw.lines` 方法在显示界面（`display`）上绘制一组不闭合的线条（`False` 表示不闭合），线条颜色设置为橙色（`(255, 136, 0)`），线条宽度为 `2` 像素，使用前面构建好的坐标点列表 `points` 来确定线条的形状，可能用于展示某种数值变化趋势或者数据关系等图形化信息。
                // - 将当前信息项设置为 `None`，表示已经处理完这个列表类型的信息项，然后将垂直偏移量增加 `18` 像素，准备绘制下一行信息。

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
                // 如果当前遍历到的信息项（`item`）是一个元组类型，则进行以下操作：
                // - 如果元组的第二个元素是布尔类型（可能用于表示某种开关状态等信息），则创建一个矩形对象（`rect`），其位置在水平方向上为 `bar_h_offset`，垂直方向上为 `v_offset + 8`，大小为宽度 `6` 像素、高度 `6` 像素，用于绘制一个小的矩形来表示布尔状态。使用 `pygame.draw.rect` 方法在显示界面（`display`）上绘制这个矩形，颜色设置为白色（`(255, 255, 255)`），如果布尔值为 `True` 则填充矩形（边框宽度设置为 `0`），如果布尔值为 `False` 则只绘制矩形边框（边框宽度设置为 `1`），以此来直观地展示布尔状态信息。
                // - 如果元组的第二个元素不是布尔类型（可能是数值类型，用于表示某个在一定范围内取值的参数等情况），则进行以下操作来绘制类似进度条的可视化表示：
                // - 首先创建一个矩形对象（`rect_border`）作为进度条的外边框，位置在水平方向上为 `bar_h_offset`，垂直方向上为 `v_offset + 8`，大小为宽度 `bar_width` 像素、高度 `6` 像素，使用 `pygame.draw.rect` 方法绘制这个白色的外边框（边框宽度设置为 `1`）。
                // - 然后计算一个比例系数 `f`，通过将当前数值（`item[1]`）减去取值范围的下限值（`item[2]`），再除以取值范围的总跨度（`item[3] - item[2]`）得到，这个系数用于确定在进度条内填充的长度比例。
                // - 根据取值范围下限值是否小于 `0.0` 来确定填充矩形（`rect`）的绘制位置和尺寸，如果下限值小于 `0.0`，则填充矩形的水平位置从 `bar_h_offset` 加上根据比例系数 `f` 计算得到的偏移量（基于进度条宽度减去 `6` 像素，可能是考虑边框等因素的一种计算方式），垂直位置为 `v_offset + 8`，大小为宽度 `6` 像素、高度 `6` 像素；如果下限值大于等于 `0.0`，则填充矩形的水平位置为 `bar_h_offset`，宽度根据比例系数 `f` 乘以进度条宽度（`bar_width`）计算得到，垂直位置和高度与前面相同，最后使用 `pygame.draw.rect` 方法绘制这个填充矩形（颜色为白色），以此来直观地展示数值在给定范围内的具体取值情况。
                // - 将当前信息项更新为元组的第一个元素（可能是对应参数的名称等描述信息），方便后续按照字符串类型进行处理和绘制。

            if item:  # At this point has to be a str.
                surface = self._font_mono.render(item, True, (255, 255, 255))
                display.blit(surface, (8, v_offset))
            v_offset += 18
            // 如果经过前面的类型判断和处理后，当前信息项（`item`）仍然存在（不为 `None`），此时它应该是一个字符串类型（根据前面的逻辑，其他类型都已经进行了相应处理或者更新了 `item`），则进行以下操作：
            // - 使用实例中的 `_font_mono` 字体对象（推测是之前初始化好的用于显示信息的特定字体）调用 `render` 方法，将字符串信息（`item`）渲染成一个图像表面（`surface`），设置文本颜色为白色（`(255, 255, 255)`），这个表面就包含了要显示的文本图像内容。
            // - 使用 `display.blit` 方法将这个包含文本图像的表面绘制到显示界面（`display`）上，位置在水平方向为 `8` 像素（可能是为了与前面绘制的其他元素保持合适的间距等），垂直方向为当前的垂直偏移量（`v_offset`）位置，实现文本信息的显示。
            // - 最后将垂直偏移量增加 `18` 像素，准备绘制下一行信息，保持合适的行间距，继续循环处理下一个信息项。

        self._notifications.render(display)
        self.help.render(display)
        // 调用实例中的 `_notifications` 对象的 `render` 方法，将通知信息渲染到显示界面（`display`）上，显示之前通过 `notification` 等方法设置的各种提示、错误等通知内容。
        // 调用实例中的 `help` 对象（推测是与帮助信息显示相关的对象，其具体实现未完全展示）的 `render` 方法，将帮助信息也渲染到显示界面上，方便用户查看相关的操作帮助等内容，完善整个界面的信息展示效果。

class HelpText(object):
    """
    Helper class to handle text output using pygame

    这个类是一个辅助类，主要用于借助 `pygame` 库来处理文本输出相关的操作，例如创建包含帮助信息文本的表面（Surface），
    控制文本的显示与隐藏，并将文本内容渲染到指定的显示界面上，方便在程序中为用户展示操作帮助等相关文本信息。
    """
    def __init__(self, font, width, height):
        """
        类的构造函数（初始化方法），用于初始化 `HelpText` 类实例的各种属性，为后续处理和展示帮助文本做准备。

        参数：
        - `font`：一个 `pygame.font.Font` 类型的对象（推测，根据常见 `pygame` 文本渲染用法），代表了要用于渲染文本的字体样式，通过它可以将文本字符串渲染成可视化的图像表面，以便在界面上显示。
        - `width`：表示显示界面的宽度（单位可能是像素，具体取决于使用场景和 `pygame` 的设置），用于确定帮助文本显示区域在水平方向上的位置等相关布局计算。
        - `height`：表示显示界面的高度（单位同样可能是像素），用于确定帮助文本显示区域在垂直方向上的位置等相关布局计算。
        """
        lines = __doc__.split('\n')
        // 将类的文档字符串（`__doc__`，即类定义上方的那个三引号包裹的描述性文本）按换行符 `\n` 进行分割，得到一个包含每一行文本的列表 `lines`，
        // 这里可能是打算将类的文档字符串作为默认的帮助文本内容来使用（虽然实际应用中可能可以根据需求替换为更具体详细的帮助信息内容）。

        self.font = font
        // 将传入的字体对象（`font`）赋值给实例属性 `self.font`，方便后续使用该字体来渲染文本。

        self.line_space = 18
        // 初始化实例属性 `self.line_space` 为 `18`，表示每行文本之间的垂直间距（单位可能是像素），用于控制帮助文本显示时的行间距，使文本排版更清晰美观。

        self.dim = (780, len(lines) * self.line_space + 12)
        // 初始化实例属性 `self.dim`，它是一个包含两个元素的元组，表示帮助文本显示区域的尺寸（宽度和高度）。宽度设置为 `780` 像素，高度根据帮助文本的行数（`len(lines)`）乘以每行的间距（`self.line_space`）再加上 `12` 像素来计算，
        // 这个 `12` 像素可能是额外预留的一些边距等空间，以此确定整个帮助文本显示区域的大小。

        self.pos = (0.5 * width - 0.5 * self.dim[0], 0.5 * height - 0.5 * self.dim[1])
        // 初始化实例属性 `self.pos`，它是一个包含两个元素的元组，表示帮助文本显示区域在整个显示界面中的位置坐标（水平坐标和垂直坐标）。
        // 通过将显示界面宽度（`width`）和高度（`height`）的一半分别减去帮助文本显示区域尺寸（`self.dim`）的一半，来将帮助文本显示区域定位在显示界面的中心位置，实现居中显示的效果。

        self.seconds_left = 0
        // 初始化实例属性 `self.seconds_left` 为 `0`，从变量名推测这个属性可能用于记录帮助文本还剩余显示的时间（单位为秒），不过在当前代码中未看到明显的基于时间控制显示时长的相关逻辑使用它，可能是预留的功能或者后续会完善的部分。

        self.surface = pygame.Surface(self.dim)
        // 创建一个新的 `pygame.Surface` 对象，它代表一个二维的图像表面，大小为之前确定的帮助文本显示区域尺寸（`self.dim`），这个表面将作为承载帮助文本内容的载体，后续会在这个表面上绘制文本等元素。

        self.surface.fill((0, 0, 0, 0))
        // 使用黑色（`(0, 0, 0)`）且完全透明（透明度值为 `0`，即 `(0, 0, 0, 0)`）的颜色填充这个表面，这里先将表面清空或者设置为初始的透明状态，方便后续绘制文本等操作，使其初始状态符合预期的显示效果要求。

        for n, line in enumerate(lines):
            text_texture = self.font.render(line, True, (255, 255, 255))
            self.surface.blit(text_texture, (22, n * self.line_space))
            self._render = False
            // 遍历之前分割得到的帮助文本行列表（`lines`），对于每一行文本进行以下操作：
            // - 使用实例中的字体对象（`self.font`）调用 `render` 方法，将当前行文本（`line`）渲染成一个图像表面（`text_texture`），设置文本颜色为白色（`(255, 255, 255)`），这样就得到了可视化的文本图像内容。
            // - 使用 `self.surface`（即前面创建的用于承载文本的表面）的 `blit` 方法，将渲染好的文本图像表面（`text_texture`）绘制到 `self.surface` 上，位置在水平方向为 `22` 像素（可能是为了预留一定的左边距，使文本看起来更协调），垂直方向为当前行的索引值（`n`）乘以每行的间距（`self.line_space`），以此实现逐行将帮助文本绘制到指定的表面上。
            // - 最后将实例属性 `_render` 设置为 `False`，从变量名和后续代码逻辑推测，这个属性用于控制帮助文本是否要渲染显示到最终的显示界面上，初始状态设为 `False` 表示默认不显示帮助文本。

        self.surface.set_alpha(220)
        // 设置帮助文本所在表面（`self.surface`）的透明度（`alpha` 值）为 `220`，使其呈现出一定的半透明效果，这样在显示帮助文本时，既可以让用户看到文本内容，又不会完全遮挡住后面的界面元素，达到较好的视觉显示效果。

    def toggle(self):
        """
        函数功能：切换帮助文本的显示与隐藏状态，通过对 `_render` 属性取反来实现，方便用户通过操作（如按下特定按键）来控制帮助信息的显示与否。
        """
        self._render = not self._render
        // 将实例属性 `_render` 的值进行取反操作，若原来为 `True`（表示显示帮助文本）则变为 `False`（隐藏帮助文本），反之亦然，以此来控制帮助文本在界面上的显示状态。

    def render(self, display):
        """
        函数功能：根据当前的显示状态（由 `_render` 属性决定），将帮助文本所在的表面（`self.surface`）渲染到给定的显示界面（`display`）上，
        如果 `_render` 为 `True`，则进行绘制操作，实现帮助文本的显示；如果为 `False`，则不进行绘制，帮助文本保持隐藏状态。

        参数说明：
        - `self`：类的实例对象本身，通过它可以访问类的实例属性等信息，用于获取帮助文本所在的表面以及其显示位置等信息，以便进行渲染操作。
        - `display`：一个 `pygame` 中的显示对象（推测，根据常见的 `pygame` 渲染相关代码结构），代表了整个要进行绘制显示的界面，所有的渲染操作都会作用到这个对象上，若帮助文本要显示，就会将其绘制到这个界面上呈现给用户。
        """
        if self._render:
            display.blit(self.surface, self.pos)
            // 如果当前 `_render` 属性为 `True`，表示需要显示帮助文本，则使用 `display`（显示界面对象）的 `blit` 方法，将帮助文本所在的表面（`self.surface`）绘制到显示界面上，
            // 绘制的位置为之前初始化时确定的位置坐标（`self.pos`），从而将帮助文本显示在界面的相应位置上，供用户查看帮助信息内容。


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
        """
        类的构造函数，用于初始化 `CollisionSensor` 实例的相关属性，并创建和配置碰撞传感器，使其能够监听碰撞事件。

        参数说明：
        - `parent_actor`：一个代表父级角色（通常是车辆或者其他可碰撞的实体对象）的对象，碰撞传感器会关联到这个父级角色上，用于检测该角色与其他对象之间的碰撞情况。
        - `hud`：一个与抬头显示（HUD，Head-Up Display）相关的对象，通过它可以向用户展示一些提示信息，比如在发生碰撞时在界面上显示相应的通知内容。
        """
        self.sensor = None
        self.history = []
        self._parent = parent_actor
        self.hud = hud
        // 初始化实例的几个属性：
        // - `self.sensor` 初始化为 `None`，后续会在这里存储创建好的碰撞传感器对象。
        // - `self.history` 初始化为一个空列表，用于记录碰撞事件的历史信息，例如碰撞发生的帧序号以及碰撞的强度等数据。
        // - `self._parent` 存储传入的父级角色对象，以便后续获取相关的世界信息以及关联传感器到这个角色上。
        // - `self.hud` 存储传入的抬头显示相关对象，用于后续在发生碰撞时进行信息提示等操作。

        world = self._parent.get_world()
        bp = world.get_blueprint_library().find('sensor.other.collision')
        self.sensor = world.spawn_actor(bp, carla.Transform(), attach_to=self._parent)
        // 通过父级角色对象（`self._parent`）获取其所在的模拟世界对象（`get_world` 方法），然后在这个世界的蓝图库（`get_blueprint_library` 方法）中查找名为 `sensor.other.collision` 的传感器蓝图。
        // 找到蓝图后，使用世界对象的 `spawn_actor` 方法在指定的变换位置（初始位置使用 `carla.Transform()` 表示默认位置，这里会关联到父级角色上）创建并生成实际的碰撞传感器对象，将其赋值给 `self.sensor` 属性，这样就成功创建了一个碰撞传感器并附着到了父级角色上，使其能够检测该角色的碰撞情况。

        # We need to pass the lambda a weak reference to self to avoid circular
        # reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda event: CollisionSensor._on_collision(weak_self, event))
        // 为了避免循环引用（在Python中，如果对象之间相互引用，可能会导致内存无法正确回收的问题），这里创建了一个对当前实例（`self`）的弱引用（`weakref.ref(self)`），并将其赋值给 `weak_self` 变量。
        // 然后让创建好的碰撞传感器（`self.sensor`）开始监听碰撞事件，通过调用 `listen` 方法并传入一个匿名函数（`lambda` 表达式）作为回调函数。当碰撞事件发生时，这个匿名函数会被调用，它会将弱引用（`weak_self`）和碰撞事件对象（`event`）作为参数传递给类的静态方法 `_on_collision`，由该静态方法来处理具体的碰撞事件逻辑。

    def get_collision_history(self):
        """
        函数功能：获取并整理碰撞历史信息，将历史记录中每个帧对应的多次碰撞强度进行累加，返回一个按照帧序号统计碰撞强度总和的字典。

        返回值：
        - 返回一个 `collections.defaultdict(int)` 类型的字典，键为帧序号，值为该帧对应的碰撞强度总和，用于展示在不同帧上发生碰撞的总体强度情况，方便后续分析碰撞历史数据。
        """
        history = collections.defaultdict(int)
        for frame, intensity in self.history:
            history[frame] += intensity
        return history
        // 创建一个默认值为整数 `0` 的字典（`collections.defaultdict(int)`），用于统计每个帧序号对应的碰撞强度总和。
        // 遍历存储碰撞历史信息的列表（`self.history`），其中每个元素是一个包含帧序号（`frame`）和碰撞强度（`intensity`）的元组。对于每个元组，将对应帧序号的碰撞强度累加到 `history` 字典中该帧序号对应的键值上（如果键不存在则初始化为 `0` 后再累加，这就是 `defaultdict` 的特性），这样最终 `history` 字典就记录了每个帧上的碰撞强度总和情况，最后返回这个整理好的碰撞历史字典。

    @staticmethod
    def _on_collision(weak_self, event):
        """
        静态方法功能：作为碰撞事件的处理函数，当碰撞发生时被调用，用于更新碰撞历史记录，并在抬头显示上给出碰撞提示信息。

        参数说明：
        - `weak_self`：一个对 `CollisionSensor` 类实例的弱引用，通过它可以获取到实际的实例对象，同时避免了循环引用问题，在方法内部需要先将其解引用还原为实际的实例对象才能访问实例的属性和方法。
        - `event`：一个包含碰撞事件详细信息的对象，例如碰撞涉及的其他角色、碰撞的冲量等信息，用于提取相关数据来记录碰撞情况和进行提示等操作。
        """
        self = weak_self()
        if not self:
            return
        // 通过弱引用（`weak_self`）获取实际的 `CollisionSensor` 类实例对象，如果获取失败（即 `weak_self` 所引用的对象已经被垃圾回收了，返回 `None`），则直接返回，不执行后续的碰撞处理逻辑。

        actor_type = get_actor_display_name(event.other_actor)
        self.hud.notification('Collision with %r' % actor_type)
        // 获取与当前父级角色发生碰撞的其他角色的显示名称（通过 `get_actor_display_name` 方法），然后使用抬头显示对象（`self.hud`）调用 `notification` 方法，在界面上显示一条提示信息，告知用户发生了与何种类型角色的碰撞，让用户及时了解碰撞情况。

        impulse = event.normal_impulse
        intensity = math.sqrt(impulse.x**2 + impulse.y**2 + impulse.z**2)
        self.history.append((event.frame, intensity))
        // 从碰撞事件对象（`event`）中获取碰撞的法向冲量（`normal_impulse`）信息，存储到 `impulse` 变量中。
        // 通过计算冲量在三个坐标轴方向上的分量的平方和的平方根（即向量的模长），得到碰撞的强度值（`intensity`），以此来衡量碰撞的剧烈程度。
        // 将碰撞发生的帧序号（`event.frame`）和计算得到的碰撞强度（`intensity`）组成一个元组，添加到 `self.history` 列表中，用于记录这次碰撞的相关信息，方便后续查询和分析碰撞历史情况。

        if len(self.history) > 4000:
            self.history.pop(0)
        // 判断碰撞历史记录列表（`self.history`）的长度是否超过了 `4000`，如果超过了，说明历史记录过多，为了避免占用过多内存或者保持历史记录的合理性，删除列表中的第一个元素（即最早记录的碰撞信息），实现一种简单的历史记录队列管理，只保留最近的一部分碰撞历史数据。


# ==============================================================================
# -- LaneInvasionSensor --------------------------------------------------------
# ==============================================================================


class LaneInvasionSensor(object):
    def __init__(self, parent_actor, hud):
        """
        类的构造函数，用于初始化 `LaneInvasionSensor` 实例相关属性，并在满足条件（父级角色是车辆类型）时创建和配置车道入侵传感器，使其能够监听车道入侵事件。

        参数说明：
        - `parent_actor`：代表父级角色的对象，通常是车辆或者其他实体，车道入侵传感器会关联到这个对象上，用于检测该对象是否发生车道入侵行为。
        - `hud`：与抬头显示（HUD）相关的对象，通过它可以向用户展示提示信息，例如在检测到车道入侵事件时，在界面上显示相应的通知内容。
        """
        self.sensor = None
        // 初始化 `self.sensor` 属性为 `None`，后续会根据具体情况在这里存储创建好的车道入侵传感器对象。

        # If the spawn object is not a vehicle, we cannot use the Lane Invasion Sensor
        if parent_actor.type_id.startswith("vehicle."):
            // 判断传入的父级角色对象（`parent_actor`）的类型 ID 是否以 "vehicle." 开头，以此来确定该角色是否为车辆类型。
            // 因为车道入侵传感器通常是针对车辆来检测其是否越过车道线等入侵行为的，所以只有当父级角色是车辆时，才进行后续的传感器创建及相关配置操作。

            self._parent = parent_actor
            self.hud = hud
            // 如果父级角色是车辆，将其赋值给实例属性 `self._parent`，方便后续获取相关世界信息以及关联传感器到这个角色上；
            // 同时将传入的抬头显示相关对象（`hud`）赋值给 `self.hud` 属性，用于后续在检测到车道入侵事件时进行信息提示等操作。

            world = self._parent.get_world()
            bp = world.get_blueprint_library().find('sensor.other.lane_invasion')
            self.sensor = world.spawn_actor(bp, carla.Transform(), attach_to=self._parent)
            // 通过父级角色对象（`self._parent`）获取其所在的模拟世界对象（`get_world` 方法），然后在这个世界的蓝图库（`get_blueprint_library` 方法）中查找名为 `sensor.other.lane_invasion` 的传感器蓝图。
            // 找到对应蓝图后，使用世界对象的 `spawn_actor` 方法在指定的变换位置（初始位置使用 `carla.Transform()` 表示默认位置，这里会关联到父级角色上）创建并生成实际的车道入侵传感器对象，将其赋值给 `self.sensor` 属性，这样就成功创建了一个车道入侵传感器并附着到了父级角色（车辆）上，使其能够检测该车辆的车道入侵情况。

            # We need to pass the lambda a weak reference to self to avoid circular
            # reference.
            weak_self = weakref.ref(self)
            self.sensor.listen(lambda event: LaneInvasionSensor._on_invasion(weak_self, event))
            // 为了避免循环引用（在Python中，如果对象之间相互引用，可能会导致内存无法正确回收的问题），这里创建了一个对当前实例（`self`）的弱引用（`weakref.ref(self)`），并将其赋值给 `weak_self` 变量。
            // 然后让创建好的车道入侵传感器（`self.sensor`）开始监听车道入侵事件，通过调用 `listen` 方法并传入一个匿名函数（`lambda` 表达式）作为回调函数。当车道入侵事件发生时，这个匿名函数会被调用，它会将弱引用（`weak_self`）和车道入侵事件对象（`event`）作为参数传递给类的静态方法 `_on_invasion`，由该静态方法来处理具体的车道入侵事件逻辑。

    @staticmethod
    def _on_invasion(weak_self, event):
        """
        静态方法功能：作为车道入侵事件的处理函数，当检测到车道入侵事件发生时被调用，用于在抬头显示上给出相应的提示信息，告知用户车辆越过了哪些类型的车道线。

        参数说明：
        - `weak_self`：一个对 `LaneInvasionSensor` 类实例的弱引用，通过它可以获取到实际的实例对象，同时避免了循环引用问题，在方法内部需要先将其解引用还原为实际的实例对象才能访问实例的属性和方法。
        - `event`：一个包含车道入侵事件详细信息的对象，例如车辆越过的车道线标记类型等信息，用于提取相关数据来生成提示内容并展示给用户。
        """
        self = weak_self()
        if not self:
            return
        // 通过弱引用（`weak_self`）获取实际的 `LaneInvasionSensor` 类实例对象，如果获取失败（即 `weak_self` 所引用的对象已经被垃圾回收了，返回 `None`），则直接返回，不执行后续的车道入侵处理逻辑。

        lane_types = set(x.type for x in event.crossed_lane_markings)
        // 从车道入侵事件对象（`event`）中提取车辆越过的所有车道线标记（`crossed_lane_markings`），并通过生成器表达式获取每个车道线标记的类型（`x.type`），然后使用 `set` 函数将这些类型去重，存储到 `lane_types` 集合中，这样就得到了车辆越过的不同类型车道线的集合。

        text = ['%r' % str(x).split()[-1] for x in lane_types]
        // 对于 `lane_types` 集合中的每个车道线类型，先将其转换为字符串（`str(x)`），然后通过 `split` 方法以空格为分隔符进行分割，取最后一个元素（`split()[-1]`），再使用 `%r` 格式化将其包装成一个带引号的字符串表示形式，将这些字符串组成一个列表存储到 `text` 列表中，这样 `text` 列表中的每个元素就是一种车道线类型的格式化表示，方便后续用于生成提示信息文本。

        self.hud.notification('Crossed line %s' % ' and '.join(text))
        // 使用抬头显示对象（`self.hud`）调用 `notification` 方法，在界面上显示一条提示信息，信息内容为告知用户车辆越过了哪些类型的车道线（通过 `' and '.join(text)` 将 `text` 列表中的车道线类型字符串用 " and " 连接起来，形成一个完整的提示文本），让用户及时了解车辆的车道入侵情况。


# ==============================================================================
# -- GnssSensor ----------------------------------------------------------------
# ==============================================================================


class GnssSensor(object):
    def __init__(self, parent_actor):
        """
        类的构造函数，用于初始化 `GnssSensor` 实例的相关属性，创建并配置全球导航卫星系统（GNSS）传感器，使其能够监听相关定位事件以获取经纬度信息。

        参数说明：
        - `parent_actor`：代表父级角色的对象，通常是车辆或者其他需要获取定位信息的实体，GNSS 传感器会附着在这个对象上，用于获取该对象所在位置的经纬度数据。
        """
        self.sensor = None
        self._parent = parent_actor
        self.lat = 0.0
        self.lon = 0.0
        // 初始化实例的几个属性：
        // - `self.sensor` 初始化为 `None`，后续会在这里存储创建好的 GNSS 传感器对象。
        // - `self._parent` 存储传入的父级角色对象，方便后续获取相关世界信息以及将传感器关联到这个角色上。
        // - `self.lat` 和 `self.lon` 分别初始化为 `0.0`，用于后续存储通过 GNSS 传感器获取到的纬度和经度信息，它们将实时更新以反映当前位置的经纬度情况。

        world = self._parent.get_world()
        bp = world.get_blueprint_library().find('sensor.other.gnss')
        self.sensor = world.spawn_actor(bp, carla.Transform(carla.Location(x=1.0, z=2.8)), attach_to=self._parent)
        // 通过父级角色对象（`self._parent`）获取其所在的模拟世界对象（`get_world` 方法），然后在这个世界的蓝图库（`get_blueprint_library` 方法）中查找名为 `sensor.other.gnss` 的传感器蓝图，这个蓝图定义了 GNSS 传感器的基本属性和行为。
        // 找到对应的传感器蓝图后，使用世界对象的 `spawn_actor` 方法创建实际的 GNSS 传感器对象。创建时指定了传感器的初始变换位置（通过 `carla.Transform(carla.Location(x=1.0, z=2.8))` 设置其在 `x` 轴方向偏移 `1.0` 单位，`z` 轴方向（通常可理解为高度方向）偏移 `2.8` 单位的位置），并且将其附着到父级角色（`attach_to=self._parent`）上，使得传感器与对应的角色关联起来，能够获取该角色所在位置的相关信息，最后将创建好的传感器对象赋值给 `self.sensor` 属性。

        # We need to pass the lambda a weak reference to self to avoid circular
        # reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda event: GnssSensor._on_gnss_event(weak_self, event))
        // 为了避免循环引用（在 Python 中，对象间相互引用可能导致内存无法正常回收的问题），创建一个对当前实例（`self`）的弱引用（`weakref.ref(self)`），并将其赋值给 `weak_self` 变量。
        // 接着让创建好的 GNSS 传感器（`self.sensor`）开始监听 GNSS 相关事件，通过调用 `listen` 方法并传入一个匿名函数（`lambda` 表达式）作为回调函数。当 GNSS 定位等相关事件发生时，这个匿名函数会被调用，它会把弱引用（`weak_self`）和事件对象（`event`）作为参数传递给类的静态方法 `_on_gnss_event`，由该静态方法来处理具体的获取并更新经纬度信息的逻辑。

    @staticmethod
    def _on_gnss_event(weak_self, event):
        """
        静态方法功能：作为 GNSS 事件的处理函数，当 GNSS 相关事件发生时被调用，用于更新实例中存储的纬度（`lat`）和经度（`lon`）信息，使其反映最新的定位数据。

        参数说明：
        - `weak_self`：一个对 `GnssSensor` 类实例的弱引用，通过它可以获取到实际的实例对象，同时避免了循环引用问题，在方法内部需要先将其解引用还原为实际的实例对象才能访问实例的属性和方法。
        - `event`：一个包含 GNSS 事件详细信息的对象，这里主要关注其中的纬度（`latitude`）和经度（`longitude`）信息，用于提取数据并更新实例中对应的属性值。
        """
        self = weak_self()
        if not self:
            return
        // 通过弱引用（`weak_self`）获取实际的 `GnssSensor` 类实例对象，如果获取失败（即 `weak_self` 所引用的对象已经被垃圾回收了，返回 `None`），则直接返回，不执行后续更新经纬度信息的逻辑。

        self.lat = event.latitude
        self.lon = event.longitude
        // 将事件对象（`event`）中的纬度信息（`latitude`）赋值给实例的 `self.lat` 属性，将经度信息（`longitude`）赋值给 `self.lon` 属性，这样实例中存储的经纬度值就会随着 GNSS 传感器获取到的最新定位数据而实时更新，从而可以在其他地方使用这些最新的经纬度信息来实现如地图显示、位置追踪等相关功能。


# ==============================================================================
# -- IMUSensor -----------------------------------------------------------------
# ==============================================================================


class IMUSensor(object):
    def __init__(self, parent_actor):
        """
        类的构造函数，用于初始化 `IMUSensor` 实例的相关属性，创建并配置惯性测量单元（IMU）传感器，使其能够监听传感器数据事件以获取加速度、陀螺仪和指南针等相关信息。

        参数说明：
        - `parent_actor`：代表父级角色的对象，通常是车辆或者其他需要获取运动状态及方向信息的实体，IMU 传感器会附着在这个对象上，用于实时获取该对象对应的加速度、旋转角速度以及方向等数据。
        """
        self.sensor = None
        self._parent = parent_actor
        self.accelerometer = (0.0, 0.0, 0.0)
        self.gyroscope = (0.0, 0.0, 0.0)
        self.compass = 0.0
        // 初始化实例的几个属性：
        // - `self.sensor` 初始化为 `None`，后续会在这里存储创建好的 IMU 传感器对象。
        // - `self._parent` 存储传入的父级角色对象，方便后续获取相关世界信息以及将传感器关联到这个角色上。
        // - `self.accelerometer` 初始化为一个包含三个 `0.0` 的元组，用于存储从 IMU 传感器获取到的加速度信息，分别对应 `x`、`y`、`z` 三个坐标轴方向上的加速度值，后续会根据传感器数据实时更新。
        // - `self.gyroscope` 同样初始化为一个包含三个 `0.0` 的元组，用于存储陀螺仪数据，即物体绕 `x`、`y`、`z` 轴旋转的角速度信息，同样会依据传感器反馈的数据进行更新。
        // - `self.compass` 初始化为 `0.0`，用于存储表示方向的指南针数据，也就是获取物体当前的朝向角度信息，会随着传感器的测量结果而改变。

        world = self._parent.get_world()
        bp = world.get_blueprint_library().find('sensor.other.imu')
        self.sensor = world.spawn_actor(
            bp, carla.Transform(), attach_to=self._parent)
        // 通过父级角色对象（`self._parent`）获取其所在的模拟世界对象（`get_world` 方法），然后在这个世界的蓝图库（`get_blueprint_library` 方法）中查找名为 `sensor.other.imu` 的传感器蓝图，该蓝图定义了 IMU 传感器的基本属性与行为规范。
        // 找到对应的传感器蓝图后，使用世界对象的 `spawn_actor` 方法创建实际的 IMU 传感器对象。创建时通过 `carla.Transform()` 指定传感器的初始变换位置为默认位置，并将其附着到父级角色（`attach_to=self._parent`）上，使传感器与对应的角色建立关联，以便获取该角色的运动状态等相关信息，最后把创建好的传感器对象赋值给 `self.sensor` 属性。

        # We need to pass the lambda a weak reference to self to avoid circular
        # reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(
            lambda sensor_data: IMUSensor._IMU_callback(weak_self, sensor_data))
        // 为了避免循环引用（在 Python 中，对象之间相互引用可能导致内存无法正常回收的问题），创建一个对当前实例（`self`）的弱引用（`weakref.ref(self)`），并将其赋值给 `weak_self` 变量。
        // 接着让创建好的 IMU 传感器（`self.sensor`）开始监听传感器数据事件，通过调用 `listen` 方法并传入一个匿名函数（`lambda` 表达式）作为回调函数。当 IMU 传感器获取到新的数据时，这个匿名函数会被调用，它会把弱引用（`weak_self`）和包含传感器数据的对象（`sensor_data`）作为参数传递给类的静态方法 `_IMU_callback`，由该静态方法来处理具体的更新加速度、陀螺仪和指南针等信息的逻辑。

    @staticmethod
    def _IMU_callback(weak_self, sensor_data):
        """
        静态方法功能：作为 IMU 传感器数据事件的处理函数，当接收到传感器数据时被调用，用于更新实例中存储的加速度、陀螺仪和指南针信息，同时对数据进行一定范围的限制处理，确保数据的合理性。

        参数说明：
        - `weak_self`：一个对 `IMUSensor` 类实例的弱引用，通过它可以获取到实际的实例对象，同时避免了循环引用问题，在方法内部需要先将其解引用还原为实际的实例对象才能访问实例的属性和方法。
        - `sensor_data`：一个包含 IMU 传感器最新测量数据的对象，其中包含了加速度、陀螺仪和指南针等相关数据信息，用于提取并处理这些数据，更新实例中对应的属性值。
        """
        self = weak_self()
        if not self:
            return
        // 通过弱引用（`weak_self`）获取实际的 `IMUSensor` 类实例对象，如果获取失败（即 `weak_self` 所引用的对象已经被垃圾回收了，返回 `None`），则直接返回，不执行后续更新传感器信息的逻辑。

        limits = (-99.9, 99.9)
        self.accelerometer = (
            max(limits[0], min(limits[1], sensor_data.accelerometer.x)),
            max(limits[0], min(limits[1], sensor_data.accelerometer.y)),
            max(limits[0], min(limits[1], sensor_data.accelerometer.z)))
        // 定义一个数据范围限制的元组 `limits`，表示允许的传感器数据取值范围在 `-99.9` 到 `99.9` 之间（这里只是一种合理的数据范围限定，可根据实际需求和传感器特性调整）。
        // 对于加速度信息，通过 `max` 和 `min` 函数组合，将 `sensor_data.accelerometer.x`（`x` 轴方向的加速度值）限制在 `limits` 所定义的范围内，即取 `limits[0]`（下限值）和 `limits[1]`（上限值）与该加速度值中合适的那个，以此确保加速度值不会超出合理区间，然后按照同样的方式处理 `y` 轴和 `z` 轴方向的加速度值，更新实例的 `self.accelerometer` 属性，使得存储的加速度数据始终在合理的范围之内。

        self.gyroscope = (
            max(limits[0], min(limits[1], math.degrees(sensor_data.gyroscope.x))),
            max(limits[0], min(limits[1], math.degrees(sensor_data.gyroscope.y))),
            max(limits[0], min(limits[1], math.degrees(sensor_data.gyroscope.z))))
        // 对于陀螺仪数据，由于传感器返回的可能是弧度制下的旋转角速度，先使用 `math.degrees` 函数将 `sensor_data.gyroscope.x`（`x` 轴方向的角速度值）转换为角度制，再通过 `max` 和 `min` 函数将其限制在 `limits` 所定义的合理角度范围内，按照同样的操作处理 `y` 轴和 `z` 轴方向的角速度值，更新实例的 `self.gyroscope` 属性，确保存储的陀螺仪数据是角度制且在合理范围之内，方便后续使用和展示。

        self.compass = math.degrees(sensor_data.compass)
        // 对于指南针数据（表示方向的角度信息），同样使用 `math.degrees` 函数将 `sensor_data.compass` 从传感器返回的可能的弧度制转换为角度制，然后更新实例的 `self.compass` 属性，使其存储的是角度制下的方向信息，符合常规的角度表示和使用习惯，便于在如导航、方位显示等相关功能中使用该数据。

# ==============================================================================
# -- RadarSensor ---------------------------------------------------------------
# ==============================================================================


class RadarSensor(object):
    def __init__(self, parent_actor):
        """
        类的构造函数，用于初始化 `RadarSensor` 实例的相关属性，创建并配置雷达传感器，包括设置其视野范围、安装位置等，同时让传感器开始监听数据事件，并指定数据处理的回调函数。

        参数说明：
        - `parent_actor`：代表父级角色的对象，通常是车辆或者其他需要使用雷达进行周边环境探测的实体，雷达传感器会附着在这个对象上，以便基于该角色的位置来探测周围目标的相关信息。
        """
        self.sensor = None
        self._parent = parent_actor
        // 初始化实例的两个属性：
        // - `self.sensor` 初始化为 `None`，后续会在这里存储创建好的雷达传感器对象。
        // - `self._parent` 存储传入的父级角色对象，方便后续获取相关世界信息、角色的边界信息以及将传感器关联到这个角色上。

        bound_x = 0.5 + self._parent.bounding_box.extent.x
        bound_y = 0.5 + self._parent.bounding_box.extent.y
        bound_z = 0.5 + self._parent.bounding_box.extent.z
        // 计算雷达传感器在 `x`、`y`、`z` 方向上相对父级角色边界的安装位置偏移量。通过获取父级角色（`self._parent`）的包围盒（`bounding_box`）的范围（`extent`），并在各方向上加上 `0.5`，确定了雷达传感器在该角色周边大致的安装边界位置，
        // 这样的设置可以使雷达传感器处于合适的位置来探测周围环境，避免因位置不合理导致探测范围受限或出现不准确的情况。

        self.velocity_range = 7.5  # m/s
        // 初始化实例属性 `self.velocity_range`，设定雷达传感器用于检测目标速度的范围为 `7.5 m/s`，这个范围值会在后续对目标速度的归一化处理以及可视化显示等相关操作中起到重要作用，用于将目标速度映射到特定的区间进行分析和展示。

        world = self._parent.get_world()
        self.debug = world.debug
        bp = world.get_blueprint_library().find('sensor.other.radar')
        // 通过父级角色对象（`self._parent`）获取其所在的模拟世界对象（`get_world` 方法），并从世界对象中获取调试工具对象（`debug`）赋值给 `self.debug` 属性，这个调试工具对象后续可用于在模拟场景中绘制一些可视化的调试信息（如绘制雷达探测到的目标点等）。
        // 在世界的蓝图库（`get_blueprint_library` 方法）中查找名为 `sensor.other.radar` 的雷达传感器蓝图，该蓝图定义了雷达传感器的基本属性和行为规范，后续将基于这个蓝图来创建实际的雷达传感器对象。

        bp.set_attribute('horizontal_fov', str(35))
        bp.set_attribute('vertical_fov', str(20))
        // 设置雷达传感器蓝图（`bp`）的属性，分别将水平视野范围（`horizontal_fov`）设置为 `35` 度，垂直视野范围（`vertical_fov`）设置为 `20` 度，以此确定雷达传感器的探测角度范围，使其能够在设定的角度区域内对周围环境进行目标探测，符合具体的功能需求。

        self.sensor = world.spawn_actor(
            bp,
            carla.Transform(
                carla.Location(x=bound_x + 0.05, z=bound_z + 0.05),
                carla.Rotation(pitch=5)),
            attach_to=self._parent)
        // 使用世界对象（`world`）的 `spawn_actor` 方法基于前面配置好的雷达传感器蓝图（`bp`）创建实际的雷达传感器对象，并设置其初始位置和姿态。
        // 位置通过 `carla.Transform` 来指定，在 `x` 轴方向上相对于之前计算的边界位置（`bound_x`）再偏移 `0.05`，在 `z` 轴方向（通常是高度方向）相对于边界位置（`bound_z`）也偏移 `0.05`，同时设置其俯仰角（`pitch`）为 `5` 度，将传感器附着到父级角色（`attach_to=self._parent`）上，使传感器与对应的角色建立关联，以便基于角色的位置进行环境探测，最后将创建好的传感器对象赋值给 `self.sensor` 属性。

        # We need a weak reference to self to avoid circular reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(
            lambda radar_data: RadarSensor._Radar_callback(weak_self, radar_data))
        // 为了避免循环引用（在 Python 中，对象之间相互引用可能导致内存无法正常回收的问题），创建一个对当前实例（`self`）的弱引用（`weakref.ref(self)`），并将其赋值给 `weak_self` 变量。
        // 接着让创建好的雷达传感器（`self.sensor`）开始监听传感器数据事件，通过调用 `listen` 方法并传入一个匿名函数（`lambda` 表达式）作为回调函数。当雷达传感器获取到新的数据时，这个匿名函数会被调用，它会把弱引用（`weak_self`）和包含雷达传感器数据的对象（`radar_data`）作为参数传递给类的静态方法 `_Radar_callback`，由该静态方法来处理具体的分析处理雷达数据以及可视化展示相关的逻辑。

    @staticmethod
    def _Radar_callback(weak_self, radar_data):
        """
        静态方法功能：作为雷达传感器数据事件的处理函数，当接收到雷达传感器数据时被调用，用于分析处理数据，包括对目标的方位、速度等信息进行转换和计算，然后根据这些信息在模拟场景中通过调试工具绘制出对应的可视化点，以展示雷达探测到的目标情况。

        参数说明：
        - `weak_self`：一个对 `RadarSensor` 类实例的弱引用，通过它可以获取到实际的实例对象，同时避免了循环引用问题，在方法内部需要先将其解引用还原为实际的实例对象才能访问实例的属性和方法。
        - `radar_data`：一个包含雷达传感器最新测量数据的对象，其中包含了探测到的目标的各项信息，如方位角、仰角、速度、距离等，用于提取并处理这些数据，实现目标的可视化展示等功能。
        """
        self = weak_self()
        if not self:
            return
        // 通过弱引用（`weak_self`）获取实际的 `RadarSensor` 类实例对象，如果获取失败（即 `weak_self` 所引用的对象已经被垃圾回收了，返回 `None`），则直接返回，不执行后续处理雷达数据及绘制可视化信息的逻辑。

        # To get a numpy [[vel, altitude, azimuth, depth],...[,,,]]:
        # points = np.frombuffer(radar_data.raw_data, dtype=np.dtype('f4'))
        # points = np.reshape(points, (len(radar_data), 4))
        // 这几行代码是注释掉的示例代码，展示了一种可以将雷达数据原始字节数据（`radar_data.raw_data`）转换为 `numpy` 数组的方式，先按照单精度浮点数（`np.dtype('f4')`）类型从字节数据中解析出数据点，然后将其重塑为二维数组，每一行代表一个目标的速度（`vel`）、高度（`altitude`）、方位角（`azimuth`）和距离（`depth`）信息，虽然当前代码未使用这种方式处理数据，但可以作为一种参考思路用于进一步的数据处理和分析。

        current_rot = radar_data.transform.rotation
        for detect in radar_data:
            azi = math.degrees(detect.azimuth)
            alt = math.degrees(detect.altitude)
            // 对于雷达数据中的每个探测到的目标（通过遍历 `radar_data` 中的每个 `detect`），先将目标的方位角（`detect.azimuth`）和仰角（`detect.altitude`）从雷达传感器返回的弧度制转换为角度制，分别存储到变量 `azi` 和 `alt` 中，这样转换后的角度值更符合常规的角度表示和使用习惯，便于后续在空间坐标转换等操作中使用。

            # The 0.25 adjusts a bit the distance so the dots can
            # be properly seen
            fw_vec = carla.Vector3D(x=detect.depth - 0.25)
            carla.Transform(
                carla.Location(),
                carla.Rotation(
                    pitch=current_rot.pitch + alt,
                    yaw=current_rot.yaw + azi,
                    roll=current_rot.roll)).transform(fw_vec)
            // 创建一个 `carla.Vector3D` 类型的向量 `fw_vec`，其 `x` 轴方向的分量初始设置为目标的距离（`detect.depth`）减去 `0.25`，这里减去 `0.25` 可能是一种微调操作，目的是调整距离使得后续绘制出的表示目标的点在可视化展示时能够更合适、清晰地被看到。
            // 然后基于当前雷达传感器的旋转姿态（`current_rot`）以及目标的方位角（`azi`）和仰角（`alt`）构建一个新的变换（`carla.Transform`），通过这个变换对向量 `fw_vec` 进行转换，将其从雷达传感器坐标系下转换到世界坐标系或者更合适的用于可视化展示的坐标系下，以便后续根据这个转换后的向量准确地在场景中定位并绘制目标点。

            def clamp(min_v, max_v, value):
                return max(min_v, min(value, max_v))
            norm_velocity = detect.velocity / self.velocity_range  # range [-1, 1]
            r = int(clamp(0.0, 1.0, 1.0 - norm_velocity) * 255.0)
            g = int(clamp(0.0, 1.0, 1.0 - abs(norm_velocity)) * 255.0)
            b = int(abs(clamp(- 1.0, 0.0, - 1.0 - norm_velocity)) * 255.0)
            // 定义一个内部函数 `clamp`，用于将一个值限制在给定的最小值（`min_v`）和最大值（`max_v`）之间，确保数值在合理的范围之内。
            // 接着将目标的速度（`detect.velocity`）除以之前设定的速度范围（`self.velocity_range`），对速度进行归一化处理，使其映射到 `[-1, 1]` 的区间内，得到 `norm_velocity`，方便后续根据速度大小来确定绘制目标点的颜色。
            // 根据归一化后的速度值计算用于表示颜色的红（`r`）、绿（`g`）、蓝（`b`）分量的值，通过 `clamp` 函数结合一些运算，将速度值对应的颜色分量限制在 `0` 到 `255` 的范围（适合 `RGB` 颜色表示的范围）内，例如对于红色分量，通过 `1.0 - norm_velocity` 这样的计算结合 `clamp` 函数和乘以 `255.0` 操作，使得速度越大，红色分量越小，颜色越偏向其他颜色，以此实现根据速度来动态改变颜色的效果，直观地展示目标的速度信息。

            self.debug.draw_point(
                radar_data.transform.location + fw_vec,
                size=0.075,
                life_time=0.06,
                persistent_lines=False,
                color=carla.Color(r, g, b))
            // 使用实例的调试工具对象（`self.debug`）调用 `draw_point` 方法，在模拟场景中绘制一个表示雷达探测到的目标的点。
            // 绘制点的位置是雷达传感器的位置（`radar_data.transform.location`）加上前面转换后的向量（`fw_vec`），确定了目标在场景中的具体位置。设置点的大小为 `0.075`，生命周期（`life_time`）为 `0.06` 秒，表示这个点在场景中显示的时长，设置 `persistent_lines` 为 `False` 表示不是绘制连续的线条（只是单个点），并根据前面计算得到的颜色（`carla.Color(r, g, b)`）来设置点的颜色，通过这样的方式将雷达探测到的每个目标以可视化的彩色点展示在模拟场景中，方便直观地观察周围环境中目标的分布和速度等相关信息。

# ==============================================================================
# -- CameraManager -------------------------------------------------------------
# ==============================================================================


class CameraManager(object):
    def __init__(self, parent_actor, hud, gamma_correction):
        """
        类的构造函数，用于初始化 `CameraManager` 实例的众多属性，例如传感器对象、显示表面、父级角色关联、抬头显示关联等，
        同时根据父级角色类型（车辆或行人）设置不同的相机变换参数，定义多种传感器及其对应配置，为后续相机图像采集、传感器数据获取等功能做准备。

        参数说明：
        - `parent_actor`：代表父级角色的对象，通常是车辆或者行人等实体，相机和传感器会附着在这个对象上，以获取基于该角色视角的相关图像或数据信息。
        - `hud`：与抬头显示（HUD）相关的对象，可能用于获取显示界面的尺寸等信息，以便对相机图像的显示尺寸等进行相应设置，也可能用于后续展示一些相关提示信息等操作。
        - `gamma_correction`：一个数值，用于进行伽马校正的参数设置，会应用到相机传感器的相关属性配置中，以调整图像的亮度、对比度等显示效果。
        """
        self.sensor = None
        self.surface = None
        self._parent = parent_actor
        self.hud = hud
        self.recording = False
        // 初始化实例的几个属性：
        // - `self.sensor` 初始化为 `None`，后续会在这里存储创建好的相机或其他传感器对象。
        // - `self.surface` 初始化为 `None`，可能用于存储相机获取到的图像数据对应的显示表面（Surface），用于后续图像展示等操作。
        // - `self._parent` 存储传入的父级角色对象，方便后续获取相关世界信息以及将相机、传感器关联到这个角色上。
        // - `self.hud` 存储抬头显示相关对象，用于与抬头显示功能交互，比如获取显示尺寸信息、展示提示信息等。
        // - `self.recording` 初始化为 `False`，从变量名推测可能用于控制是否正在进行图像或数据记录的状态标识，后续可根据需要进行设置来开启或关闭记录功能。

        bound_x = 0.5 + self._parent.bounding_box.extent.x
        bound_y = 0.5 + self._parent.bounding_box.extent.y
        bound_z = 0.5 + self._parent.bounding_box.extent.z
        // 计算相机等传感器在 `x`、`y`、`z` 方向上相对父级角色边界的位置偏移量。通过获取父级角色（`self._parent`）的包围盒（`bounding_box`）的范围（`extent`），并在各方向上加上 `0.5`，确定了相机、传感器在该角色周边大致的安装边界位置，
        // 这样可以根据角色的实际大小和形状来合理放置相机、传感器，以获取期望视角下的图像和数据，避免位置不合理导致拍摄或检测效果不佳的情况。

        Attachment = carla.AttachmentType
        // 获取 `carla.AttachmentType` 枚举类型并赋值给 `Attachment` 变量，这个枚举类型通常用于指定相机等传感器附着到父级角色上的方式，例如是刚性连接（`Rigid`）还是弹性臂连接（`SpringArmGhost`）等，后续会在设置相机变换参数时用到。

        if not self._parent.type_id.startswith("walker.pedestrian"):
            self._camera_transforms = [
                (carla.Transform(carla.Location(x=-2.0*bound_x, y=+0.0*bound_y, z=2.0*bound_z), carla.Rotation(pitch=8.0)), Attachment.SpringArmGhost),
                (carla.Transform(carla.Location(x=+0.8*bound_x, y=+0.0*bound_y, z=1.3*bound_z)), Attachment.Rigid),
                (carla.Transform(carla.Location(x=+1.9*bound_x, y=+1.0*bound_y, z=1.2*bound_z)), Attachment.SpringArmGhost),
                (carla.Transform(carla.Location(x=-2.8*bound_x, y=+0.0*bound_y, z=4.6*bound_z), carla.Rotation(pitch=6.0)), Attachment.SpringArmGhost),
                (carla.Transform(carla.Location(x=-1.0, y=-1.0*bound_y, z=0.4*bound_z)), Attachment.Rigid)]
            // 判断父级角色的类型 ID 是否不以 "walker.pedestrian" 开头，即判断是否不是行人角色，如果是车辆等其他类型角色，则设置相机的变换参数列表（`_camera_transforms`）。
            // 每个元素是一个包含 `carla.Transform`（代表相机的位置和旋转姿态）和 `Attachment`（附着方式）的元组，例如第一个元素中，通过 `carla.Transform` 设置相机在 `x` 轴方向上相对于边界位置偏移 `-2.0*bound_x`，`y` 轴方向偏移 `0.0*bound_y`，`z` 轴方向偏移 `2.0*bound_z`，同时设置俯仰角（`pitch`）为 `8.0` 度，附着方式为 `Attachment.SpringArmGhost`（弹性臂连接），以此定义了不同位置和姿态的相机设置，
            // 这些不同的相机变换配置可以实现从多个角度、不同位置来拍摄获取图像，模拟多视角观察车辆周边环境的效果。

        else:
            self._camera_transforms = [
                (carla.Transform(carla.Location(x=-2.5, z=0.0), carla.Rotation(pitch=-8.0)), Attachment.SpringArmGhost),
                (carla.Transform(carla.Location(x=1.6, z=1.7)), Attachment.Rigid),
                (carla.Transform(carla.Location(x=2.5, y=0.5, z=0.0), carla.Rotation(pitch=-8.0)), Attachment.SpringArmGhost),
                (carla.Transform(carla.Location(x=-4.0, z=2.0), carla.Rotation(pitch=6.0)), Attachment.SpringArmGhost),
                (carla.Transform(carla.Location(x=0, y=-2.5, z=-0.0), carla.Rotation(yaw=90.0)), Attachment.Rigid)]
            // 如果父级角色是行人（类型 ID 以 "walker.pedestrian" 开头），则设置针对行人的相机变换参数列表。同样每个元素包含相机的位置、旋转姿态以及附着方式信息，
            // 不过这里的位置、姿态数值是根据行人角色特点设定的，例如不同的 `x`、`y`、`z` 坐标以及俯仰角（`pitch`）、偏航角（`yaw`）等角度值，使得相机能够以合适的视角拍摄行人周边的情况，满足不同观察需求。

        self.transform_index = 1
        // 初始化实例属性 `self.transform_index` 为 `1`，从变量名推测这个属性可能用于记录当前选择的相机变换参数的索引值，后续可以通过改变这个索引来切换不同视角的相机配置，默认初始化为 `1`，可能对应某个特定的初始视角相机设置。

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
        // 初始化实例属性 `self.sensors`，它是一个包含多个子列表的列表，每个子列表定义了一种传感器的相关信息，包括传感器类型（如相机的不同模式、激光雷达等）、数据处理模式（如 `cc.Raw` 表示原始数据等）、传感器的名称描述以及一个用于设置传感器特定属性的字典（初始为空或包含部分默认属性设置）。
        // 例如第一个子列表 `['sensor.camera.rgb', cc.Raw, 'Camera RGB', {}]` 表示一个普通的彩色相机（`sensor.camera.rgb`），采用原始数据模式（`cc.Raw`），名称为 `Camera RGB`，暂时没有额外的属性设置（属性字典为空），通过这样的列表结构定义了多种不同功能的相机和传感器及其相关配置信息，方便后续基于这些定义去创建和配置实际的传感器对象。

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
                // 通过父级角色对象（`self._parent`）获取其所在的模拟世界对象（`get_world` 方法），然后获取世界的蓝图库（`get_blueprint_library` 方法），用于查找各种传感器的蓝图定义。
                // 遍历 `self.sensors` 列表中的每个传感器定义项（`item`），先在蓝图库中查找对应的传感器蓝图（`bp`），如果传感器类型是以 `sensor.camera` 开头（即表示是相机类型传感器），则进行以下属性设置：
                // - 将相机蓝图的 `image_size_x` 属性设置为抬头显示对象（`hud`）的宽度（`dim[0]`），`image_size_y` 属性设置为抬头显示对象的高度（`dim[1]`），这样相机拍摄的图像尺寸会与显示界面尺寸相适配，便于后续图像展示等操作。
                // - 如果相机蓝图有 `gamma` 属性（通过 `bp.has_attribute('gamma')` 判断），则将其设置为传入的伽马校正参数（`gamma_correction`），用于调整相机图像的亮度、对比度等显示效果。
                // - 遍历传感器定义项中的属性字典（`item[3]`），将其中的每个属性名（`attr_name`）和对应的值（`attr_value`）设置到相机蓝图中，实现对相机的其他特定属性（如可能存在的镜头相关参数等）进行自定义设置。

            elif item[0].startswith('sensor.lidar'):
                self.lidar_range = 50
                for attr_name, attr_value in item[3].items():
                    bp.set_attribute(attr_name, attr_value)
                    if attr_name == 'range':
                        self.lidar_range = float(attr_value)
                // 如果传感器类型是以 `sensor.lidar` 开头（即表示是激光雷达类型传感器），则进行以下操作：
                // - 先初始化实例属性 `self.lidar_range` 为 `50`，作为激光雷达默认的探测范围值（单位可能根据模拟环境设定，比如米等）。
                // - 然后遍历传感器定义项中的属性字典（`item[3]`），将其中的每个属性名（`attr_name`）和对应的值（`attr_value`）设置到激光雷达蓝图中，实现对激光雷达的特定属性进行配置。
                // - 当属性名是 `range`（表示探测范围）时，将实例的 `self.lidar_range` 属性更新为该属性值对应的浮点数类型，这样可以确保 `self.lidar_range` 始终记录着当前激光雷达实际设置的探测范围数值，方便后续在相关功能（如根据范围进行目标检测、显示等）中使用这个准确的范围信息。

            item.append(bp)
            // 将配置好的传感器蓝图（`bp`）添加到对应的传感器定义项（`item`）列表末尾，使得每个传感器定义项不仅包含了初始的类型、模式、名称和属性字典信息，还包含了配置好的蓝图对象，方便后续基于完整的定义去创建实际的传感器。

        self.index = None
        // 初始化实例属性 `self.index` 为 `None`，从变量名推测这个属性可能用于记录当前选择的传感器的索引值，后续可以通过改变这个索引来切换不同功能的传感器，不过目前初始化为 `None`，可能等待后续在具体使用场景中进行赋值和操作。

# ==============================================================================
# -- game_loop() ---------------------------------------------------------------
# ==============================================================================


def game_loop(args):
    """
    主游戏循环函数，负责初始化游戏相关环境（如 Pygame、Carla 客户端等），设置模拟世界的参数，创建游戏中的各种对象（如 HUD、世界对象、控制器等），
    然后进入循环不断更新游戏状态、渲染画面，直到满足退出条件。最后在结束时进行资源清理和环境恢复操作。

    参数说明：
    - `args`：一个包含命令行参数的对象（推测，根据函数中对其属性的使用情况），用于传递诸如主机地址、端口号、是否同步模式、窗口宽度高度、是否开启自动驾驶等配置信息，控制游戏的运行模式和相关设置。
    """
    pygame.init()
    pygame.font.init()
    world = None
    original_settings = None
    // 初始化 Pygame 库，这是用于创建游戏界面、处理输入输出等操作的基础库，`pygame.init()` 会初始化 Pygame 的各个模块。
    // 同时初始化 Pygame 的字体模块（`pygame.font.init()`），方便后续在游戏中进行文本渲染等操作。
    // 初始化 `world` 变量为 `None`，用于后续存储代表游戏世界的对象，`original_settings` 变量也初始化为 `None`，用于保存模拟世界的原始设置信息，以便在游戏结束时恢复初始状态。

    try:
        client = carla.Client(args.host, args.port)
        client.set_timeout(2000.0)
        // 创建一个 Carla 客户端对象，通过传入命令行参数中的主机地址（`args.host`）和端口号（`args.port`）来连接到 Carla 模拟器服务器，
        // 并设置客户端的超时时间为 `2000.0` 毫秒，确保在与服务器通信时如果长时间没有响应能够及时抛出异常，避免程序阻塞。

        sim_world = client.get_world()
        if args.sync:
            original_settings = sim_world.get_settings()
            settings = sim_world.get_settings()
            if not settings.synchronous_mode:
                settings.synchronous_mode = True
                settings.fixed_delta_seconds = 0.05
            sim_world.apply_settings(settings)
            // 获取客户端连接的模拟世界对象（`sim_world`），如果命令行参数指定了同步模式（`args.sync` 为 `True`），则进行以下操作：
            // - 首先获取模拟世界当前的设置信息并保存到 `original_settings` 变量中，以便后续游戏结束时可以恢复这些原始设置。
            // - 再获取一次设置信息到 `settings` 变量（这一步可能是为了后续方便修改设置），检查当前设置中是否没有开启同步模式（`synchronous_mode` 为 `False`），如果是，则将同步模式设置为 `True`，
            // 同时设置固定的时间步长（`fixed_delta_seconds`）为 `0.05` 秒，这意味着在同步模式下，每次模拟世界更新的时间间隔固定为 `0.05` 秒，使模拟更加稳定和可预测。
            // - 最后将修改后的设置应用到模拟世界（`sim_world.apply_settings(settings)`），使其生效。

            traffic_manager = client.get_trafficmanager()
            traffic_manager.set_synchronous_mode(True)
            // 获取 Carla 客户端的交通管理器对象（`traffic_manager`），并将其同步模式也设置为 `True`，确保在同步模式下交通相关的模拟（如车辆行驶、交通信号灯等）也能与模拟世界的更新同步进行，保持整体模拟的一致性。

        if args.autopilot and not sim_world.get_settings().synchronous_mode:
            print("WARNING: You are currently in asynchronous mode and could "
                  "experience some issues with the traffic simulation")
            // 如果命令行参数指定了开启自动驾驶（`args.autopilot` 为 `True`），但当前模拟世界的设置不是同步模式（`sim_world.get_settings().synchronous_mode` 为 `False`），
            // 则打印一条警告信息，提示用户当前处于异步模式，可能会在交通模拟方面遇到一些问题，因为自动驾驶功能在异步模式下可能与交通模拟的协同效果不佳。

        display = pygame.display.set_mode(
            (args.width, args.height),
            pygame.HWSURFACE | pygame.DOUBLEBUF)
        display.fill((0, 0, 0))
        pygame.display.flip()
        // 使用 Pygame 创建游戏显示窗口，设置窗口的尺寸为命令行参数指定的宽度（`args.width`）和高度（`args.height`），
        // 并使用 `pygame.HWSURFACE`（硬件加速表面，利用显卡硬件加速来提高渲染效率）和 `pygame.DOUBLEBUF`（双缓冲模式，避免画面闪烁）标志来优化显示效果。
        // 然后用黑色（`(0, 0, 0)`）填充整个显示窗口，最后通过 `pygame.display.flip()` 方法更新显示窗口，使其显示填充后的黑色画面，完成显示窗口的初始化。

        hud = HUD(args.width, args.height)
        world = World(sim_world, hud, args)
        controller = KeyboardControl(world, args.autopilot)
        // 创建抬头显示（HUD）对象（`hud`），传入窗口宽度和高度参数，用于在游戏界面上显示各种提示信息、状态信息等。
        // 创建游戏世界对象（`world`），将前面获取的模拟世界对象（`sim_world`）、抬头显示对象（`hud`）以及命令行参数（`args`）传入构造函数，这个世界对象可能负责管理游戏世界中的各种实体、场景等内容。
        // 创建键盘控制对象（`controller`），传入游戏世界对象（`world`）和自动驾驶相关参数（`args.autopilot`），用于处理用户通过键盘输入的操作以及控制自动驾驶相关功能（如果开启的话）。

        if args.sync:
            sim_world.tick()
        else:
            sim_world.wait_for_tick()
        // 如果是同步模式（`args.sync` 为 `True`），则手动调用模拟世界的 `tick` 方法，使模拟世界进行一次更新，按照之前设置的固定时间步长推进模拟状态。
        // 如果是异步模式，则调用 `wait_for_tick` 方法，让程序等待模拟世界进行一次更新，确保后续操作是基于最新的模拟世界状态进行的。

        clock = pygame.time.Clock()
        while True:
            if args.sync:
                sim_world.tick()
            clock.tick_busy_loop(60)
            // 创建一个 Pygame 的时钟对象（`clock`），用于控制游戏的帧率等时间相关操作。
            // 进入无限循环，只要游戏运行就不断重复以下操作：
            // - 如果是同步模式，再次调用模拟世界的 `tick` 方法，按照固定时间步长更新模拟世界的状态，使游戏世界中的各种实体、物理模拟等继续推进。
            // - 调用时钟对象的 `tick_busy_loop` 方法，尝试将游戏帧率限制在每秒 `60` 帧左右（尽可能接近这个帧率），通过控制每次循环的时间间隔来实现帧率稳定，保证游戏运行的流畅性。

            if controller.parse_events(client, world, clock, args.sync):
                return
            // 调用键盘控制对象（`controller`）的 `parse_events` 方法，传入客户端对象（`client`）、游戏世界对象（`world`）、时钟对象（`clock`）以及同步模式参数（`args.sync`），
            // 这个方法用于处理用户键盘输入事件、更新游戏状态等操作，如果在处理过程中满足了退出游戏的条件（例如用户按下了退出键等），则该方法返回 `True`，此时就会跳出游戏循环，结束游戏。

            world.tick(clock)
            world.render(display)
            pygame.display.flip()
            // 调用游戏世界对象（`world`）的 `tick` 方法，传入时钟对象（`clock`），在游戏世界内部进行各种实体更新、逻辑处理等操作，使游戏世界的状态根据时间推进而更新。
            // 调用游戏世界对象的 `render` 方法，传入显示窗口对象（`display`），将游戏世界中的场景、实体等内容渲染绘制到显示窗口上，实现游戏画面的展示。
            // 最后调用 `pygame.display.flip()` 方法更新显示窗口，使渲染后的新画面显示出来，完成一帧画面的更新和显示，不断循环这个过程就实现了游戏的动态画面展示效果。

    finally:
        if original_settings:
            sim_world.apply_settings(original_settings)
        // 在无论游戏正常结束还是出现异常结束的情况下（通过 `finally` 块确保执行），如果之前保存了模拟世界的原始设置信息（`original_settings` 不为 `None`），
        // 则将原始设置重新应用到模拟世界（`sim_world.apply_settings(original_settings)`），恢复模拟世界的初始状态，避免对下次使用产生影响。

        if (world and world.recording_enabled):
            client.stop_recorder()
        // 如果游戏世界对象（`world`）存在并且其记录功能（`recording_enabled` 属性为 `True`，推测是用于控制是否正在进行游戏过程记录的标识）是开启的，
        // 则通过客户端对象（`client`）调用 `stop_recorder` 方法，停止游戏过程的记录操作（如果之前正在记录的话）。

        if world is not None:
            world.destroy()
        // 如果游戏世界对象（`world`）不为 `None`，则调用其 `destroy` 方法，进行游戏世界相关资源的清理和销毁操作，释放内存等资源，避免内存泄漏等问题。

        pygame.quit()
        // 最后调用 `pygame.quit()` 方法，关闭 Pygame 库，释放 Pygame 使用的相关资源，彻底结束游戏程序。


# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================


def main():
    """
    整个程序的主函数，作为程序的入口点，主要负责解析命令行参数、配置日志级别、输出服务器连接相关信息、打印文档字符串（如果有的话），
    并尝试启动游戏循环，同时处理用户通过键盘中断程序时的异常情况，以友好的方式结束程序。
    """
    argparser = argparse.ArgumentParser(
        description='CARLA Manual Control Client')
    // 创建一个 `argparse.ArgumentParser` 对象，用于解析命令行参数。传入 `description` 参数，为这个参数解析器设置一个描述性的文本，
    // 说明这个程序是用于 `CARLA` 手动控制客户端相关功能的，方便用户在查看帮助信息时了解程序的大致用途。

    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='print debug information')
    // 向参数解析器添加一个命令行参数选项 `-v` 或 `--verbose`，当用户在命令行中使用这个选项时，其行为是将对应的值存储为 `True`，并将这个参数对应的变量名设置为 `debug`（通过 `dest` 参数指定），
    // 它的作用是用于控制是否打印调试信息，若指定了该选项，则后续程序运行过程中会输出更多详细的调试相关内容，方便开发者排查问题。

    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    // 添加 `--host` 命令行参数选项，通过 `metavar` 参数指定在帮助信息中显示的参数名称占位符为 `H`，设置默认值为 `127.0.0.1`，
    // 该参数用于指定要连接的主机服务器的 `IP` 地址，用户可以通过命令行传入自定义的 `IP` 地址来连接不同的服务器，如果不传则使用默认的本地地址 `127.0.0.1`。

    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    // 添加 `-p` 或 `--port` 命令行参数选项，`metavar` 设置参数名称占位符为 `P`，默认值为 `2000`，并且指定参数类型为整数（`type=int`），
    // 这个参数用于指定要监听的 `TCP` 端口号，用户可以传入其他端口号来改变程序监听的端口，若不传则使用默认的 `2000` 端口与服务器通信。

    argparser.add_argument(
        '-a', '--autopilot',
        action='store_true',
        help='enable autopilot')
    // 添加 `-a` 或 `--autopilot` 命令行参数选项，使用 `action='store_true'` 表示当用户指定该选项时，对应的值存储为 `True`，
    // 其作用是用于控制是否开启自动驾驶功能，若在命令行中指定了这个选项，则程序在运行过程中会启用相关的自动驾驶逻辑（前提是程序具备相应功能实现）。

    argparser.add_argument(
        '--res',
        metavar='WIDTHxHEIGHT',
        default='1280x720',
        help='window resolution (default: 1280x720)')
    // 添加 `--res` 命令行参数选项，`metavar` 指定参数在帮助信息中的占位符形式为 `WIDTHxHEIGHT`，默认值为 `1280x720`，
    // 该参数用于设置程序窗口的分辨率，用户可以按照 `宽度x高度` 的格式传入自定义的分辨率值，若不传则使用默认的 `1280x720` 分辨率来显示窗口内容。

    argparser.add_argument(
        '--filter',
        metavar='PATTERN',
        default='vehicle.*',
        help='actor filter (default: "vehicle.*")')
    // 添加 `--filter` 命令行参数选项，`metavar` 设置参数占位符为 `PATTERN`，默认值为 `vehicle.*`，
    // 这个参数用于设置对游戏中角色（`actor`）的筛选模式，例如默认按照 `vehicle.*` 的模式会筛选出所有类型为车辆相关的角色，用户可以传入其他的正则表达式模式来改变筛选的角色类型，以满足不同的场景需求。

    argparser.add_argument(
        '--generation',
        metavar='G',
        default='2',
        help='restrict to certain actor generation (values: "1","2","All" - default: "2")')
    // 添加 `--generation` 命令行参数选项，`metavar` 设置参数占位符为 `G`，默认值为 `2`，
    // 该参数用于限制游戏中角色所属的特定代次，可选择的值有 `"1"`、`"2"`、`"All"`，用户可以通过传入相应的值来指定只使用特定代次的角色，若不传则默认使用第 `2` 代角色，不同代次的角色可能在模型、功能等方面有所不同。

    argparser.add_argument(
        '--rolename',
        metavar='NAME',
        default='hero',
        help='actor role name (default: "hero")')
    // 添加 `--rolename` 命令行参数选项，`metavar` 设置参数占位符为 `NAME`，默认值为 `hero`，
    // 这个参数用于指定游戏中角色的角色名，默认角色名设置为 `"hero"`，用户可以传入其他自定义的角色名来改变角色的标识，在一些需要区分不同角色的场景中会用到。

    argparser.add_argument(
        '--gamma',
        default=2.2,
        type=float,
        help='Gamma correction of the camera (default: 2.2)')
    // 添加 `--gamma` 命令行参数选项，默认值为 `2.2`，指定参数类型为浮点数（`type=float`），
    // 该参数用于设置相机的伽马校正值，伽马校正会影响相机拍摄图像的亮度、对比度等显示效果，用户可以传入其他浮点数来调整校正程度，若不传则使用默认的 `2.2` 值。

    argparser.add_argument(
        '--sync',
        action='store_true',
        help='Activate synchronous mode execution')
    // 添加 `--sync` 命令行参数选项，使用 `action='store_true'` 表示指定该选项时对应的值存储为 `True`，
    // 其作用是用于控制是否激活同步模式执行，在同步模式下，游戏世界的更新、渲染等操作会按照固定的时间步长等规则同步进行，若用户在命令行中指定该选项，则开启同步模式相关逻辑。

    args = argparser.parse_args()
    // 调用 `argparser` 的 `parse_args` 方法，解析命令行传入的参数，并将解析后的参数值存储到 `args` 对象中，后续程序可以通过访问 `args` 对象的不同属性来获取相应的参数值，用于控制程序的各种行为和设置。

    args.width, args.height = [int(x) for x in args.res.split('x')]
    // 从解析后的 `args` 对象中获取 `res` 属性（代表窗口分辨率的字符串，格式如 `1280x720`），通过 `split('x')` 方法将其按照 `x` 字符分割为宽度和高度的字符串列表，
    // 然后使用列表推导式将每个字符串转换为整数类型，并分别赋值给 `args` 对象的 `width` 和 `height` 属性，这样就将窗口分辨率的字符串参数解析为了具体的宽度和高度数值，方便后续使用。

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)
    // 根据 `args` 对象中的 `debug` 属性值来确定日志级别。如果 `debug` 属性为 `True`（即用户在命令行指定了 `-v` 或 `--verbose` 选项），则将日志级别设置为 `DEBUG`，
    // 这样会输出更详细的调试信息；否则将日志级别设置为 `INFO`，只输出一般的程序运行信息。然后通过 `logging.basicConfig` 方法配置日志的基本格式（按照级别和消息内容的格式显示）以及设定的日志级别，
    // 使得程序在运行过程中能够按照相应的级别输出日志信息，方便查看程序状态和排查问题。

    logging.info('listening to server %s:%s', args.host, args.port)
    // 使用配置好的日志系统，按照 `INFO` 级别输出一条信息，告知用户程序正在监听的服务器地址和端口号，即显示当前程序准备连接的服务器相关信息，方便用户确认连接情况。

    print(__doc__)
    // 打印当前模块（可能是整个程序所在的主模块）的文档字符串（`__doc__`），通常文档字符串包含了对程序功能、模块用途等方面的描述性内容，将其打印出来可以让用户更直观地了解程序的大致情况。

    try:
        game_loop(args)
        // 尝试执行 `game_loop` 函数，并将解析好的命令行参数 `args` 传入，`game_loop` 函数应该是整个程序的核心游戏循环逻辑所在，在这里启动游戏的主要运行过程，
        // 如果在 `game_loop` 函数执行过程中出现异常，则会被下面的 `except` 块捕获并进行相应处理。

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
        // 捕获 `KeyboardInterrupt` 异常，这个异常通常在用户通过键盘（比如按下 `Ctrl+C`）中断程序运行时抛出。
        // 当捕获到这个异常时，打印一条友好的提示信息告知用户程序是被其手动取消的，然后结束程序，以一种比较友好的方式处理用户主动中断程序的情况。


if __name__ == '__main__':
    main()
    // 这是 Python 程序中常见的入口点判断语句，当脚本直接运行（而不是被作为模块导入到其他程序中）时，`__name__` 的值会是 `"__main__"`，
    // 此时就会执行 `main` 函数，启动整个程序的运行逻辑，实现程序从入口开始执行相应的功能。
