#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys

# 尝试将Carla库的相关路径添加到Python的模块搜索路径中，根据不同的操作系统（Windows或Linux）选择对应的库文件（以.egg格式存在）
# 通过格式化字符串，根据当前Python版本信息（主版本号和次版本号）以及操作系统类型来匹配正确的库文件路径
# 如果找到了对应的.egg文件，就将其路径添加到sys.path中，以便后续能够正确导入carla模块
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

# 导入Carla库，Carla可能是一个用于自动驾驶模拟等相关功能的库，提供了创建虚拟场景、控制车辆等诸多功能的接口
import carla

import argparse

def main():
    # 创建一个命令行参数解析器对象，使用模块文档字符串作为描述信息，用于解析后续从命令行传入的参数
    argparser = argparse.ArgumentParser(
        description=__doc__)
    # 添加一个名为'--host'的命令行参数，用于指定主机服务器的IP地址，默认值为'127.0.0.1'，也就是本地主机地址，帮助信息简要说明了其作用
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    # 添加一个名为'-p'或'--port'的命令行参数，用于指定要监听的TCP端口号，默认值为2000，类型为整数，帮助信息说明了其作用及默认值情况
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    # 添加一个名为'-f'或'--recorder_filename'的命令行参数，用于指定记录器文件名，默认值为"test1.rec"，帮助信息说明了其默认值情况
    argparser.add_argument(
        '-f', '--recorder_filename',
        metavar='F',
        default="test1.rec",
        help='recorder filename (test1.rec)')
    # 添加一个名为'-t'或'--time'的命令行参数，用于指定判断阻塞的最小时间，默认值为"30"（这里单位应该是秒，从参数含义推测），类型为浮点数，帮助信息说明了其作用及默认值情况
    argparser.add_argument(
        '-t', '--time',
        metavar='T',
        default="30",
        type=float,
        help='minimum time to consider it is blocked')
    # 添加一个名为'-d'或'--distance'的命令行参数，用于指定判断未移动的最小距离（单位为厘米），默认值为"100"，类型为浮点数，帮助信息说明了其作用及默认值情况
    argparser.add_argument(
        '-d', '--distance',
        metavar='D',
        default="100",
        type=float,
        help='minimum distance to consider it is not moving moving (in cm)')
    # 解析命令行传入的参数，将解析结果保存在args对象中，后续可以通过args的属性来获取各个参数的值
    args = argparser.parse_args()

    try:
        # 使用指定的主机IP地址和端口号创建一个Carla客户端对象，用于与Carla服务器进行通信交互
        client = carla.Client(args.host, args.port)
        # 设置客户端的超时时间为60.0秒，即如果在60秒内没有收到服务器响应，则认为操作超时
        client.set_timeout(60.0)

        # 调用客户端的show_recorder_actors_blocked方法，传入记录器文件名以及判断阻塞的最小时间和最小距离参数，
        # 该方法可能用于展示记录器中哪些角色（actors，在Carla中可能指代车辆、行人等各种实体）处于阻塞状态（根据传入的时间和距离条件判断），
        # 并打印该方法的返回结果
        print(client.show_recorder_actors_blocked(args.recorder_filename, args.time, args.distance))

    finally:
        pass


if __name__ == '__main__':
    try:
        main()
    # 捕获键盘中断异常（比如用户按下Ctrl+C），不做任何具体处理，只是为了程序能优雅地结束，避免异常退出
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
 # 'finally'块中的代码无论是否发生异常都会被执行。在这里，不管'main'函数执行过程中有没有出现异常，
        # 也不管有没有捕获到键盘中断异常，都会打印出'\ndone.'这个字符串，用于向用户提示程序已经结束了，起到一个友好的提示作用。
