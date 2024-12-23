#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Blocks until the simulator is ready or the time-out is met."""

#导入三个模块
import glob
import os
import sys

#异常处理结构
try:
    #确保能够正确导入carla模块
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass


#尝试导入carla模块，以便在后续代码中使用carla相关的功能
import carla

#argparse模块用于处理命令行参数
import argparse
#这个模块用于处理时间相关的操作
import time


#将主要的逻辑放在这样一个函数中，以便组织代码和在脚本的其他部分调用
def main():
    argparser = argparse.ArgumentParser(
        description=__doc__)
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
        '--timeout',
        metavar='T',
        default=10.0,
        type=float,
        help='time-out in seconds (default: 10)')
    args = argparser.parse_args()

    t0 = time.time()

    while args.timeout > (time.time() - t0):
        try:
            client = carla.Client(args.host, args.port)
            client.set_timeout(0.1)
            print('CARLA %s connected at %s:%d.' % (client.get_server_version(), args.host, args.port))
            return 0
        except RuntimeError:
            pass

    print('Failed to connect to %s:%d.' % (args.host, args.port))
    return 1


if __name__ == '__main__':

    sys.exit(main())
