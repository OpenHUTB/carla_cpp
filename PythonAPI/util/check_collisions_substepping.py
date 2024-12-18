#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Test collisions example for CARLA
This script runs several scenarios involving collisions and check if they
are deterministic for different simulation parameters.
"""

import glob
import os
import sys
import argparse
import time
import filecmp
import shutil

import numpy as np

# 尝试将CARLA模块所在路径添加到Python系统路径中，通过查找特定格式的CARLA egg文件路径来确定模块位置，根据操作系统不同选择不同的后缀格式（win-amd64或linux-x86_64）
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

# 定义一个基础的场景类，用于构建各种碰撞测试场景的通用逻辑和功能
class Scenario():
    def __init__(self, client, world, save_snapshots_mode=False):
        """
        构造函数，初始化场景相关的各种属性。

        :param client: CARLA客户端对象，用于与CARLA服务器通信。
        :param world: CARLA世界对象，代表模拟的环境。
        :param save_snapshots_mode: 是否开启保存场景快照模式，默认为False。
        """
        self.world = world
        self.client = client
        self.actor_list = []  # 存储场景中的所有参与者（如车辆、行人等）信息的列表
        self.init_timestamp = []  # 记录场景初始化的时间戳相关信息
        self.active = False  # 场景是否处于活动状态
        self.prefix = ""  # 用于生成文件名等的前缀字符串
        self.save_snapshots_mode = save_snapshots_mode
        self.snapshots = []  # 存储场景快照数据的列表，每个元素对应一个参与者的快照信息

    def init_scene(self, prefix, settings = None, spectator_tr = None):
        """
        初始化场景，设置场景的基本属性、重新加载世界、记录初始时间戳等。

        :param prefix: 场景相关文件名等的前缀字符串。
        :param settings: 可选的世界设置参数，用于配置世界的运行模式等。
        :param spectator_tr: 可选的观察者（摄像机）的变换信息，用于设置观察视角。
        """
        self.prefix = prefix
        self.actor_list = []
        self.active = True
        self.snapshots = []

        self.reload_world(settings, spectator_tr)

        # 记录场景初始化时的世界快照信息，获取帧编号和经过的时间作为初始时间戳
        world_snapshot = self.world.get_snapshot()
        self.init_timestamp = {'frame0' : world_snapshot.frame, 'time0' : world_snapshot.timestamp.elapsed_seconds}

    def add_actor(self, actor, actor_name="Actor"):
        """
        将一个参与者添加到场景中，并在保存快照模式下为其初始化相应的快照数据存储结构。

        :param actor: 要添加的参与者对象（如车辆、行人等实体）。
        :param actor_name: 参与者的名称，默认为"Actor"，用于标识参与者。
        """
        actor_idx = len(self.actor_list)
        name = str(actor_idx) + "_" + actor_name

        self.actor_list.append((name, actor))

        if self.save_snapshots_mode:
            self.snapshots.append(np.empty((0, 11), float))

    def wait(self, frames=100):
        """
        让世界进行指定帧数的更新，模拟时间推进。

        :param frames: 要更新的帧数，默认为100帧。
        """
        for _i in range(0, frames):
            self.world.tick()

    def clear_scene(self):
        """
        清除场景中的所有参与者，销毁对应的实体对象，并将场景设置为非活动状态。
        """
        for actor in self.actor_list:
            actor[1].destroy()

        self.active = False

    def reload_world(self, settings = None, spectator_tr = None):
        """
        重新加载CARLA世界，可应用指定的设置参数和观察者变换信息。

        :param settings: 可选的世界设置参数，用于配置世界的运行模式等。
        :param spectator_tr: 可选的观察者（摄像机）的变换信息，用于设置观察视角。
        """
        self.client.reload_world()
        if settings is not None:
            self.world.apply_settings(settings)
        if spectator_tr is not None:
            self.reset_spectator(spectator_tr)

    def reset_spectator(self, spectator_tr):
        """
        根据给定的变换信息设置观察者（摄像机）的位置和朝向等。

        :param spectator_tr: 观察者的变换信息，包含位置和旋转角度等。
        """
        spectator = self.world.get_spectator()
        spectator.set_transform(spectator_tr)

    def save_snapshot(self, actor):
        """
        获取给定参与者的当前状态快照信息，包括帧编号差值、时间差值以及位置、速度、角速度等信息。

        :param actor: 要获取快照的参与者对象。
        :return: 包含参与者状态信息的一维numpy数组，长度为11。
        """
        snapshot = self.world.get_snapshot()

        actor_snapshot = np.array([
                float(snapshot.frame - self.init_timestamp['frame0']), \
                snapshot.timestamp.elapsed_seconds - self.init_timestamp['time0'], \
                actor.get_location().x, actor.get_location().y, actor.get_location().z, \
                actor.get_velocity().x, actor.get_velocity().y, actor.get_velocity().z, \
                actor.get_angular_velocity().x, actor.get_angular_velocity().y, actor.get_angular_velocity().z])
        return actor_snapshot

    def save_snapshots(self):
        """
        在保存快照模式下，将所有参与者的当前快照信息添加到对应的快照数据存储结构中。
        """
        if not self.save_snapshots_mode:
            return

        for i in range (0, len(self.actor_list)):
            self.snapshots[i] = np.vstack((self.snapshots[i], self.save_snapshot(self.actor_list[i][1])))

    def save_snapshots_to_disk(self):
        """
        在保存快照模式下，将所有参与者的快照数据保存到磁盘文件中，文件名基于参与者名称和前缀等生成。
        """
        if not self.save_snapshots_mode:
            return

        for i, actor in enumerate(self.actor_list):
            np.savetxt(self.get_filename(actor[0]), self.snapshots[i])

    def get_filename_with_prefix(self, prefix, actor_id=None, frame=None):
        """
        根据给定的前缀、参与者ID和帧编号生成完整的文件名，若参与者ID或帧编号为None则不添加相应部分到文件名中。

        :param prefix: 文件名的前缀部分。
        :param actor_id: 参与者的ID，可选参数，默认为None。
        :param frame: 帧编号，可选参数，默认为None。
        :return: 生成的完整文件名字符串。
        """
        add_id = "" if actor_id is None else "_" + actor_id
        add_frame = "" if frame is None else ("_%04d") % frame
        return prefix + add_id + add_frame + ".out"

    def get_filename(self, actor_id=None, frame=None):
        """
        通过调用get_filename_with_prefix方法获取文件名，使用场景自身的前缀字符串。

        :param actor_id: 参与者的ID，可选参数，默认为None。
        :param frame: 帧编号，可选参数，默认为None。
        :return: 生成的完整文件名字符串。
        """
        return self.get_filename_with_prefix(self.prefix, actor_id, frame)

    def run_simulation(self, prefix, run_settings, spectator_tr, tics = 200):
        """
        运行整个场景的模拟过程，包括初始化场景、模拟指定帧数、保存快照到磁盘以及清理场景等操作，并返回模拟耗时。

        :param prefix: 场景相关文件名等的前缀字符串，用于本次模拟过程。
        :param run_settings: 模拟过程中要应用的世界设置参数。
        :param spectator_tr: 观察者（摄像机）的变换信息，用于设置观察视角。
        :param tics: 要模拟的帧数，默认为200帧。
        :return: 模拟过程所花费的时间（以秒为单位）。
        """
        original_settings = self.world.get_settings()

        self.init_scene(prefix, run_settings, spectator_tr)

        t_start = time.perf_counter()
        for _i in range(0, tics):
            self.world.tick()
            self.save_snapshots()
        t_end = time.perf_counter()

        self.world.apply_settings(original_settings)
        self.save_snapshots_to_disk()
        self.clear_scene()

        return t_end - t_start


# 以下是继承自Scenario类的各种具体碰撞场景类，每个类实现了特定的碰撞场景初始化逻辑，即在场景中生成相应的车辆并设置初始速度等信息

class TwoSpawnedCars(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(100, -257, 0.02), carla.Rotation(yaw=181.5))
        vehicle00 = self.world.spawn_actor(blueprint_library.filter("tt")[0], vehicle00_tr)

        vehicle01_tr = carla.Transform(carla.Location(110, -253, 0.04), carla.Rotation(yaw=181.5))
        vehicle01 = self.world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle01_tr)
        self.wait(1)

        vehicle00.set_target_velocity(carla.Vector3D(-25, 0, 0))
        vehicle01.set_target_velocity(carla.Vector3D(-25, 0, 0))

        self.add_actor(vehicle00, "Car")
        self.add_actor(vehicle01, "Car")

        self.wait(1)



class TwoCarsSlowSpeedCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(100, -256, 0.015), carla.Rotation(yaw=178))
        vehicle00 = self.world.spawn_actor(blueprint_library.filter("tt")[0], vehicle00_tr)

        vehicle01_tr = carla.Transform(carla.Location(40, -255, 0.04), carla.Rotation(yaw=0))
        vehicle01 = self.world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle01_tr)
        self.wait(1)

        vehicle00.set_target_velocity(carla.Vector3D(-12, 0, 0))
        vehicle01.set_target_velocity(carla.Vector3D(+12, 0, 0))

        self.add_actor(vehicle00, "Car")
        self.add_actor(vehicle01, "Car")

        self.wait(1)


class TwoCarsHighSpeedCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(140, -256, 0.015), carla.Rotation(yaw=180))
        vehicle00 = self.world.spawn_actor(blueprint_library.filter("tt")[0], vehicle00_tr)

        vehicle01_tr = carla.Transform(carla.Location(40, -255, 0.04), carla.Rotation(yaw=0))
        vehicle01 = self.world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle01_tr)
        self.wait(1)

        vehicle00.set_target_velocity( carla.Vector3D(-50, 0, 0))
        vehicle01.set_target_velocity(carla.Vector3D(+50, 0, 0))

        self.add_actor(vehicle00, "Car")
        self.add_actor(vehicle01, "Car")

        self.wait(1)


class ThreeCarsSlowSpeedCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(110, -255, 0.05), carla.Rotation(yaw=180))
        vehicle00 = self.world.spawn_actor(blueprint_library.filter("prius")[0], vehicle00_tr)

        vehicle01_tr = carla.Transform(carla.Location(53, -257, 0.00), carla.Rotation(yaw=0))
        vehicle01 = self.world.spawn_actor(blueprint_library.filter("a2")[0], vehicle01_tr)

        vehicle02_tr = carla.Transform(carla.Location(85, -230, 0.04), carla.Rotation(yaw=-90))
        vehicle02 = self.world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle02_tr)

        self.wait(1)

        vehicle00.set_target_velocity(carla.Vector3D(-15, 0, 0))
        vehicle01.set_target_velocity(carla.Vector3D(+15, 0, 0))
        vehicle02.set_target_velocity(carla.Vector3D(0, -15, 0))

        self.add_actor(vehicle00, "Car")
        self.add_actor(vehicle01, "Car")
        self.add_actor(vehicle02, "Car")

        self.wait(1)



class ThreeCarsHighSpeedCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(110, -255, 0.05), carla.Rotation(yaw=180))
        vehicle00 = self.world.spawn_actor(blueprint_library.filter("prius")[0], vehicle00_tr)

        vehicle01_tr = carla.Transform(carla.Location(53, -257, 0.00), carla.Rotation(yaw=0))
        vehicle01 = self.world.spawn_actor(blueprint_library.filter("a2")[0], vehicle01_tr)

        vehicle02_tr = carla.Transform(carla.Location(85, -230, 0.04), carla.Rotation(yaw=-90))
        vehicle02 = self.world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle02_tr)

        self.wait(1)

        vehicle00.set_target_velocity(carla.Vector3D(-30, 0, 0))
        vehicle01.set_target_velocity(carla.Vector3D(+30, 0, 0))
        vehicle02.set_target_velocity(carla.Vector3D(0, -30, 0))

        self.add_actor(vehicle00, "Car")
        self.add_actor(vehicle01, "Car")
        self.add_actor(vehicle02, "Car")

        self.wait(1)



class CarBikeCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        car_tr = carla.Transform(carla.Location(50, -255, 0.04), carla.Rotation(yaw=0))
        car = self.world.spawn_actor(blueprint_library.filter("*lincoln*")[0], car_tr)

        bike_tr = carla.Transform(carla.Location(85, -245, 0.04), carla.Rotation(yaw=-90))
        bike = self.world.spawn_actor(blueprint_library.filter("*gazelle*")[0], bike_tr)
        self.wait(1)

        car.set_target_velocity(carla.Vector3D(+30, 0, 0))
        bike.set_target_velocity(carla.Vector3D(0, -12, 0))

        self.add_actor(car, "Car")
        self.add_actor(bike, "Bike")

        self.wait(1)



        # 以下是继承自Scenario类的各种具体碰撞场景类，每个类实现了特定的碰撞场景初始化逻辑，即在场景中生成相应的车辆等实体并设置初始速度等信息（续）

class CarBikeCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        car_tr = carla.Transform(carla.Location(50, -255, 0.04), carla.Rotation(yaw=0))
        car = self.world.spawn_actor(blueprint_library.filter("*lincoln*")[0], car_tr)

        bike_tr = carla.Transform(carla.Location(85, -245, 0.04), carla.Rotation(yaw=-90))
        bike = self.world.spawn_actor(blueprint_library.filter("*gazelle*")[0], bike_tr)
        self.wait(1)

        car.set_target_velocity(carla.Vector3D(+30, 0, 0))
        bike.set_target_velocity(carla.Vector3D(0, -12, 0))

        self.add_actor(car, "Car")
        self.add_actor(bike, "Bike")

        self.wait(1)



class CarWalkerCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        car_tr = carla.Transform(carla.Location(50, -255, 0.04), carla.Rotation(yaw=0))
        car = self.world.spawn_actor(blueprint_library.filter("*lincoln*")[0], car_tr)

        walker_tr = carla.Transform(carla.Location(85, -255, 1.00), carla.Rotation(yaw=-90))
        walker_bp = blueprint_library.filter("walker.pedestrian.0007")[0]
        if walker_bp.has_attribute('is_invincible'):
            walker_bp.set_attribute('is_invincible', 'false')
        walker = self.world.spawn_actor(walker_bp, walker_tr)

        self.wait(1)

        car.set_target_velocity(carla.Vector3D(+20, 0, 0))
        walker.set_simulate_physics(True)
        self.add_actor(car, "Car")
        self.add_actor(walker, "Walker")

        self.wait(1)


# 用于测试碰撞场景是否具有确定性的类，通过比较多次模拟的结果来判断
class CollisionScenarioTester():
    def __init__(self, scene, output_path):
        """
        构造函数，初始化测试器相关属性。

        :param scene: 要测试的具体碰撞场景对象。
        :param output_path: 输出文件的保存路径。
        """
        self.scene = scene
        self.world = self.scene.world
        self.client = self.scene.client
        self.scenario_name = self.scene.__class__.__name__
        self.output_path = output_path

    def compare_files(self, file_i, file_j):
        """
        比较两个文件是否完全相同，如果文件内容不同，则进一步比较文件中的数据，判断数据差异是否小于设定阈值（0.01）。

        :param file_i: 要比较的第一个文件路径。
        :param file_j: 要比较的第二个文件路径。
        :return: 如果文件相同或者数据差异小于阈值则返回True，否则返回False。
        """
        check_ij = filecmp.cmp(file_i, file_j)

        if check_ij:
            return True

        data_i = np.loadtxt(file_i)
        data_j = np.loadtxt(file_j)

        max_error = np.amax(np.abs(data_i - data_j))

        return max_error < 0.01

    def check_simulations(self, rep_prefixes, gen_prefix):
        """
        检查多次模拟的结果是否具有确定性，通过比较每次模拟中各个参与者的状态文件来判断。

        :param rep_prefixes: 多次模拟的文件名前缀列表，每个前缀对应一次模拟过程。
        :param gen_prefix: 用于生成参考文件名等的通用前缀字符串。
        :return: 一个包含每次模拟与其他模拟比较结果的集合，元素表示每次模拟与其他模拟相同的次数，按从大到小排序。
        """
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

        determinism = np.sum(mat_check, axis=1)
        # max_rep_equal = np.amax(determinism)
        max_rep_equal_idx = np.argmax(determinism)
        min_rep_equal_idx = np.argmin(determinism)

        determinism_set = list(set(determinism))
        determinism_set.sort(reverse=True)

        # print(determinism)
        # print(np.argmax(determinism))
        # print(np.argmin(determinism))

        self.save_simulations(rep_prefixes, gen_prefix, max_rep_equal_idx, min_rep_equal_idx)

        return determinism_set

    def save_simulations(self, rep_prefixes, prefix, max_idx, min_idx):
        """
        根据比较结果保存模拟相关的文件，将确定性最高（相同次数最多）的模拟文件作为参考文件复制保存，同时可将确定性最低的模拟文件也复制保存（如果与最高的不同），最后删除所有原始的模拟文件。

        :param rep_prefixes: 多次模拟的文件名前缀列表，每个前缀对应一次模拟过程。
        :param prefix: 用于生成参考文件名等的通用前缀字符串。
        :param max_idx: 确定性最高（相同次数最多）的模拟索引。
        :param min_idx: 确定性最低（相同次数最少）的模拟索引。
        """
        for actor in self.scene.actor_list:
            actor_id = actor[0]
            reference_id = "reference_" + actor_id
            file_repetition = self.scene.get_filename_with_prefix(rep_prefixes[max_idx], actor_id)
            file_reference  = self.scene.get_filename_with_prefix(prefix, reference_id)

            shutil.copyfile(file_repetition, file_reference)

        if min_idx!= max_idx:
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

                os.remove(file_repetition)

    def test_scenario(self, fps=20, fps_phys=100, repetitions = 1, sim_tics = 100):
        """
        对给定的碰撞场景进行确定性测试，设置模拟相关参数，运行多次模拟，检查模拟结果的确定性，并输出相应的测试信息。

        :param fps: 渲染帧率，默认为20帧/秒。
        :param fps_phys: 物理帧率，默认为100帧/秒，用于细分物理模拟的时间步长以提高精度。
        :param repetitions: 模拟重复次数，默认为1次。
        :param sim_tics: 每次模拟的帧数，默认为100帧。
        :return: 包含测试结果信息的字符串，描述场景名称、模拟参数以及确定性相关情况等内容。
        """
        output_str = "Testing Determinism in %s for %3d render FPS and %3d physics FPS -> " % (self.scenario_name, fps, fps_phys)

        # 创建运行相关的参数，包括文件名前缀、世界设置以及观察者变换信息等
        prefix = self.output_path + self.scenario_name + "_" + str(fps) + "_" + str(fps_phys)

        config_settings = self.world.get_settings()
        config_settings.synchronous_mode = True
        config_settings.fixed_delta_seconds = 1.0 / fps
        config_settings.substepping = True
        config_settings.max_substep_delta_time = 1.0 / fps_phys
        config_settings.max_substeps = 16

        spectator_tr = carla.Transform(carla.Location(120, -256, 10), carla.Rotation(yaw=180))

        t_comp = 0
        sim_prefixes = []
        for i in range(0, repetitions):
            prefix_rep = prefix + "_rep" + str(i)
            t_comp += self.scene.run_simulation(prefix_rep, config_settings, spectator_tr, tics=sim_tics)
            sim_prefixes.append(prefix_rep)

        determ_repet = self.check_simulations(sim_prefixes, prefix)
        output_str += "Deterministic Repetitions: %r / %2d" % (determ_repet, repetitions)
        output_str += "  -> Comp. Time per frame: %.0f" % (t_comp / repetitions * sim_tics)

        if determ_repet[0]!= repetitions:
            print("Error!!! Scenario %s is not deterministic: %d / %d" % (self.scenario_name, determ_repet[0], repetitions))

        return output_str



def main(arg):
    """Main function of the script"""
    client = carla.Client(arg.host, arg.port)
    client.set_timeout(30.0)
    world = client.get_world()
    pre_settings = world.get_settings()
    world = client.load_world("Town03")

    spectator_transform = carla.Transform(carla.Location(120, -256, 5), carla.Rotation(yaw=180))
    spectator_transform.location.z += 5
    spectator = world.get_spectator()
    spectator.set_transform(spectator_transform)

    try:
        # 设置输出临时文件夹路径，如果不存在则创建该文件夹
        output_path = os.path.dirname(os.path.realpath(__file__))
        output_path = os.path.join(output_path, "_collisions") + os.path.sep
        if not os.path.exists(output_path):
            os.mkdir(output_path)

        # 创建多个不同碰撞场景的测试器对象列表，每个测试器对应一个具体的碰撞场景，并开启保存快照模式
        test_list = [
            CollisionScenarioTester(TwoSpawnedCars(client, world, True), output_path),
            CollisionScenarioTester(TwoCarsSlowSpeedCollision(client, world, True), output_path),
            CollisionScenarioTester(TwoCarsHighSpeedCollision(client, world, True), output_path),
            CollisionScenarioTester(CarBikeCollision(client, world, True), output_path),
            CollisionScenarioTester(CarWalkerCollision(client, world, True), output_path),
            CollisionScenarioTester(ThreeCarsSlowSpeedCollision(client, world, True), output_path),
            CollisionScenarioTester(ThreeCarsHighSpeedCollision(client, world, True), output_path),
        ]

        repetitions = 10
        for item in test_list:
            print("--------------------------------------------------------------")
            # 对每个测试器对应的场景进行多次不同参数组合下的确定性测试，此处注释掉了部分测试参数组合，只保留了一组示例测试
            # item.test_scenario(20,  20, repetitions)
            # item.test_scenario(20,  40, repetitions)
            # item.test_scenario(20,  60, repetitions)
            # item.test_scenario(20,  80, repetitions)
            out = item.test_scenario(20, 100, repetitions)
            print(out)

        print("--------------------------------------------------------------")

        # 以下代码被注释掉了，原本功能是删除所有的输出文件所在文件夹及其内容，可能用于清理测试产生的临时文件
        # Remove all the output files
        # shutil.rmtree(path)

    finally:
        world.apply_settings(pre_settings)



if __name__ == "__main__":

    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='localhost',
        help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port of CARLA Simulator (default: 2000)')
    argparser.add_argument(
        '--filter',
        metavar='PATTERN',
        default='model3',
        help='actor filter (default: "vehicle.*")')
#    argparser.add_argument(
#        '-fps', '--fps',
#        metavar='FPS',
#        default=20,
#        type=int,
#        help='Frames per simulatation second (default: 20)')
#    argparser.add_argument(
#        '-phys_fps', '--phys_fps',
#        metavar='PHYSFPS',
#        default=100,
#        type=int,
#        help='Target physical frames per simulatation second, it will \
#            divide the dt in substeps if required to get more precision.  (default: 100)')
    args = argparser.parse_args()

    try:
        main(args)
    except KeyboardInterrupt:
        print(' - Exited by user.')
