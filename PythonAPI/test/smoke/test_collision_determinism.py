# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from . import SmokeTest

import carla# 导入CARLA模拟器相关的库，用于场景创建、车辆等实体操作等
import time
import numpy as np
import filecmp
import shutil
import os
# 根据Python版本导入不同的队列模块，用于多线程等场景下的数据传递
try:
    # python 3
    from queue import Queue as Queue
    from queue import Empty
except ImportError:
    # python 2
    from Queue import Queue as Queue
    from Queue import Empty
# 自定义异常类，用于表示确定性错误（例如模拟结果不一致等情况）
class DeterminismError(Exception):
    pass
# 从CARLA库中导入用于生成角色、处理角色未来状态以及设置目标速度的相关命令类
SpawnActor = carla.command.SpawnActor
FutureActor = carla.command.FutureActor
ApplyTargetVelocity = carla.command.ApplyTargetVelocity
# 场景基类，定义了场景相关的通用操作和属性
class Scenario(object):
    def __init__(self, client, world, save_snapshots_mode=False):# CARLA世界对象，包含场景中的所有实体和信息
        self.world = world
        self.client = client
        self.actor_list = []
        self.init_timestamp = []
        self.active = False
        self.prefix = ""
        self.save_snapshots_mode = save_snapshots_mode
        self.snapshots = []
# 初始化场景，设置前缀、加载相关设置、重置观察者位置等
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        self.prefix = prefix
        self.actor_list = []
        self.active = True
        self.snapshots = []

        self.reload_world(settings, spectator_tr)

        # Init timestamp
        snapshot = self.world.get_snapshot()
        self.init_timestamp = {'frame0' : snapshot.frame, 'time0' : snapshot.timestamp.elapsed_seconds}
 # 向场景中添加角色，给角色分配一个编号加名称的标识，并在保存快照模式下初始化对应的数据结构
    def add_actor(self, actor, actor_name="Actor"):
        actor_idx = len(self.actor_list)

        name = str(actor_idx) + "_" + actor_name

        self.actor_list.append((name, actor))

        if self.save_snapshots_mode:
            self.snapshots.append(np.empty((0,11), float))
 # 等待指定帧数，让世界进行相应的更新
    def wait(self, frames=100):
        for _i in range(0, frames):
            self.world.tick()
# 清理场景，销毁场景中的所有角色
    def clear_scene(self):
        for actor in self.actor_list:
            actor[1].destroy()

        self.active = False
# 重新加载世界，应用设置并等待一段时间让UE4完成相关资源清理和加载
    def reload_world(self, settings = None, spectator_tr = None):
        if settings is not None:
            self.world.apply_settings(settings)
        self.wait(5)

        self.client.reload_world(False)
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)

        self.wait(5)
 # 重置场景观察者（摄像机等）的位置和姿态
    def reset_spectator(self, spectator_tr):
        spectator = self.world.get_spectator()
        spectator.set_transform(spectator_tr)
# 获取单个角色的当前快照数据，包括相对帧数、时间、速度、位置、角速度等信息
    def save_snapshot(self, actor):
        snapshot = self.world.get_snapshot()

        actor_snapshot = np.array([
                float(snapshot.frame - self.init_timestamp['frame0']), \
                snapshot.timestamp.elapsed_seconds - self.init_timestamp['time0'], \
                actor.get_velocity().x, actor.get_velocity().y, actor.get_velocity().z, \
                actor.get_location().x, actor.get_location().y, actor.get_location().z, \
                actor.get_angular_velocity().x, actor.get_angular_velocity().y, actor.get_angular_velocity().z])
        return actor_snapshot
 # 保存所有角色的当前快照数据
    def save_snapshots(self):
        if not self.save_snapshots_mode:
            return

        for i in range (0, len(self.actor_list)):
            self.snapshots[i] = np.vstack((self.snapshots[i], self.save_snapshot(self.actor_list[i][1])))
 # 将所有角色的快照数据保存到磁盘文件中
    def save_snapshots_to_disk(self):
        if not self.save_snapshots_mode:
            return

        for i, actor in enumerate(self.actor_list):
            np.savetxt(self.get_filename(actor[0]), self.snapshots[i])
# 根据前缀、角色ID和帧数生成带前缀的文件名（格式相关）
    def get_filename_with_prefix(self, prefix, actor_id=None, frame=None):
        add_id = "" if actor_id is None else "_" + actor_id
        add_frame = "" if frame is None else ("_%04d") % frame
        return prefix + add_id + add_frame + ".out"
 # 根据角色ID和帧数生成文件名
    def get_filename(self, actor_id=None, frame=None):
        return self.get_filename_with_prefix(self.prefix, actor_id, frame)
# 运行整个模拟场景，包括初始化、模拟推进、保存快照、恢复原始设置以及清理场景等操作
    def run_simulation(self, prefix, run_settings, spectator_tr, tics = 200):
        original_settings = self.world.get_settings()

        self.init_scene(prefix, run_settings, spectator_tr)

        for _i in range(0, tics):
            self.world.tick()
            self.save_snapshots()

        self.world.apply_settings(original_settings)
        self.save_snapshots_to_disk()
        self.clear_scene()

# 两车高速碰撞场景类，继承自Scenario类，用于定义特定的两车高速碰撞场景初始化等操作
class TwoCarsHighSpeedCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super(TwoCarsHighSpeedCollision, self).init_scene(prefix, settings, spectator_tr)
 # 获取场景的蓝图库，用于查找创建车辆等实体的蓝图（模板）
        blueprint_library = self.world.get_blueprint_library()

        vehicle00_bp = blueprint_library.filter("tt")[0]
        vehicle01_bp = blueprint_library.filter("mkz_2017")[0]

        vehicle00_tr = carla.Transform(carla.Location(140, -256, 0.015), carla.Rotation(yaw=180))
        vehicle01_tr = carla.Transform(carla.Location(40, -255, 0.04), carla.Rotation(yaw=0))

        batch = [
            SpawnActor(vehicle00_bp, vehicle00_tr)
            .then(ApplyTargetVelocity(FutureActor, carla.Vector3D(-50, 0, 0))),
            SpawnActor(vehicle01_bp, vehicle01_tr)
            .then(ApplyTargetVelocity(FutureActor, carla.Vector3D(+50, 0, 0)))
        ]

        responses = self.client.apply_batch_sync(batch)

        veh_ids = [x.actor_id for x in responses]
        veh_refs = [self.world.get_actor(x) for x in veh_ids]

        if (0 in veh_ids) or (None in veh_refs):
            self.fail("%s: The test cars could not be correctly spawned" % (bp_veh.id))

        self.add_actor(veh_refs[0], "Car")
        self.add_actor(veh_refs[1], "Car")

        self.wait(1)

# 三车低速碰撞场景类，继承自Scenario类，类似上面用于定义特定的三车低速碰撞场景相关操作
class ThreeCarsSlowSpeedCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super(ThreeCarsSlowSpeedCollision, self).init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        vehicle00_bp = blueprint_library.filter("prius")[0]
        vehicle01_bp = blueprint_library.filter("a2")[0]
        vehicle02_bp = blueprint_library.filter("lincoln")[0]

        vehicle00_tr = carla.Transform(carla.Location(110, -255, 0.05), carla.Rotation(yaw=180))
        vehicle01_tr = carla.Transform(carla.Location(53, -257, 0.00), carla.Rotation(yaw=0))
        vehicle02_tr = carla.Transform(carla.Location(85, -230, 0.04), carla.Rotation(yaw=-90))

        batch = [
            SpawnActor(vehicle00_bp, vehicle00_tr)
            .then(ApplyTargetVelocity(FutureActor, carla.Vector3D(-15, 0, 0))),
            SpawnActor(vehicle01_bp, vehicle01_tr)
            .then(ApplyTargetVelocity(FutureActor, carla.Vector3D(+15, 0, 0))),
            SpawnActor(vehicle02_bp, vehicle02_tr)
            .then(ApplyTargetVelocity(FutureActor, carla.Vector3D(0, -15, 0)))
        ]

        responses = self.client.apply_batch_sync(batch)

        veh_ids = [x.actor_id for x in responses]
        veh_refs = [self.world.get_actor(x) for x in veh_ids]

        self.add_actor(veh_refs[0], "Car")
        self.add_actor(veh_refs[1], "Car")
        self.add_actor(veh_refs[2], "Car")

        self.wait(1)

# 汽车与自行车碰撞场景类，继承自Scenario类，定义汽车与自行车碰撞场景相关操作
class CarBikeCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super(CarBikeCollision, self).init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        car_bp = blueprint_library.filter("mkz_2017")[0]
        bike_bp = blueprint_library.filter("gazelle")[0]

        car_tr = carla.Transform(carla.Location(50, -255, 0.04), carla.Rotation(yaw=0))
        bike_tr = carla.Transform(carla.Location(85, -245, 0.04), carla.Rotation(yaw=-90))

        batch = [
            SpawnActor(car_bp, car_tr)
            .then(ApplyTargetVelocity(FutureActor, carla.Vector3D(30, 0, 0))),
            SpawnActor(bike_bp, bike_tr)
            .then(ApplyTargetVelocity(FutureActor, carla.Vector3D(0, -12, 0)))
        ]

        responses = self.client.apply_batch_sync(batch)

        veh_ids = [x.actor_id for x in responses]
        veh_refs = [self.world.get_actor(x) for x in veh_ids]

        if (0 in veh_ids) or (None in veh_refs):
            self.fail("%s: The test cars could not be correctly spawned" % (bp_veh.id))

        self.add_actor(veh_refs[0], "Car")
        self.add_actor(veh_refs[1], "Bike")

        self.wait(1)

# 汽车与行人碰撞场景类，继承自Scenario类，定义汽车与行人碰撞场景相关操作
class CarWalkerCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super(CarWalkerCollision, self).init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        car_bp = blueprint_library.filter("mkz_2017")[0]
        walker_bp = blueprint_library.filter("walker.pedestrian.0007")[0]
        if walker_bp.has_attribute('is_invincible'):
            walker_bp.set_attribute('is_invincible', 'false')

        car_tr = carla.Transform(carla.Location(50, -255, 0.04), carla.Rotation(yaw=0))
        walker_tr = carla.Transform(carla.Location(85, -255, 1.00), carla.Rotation(yaw=-90))

        batch = [
            SpawnActor(car_bp, car_tr)
            .then(ApplyTargetVelocity(FutureActor, carla.Vector3D(20, 0, 0))),
            SpawnActor(walker_bp, walker_tr)
        ]

        responses = self.client.apply_batch_sync(batch)

        veh_ids = [x.actor_id for x in responses]
        veh_refs = [self.world.get_actor(x) for x in veh_ids]

        if (0 in veh_ids) or (None in veh_refs):
            self.fail("%s: The test cars could not be correctly spawned" % (bp_veh.id))

        self.wait(1)

        self.add_actor(veh_refs[0], "Car")
        self.add_actor(veh_refs[1], "Walker")

        self.wait(1)

# 碰撞场景测试器类，用于对具体的碰撞场景进行测试相关操作，如比较文件、检查模拟确定性等
class CollisionScenarioTester():
class CollisionScenarioTester():
    def __init__(self, scene, output_path):
        self.scene = scene
        self.world = self.scene.world
        self.client = self.scene.client
        self.scenario_name = self.scene.__class__.__name__
        self.output_path = output_path
# 比较两个文件是否相同，若文件内容不完全相同则计算最大误差看是否小于阈值
    def compare_files(self, file_i, file_j):
        check_ij = filecmp.cmp(file_i, file_j)

        if check_ij:
            return True

        data_i = np.loadtxt(file_i)
        data_j = np.loadtxt(file_j)

        max_error = np.amax(np.abs(data_i-data_j))

        return max_error < 0.2
# 检查多次模拟结果之间的确定性，通过比较各次模拟中各角色的文件数据来判断
    def check_simulations(self, rep_prefixes, gen_prefix):
        repetitions = len(rep_prefixes)
        mat_check = np.zeros((repetitions, repetitions), int)

        for i in range(0, repetitions):
            mat_check[i][i] = 1
            for j in range(0, i):
                sim_check = True
                for actor in self.scene.actor_list:
                    actor_id = actor[0]
                    file_i = self.scene.get_filename_with_prefix(rep_prefixes[i], actor_id)
                    file_j = self.scene.get_filename_with_prefix(rep_prefixes[j], actor_id)

                    check_ij = self.compare_files(file_i, file_j)
                    sim_check = sim_check and check_ij
                mat_check[i][j] = int(sim_check)
                mat_check[j][i] = int(sim_check)

        determinism = np.sum(mat_check,axis=1)

        determinism_set = list(set(determinism))
        determinism_set.sort(reverse=True)

        return determinism_set
 # 根据模拟结果保存相关文件，如参考文件和失败文件
    def save_simulations(self, rep_prefixes, prefix, max_idx, min_idx):
        for actor in self.scene.actor_list:
            actor_id = actor[0]
            reference_id = "reference_" + actor_id
            file_repetition = self.scene.get_filename_with_prefix(rep_prefixes[max_idx], actor_id)
            file_reference  = self.scene.get_filename_with_prefix(prefix, reference_id)

            shutil.copyfile(file_repetition, file_reference)

        if min_idx != max_idx:
            for actor in self.scene.actor_list:
                actor_id = actor[0]
                failed_id = "failed_" + actor_id
                file_repetition = self.scene.get_filename_with_prefix(rep_prefixes[min_idx], actor_id)
                file_failed     = self.scene.get_filename_with_prefix(prefix, failed_id)

                shutil.copyfile(file_repetition, file_failed)

        for r_prefix in rep_prefixes:
            for actor in self.scene.actor_list:
                actor_id = actor[0]
                file_repetition = self.scene.get_filename_with_prefix(r_prefix, actor_id)

                #os.remove(file_repetition)

    def test_scenario(self, fps=20, fps_phys=100, repetitions=1, sim_tics=100):
        # Creating run features: prefix, settings and spectator options
        prefix = self.output_path + self.scenario_name + "_" + str(fps) + "_" + str(fps_phys)
# 创建运行相关的特征信息，包括生成用于文件名等用途的前缀，以及获取、配置模拟的相关设置和观察者的位置姿态等
        config_settings = self.world.get_settings()
        config_settings.synchronous_mode = True
        config_settings.fixed_delta_seconds = 1.0/fps
        config_settings.substepping = True
        config_settings.max_substep_delta_time = 1.0/fps_phys
        config_settings.max_substeps = 16

        spectator_tr = carla.Transform(carla.Location(120, -256, 10), carla.Rotation(yaw=180))

        sim_prefixes = []
        for i in range(0, repetitions): # 按照指定的重复次数进行模拟运行
            prefix_rep = prefix + "_rep" + str(i)
            self.scene.run_simulation(prefix_rep, config_settings, spectator_tr, tics=sim_tics)
            sim_prefixes.append(prefix_rep)
结果之间的确定性，比较各次模拟产生的数据文件等
        determ_repet = self.check_simulations(sim_prefixes, prefix)

        if determ_repet[0] != repetitions:
            raise DeterminismError("CollisionTransfError: Scenario %s is not deterministic: %d / %d" % (self.scenario_name, determ_repet[0], repetitions))


class TestCollisionDeterminism(SmokeTest):
    def setUp(self):
        super(TestCollisionDeterminism, self).setUp()
        self.world = self.client.get_world()
        self.settings = self.world.get_settings()
        settings = carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=0.05)
        self.world.apply_settings(settings)
        self.world.tick()

    def tearDown(self):
        self.settings.synchronous_mode = False
        self.world.apply_settings(self.settings)
        self.world.tick()
        self.settings = None
        self.world = None
        super(TestCollisionDeterminism, self).tearDown()

    def test_two_cars(self):
        print("TestCollisionDeterminism.test_two_cars")
# 设置用于存储输出文件的临时文件夹路径，如果文件夹不存在则创建它
        # Setting output temporal folder
        output_path = os.path.dirname(os.path.realpath(__file__))
        output_path = os.path.join(output_path, "_collisions") + os.path.sep
        if not os.path.exists(output_path):
            os.mkdir(output_path)

        # Loading Town03 for test
        self.client.load_world("Town03")
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)

        try:# 创建针对两车高速碰撞场景的测试器对象，传入场景对象和输出路径
            test_collision = CollisionScenarioTester(scene=TwoCarsHighSpeedCollision(self.client, self.world, True), output_path=output_path)
            test_collision.test_scenario(repetitions=5, sim_tics=100)
        except DeterminismError as err:
            test_collision.scene.clear_scene()
            # Remove all the output files
            shutil.rmtree(output_path)
            self.fail(err)

        # Remove all the output files
        shutil.rmtree(output_path)

    def test_three_cars(self):
        print("TestCollisionDeterminism.test_three_cars")
# 以下逻辑与test_two_cars方法类似，只是针对的是三车低速碰撞场景的测试操作
        # Setting output temporal folder
        output_path = os.path.dirname(os.path.realpath(__file__))
        output_path = os.path.join(output_path, "_collisions") + os.path.sep
        if not os.path.exists(output_path):
            os.mkdir(output_path)

        # Loading Town03 for test
        self.client.load_world("Town03")
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)

        try:
            test_collision = CollisionScenarioTester(scene=ThreeCarsSlowSpeedCollision(self.client, self.world, True), output_path=output_path)
            test_collision.test_scenario(repetitions=5, sim_tics = 100)
        except DeterminismError as err:
            test_collision.scene.clear_scene()
            # Remove all the output files
            shutil.rmtree(output_path)
            self.fail(err)

        # Remove all the output files
        shutil.rmtree(output_path)

    def test_car_bike(self):
        print("TestCollisionDeterminism.test_car_bike")
# 以下逻辑与test_two_cars方法类似，只是针对的是汽车与自行车碰撞场景的测试操作
        # Setting output temporal folder
        output_path = os.path.dirname(os.path.realpath(__file__))
        output_path = os.path.join(output_path, "_collisions") + os.path.sep
        if not os.path.exists(output_path):
            os.mkdir(output_path)

        # Loading Town03 for test
        self.client.load_world("Town03")
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)

        try:
            test_collision = CollisionScenarioTester(scene=CarBikeCollision(self.client, self.world, True), output_path=output_path)
            test_collision.test_scenario(repetitions=5, sim_tics=100)
        except DeterminismError as err:
            test_collision.scene.clear_scene()
            # Remove all the output files
            shutil.rmtree(output_path)
            self.fail(err)

        # Remove all the output files
        shutil.rmtree(output_path)

    def test_car_walker(self):
        print("TestCollisionDeterminism.test_car_walker")
 # 以下逻辑与test_two_cars方法类似，只是针对的是汽车与行人碰撞场景的测试操作
        # Setting output temporal folder
        output_path = os.path.dirname(os.path.realpath(__file__))
        output_path = os.path.join(output_path, "_collisions") + os.path.sep
        if not os.path.exists(output_path):
            os.mkdir(output_path)

        # Loading Town03 for test
        self.client.load_world("Town03")
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)

        try:
            test_collision = CollisionScenarioTester(scene=CarWalkerCollision(self.client, self.world, True), output_path=output_path)
            test_collision.test_scenario(repetitions=5, sim_tics=100)
        except DeterminismError as err:
            test_collision.scene.clear_scene()
            # Remove all the output files
            shutil.rmtree(output_path)
            self.fail(err)

        # Remove all the output files
        shutil.rmtree(output_path)
