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
            # 通过 `self.world`（也就是CARLA世界对象）调用 `get_map` 方法尝试获取模拟世界的地图对象，并赋值给实例属性 `self.map`。
            # 如果获取过程出现运行时错误（比如地图文件不存在、加载失败等情况），会进入下面的 `except` 块进行相应处理。
        except RuntimeError as error:
            print('RuntimeError: {}'.format(error))
            print('  The server could not send the OpenDRIVE (.xodr) file:')
            print('  Make sure it exists, has the same name of your town, and is correct.')
            # 如果在获取地图时出现运行时错误，打印出错误信息，方便调试和排查问题。
            sys.exit(1)
            # 使用 `sys.exit(1)` 终止程序运行，返回状态码为1，表示程序因错误而异常退出，这里提示用户确保地图文件（OpenDRIVE格式的 `.xodr` 文件）存在、名称与当前模拟的城镇名称一致且文件内容正确。
        self.external_actor = args.externalActor
        # 将传入的 `args.externalActor` 参数赋值给实例属性 `self.external_actor`，从名称推测这个属性可能用于标识是否使用外部定义的演员对象，
        # 后续会根据这个属性的值来决定如何创建或获取模拟世界中的主要演员（比如车辆等）。

        self.hud = HUD(args.width, args.height, carla_world)
        # 创建一个 `HUD`（抬头显示，Head-Up Display）类的实例，传入窗口的宽 `args.width`、高 `args.height` 以及 `carla_world` 对象，
        # 用于管理和显示模拟世界中的一些抬头显示相关信息，比如车速、车辆状态等信息展示，将创建好的实例赋值给 `self.hud` 属性。
        self.recording_frame_num = 0
        # 用于记录当前正在录制的帧数，初始化为0，在开启录制功能后会随着每一帧的渲染逐步递增，用于给录制的图像文件命名等操作。
        self.recording = False
        # 用于标识当前是否正在进行录制操作，初始化为 `False`，通过调用 `toggle_recording` 方法可以切换这个状态。
        self.recording_dir_num = 0
        # 用于记录录制目录的编号，初始化为0，当创建新的录制目录时，如果目录已存在会递增这个编号，以保证每次录制生成的目录名称不重复。
        self.player = None
         # 用于存储模拟世界中的主要演员对象（很可能是车辆等可控制对象），初始化为 `None`，会在后续 `restart` 等方法中进行赋值。
        self.actors = []
         # 用于存储模拟世界中的各种演员（actor）对象列表，初始为空列表，后续会在创建或获取演员对象后将它们添加到这个列表中进行统一管理。
        self.rss_sensor = None
         # 用于存储与RSS（可能是某种车辆安全相关系统，如Responsive Safety System等）相关的传感器对象，初始化为 `None`，会在后续 `restart` 等操作中进行实例化和赋值。
        self.rss_unstructured_scene_visualizer = None
         # 用于存储无结构场景可视化相关的对象（可能是用于将模拟世界中的一些不规则场景元素进行可视化展示的对象），初始化为 `None`，同样会在相关操作中进行初始化和赋值。
        self.rss_bounding_box_visualizer = None
        # 用于存储边界框可视化相关的对象（可能用于显示车辆、障碍物等物体的边界框，方便直观看到它们的位置和范围等信息），初始化为 `None`，后续会按需进行初始化和赋值。
        self._actor_filter = args.filter
         # 将传入的 `args.filter` 参数赋值给实例属性 `_actor_filter`，从名称推测这个属性可能用于筛选特定类型的演员对象，
         # 例如只筛选车辆类型的演员等，会在后续创建或获取演员对象时起到过滤作用。
        if not self._actor_filter.startswith("vehicle."):
            print('Error: RSS only supports vehicles as ego.')
            sys.exit(1)
        # 如果 `_actor_filter` 参数所表示的筛选条件不是以 "vehicle." 开头，说明不符合RSS系统要求（RSS可能只支持以车辆作为主要关注对象，也就是所谓的“ego”主体），
        # 则打印错误信息提示用户，并终止程序运行，返回状态码为1，表示出现了不符合预期的参数配置错误。

        self.restart()
        # 调用 `restart` 方法，这个方法可能用于重新初始化模拟世界中的各种元素，比如重新创建车辆、设置传感器等，使世界恢复到一个初始可用状态。
        self.world_tick_id = self.world.on_tick(self.hud.on_world_tick)
        # 通过 `self.world`（CARLA世界对象）调用 `on_tick` 方法，并传入 `self.hud.on_world_tick` 作为回调函数，
        # 用于在模拟世界每一次更新（tick）时执行 `self.hud.on_world_tick` 函数来更新抬头显示等相关信息，将返回的标识（可能是用于后续取消这个回调绑定的一个唯一标识符等）赋值给 `self.world_tick_id` 属性。

    def toggle_pause(self):
    # 定义一个方法用于切换模拟世界的暂停状态，也就是在暂停和继续运行之间切换。
        settings = self.world.get_settings()
        # 通过 `self.world`（CARLA世界对象）调用 `get_settings` 方法获取当前世界的设置信息，比如同步模式、时间步长等设置，赋值给 `settings` 变量。
        self.pause_simulation(not settings.synchronous_mode)
        # 调用 `pause_simulation` 方法，传入当前同步模式的取反值（即如果当前是同步模式就传入 `False`，如果当前是非同步模式就传入 `True`），
        # 来实现切换暂停状态的操作，根据传入的值来决定是暂停还是继续运行模拟世界。

    def pause_simulation(self, pause):
    # 定义一个方法用于设置模拟世界的暂停状态，根据传入的 `pause` 参数值来决定是暂停还是恢复运行。
        settings = self.world.get_settings()
        # 通过 `self.world`（CARLA世界对象）调用 `get_settings` 方法获取当前世界的设置信息，赋值给 `settings` 变量。
        if pause and not settings.synchronous_mode:
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 0.05
            self.world.apply_settings(settings)
            # 如果 `pause` 参数为 `True`（表示要暂停）且当前世界不是同步模式，那么将同步模式设置为 `True`，
            # 也就是开启同步模式，同时设置固定的时间步长为0.05秒（ `fixed_delta_seconds` 属性用于控制每次世界更新的时间间隔，在同步模式下通常需要设置一个固定值），
            # 最后通过 `self.world.apply_settings` 方法将修改后的设置应用到模拟世界中，使其生效。
        elif not pause and settings.synchronous_mode:
            settings.synchronous_mode = False
            settings.fixed_delta_seconds = None
            self.world.apply_settings(settings)
            # 如果 `pause` 参数为 `False`（表示要恢复运行）且当前世界处于同步模式，那么将同步模式设置为 `False`，关闭同步模式，
            # 并将固定时间步长设置为 `None`（在非同步模式下不需要固定的时间步长），再通过 `self.world.apply_settings` 方法将新设置应用到模拟世界中，实现恢复运行的操作。

    def restart(self):
    # 定义一个方法用于重新启动模拟世界相关的各种元素，比如重新创建车辆、传感器等，使其恢复到一个初始可用状态。

        if self.external_actor:
            # Check whether there is already an actor with defined role name
            for actor in self.world.get_actors():
                if actor.attributes.get('role_name') == self.actor_role_name:
                    self.player = actor
                    break
        # 如果 `self.external_actor` 属性为 `True`，表示使用外部定义的演员对象，那么执行以下操作来查找并设置 `self.player`（主要演员，可能是车辆）。
        # 遍历模拟世界中所有的演员对象（通过 `self.world.get_actors` 方法获取），查找属性 `role_name` 与 `self.actor_role_name`（之前初始化时设置的特定角色名称）相等的演员对象，
        # 如果找到则将其赋值给 `self.player`，并跳出循环，表示找到了对应的外部定义的演员。
        else:
            # Get a random blueprint.
            blueprint = random.choice(self.world.get_blueprint_library().filter(self._actor_filter))
            # 如果不使用外部定义的演员对象，那么执行以下操作来创建一个新的主要演员（可能是车辆）。
            # 首先，从世界的蓝图库（ `self.world.get_blueprint_library` 方法返回所有可用的蓝图对象列表，蓝图可以理解为创建各种演员的模板）中，
            # 通过 `filter` 方法根据 `self._actor_filter`（之前设置的筛选条件，应该是筛选车辆相关蓝图）筛选出符合条件的蓝图对象，然后随机选择一个蓝图，赋值给 `blueprint` 变量。
            blueprint.set_attribute('role_name', self.actor_role_name)
            # 给选中的蓝图对象设置 `role_name` 属性为 `self.actor_role_name`（特定角色名称），用于标识这个即将创建的演员对象的角色。
            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)
                # 如果蓝图对象有 `color` 属性（表示可以设置颜色），则从 `color` 属性推荐的颜色值列表（ `recommended_values` ）中随机选择一个颜色，
                # 并将其设置为蓝图对象的颜色属性值，这样创建出来的演员（比如车辆）就会有随机的外观颜色。
            if blueprint.has_attribute('driver_id'):
                driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
                blueprint.set_attribute('driver_id', driver_id)
                # 如果蓝图对象有 `driver_id` 属性（表示可以设置驾驶员标识等相关信息），则从 `driver_id` 属性推荐的标识值列表中随机选择一个标识，
                # 并将其设置为蓝图对象的 `driver_id` 属性值，用于模拟不同驾驶员等情况。
            if blueprint.has_attribute('is_invincible'):
                blueprint.set_attribute('is_invincible', 'true')
                #如果蓝图对象有 `is_invincible` 属性（表示是否无敌，可能在模拟中不受碰撞等影响），则将其设置为 `true`，使创建出来的演员具有无敌属性，
                # 这可能在一些特定的测试或模拟场景下有需要，避免演员过早被破坏等情况。
            # Spawn the player.
            if self.player is not None:
                spawn_point = self.player.get_transform()
                spawn_point.location.z += 2.0
                spawn_point.rotation.roll = 0.0
                spawn_point.rotation.pitch = 0.0
                # 如果之前已经存在 `self.player`（可能之前创建过主要演员对象），则获取它的位置和姿态信息（通过 `get_transform` 方法获取 `Transform` 对象，包含位置、旋转等信息），
                # 然后将位置的 `z` 坐标（垂直方向）增加2.0（可能是将其抬高一点，避免与之前的位置冲突等），并将旋转的 `roll`（翻滚）和 `pitch`（俯仰）角度设置为0.0，保证姿态正常，
                # 最后将修改后的位置和姿态信息赋值给 `spawn_point` 变量，用于作为新创建演员的出生点。
                self.destroy()
                # 调用 `destroy` 方法（这个方法应该是用于销毁之前创建的相关对象，比如之前的主要演员等），清理之前的相关资源。
                self.player = self.world.try_spawn_actor(blueprint, spawn_point)
                # 使用修改后的 `spawn_point` 作为出生点，通过 `self.world.try_spawn_actor` 方法尝试根据 `blueprint` 蓝图对象创建新的演员对象，
                # 如果创建成功则赋值给 `self.player`，否则继续下面的循环尝试操作。
            while self.player is None:
                if not self.map.get_spawn_points():
                    print('There are no spawn points available in your map/town.')
                    print('Please add some Vehicle Spawn Point to your UE4 scene.')
                    sys.exit(1)
                spawn_points = self.map.get_spawn_points()
                spawn_point = random.choice(spawn_points) if spawn_points else carla.Transform()
                self.player = self.world.try_spawn_actor(blueprint, spawn_point)
                # 如果地图中没有可用的出生点（通过 `self.map.get_spawn_points` 方法获取出生点列表，如果为空则表示没有可用出生点），
                # 则打印提示信息，告知用户在地图（UE4场景中对应的地图）里添加一些车辆出生点，并终止程序运行，返回状态码为1，表示出现了无法创建演员的错误情况。
                # 尝试从地图获取可用的出生点列表，如果有则随机选择一个作为出生点，否则使用默认的 `carla.Transform`（可能是一个默认的位置和姿态）作为出生点，
                # 然后再次尝试根据 `blueprint` 蓝图对象和选择的出生点创建新的演员对象，直到创建成功（ `self.player` 不为 `None` ）为止。
        
        if self.external_actor:
            # 如果使用外部定义的演员对象，那么执行以下操作来处理与之相关的传感器等附属对象。
            ego_sensors = []
            for actor in self.world.get_actors():
                if actor.parent == self.player:
                    ego_sensors.append(actor)
            # 遍历模拟世界中的所有演员对象，查找父对象是 `self.player`（主要演员）的演员对象，将它们添加到 `ego_sensors` 列表中，
            # 这些对象可能是与主要演员相关的传感器等附属对象，后续需要进行相应处理。

            for ego_sensor in ego_sensors:
                if ego_sensor is not None:
                    ego_sensor.destroy()
            # 遍历 `ego_sensors` 列表，对于不为 `None` 的传感器等附属对象，调用 `destroy` 方法将它们销毁，清理相关资源，
            # 可能是为了重新设置或更新这些附属对象做准备。

        # Set up the sensors.
        # 设置各种传感器相关的操作，以下分别创建不同类型的传感器对象并进行初始化赋值。
        self.camera = Camera(self.player, self.dim)
        self.rss_unstructured_scene_visualizer = RssUnstructuredSceneVisualizer(self.player, self.world, self.dim)
        self.rss_bounding_box_visualizer = RssBoundingBoxVisualizer(self.dim, self.world, self.camera.sensor)
        self.rss_sensor = RssSensor(self.player, self.world,
                                    self.rss_unstructured_scene_visualizer, self.rss_bounding_box_visualizer, self.hud.rss_state_visualizer)

        if self.sync:
            self.world.tick()
            # 如果 `self.sync` 属性为 `True`（表示处于同步模式），则调用 `self.world.tick` 方法，触发模拟世界进行一次更新操作，
            # 按照之前设置的固定时间步长等同步模式规则来更新世界状态。
        else:
            self.world.wait_for_tick()
            # 如果 `self.sync` 属性为 `False`（表示处于非同步模式），则调用 `self.world.wait_for_tick` 方法，等待模拟世界进行一次更新操作，
            # 非同步模式下更新时间间隔可能不固定，通过这个方法等待世界自然更新。

    def tick(self, clock):
    # 这个方法名为 `tick`，通常在模拟相关的程序中，`tick` 表示一次时间步的更新操作，这里可能是用于更新与世界相关的一些显示或者状态信息等。
    # 接收一个参数 `clock`，从名称推测这个参数可能是用于记录时间相关信息的对象，比如模拟世界的时钟，用于控制更新的节奏或者获取当前时间等信息。
        self.hud.tick(self.player, clock)
        # 调用 `self.hud`（应该是抬头显示相关的对象，之前在类的初始化等操作中已经创建）的 `tick` 方法，传入 `self.player`（可能是模拟世界中的主要演员，比如车辆等可操控对象）和 `clock` 参数。
        # 目的是根据当前模拟世界中的主要演员状态以及时间信息，来更新抬头显示的内容，例如更新车辆的速度、位置等信息在抬头显示界面上的展示情况。
   

    def toggle_recording(self):
    # 这个方法名为 `toggle_recording`，从名称可以推断其功能是用于切换录制状态，也就是实现开启或关闭模拟过程的录制功能。
        if not self.recording:
        # 判断当前是否没有处于录制状态（`self.recording` 为 `False` 表示未录制），如果是，则执行以下操作来开启录制相关的初始化设置。
            dir_name = "_out%04d" % self.recording_dir_num
            # 根据 `self.recording_dir_num`（录制目录编号，初始化为0，每次创建新的录制目录时可能会递增）来生成一个目录名称格式为 `_out` 加上四位数字编号的字符串，
            # 例如 `_out0000`，这个目录将用于存放录制过程中的相关文件，比如图像文件等。
            while os.path.exists(dir_name):
            # 检查当前生成的目录名称对应的目录是否已经存在于文件系统中，如果存在，则执行以下操作来生成一个新的、不存在的目录名称。
                self.recording_dir_num += 1
                # 将录制目录编号递增1，以便生成一个不同的编号用于新的目录名称。
                dir_name = "_out%04d" % self.recording_dir_num
                # 根据更新后的编号重新生成目录名称。
            self.recording_frame_num = 0
            # 将记录当前录制帧数的变量 `self.recording_frame_num` 重置为0，因为即将开启新的录制过程，从第一帧开始记录。
            os.mkdir(dir_name)
            # 使用 `os.mkdir` 函数创建以 `dir_name` 命名的新目录，用于存放本次录制的文件。
        else:
            # 如果当前已经处于录制状态（`self.recording` 为 `True`），则执行以下操作来结束录制，并给出相应提示信息。
            self.hud.notification('Recording finished (folder: _out%04d)' % self.recording_dir_num)
            # 通过 `self.hud`（抬头显示对象）调用 `notification` 方法，向用户显示一条提示信息，告知用户录制已经完成，并显示本次录制文件存放的目录名称（使用 `self.recording_dir_num` 编号来表示）。

        self.recording = not self.recording
        # 对 `self.recording` 属性取反，实现切换录制状态的操作。如果之前是未录制状态（`False`），现在就变为录制状态（`True`），反之亦然。

    def render(self, display):
        # 这个方法名为 `render`，通常用于将模拟世界中的各种元素渲染显示出来，比如渲染车辆、场景、传感器数据可视化等内容，接收一个参数 `display`，
        # 从名称推测这个参数可能是用于显示图像或者图形界面的对象，例如在 `pygame` 等图形库中，可能是代表屏幕显示的对象。
        self.camera.render(display)
        # 调用 `self.camera`（应该是摄像头相关的对象，用于获取图像数据等）的 `render` 方法，传入 `display` 参数，目的是将摄像头获取到的图像内容渲染显示到指定的显示界面上，
        # 让用户可以看到模拟世界中相应视角下的画面。
        self.rss_bounding_box_visualizer.render(display, self.camera.current_frame)
        # 调用 `self.rss_bounding_box_visualizer`（边界框可视化相关对象，可能用于显示车辆、障碍物等物体的边界框）的 `render` 方法，传入 `display`（显示界面对象）和 `self.camera.current_frame`（摄像头当前获取到的图像帧）参数，
        # 这样可以在显示界面上根据摄像头获取的当前画面，渲染出相应物体的边界框，方便用户直观地看到物体的位置和范围等信息。
        self.rss_unstructured_scene_visualizer.render(display)
        # 调用 `self.rss_unstructured_scene_visualizer`（无结构场景可视化相关对象，可能用于展示模拟世界中不规则场景元素的可视化情况）的 `render` 方法，传入 `display` 参数，
        # 将无结构场景相关的可视化内容渲染显示到显示界面上，丰富用户看到的模拟世界的整体视觉呈现效果。
        self.hud.render(display)
        # 调用 `self.hud`（抬头显示对象）的 `render` 方法，传入 `display` 参数，将抬头显示相关的信息（如车速、车辆状态等文字或图形信息）渲染显示到显示界面上，
        # 使得用户可以同时看到模拟世界画面以及相关的重要提示信息。

        if self.recording:
            # 判断当前是否处于录制状态（`self.recording` 为 `True` 表示正在录制），如果是，则执行以下操作来保存当前渲染的画面到文件中，用于录制模拟过程。
            pygame.image.save(display, "_out%04d/%08d.bmp" % (self.recording_dir_num, self.recording_frame_num))
            # 使用 `pygame` 库的 `image.save` 方法，将 `display`（显示界面当前的画面内容）保存为一个 `.bmp` 格式的图像文件，文件路径由录制目录编号（`self.recording_dir_num`）和当前录制帧数（`self.recording_frame_num`）组成，
            # 例如 `_out0000/00000000.bmp`，这样每个录制的画面都会有一个按照顺序编号的文件名，方便后续查看和整理。
            self.recording_frame_num += 1
            # 将当前录制帧数递增1，用于下一次保存图像文件时的文件名编号，保证每个文件的编号是依次递增的，符合录制顺序。

    def destroy(self):
        # 这个方法名为 `destroy`，从名称推测其功能是用于销毁或清理与模拟世界相关的各种资源，比如释放内存、关闭文件、删除对象等操作，避免资源泄漏以及为程序结束或重新初始化做准备。
        # stop from ticking
        if self.world_tick_id:
            # 判断 `self.world_tick_id` 是否存在（不为 `None`），这个属性在之前的代码中可能是用于标识世界每一次更新（tick）时的回调绑定等相关操作的一个标识符，
            # 如果存在，则执行以下操作来移除这个在世界更新时的回调绑定，避免后续不必要的调用或者出现异常。
            self.world.remove_on_tick(self.world_tick_id)
            # 通过 `self.world`（模拟世界对象）调用 `remove_on_tick` 方法，传入 `self.world_tick_id` 参数，解除之前在世界更新时注册的回调关联，停止相应的回调操作。

        if self.camera:
             # 判断 `self.camera`（摄像头相关对象）是否存在（不为 `None`），如果存在，则执行以下操作来销毁摄像头相关的资源，例如释放摄像头占用的内存、关闭相关设备等（具体取决于其实现）。
            self.camera.destroy()
            # 调用 `self.camera` 的 `destroy` 方法，进行摄像头资源的清理操作。
        if self.rss_sensor:
            # 判断 `self.rss_sensor`（与RSS相关的传感器对象）是否存在（不为 `None`），如果存在，则执行以下操作来销毁这个传感器相关的资源。
            self.rss_sensor.destroy()
            # 调用 `self.rss_sensor` 的 `destroy` 方法，进行传感器资源的清理操作。
        if self.rss_unstructured_scene_visualizer:
            # 判断 `self.rss_unstructured_scene_visualizer`（无结构场景可视化对象）是否存在（不为 `None`），如果存在，则执行以下操作来销毁这个可视化相关的资源。
            self.rss_unstructured_scene_visualizer.destroy()
            # 调用 `self.rss_unstructured_scene_visualizer` 的 `destroy` 方法，进行可视化资源的清理操作。
        if self.player:
            # 判断 `self.player`（模拟世界中的主要演员对象，可能是车辆等）是否存在（不为 `None`），如果存在，则执行以下操作来销毁这个演员对象，例如从模拟世界中移除、释放相关内存等（具体取决于其实现）。
            self.player.destroy()
            # 调用 `self.player` 的 `destroy` 方法，进行主要演员对象的清理操作。


# ==============================================================================
# -- Camera --------------------------------------------------------------------
# ==============================================================================

class Camera(object):
# 定义一个名为 `Camera` 的类，这个类大概率是用于模拟环境中相机相关的功能实现，比如相机图像的获取、处理以及显示等操作。

    def __init__(self, parent_actor, display_dimensions):
    # 类的初始化方法，在创建 `Camera` 类的实例时会被调用，用于初始化相机相关的各种属性以及设置相机在模拟世界中的相关参数。
        self.surface = None
        # 用于存储相机获取的图像数据渲染后的表面对象（在图形库中，表面 `surface` 通常是用于最终显示或者进一步处理图像的对象表示），初始化为 `None`，后续会在获取并处理图像后进行赋值。
        self._parent = parent_actor
        # 将传入的 `parent_actor` 参数赋值给实例属性 `_parent`，从名称推测这个参数可能是相机所依附的父对象，比如在模拟世界中相机可能挂载在车辆等主体上，这个父对象就是对应的车辆等主体对象。
        self.current_frame = None
        # 用于存储相机当前获取到的图像帧数据，初始化为 `None`，每当相机获取到新的一帧图像时会进行更新赋值。
        bp_library = self._parent.get_world().get_blueprint_library()
        # 获取相机父对象（也就是 `_parent` 所代表的对象，如车辆）所在的模拟世界的蓝图库（蓝图库中包含了可以创建各种模拟元素的蓝图模板），通过先获取父对象所在的世界，再调用世界对象的 `get_blueprint_library` 方法来实现。
        bp = bp_library.find('sensor.camera.rgb')
        # 从蓝图库中查找名为 `sensor.camera.rgb` 的蓝图，这个蓝图应该是用于创建一个能获取RGB彩色图像的相机传感器，后续会基于这个蓝图来创建实际的相机传感器对象。
        bp.set_attribute('image_size_x', str(display_dimensions[0]))
        # 设置相机蓝图的 `image_size_x` 属性，将其值设置为传入的 `display_dimensions` 参数中表示宽度的元素（通过索引 `0` 获取宽度值，并转换为字符串类型，因为蓝图属性设置可能要求字符串格式的参数），用于指定相机获取图像的宽度尺寸。
        bp.set_attribute('image_size_y', str(display_dimensions[1]))
        # 类似地，设置相机蓝图的 `image_size_y` 属性，将其值设置为 `display_dimensions` 参数中表示高度的元素（通过索引 `1` 获取高度值，并转换为字符串类型），用于指定相机获取图像的高度尺寸。
        self.sensor = self._parent.get_world().spawn_actor(bp, carla.Transform(carla.Location(
            x=-5.5, z=2.5), carla.Rotation(pitch=8.0)), attach_to=self._parent, attachment_type=carla.AttachmentType.SpringArmGhost)
        # 在相机父对象所在的世界中，根据上述配置好的蓝图 `bp`，在指定的位置和姿态（通过 `carla.Transform` 来描述位置和旋转信息，这里位置在 `x = -5.5`，`z = 2.5` 处，旋转的 `pitch` 角度为 `8.0` 度）创建相机传感器对象，并将其附着在父对象 `_parent` 上，附着类型为 `carla.AttachmentType.SpringArmGhost`（一种特定的附着方式，可能实现类似弹簧臂的效果，使相机相对父对象有一定的位置和姿态调整灵活性），将创建好的相机传感器对象赋值给实例属性 `self.sensor`。

        # 以下是为了避免循环引用问题的相关操作。在Python中，如果对象之间相互引用形成闭环，可能会导致内存无法正确回收等问题。
        # 通过 `weakref.ref` 创建对当前 `Camera` 实例（也就是 `self`）的弱引用，弱引用不会增加对象的引用计数，使得对象可以在合适的时候被垃圾回收机制正常回收，将弱引用对象赋值给 `weak_self`。
        # We need to pass the lambda a weak reference to self to avoid
        # circular reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda image: Camera._parse_image(weak_self, image))

    def destroy(self):
     # 定义一个方法用于销毁相机相关的资源，比如停止传感器监听、释放传感器对象等操作，通常在不需要相机或者程序结束等场景下调用。
        self.sensor.stop()
        # 首先调用相机传感器的 `stop` 方法，停止传感器继续获取新的图像数据，避免后续不必要的数据处理以及可能的资源占用。
        self.sensor.destroy()
        self.sensor = None
        # 接着调用相机传感器的 `destroy` 方法，释放与传感器相关的资源，例如内存等，将传感器对象设置为 `None`，表示该相机传感器已被销毁。

    def render(self, display):
    # 定义一个方法用于将相机获取的图像渲染显示到指定的显示界面上，接收一个 `display` 参数，这个参数可能是代表图形显示界面的对象（例如在 `pygame` 等图形库中用于显示图像的对象）。
        if self.surface is not None:
            display.blit(self.surface, (0, 0))
        # 判断 `self.surface`（相机图像渲染后的表面对象）是否不为 `None`，如果是，则将其绘制（ `blit` 操作在图形库中常用于将一个图像表面绘制到另一个表面上）到 `display`（显示界面）的 `(0, 0)` 坐标位置，实现图像的显示。

    @staticmethod
    def _parse_image(weak_self, image):
    # 定义一个静态方法，用于解析处理相机获取到的图像数据，这个方法接收一个弱引用 `weak_self`（指向 `Camera` 类的实例）和图像数据 `image` 作为参数。
    # 静态方法属于类本身，不需要实例化类就可以调用，通常用于处理与类相关但不依赖于具体实例状态的操作。
        self = weak_self()
        if not self:
            return
        # 通过弱引用 `weak_self` 获取对应的 `Camera` 类的实例对象，如果弱引用对应的实例已经被垃圾回收（不存在了），则返回，不进行后续处理。
        self.current_frame = image.frame
        # 将获取到的图像的当前帧编号赋值给实例的 `current_frame` 属性，用于记录当前是第几帧图像，方便后续可能的按帧处理或者显示顺序控制等操作。
        image.convert(cc.Raw)
        # 将图像的数据格式转换为原始数据格式（ `cc.Raw` 应该是代表某种预定义的原始数据格式，通过 `convert` 方法进行转换），方便后续以字节流等形式进行处理。
        array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
        # 使用 `numpy` 库（导入时简记为 `np`）的 `frombuffer` 方法，从图像的原始数据字节流（ `image.raw_data` ）中创建一个 `numpy` 数组，指定数据类型为无符号8位整数（ `dtype=np.dtype("uint8")` ），这样就可以像操作普通数组一样对图像数据进行处理了。
        array = np.reshape(array, (image.height, image.width, 4))
        # 根据图像的高度、宽度以及通道数（这里从原始数据解析出来后可能是4通道，包含透明度等信息），使用 `reshape` 方法将数组重新调整形状，使其符合图像数据的维度结构，得到一个三维数组，分别表示图像的高度、宽度和通道维度。
        array = array[:, :, :3]
        # 去除数组中表示透明度的通道（只取前三个通道，也就是RGB通道），得到一个只包含彩色信息的三维数组，用于后续转换为可显示的图像表面对象。
        array = array[:, :, ::-1]
        # 对数组的通道顺序进行反转，将RGB顺序转换为BGR顺序（不同的图形库或者图像显示系统可能对颜色通道顺序有不同要求，这里可能是为了适配后续使用的图形显示相关的操作）。
        self.surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))
        # 使用 `pygame` 库的 `surfarray.make_surface` 方法，将处理后的 `numpy` 数组转换为 `pygame` 中的图像表面对象（ `surface` ），在转换前先交换数组的第一维和第二维（也就是图像的高度和宽度维度，通过 `swapaxes(0, 1)` 操作），以符合 `pygame` 对图像数据维度的要求，最后将生成的图像表面对象赋值给实例的 `self.surface` 属性，以便后续可以通过 `render` 方法将其显示出来。

# ==============================================================================
# -- VehicleControl -----------------------------------------------------------
# ==============================================================================


class VehicleControl(object):
# 定义一个名为 `VehicleControl` 的类，从名称推测这个类主要用于处理模拟环境中车辆的控制相关操作，比如接收用户输入（键盘、鼠标等）来控制车辆的行驶、灯光等状态。

    MOUSE_STEERING_RANGE = 200
    # 定义一个类属性，表示鼠标转向操作时的有效范围，这里设置为200，可能用于控制鼠标在屏幕上操作车辆转向时的灵敏度或者有效操作区域范围等，单位可能与屏幕坐标等相关（具体取决于整个模拟环境的坐标系设定）。
    signal_received = False
    # 定义一个类属性，用于记录是否接收到特定的信号，初始化为 `False`，在后续的信号处理相关方法中会根据实际情况更新这个值，用于控制程序的一些行为，比如是否停止循环等。

    """Class that handles keyboard input."""
    # 这是一个类的文档字符串，简要说明了这个类的作用是处理键盘输入，用于控制车辆相关操作（实际上从后续代码看也处理了鼠标等其他输入，但这里主要强调了键盘输入方面的功能）。

    def __init__(self, world, start_in_autopilot):
    # 类的初始化方法，在创建 `VehicleControl` 类的实例时会被调用，用于初始化车辆控制相关的各种属性以及设置车辆的初始状态等操作。
        self._autopilot_enabled = start_in_autopilot
        # 将传入的 `start_in_autopilot` 参数赋值给实例属性 `_autopilot_enabled`，用于标识车辆初始是否开启自动驾驶模式，传入的参数应该是一个布尔值， `True` 表示开启， `False` 表示关闭。
        self._world = world
        # 将传入的 `world` 参数赋值给实例属性 `_world`，这个 `world` 参数应该是代表整个模拟世界的对象，通过它可以访问和操作世界中的各种元素，比如车辆、地图、传感器等，后续会用于获取车辆对象以及与世界相关的其他操作。
        self._control = carla.VehicleControl()
        # 创建一个 `carla.VehicleControl` 类的实例，用于存储和管理车辆的控制指令（如油门、刹车、转向等操作指令），并赋值给实例属性 `_control`，初始状态下这些指令的值应该是默认值，后续会根据用户输入等情况进行更新调整。
        self._lights = carla.VehicleLightState.NONE
        # 设置车辆的灯光状态初始化为 `carla.VehicleLightState.NONE`，表示所有灯光都关闭，后续会根据用户操作来切换不同的灯光状态，通过 `_lights` 属性来记录和管理车辆的灯光状态。
        world.player.set_autopilot(self._autopilot_enabled)
        # 通过模拟世界对象 `_world` 获取其中的主要车辆对象（ `world.player` 可能表示模拟世界中的玩家控制车辆，也就是要进行控制操作的目标车辆），并调用其 `set_autopilot` 方法，根据 `_autopilot_enabled` 的值来设置车辆是否开启自动驾驶模式。
        self._restrictor = carla.RssRestrictor()
        # 创建一个 `carla.RssRestrictor` 类的实例（从名称推测这个类可能与某种限制车辆操作的功能相关，也许是基于RSS，即可能是Responsive Safety System等车辆安全相关系统的限制机制），用于后续对车辆控制进行一些限制操作，将其赋值给实例属性 `_restrictor`。
        self._vehicle_physics = world.player.get_physics_control()
        # 获取模拟世界中主要车辆（ `world.player` ）的物理控制相关属性对象（通过 `get_physics_control` 方法获取，这个对象包含了车辆物理特性相关的各种参数，如质量、摩擦力等，可能会影响车辆的行驶行为以及与其他物体的交互情况），并赋值给实例属性 `_vehicle_physics`，用于后续可能的基于车辆物理特性的控制操作。
        world.player.set_light_state(self._lights)
        # 通过模拟世界中的主要车辆对象（ `world.player` ）调用 `set_light_state` 方法，将车辆灯光状态设置为之前初始化的 `_lights` 的值，也就是初始关闭所有灯光。
        self._steer_cache = 0.0
        # 用于缓存车辆的转向值，初始化为0.0，在处理键盘或者鼠标输入的转向操作时，会先将转向值暂存到这个变量中，然后再更新到 `_control` 对象里的正式转向指令属性中，可能用于平滑转向操作或者处理一些特殊的转向逻辑等情况。
        self._mouse_steering_center = None
        # 用于记录鼠标转向操作时的中心位置坐标，初始化为 `None`，当鼠标按下进行转向操作时会记录下鼠标的当前位置作为中心位置，鼠标松开时会重置为 `None`，用于后续根据鼠标移动相对于这个中心位置来计算转向等操作。

        self._surface = pygame.Surface((self.MOUSE_STEERING_RANGE * 2, self.MOUSE_STEERING_RANGE * 2))
        # 创建一个 `pygame` 库中的 `Surface` 对象（表面对象，用于图形绘制、显示等操作），其大小由 `MOUSE_STEERING_RANGE` 属性决定，这里创建的表面大小是 `(MOUSE_STEERING_RANGE * 2, MOUSE_STEERING_RANGE * 2)`，可能用于后续在屏幕上显示与鼠标转向相关的提示信息或者可视化元素等操作，将其赋值给实例属性 `_surface`。
        self._surface.set_colorkey(pygame.Color('black'))
        # 设置表面对象的透明颜色键（ `colorkey` ）为黑色（ `pygame.Color('black')` ），这意味着在显示这个表面时，所有黑色的像素点会被当作透明处理，方便实现一些特殊的图形显示效果，比如只显示特定颜色部分的图像等。
        self._surface.set_alpha(60)
        # 设置表面对象的透明度为60（取值范围通常是0 - 255，表示从完全透明到完全不透明，这里设置为60表示半透明效果），使得这个表面在显示时呈现出一定的透明状态，可能用于叠加显示在其他图形元素之上，不遮挡太多背景内容等情况。

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
        # 使用 `pygame` 库的 `draw.polygon` 方法在之前创建的表面对象 `_surface` 上绘制多边形，这里绘制的是一个蓝色（颜色值为 `(0, 0, 255)` ）的多边形，多边形的顶点坐标通过给定的列表来指定，并且设置多边形的线宽为2像素，这个多边形可能是用于在屏幕上显示与鼠标转向相关的操作提示或者区域范围等可视化信息。        

        world.hud.notification("Press 'H' or '?' for help.", seconds=4.0)
        # 通过模拟世界对象的 `hud`（抬头显示相关对象，用于显示各种提示信息等）调用 `notification` 方法，向用户显示一条提示信息，告知用户可以按下 `H` 键或者 `?` 键获取帮助信息，并且设置这条提示信息显示的时长为4.0秒，之后会自动消失。

    def render(self, display):
    # `render` 方法用于将与鼠标转向相关的可视化元素渲染显示到指定的显示界面上。
    # 接收一个 `display` 参数，这个参数通常代表图形显示界面的对象（例如在 `pygame` 等图形库中用于显示图像的对象）。
        if self._mouse_steering_center:
        # 判断 `self._mouse_steering_center` 是否存在（不为 `None`），它记录着鼠标转向操作时的中心位置坐标。
        # 如果存在，则执行以下操作将之前创建的带有转向提示等可视化元素的表面对象（`self._surface`）绘制到显示界面上。
            display.blit(
                self._surface, (self._mouse_steering_center[0] - self.MOUSE_STEERING_RANGE, self._mouse_steering_center[1] - self.MOUSE_STEERING_RANGE))
            # 使用 `display` 对象的 `blit` 方法将 `self._surface` 绘制到显示界面上，绘制的位置根据 `self._mouse_steering_center` 坐标进行偏移，
            # 通过减去 `MOUSE_STEERING_RANGE` 来确保可视化元素能以鼠标转向中心位置为参照，正确地显示在合适的区域，方便用户直观地看到与鼠标转向操作相关的提示信息。

    @staticmethod
    def signal_handler(signum, _):
    # 这是一个静态方法，用于处理接收到的信号。静态方法属于类本身，不需要实例化类就可以调用，通常用于处理与类相关但不依赖于具体实例状态的操作。
    # 接收信号编号 `signum` 参数以及一个未使用的占位参数（按照Python惯例，用下划线 `_` 表示不使用的参数）
        print('\nReceived signal {}. Trigger stopping...'.format(signum))
        # 当接收到信号时，打印出接收到的信号编号以及提示信息，表示接收到信号并即将触发停止相关操作。
        VehicleControl.signal_received = True
        # 将类属性 `signal_received` 设置为 `True`，这个类属性用于在其他地方（比如循环控制等逻辑中）判断是否接收到了信号，从而决定是否停止某些操作或者整个程序的运行等情况。

    def parse_events(self, world, clock, sync_mode):
    # `parse_events` 方法用于解析处理各种输入事件（如键盘按键事件、鼠标点击事件等），并根据不同的事件类型执行相应的操作，以控制车辆的状态以及模拟世界中的相关显示等功能。
    # 接收 `world`（模拟世界对象，通过它可以访问和操作世界中的各种元素）、`clock`（可能是用于记录时间相关信息的对象，用于控制更新节奏或者获取当前时间等信息）和 `sync_mode`（可能表示同步模式相关的参数，用于确定模拟世界是否以同步方式运行等情况）这几个参数
        if VehicleControl.signal_received:
        # 首先判断类属性 `VehicleControl.signal_received` 是否为 `True`，即是否接收到了特定信号，如果是，则执行以下操作来停止相关循环或操作流程。
            print('\nAccepted signal. Stopping loop...')
            return True
             # 打印提示信息表示接受了信号并即将停止循环，然后返回 `True`，这个返回值可能在调用该方法的外层循环等逻辑中用于判断是否跳出循环，终止后续操作。
        if isinstance(self._control, carla.VehicleControl):
        # 判断 `self._control` 是否是 `carla.VehicleControl` 类型的对象（在初始化时创建了这个对象用于存储车辆的控制指令），如果是，则执行以下操作来获取当前的车辆灯光状态备份。
            current_lights = self._lights
             # 将当前车辆的灯光状态（记录在 `self._lights` 属性中）赋值给 `current_lights` 变量，用于后续在处理事件过程中，根据车辆控制指令的变化来相应地更新车辆灯光状态时做对比和判断等操作。
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
