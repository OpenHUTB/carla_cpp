# 版权声明相关信息
# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from. import SmokeTest

import carla
import time
import numpy as np
import filecmp
import shutil
import os

# 根据Python版本导入相应的队列模块（Python 3和Python 2的导入方式有所不同）
try:
    # python 3
    from queue import Queue as Queue
    from queue import Empty
except ImportError:
    # python 2
    from Queue import Queue as Queue
    from Queue import Empty

# 自定义异常类，用于表示确定性相关的错误
class DeterminismError(Exception):
    pass

# 表示一个模拟场景的基类，包含场景相关的各种操作和属性
class Scenario(object):
    def __init__(self, client, world, save_snapshots_mode=False):
        # CARLA世界对象，用于操作场景中的各种元素（车辆、传感器等）
        self.world = world
        # CARLA客户端对象，用于与服务器通信等操作
        self.client = client
        # 存储场景中的演员（如车辆等）列表，每个元素是一个包含名称和演员对象的元组
        self.actor_list = []
        # 记录场景初始化时的时间戳相关信息（帧编号和经过的时间）
        self.init_timestamp = []
        # 表示场景是否处于活动状态
        self.active = False
        # 用于生成文件名等的前缀字符串
        self.prefix = ""
        # 是否保存快照的模式标志
        self.save_snapshots_mode = save_snapshots_mode
        # 存储每个演员的快照数据（位置、速度等信息的数组）
        self.snapshots = []
        # 存储场景中的传感器列表，每个元素是一个包含名称和传感器对象的元组
        self.sensor_list = []
        # 用于传感器数据同步的队列，存储传感器获取的数据帧等信息
        self.sensor_queue = Queue()

    # 初始化场景的方法，设置前缀、清空相关列表、重新加载世界等操作
    def init_scene(self, prefix, settings=None, spectator_tr=None):
        self.prefix = prefix
        self.actor_list = []
        self.active = True
        self.snapshots = []
        self.sensor_list = []
        self.sensor_queue = Queue()

        self.reload_world(settings, spectator_tr)
        # 工作区解决办法：给UE4一些时间清理加载旧资产后的内存
        time.sleep(5)

        # 获取当前世界的快照信息，初始化时间戳相关信息（帧编号和经过的时间）
        snapshot = self.world.get_snapshot()
        self.init_timestamp = {'frame0': snapshot.frame, 'time0': snapshot.timestamp.elapsed_seconds}

    # 向场景中添加演员（如车辆）的方法，为演员设置名称并添加到演员列表中
    def add_actor(self, actor, actor_name="Actor"):
        actor_idx = len(self.actor_list)

        name = str(actor_idx) + "_" + actor_name

        self.actor_list.append((name, actor))

        if self.save_snapshots_mode:
            self.snapshots.append(np.empty((0, 11), float))

    # 等待一定帧数的方法，在每一帧进行世界的更新以及传感器数据的获取（如果场景处于活动状态）
    def wait(self, frames=100):
        for _i in range(0, frames):
            self.world.tick()
            if self.active:
                for _s in self.sensor_list:
                    self.sensor_queue.get(True, 15.0)

    # 清理场景的方法，销毁场景中的所有传感器和演员，并将场景设置为非活动状态
    def clear_scene(self):
        for sensor in self.sensor_list:
            sensor[1].destroy()

        for actor in self.actor_list:
            actor[1].destroy()

        self.active = False

    # 重新加载世界的方法，应用给定的设置并重置观察者位置（如果有相应参数传入）
    def reload_world(self, settings=None, spectator_tr=None):
        if settings is not None:
            self.world.apply_settings(settings)
        if spectator_tr is not None:
            self.reset_spectator(spectator_tr)

        self.client.reload_world(False)
        # 工作区解决办法：给UE4一些时间清理加载旧资产后的内存
        time.sleep(5)

    # 重置观察者位置的方法，根据给定的变换信息设置观察者的位置和朝向
    def reset_spectator(self, spectator_tr):
        spectator = self.world.get_spectator()
        spectator.set_transform(spectator_tr)

    # 获取单个演员的当前快照数据的方法，包括帧编号差值、时间差值以及位置、速度、角速度等信息
    def save_snapshot(self, actor):
        snapshot = self.world.get_snapshot()

        actor_snapshot = np.array([
            float(snapshot.frame - self.init_timestamp['frame0']), \
            snapshot.timestamp.elapsed_seconds - self.init_timestamp['time0'], \
            actor.get_location().x, actor.get_location().y, actor.get_location().z, \
            actor.get_velocity().x, actor.get_velocity().y, actor.get_velocity().z, \
            actor.get_angular_velocity().x, actor.get_angular_velocity().y,
            actor.get_angular_velocity().z])
        return actor_snapshot

    # 保存所有演员的快照数据的方法，将当前时刻的快照数据添加到相应演员的快照数据数组中（如果处于保存快照模式）
    def save_snapshots(self):
        if not self.save_snapshots_mode:
            return

        for i in range(0, len(self.actor_list)):
            self.snapshots[i] = np.vstack((self.snapshots[i], self.save_snapshot(self.actor_list[i][1])))

    # 将所有演员的快照数据保存到磁盘文件的方法，每个演员的数据保存为一个单独的文件（如果处于保存快照模式）
    def save_snapshots_to_disk(self):
        if not self.save_snapshots_mode:
            return

        for i, actor in enumerate(self.actor_list):
            np.savetxt(self.get_filename(actor[0]), self.snapshots[i])

    # 根据前缀、演员ID和帧编号生成带前缀的文件名的方法，用于构建保存数据的文件名
    def get_filename_with_prefix(self, prefix, actor_id=None, frame=None):
        add_id = "" if actor_id is None else "_" + actor_id
        add_frame = "" if frame is None else ("_%04d") % frame
        return prefix + add_id + add_frame + ".out"

    # 根据演员ID和帧编号生成文件名的方法（使用默认前缀），调用get_filename_with_prefix方法实现
    def get_filename(self, actor_id=None, frame=None):
        return self.get_filename_with_prefix(self.prefix, actor_id, frame)

    # 运行模拟的方法，初始化场景、进行指定帧数的模拟循环（更新世界、同步传感器、保存快照），最后恢复原始设置并清理场景
    def run_simulation(self, prefix, run_settings, spectator_tr, tics=200):
        original_settings = self.world.get_settings()

        self.init_scene(prefix, run_settings, spectator_tr)

        for _i in range(0, tics):
            self.world.tick()
            self.sensor_syncronization()
            self.save_snapshots()

        self.world.apply_settings(original_settings)
        self.save_snapshots_to_disk()
        self.clear_scene()

    # 向场景中添加传感器的方法，根据传感器类型设置不同的监听回调函数，用于处理传感器获取的数据
    def add_sensor(self, sensor, sensor_type):
        sen_idx = len(self.sensor_list)
        if sensor_type == "LiDAR":
            name = str(sen_idx) + "_LiDAR"
            sensor.listen(lambda data: self.add_lidar_snapshot(data, name))
        elif sensor_type == "SemLiDAR":
            name = str(sen_idx) + "_SemLiDAR"
            sensor.listen(lambda data: self.add_semlidar_snapshot(data, name))
        elif sensor_type == "Radar":
            name = str(sen_idx) + "_Radar"
            sensor.listen(lambda data: self.add_radar_snapshot(data, name))

        self.sensor_list.append((name, sensor))

    # 处理LiDAR传感器数据的方法，将数据保存为文件并将相关信息放入传感器队列（如果场景处于活动状态）
    def add_lidar_snapshot(self, lidar_data, name="LiDAR"):
        if not self.active:
            return

        points = np.frombuffer(lidar_data.raw_data, dtype=np.dtype('f4'))
        points = np.reshape(points, (int(points.shape[0] / 4), 4))

        frame = lidar_data.frame - self.init_timestamp['frame0']
        np.savetxt(self.get_filename(name, frame), points)
        self.sensor_queue.put((lidar_data.frame, name))

    # 处理SemLiDAR传感器数据的方法，解析数据、保存为文件并将相关信息放入传感器队列（如果场景处于活动状态）
    def add_semlidar_snapshot(self, lidar_data, name="SemLiDAR"):
        if not self.active:
            return

        data = np.frombuffer(lidar_data.raw_data, dtype=np.dtype([
            ('x', np.float32), ('y', np.float32), ('z', np.float32),
            ('CosAngle', np.float32), ('ObjIdx', np.uint32), ('ObjTag', np.uint32)]))
        points = np.array([data['x'], data['y'], data['z'], data['CosAngle'], data['ObjTag']]).T

        frame = lidar_data.frame - self.init_timestamp['frame0']
        np.savetxt(self.get_filename(name, frame), points)
        self.sensor_queue.put((lidar_data.frame, name))

    # 处理Radar传感器数据的方法，解析数据、保存为文件并将相关信息放入传感器队列（如果场景处于活动状态）
    def add_radar_snapshot(self, radar_data, name="Radar"):
        if not self.active:
            return

        points = np.frombuffer(radar_data.raw_data, dtype=np.dtype('f4'))
        points = np.reshape(points, (int(points.shape[0] / 4), 4))

        frame = radar_data.frame - self.init_timestamp['frame0']
        np.savetxt(self.get_filename(name, frame), points)
        self.sensor_queue.put((radar_data.frame, name))

    # 传感器同步的方法，确保传感器获取的数据帧与世界的当前帧匹配，否则抛出确定性错误
    def sensor_syncronization(self):
        # Sensor Syncronization
        w_frame = self.world.get_snapshot().frame
        for sensor in self.sensor_list:
            s_frame = self.sensor_queue.get(True, 15.0)[0]

            while s_frame < w_frame:
                s_frame = self.sensor_queue.get(True, 15.0)[0]

            if w_frame!= s_frame:
                raise DeterminismError("FrameSyncError: Frames are not equal for sensor %s: %d %d"
                                       % (sensor[0], w_frame, s_frame))


# 继承自Scenario类，用于特定场景下（包含所有射线投射传感器）的初始化操作
class SpawnAllRaycastSensors(Scenario):
    def init_scene(self, prefix, settings=None, spectator_tr=None):
        super(SpawnAllRaycastSensors, self).init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        # 生成第一辆车的位置和朝向信息，并在世界中生成车辆，设置目标速度
        vehicle00_tr = carla.Transform(carla.Location(140, -205, 0.1), carla.Rotation(yaw=181.5))
        vehicle00 = self.world.spawn_actor(blueprint_library.filter("tt")[0], vehicle00_tr)
        vehicle00.set_target_velocity(carla.Vector3D(-25, 0, 0))

        # 生成第二辆车的位置和朝向信息，并在世界中生成车辆，设置目标速度
        vehicle01_tr = carla.Transform(carla.Location(50, -200, 0.1), carla.Rotation(yaw=1.5))
        vehicle01 = self.world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle01_tr)
        vehicle01.set_target_velocity(carla.Vector3D(25, 0, 0))

        # 获取雷达传感器的蓝图，设置噪声种子，生成雷达传感器并附着到第一辆车
        radar_bp = self.world.get_blueprint_library().find('sensor.other.radar')
        radar_bp.set_attribute('noise_seed', '54283')
        radar_tr = carla.Transform(carla.Location(z=2))
        radar = self.world.spawn_actor(radar_bp, radar_tr, attach_to=vehicle00)

        # 获取普通LiDAR传感器的蓝图，设置噪声种子，生成LiDAR传感器并附着到第一辆车
        lidar01_bp = self.world.get_blueprint_library().find('sensor.lidar.ray_cast')
        lidar01_bp.set_attribute('noise_seed', '12134')
        lidar01_tr = carla.Transform(carla.Location(x=1, z=2))
        lidar01 = self.world.spawn_actor(lidar01_bp, lidar01_tr, attach_to=vehicle00)

        # 获取语义LiDAR传感器的蓝图，生成语义LiDAR传感器并附着到第二辆车
        lidar02_bp = self.world.get_blueprint_library().find('sensor.lidar.ray_cast_semantic')
        lidar02_tr = carla.Transform(carla.Location(x=1, z=2))
        lidar02 = self.world.spawn_actor(lidar02_bp, lidar02_tr, attach_to=vehicle01)

        # 获取普通LiDAR传感器的蓝图，设置噪声种子，生成LiDAR传感器并附着到第二辆车
        lidar03_bp = self.world.get_blueprint_library().find('sensor.lidar.ray_cast')
        lidar03_bp.set_attribute('noise_seed', '23135')
        lidar03_tr = carla.Transform(carla.Location(z=2))
        lidar03 = self.world.spawn_actor(lidar03_bp, lidar03_tr, attach_to=vehicle01)

        # 将各个传感器添加到传感器列表中，并添加相应的车辆到演员列表中
        self.add_sensor(radar, "Radar")
        self.add_sensor(lidar01, "LiDAR")
        self.add_sensor(lidar02, "SemLiDAR")
        self.add_sensor(lidar03, "LiDAR")
        self.add_actor(vehicle00, "Car")
        self.add_actor(vehicle01, "Car")

        self.wait(1)


# 用于测试传感器场景确定性的类，包含比较文件、检查模拟等相关方法
class SensorScenarioTester():
    def __init__(self, scene, output_path):
        self.scene = scene
        self.world = self.scene.world
        self.client = self.scene.client
        self.scenario_name = self.scene.__class__.__name__
        self.output_path = output_path

    # 比较两个文件是否相等或者在一定误差范围内相等的方法，用于判断传感器输出是否一致
    def compare_files(self, file_i, file_j):
        # 首先直接比较文件是否完全相等，如果相等则模拟结果等效
        check_ij = filecmp.cmp(file_i, file_j)
        if check_ij:
            return True

        # 如果文件不相等，检查数据点数量是否不同，如果不同则模拟结果不等效
        data_i = np.loadtxt(file_i)
        data_j = np.loadtxt(file_j)
        if data_i.shape!= data_j.shape:
            return False

        # 如果数据点数量相同但有小差异，可能是由于浮点数运算误差导致，判断最大误差是否在允许范围内
        max_error = np.amax(np.abs(data_i - data_j))

        return max_error < 0.01

    # 检查多次模拟结果是否具有确定性的方法，通过比较不同重复模拟的传感器输出文件来判断
    def check_simulations(self, rep_prefixes, sim_tics):
        repetitions = len(rep_prefixes)
        mat_check = np.zeros((repetitions, repetitions), int)

        for i in range(0, repetitions):
            mat_check[i][i] = 1
            for j in range(0, i):
                sim_check = True
                for f_idx in range(1, sim_tics):
                    for sensor in self.scene.sensor_list:
                        file_i = self.scene.get_filename_with_prefix(rep_prefixes[i], sensor[0], f_idx)
                        file_j = self.scene.get_filename_with_prefix(rep_prefixes[j], sensor[0], f_idx)

                        check_ij = self.compare_files(file_i, file_j)
                        sim_check = sim_check and check_ij
                mat_check[i][j] = int(sim_check)
                mat_check[j][i] = int(sim_check)

        determinism = np.sum(mat_check, axis=1)

        determinism_set = list(set(determinism))
        determinism_set.sort(reverse=True)

        return determinism_set

    # 测试场景的方法，进行多次模拟并检查模拟结果的确定性，根据结果判断场景是否满足确定性要求
    def test_scenario(self, repetitions=1, sim_tics=100):
        prefix = self.output_path + self.scenario_name

        config_settings = self.world.get_settings()
        config_settings.synchronous_mode = True
        config_settings.fixed_delta_seconds = 0.05

        spectator_tr = carla.Transform(carla.Location(160, -205, 10), carla.Rotation(yaw=180))

        sim_prefixes = []
        for i in range(0, repetitions):
            prefix_rep = prefix + "_rep_" + ("%03d" % i)
            self.scene.run_simulation(prefix_rep, config_settings, spectator_tr, tics=sim_tics)
            sim_prefixes.append(prefix_rep)

        determ_repet = self.check_simulations(sim_prefixes, sim_tics)

        if determ_repet[0]!= repetitions:
            raise DeterminismError("SensorOutputError: Scenario %s is not deterministic: %d / %d" % (self.scenario_name, determ_repet[0], repetitions))


# 继承自SmokeTest类，用于具体执行传感器确定性测试的类
class TestSensorDeterminism(SmokeTest):
    def test_all_sensors(self):
        print("TestSensorDeterminism.test_all_sensors")

        orig_settings = self.world.get_settings()

        # 设置输出临时文件夹路径，获取当前文件所在目录并添加子文件夹名称
        output_path = os.path.dirname(os.path.realpath(__file__))
        output_path = os.path.join(output_path, "_sensors") + os.path.sep
        if not os.path.exists(output_path):
            os.mkdir(output_path)

        try:
            # 创建SensorScenarioTester实例，传入特定场景和输出路径，进行测试
            test_sensors = SensorScenarioTester(SpawnAllRaycastSensors(self.client, self.world), output_path)
            test_sensors.test_scenario(repetitions=5, sim_tics=100)
        except DeterminismError as err:
            # 如果出现确定性错误，清理场景并删除输出文件夹，然后使测试用例失败
            test_sensors.scene.clear_scene()
            # Remove all the output files
            shutil.rmtree(output_path)
            self.fail(err)

        self.world.apply_settings(orig_settings)

        # Remove all the output files
        shutil.rmtree(output_path)
