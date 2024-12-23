#这段代码的主要功能是在CARLA仿真环境中生成交通流量。具体来说，它能够：根据用户提供的参数，生成指定数量的车辆（--number-of-vehicles 或 -n）和行人（--number-of-walkers 或 -w）。
#允许用户指定车辆和行人的模型过滤器（--filterv 和 --filterw），以及它们的生成版本（--generationv 和 --generationw）。将生成的车辆设置为自动驾驶模式（使用CARLA的交通管理器），并可控制车辆的灯光状态（--car-lights-on）。
#支持异步模式（--asynch）和混合模式（--hybrid）的交通管理，允许更灵活的交通仿真。
#允许设置随机种子（--seed 和 --seedw），以便于结果的复现。
#支持在大型地图上自动重新生成休眠的车辆（--respawn）。
#支持无渲染模式（--no-rendering），可以提高仿真性能。
#通过命令行参数接受用户的输入，并根据这些输入在仿真世界中生成相应的车辆和行人。
#总的来说，这个脚本是一个用于在CARLA仿真环境中创建和管理交通流量的工具，可以用来测试和模拟各种交通场景。
#!/usr/bin/env python

# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Example script to generate traffic in the simulation"""
# 导入必要的库
import glob
import os
import sys
import time

try:
     # 尝试添加CARLA模块到系统路径中
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla
//从Carla库中导入VehicleLightState并将其重命名为vls，可能用于后续控制车辆灯光状态相关操作
from carla import VehicleLightState as vls

import argparse
import logging
from numpy import random
# 从世界场景的蓝图库中筛选出符合给定过滤器条件的蓝图列表
def get_actor_blueprints(world, filter, generation):
//从世界场景的蓝图库中筛选出符合给定过滤器条件的蓝图列表
    bps = world.get_blueprint_library().filter(filter)

    if generation.lower() == "all":
        return bps

    # If the filter returns only one bp, we assume that this one needed
    # and therefore, we ignore the generation
// 如果过滤器返回的蓝图只有一个，那么就认为这就是需要的那个，此时忽略生成版本的限制
    if len(bps) == 1:
        return bps
//将传入的生成版本字符串转换为整数类型，以便后续进行数值比较判断
    try:
        int_generation = int(generation)
        # Check if generation is in available generations
//检查转换后的生成版本数值是否在可用的版本列表中（这里限定为1、2、3）
        if int_generation in [1, 2, 3]:
            bps = [x for x in bps if int(x.get_attribute('generation')) == int_generation]
            return bps
        else:
            print("   Warning! Actor Generation is not valid. No actor will be spawned.")
            return []
    except:
        print("   Warning! Actor Generation is not valid. No actor will be spawned.")
        return []
# 主函数
def main():
    argparser = argparse.ArgumentParser(
        description=__doc__)
//添加一个名为'--host'的命令行参数，用于指定主机服务器的IP地址，默认值为'127.0.0.1'
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
// 添加一个名为'-p'（短格式）或'--port'（长格式）的命令行参数，用于指定要监听的TCP端口号，默认值为2000，类型为整数
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-n', '--number-of-vehicles',
        metavar='N',
        default=30,
        type=int,
        help='Number of vehicles (default: 30)')
//添加一个名为'-w'（短格式）或'--number-of-walkers'（长格式）的命令行参数，用于指定要生成的行人数量，默认值为10，类型为整数
    argparser.add_argument(
        '-w', '--number-of-walkers',
        metavar='W',
        default=10,
        type=int,
        help='Number of walkers (default: 10)')
    argparser.add_argument(
        '--safe',
        action='store_true',
        help='Avoid spawning vehicles prone to accidents')
// 添加一个名为'--seedw'的命令行参数，用于设置行人模块的种子，默认值为0，类型为整数
    argparser.add_argument(
        '--filterv',
        metavar='PATTERN',
        default='vehicle.*',
        help='Filter vehicle model (default: "vehicle.*")')
    argparser.add_argument(
        '--generationv',
        metavar='G',
        default='All',
        help='restrict to certain vehicle generation (values: "1","2","All" - default: "All")')
    argparser.add_argument(
        '--filterw',
        metavar='PATTERN',
        default='walker.pedestrian.*',
        help='Filter pedestrian type (default: "walker.pedestrian.*")')
    argparser.add_argument(
        '--generationw',
        metavar='G',
        default='2',
        help='restrict to certain pedestrian generation (values: "1","2","All" - default: "2")')
    argparser.add_argument(
        '--tm-port',
        metavar='P',
        default=8000,
        type=int,
        help='Port to communicate with TM (default: 8000)')
    argparser.add_argument(
        '--asynch',
        action='store_true',
        help='Activate asynchronous mode execution')
    argparser.add_argument(
        '--hybrid',
        action='store_true',
        help='Activate hybrid mode for Traffic Manager')
    argparser.add_argument(
        '-s', '--seed',
        metavar='S',
        type=int,
        help='Set random device seed and deterministic mode for Traffic Manager')
    argparser.add_argument(
        '--seedw',
        metavar='S',
        default=0,
        type=int,
        help='Set the seed for pedestrians module')
    argparser.add_argument(
        '--car-lights-on',
        action='store_true',
        default=False,
        help='Enable automatic car light management')
    argparser.add_argument(
        '--hero',
        action='store_true',
        default=False,
        help='Set one of the vehicles as hero')
//添加一个名为'--respawn'的命令行参数，当指定该参数时（action='store_true'），自动重新生成休眠的车辆（仅在大地图中有效），默认值为False
    argparser.add_argument(
        '--respawn',
        action='store_true',
        default=False,
        help='Automatically respawn dormant vehicles (only in large maps)')
    argparser.add_argument(
        '--no-rendering',
        action='store_true',
        default=False,
        help='Activate no rendering mode')

    args = argparser.parse_args()

    logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)
//用于存储生成的车辆的ID列表，初始化为空列表
    vehicles_list = []
// 用于存储生成的行人的相关信息（以字典形式，包含ID等）的列表，初始化为空列表
    walkers_list = []
//用于存储所有相关角色（车辆、行人等）的ID的列表，初始化为空列表
    all_id = []
// 创建一个Carla客户端对象，用于连接到Carla服务器，传入之前解析得到的主机IP地址和端口号参数
    client = carla.Client(args.host, args.port)
//设置客户端的超时时间为10.0秒，即如果在10秒内没有收到服务器响应，则认为操作超时
    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    synchronous_master = False
    random.seed(args.seed if args.seed is not None else int(time.time()))
//通过客户端获取Carla世界场景对象，后续所有与世界场景相关的操作（如获取地图、获取角色等）都基于这个对象进行
    try:
        world = client.get_world()

        traffic_manager = client.get_trafficmanager(args.tm_port)
        traffic_manager.set_global_distance_to_leading_vehicle(2.5)
        if args.respawn:
            traffic_manager.set_respawn_dormant_vehicles(True)
        if args.hybrid:
            traffic_manager.set_hybrid_physics_mode(True)
            traffic_manager.set_hybrid_physics_radius(70.0)
        if args.seed is not None:
            traffic_manager.set_random_device_seed(args.seed)

        settings = world.get_settings()
        if not args.asynch:
// 如果没有激活异步模式（即运行在同步模式下），则设置交通管理器为同步模式
            traffic_manager.set_synchronous_mode(True)
//如果当前世界场景的设置中不是同步模式，则将同步主控制者标记为True，表示当前脚本将作为同步模式的控制者进行相关设置和操作，同时将世界场景的同步模式设置为True，并设置固定的时间步长为0.05秒（用于同步更新世界场景等操作）
            if not settings.synchronous_mode:
                synchronous_master = True
                settings.synchronous_mode = True
                settings.fixed_delta_seconds = 0.05
            else:
                synchronous_master = False
        else:
            print("You are currently in asynchronous mode. If this is a traffic simulation, \
            you could experience some issues. If it's not working correctly, switch to synchronous \
            mode by using traffic_manager.set_synchronous_mode(True)")

        if args.no_rendering:
            settings.no_rendering_mode = True
        world.apply_settings(settings)

        blueprints = get_actor_blueprints(world, args.filterv, args.generationv)
        if not blueprints:
            raise ValueError("Couldn't find any vehicles with the specified filters")
        blueprintsWalkers = get_actor_blueprints(world, args.filterw, args.generationw)
        if not blueprintsWalkers:
            raise ValueError("Couldn't find any walkers with the specified filters")

        if args.safe:
            blueprints = [x for x in blueprints if x.get_attribute('base_type') == 'car']

        blueprints = sorted(blueprints, key=lambda bp: bp.id)

        spawn_points = world.get_map().get_spawn_points()
// 获取生成点的数量，用于后续判断要生成的角色数量是否超过了可用生成点数量等情况
        number_of_spawn_points = len(spawn_points)

        if args.number_of_vehicles < number_of_spawn_points:
            random.shuffle(spawn_points)
        elif args.number_of_vehicles > number_of_spawn_points:
            msg = 'requested %d vehicles, but could only find %d spawn points'
            logging.warning(msg, args.number_of_vehicles, number_of_spawn_points)
            args.number_of_vehicles = number_of_spawn_points

        # @todo cannot import these directly.
        SpawnActor = carla.command.SpawnActor
        SetAutopilot = carla.command.SetAutopilot
        FutureActor = carla.command.FutureActor

        # --------------
        # Spawn vehicles
        # --------------
        batch = []
        hero = args.hero
        for n, transform in enumerate(spawn_points):
            if n >= args.number_of_vehicles:
                break
            blueprint = random.choice(blueprints)
            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)
            if blueprint.has_attribute('driver_id'):
                driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
                blueprint.set_attribute('driver_id', driver_id)
            if hero:
                blueprint.set_attribute('role_name', 'hero')
                hero = False
            else:
                blueprint.set_attribute('role_name', 'autopilot')

            # spawn the cars and set their autopilot and light state all together
            batch.append(SpawnActor(blueprint, transform)
                .then(SetAutopilot(FutureActor, True, traffic_manager.get_port())))

        for response in client.apply_batch_sync(batch, synchronous_master):
            if response.error:
                logging.error(response.error)
            else:
                vehicles_list.append(response.actor_id)

        # Set automatic vehicle lights update if specified
        if args.car_lights_on:
            all_vehicle_actors = world.get_actors(vehicles_list)
            for actor in all_vehicle_actors:
                traffic_manager.update_vehicle_lights(actor, True)

        # -------------
        # Spawn Walkers
        # -------------
        # some settings
        percentagePedestriansRunning = 0.0      # how many pedestrians will run
        percentagePedestriansCrossing = 0.0     # how many pedestrians will walk through the road
        if args.seedw:
            world.set_pedestrians_seed(args.seedw)
            random.seed(args.seedw)
        # 1. take all the random locations to spawn
        spawn_points = []
        for i in range(args.number_of_walkers):
            spawn_point = carla.Transform()
            loc = world.get_random_location_from_navigation()
            if (loc != None):
                spawn_point.location = loc
                spawn_points.append(spawn_point)
        # 2. we spawn the walker object
        batch = []
        walker_speed = []
        for spawn_point in spawn_points:
            walker_bp = random.choice(blueprintsWalkers)
            # set as not invincible
            probability = random.randint(0,100 + 1);
            if walker_bp.has_attribute('is_invincible'):
                walker_bp.set_attribute('is_invincible', 'false')
            if walker_bp.has_attribute('can_use_wheelchair') and probability < 11:
                walker_bp.set_attribute('use_wheelchair', 'true')
            # set the max speed
            if walker_bp.has_attribute('speed'):
                if (random.random() > percentagePedestriansRunning):
                    # walking
                    walker_speed.append(walker_bp.get_attribute('speed').recommended_values[1])
                else:
                    # running
                    walker_speed.append(walker_bp.get_attribute('speed').recommended_values[2])
            else:
                print("Walker has no speed")
                walker_speed.append(0.0)
            batch.append(SpawnActor(walker_bp, spawn_point))
        results = client.apply_batch_sync(batch, True)
        walker_speed2 = []
        for i in range(len(results)):
            if results[i].error:
                logging.error(results[i].error)
            else:
                walkers_list.append({"id": results[i].actor_id})
                walker_speed2.append(walker_speed[i])
        walker_speed = walker_speed2
        # 3. we spawn the walker controller
        batch = []
        walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
        for i in range(len(walkers_list)):
            batch.append(SpawnActor(walker_controller_bp, carla.Transform(), walkers_list[i]["id"]))
        results = client.apply_batch_sync(batch, True)
        for i in range(len(results)):
            if results[i].error:
                logging.error(results[i].error)
            else:
                walkers_list[i]["con"] = results[i].actor_id
        # 4. we put together the walkers and controllers id to get the objects from their id
        for i in range(len(walkers_list)):
            all_id.append(walkers_list[i]["con"])
            all_id.append(walkers_list[i]["id"])
        all_actors = world.get_actors(all_id)

        # wait for a tick to ensure client receives the last transform of the walkers we have just created
        if args.asynch or not synchronous_master:
            world.wait_for_tick()
        else:
            world.tick()

        # 5. initialize each controller and set target to walk to (list is [controler, actor, controller, actor ...])
        # set how many pedestrians can cross the road
        world.set_pedestrians_cross_factor(percentagePedestriansCrossing)
        for i in range(0, len(all_id), 2):
            # start walker
            all_actors[i].start()
            # set walk to random point
            all_actors[i].go_to_location(world.get_random_location_from_navigation())
            # max speed
            all_actors[i].set_max_speed(float(walker_speed[int(i/2)]))

        print('spawned %d vehicles and %d walkers, press Ctrl+C to exit.' % (len(vehicles_list), len(walkers_list)))

        # Example of how to use Traffic Manager parameters
        traffic_manager.global_percentage_speed_difference(30.0)

        while True:
            if not args.asynch and synchronous_master:
                world.tick()
            else:
                world.wait_for_tick()

    finally:

        if not args.asynch and synchronous_master:
            settings = world.get_settings()
            settings.synchronous_mode = False
            settings.no_rendering_mode = False
            settings.fixed_delta_seconds = None
            world.apply_settings(settings)

        print('\ndestroying %d vehicles' % len(vehicles_list))
        client.apply_batch([carla.command.DestroyActor(x) for x in vehicles_list])

        # stop walker controllers (list is [controller, actor, controller, actor ...])
        for i in range(0, len(all_id), 2):
            all_actors[i].stop()

        print('\ndestroying %d walkers' % len(walkers_list))
        client.apply_batch([carla.command.DestroyActor(x) for x in all_id])

        time.sleep(0.5)

if __name__ == '__main__':   
    # 这是Python中用于判断当前模块是否作为主程序入口被运行的常用语句结构。
    # 当一个Python脚本直接被执行时（例如在命令行中运行该.py文件），Python会自动将该模块的__name__属性设置为'__main__'，此时下面代码块中的代码将会被执行。
    # 而如果该模块是被其他模块导入使用的话，__name__属性的值会是该模块自身的名称（即模块文件名去掉.py后缀后的名字），这段代码块则不会执行，这样可以避免在被导入时不必要的代码运行。

    try: 
        # 尝试调用名为'main'的函数，通常'main'函数是整个程序的核心逻辑所在，里面可能包含了诸如初始化操作、参数解析、主要业务逻辑的执行等一系列操作。
        # 如果'main'函数在执行过程中没有出现异常情况，程序就会按照'main'函数内部定义的逻辑顺序依次执行下去，直至该函数执行完毕或者遇到其他结束条件（比如函数内自己定义的循环结束、达到某个终止条件等）。
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
