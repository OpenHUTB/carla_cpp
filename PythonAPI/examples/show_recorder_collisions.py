#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys

# 尝试将Carla库对应的模块路径添加到Python的系统路径中，以便后续能正确导入carla模块。
# 具体做法是通过查找符合特定命名格式的.egg文件（Carla库在不同操作系统和Python版本下的一种打包格式）。
# 根据当前Python版本的主版本号（sys.version_info.major）、次版本号（sys.version_info.minor）以及操作系统类型（通过os.name判断，'nt'表示Windows，'linux-x86_64'表示Linux）来构造匹配的文件名模式，
# 然后使用glob.glob获取匹配的文件路径列表，并取其中第一个路径（[0]）添加到sys.path中。
# 如果没有找到匹配的文件（IndexError异常），则跳过添加操作，直接往下执行。
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

# 导入Carla库，这是一个用于自动驾驶模拟等相关功能的库，提供了创建虚拟场景、控制车辆等诸多功能的接口，后续将基于它来实现与Carla服务器的交互操作。
import carla

# 导入Python的标准库argparse，用于方便地解析命令行传入的参数，使得程序可以通过命令行来配置不同的运行参数。
import argparse


def main():
    # 创建一个argparse.ArgumentParser对象，用于定义和解析命令行参数。
    # 使用脚本开头的文档字符串（__doc__）作为这个参数解析器的描述信息，这样在使用--help选项查看帮助时，会显示这段文档字符串内容，帮助用户了解程序的基本功能和参数用法。
    argparser = argparse.ArgumentParser(
        description=__doc__)
    # 添加一个名为'--host'的命令行参数，用于指定Carla服务器所在主机的IP地址。
    # metavar参数定义了在帮助信息中显示该参数时使用的变量名（这里是'H'），主要是为了让帮助信息更美观、易读。
    # default参数指定了该参数的默认值为'127.0.0.1'，也就是本地主机地址，意味着如果用户在运行程序时没有通过命令行指定这个参数，程序将默认连接本地的Carla服务器。
    # help参数提供了对这个参数的详细说明，告知用户该参数的作用以及默认值情况，方便用户根据实际需求进行配置。
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    # 添加一个名为'-p'或'--port'的命令行参数，用于指定客户端要监听的TCP端口号。
    # metavar参数同样用于帮助信息展示，这里设为'P'。
    # default参数将默认值设为2000，表示如果用户未指定端口号，程序将默认使用2000端口与服务器进行通信。
    # type参数指定了该参数的数据类型为整数（int），确保用户传入的值能正确转换为整数类型进行后续处理。
    # help参数对参数作用及默认值进行了说明，方便用户了解如何根据实际情况调整端口号。
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    # 添加一个名为'-f'或'--recorder_filename'的命令行参数，用于指定记录器的文件名，这个记录器文件可能包含了Carla模拟场景中的一些数据记录，比如车辆行驶轨迹、碰撞情况等信息。
    # metavar设为'F'，用于帮助信息展示。
    # default参数指定了默认文件名是"test1.rec"，当用户没有在命令行指定文件名时，程序将默认查找或操作这个名为"test1.rec"的记录器文件。
    # help参数说明了该参数对应的默认文件名情况，让用户知晓如何根据需要指定不同的记录器文件名。
    argparser.add_argument(
        '-f', '--recorder_filename',
        metavar='F',
        default="test1.rec",
        help='recorder filename (test1.rec)')
    # 添加一个名为'-t'或'--types'的命令行参数，用于指定碰撞类型的配对情况，以此来筛选想要查看的碰撞记录类型。
    # metavar设为'T'，用于帮助信息展示。
    # default参数将默认值设为"aa"，这里的每个字符都有特定含义（在帮助信息中有详细说明），代表不同的实体类型（如'a'表示任意，'h'表示主角，'v'表示车辆等），默认的"aa"表示显示任意类型到任意类型的碰撞记录，也就是全部碰撞记录。
    # help参数详细解释了不同字符代表的实体类型以及不同组合下显示碰撞记录的规则示例，帮助用户理解如何通过该参数来筛选碰撞记录，例如"vv"表示只看车辆与车辆之间的碰撞记录等。
    argparser.add_argument(
        '-t', '--types',
        metavar='T',
        default="aa",
        help='pair of types (a=any, h=hero, v=vehicle, w=walkers, t=trafficLight, o=others')
    # 调用argparse.ArgumentParser对象的parse_args()方法，解析从命令行传入的参数，并将解析结果保存在args对象中。
    # 后续可以通过访问args的不同属性（如args.host、args.port等）来获取各个参数的值，方便在程序中使用用户指定的参数进行相应操作。
    args = argparser.parse_args()

    try:
        # 使用通过命令行参数指定的主机IP地址（args.host）和端口号（args.port）创建一个Carla客户端对象。
        # 这个客户端对象是与Carla服务器进行通信交互的接口，通过它可以向服务器发送请求、获取服务器端的数据等操作。
        client = carla.Client(args.host, args.port)
        # 为客户端设置超时时间为60.0秒，即如果客户端向服务器发送请求后，在60秒内没有收到服务器的相应回复，就会判定此次操作超时，抛出相应的异常，避免程序长时间无响应地等待。
        client.set_timeout(60.0)

        # 以下是对不同碰撞类型模式的详细解释，方便用户理解如何通过命令行参数'-t'（或'--types'）来筛选碰撞记录：
        # - -t aa 表示任意类型到任意类型的碰撞，也就是会显示所有的碰撞记录（这是默认情况），意味着只要有任何实体之间发生碰撞，都会在结果中展示出来。
        # - -t vv 表示车辆到车辆的碰撞，意味着只会展示车辆之间发生的每一次碰撞情况，其他类型实体之间的碰撞不会显示，方便用户聚焦查看车辆之间的碰撞情况。
        # - -t vt 表示车辆到交通信号灯的碰撞，即会显示车辆与交通信号灯之间的每一次碰撞记录，有助于分析车辆与交通信号灯相关的交互情况。
        # - -t hh 表示主角到主角的碰撞，会展示一个主角与另一个主角之间的碰撞情况，这里的“主角”可能是模拟场景中有特定标识或角色的实体，具体取决于Carla的场景设定。
        # 调用客户端的show_recorder_collisions方法，传入记录器文件名（args.recorder_filename）以及通过命令行参数指定的两种碰撞类型字符（args.types[0]和args.types[1]），
        # 该方法可能用于从记录器文件中筛选并展示符合指定碰撞类型的碰撞记录信息，然后将这个方法返回的结果进行打印输出，使得用户可以在终端看到符合条件的碰撞记录相关内容。
        print(client.show_recorder_collisions(args.recorder_filename, args.types[0], args.types[1]))

    finally:
        pass


if __name__ == '__main__':
    try:
        main()
    # 捕获键盘中断异常（比如用户按下Ctrl+C），在捕获到该异常时不做任何具体处理操作，只是简单地让程序能够相对优雅地结束运行，避免异常崩溃退出，给用户一个相对友好的交互体验。
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
