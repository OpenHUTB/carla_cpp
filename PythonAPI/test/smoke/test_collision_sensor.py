# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from. import SyncSmokeTest
import carla

class TestCollisionSensor(SyncSmokeTest):
    def wait(self, frames=100):
        """
        这个方法用于让世界（world）进行指定帧数的模拟推进。
        参数:
            frames (int): 要推进的帧数，默认值为100，表示让世界模拟推进这么多帧。
        """
        for _i in range(0, frames):
            self.world.tick()

    def collision_callback(self, event, event_list):
        """
        碰撞事件的回调函数，用于将接收到的碰撞事件添加到给定的事件列表中。
        参数:
            event: 接收到的碰撞事件对象，包含了碰撞相关的各种信息，比如碰撞的参与者等。
            event_list (list): 用于存储碰撞事件的列表，会将新的碰撞事件添加到这个列表里。
        """
        event_list.append(event)

    def run_collision_single_car_against_wall(self, bp_vehicle):
        """
        运行单个汽车与墙碰撞的模拟场景。
        参数:
            bp_vehicle (carla.Blueprint): 汽车的蓝图对象，用于创建具体的汽车实例。
        返回值:
            event_list (list): 存储了在模拟过程中发生的碰撞事件的列表。
        """
        # 创建汽车的初始变换（位置和朝向），位置在坐标(30, -6, 1)处，朝向为yaw=-90（即车头朝某个特定方向）
        veh_transf = carla.Transform(carla.Location(30, -6, 1), carla.Rotation(yaw=-90))
        # 根据给定的汽车蓝图和初始变换在世界中生成汽车实例
        vehicle = self.world.spawn_actor(bp_vehicle, veh_transf)

        # 从世界的蓝图库中查找碰撞传感器的蓝图，这里找的是名为'sensor.other.collision'的碰撞传感器蓝图
        bp_col_sensor = self.world.get_blueprint_library().find('sensor.other.collision')
        # 根据查找到的碰撞传感器蓝图，在默认变换位置（这里暂未指定具体位置偏移，就是默认位置）下生成碰撞传感器实例，并将其附着到前面生成的汽车上
        col_sensor = self.world.spawn_actor(bp_col_sensor, carla.Transform(), attach_to=vehicle)

        event_list = []
        # 为碰撞传感器设置监听回调函数，当检测到碰撞事件时，会调用self.collision_callback方法将事件添加到event_list中
        col_sensor.listen(lambda data: self.collision_callback(data, event_list))

        # 先让世界模拟推进100帧，等待一段时间，可能是给场景一些准备时间之类的操作
        self.wait(100)
        # 设置汽车的目标速度，速度大小为10.0，方向是根据汽车当前的朝向（rotation）获取的向前方向向量来确定
        vehicle.set_target_velocity(10.0 * veh_transf.rotation.get_forward_vector())

        # 再让世界模拟推进100帧，此时汽车以设定速度行驶，可能就会与墙发生碰撞并触发碰撞传感器记录事件
        self.wait(100)

        # 销毁碰撞传感器实例，释放相关资源
        col_sensor.destroy()
        # 销毁汽车实例，释放相关资源
        vehicle.destroy()

        return event_list

    def test_single_car(self):
        """
        测试单个汽车的碰撞传感器功能是否正常的主方法。
        """
        print("TestCollisionSensor.test_single_car")

        # 从世界的蓝图库中过滤出所有以'vehicle.*'开头的汽车蓝图，获取所有可用的汽车蓝图列表
        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")
        # 进一步筛选汽车蓝图，可能是针对老城镇场景等特定需求筛选合适的汽车蓝图（具体筛选逻辑在filter_vehicles_for_old_towns方法中，这里未展示其实现）
        bp_vehicles = self.filter_vehicles_for_old_towns(bp_vehicles)

        cars_failing = ""
        for bp_veh in bp_vehicles:
            # 针对每个筛选后的汽车蓝图，运行单个汽车与墙碰撞的模拟场景，并获取碰撞事件列表
            event_list = self.run_collision_single_car_against_wall(bp_veh)

            if len(event_list) == 0:
                # 如果某个汽车蓝图对应的模拟场景中没有检测到碰撞事件，将该汽车蓝图的ID添加到cars_failing字符串中记录下来
                cars_failing += " %s" % bp_veh.id

        # 检查结果事件，如果有汽车的碰撞传感器未检测到碰撞（即cars_failing字符串不为空），则测试失败并输出相应提示信息
        if cars_failing!= "":
            self.fail("The collision sensor have failed for the cars: %s" % cars_failing)
