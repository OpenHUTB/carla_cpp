# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
# 导入同步冒烟测试相关模块
from . import SyncSmokeTest
# 导入冒烟测试相关模块
from . import SmokeTest
# 导入CARLA库，用于与CARLA模拟器交互
import carla
# 导入时间模块，可用于实现例如等待、计时等功能
import time
# 导入数学模块，用于进行数学运算
import math
# 导入numpy库，用于处理数值计算，例如数组操作等
import numpy as np
#导入枚举类相关模块，用于定义枚举类型
from enum import Enum
# 导入队列模块，用于创建队列来存储数据等操作
from queue import Queue
# 导入Empty异常类，用于处理从队列取数据时队列为空的情况
from queue import Empty
# 定义一个枚举类型SensorType，表示传感器类型，这里有两种类型：LIDAR（激光雷达）和SEMLIDAR（语义激光雷达）
class SensorType(Enum):
    LIDAR = 1
    SEMLIDAR = 2
# 定义Sensor类，用于表示一个传感器
class Sensor():
    def __init__(self, test, sensor_type, attributes, sensor_name = None, sensor_queue = None):
        # 保存传入的测试对象，可能用于后续调用测试相关的功能或者获取相关上下文信息
        self.test = test
        # 获取测试对象关联的世界对象（在CARLA中代表整个模拟世界）
        self.world = test.world
        # 记录传感器的类型，通过传入的SensorType枚举值来指定
        self.sensor_type = sensor_type
        # 用于记录传感器出现的错误信息，如果有错误会在这里保存相关的错误描述
        self.error = None
        # 传感器的名称，如果在创建时传入了自定义名称则使用传入的，否则为None
        self.name = sensor_name
        # 用于存储传感器数据的队列，如果创建时传入了相应队列则使用传入的，否则为None
        self.queue = sensor_queue
        # 当前检测到的点数，初始化为0，可能用于后续统计传感器检测到的数据点数等情况
        self.curr_det_pts = 0
        # 根据传入的传感器类型来获取对应的传感器蓝图（blueprint）
        if self.sensor_type == SensorType.LIDAR:
            # 从世界对象的蓝图库中筛选出激光雷达（ray_cast类型）的蓝图，并取第一个（通常只有一个符合条件的）
            self.bp_sensor = self.world.get_blueprint_library().filter("sensor.lidar.ray_cast")[0]
        elif self.sensor_type == SensorType.SEMLIDAR:
            # 同理，获取语义激光雷达（ray_cast_semantic类型）的蓝图
            self.bp_sensor = self.world.get_blueprint_library().filter("sensor.lidar.ray_cast_semantic")[0]
        else:
            # 如果传入的传感器类型不在定义的枚举类型中，设置错误信息
            self.error = "Unknown type of sensor"
        # 遍历传入的传感器属性字典，为传感器蓝图设置相应的属性
        for key in attributes:
            self.bp_sensor.set_attribute(key, attributes[key])
        # 获取地图中的出生点列表，并取第一个出生点作为传感器的初始位置变换信息
        tranf = self.world.get_map().get_spawn_points()[0]
        # 将传感器的初始位置在Z轴方向上抬高3个单位（可能是为了让传感器处于合适的高度位置来检测）
        tranf.location.z += 3
        # 在世界中根据传感器蓝图和设置好的位置变换信息生成具体的传感器实例（actor）
        self.sensor = self.world.spawn_actor(self.bp_sensor, tranf)
        # 让传感器开始监听数据，当有数据到来时，会调用callback函数进行处理，
        # 并传入传感器数据、传感器名称以及存储数据的队列作为参数
        self.sensor.listen(lambda sensor_data: self.callback(sensor_data, self.name, self.queue))

    def destroy(self):
        self.sensor.destroy()

    def callback(self, sensor_data, sensor_name=None, queue=None):
        # Compute the total sum of points adding all channels
        total_channel_points = 0
        for i in range(0, sensor_data.channels):
            total_channel_points += sensor_data.get_point_count(i)

        # Total points iterating in the LidarMeasurement
        total_detect_points = 0
        for _detection in sensor_data:
            total_detect_points += 1

        # Point cloud used with numpy from the raw data
        if self.sensor_type == SensorType.LIDAR:
            points = np.frombuffer(sensor_data.raw_data, dtype=np.dtype('f4'))
            points = np.reshape(points, (int(points.shape[0] / 4), 4))
            total_np_points = points.shape[0]
            self.curr_det_pts = total_np_points
        elif self.sensor_type == SensorType.SEMLIDAR:
            data = np.frombuffer(sensor_data.raw_data, dtype=np.dtype([
                ('x', np.float32), ('y', np.float32), ('z', np.float32),
                ('CosAngle', np.float32), ('ObjIdx', np.uint32), ('ObjTag', np.uint32)]))
            points = np.array([data['x'], data['y'], data['z']]).T
            total_np_points = points.shape[0]
            self.curr_det_pts = total_np_points
        else:
            self.error = "It should never reach this point"
            return

        if total_np_points != total_detect_points:
            self.error = "The number of points of the raw data does not match with the LidarMeasurament array"

        if total_channel_points != total_detect_points:
            self.error = "The sum of the points of all channels does not match with the LidarMeasurament array"

        # Add option to synchronization queue
        if queue is not None:
            queue.put((sensor_data.frame, sensor_name, self.curr_det_pts))

    def is_correct(self):
        return self.error is None

    def get_current_detection_points():
        return self.curr_det_pts

class TestSyncLidar(SyncSmokeTest):
    def test_lidar_point_count(self):
        print("TestSyncLidar.test_lidar_point_count")
        sensors = []

        att_l00={'channels' : '64', 'dropoff_intensity_limit': '0.0', 'dropoff_general_rate': '0.0',
          'range' : '50', 'points_per_second': '100000', 'rotation_frequency': '20'}
        att_l01={'channels' : '64', 'range' : '200', 'points_per_second': '500000',
          'rotation_frequency': '5'}
        att_l02={'channels' : '64', 'dropoff_intensity_limit': '1.0', 'dropoff_general_rate': '0.0',
          'range' : '50', 'points_per_second': '100000', 'rotation_frequency': '50'}

        sensors.append(Sensor(self, SensorType.LIDAR, att_l00))
        sensors.append(Sensor(self, SensorType.LIDAR, att_l01))
        sensors.append(Sensor(self, SensorType.LIDAR, att_l02))

        for _ in range(0, 10):
            self.world.tick()
        time.sleep(0.5)

        for sensor in sensors:
            sensor.destroy()

        for sensor in sensors:
            if not sensor.is_correct():
                self.fail(sensor.error)


    def test_semlidar_point_count(self):
        print("TestSyncLidar.test_semlidar_point_count")
        sensors = []

        att_s00 = {'channels' : '64', 'range' : '100', 'points_per_second': '100000',
          'rotation_frequency': '20'}
        att_s01 = {'channels' : '32', 'range' : '200', 'points_per_second': '500000',
          'rotation_frequency': '50'}

        sensors.append(Sensor(self, SensorType.SEMLIDAR, att_s00))
        sensors.append(Sensor(self, SensorType.SEMLIDAR, att_s01))

        for _ in range(0, 10):
            self.world.tick()
        time.sleep(0.5)

        for sensor in sensors:
            sensor.destroy()

        for sensor in sensors:
            if not sensor.is_correct():
                self.fail(sensor.error)


class TestASyncLidar(SmokeTest):
    def test_lidar_point_count(self):
        print("TestASyncLidar.test_lidar_point_count")
        sensors = []

        att_l00={'channels' : '64', 'dropoff_intensity_limit': '0.0', 'dropoff_general_rate': '0.0',
          'range' : '50', 'points_per_second': '100000', 'rotation_frequency': '20'}
        att_l01={'channels' : '64', 'range' : '200', 'points_per_second': '500000',
          'rotation_frequency': '5'}
        att_l02={'channels' : '64', 'dropoff_intensity_limit': '1.0', 'dropoff_general_rate': '0.0',
          'range' : '50', 'points_per_second': '100000', 'rotation_frequency': '50'}

        sensors.append(Sensor(self, SensorType.LIDAR, att_l00))
        sensors.append(Sensor(self, SensorType.LIDAR, att_l01))
        sensors.append(Sensor(self, SensorType.LIDAR, att_l02))

        time.sleep(3.0)

        for sensor in sensors:
            sensor.destroy()

        for sensor in sensors:
            if not sensor.is_correct():
                self.fail(sensor.error)


    def test_semlidar_point_count(self):
        print("TestASyncLidar.test_semlidar_point_count")
        sensors = []

        att_s00 = {'channels' : '64', 'range' : '100', 'points_per_second': '100000',
          'rotation_frequency': '20'}
        att_s01 = {'channels' : '32', 'range' : '200', 'points_per_second': '500000',
          'rotation_frequency': '50'}

        sensors.append(Sensor(self, SensorType.SEMLIDAR, att_s00))
        sensors.append(Sensor(self, SensorType.SEMLIDAR, att_s01))

        time.sleep(3.0)

        for sensor in sensors:
            sensor.destroy()

        for sensor in sensors:
            if not sensor.is_correct():
                self.fail(sensor.error)

class TestCompareLidars(SyncSmokeTest):
    def test_lidar_comparison(self):
        print("TestCompareLidars.test_lidar_comparison")
        sensors = []

        att_sem_lidar={'channels' : '64', 'range' : '200', 'points_per_second': '500000'}
        att_lidar_nod={'channels' : '64', 'dropoff_intensity_limit': '0.0', 'dropoff_general_rate': '0.0',
          'range' : '200', 'points_per_second': '500000'}
        att_lidar_def={'channels' : '64', 'range' : '200', 'points_per_second': '500000'}

        sensor_queue = Queue()
        sensors.append(Sensor(self, SensorType.SEMLIDAR, att_sem_lidar, "SemLidar", sensor_queue))
        sensors.append(Sensor(self, SensorType.LIDAR, att_lidar_nod, "LidarNoD", sensor_queue))
        sensors.append(Sensor(self, SensorType.LIDAR, att_lidar_def, "LidarDef", sensor_queue))

        for _ in range(0, 15):
            self.world.tick()

            data_sem_lidar = None
            data_lidar_nod = None
            data_lidar_def = None
            for _ in range(len(sensors)):
                data = sensor_queue.get(True, 10.0)
                if data[1] == "SemLidar":
                    data_sem_lidar = data
                elif data[1] == "LidarNoD":
                    data_lidar_nod = data
                elif data[1] == "LidarDef":
                    data_lidar_def = data
                else:
                    self.fail("It should never reach this point")

            # Check that frame number are correct
            self.assertEqual(data_sem_lidar[0], data_lidar_nod[0], "The frame numbers of LiDAR and SemLiDAR do not match.")
            self.assertEqual(data_sem_lidar[0], data_lidar_def[0], "The frame numbers of LiDAR and SemLiDAR do not match.")

            # The detections of the semantic lidar and the Lidar with no dropoff should have the same point count always
            self.assertEqual(data_sem_lidar[2], data_lidar_nod[2], "The point count of the detections of this frame of LiDAR(No dropoff) and SemLiDAR do not match.")

            # Default lidar should drop a minimum of 45% of the points so we check that but with a high tolerance to account for 'rare' cases
            if data_lidar_def[2] > 0.75 * data_sem_lidar[2]:
                self.fail("The point count of the default lidar should be much less than the Semantic Lidar point count.")

        time.sleep(1)
        for sensor in sensors:
            sensor.destroy()
