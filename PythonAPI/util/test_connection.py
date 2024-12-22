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

    while args.timeout > (time.time() - t0):# 进入一个循环，只要设定的超时时间（args.timeout）大于从起始时间（t0）到当前时间（time.time()）的时间差，就持续尝试连接
        try:
            client = carla.Client(args.host, args.port)# 获取CARLA服务器版本信息，并打印出连接成功的提示信息，包含服务器版本、连接的主机和端口
            client.set_timeout(0.1) # 为客户端设置超时时间为0.1秒，意味着如果在0.1秒内没有得到服务器响应，将会抛出异常
            print('CARLA %s connected at %s:%d.' % (client.get_server_version(), args.host, args.port))  # 如果成功连接并执行到这里，返回0表示连接成功，函数正常结束
            return 0
        except RuntimeError: # 如果在尝试连接过程中出现运行时错误（比如超时等情况），不做任何处理，直接进入下一次循环继续尝试连接
            pass

    print('Failed to connect to %s:%d.' % (args.host, args.port))# 如果循环结束后仍未成功连接（即超出了设定的超时时间），打印出连接失败的提示信息，包含尝试连接的主机和端口
    return 1 # 返回1表示连接失败，程序以非零状态码退出，通常可用于表示出现了错误情况


if __name__ == '__main__':    
    # 调用main函数，并使用sys.exit()来根据main函数的返回值退出整个程序，
    # 如果main函数返回0则正常退出，返回其他值则以对应错误码退出程序
    sys.exit(main())

    sys.exit(main())
