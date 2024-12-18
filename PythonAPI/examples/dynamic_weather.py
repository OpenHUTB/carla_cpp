#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
CARLA Dynamic Weather:

Connect to a CARLA Simulator instance and control the weather. Change Sun
position smoothly with time and generate storms occasionally.
"""

import glob
import os
import sys

# 尝试将CARLA相关的模块路径添加到系统路径中，根据Python版本和操作系统选择合适的egg文件路径
# 如果是Windows系统（os.name == 'nt'），则选择win-amd64架构的文件，否则选择linux-x86_64架构的文件
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

import argparse
import math


# 限制值在给定的最小值和最大值范围内，返回限制后的值
def clamp(value, minimum=0.0, maximum=100.0):
    return max(minimum, min(value, maximum))


# 表示太阳的类，包含太阳的方位角和高度角信息，以及时间相关的属性和更新方法
class Sun(object):
    def __init__(self, azimuth, altitude):
        self.azimuth = azimuth
        self.altitude = altitude
        self._t = 0.0

    # 根据时间间隔更新太阳的状态（方位角、高度角）
    # _t是一个内部的时间相关变量，用于计算高度角的周期性变化
    def tick(self, delta_seconds):
        self._t += 0.008 * delta_seconds
        self._t %= 2.0 * math.pi
        self.azimuth += 0.25 * delta_seconds
        self.azimuth %= 360.0
        self.altitude = (70 * math.sin(self._t)) - 20

    def __str__(self):
        return 'Sun(alt: %.2f, azm: %.2f)' % (self.altitude, self.azimuth)


# 表示风暴的类，包含各种天气相关属性（云量、降雨量等）以及更新这些属性的方法
class Storm(object):
    def __init__(self, precipitation):
        self._t = precipitation if precipitation > 0.0 else -50.0
        self._increasing = True
        self.clouds = 0.0
        self.rain = 0.0
        self.wetness = 0.0
        self.puddles = 0.0
        self.wind = 0.0
        self.fog = 0.0

    # 根据时间间隔更新风暴相关的各种天气属性，例如云量、降雨量、积水情况等
    # 根据_increasing标志来决定属性是增加还是减少，同时会根据一些规则限制属性值的范围
    def tick(self, delta_seconds):
        delta = (1.3 if self._increasing else -1.3) * delta_seconds
        self._t = clamp(delta + self._t, -250.0, 100.0)
        self.clouds = clamp(self._t + 40.0, 0.0, 90.0)
        self.rain = clamp(self._t, 0.0, 80.0)
        delay = -10.0 if self._increasing else 90.0
        self.puddles = clamp(self._t + delay, 0.0, 85.0)
        self.wetness = clamp(self._t * 5, 0.0, 100.0)
        self.wind = 5.0 if self.clouds <= 20 else 90 if self.clouds >= 70 else 40
        self.fog = clamp(self._t - 10, 0.0, 30.0)
        if self._t == -250.0:
            self._increasing = True
        if self._t == 100.0:
            self._increasing = False

    def __str__(self):
        return 'Storm(clouds=%d%%, rain=%d%%, wind=%d%%)' % (self.clouds, self.rain, self.wind)


# 综合管理天气的类，包含了太阳和风暴相关的对象，并负责整体天气状态的更新和设置
class Weather(object):
    def __init__(self, weather):
        self.weather = weather
        self._sun = Sun(weather.sun_azimuth_angle, weather.sun_altitude_angle)
        self._storm = Storm(weather.precipitation)

    # 更新整体天气状态，调用太阳和风暴对象的更新方法，并将更新后的天气属性设置到对应的天气对象中
    def tick(self, delta_seconds):
        self._sun.tick(delta_seconds)
        self._storm.tick(delta_seconds)
        self.weather.cloudiness = self._storm.clouds
        self.weather.precipitation = self._storm.rain
        self.weather.precipitation_deposits = self._storm.puddles
        self.weather.wind_intensity = self._storm.wind
        self.weather.fog_density = self._storm.fog
        self.weather.wetness = self._storm.wetness
        self.weather.sun_azimuth_angle = self._sun.azimuth
        self.weather.sun_altitude_angle = self._sun.altitude

    def __str__(self):
        return '%s %s' % (self._sun, self._storm)


def main():
    # 创建命令行参数解析器，用于解析用户输入的参数，文档字符串描述了程序的基本功能
    argparser = argparse.ArgumentParser(
        description=__doc__)
    # 添加主机IP参数，默认值为127.0.0.1，用于指定连接CARLA模拟器的服务器IP地址
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    # 添加端口参数，默认值为2000，用于指定连接CARLA模拟器的TCP端口号
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    # 添加天气变化速度参数，默认值为1.0，用于控制天气变化的速率
    argparser.add_argument(
        '-s', '--speed',
        metavar='FACTOR',
        default=1.0,
        type=float,
        help='rate at which the weather changes (default: 1.0)')
    args = argparser.parse_args()

    speed_factor = args.speed
    # 根据天气变化速度计算更新频率，用于控制天气更新的时间间隔
    update_freq = 0.1 / speed_factor

    # 创建CARLA客户端对象，用于连接到CARLA模拟器服务器，设置连接超时时间为2秒
    client = carla.Client(args.host, args.port)
    client.set_timeout(2.0)
    # 获取CARLA模拟器中的世界对象，后续用于获取和设置天气等操作
    world = client.get_world()

    # 根据当前世界的天气状况创建Weather对象，用于管理和更新天气
    weather = Weather(world.get_weather())

    elapsed_time = 0.0

    # 主循环，不断更新天气状态并设置到世界对象中，同时在控制台输出当前天气状态信息
    while True:
        # 等待世界的时间滴答，获取时间戳信息，等待最长时间为30秒
        timestamp = world.wait_for_tick(seconds=30.0).timestamp
        elapsed_time += timestamp.delta_seconds
        if elapsed_time > update_freq:
            weather.tick(speed_factor * elapsed_time)
            world.set_weather(weather.weather)
            # 在控制台输出当前天气状态信息，通过'\r'实现覆盖上一次输出内容，保持在同一行显示更新后的信息
            sys.stdout.write('\r' + str(weather) + 12 * ' ')
            sys.stdout.flush()
            elapsed_time = 0.0


if __name__ == '__main__':
    main()
