#!/usr/bin/env python

# Copyright (c) 2018 Intel Labs.
# authors: German Ros (german.ros@intel.com)
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Example of automatic vehicle control from client side."""

from __future__ import print_function

import argparse
import collections
import datetime
import glob
import logging
import math
import os
import numpy.random as random
import re
import sys
import weakref

try:
    import pygame
    from pygame.locals import KMOD_CTRL
    from pygame.locals import K_ESCAPE
    from pygame.locals import K_q
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError(
        'cannot import numpy, make sure numpy package is installed')

# ==============================================================================
# -- Find CARLA module ---------------------------------------------------------
# ==============================================================================
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

# ==============================================================================
# -- Add PythonAPI for release mode --------------------------------------------
# ==============================================================================
try:
# 尝试将当前文件所在目录的上一级目录添加到系统路径中，以便能够导入 'carla' 模块。
    sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + '/carla')
except IndexError:
    pass

import carla
from carla import ColorConverter as cc

from agents.navigation.behavior_agent import BehaviorAgent  # pylint: disable=import-error
from agents.navigation.basic_agent import BasicAgent  # pylint: disable=import-error
from agents.navigation.constant_velocity_agent import ConstantVelocityAgent  # pylint: disable=import-error


# ==============================================================================
# -- Global functions ----------------------------------------------------------
# ==============================================================================


def find_weather_presets():
    """
    Method to find weather presets

    该函数通过正则表达式将类属性名（采用驼峰命名法）转换为更易读的格式（单词之间用空格分隔），
    然后从 `carla.WeatherParameters` 类中获取所有符合特定命名规范（以大写字母开头）的属性名，
    并将属性值和对应的可读名称组成元组放入列表返回，代表不同的天气预设情况。
    """
    rgx = re.compile('.+?(?:(?<=[a-z])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])|$)')
    def name(x): return ' '.join(m.group(0) for m in rgx.finditer(x))
    presets = [x for x in dir(carla.WeatherParameters) if re.match('[A-Z].+', x)]
    return [(getattr(carla.WeatherParameters, x), name(x)) for x in presets]


def get_actor_display_name(actor, truncate=250):
    """
    Method to get actor display name

    该函数获取给定 `actor` 的类型 ID，将其中的下划线替换为点，然后提取并转换为标题格式（单词首字母大写），
    最后根据截断长度限制返回合适的显示名称，若超过截断长度则添加省略号。
    """
    name = ' '.join(actor.type_id.replace('_', '.').title().split('.')[1:])
    return (name[:truncate - 1] + u'\u2026') if len(name) > truncate else name

def get_actor_blueprints(world, filter, generation):
    """
    根据给定的世界 `world`、筛选条件 `filter` 和生成版本 `generation` 获取符合要求的 actor 蓝图列表。

    首先通过 `filter` 从世界的蓝图库中筛选出符合条件的蓝图列表 `bps`。
    如果 `generation` 参数为 `"all"`，则直接返回筛选后的所有蓝图。
    如果 `bps` 列表中只有一个蓝图，就直接返回该蓝图，忽略生成版本的检查。
    否则尝试将 `generation` 转换为整数，检查其是否是有效的生成版本（1、2、3），
    如果是，则进一步筛选出对应生成版本的蓝图并返回；若不是有效版本，则打印警告信息并返回空列表。
    """
    bps = world.get_blueprint_library().filter(filter)

    if generation.lower() == "all":
        return bps

    # If the filter returns only one bp, we assume that this one needed
    # and therefore, we ignore the generation
    if len(bps) == 1:
        return bps

    try:
        int_generation = int(generation)
        # Check if generation is in available generations
        if int_generation in [1, 2, 3]:
            bps = [x for x in bps if int(x.get_attribute('generation')) == int_generation]
            return bps
        else:
            print("   Warning! Actor Generation is not valid. No actor will be spawned.")
            return []
    except:
        print("   Warning! Actor Generation is not valid. No actor will be spawned.")
        return []

# ==============================================================================
# -- World ---------------------------------------------------------------
# ==============================================================================

class World(object):
    """ Class representing the surrounding environment """

    def __init__(self, carla_world, hud, args):
        """
        Constructor method

        参数：
        - `carla_world`：Carla世界对象，代表整个模拟环境。
        - `hud`：HUD对象，用于显示相关信息（如文字提示等）。
        - `args`：命令行参数对象，包含一些运行时的配置参数。

        在构造函数中，初始化了诸多实例变量，包括保存传入的参数、获取地图对象、初始化传感器等相关对象的引用，
        查找天气预设情况，记录当前天气索引、actor筛选条件和生成版本等信息，然后调用 `restart` 方法来初始化场景。
        """
        self._args = args
        self.world = carla_world
        try:
            self.map = self.world.get_map()
        except RuntimeError as error:
            print('RuntimeError: {}'.format(error))
            print('  The server could not send the OpenDRIVE (.xodr) file:')
            print('  Make sure it exists, has the same name of your town, and is correct.')
            sys.exit(1)
        self.hud = hud
        self.player = None
        self.collision_sensor = None
        self.lane_invasion_sensor = None
        self.gnss_sensor = None
        self.camera_manager = None
        self._weather_presets = find_weather_presets()
        self._weather_index = 0
        self._actor_filter = args.filter
        self._actor_generation = args.generation
        self.restart(args)
        self.world.on_tick(hud.on_world_tick)
        self.recording_enabled = False
        self.recording_start = 0

    def restart(self, args):
        """
        Restart the world

        该方法用于重新初始化整个世界场景，具体步骤如下：
        1. 尝试保持之前的相机配置（如果相机管理器存在的话），获取对应的索引和变换索引。
        2. 通过 `get_actor_blueprints` 方法获取符合条件的 actor 蓝图列表，若找不到则抛出异常。
        3. 从蓝图列表中随机选择一个蓝图，设置其角色名为 `hero`，如果蓝图有颜色属性，则随机选择一个推荐颜色并设置。
        4. 尝试在之前玩家所在位置（如果存在）上方一定高度处重新生成玩家（车辆），若生成失败，则从地图的随机出生点（如果有）尝试重新生成，
           生成过程中会调用 `modify_vehicle_physics` 方法设置车辆物理属性，直到成功生成玩家。
        5. 根据是否同步模式，等待世界的一次 tick（更新）。
        6. 设置各种传感器（碰撞传感器、车道入侵传感器、全球导航卫星系统传感器、相机管理器等），并更新相关的显示信息。
        """
        # Keep same camera config if the camera manager exists.
        cam_index = self.camera_manager.index if self.camera_manager is not None else 0
        cam_pos_id = self.camera_manager.transform_index if self.camera_manager is not None else 0

        # Get a random blueprint.
        blueprint_list = get_actor_blueprints(self.world, self._actor_filter, self._actor_generation)
        if not blueprint_list:
            raise ValueError("Couldn't find any blueprints with the specified filters")
        blueprint = random.choice(blueprint_list)
        blueprint.set_attribute('role_name', 'hero')
        if blueprint.has_attribute('color'):
            color = random.choice(blueprint.get_attribute('color').recommended_values)
            blueprint.set_attribute('color', color)

        # Spawn the player.
        if self.player is not None:
            spawn_point = self.player.get_transform()
            spawn_point.location.z += 2.0
            spawn_point.rotation.roll = 0.0
            spawn_point.rotation.pitch = 0.0
            self.destroy()
            self.player = self.world.try_spawn_actor(blueprint, spawn_point)
            self.modify_vehicle_physics(self.player)
        while self.player is None:
            if not self.map.get_spawn_points():
                print('There are no spawn points available in your map/town.')
                print('Please add some Vehicle Spawn Point to your UE4 scene.')
                sys.exit(1)
            spawn_points = self.map.get_spawn_points()
            spawn_point = random.choice(spawn_points) if spawn_points else carla.Transform()
            self.player = self.world.try_spawn_actor(blueprint, spawn_point)
            self.modify_vehicle_physics(self.player)

        if self._args.sync:
            self.world.tick()
        else:
            self.world.wait_for_tick()

        # Set up the sensors.
        self.collision_sensor = CollisionSensor(self.player, self.hud)
        self.lane_invasion_sensor = LaneInvasionSensor(self.player, self.hud)
        self.gnss_sensor = GnssSensor(self.player)
        self.camera_manager = CameraManager(self.player, self.hud)
        self.camera_manager.transform_index = cam_pos_id
        self.camera_manager.set_sensor(cam_index, notify=False)
        actor_type = get_actor_display_name(self.player)
        self.hud.notification(actor_type)

    def next_weather(self, reverse=False):
        """
        Get next weather setting

        根据传入的 `reverse` 参数决定是向前还是向后切换天气预设，更新天气索引，并获取对应的天气预设，
        通过HUD显示当前切换到的天气名称，然后在游戏世界中设置相应的天气。
        """
        self._weather_index += -1 if reverse else 1
        self._weather_index %= len(self._weather_presets)
        preset = self._weather_presets[self._weather_index]
        self.hud.notification('Weather: %s' % preset[1])
        self.player.get_world().set_weather(preset[0])

    def modify_vehicle_physics(self, actor):
        """
        如果传入的 `actor` 是车辆（通过尝试获取其物理控制来判断），则设置其物理控制属性 `use_sweep_wheel_collision` 为 `True`，
        并应用该物理控制设置，若 `actor` 不是车辆，捕获异常并忽略（不进行设置操作）。
        """
        # If actor is not a vehicle, we cannot use the physics control
        try:
            physics_control = actor.get_physics_control()
            physics_control.use_sweep_wheel_collision = True
            actor.apply_physics_control(physics_control)
        except Exception:
            pass

    def tick(self, clock):
        """
        Method for every tick

        在每一帧（tick）更新时，调用HUD的 `tick` 方法来更新相关显示信息（如帧率、模拟时间等）。
        """
        self.hud.tick(self, clock)

    def render(self, display):
        """
        Render world

        调用相机管理器的 `render` 方法来渲染相机画面，并调用HUD的 `render` 方法来渲染HUD上的相关信息（如文字提示等）到给定的显示 `display` 上。
        """
        self.camera_manager.render(display)
        self.hud.render(display)

    def destroy_sensors(self):
        """
        Destroy sensors

        销毁相机管理器中的传感器，将其相关引用设置为 `None`，释放相关资源。
        """
        self.camera_manager.sensor.destroy()
        self.camera_manager.sensor = None
        self.camera_manager.index = None

    def destroy(self):
        """
        Destroys all actors

        遍历并销毁一系列相关的 actor（包括相机传感器、碰撞传感器、车道入侵传感器、全球导航卫星系统传感器以及玩家车辆），
        释放资源，清理场景。
        """
        actors = [
            self.camera_manager.sensor,
            self.collision_sensor.sensor,
            self.lane_invasion_sensor.sensor,
            self.gnss_sensor.sensor,
            self.player]
        for actor in actors:
            if actor is not None:
                actor.destroy()


# ==============================================================================
# -- KeyboardControl -----------------------------------------------------------
# ==============================================================================


class KeyboardControl(object):
    def __init__(self, world):
        """
        初始化 `KeyboardControl` 类，在初始化时通过HUD显示一条帮助提示信息，告知用户按下 'H' 或 '?' 可查看更多帮助内容。
        """
        world.hud.notification("Press 'H' or '?' for help.", seconds=4.0)

    def parse_events(self):
        """
        解析 `pygame` 事件队列中的事件。

        遍历所有事件，如果事件类型是 `pygame.QUIT`（表示窗口关闭事件），则返回 `True`，表示要退出程序。
        如果事件类型是 `pygame.KEYUP`（表示按键松开事件），则调用 `_is_quit_shortcut` 方法判断是否按下了退出快捷键（如 `ESC` 键或 `Ctrl + Q` 组合键），
        如果是则返回 `True`，表示要退出程序，否则继续处理后续事件。
        """
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return True
            if event.type == pygame.KEYUP:
                if self._is_quit_shortcut(event.key):
                    return True

    @staticmethod
    def _is_quit_shortcut(key):
        """
        Shortcut for quitting

        判断给定的按键 `key` 是否是退出快捷键（`ESC` 键或 `Ctrl + Q` 组合键），如果是则返回 `True`，否则返回 `False`。
        """
        return (key == K_ESCAPE) or (key == K_q and pygame.key.get_mods() & KMOD_CTRL)

# ==============================================================================
# -- HUD -----------------------------------------------------------------------
# ==============================================================================


class HUD(object):
    """Class for HUD text"""

    def __init__(self, width, height):
        """
        Constructor method

        参数：
        - `width`：显示窗口的宽度，用于定位HUD元素的显示位置等。
        - `height`：显示窗口的高度，同样用于定位HUD元素的显示位置等。

        在构造函数中，初始化了HUD相关的一些属性，包括尺寸、字体、用于显示提示信息的 `FadingText` 对象、帮助文本 `HelpText` 对象，
        以及一些用于记录服务器帧率、当前帧数、模拟时间等信息的变量，并设置了是否显示信息的标志位和信息文本列表等。
        """
        self.dim = (width, height)
        font = pygame.font.Font(pygame.font.get_default_font(), 20)
        font_name = 'courier' if os.name == 'nt' else 'mono'
        fonts = [x for x in pygame.font.get_fonts() if font_name in x]
        default_font = 'ubuntumono'
        mono = default_font if default_font in fonts else fonts[0]
        mono = pygame.font.match_font(mono)
        self._font_mono = pygame.font.Font(mono, 12 if os.name == 'nt' else 14)
        self._notifications = FadingText(font, (width, 40), (0, height - 40))
        self.help = HelpText(pygame.font.Font(mono, 24), width, height)
        self.server_fps = 0
        self.frame = 0
        self.simulation_time = 0
        self._show_info = True
        self._info_text = []
        self._server_clock = pygame.time.Clock()

    def on_world_tick(self, timestamp):
    """
    Gets informations from the world at every tick

    该方法在每一次世界更新（tick）时被调用，用于获取世界相关的信息。
    具体操作如下：
    1. 调用 `_server_clock` 的 `tick` 方法，这个方法可能用于更新某种计时相关的逻辑（比如帧率计算等的时间间隔更新）。
    2. 获取并记录服务器的帧率，通过 `_server_clock` 的 `get_fps` 方法获取当前帧率并赋值给 `server_fps` 属性。
    3. 获取并记录当前的帧数，直接从传入的 `timestamp` 参数中获取帧计数赋值给 `frame` 属性。
    4. 获取并记录模拟时间，从 `timestamp` 参数中获取经过的秒数赋值给 `simulation_time` 属性。
    """
    self._server_clock.tick()
    self.server_fps = self._server_clock.get_fps()
    self.frame = timestamp.frame_count
    self.simulation_time = timestamp.elapsed_seconds

def tick(self, world, clock):
    """
    HUD method for every tick

    这是HUD在每一次tick时执行的主要方法，用于更新HUD上显示的各种信息，具体步骤如下：
    1. 首先调用 `_notifications` 的 `tick` 方法，可能用于更新一些渐隐提示信息相关的逻辑（比如更新剩余显示时间等），传入当前世界和时钟对象。
    2. 根据 `_show_info` 标志判断是否需要显示详细信息，如果为 `False` 则直接返回，不进行后续信息更新操作。
    3. 获取玩家车辆的相关信息，包括位置变换信息（`transform`）、速度信息（`vel`）、控制信息（`control`）等，用于后续信息的组装和显示。
    4. 根据车辆的朝向角度（`yaw`）计算并确定车辆的朝向方向（如 `N`、`S`、`E`、`W` 等），组合成 `heading` 字符串。
    5. 获取碰撞历史信息（`colhist`），并处理最近200帧的碰撞数据，进行归一化处理，得到 `collision` 列表，用于显示碰撞相关情况。
    6. 获取世界中所有的车辆actor，通过过滤 `vehicle.*` 类型来实现，用于后续显示车辆数量以及附近车辆信息等。
    7. 组装 `_info_text` 列表，里面包含了服务器帧率、客户端帧率、车辆名称、地图名称、模拟时间、速度、朝向、位置、GNSS信息、高度等各种详细信息，
       根据车辆控制类型（是车辆控制还是行人控制）添加不同的控制相关信息，最后添加碰撞信息和车辆数量等内容。
    8. 如果车辆数量大于1，进一步计算并添加附近车辆的距离和类型信息，按照距离排序，只添加距离在200.0以内的车辆信息到 `_info_text` 列表中。
    """
    self._notifications.tick(world, clock)
    if not self._show_info:
        return
    transform = world.player.get_transform()
    vel = world.player.get_velocity()
    control = world.player.get_control()
    heading = 'N' if abs(transform.rotation.yaw) < 89.5 else ''
    heading += 'S' if abs(transform.rotation.yaw) > 90.5 else ''
    heading += 'E' if 179.5 > transform.rotation.yaw > 0.5 else ''
    heading += 'W' if -0.5 > transform.rotation.yaw > -179.5 else ''
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
        'Speed:   % 15.0f km/h' % (3.6 * math.sqrt(vel.x**2 + vel.y**2 + vel.z**2)),
        u'Heading:% 16.0f\N{DEGREE SIGN} % 2s' % (transform.rotation.yaw, heading),
        'Location:% 20s' % ('(% 5.1f, % 5.1f)' % (transform.location.x, transform.location.y)),
        'GNSS:% 24s' % ('(% 2.6f, % 3.6f)' % (world.gnss_sensor.lat, world.gnss_sensor.lon)),
        'Height:  % 18.0f m' % transform.location.z,
        '']
    if isinstance(control, carla.VehicleControl):
        self._info_text += [
            ('Throttle:', control.throttle, 0.0, 1.0),
            ('Steer:', control.steer, -1.0, 1.0),
            ('Brake:', control.brake, 0.0, 1.0),
            ('Reverse:', control.reverse),
            ('Hand brake:', control.hand_brake),
            ('Manual:', control.manual_gear_shift),
            'Gear:        %s' % {-1: 'R', 0: 'N'}.get(control.gear, control.gear)]
    elif isinstance(control, carla.WalkerControl):
        self._info_text += [
            ('Speed:', control.speed, 0.0, 5.556),
            ('Jump:', control.jump)]
    self._info_text += [
        '',
        'Collision:',
        collision,
        '',
        'Number of vehicles: % 8d' % len(vehicles)]

    if len(vehicles) > 1:
        self._info_text += ['Nearby vehicles:']

    def dist(l):
        return math.sqrt((l.x - transform.location.x)**2 + (l.y - transform.location.y)
                         ** 2 + (l.z - transform.location.z)**2)
    vehicles = [(dist(x.get_location()), x) for x in vehicles if x.id!= world.player.id]

    for dist, vehicle in sorted(vehicles):
        if dist > 200.0:
            break
        vehicle_type = get_actor_display_name(vehicle, truncate=22)
        self._info_text.append('% 4dm %s' % (dist, vehicle_type))

def toggle_info(self):
    """
    Toggle info on or off

    用于切换 `_show_info` 标志的状态，即实现显示或隐藏HUD上详细信息的功能，通过取反操作来改变其当前值。
    """
    self._show_info = not self._show_info

def notification(self, text, seconds=2.0):
    """
    Notification text

    用于设置显示提示信息，调用 `_notifications` 对象的 `set_text` 方法，传入要显示的文本内容以及显示时长（默认2.0秒），
    让 `_notifications` 对象来处理提示信息的显示逻辑。
    """
    self._notifications.set_text(text, seconds=seconds)

def error(self, text):
    """
    Error text

    用于显示错误信息，调用 `_notifications` 对象的 `set_text` 方法，将传入的文本内容包装成错误格式（添加 `Error:` 前缀），
    并设置颜色为红色（`(255, 0, 0)`），让 `_notifications` 对象按照设定来显示错误提示信息。
    """
    self._notifications.set_text('Error: %s' % text, (255, 0, 0))

def render(self, display):
    """
    Render for HUD class

    该方法用于渲染HUD上的所有信息到给定的显示 `display` 上，具体操作如下：
    1. 根据 `_show_info` 标志判断是否需要显示详细信息，如果为 `False`，则跳过详细信息渲染，直接渲染提示信息和帮助信息。
    2. 如果需要显示详细信息，首先创建一个半透明的表面（`info_surface`）用于承载详细信息文本，设置其透明度、位置等属性，然后将其绘制到 `display` 上。
    3. 通过循环遍历 `_info_text` 列表中的每一项，根据其数据类型（字符串、列表、元组等）进行不同的渲染操作：
       - 如果是列表类型且长度大于1，可能用于绘制一些图表之类的数据，通过计算坐标点绘制折线图。
       - 如果是元组类型，根据元组中的内容判断是绘制布尔类型的方块（比如手刹状态等）还是绘制数值类型的进度条（比如油门、刹车、转向等控制量），进行相应的图形绘制操作。
       - 如果是字符串类型，则直接使用设定的字体渲染文本，并绘制到合适的位置上。
    4. 最后分别调用 `_notifications` 和 `help` 对象的 `render` 方法，将提示信息和帮助信息也渲染到 `display` 上，完成整个HUD的渲染过程。
    """
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
                    points = [(x + 8, v_offset + 8 + (1 - y) * 30) for x, y in enumerate(item)]
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
                    fig = (item[1] - item[2]) / (item[3] - item[2])
                    if item[2] < 0.0:
                        rect = pygame.Rect(
                            (bar_h_offset + fig * (bar_width - 6), v_offset + 8), (6, 6))
                    else:
                        rect = pygame.Rect((bar_h_offset, v_offset + 8), (fig * bar_width, 6))
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
    """ Class for fading text """

    def __init__(self, font, dim, pos):
        """
        Constructor method

        参数：
        - `font`：用于渲染文本的字体对象，决定了文本的字体样式等。
        - `dim`：一个包含宽度和高度的元组，表示要创建的用于显示文本的表面（Surface）的尺寸大小。
        - `pos`：一个包含x和y坐标的元组，表示文本在显示区域中的位置。

        在构造函数中，初始化了文本相关的属性，包括字体、尺寸、位置、剩余显示时间以及创建了一个初始透明的用于绘制文本的表面对象。
        """
        self.font = font
        self.dim = dim
        self.pos = pos
        self.seconds_left = 0
        self.surface = pygame.Surface(self.dim)

    def set_text(self, text, color=(255, 255, 255), seconds=2.0):
        """
        Set fading text

        该方法用于设置要显示的渐隐文本内容及相关属性，具体步骤如下：
        1. 使用传入的字体和文本内容渲染出文本纹理（`text_texture`），设置其颜色等样式。
        2. 重新创建一个用于显示文本的表面对象，填充为完全透明（`(0, 0, 0, 0)`），以清除之前可能存在的内容。
        3. 将渲染好的文本纹理绘制到新创建的表面对象上，指定合适的位置（偏移量）。
        4. 设置文本的剩余显示时间为传入的 `seconds` 参数值（默认2.0秒）。
        """
        text_texture = self.font.render(text, True, color)
        self.surface = pygame.Surface(self.dim)
        self.seconds_left = seconds
        self.surface.fill((0, 0, 0, 0))
        self.surface.blit(text_texture, (10, 11))

    def tick(self, _, clock):
        """
        Fading text method for every tick

        在每一次tick时被调用，用于更新渐隐文本的剩余显示时间以及透明度，具体操作如下：
        1. 获取自上次tick以来经过的时间（以秒为单位），通过 `clock` 对象的 `get_time` 方法获取毫秒数并转换为秒数（乘以 `1e-3`）。
        2. 根据经过的时间减少剩余显示时间，确保剩余时间不会小于0.0，使用 `max` 函数来限制最小值。
        3. 根据剩余显示时间更新文本表面的透明度，透明度与剩余时间成正比（乘以一个系数 `500.0`），实现渐隐效果。
        """
        delta_seconds = 1e-3 * clock.get_time()
        self.seconds_left = max(0.0, self.seconds_left - delta_seconds)
        self.surface.set_alpha(500.0 * self.seconds_left)

    def render(self, display):
        """
        Render fading text method

        将带有渐隐效果的文本表面绘制到给定的显示 `display` 上，实现文本在屏幕上的显示，传入的 `display` 通常是整个游戏窗口或相关的显示区域对象。
        """
        display.blit(self.surface, self.pos)

# ==============================================================================
# -- HelpText ------------------------------------------------------------------
# ==============================================================================

class HelpText(object):
    """ Helper class for text render"""

    def __init__(self, font, width, height):
        """Constructor method"""
        lines = __doc__.split('\n')
        self.font = font
        self.dim = (680, len(lines) * 22 + 12)
        self.pos = (0.5 * width - 0.5 * self.dim[0], 0.5 * height - 0.5 * self.dim[1])
        self.seconds_left = 0
        self.surface = pygame.Surface(self.dim)
        self.surface.fill((0, 0, 0, 0))
        for i, line in enumerate(lines):
            text_texture = self.font.render(line, True, (255, 255, 255))
            self.surface.blit(text_texture, (22, i * 22))
            self._render = False
        self.surface.set_alpha(220)

    def toggle(self):
        """Toggle on or off the render help"""
        self._render = not self._render

    def render(self, display):
        """Render help text method"""
        if self._render:
            display.blit(self.surface, self.pos)

# ==============================================================================
# -- CollisionSensor -----------------------------------------------------------
# ==============================================================================


class CollisionSensor(object):
    """ Class for collision sensors"""

    def __init__(self, parent_actor, hud):
        """Constructor method"""
        self.sensor = None
        self.history = []
        self._parent = parent_actor
        self.hud = hud
        world = self._parent.get_world()
        blueprint = world.get_blueprint_library().find('sensor.other.collision')
        self.sensor = world.spawn_actor(blueprint, carla.Transform(), attach_to=self._parent)
        # We need to pass the lambda a weak reference to
        # self to avoid circular reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda event: CollisionSensor._on_collision(weak_self, event))

    def get_collision_history(self):
        """Gets the history of collisions"""
        history = collections.defaultdict(int)
        for frame, intensity in self.history:
            history[frame] += intensity
        return history

    @staticmethod
    def _on_collision(weak_self, event):
        """On collision method"""
        self = weak_self()
        if not self:
            return
        actor_type = get_actor_display_name(event.other_actor)
        self.hud.notification('Collision with %r' % actor_type)
        impulse = event.normal_impulse
        intensity = math.sqrt(impulse.x ** 2 + impulse.y ** 2 + impulse.z ** 2)
        self.history.append((event.frame, intensity))
        if len(self.history) > 4000:
            self.history.pop(0)

# ==============================================================================
# -- LaneInvasionSensor --------------------------------------------------------
# ==============================================================================


class LaneInvasionSensor(object):
    """Class for lane invasion sensors"""

    def __init__(self, parent_actor, hud):
        """Constructor method"""
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
        """On invasion method"""
        self = weak_self()
        if not self:
            return
        lane_types = set(x.type for x in event.crossed_lane_markings)
        text = ['%r' % str(x).split()[-1] for x in lane_types]
        self.hud.notification('Crossed line %s' % ' and '.join(text))

# ==============================================================================
# -- GnssSensor --------------------------------------------------------
# ==============================================================================


class GnssSensor(object):
    """ Class for GNSS sensors"""

    def __init__(self, parent_actor):
        """Constructor method"""
        self.sensor = None
        self._parent = parent_actor
        self.lat = 0.0
        self.lon = 0.0
        world = self._parent.get_world()
        blueprint = world.get_blueprint_library().find('sensor.other.gnss')
        self.sensor = world.spawn_actor(blueprint, carla.Transform(carla.Location(x=1.0, z=2.8)),
                                        attach_to=self._parent)
        # We need to pass the lambda a weak reference to
        # self to avoid circular reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda event: GnssSensor._on_gnss_event(weak_self, event))

    @staticmethod
    def _on_gnss_event(weak_self, event):
        """GNSS method"""
        self = weak_self()
        if not self:
            return
        self.lat = event.latitude
        self.lon = event.longitude

# ==============================================================================
# -- CameraManager -------------------------------------------------------------
# ==============================================================================


class CameraManager(object):
    """ Class for camera management"""

    def __init__(self, parent_actor, hud):
        """Constructor method"""
        self.sensor = None
        self.surface = None
        self._parent = parent_actor
        self.hud = hud
        self.recording = False
        bound_x = 0.5 + self._parent.bounding_box.extent.x
        bound_y = 0.5 + self._parent.bounding_box.extent.y
        bound_z = 0.5 + self._parent.bounding_box.extent.z
        attachment = carla.AttachmentType
        self._camera_transforms = [
            (carla.Transform(carla.Location(x=-2.0*bound_x, y=+0.0*bound_y, z=2.0*bound_z), carla.Rotation(pitch=8.0)), attachment.SpringArmGhost),
            (carla.Transform(carla.Location(x=+0.8*bound_x, y=+0.0*bound_y, z=1.3*bound_z)), attachment.Rigid),
            (carla.Transform(carla.Location(x=+1.9*bound_x, y=+1.0*bound_y, z=1.2*bound_z)), attachment.SpringArmGhost),
            (carla.Transform(carla.Location(x=-2.8*bound_x, y=+0.0*bound_y, z=4.6*bound_z), carla.Rotation(pitch=6.0)), attachment.SpringArmGhost),
            (carla.Transform(carla.Location(x=-1.0, y=-1.0*bound_y, z=0.4*bound_z)), attachment.Rigid)]

        self.transform_index = 1
        self.sensors = [
            ['sensor.camera.rgb', cc.Raw, 'Camera RGB'],
            ['sensor.camera.depth', cc.Raw, 'Camera Depth (Raw)'],
            ['sensor.camera.depth', cc.Depth, 'Camera Depth (Gray Scale)'],
            ['sensor.camera.depth', cc.LogarithmicDepth, 'Camera Depth (Logarithmic Gray Scale)'],
            ['sensor.camera.semantic_segmentation', cc.Raw, 'Camera Semantic Segmentation (Raw)'],
            ['sensor.camera.semantic_segmentation', cc.CityScapesPalette,
             'Camera Semantic Segmentation (CityScapes Palette)'],
            ['sensor.lidar.ray_cast', None, 'Lidar (Ray-Cast)']]
        world = self._parent.get_world()
        bp_library = world.get_blueprint_library()
        for item in self.sensors:
            blp = bp_library.find(item[0])
            if item[0].startswith('sensor.camera'):
                blp.set_attribute('image_size_x', str(hud.dim[0]))
                blp.set_attribute('image_size_y', str(hud.dim[1]))
            elif item[0].startswith('sensor.lidar'):
                blp.set_attribute('range', '50')
            item.append(blp)
        self.index = None

    def toggle_camera(self):
        """Activate a camera"""
        self.transform_index = (self.transform_index + 1) % len(self._camera_transforms)
        self.set_sensor(self.index, notify=False, force_respawn=True)

    def set_sensor(self, index, notify=True, force_respawn=False):
        """Set a sensor"""
        index = index % len(self.sensors)
        needs_respawn = True if self.index is None else (
            force_respawn or (self.sensors[index][0] != self.sensors[self.index][0]))
        if needs_respawn:
            if self.sensor is not None:
                self.sensor.destroy()
                self.surface = None
            self.sensor = self._parent.get_world().spawn_actor(
                self.sensors[index][-1],
                self._camera_transforms[self.transform_index][0],
                attach_to=self._parent,
                attachment_type=self._camera_transforms[self.transform_index][1])

            # We need to pass the lambda a weak reference to
            # self to avoid circular reference.
            weak_self = weakref.ref(self)
            self.sensor.listen(lambda image: CameraManager._parse_image(weak_self, image))
        if notify:
            self.hud.notification(self.sensors[index][2])
        self.index = index

    def next_sensor(self):
        """Get the next sensor"""
        self.set_sensor(self.index + 1)

    def toggle_recording(self):
        """Toggle recording on or off"""
        self.recording = not self.recording
        self.hud.notification('Recording %s' % ('On' if self.recording else 'Off'))

    def render(self, display):
        """Render method"""
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
            lidar_data *= min(self.hud.dim) / 100.0
            lidar_data += (0.5 * self.hud.dim[0], 0.5 * self.hud.dim[1])
            lidar_data = np.fabs(lidar_data)  # pylint: disable=assignment-from-no-return
            lidar_data = lidar_data.astype(np.int32)
            lidar_data = np.reshape(lidar_data, (-1, 2))
            lidar_img_size = (self.hud.dim[0], self.hud.dim[1], 3)
            lidar_img = np.zeros(lidar_img_size)
            lidar_img[tuple(lidar_data.T)] = (255, 255, 255)
            self.surface = pygame.surfarray.make_surface(lidar_img)
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
# -- Game Loop ---------------------------------------------------------
# ==============================================================================


def game_loop(args):
    """
    Main loop of the simulation. It handles updating all the HUD information,
    ticking the agent and, if needed, the world.
    """

    pygame.init()
    pygame.font.init()
    world = None

    try:
        if args.seed:
            random.seed(args.seed)

        client = carla.Client(args.host, args.port)
        client.set_timeout(60.0)

        traffic_manager = client.get_trafficmanager()
        sim_world = client.get_world()

        if args.sync:
            settings = sim_world.get_settings()
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 0.05
            sim_world.apply_settings(settings)

            traffic_manager.set_synchronous_mode(True)

        display = pygame.display.set_mode(
            (args.width, args.height),
            pygame.HWSURFACE | pygame.DOUBLEBUF)

        hud = HUD(args.width, args.height)
        world = World(client.get_world(), hud, args)
        controller = KeyboardControl(world)
        if args.agent == "Basic":
            agent = BasicAgent(world.player, 30)
            agent.follow_speed_limits(True)
        elif args.agent == "Constant":
            agent = ConstantVelocityAgent(world.player, 30)
            ground_loc = world.world.ground_projection(world.player.get_location(), 5)
            if ground_loc:
                world.player.set_location(ground_loc.location + carla.Location(z=0.01))
            agent.follow_speed_limits(True)
        elif args.agent == "Behavior":
            agent = BehaviorAgent(world.player, behavior=args.behavior)

        # Set the agent destination
        spawn_points = world.map.get_spawn_points()
        destination = random.choice(spawn_points).location
        agent.set_destination(destination)

        clock = pygame.time.Clock()

        while True:
            clock.tick()
            if args.sync:
                world.world.tick()
            else:
                world.world.wait_for_tick()
            if controller.parse_events():
                return

            world.tick(clock)
            world.render(display)
            pygame.display.flip()

            if agent.done():
                if args.loop:
                    agent.set_destination(random.choice(spawn_points).location)
                    world.hud.notification("Target reached", seconds=4.0)
                    print("The target has been reached, searching for another target")
                else:
                    print("The target has been reached, stopping the simulation")
                    break

            control = agent.run_step()
            control.manual_gear_shift = False
            world.player.apply_control(control)

    finally:

        if world is not None:
            settings = world.world.get_settings()
            settings.synchronous_mode = False
            settings.fixed_delta_seconds = None
            world.world.apply_settings(settings)
            traffic_manager.set_synchronous_mode(True)

            world.destroy()

        pygame.quit()


# ==============================================================================
# -- main() --------------------------------------------------------------
# ==============================================================================


def main():
    """Main method"""

    argparser = argparse.ArgumentParser(
        description='CARLA Automatic Control Client')
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='Print debug information')
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
        '--res',
        metavar='WIDTHxHEIGHT',
        default='1280x720',
        help='Window resolution (default: 1280x720)')
    argparser.add_argument(
        '--sync',
        action='store_true',
        help='Synchronous mode execution')
    argparser.add_argument(
        '--filter',
        metavar='PATTERN',
        default='vehicle.*',
        help='Actor filter (default: "vehicle.*")')
    argparser.add_argument(
        '--generation',
        metavar='G',
        default='2',
        help='restrict to certain actor generation (values: "1","2","All" - default: "2")')
    argparser.add_argument(
        '-l', '--loop',
        action='store_true',
        dest='loop',
        help='Sets a new random destination upon reaching the previous one (default: False)')
    argparser.add_argument(
        "-a", "--agent", type=str,
        choices=["Behavior", "Basic", "Constant"],
        help="select which agent to run",
        default="Behavior")
    argparser.add_argument(
        '-b', '--behavior', type=str,
        choices=["cautious", "normal", "aggressive"],
        help='Choose one of the possible agent behaviors (default: normal) ',
        default='normal')
    argparser.add_argument(
        '-s', '--seed',
        help='Set seed for repeating executions (default: None)',
        default=None,
        type=int)

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
