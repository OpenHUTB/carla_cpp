# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#从当前目录导入SmokeTest
from . import SmokeTest

#导入carla
import carla
#导入time
import time
#导入numpy并将其简称为np
import numpy as np
#导入filecmp
import filecmp
#导入shutil
import shutil
#导入os
import os

try:
    # python 3
    #从queue模块导入Queue并将其命名为Queue
    from  import Queue as Queue
    from queue import Empty
except ImportError:
    # python 2
    #从queue模块导入Queue并将其命名为Queue
    from Queue import Queue as Queue
    from Queue import Empty

# 定义一个自定义异常 DeterminismError，用于处理场景中可能出现的确定性问题
class DeterminismError(Exception):
    pass

# 导入 Carla 模拟器中的命令，以便在场景中使用
SpawnActor = carla.command.SpawnActor
FutureActor = carla.command.FutureActor
ApplyTargetVelocity = carla.command.ApplyTargetVelocity

class Scenario(object):
    def __init__(self, client, world, save_snapshots_mode=False):
        self.world = world
        self.client = client
        self.actor_list = []
        self.init_timestamp = []
        self.active = False
        self.prefix = ""
        self.save_snapshots_mode = save_snapshots_mode
        self.snapshots = []

    def init_scene(self, prefix, settings = None, spectator_tr = None):
        self.prefix = prefix
        self.actor_list = []
        self.active = True
        self.snapshots = []

         # 重新加载世界设置，并可选择设置观众的位置
        self.reload_world(settings, spectator_tr)

        # Init timestamp
        snapshot = self.world.get_snapshot()
        self.init_timestamp = {'frame0' : snapshot.frame, 'time0' : snapshot.timestamp.elapsed_seconds}

    def add_actor(self, actor, actor_name="Actor"):
        actor_idx = len(self.actor_list)

        name = str(actor_idx) + "_" + actor_name

        self.actor_list.append((name, actor))

        if self.save_snapshots_mode:
            self.snapshots.append(np.empty((0,11), float))

    def wait(self, frames=100):              #函数用于等待一定帧数，此处为等待100帧
        for _i in range(0, frames):
            self.world.tick()

    def clear_scene(self):                   #函数用于清楚场景中的某个元素
        for actor in self.actor_list:        #遍历列表中的每个元素
            actor[1].destroy()               #用于销毁一个游戏中的某个资源
        self.active = False                  #将被销毁的资源标记为非活动状态

   def reload_world(self, settings=None, spectator_tr=None):
    # 检查是否提供了设置（settings）。如果提供了，就应用这些设置到世界（world）中。
    if settings is not None:
        self.world.apply_settings(settings)  # 应用新的世界设置

    self.wait(5)  # 调用实例方法wait，暂停5秒钟

    self.client.reload_world(False)  # 调用客户端的重新加载世界方法，参数False可能表示不重载资源

    # 注释说明接下来的代码行是为了解决一个临时性问题：给UE4（虚幻引擎4）一些时间来清理加载后（旧资源）的内存。
    time.sleep(5)  # 使程序暂停5秒，以便内存清理完成

    self.wait(5)  # 再次调用实例方法wait，暂停5秒钟

    def reset_spectator(self, spectator_tr):
        spectator = self.world.get_spectator()
        spectator.set_transform(spectator_tr)

    def save_snapshot(self, actor):
        snapshot = self.world.get_snapshot()

        actor_snapshot = np.array([
                float(snapshot.frame - self.init_timestamp['frame0']), \
                snapshot.timestamp.elapsed_seconds - self.init_timestamp['time0'], \
                actor.get_velocity().x, actor.get_velocity().y, actor.get_velocity().z, \
                actor.get_location().x, actor.get_location().y, actor.get_location().z, \
                actor.get_angular_velocity().x, actor.get_angular_velocity().y, actor.get_angular_velocity().z])
        return actor_snapshot

    def save_snapshots(self):                      #函数用于保存snapshots（快照）
        if not self.save_snapshots_mode:           #检查快照是否为真
            return                                 #若不为真，则返回

        for i in range (0, len(self.actor_list)):  #若为真，则i将进行从0到self.actor_list长度的遍历
            self.snapshots[i] = np.vstack((self.snapshots[i], self.save_snapshot(self.actor_list[i][1])))    #每次循环执行该函数

    # 保存快照到磁盘的方法
    def save_snapshots_to_disk(self):
        # 如果不开启保存快照模式，则直接返回
        if not self.save_snapshots_mode:
            return

        # 遍历所有actor，并保存每个actor的快照
        for i, actor in enumerate(self.actor_list):
            np.savetxt(self.get_filename(actor[0]), self.snapshots[i])

    # 获取带前缀的文件名的方法
    def get_filename_with_prefix(self, prefix, actor_id=None, frame=None):
        add_id = "" if actor_id is None else "_" + actor_id # 如果actor_id为空，则不添加ID
        add_frame = "" if frame is None else ("_%04d") % frame  # 如果frame为空，则不添加帧编号
        return prefix + add_id + add_frame + ".out" # 返回完整的文件名

     # 获取文件名的方法
    def get_filename(self, actor_id=None, frame=None):
        return self.get_filename_with_prefix(self.prefix, actor_id, frame)# 使用默认前缀

     # 运行仿真的方法
    def run_simulation(self, prefix, run_settings, spectator_tr, tics = 200): 
        original_settings = self.world.get_settings() # 获取原始设置

        self.init_scene(prefix, run_settings, spectator_tr)# 初始化场景

         # 运行仿真指定的帧数
        for _i in range(0, tics):
            self.world.tick()# 仿真世界前进一帧
            self.save_snapshots() # 保存快照

        self.world.apply_settings(original_settings) # 恢复原始设置
        self.save_snapshots_to_disk() # 保存快照到磁盘
        self.clear_scene() # 清理场景


class TwoCarsHighSpeedCollision(Scenario):  # 继承Scenario类，定义一个高速碰撞场景
     # 初始化场景的方法
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super(TwoCarsHighSpeedCollision, self).init_scene(prefix, settings, spectator_tr) # 调用基类的初始化方法

        blueprint_library = self.world.get_blueprint_library()# 获取蓝图库

        # 获取车辆蓝图
        vehicle00_bp = blueprint_library.filter("tt")[0]
        vehicle01_bp = blueprint_library.filter("mkz_2017")[0]

        # 设置车辆的初始位置和朝向
        vehicle00_tr = carla.Transform(carla.Location(140, -256, 0.015), carla.Rotation(yaw=180))
        vehicle01_tr = carla.Transform(carla.Location(40, -255, 0.04), carla.Rotation(yaw=0))

        # 创建一个批量请求，用于生成两个车辆并设置它们的目标速度
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


class ThreeCarsSlowSpeedCollision(Scenario):# 继承Scenario类，定义一个低速碰撞场景
     # 初始化场景的方法
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super(ThreeCarsSlowSpeedCollision, self).init_scene(prefix, settings, spectator_tr)  # 调用基类的初始化方法

        blueprint_library = self.world.get_blueprint_library()# 获取蓝图库
        # 获取车辆蓝图
        vehicle00_bp = blueprint_library.filter("prius")[0]
        vehicle01_bp = blueprint_library.filter("a2")[0]
        vehicle02_bp = blueprint_library.filter("lincoln")[0]
         # 设置车辆的初始位置和朝向
        vehicle00_tr = carla.Transform(carla.Location(110, -255, 0.05), carla.Rotation(yaw=180))
        vehicle01_tr = carla.Transform(carla.Location(53, -257, 0.00), carla.Rotation(yaw=0))
        vehicle02_tr = carla.Transform(carla.Location(85, -230, 0.04), carla.Rotation(yaw=-90))
         # 创建一个批量请求，用于生成两个车辆并设置它们的目标速度
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


class CarBikeCollision(Scenario):
    # 初始化场景的方法，接收前缀、设置和观察者变换矩阵作为参数
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        #调用父类的init_scene方法，传递相同的参数
        super(CarBikeCollision, self).init_scene(prefix, settings, spectator_tr)
        #从世界环境中获取蓝图库，蓝图库包含了可以生成的所有物体的蓝图
        blueprint_library = self.world.get_blueprint_library()
        # 从蓝图库中筛选出汽车的蓝图
        car_bp = blueprint_library.filter("mkz_2017")[0]
         # 从蓝图库中筛选出自行车（gazelle）的蓝图
        bike_bp = blueprint_library.filter("gazelle")[0]
        # 定义汽车的初始位置和朝向，位置为(50, -255, 0.04)，朝向为0度（
        car_tr = carla.Transform(carla.Location(50, -255, 0.04), carla.Rotation(yaw=0))
        # 定义自行车的初始位置和朝向
        bike_tr = carla.Transform(carla.Location(85, -245, 0.04), carla.Rotation(yaw=-90))

        batch = [
            SpawnActor(car_bp, car_tr)
            .then(ApplyTargetVelocity(FutureActor, carla.Vector3D(30, 0, 0))), # 生成汽车，并设置其目标速度为(30, 0, 0)
            SpawnActor(bike_bp, bike_tr)
            .then(ApplyTargetVelocity(FutureActor, carla.Vector3D(0, -12, 0))) # 生成自行车，并设置其目标速度为(0, -12, 0)（即向左12米每秒）
        ]

        responses = self.client.apply_batch_sync(batch)
      # 检查响应中的每个元素，确保加入组件被正确创建
        veh_ids = [x.actor_id for x in responses]
        veh_refs = [self.world.get_actor(x) for x in veh_ids]

        if (0 in veh_ids) or (None in veh_refs):
            self.fail("%s: The test cars could not be correctly spawned" % (bp_veh.id))
        # 将组件添加到场景中
        self.add_actor(veh_refs[0], "Car")
        self.add_actor(veh_refs[1], "Bike")

        self.wait(1)


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
        # 检查响应中的每个元素，确保被正确创建
        veh_ids = [x.actor_id for x in responses]
        veh_refs = [self.world.get_actor(x) for x in veh_ids]

        if (0 in veh_ids) or (None in veh_refs):
            self.fail("%s: The test cars could not be correctly spawned" % (bp_veh.id))

        self.wait(1)

        self.add_actor(veh_refs[0], "Car")
        self.add_actor(veh_refs[1], "Walker")

        self.wait(1)


class CollisionScenarioTester():
    def __init__(self, scene, output_path):
        self.scene = scene
        self.world = self.scene.world
        self.client = self.scene.client
        self.scenario_name = self.scene.__class__.__name__
        self.output_path = output_path

    def compare_files(self, file_i, file_j):
        check_ij = filecmp.cmp(file_i, file_j)

        if check_ij:
            return True

        data_i = np.loadtxt(file_i)
        data_j = np.loadtxt(file_j)

        max_error = np.amax(np.abs(data_i-data_j))

        return max_error < 0.2

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

        config_settings = self.world.get_settings()
        config_settings.synchronous_mode = True
        config_settings.fixed_delta_seconds = 1.0/fps
        config_settings.substepping = True
        config_settings.max_substep_delta_time = 1.0/fps_phys
        config_settings.max_substeps = 16

        spectator_tr = carla.Transform(carla.Location(120, -256, 10), carla.Rotation(yaw=180))

        sim_prefixes = []
        for i in range(0, repetitions):
            prefix_rep = prefix + "_rep" + str(i)
            self.scene.run_simulation(prefix_rep, config_settings, spectator_tr, tics=sim_tics)
            sim_prefixes.append(prefix_rep)

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
