from numpy import random
from. import SmokeTest
import time
import carla

# 交通管理器使用的端口号
TM_PORT = 7056
# 模拟的总帧数（tick次数）
NUM_TICKS = 1000


# 用于记录每一帧的相关信息的类，包含帧编号以及车辆位置列表
class FrameRecord():
    def __init__(self, frame, vehicle_position_list):
        self.frame = frame
        self.vehicle_position_list = vehicle_position_list


class TestDeterminism(SmokeTest):
    # 比较两组记录是否一致的方法，用于验证模拟的确定性
    def compare_records(self, record1_list, record2_list):
        # 获取记录1的长度
        record1_size = len(record1_list)
        # 获取记录2的长度
        record2_size = len(record2_list)
        # 比较两组记录的长度是否相等，如果不相等则报错提示记录大小不匹配
        self.assertEqual(record1_size, record2_size, msg="Record size missmatch")
        for i in range(0, record1_size):
            frame_errors = 0
            record1 = record1_list[i]
            record2 = record2_list[i]
            # 比较对应位置记录的帧编号是否相等，不相等则报错提示帧不匹配
            self.assertEqual(record1.frame, record2.frame, msg="Frame missmatch")
            num_actors1 = len(record1.vehicle_position_list)
            num_actors2 = len(record2.vehicle_position_list)
            # 比较对应位置记录中的车辆（演员）数量是否相等，不相等则报错提示演员数量不匹配
            self.assertEqual(num_actors1, num_actors2, msg="Number of actors mismatch")
            for j in range(0, num_actors1):
                loc1 = record1.vehicle_position_list[j]
                loc2 = record2.vehicle_position_list[j]
                # 比较对应车辆在对应帧的位置是否相等，不相等则报错提示位置不匹配，并显示具体帧、位置信息
                self.assertEqual(loc1, loc2, msg="Actor location missmatch at frame %s. %s!= %s"
                    % (str(record1.frame), str(loc1), str(loc2)))

    # 在世界中生成车辆的方法，根据给定的蓝图和变换信息生成车辆并设置自动导航等相关属性
    def spawn_vehicles(self, world, blueprint_transform_list):
        # 获取交通管理器实例
        traffic_manager = self.client.get_trafficmanager(TM_PORT)
        vehicle_actor_list = []

        # 导入用于生成演员、设置自动导航以及获取未来演员的相关类
        SpawnActor = carla.command.SpawnActor
        SetAutopilot = carla.command.SetAutopilot
        FutureActor = carla.command.FutureActor

        batch = []
        # 遍历蓝图和变换信息列表，构建生成车辆并设置自动导航的命令批处理列表
        for blueprint, transform in blueprint_transform_list:
            batch.append(SpawnActor(blueprint, transform)
               .then(SetAutopilot(FutureActor, True, traffic_manager.get_port())))

        vehicle_actor_ids = []
        # 批量执行命令，获取生成的车辆的actor_id列表，如果没有错误则添加到列表中
        for response in self.client.apply_batch_sync(batch, True):
            if not response.error:
                vehicle_actor_ids.append(response.actor_id)

        # 根据actor_id列表获取对应的车辆演员列表
        vehicle_actor_list = world.get_actors(vehicle_actor_ids)

        return vehicle_actor_list

    # 运行模拟的方法，在每一帧记录车辆的位置信息，直到达到设定的总帧数
    def run_simulation(self, world, vehicle_actor_list):
        simulation_record = []
        ticks = 1
        while True:
            if ticks == NUM_TICKS:
                break
            else:
                position_list = []
                # 遍历车辆列表，获取每辆车当前的位置信息并添加到位置列表中
                for vehicle in vehicle_actor_list:
                    location = vehicle.get_location()
                    position_list.append(location)
                # 创建一个FrameRecord实例记录当前帧的信息（帧编号和车辆位置列表），并添加到模拟记录列表中
                simulation_record.append(FrameRecord(ticks, position_list))
                ticks = ticks + 1
                world.tick()
        return simulation_record

    def test_determ(self):
        print("TestDeterminism.test_determ")
        # 要生成的车辆数量
        number_of_vehicles = 100
        # 交通管理器的随机种子，用于控制随机性保证可重复性
        tm_seed = 1

        # 加载名为Town03的世界场景
        self.client.load_world("Town03")
        # 工作区解决办法：给UE4一些时间清理加载旧资产后的内存
        time.sleep(5)

        # 设置第一轮模拟的相关设置
        world = self.client.get_world()
        old_settings = world.get_settings()
        new_settings = world.get_settings()
        new_settings.synchronous_mode = True
        new_settings.fixed_delta_seconds = 0.05
        world.apply_settings(new_settings)

        # 获取世界中的车辆蓝图库，并筛选出车辆相关的蓝图
        blueprints = world.get_blueprint_library().filter('vehicle.*')
        # 获取世界地图中的生成点（车辆可生成的位置）
        spawn_points = world.get_map().get_spawn_points()

        # --------------
        # Spawn vehicles
        # --------------
        blueprint_transform_list = []
        hero = True
        # 遍历生成点，选择一定数量的生成点来生成车辆
        for n, transform in enumerate(spawn_points):
            if n >= number_of_vehicles:
                break
            blueprint = random.choice(blueprints)
            # 如果蓝图有颜色属性，则随机选择一个推荐的颜色值并设置
            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)
            # 如果蓝图有司机ID属性，则随机选择一个推荐的司机ID值并设置
            if blueprint.has_attribute('driver_id'):
                driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
                blueprint.set_attribute('driver_id', driver_id)
            if hero:
                blueprint.set_attribute('role_name', 'hero')
                hero = False
            else:
                blueprint.set_attribute('role_name', 'autopilot')
            blueprint_transform_list.append((blueprint, transform))

        # 重置世界（不重新加载地图），用于模拟1前的准备
        self.client.reload_world(False)
        # 工作区解决办法：给UE4一些时间清理加载旧资产后的内存
        time.sleep(5)
        world = self.client.get_world()
        traffic_manager = self.client.get_trafficmanager(TM_PORT)
        traffic_manager.set_synchronous_mode(True)
        traffic_manager.set_random_device_seed(tm_seed)
        traffic_manager.set_hybrid_physics_mode(True)

        # 运行模拟1，生成车辆并记录模拟过程中的相关信息
        vehicle_actor_list = self.spawn_vehicles(world, blueprint_transform_list)
        record_run1 = self.run_simulation(world, vehicle_actor_list)
        traffic_manager.shut_down()

        # 重置世界（不重新加载地图），用于模拟2前的准备
        self.client.reload_world(False)
        # 工作区解决办法：给UE4一些时间清理加载旧资产后的内存
        time.sleep(5)
        world = self.client.get_world()
        traffic_manager = self.client.get_trafficmanager(TM_PORT)
        traffic_manager.set_synchronous_mode(True)
        traffic_manager.set_random_device_seed(tm_seed)
        traffic_manager.set_hybrid_physics_mode(True)

        # 运行模拟2，再次生成车辆并记录模拟过程中的相关信息
        vehicle_actor_list = self.spawn_vehicles(world, blueprint_transform_list)
        record_run2 = self.run_simulation(world, vehicle_actor_list)
        traffic_manager.shut_down()

        # 重新加载世界，恢复原始设置
        self.client.reload_world()
        world.apply_settings(old_settings)
        # 工作区解决办法：给UE4一些时间清理加载旧资产后的内存
        time.sleep(5)

        # 比较两次模拟记录是否一致，以验证模拟的确定性
        self.compare_records(record_run1, record_run2)
