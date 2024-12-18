# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys
import unittest

try:
    # 尝试将 Carla 库的路径添加到 Python 解释器的搜索路径中
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla
import time

# 测试使用的地址和端口
TESTING_ADDRESS = ('localhost', 3654)
# 不允许在旧城镇中使用的车辆类型列表
VEHICLE_VEHICLES_EXCLUDE_FROM_OLD_TOWNS = ['vehicle.mitsubishi.fusorosa', 'vehicle.carlamotors.european_hgv', 'vehicle.carlamotors.firetruck']


class SmokeTest(unittest.TestCase):
    def setUp(self):
        # 初始化测试环境，设置测试地址
        self.testing_address = TESTING_ADDRESS
        # 创建 Carla 客户端对象，连接到指定地址
        self.client = carla.Client(*TESTING_ADDRESS)
        # 存储不允许在旧城镇中使用的车辆类型列表
        self.vehicle_vehicles_exclude_from_old_towns = VEHICLE_VEHICLES_EXCLUDE_FROM_OLD_TOWNS
        # 设置客户端超时时间为 120 秒
        self.client.set_timeout(120.0)
        # 获取 Carla 世界对象
        self.world = self.client.get_world()

    def tearDown(self):
        # 测试结束后加载 Town03 场景
        self.client.load_world("Town03")
        # 等待 5 秒，给 UE4 时间清理内存（旧资源）
        time.sleep(5)
        # 释放世界对象
        self.world = None
        # 释放客户端对象
        self.client = None

    def filter_vehicles_for_old_towns(self, blueprint_list):
        # 用于存储过滤后的车辆蓝图列表
        new_list = []
        # 遍历输入的车辆蓝图列表
        for blueprint in blueprint_list:
            # 如果蓝图的 id 不在排除列表中，则添加到新列表中
            if blueprint.id not in self.vehicle_vehicles_exclude_from_old_towns:
                new_list.append(blueprint)
        return new_list


class SyncSmokeTest(SmokeTest):
    def setUp(self):
        # 调用父类的 setUp 方法进行初始化
        super(SyncSmokeTest, self).setUp()
        # 获取当前世界的设置
        self.settings = self.world.get_settings()
        # 创建新的世界设置对象，设置无渲染模式为 False，同步模式为 True，固定时间间隔为 0.05 秒
        settings = carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=0.05)
        # 应用新的世界设置
        self.world.apply_settings(settings)
        # 执行一次世界的 tick 操作
        self.world.tick()

    def tearDown(self):
        # 恢复世界设置
        self.world.apply_settings(self.settings)
        # 执行一次世界的 tick 操作
        self.world.tick()
        # 释放设置对象
        self.settings = None
        # 调用父类的 tearDown 方法进行清理操作
        super(SyncSmokeTest, self).tearDown()
