# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla
import random
import time

from. import SyncSmokeTest

# 定义一个测试类，用于测试与世界快照（Snapshot）相关的功能，继承自SyncSmokeTest（可能是自定义的测试基类）
class TestSnapshot(SyncSmokeTest):
    def test_spawn_points(self):
        print("TestSnapshot.test_spawn_points")
        # 重新加载世界，获取新的世界实例，这可能用于重置世界状态等操作
        self.world = self.client.reload_world()
        # 工作区处理：给UE4（Unreal Engine 4，Carla基于的游戏引擎）一些时间来清理加载旧资源后的内存，等待5秒
        time.sleep(5)

        # 检查为什么世界设置在重新加载后没有被应用，先获取当前世界的设置
        self.settings = self.world.get_settings()
        # 创建新的世界设置对象，设置如下参数：
        # no_rendering_mode：是否开启无渲染模式，这里设置为False，即正常渲染模式
        # synchronous_mode：是否开启同步模式，设置为True，意味着世界的更新将按照固定的时间步长进行，便于精确控制和同步相关操作
        # fixed_delta_seconds：固定的时间步长，设置为0.05秒，即每一次世界更新的时间间隔为0.05秒
        settings = carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=0.05)
        # 将新的世界设置应用到当前世界中
        self.world.apply_settings(settings)

        # 获取地图上的出生点（车辆等可以生成的位置点），并取前20个出生点用于后续操作
        spawn_points = self.world.get_map().get_spawn_points()[:20]
        # 从世界的蓝图库中筛选出所有以'vehicle.*'开头的蓝图，这些通常是各种车辆的蓝图
        vehicles = self.world.get_blueprint_library().filter('vehicle.*')
        # 生成一个包含车辆蓝图和对应出生点的元组列表，每个元组表示一个要生成的车辆及其出生位置，
        # 这里通过随机选择车辆蓝图与每个出生点进行组合，用于批量生成车辆
        batch = [(random.choice(vehicles), t) for t in spawn_points]
        # 将上述的元组列表转换为Carla中的SpawnActor命令列表，每个命令用于在对应的出生点生成对应的车辆
        batch = [carla.command.SpawnActor(*args) for args in batch]
        # 以同步方式向服务器客户端发送批量生成车辆的命令，并获取响应，False表示不等待所有命令都成功执行（只要有部分成功就返回响应）
        response = self.client.apply_batch_sync(batch, False)

        # 断言响应中没有任何一个命令出现错误，即所有车辆生成操作都应该是成功的（如果有错误，any(x.error for x in response)会返回True）
        self.assertFalse(any(x.error for x in response))
        # 从响应中提取出所有成功生成的车辆的ID，组成一个列表
        ids = [x.actor_id for x in response]
        # 断言生成的车辆数量与选取的出生点数量相等，即每个出生点都应该成功生成了一辆车
        self.assertEqual(len(ids), len(spawn_points))

        # 让世界进行一次更新（按照之前设置的同步模式和固定时间步长进行更新），获取当前世界的帧编号
        frame = self.world.tick()
        # 获取当前世界的快照，快照包含了世界在某一时刻的状态信息，如各个演员（Actor，这里指车辆等）的位置、状态等
        snapshot = self.world.get_snapshot()
        # 断言当前世界的帧编号与快照的时间戳对应的帧编号相等，即确保快照获取的是当前最新的世界状态
        self.assertEqual(frame, snapshot.timestamp.frame)

        # 获取当前世界中所有的演员（Actor，包括之前生成的车辆等）
        actors = self.world.get_actors()
        # 断言对于世界中的每一个演员，在快照中都能找到对应的记录，即快照包含了所有当前世界中的演员信息
        self.assertTrue(all(snapshot.has_actor(x.id) for x in actors))

        # 遍历生成的车辆ID列表和对应的出生点列表，对每个车辆进行相关的位置和姿态信息的检查
        for actor_id, t0 in zip(ids, spawn_points):
            # 在快照中查找对应ID的车辆的快照信息
            actor_snapshot = snapshot.find(actor_id)
            # 断言能找到该车辆的快照信息，即该车辆在快照中应该有对应的记录
            self.assertIsNotNone(actor_snapshot)
            # 获取车辆在快照中的变换信息（包含位置、旋转等）
            t1 = actor_snapshot.get_transform()
            # 忽略Z轴坐标的比较，因为车辆可能处于下落状态（比如刚生成时可能有物理模拟导致下落等情况），只比较X、Y轴坐标以及旋转角度的Pitch、Yaw、Roll值
            # 使用assertAlmostEqual进行近似相等的比较，保留2位小数，确保车辆在快照中的位置和旋转姿态与初始出生点的相应信息相近，符合预期
            self.assertAlmostEqual(t0.location.x, t1.location.x, places=2)
            self.assertAlmostEqual(t0.location.y, t1.location.y, places=2)
            self.assertAlmostEqual(t0.rotation.pitch, t1.rotation.pitch, places=2)
            self.assertAlmostEqual(t0.rotation.yaw, t1.rotation.yaw, places=2)
            self.assertAlmostEqual(t0.rotation.roll, t1.rotation.roll, places=2)
