# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from. import SyncSmokeTest

import carla
import time
import math

# 定义传感器类，用于管理单个传感器的相关操作
class Sensor():
    def __init__(self, world, bp_sensor, sensor_tick):
        # 保存传感器蓝图（blueprint）对象
        self.bp_sensor = bp_sensor
        # 设置传感器的触发时间间隔属性（将其转换为字符串类型进行设置）
        bp_sensor.set_attribute("sensor_tick", str(sensor_tick))
        # 在世界中根据蓝图生成具体的传感器实例，并设置其初始变换为默认（carla.Transform()）
        self.sensor = world.spawn_actor(bp_sensor, carla.Transform())
        # 为传感器注册监听回调函数，当传感器有数据产生时会调用self.listen方法
        self.sensor.listen(lambda sensor_data: self.listen(sensor_data))
        # 用于记录传感器已经触发的次数，初始化为0
        self.num_ticks = 0

    def destroy(self):
        # 销毁该传感器实例，释放相关资源
        self.sensor.destroy()

    def listen(self, sensor_data):
        # 每次传感器有数据传入时，触发次数加1
        self.num_ticks += 1


# 测试传感器触发时间间隔的测试类，继承自SyncSmokeTest（可能是自定义的测试基类）
class TestSensorTickTime(SyncSmokeTest):
    def test_sensor_tick_time(self):
        print("TestSensorTickTime.test_sensor_tick_time")

        # 获取世界中的蓝图库，用于查找各种传感器蓝图
        bp_lib = self.world.get_blueprint_library()

        # 定义一组不需要进行测试的传感器id集合，这些传感器可能有特殊情况或者不符合当前测试需求
        sensor_exception = {
            "sensor.camera.depth",
            "sensor.camera.normals",
            "sensor.camera.optical_flow",
            "sensor.camera.rgb",
            "sensor.camera.semantic_segmentation",
            "sensor.camera.dvs",
            "sensor.other.obstacle",
            "sensor.camera.instance_segmentation",
            "sensor.other.v2x",
            "sensor.other.v2x_custom"
        }

        # 用于存储已经生成的传感器实例的列表
        spawned_sensors = []
        # 设置传感器的触发时间间隔，单位应该是秒（这里设置为1.0秒）
        sensor_tick = 1.0

        # 遍历蓝图库中所有以 "sensor.*" 开头的传感器蓝图
        for bp_sensor in bp_lib.filter("sensor.*"):
            # 如果传感器蓝图的id在不需要测试的集合中，则跳过该传感器
            if bp_sensor.id in sensor_exception:
                continue
            # 如果传感器蓝图有 "sensor_tick" 属性（意味着可以设置触发时间间隔属性），则生成该传感器实例并添加到已生成传感器列表中
            if bp_sensor.has_attribute("sensor_tick"):
                spawned_sensors.append(Sensor(self.world, bp_sensor, sensor_tick))

        # 设置世界模拟的步数，这里设置为50步
        num_ticks = 50
        # 循环让世界进行指定次数的模拟更新
        for _ in range(0, num_ticks):
            self.world.tick()
        # 暂停1秒，可能是为了等待一些异步操作完成或者确保数据稳定等（具体取决于应用场景）
        time.sleep(1.0)

        # 获取世界设置中的固定时间步长（每一步模拟的时间间隔），单位为秒
        dt = self.world.get_settings().fixed_delta_seconds
        # 计算模拟的总时间，等于模拟步数乘以时间步长
        total_time = num_ticks * dt
        # 根据总时间和传感器触发时间间隔，计算传感器理论上应该触发的次数（向上取整）
        num_sensor_ticks = int(math.ceil(total_time / sensor_tick))

        # 遍历所有已经生成的传感器实例，进行测试断言
        for sensor in spawned_sensors:
            # 断言传感器实际触发的次数是否与理论上应该触发的次数相等，如果不相等则输出相应的错误提示信息
            self.assertEqual(sensor.num_ticks, num_sensor_ticks,
                             "\n\n {} does not match tick count".format(sensor.bp_sensor.id))
            # 销毁当前传感器实例，释放资源
            sensor.destroy()
