#!/usr/bin/env python
# 这是一个被称为“shebang”（也叫“hashbang”）的特殊注释行，用于指定该Python脚本的解释器路径。
# 在类Unix系统（如Linux、macOS等）中，当脚本文件具有可执行权限时，通过这行指定的解释器来运行该脚本。
# 这里指定使用系统环境变量中找到的 `python` 解释器来执行脚本内容，如果系统中安装了多个Python版本，会根据环境变量的配置来选择对应的解释器。

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
# Copyright (c) 2019-2020 Intel Corporation
# 这部分是版权声明信息，指出该代码的版权归属情况。表明该代码的版权在2019年归属于巴塞罗那自治大学（Universitat Autonoma de Barcelona，简称UAB）的计算机视觉中心（Computer Vision Center，简称CVC），
# 以及在2019 - 2020年归属于英特尔公司（Intel Corporation），意味着未经授权不能随意使用、修改或分发该代码，需遵循相应的版权规则。

# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
# 这是关于代码许可协议的说明，告知使用者该代码是依据麻省理工学院（MIT）许可协议进行授权的。
# 同时提供了获取该许可协议具体内容的链接，使用者可以通过访问指定的网址（https://opensource.org/licenses/MIT）查看详细的许可条款，
# 例如在符合哪些条件下可以对代码进行再分发、修改、使用等情况。

# Allows controlling a vehicle with a keyboard. For a simpler and more
# documented example, please take a look at tutorial.py.
# 这是一段对该脚本功能的简要描述性注释，说明这个Python脚本的主要作用是实现通过键盘来控制一辆车辆（在对应的模拟环境或者特定应用场景中）。
# 并且还提示使用者，如果想要查看一个更简单且有更详细文档注释的示例代码，可以去查看名为 `tutorial.py` 的文件，这可能是项目中提供的用于学习和参考的示例代码，有助于更好地理解相关功能的实现方式。

"""
Welcome to CARLA manual control.
# 欢迎来到CARLA手动控制相关说明
# 以下是关于在CARLA环境中进行手动控制操作时各个按键对应的功能介绍


Use ARROWS or WASD keys for control.
# 使用方向键（ARROWS）或者W、A、S、D键来进行控制操作

    W            : throttle     # 'W'键的功能，按下该键用于控制车辆加速（相当于踩油门，throttle在这里表示给车辆提供动力使车辆加速前进的操作）
    S            : brake        # 'S'键的功能，按下该键用于控制车辆刹车（使车辆减速，brake表示启动刹车机制来降低车速）
    AD           : steer        # 'A'键和 'D'键的功能，按下这两个键用于控制车辆转向（steer表示改变车辆行驶的方向，'A'键通常使车辆向左转向，'D'键通常使车辆向右转向）
    Q            : toggle reverse      # 'Q'键的功能，按下该键用于切换车辆的倒车状态（toggle表示切换，reverse表示倒车，即通过该按键可以在前进和倒车两种行驶状态之间切换）
    Space        : hand-brake   # 'Space'键（空格键）的功能，按下该键用于拉起手刹（hand-brake表示手刹，操作后可以使车辆保持静止状态，防止车辆滑动等）
    P            : toggle autopilot    # 'P'键的功能，按下该键用于切换自动驾驶模式（toggle表示切换，autopilot即自动驾驶，按下该键可以在手动驾驶和自动驾驶两种模式之间切换）

    TAB          : change view      # 'TAB'键的功能，按下该键用于切换视图（比如切换不同的视角来查看车辆周围环境、车辆内部视角等不同的观察角度）
    Backspace    : change vehicle   # 'Backspace'键的功能，按下该键用于切换车辆（如果场景中有多辆车可供选择操作，通过该键可以更换当前正在控制的车辆）

    R            : toggle recording images to disk    # 'R'键的功能，按下该键用于切换是否将图像记录到磁盘上（toggle表示切换开启或者关闭的状态，意味着可以通过该键决定是否把当前看到的图像保存到本地磁盘中，方便后续查看或者分析等）

    F2           : toggle RSS visualization mode   # 'F2'键的功能，按下该键用于切换RSS可视化模式（toggle表示切换，RSS可能是某种特定的与车辆相关的安全系统或者分析系统，visualization mode表示可视化模式，按下此键可以决定是否开启该系统的可视化展示，方便查看相关信息）
    F3           : increase log level              # 'F3'键的功能，按下该键用于增加日志级别（日志级别通常用于控制记录信息的详细程度，increase表示提升，按此键可以让系统记录更详细的日志信息，便于调试或者分析系统运行情况）
    F4           : decrease log level              # 'F4'键的功能，按下该键用于降低日志级别（与F3相反，按下此键可以让系统记录相对不那么详细的日志信息，在不需要过多细节时减少日志数据量等）
    F5           : increase map log level          # 'F5'键的功能，按下该键用于增加地图日志级别（专门针对地图相关的日志，increase表示提升其详细程度，有助于更深入地了解地图相关模块运行及数据情况）
    F6           : decrease map log level          # 'F6'键的功能，按下该键用于降低地图日志级别（与F5相反，降低地图相关日志记录的详细程度）
    B            : toggle RSS Road Boundaries Mode # 'B'键的功能，按下该键用于切换RSS道路边界模式（toggle表示切换开启或关闭，RSS Road Boundaries Mode可能是与RSS系统中道路边界相关的一种特定显示或者处理模式）
    G            : RSS check drop current route    # 'G'键的功能，按下该键用于让RSS检查并放弃当前路线（可能是在RSS相关功能下，触发重新规划或者放弃当前正在行驶的路线等操作，具体取决于该系统的设计逻辑）
    T            : toggle RSS                      # 'T'键的功能，按下该键用于切换RSS（整体开启或者关闭RSS相关的功能模块等）
    N            : pause simulation                # 'N'键的功能，按下该键用于暂停模拟（比如暂停整个车辆行驶、环境变化等模拟过程，方便查看当前状态或者进行一些调整操作）

    F1           : toggle HUD     # 'F1'键的功能，按下该键用于切换抬头显示（HUD即Head-Up Display，通常是在驾驶视角中显示一些重要信息，如车速、导航等，通过该键可以决定是否显示这些信息）
    H/?          : toggle help    # 'H'键或者'/?'键的功能，按下这些键用于切换帮助信息显示（可以决定是否在界面上显示操作帮助相关的提示内容，方便用户了解各个按键功能等）
    ESC          : quit           # 'ESC'键的功能，按下该键用于退出整个程序或者当前的操作界面（quit表示完全退出，结束当前在CARLA中的相关操作）
"""

from __future__ import print_function
# 这行代码的作用是从Python的`__future__`模块中导入`print_function`特性。在Python 2中，`print`是一条语句，例如：`print "hello"`。
# 而在Python 3中，`print`被当作函数来使用，形式为`print("hello")`。导入这个特性使得在Python 2环境下编写代码时，也可以按照Python 3中`print`函数的使用方式来书写，
# 方便代码在不同Python版本间保持一定的兼容性，避免因`print`语法差异在迁移代码时带来过多修改成本，后续代码中就能统一以函数形式来使用`print`操作了。


# ==============================================================================
# -- find carla module ---------------------------------------------------------
# ==============================================================================

# 下面这一组导入语句引入了一些Python标准库中常用的模块，它们各自有着特定的功能用途。
import glob
# `glob`模块主要用于文件路径的通配符搜索，它可以根据指定的模式（比如包含通配符`*`、`?`等的路径表达式）来查找匹配的文件或目录路径，
# 在后续代码中很可能用于定位特定格式或名称的文件（在这里大概率与CARLA模块相关文件的查找有关）。
import os
# `os`模块提供了与操作系统交互的各种功能接口，例如文件和目录的操作（创建、删除、重命名等）、获取系统环境变量、操作进程相关信息以及路径处理等，
# 是在Python中进行系统级操作不可或缺的模块，此处用于辅助完成诸如获取文件路径、判断操作系统类型等任务。
import sys
# `sys`模块主要用于处理Python运行时的环境相关操作，像获取命令行参数、操作Python解释器的模块搜索路径（也就是`sys.path`）、控制标准输出和错误输出等，
# 在本代码中核心作用之一就是对模块搜索路径进行调整，以便后续能正确导入需要的模块。
import signal
# `signal`模块用于处理各种操作系统发送给Python进程的信号，例如常见的终止信号（如`SIGINT`表示通过Ctrl+C发送的中断信号等），
# 可以通过该模块来注册信号处理函数，以实现对进程收到特定信号时执行相应的自定义操作，不过在此处代码当前阶段可能暂时未直接体现其功能运用，也许后续会涉及相关处理。

try:
    sys.path.append(glob.glob(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
# 这段代码是这一部分的核心，目的是将CARLA模块所在的路径添加到Python的模块搜索路径（`sys.path`）中，使得Python解释器能够找到并正确导入CARLA相关的模块。
# 详细的执行逻辑如下：
# 1. 首先，通过多次调用`os.path.dirname`函数来获取当前文件（`__file__`表示当前Python脚本文件的路径）的多层上级目录路径。
#    每次调用`os.path.dirname`都会向上回溯一层目录，这里连续调用三次的目的是从当前脚本所在目录向上定位到更上层的目录结构，期望找到CARLA模块所在的大致位置，
#    因为CARLA模块在项目的文件系统布局中可能处于特定的相对目录层级下。
# 2. 接着，拼接上一个特定格式的字符串，该字符串用于描述CARLA模块在不同操作系统下所在的相对路径以及与Python版本对应的部分路径格式。
#    其中`carla/dist/carla-*%d.%d-%s.egg`是基本的路径模式，`*`是通配符，用于匹配具体的版本号等可变部分，`%d.%d`会分别被替换为Python的主版本号（通过`sys.version_info.major`获取）和次版本号（通过`sys.version_info.minor`获取），
#    而`win-amd64`（当`os.name == 'nt'`时，`nt`表示Windows操作系统）或者`linux-x86_64`（用于Linux操作系统）则是根据当前操作系统类型来确定的特定平台相关后缀，整体构成了一个能适配不同操作系统和Python版本的CARLA模块路径模式。
# 3. 然后，使用`glob.glob`函数按照上述拼接好的路径模式去查找匹配的文件路径，它会返回一个符合条件的文件路径列表（理论上期望只有一个正确的CARLA模块路径，但也可能因为文件系统情况返回多个或者空列表）。
# 4. 最后，取列表中的第一个元素（也就是假设找到的CARLA模块所在的`.egg`文件路径），并通过`sys.path.append`将其添加到Python的模块搜索路径中，这样Python在后续导入模块时就能在这个新增的路径下查找CARLA相关模块了。
except IndexError:
    pass
# 如果在上述`glob.glob`操作中没有找到符合条件的路径（即返回的列表为空，此时尝试取第一个元素会引发`IndexError`异常），
# 那么通过`pass`语句忽略这次添加操作，不过这可能导致后续尝试导入CARLA模块时因找不到对应路径而失败，需要确保CARLA模块的安装路径符合预期且可被正确查找。


# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================


import carla     # 导入CARLA模块，它是整个代码可能围绕进行操作的核心模块，提供了创建和控制虚拟驾驶场景、车辆、传感器等各种功能的接口和类。

from carla import ColorConverter as cc
# 从CARLA模块中导入`ColorConverter`类，并将其简称为`cc`。这个类通常用于在CARLA环境下进行图像颜色格式的转换操作，
# 比如把摄像头获取的图像从一种颜色空间转换到另一种颜色空间，以满足不同的显示、处理或分析需求。

import argparse
# 导入Python标准库中的`argparse`模块，用于方便地解析命令行参数，使得程序可以在启动时通过命令行传入不同的配置选项，
# 例如指定场景配置、模拟参数等，增强程序的灵活性和可配置性。
import logging
# 导入Python标准库中的`logging`模块，用于记录程序运行过程中的各种信息，比如调试信息、警告信息、错误信息等，
# 方便在开发、测试以及实际运行中排查问题、了解程序状态以及跟踪执行流程等。
import math
# 导入Python标准库中的`math`模块，提供了各种数学函数和常量，用于进行常见的数学运算，例如三角函数计算、数值运算、几何计算等，
# 在涉及车辆运动模拟、坐标计算等场景下会经常用到。
import random
# 导入Python标准库中的`random`模块，用于生成随机数，可在模拟场景中实现一些随机化的行为，比如随机初始化车辆的位置、速度、生成随机的交通参与者等，
# 让模拟环境更接近真实且多样化。
import weakref
# 导入Python标准库中的`weakref`模块，它提供了创建弱引用的功能。弱引用是一种特殊的对象引用方式，不会增加对象的引用计数，
# 常用于避免循环引用导致的内存泄漏问题，或者在一些需要对对象进行松散关联管理的场景中使用，比如对临时创建的对象进行管理等。
from rss_sensor import RssSensor # pylint: disable=relative-import
# 从名为`rss_sensor`的模块中导入`RssSensor`类。这里通过相对导入的方式（虽然禁用了`pylint`关于相对导入的检查提示，
# 可能是由于项目的目录结构或特定需求使得相对导入在代码静态检查工具中会产生一些告警，但实际上符合代码逻辑要求），
# 这个`RssSensor`类可能是用于实现与某种RSS（可能是Responsive Safety System等与车辆安全相关的系统概念）相关的传感器功能，
# 比如检测车辆周边环境信息以保障行车安全等。
from rss_visualization import RssUnstructuredSceneVisualizer, RssBoundingBoxVisualizer, RssStateVisualizer # pylint: disable=relative-import
# 从名为`rss_visualization`的模块中导入多个与RSS可视化相关的类，同样是相对导入方式且禁用了`pylint`的相关检查提示。
# 这些类分别为`RssUnstructuredSceneVisualizer`、`RssBoundingBoxVisualizer`、`RssStateVisualizer`，
# 它们可能各自承担着不同方面的可视化任务，例如`RssUnstructuredSceneVisualizer`可能用于展示无结构场景（比如不规则的道路周边环境等）相关的可视化效果，
# `RssBoundingBoxVisualizer`可能用于显示物体的边界框（如车辆、障碍物等的包围框，方便识别其位置和范围）可视化，
# `RssStateVisualizer`可能用于展示RSS系统自身的状态信息可视化，帮助开发人员或使用者直观地了解系统的运行情况。

try:
    import pygame
    from pygame.locals import KMOD_CTRL
    from pygame.locals import KMOD_SHIFT
    from pygame.locals import K_BACKSPACE
    from pygame.locals import K_TAB
    from pygame.locals import K_DOWN
    from pygame.locals import K_ESCAPE
    from pygame.locals import K_F1
    from pygame.locals import K_F2
    from pygame.locals import K_F3
    from pygame.locals import K_F4
    from pygame.locals import K_F5
    from pygame.locals import K_F6
    from pygame.locals import K_LEFT
    from pygame.locals import K_RIGHT
    from pygame.locals import K_SLASH
    from pygame.locals import K_SPACE
    from pygame.locals import K_UP
    from pygame.locals import K_a
    from pygame.locals import K_b
    from pygame.locals import K_d
    from pygame.locals import K_g
    from pygame.locals import K_h
    from pygame.locals import K_n
    from pygame.locals import K_p
    from pygame.locals import K_q
    from pygame.locals import K_r
    from pygame.locals import K_s
    from pygame.locals import K_w
    from pygame.locals import K_l
    from pygame.locals import K_i
    from pygame.locals import K_z
    from pygame.locals import K_x
    from pygame.locals import MOUSEBUTTONDOWN
    from pygame.locals import MOUSEBUTTONUP
# 尝试导入`pygame`模块，`pygame`是一个广泛用于Python的游戏开发和多媒体应用开发的库，在这里的应用场景可能是用于创建图形界面、
# 处理用户输入（如键盘按键、鼠标操作等）以及进行一些简单的图形渲染和交互相关操作，为整个模拟环境提供可视化展示和交互的基础功能。
# 同时从`pygame.locals`模块中导入一系列表示键盘按键状态和鼠标按键状态的常量，这些常量对应了键盘上不同按键对应的代码值以及鼠标按键的相关代码值，
# 后续代码可以通过检测这些常量来判断用户具体按下了哪个按键或者操作了哪个鼠标按键，从而执行相应的程序逻辑，例如根据按键来控制车辆的操作、切换视图等。
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')
# 如果导入`pygame`模块失败，即出现`ImportError`异常，说明`pygame`包没有安装或者安装出现问题，
# 此时抛出一个运行时错误（`RuntimeError`），提示用户需要确保`pygame`包已经正确安装，因为后续代码依赖`pygame`来实现相关的可视化、交互等功能。

try:
    import numpy as np
# 尝试导入`numpy`模块，`numpy`是Python中用于高效进行数值计算、处理多维数组以及实现各种科学计算相关功能的核心库，
# 在很多涉及数据处理、数学运算、图像数据操作（比如处理传感器获取的图像数据等）等场景下会起到关键作用，是很多科学计算和数据处理项目中不可或缺的一部分。
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')
# 如果导入`numpy`模块失败，抛出运行时错误，提示用户需要确保`numpy`包已经安装，因为后续代码很可能会用到`numpy`提供的功能来进行相应的计算和数据处理操作。


# ==============================================================================
# -- World ---------------------------------------------------------------------
# ==============================================================================


class World(object):
# 定义一个名为 `World` 的类，这个类很可能是用于表示整个模拟世界相关的操作和状态管理，它继承自Python的 `object` 基类。

    def __init__(self, carla_world, args):
    # 类的初始化方法，在创建 `World` 类的实例时会被调用，用于初始化实例的各种属性和执行一些必要的初始化操作。
        self.world = carla_world
        # 将传入的 `carla_world` 参数赋值给实例属性 `self.world`，这个 `carla_world` 应该是来自CARLA模拟环境的世界对象，
        # 通过它可以访问和操作模拟世界中的各种元素，比如车辆、地图、传感器等。
        self.sync = args.sync
        # 将传入的 `args.sync` 参数赋值给实例属性 `self.sync`，从参数名推测这个属性可能用于控制模拟世界是否以同步模式运行，
        # 同步模式下可能需要按照固定的时间间隔来更新世界状态等操作，具体取决于CARLA的实现逻辑。
        self.actor_role_name = args.rolename
        # 将传入的 `args.rolename` 参数赋值给实例属性 `self.actor_role_name`，这个属性可能用于标识某个特定角色的演员（actor，在CARLA中可以指代车辆、行人等各种参与模拟的对象）名称，
        # 可能在后续寻找、创建或管理特定角色的对象时会用到。
        self.dim = (args.width, args.height)
        # 将传入的 `args.width` 和 `args.height` 组成的元组赋值给实例属性 `self.dim`，从名称推测这可能表示屏幕显示或者图像相关的维度信息，
        # 比如窗口的宽和高，也许用于后续图像渲染、显示等操作的尺寸设置。
        try:
            self.map = self.world.get_map()
        except RuntimeError as error:
            print('RuntimeError: {}'.format(error))
            print('  The server could not send the OpenDRIVE (.xodr) file:')
            print('  Make sure it exists, has the same name of your town, and is correct.')
            sys.exit(1)
        self.external_actor = args.externalActor

        self.hud = HUD(args.width, args.height, carla_world)
        self.recording_frame_num = 0
        self.recording = False
        self.recording_dir_num = 0
        self.player = None
        self.actors = []
        self.rss_sensor = None
        self.rss_unstructured_scene_visualizer = None
        self.rss_bounding_box_visualizer = None
        self._actor_filter = args.filter
        if not self._actor_filter.startswith("vehicle."):
            print('Error: RSS only supports vehicles as ego.')
            sys.exit(1)

        self.restart()
        self.world_tick_id = self.world.on_tick(self.hud.on_world_tick)

    def toggle_pause(self):
        settings = self.world.get_settings()
        self.pause_simulation(not settings.synchronous_mode)

    def pause_simulation(self, pause):
        settings = self.world.get_settings()
        if pause and not settings.synchronous_mode:
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 0.05
            self.world.apply_settings(settings)
        elif not pause and settings.synchronous_mode:
            settings.synchronous_mode = False
            settings.fixed_delta_seconds = None
            self.world.apply_settings(settings)

    def restart(self):

        if self.external_actor:
            # Check whether there is already an actor with defined role name
            for actor in self.world.get_actors():
                if actor.attributes.get('role_name') == self.actor_role_name:
                    self.player = actor
                    break
        else:
            # Get a random blueprint.
            blueprint = random.choice(self.world.get_blueprint_library().filter(self._actor_filter))
            blueprint.set_attribute('role_name', self.actor_role_name)
            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)
            if blueprint.has_attribute('driver_id'):
                driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
                blueprint.set_attribute('driver_id', driver_id)
            if blueprint.has_attribute('is_invincible'):
                blueprint.set_attribute('is_invincible', 'true')
            # Spawn the player.
            if self.player is not None:
                spawn_point = self.player.get_transform()
                spawn_point.location.z += 2.0
                spawn_point.rotation.roll = 0.0
                spawn_point.rotation.pitch = 0.0
                self.destroy()
                self.player = self.world.try_spawn_actor(blueprint, spawn_point)
            while self.player is None:
                if not self.map.get_spawn_points():
                    print('There are no spawn points available in your map/town.')
                    print('Please add some Vehicle Spawn Point to your UE4 scene.')
                    sys.exit(1)
                spawn_points = self.map.get_spawn_points()
                spawn_point = random.choice(spawn_points) if spawn_points else carla.Transform()
                self.player = self.world.try_spawn_actor(blueprint, spawn_point)

        if self.external_actor:
            ego_sensors = []
            for actor in self.world.get_actors():
                if actor.parent == self.player:
                    ego_sensors.append(actor)

            for ego_sensor in ego_sensors:
                if ego_sensor is not None:
                    ego_sensor.destroy()

        # Set up the sensors.
        self.camera = Camera(self.player, self.dim)
        self.rss_unstructured_scene_visualizer = RssUnstructuredSceneVisualizer(self.player, self.world, self.dim)
        self.rss_bounding_box_visualizer = RssBoundingBoxVisualizer(self.dim, self.world, self.camera.sensor)
        self.rss_sensor = RssSensor(self.player, self.world,
                                    self.rss_unstructured_scene_visualizer, self.rss_bounding_box_visualizer, self.hud.rss_state_visualizer)

        if self.sync:
            self.world.tick()
        else:
            self.world.wait_for_tick()

    def tick(self, clock):
        self.hud.tick(self.player, clock)

    def toggle_recording(self):
        if not self.recording:
            dir_name = "_out%04d" % self.recording_dir_num
            while os.path.exists(dir_name):
                self.recording_dir_num += 1
                dir_name = "_out%04d" % self.recording_dir_num
            self.recording_frame_num = 0
            os.mkdir(dir_name)
        else:
            self.hud.notification('Recording finished (folder: _out%04d)' % self.recording_dir_num)

        self.recording = not self.recording

    def render(self, display):
        self.camera.render(display)
        self.rss_bounding_box_visualizer.render(display, self.camera.current_frame)
        self.rss_unstructured_scene_visualizer.render(display)
        self.hud.render(display)

        if self.recording:
            pygame.image.save(display, "_out%04d/%08d.bmp" % (self.recording_dir_num, self.recording_frame_num))
            self.recording_frame_num += 1

    def destroy(self):
        # stop from ticking
        if self.world_tick_id:
            self.world.remove_on_tick(self.world_tick_id)

        if self.camera:
            self.camera.destroy()
        if self.rss_sensor:
            self.rss_sensor.destroy()
        if self.rss_unstructured_scene_visualizer:
            self.rss_unstructured_scene_visualizer.destroy()
        if self.player:
            self.player.destroy()


# ==============================================================================
# -- Camera --------------------------------------------------------------------
# ==============================================================================

class Camera(object):

    def __init__(self, parent_actor, display_dimensions):
        self.surface = None
        self._parent = parent_actor
        self.current_frame = None
        bp_library = self._parent.get_world().get_blueprint_library()
        bp = bp_library.find('sensor.camera.rgb')
        bp.set_attribute('image_size_x', str(display_dimensions[0]))
        bp.set_attribute('image_size_y', str(display_dimensions[1]))
        self.sensor = self._parent.get_world().spawn_actor(bp, carla.Transform(carla.Location(
            x=-5.5, z=2.5), carla.Rotation(pitch=8.0)), attach_to=self._parent, attachment_type=carla.AttachmentType.SpringArmGhost)

        # We need to pass the lambda a weak reference to self to avoid
        # circular reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda image: Camera._parse_image(weak_self, image))

    def destroy(self):
        self.sensor.stop()
        self.sensor.destroy()
        self.sensor = None

    def render(self, display):
        if self.surface is not None:
            display.blit(self.surface, (0, 0))

    @staticmethod
    def _parse_image(weak_self, image):
        self = weak_self()
        if not self:
            return
        self.current_frame = image.frame
        image.convert(cc.Raw)
        array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
        array = np.reshape(array, (image.height, image.width, 4))
        array = array[:, :, :3]
        array = array[:, :, ::-1]
        self.surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))

# ==============================================================================
# -- VehicleControl -----------------------------------------------------------
# ==============================================================================


class VehicleControl(object):

    MOUSE_STEERING_RANGE = 200
    signal_received = False

    """Class that handles keyboard input."""

    def __init__(self, world, start_in_autopilot):
        self._autopilot_enabled = start_in_autopilot
        self._world = world
        self._control = carla.VehicleControl()
        self._lights = carla.VehicleLightState.NONE
        world.player.set_autopilot(self._autopilot_enabled)
        self._restrictor = carla.RssRestrictor()
        self._vehicle_physics = world.player.get_physics_control()
        world.player.set_light_state(self._lights)
        self._steer_cache = 0.0
        self._mouse_steering_center = None

        self._surface = pygame.Surface((self.MOUSE_STEERING_RANGE * 2, self.MOUSE_STEERING_RANGE * 2))
        self._surface.set_colorkey(pygame.Color('black'))
        self._surface.set_alpha(60)

        line_width = 2
        pygame.draw.polygon(self._surface,
                            (0, 0, 255),
                            [
                                (0, 0),
                                (0, self.MOUSE_STEERING_RANGE * 2 - line_width),
                                (self.MOUSE_STEERING_RANGE * 2 - line_width,
                                 self.MOUSE_STEERING_RANGE * 2 - line_width),
                                (self.MOUSE_STEERING_RANGE * 2 - line_width, 0),
                                (0, 0)
                            ], line_width)
        pygame.draw.polygon(self._surface,
                            (0, 0, 255),
                            [
                                (0, self.MOUSE_STEERING_RANGE),
                                (self.MOUSE_STEERING_RANGE * 2, self.MOUSE_STEERING_RANGE)
                            ], line_width)
        pygame.draw.polygon(self._surface,
                            (0, 0, 255),
                            [
                                (self.MOUSE_STEERING_RANGE, 0),
                                (self.MOUSE_STEERING_RANGE, self.MOUSE_STEERING_RANGE * 2)
                            ], line_width)

        world.hud.notification("Press 'H' or '?' for help.", seconds=4.0)

    def render(self, display):
        if self._mouse_steering_center:
            display.blit(
                self._surface, (self._mouse_steering_center[0] - self.MOUSE_STEERING_RANGE, self._mouse_steering_center[1] - self.MOUSE_STEERING_RANGE))

    @staticmethod
    def signal_handler(signum, _):
        print('\nReceived signal {}. Trigger stopping...'.format(signum))
        VehicleControl.signal_received = True

    def parse_events(self, world, clock, sync_mode):
        if VehicleControl.signal_received:
            print('\nAccepted signal. Stopping loop...')
            return True
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
                elif event.key == K_h or (event.key == K_SLASH and pygame.key.get_mods() & KMOD_SHIFT):
                    world.hud.help.toggle()
                elif event.key == K_TAB:
                    world.rss_unstructured_scene_visualizer.toggle_camera()
                elif event.key == K_n:
                    world.toggle_pause()
                elif event.key == K_r:
                    world.toggle_recording()
                elif event.key == K_F2:
                    if self._world and self._world.rss_sensor:
                        self._world.rss_sensor.toggle_debug_visualization_mode()
                elif event.key == K_F3:
                    if self._world and self._world.rss_sensor:
                        self._world.rss_sensor.decrease_log_level()
                        self._restrictor.set_log_level(self._world.rss_sensor.log_level)
                elif event.key == K_F4:
                    if self._world and self._world.rss_sensor:
                        self._world.rss_sensor.increase_log_level()
                        self._restrictor.set_log_level(self._world.rss_sensor.log_level)
                elif event.key == K_F5:
                    if self._world and self._world.rss_sensor:
                        self._world.rss_sensor.decrease_map_log_level()
                elif event.key == K_F6:
                    if self._world and self._world.rss_sensor:
                        self._world.rss_sensor.increase_map_log_level()
                elif event.key == K_b:
                    if self._world and self._world.rss_sensor:
                        if self._world.rss_sensor.sensor.road_boundaries_mode == carla.RssRoadBoundariesMode.Off:
                            self._world.rss_sensor.sensor.road_boundaries_mode = carla.RssRoadBoundariesMode.On
                            print("carla.RssRoadBoundariesMode.On")
                        else:
                            self._world.rss_sensor.sensor.road_boundaries_mode = carla.RssRoadBoundariesMode.Off
                            print("carla.RssRoadBoundariesMode.Off")
                elif event.key == K_g:
                    if self._world and self._world.rss_sensor:
                        self._world.rss_sensor.drop_route()
                if isinstance(self._control, carla.VehicleControl):
                    if event.key == K_q:
                        self._control.gear = 1 if self._control.reverse else -1
                    elif event.key == K_p and not pygame.key.get_mods() & KMOD_CTRL:
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
            elif event.type == MOUSEBUTTONDOWN:
                # store current mouse position for mouse-steering
                if event.button == 1:
                    self._mouse_steering_center = event.pos
            elif event.type == MOUSEBUTTONUP:
                if event.button == 1:
                    self._mouse_steering_center = None
        if not self._autopilot_enabled:
            prev_steer_cache = self._steer_cache
            self._parse_vehicle_keys(pygame.key.get_pressed(), clock.get_time())
            if pygame.mouse.get_pressed()[0]:
                self._parse_mouse(pygame.mouse.get_pos())
            self._control.reverse = self._control.gear < 0

            vehicle_control = self._control
            world.hud.original_vehicle_control = vehicle_control
            world.hud.restricted_vehicle_control = vehicle_control

            # limit speed to 30kmh
            v = self._world.player.get_velocity()
            if (3.6 * math.sqrt(v.x**2 + v.y**2 + v.z**2)) > 30.0:
                self._control.throttle = 0

            # if self._world.rss_sensor and self._world.rss_sensor.ego_dynamics_on_route and not self._world.rss_sensor.ego_dynamics_on_route.ego_center_within_route:
            #    print ("Not on route!" +  str(self._world.rss_sensor.ego_dynamics_on_route))
            if self._restrictor:
                rss_proper_response = self._world.rss_sensor.proper_response if self._world.rss_sensor and self._world.rss_sensor.response_valid else None
                if rss_proper_response:
                    if not (pygame.key.get_mods() & KMOD_CTRL):
                        vehicle_control = self._restrictor.restrict_vehicle_control(
                            vehicle_control, rss_proper_response, self._world.rss_sensor.ego_dynamics_on_route, self._vehicle_physics)
                    world.hud.restricted_vehicle_control = vehicle_control
                    world.hud.allowed_steering_ranges = self._world.rss_sensor.get_steering_ranges()
                    if world.hud.original_vehicle_control.steer != world.hud.restricted_vehicle_control.steer:
                        self._steer_cache = prev_steer_cache

            # Set automatic control-related vehicle lights
            if vehicle_control.brake:
                current_lights |= carla.VehicleLightState.Brake
            else:  # Remove the Brake flag
                current_lights &= carla.VehicleLightState.All ^ carla.VehicleLightState.Brake
            if vehicle_control.reverse:
                current_lights |= carla.VehicleLightState.Reverse
            else:  # Remove the Reverse flag
                current_lights &= carla.VehicleLightState.All ^ carla.VehicleLightState.Reverse
            if current_lights != self._lights:  # Change the light state only if necessary
                self._lights = current_lights
                world.player.set_light_state(carla.VehicleLightState(self._lights))

            world.player.apply_control(vehicle_control)

    def _parse_vehicle_keys(self, keys, milliseconds):
        if keys[K_UP] or keys[K_w]:
            self._control.throttle = min(self._control.throttle + 0.2, 1)
        else:
            self._control.throttle = max(self._control.throttle - 0.2, 0)

        if keys[K_DOWN] or keys[K_s]:
            self._control.brake = min(self._control.brake + 0.2, 1)
        else:
            self._control.brake = max(self._control.brake - 0.2, 0)

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
        elif self._steer_cache > 0:
            self._steer_cache = max(self._steer_cache - steer_increment, 0.0)
        elif self._steer_cache < 0:
            self._steer_cache = min(self._steer_cache + steer_increment, 0.0)
        else:
            self._steer_cache = 0

        self._steer_cache = min(1.0, max(-1.0, self._steer_cache))
        self._control.steer = round(self._steer_cache, 1)
        self._control.hand_brake = keys[K_SPACE]

    def _parse_mouse(self, pos):
        if not self._mouse_steering_center:
            return

        lateral = float(pos[0] - self._mouse_steering_center[0])
        longitudinal = float(pos[1] - self._mouse_steering_center[1])
        max_val = self.MOUSE_STEERING_RANGE
        lateral = -max_val if lateral < -max_val else max_val if lateral > max_val else lateral
        longitudinal = -max_val if longitudinal < -max_val else max_val if longitudinal > max_val else longitudinal
        self._control.steer = lateral / max_val
        if longitudinal < 0.0:
            self._control.throttle = -longitudinal / max_val
            self._control.brake = 0.0
        elif longitudinal > 0.0:
            self._control.throttle = 0.0
            self._control.brake = longitudinal / max_val

    @staticmethod
    def _is_quit_shortcut(key):
        return (key == K_ESCAPE) or (key == K_q and pygame.key.get_mods() & KMOD_CTRL)


# ==============================================================================
# -- HUD -----------------------------------------------------------------------
# ==============================================================================


class HUD(object):

    def __init__(self, width, height, world):
        self.dim = (width, height)
        self._world = world
        self.map_name = world.get_map().name
        font = pygame.font.Font(pygame.font.get_default_font(), 20)
        font_name = 'courier' if os.name == 'nt' else 'mono'
        fonts = [x for x in pygame.font.get_fonts() if font_name in x]
        default_font = 'ubuntumono'
        mono = default_font if default_font in fonts else fonts[0]
        mono = pygame.font.match_font(mono)
        self._font_mono = pygame.font.Font(mono, 12 if os.name == 'nt' else 14)
        self._notifications = FadingText(font, (width, 40), (0, height - 40))
        self.help = HelpText(pygame.font.Font(mono, 16), width, height)
        self.server_fps = 0
        self.frame = 0
        self.simulation_time = 0
        self.original_vehicle_control = None
        self.restricted_vehicle_control = None
        self.allowed_steering_ranges = []
        self._show_info = True
        self._info_text = []
        self._server_clock = pygame.time.Clock()
        self.rss_state_visualizer = RssStateVisualizer(self.dim, self._font_mono, self._world)

    def on_world_tick(self, timestamp):
        self._server_clock.tick()
        self.server_fps = self._server_clock.get_fps()
        self.frame = timestamp.frame
        self.simulation_time = timestamp.elapsed_seconds

    def tick(self, player, clock):
        self._notifications.tick(clock)
        if not self._show_info:
            return
        t = player.get_transform()
        v = player.get_velocity()
        c = player.get_control()

        self._info_text = [
            'Server:  % 16.0f FPS' % self.server_fps,
            'Client:  % 16.0f FPS' % clock.get_fps(),
            'Map:     % 20s' % self.map_name,
            '',
            'Speed:   % 15.0f km/h' % (3.6 * math.sqrt(v.x**2 + v.y**2 + v.z**2)),
            'Location:% 20s' % ('(% 5.1f, % 5.1f)' % (t.location.x, t.location.y)),
            'Heading: % 20.2f' % math.radians(t.rotation.yaw),
            '']
        if self.original_vehicle_control:
            orig_control = self.original_vehicle_control
            restricted_control = self.restricted_vehicle_control
            allowed_steering_ranges = self.allowed_steering_ranges
            self._info_text += [
                ('Throttle:', orig_control.throttle, 0.0, 1.0, restricted_control.throttle),
                ('Steer:', orig_control.steer, -1.0, 1.0, restricted_control.steer, allowed_steering_ranges),
                ('Brake:', orig_control.brake, 0.0, 1.0, restricted_control.brake)]
        self._info_text += [
            ('Reverse:', c.reverse),
            '']

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
                text_color = (255, 255, 255)
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
                        rect = pygame.Rect((bar_h_offset, v_offset + 2), (10, 10))
                        pygame.draw.rect(display, (255, 255, 255), rect, 0 if item[1] else 1)
                    else:
                        # draw allowed steering ranges
                        if len(item) == 6 and item[2] < 0.0:
                            for steering_range in item[5]:
                                starting_value = min(steering_range[0], steering_range[1])
                                length = (max(steering_range[0], steering_range[1]) -
                                          min(steering_range[0], steering_range[1])) / 2
                                rect = pygame.Rect(
                                    (bar_h_offset + (starting_value + 1) * (bar_width / 2), v_offset + 2), (length * bar_width, 14))
                                pygame.draw.rect(display, (0, 255, 0), rect)

                        # draw border
                        rect_border = pygame.Rect((bar_h_offset, v_offset + 2), (bar_width, 14))
                        pygame.draw.rect(display, (255, 255, 255), rect_border, 1)

                        # draw value / restricted value
                        input_value_rect_fill = 0
                        if len(item) >= 5:
                            if item[1] != item[4]:
                                input_value_rect_fill = 1
                                f = (item[4] - item[2]) / (item[3] - item[2])
                                if item[2] < 0.0:
                                    rect = pygame.Rect(
                                        (bar_h_offset + 1 + f * (bar_width - 6), v_offset + 3), (12, 12))
                                else:
                                    rect = pygame.Rect((bar_h_offset + 1, v_offset + 3), (f * bar_width, 12))
                                pygame.draw.rect(display, (255, 0, 0), rect)

                        f = (item[1] - item[2]) / (item[3] - item[2])
                        rect = None
                        if item[2] < 0.0:
                            rect = pygame.Rect((bar_h_offset + 2 + f * (bar_width - 14), v_offset + 4), (10, 10))
                        else:
                            if item[1] != 0:
                                rect = pygame.Rect((bar_h_offset + 2, v_offset + 4), (f * (bar_width - 4), 10))
                        if rect:
                            pygame.draw.rect(display, (255, 255, 255), rect, input_value_rect_fill)
                    item = item[0]
                if item:  # At this point has to be a str.
                    surface = self._font_mono.render(item, True, text_color)
                    display.blit(surface, (8, v_offset))
                v_offset += 18

            self.rss_state_visualizer.render(display, v_offset)
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

    def tick(self, clock):
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
# -- game_loop() ---------------------------------------------------------------
# ==============================================================================


def game_loop(args):
    pygame.init()
    pygame.font.init()
    world = None

    try:
        client = carla.Client(args.host, args.port)
        client.set_timeout(2.0)

        display = pygame.display.set_mode(
            (args.width, args.height),
            pygame.HWSURFACE | pygame.DOUBLEBUF)

        sim_world = client.get_world()
        original_settings = sim_world.get_settings()
        settings = sim_world.get_settings()
        if args.sync != settings.synchronous_mode:
            args.sync = True
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 0.05
            sim_world.apply_settings(settings)

            traffic_manager = client.get_trafficmanager()
            traffic_manager.set_synchronous_mode(True)

        world = World(sim_world, args)
        controller = VehicleControl(world, args.autopilot)

        clock = pygame.time.Clock()
        while True:
            if args.sync:
                sim_world.tick()
            clock.tick_busy_loop(60)
            if controller.parse_events(world, clock, args.sync):
                return
            world.tick(clock)
            world.render(display)
            controller.render(display)
            pygame.display.flip()

    finally:

        if world is not None:
            print('Destroying the world...')
            world.destroy()
            print('Destroyed!')

        pygame.quit()


# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================

def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Manual Control Client RSS')
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
        '--externalActor',
        action='store_true',
        help='attaches to externally created actor by role name')
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

    signal.signal(signal.SIGINT, VehicleControl.signal_handler)

    try:
        game_loop(args)

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')


if __name__ == '__main__':

    main()
