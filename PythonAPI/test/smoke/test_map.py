# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla
import random
from. import SmokeTest
import time


# 定义 TestMap 类，它继承自 SmokeTest 类，此类可能是用于进行冒烟测试（SmokeTest），专门针对地图（Map）相关功能进行测试
class TestMap(SmokeTest):
    def test_reload_world(self):
        """
        这个方法用于测试重新加载世界（地图）的功能，主要验证重新加载后地图的名称是否保持不变。
        """
        print("TestMap.test_reload_world")
        # 获取当前客户端（client）对应的世界（world）中地图（map）的名称，作为原始地图名称保存下来
        map_name = self.client.get_world().get_map().name
        # 通过客户端重新加载世界（地图），得到重新加载后的世界对象
        world = self.client.reload_world()
        # 验证重新加载后的世界中的地图名称与原始地图名称是否一致，
        # 如果一致，说明重新加载地图功能在地图名称方面没有出现异常，符合预期
        self.assertEqual(map_name, world.get_map().name)

    def test_load_all_maps(self):
        """
        该方法用于测试加载所有可用地图的功能，会遍历所有可用地图（除了特定排除的地图），
        加载每个地图后进行一系列验证操作，包括地图名称验证以及调用私有方法 _check_map 对地图做更详细的检查。
        """
        print("TestMap.test_load_all_maps")
        # 获取客户端可用的所有地图名称列表，每个名称对应一个游戏中的地图场景
        map_names = list(self.client.get_available_maps())
        # 对地图名称列表进行随机打乱，使得每次加载地图的顺序不同，
        # 这样可以更全面地测试地图加载功能在不同顺序下的表现（避免顺序依赖导致的潜在问题未被发现）
        random.shuffle(map_names)
        for map_name in map_names:
            # 暂时忽略特定的地图（BaseMap、Town11、Town12），对其他地图进行测试操作，
            # 原因可能是这些特定地图不需要进行当前测试或者有其他特殊处理逻辑
            if map_name!= '/Game/Carla/Maps/BaseMap/BaseMap' and map_name!= '/Game/Carla/Maps/Town11/Town11' and map_name!= '/Game/Carla/Maps/Town12/Town12':
                # 通过客户端加载指定名称的地图，得到对应的世界（world）对象，
                # 后续基于这个世界对象可以对加载后的地图进行相关操作和验证
                world = self.client.load_world(map_name)
                # 由于加载地图后虚幻引擎 4（UE4）可能需要时间清理旧的资源占用的内存，所以这里暂停 5 秒钟，
                # 这是一种临时的解决办法（workaround），确保后续操作能正常进行
                time.sleep(5)
                # 获取加载后的世界中的地图对象，用于后续验证等操作
                m = world.get_map()
                # 验证加载后的地图名称（去除路径部分，只取最后的地图名称部分）是否和传入的地图名称（同样去除路径部分）一致，
                # 以此来确保正确加载了期望的地图
                self.assertEqual(map_name.split('/')[-1], m.name.split('/')[-1])
                # 调用私有方法 _check_map 对当前加载的地图进行更详细的检查，
                # 例如检查生成点、拓扑结构、路径点等相关信息是否符合预期
                self._check_map(m)

    def _check_map(self, m):
        """
        这个私有方法用于对传入的地图对象进行详细的检查，包括检查生成点、拓扑结构、路径点等相关信息是否满足预期条件。

        参数：
        m：要进行详细检查的地图对象。
        """
        for spawn_point in m.get_spawn_points():
            # 对于地图中的每个生成点（spawn_point），获取对应的路径点（waypoint），
            # 这里设置不将位置投影到道路上（project_to_road=False），获取的是原始位置对应的路径点信息，
            # 并验证获取到的路径点不为空（即存在有效的路径点信息）
            waypoint = m.get_waypoint(spawn_point.location, project_to_road=False)
            self.assertIsNotNone(waypoint)
        # 获取地图的拓扑结构（topology）信息，拓扑结构包含了地图中道路等元素的连接关系等重要信息，
        # 验证获取到的拓扑结构列表长度大于 0，即确保地图存在有效的拓扑结构
        topology = m.get_topology()
        self.assertGreater(len(topology), 0)
        # 在地图上生成间距为 2 的路径点列表，这里生成的路径点可以用于表示道路上的关键位置等信息，
        # 验证生成的路径点列表长度大于 0，确保能成功生成有效的路径点
        waypoints = list(m.generate_waypoints(2))
        self.assertGreater(len(waypoints), 0)
        # 对生成的路径点列表进行随机打乱，使得后续遍历顺序随机化，更全面地测试路径点相关功能
        random.shuffle(waypoints)
        for waypoint in waypoints[:200]:
            for _ in range(0, 20):
                # 验证路径点的车道宽度（lane_width）大于等于 0.0，确保车道宽度信息的合理性
                self.assertGreaterEqual(waypoint.lane_width, 0.0)
                # 获取路径点右侧的车道信息（虽然这里获取后没有进一步使用，但可能是用于检查右侧车道是否存在等相关验证）
                _ = waypoint.get_right_lane()
                # 获取路径点左侧的车道信息（同理，可能用于检查左侧车道相关情况）
                _ = waypoint.get_left_lane()
                # 获取当前路径点往后的 4 个相邻路径点列表，用于后续在路径上移动遍历等操作（比如模拟车辆沿着道路行驶的路径查找）
                next_waypoints = waypoint.next(4)
                if not next_waypoints:
                    break
                # 从相邻路径点列表中随机选择一个作为下一个要检查的路径点，继续循环进行相关验证
                waypoint = random.choice(next_waypoints)
        # 将一个默认的位置（这里是 carla.Location()，坐标默认初始值）转换为地理坐标（geolocation），
        # 虽然这里获取转换后的结果没有进一步使用，但可能是用于验证地图坐标转换相关功能是否正常
        _ = m.transform_to_geolocation(carla.Location())
        # 验证将地图转换为 OpenDRIVE 格式的字符串表示是否为真值（即是否能正确转换并返回有效的字符串内容），
        # 这可以用于检查地图是否能正确导出为特定的标准格式，便于和其他支持 OpenDRIVE 格式的系统交互等
        self.assertTrue(str(m.to_opendrive()))
