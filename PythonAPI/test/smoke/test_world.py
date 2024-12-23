# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# 从当前目录（相对导入）导入SmokeTest类，这里暗示可能存在某种测试框架相关的基础类结构，SmokeTest类或许包含了一些通用的测试相关的功能和属性
from. import SmokeTest


# 定义一个名为TestWorld的类，它继承自SmokeTest类，意味着它可以复用SmokeTest类中的方法和属性，大概率是用于针对"世界"相关功能的测试场景
class TestWorld(SmokeTest):
    # 定义一个测试方法，用于测试固定时间步长（fixed_delta_seconds）相关的功能
    def test_fixed_delta_seconds(self):
        # 打印测试方法开始执行的提示信息，方便在测试运行时查看具体执行到哪个测试环节
        print("TestWorld.test_fixed_delta_seconds")
        # 通过客户端（client，具体来源可能在SmokeTest类或者其继承体系中有定义）获取游戏世界（world）对象，这个世界对象通常包含了游戏场景中的各种设置、实体等相关信息
        world = self.client.get_world()
        # 从获取到的世界对象中获取其设置（settings）信息，这些设置信息可能包含了如同步模式、时间步长等诸多影响世界运行状态的参数
        settings = world.get_settings()
        # 断言当前世界的设置中同步模式（synchronous_mode）为False，即验证当前不是处于同步模式下，这可能是该测试方法的前置条件假设或者是一种状态检查
        self.assertFalse(settings.synchronous_mode)
        # 遍历一个预定义的期望时间步长（fixed_delta_seconds）列表，这些值可能是常见的或者测试重点关注的时间步长取值
        for expected_delta_seconds in [0.1, 0.066667, 0.05, 0.033333, 0.016667, 0.011112]:
            # 将世界设置中的固定时间步长设置为当前遍历到的期望时间步长值，这一步是在模拟设置不同的时间步长场景
            settings.fixed_delta_seconds = expected_delta_seconds
            # 将更新后的设置应用到世界对象中，使设置生效，以便后续验证在该设置下世界的运行情况是否符合预期
            world.apply_settings(settings)
            # 循环20次，这里可能是为了多次验证在当前设置的固定时间步长下世界运行时的时间步长情况，以确保稳定性和准确性
            for _ in range(0, 20):
                # 等待世界进行一次 tick（可以理解为一次时间推进或者更新操作），并获取此次tick对应的时间戳信息中的时间步长（delta_seconds）
                delta_seconds = world.wait_for_tick().timestamp.delta_seconds
                # 使用断言来比较实际获取到的时间步长与期望的时间步长是否近似相等（考虑到浮点数运算的精度问题，使用近似相等的断言），以此验证在设置了特定固定时间步长后世界实际运行的时间步长是否符合预期
                self.assertAlmostEqual(expected_delta_seconds, delta_seconds)
        # 将世界设置中的固定时间步长设置为None，可能是为了恢复到默认或者初始的时间步长设置状态，以便不影响后续其他测试或者世界的正常使用
        settings.fixed_delta_seconds = None
        # 将更新后的设置（将固定时间步长设置为None的设置）应用到世界对象中，使设置生效
        world.apply_settings(settings)
