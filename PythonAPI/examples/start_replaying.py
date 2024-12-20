#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys

# 尝试将Carla模块所在的路径添加到Python的系统路径中，以便能够正确导入Carla相关模块
# 根据Python版本（major和minor）以及操作系统类型（win-amd64或linux-x86_64）来确定具体的.egg文件路径
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla
import argparse

def main():
    # 创建一个命令行参数解析器对象，用于解析用户输入的各种参数
    argparser = argparse.ArgumentParser(
        description=__doc__)
    # 添加主机IP地址参数，默认值为127.0.0.1，用于指定要连接的服务器IP
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    # 添加端口号参数，默认值为2000，指定要监听的TCP端口，类型为整数
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    # 添加起始时间参数，默认值为0.0，用于指定重放操作开始的时间，类型为浮点数
    argparser.add_argument(
        '-s', '--start',
        metavar='S',
        default=0.0,
        type=float,
        help='starting time (default: 0.0)')
    # 添加持续时间参数，默认值为0.0，用于指定重放操作持续的时长，类型为浮点数
    argparser.add_argument(
        '-d', '--duration',
        metavar='D',
        default=0.0,
        type=float,
        help='duration (default: 0.0)')
    # 添加记录文件名参数，默认值为"test1.log"，用于指定重放操作所使用的记录文件名称
    argparser.add_argument(
        '-f', '--recorder-filename',
        metavar='F',
        default="test1.log",
        help='recorder filename (test1.log)')
    # 添加相机相关参数，默认值为0，可能用于指定相机跟随的某个参与者（actor）的编号之类的情况，类型为整数
    argparser.add_argument(
        '-c', '--camera',
        metavar='C',
        default=0,
        type=int,
        help='camera follows an actor (ex: 82)')
    # 添加时间因子参数，默认值为1.0，用于设置重放时的时间缩放因子，类型为浮点数
    argparser.add_argument(
        '-x', '--time-factor',
        metavar='X',
        default=1.0,
        type=float,
        help='time factor (default 1.0)')
    # 添加是否忽略主角车辆的参数，是一个布尔类型的标志参数，若指定则表示忽略主角车辆
    argparser.add_argument(
        '-i', '--ignore-hero',
        action='store_true',
        help='ignore hero vehicles')
    # 添加是否移动旁观者相机的参数，是一个布尔类型的标志参数，若指定则表示移动旁观者相机
    argparser.add_argument(
        '--move-spectator',
        action='store_true',
        help='move spectator camera')
    # 添加是否在重放的世界中生成传感器的参数，是一个布尔类型的标志参数，若指定则表示生成传感器
    argparser.add_argument(
        '--spawn-sensors',
        action='store_true',
        help='spawn sensors in the replayed world')
    args = argparser.parse_args()

    try:
        # 创建一个Carla客户端对象，使用命令行参数中指定的主机IP和端口号进行连接
        client = carla.Client(args.host, args.port)
        # 设置客户端操作的超时时间为60.0秒，若超过该时间操作未完成则可能抛出异常
        client.set_timeout(60.0)

        # 设置重放器的时间因子，根据命令行参数传入的值来调整重放时的时间快慢
        client.set_replayer_time_factor(args.time-factor)

        # 根据命令行参数设置是否忽略主角车辆，传入的是布尔值（取反操作是因为函数参数含义与命令行参数的逻辑对应关系）
        client.set_replayer_ignore_hero(args.ignore_hero)

        # 根据命令行参数设置是否忽略旁观者相机，传入的是布尔值（取反操作是因为函数参数含义与命令行参数的逻辑对应关系）
        client.set_replayer_ignore_spectator(not args.move_spectator)

        # 调用客户端对象的replay_file方法来重放指定的记录文件，传入一系列命令行参数指定的相关设置，并打印重放的结果
        print(client.replay_file(args.recorder-filename, args.start, args.duration, args.camera, args.spawn-sensors))

    finally:
        pass


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')