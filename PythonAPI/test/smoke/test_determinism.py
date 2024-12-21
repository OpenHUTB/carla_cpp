# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from numpy import random
from . import SmokeTest
import time
import carla

TM_PORT = 7056
NUM_TICKS = 1000

class FrameRecord():
    def __init__(self, frame, vehicle_position_list):
        self.frame = frame
        self.vehicle_position_list = vehicle_position_list

class TestDeterminism(SmokeTest):

    def compare_records(self, record1_list, record2_list):
        record1_size = len(record1_list)
        record2_size = len(record2_list)
        self.assertEqual(record1_size, record2_size, msg="Record size missmatch")
        for i in range(0, record1_size):
            frame_errors = 0
            record1 = record1_list[i]
            record2 = record2_list[i]
            self.assertEqual(record1.frame, record2.frame, msg="Frame missmatch")
            num_actors1 = len(record1.vehicle_position_list)
            num_actors2 = len(record2.vehicle_position_list)
            self.assertEqual(num_actors1, num_actors2, msg="Number of actors mismatch")
            for j in range(0, num_actors1):
                loc1 = record1.vehicle_position_list[j]
                loc2 = record2.vehicle_position_list[j]
                self.assertEqual(loc1, loc2, msg="Actor location missmatch at frame %s. %s != %s"
                    % (str(record1.frame), str(loc1), str(loc2)))

    def spawn_vehicles(self, world, blueprint_transform_list):
        traffic_manager = self.client.get_trafficmanager(TM_PORT)
        vehicle_actor_list = []

        SpawnActor = carla.command.SpawnActor
        SetAutopilot = carla.command.SetAutopilot
        FutureActor = carla.command.FutureActor

        batch = []
        for blueprint, transform in blueprint_transform_list:
            batch.append(SpawnActor(blueprint, transform)
                .then(SetAutopilot(FutureActor, True, traffic_manager.get_port())))

        vehicle_actor_ids = []
        for response in self.client.apply_batch_sync(batch, True):
            if not response.error:
                vehicle_actor_ids.append(response.actor_id)

        vehicle_actor_list = world.get_actors(vehicle_actor_ids)

        return vehicle_actor_list

    def run_simulation(self, world, vehicle_actor_list):
        simulation_record = []
        ticks = 1
        while True:
            if ticks == NUM_TICKS:
                break
            else:
                position_list = []
                for vehicle in vehicle_actor_list:
                    location = vehicle.get_location()
                    position_list.append(location)
                simulation_record.append(FrameRecord(ticks, position_list))
                ticks = ticks + 1
                world.tick()
        return simulation_record

    def test_determ(self):                      #函数用于测试确定性相关的功能
        print("TestDeterminism.test_determ")    #打印当前正在执行的测试方法名称
        number_of_vehicles = 100                #定义变量number_of_vehicles并赋值为100
        tm_seed = 1                             #定义变量tm_seed并赋值为1

        self.client.load_world("Town03")        #通过客户端对象（self.client）加载名为"Town03"的世界场景
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)            
        #暂停程序执行5秒钟，清理加载场景后可能残留的旧资源内存，以确保后续操作的稳定性

        # set setting for round 1
        world = self.client.get_world()
        #获取当前客户端所连接的世界对象
        old_settings = world.get_settings()
        #获取当前世界的设置信息，并存储在old_settings中
        new_settings = world.get_settings()
        #再次获取当前世界的设置信息，并存储在new_settings中
        new_settings.synchronous_mode = True
        #将世界的同步模式设置为True
        new_settings.fixed_delta_seconds = 0.05
        #设置世界的固定时间步长为0.05秒
        world.apply_settings(new_settings)
        #将修改后的设置应用到世界中，使新的设置生效
        blueprints = world.get_blueprint_library().filter('vehicle.*')
        #从世界的蓝图库中筛选出所有以"vehicle."开头的蓝图，筛选结果存储在blueprints中。
        spawn_points = world.get_map().get_spawn_points()
        #获取当前世界地图中的所有生成点，存储在spawn_points中

        # --------------
        # Spawn vehicles
        # --------------
        blueprint_transform_list = []                       #创建一个列表
        hero = True                                         #设置一个布尔变量，用于区分不同的蓝图
        for n, transform in enumerate(spawn_points):        #用for循环和enumerate函数遍历spawm_points
            if n >= number_of_vehicles:                     #如果n大于等于车辆的数量，则停止循环
                break
            blueprint = random.choice(blueprints)           #从blueprints中随机选择一个蓝图
            if blueprint.has_attribute('color'):            #检查蓝图是否有color属性
                color = random.choice(blueprint.get_attribute('color').recommended_values)  
                blueprint.set_attribute('color', color)     #如果有color属性，就从color属性中随机选择一个，并设置给blueprint（蓝图）
            if blueprint.has_attribute('driver_id'):
                driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
                blueprint.set_attribute('driver_id', driver_id)
            if hero:
                blueprint.set_attribute('role_name', 'hero')
                hero = False
            else:
                blueprint.set_attribute('role_name', 'autopilot')
            blueprint_transform_list.append((blueprint, transform))

        # reset for simulation 1
        self.client.reload_world(False)
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)
        world = self.client.get_world()
        traffic_manager = self.client.get_trafficmanager(TM_PORT)
        traffic_manager.set_synchronous_mode(True)
        traffic_manager.set_random_device_seed(tm_seed)
        traffic_manager.set_hybrid_physics_mode(True)

        # run simulation 1
        vehicle_actor_list = self.spawn_vehicles(world, blueprint_transform_list)
        record_run1 = self.run_simulation(world, vehicle_actor_list)
        traffic_manager.shut_down()

        # reset for simulation 2
        self.client.reload_world(False)
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)
        world = self.client.get_world()
        traffic_manager = self.client.get_trafficmanager(TM_PORT)
        traffic_manager.set_synchronous_mode(True)
        traffic_manager.set_random_device_seed(tm_seed)
        traffic_manager.set_hybrid_physics_mode(True)

        #run simulation 2
        vehicle_actor_list = self.spawn_vehicles(world, blueprint_transform_list)
        record_run2 = self.run_simulation(world, vehicle_actor_list)
        traffic_manager.shut_down()

        self.client.reload_world()
        world.apply_settings(old_settings)
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)

        self.compare_records(record_run1, record_run2)
