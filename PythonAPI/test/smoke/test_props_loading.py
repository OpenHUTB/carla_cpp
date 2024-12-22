# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
import carla
import random
from. import SmokeTest
# 定义一个名为 TestPropsLoading 的类，它继承自 SmokeTest 类，
# 此类可能是用于进行某种冒烟测试（SmokeTest），专门针对道具加载相关功能进行测试
class TestPropsLoading(SmokeTest):
    def test_spawn_loaded_props(self):
        """
        这个方法是 TestPropsLoading 类中的一个测试方法，用于测试生成已加载的道具（props）功能是否正常。
        它的大致逻辑是从世界（world）中获取道具蓝图（blueprint），选择生成点，然后批量生成道具，并对生成结果进行验证。
        """
        print("TestPropsLoading.test_spawn_loaded_props")

        # 获取当前测试用例关联的客户端（client）对象，
        # 这个 client 对象可能用于和游戏世界等进行交互通信，比如发送指令、获取信息等
        client = self.client
        # 通过客户端对象获取对应的游戏世界（world）对象，
        # 游戏世界对象包含了地图、角色、道具等众多游戏元素相关的操作方法和属性
        world = client.get_world()

        # 获取用于生成角色（actor）的命令构造函数 SpawnActor，
        # 后续可以使用它来创建要在游戏世界中生成的道具等角色实例
        SpawnActor = carla.command.SpawnActor

        # 从游戏世界的蓝图库（blueprint_library）中筛选出名称匹配 "static.prop.*" 模式的道具蓝图列表，
        # 也就是获取所有符合该命名规则的道具蓝图，这些蓝图可用于后续生成具体的道具实例
        props = world.get_blueprint_library().filter("static.prop.*")
        # 初始化一个空列表，用于存储已经成功生成的道具的 actor_id，
        # actor_id 可以唯一标识游戏世界中的每个角色（包括道具），方便后续对生成的道具进行管理和验证
        spawned_props = []

        # 获取游戏世界地图（map）中的所有生成点（spawn_points）列表，
        # 生成点就是可以在游戏世界中生成角色（如道具、车辆、行人等）的位置坐标信息集合
        spawn_points = world.get_map().get_spawn_points()

        # 初始化一个高度变量 z，用于在后续为每个道具设置不同的垂直高度（z轴坐标），初始值设为 0
        z = 0
        # 初始化一个空列表，用于存储要批量执行的生成道具的命令，
        # 后续会将多个生成道具的指令添加到这个列表中，然后一次性发送给客户端去执行
        batch = []
        for prop in props:
            # 从所有的生成点中随机选择一个作为当前道具的生成位置，
            # 这样每个道具都会有不同的生成位置，模拟不同场景下道具的生成情况
            spawn_point = random.choice(spawn_points)
            # 将选择的生成点的垂直高度（z轴坐标）增加 z 的值，
            # 通过每次增加一定的高度值（这里每次增加 100），可以让生成的道具在不同的高度上，避免重叠等情况
            spawn_point.location.z += z

            # 使用之前获取的 SpawnActor 构造函数创建一个生成当前道具的命令，
            # 将道具蓝图（prop）和调整后的生成点（spawn_point）作为参数传入，然后添加到批量命令列表（batch）中
            batch.append(SpawnActor(prop, spawn_point))
            # 每次循环增加 z 的值，为下一个道具设置不同的高度
            z += 100

        # 向客户端发送批量生成道具的命令，并等待所有命令执行完成，获取每个命令的响应结果列表，
        # 这里的 client.apply_batch_sync 方法会同步地执行所有的生成命令，并返回对应的响应信息
        for response in client.apply_batch_sync(batch):
            # 对每个响应进行验证，检查是否有错误发生，
            # 如果响应的 error 属性为 False，表示生成该道具没有出现错误，即生成成功
            self.assertFalse(response.error)
            # 如果生成成功，将该道具的 actor_id 添加到已生成道具列表（spawned_props）中，方便后续统计和验证
            spawned_props.append(response.actor_id)

        # 最后验证生成的道具数量是否和预期的道具蓝图数量一致，
        # 通过比较已生成道具列表（spawned_props）的长度和获取到的道具蓝图（props）的长度来判断，
        # 如果长度相等，说明所有的道具都成功生成了，符合预期；否则表示生成过程可能出现了遗漏等问题
        self.assertEqual(len(spawned_props), len(props))