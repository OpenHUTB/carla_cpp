#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
使用InvertedAI API生成交通的示例脚本
"""

import os  # 导入操作系统相关的功能模块
import time  # 导入时间处理相关的功能模块
import carla  # 导入CARLA模拟环境的API，用于车联网应用
import argparse  # 导入命令行参数解析模块
import logging  # 导入日志模块，用于记录日志信息
import math  # 引入数学函数的模块
import random  # 导入随机数生成模块
import invertedai as iai  # 导入invertedai库，并为其取别名为iai
from invertedai.common import AgentProperties, AgentState, TrafficLightState  # 从invertedai.common模块导入所需的类，用于智能体和交通灯状态管理

SpawnActor = carla.command.SpawnActor

#---------
# CARLA 工具
#---------

# 参数解析器
def argument_parser():
    # 创建一个 ArgumentParser 对象，用于解析命令行参数
    argparser = argparse.ArgumentParser(
        # 描述信息，通常使用 __doc__ 变量来提供模块级别的文档字符串
        description=__doc__)
    # 添加一个命令行参数 '--host'，用于指定服务器的IP地址
    argparser.add_argument(
        '--host',
        # 指定参数的占位符
        metavar='H',
        # 设置参数的默认值为 '127.0.0.1'
        default='127.0.0.1',
        # 提供参数的帮助信息，说明其作用是设置主机服务器的IP地址
        help='IP of the host server (default: 127.0.0.1)')
    # 添加一个命令行参数 '--port' 或 '-p'，用于指定TCP端口
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        # 提供参数的帮助信息，说明其作用是设置监听的TCP端口
        help='TCP port to listen to (default: 2000)')
    # 添加一个命令行参数 '--number-of-vehicles' 或 '-n'，用于指定生成的车辆数量
    argparser.add_argument(
        '-n', '--number-of-vehicles',
        metavar='N',
        default=30,
        type=int,
        # 提供参数的帮助信息，说明其作用是设置由InvertedAI生成的车辆数量
        help='Number of vehicles spawned by InvertedAI (default: 30)')
    # 添加一个命令行参数 '--number-of-walkers' 或 '-w'，用于指定生成的行人数量
    argparser.add_argument(
        '-w', '--number-of-walkers',
        metavar='W',
        default=10,
        type=int,
        # 提供参数的帮助信息，说明其作用是设置行人的数量
        help='Number of walkers (default: 10)')
    # 添加一个命令行参数 '--safe'，用于指定是否避免生成容易发生事故的车辆
    argparser.add_argument(
        '--safe',
        # 指定参数的类型为布尔值
        type=bool,
        # 设置参数的默认值为 True
        default=True,
        # 提供参数的帮助信息，说明其作用是避免生成容易发生事故的车辆
        help='Avoid spawning vehicles prone to accidents (default True)')
    # 添加一个命令行参数 '--filterv'，用于指定车辆模型的过滤模式
    argparser.add_argument(
        '--filterv',
        metavar='PATTERN',
        default='vehicle.*',
        # 提供参数的帮助信息，说明其作用是过滤车辆模型
        help='Filter vehicle model (default: "vehicle.*")')
    # 添加一个命令行参数 '--generationv'，用于限制车辆的代数
    argparser.add_argument(
        '--generationv',
        metavar='G',
        default='All',
        # 提供参数的帮助信息，说明其作用是限制车辆的代数
        help='restrict to certain vehicle generation (default: "All")')
    argparser.add_argument(
        '--filterw',
        metavar='PATTERN',
        default='walker.pedestrian.*',
        help='Filter pedestrian type (default: "walker.pedestrian.*")')
    argparser.add_argument(
        '--generationw',
        metavar='G',
        default='All',
        help='restrict to certain pedestrian generation (default: "All")')
    argparser.add_argument(
        '-s', '--seed',
        metavar='S',
        type=int,
        help='Set random seed')
    argparser.add_argument(
        '--hero',
        action='store_true',
        default=False,
        help='Set one of the vehicles as hero')
    argparser.add_argument(
        '--iai-key',
        type=str,
        help="InvertedAI API key.")
    argparser.add_argument(
        '--record',
        action='store_true',
        help="Record the simulation using the CARLA recorder",
        default=False)
    argparser.add_argument(
        '--sim-length',
        type=int,
        default=120,
        help="Length of the simulation in seconds (default: 120)")
    argparser.add_argument(
        '--location',
        type=str,
        help=f"IAI formatted map on which to create simulate (default: carla:Town10HD, only tested there)",
        default='carla:Town10HD')
    argparser.add_argument(
        '--capacity',
        type=int,
        help=f"The capacity parameter of a quadtree leaf before splitting (default: 100)",
        default=100)
    argparser.add_argument(
        '--width',
        type=int,
        help=f"Full width of the area to initialize (default: 250)",
        default=250)
    argparser.add_argument(
        '--height',
        type=int,
        help=f"Full height of the area to initialize (default: 250)",
        default=250)
    argparser.add_argument(
        '--map-center',
        type=int,
        nargs='+',
        help=f"Center of the area to initialize (default: [0,30])",
        default=tuple([0,30]))
    argparser.add_argument(
        '--iai-async',
        type=bool,
        help=f"Whether to call drive asynchronously (default: True)",
        default=True)
    argparser.add_argument(
        '--api-model',
        type=str,
        help=f"IAI API model version (default: bI5p)",
        default="bI5p")
    argparser.add_argument(
        '--iai-log',
        action="store_true",
        help=f"Export a log file for the InvertedAI cosimulation, which can be replayed afterwards")

    args = argparser.parse_args()

    return args

# 设置CARLA客户端和世界
def setup_carla_environment(host, port):

    step_length = 0.1 #  0.1是目前invertedai支持的唯一步长

    client = carla.Client(host, port)
    client.set_timeout(200.0)

    #配置模拟环境
    world = client.get_world()
    world_settings = carla.WorldSettings(
        synchronous_mode=True,
        fixed_delta_seconds=step_length,
    )
    world.apply_settings(world_settings)

    return client, world

# 设置观众视角到相应车辆
def set_spectator(world, hero_v):

    # 定义观察者相对于车辆的位置偏移量
    spectator_offset_x = -6.# X轴偏移量
    spectator_offset_z = 6.# Z轴偏移量
    spectator_offset_pitch = 20# 观察者俯仰角偏移量
    
    # 获取对应车辆的变换信息，包括位置和旋转
    hero_t = hero_v.get_transform()

    # 获取对应车辆的偏航角
    yaw = hero_t.rotation.yaw
    # 计算观察者的位置，基于英雄车辆的位置和偏移量
    spectator_l = hero_t.location + carla.Location(
        spectator_offset_x * math.cos(math.radians(yaw)), # 计算X轴上的偏移
        spectator_offset_x * math.sin(math.radians(yaw)), # 计算Y轴上的偏移
        spectator_offset_z,# Z轴上的偏移量
    )
     # 创建观察者的变换信息，包括位置和旋转
    spectator_t = carla.Transform(spectator_l, hero_t.rotation)
    # 调整观察者的俯仰角
    spectator_t.rotation.pitch -= spectator_offset_pitch
    # 设置世界中的观察者变换，使观察者视角跟随英雄车辆
    world.get_spectator().set_transform(spectator_t)

#---------
# 初始化参与者
#---------

# 从CARLA参与者中初始化逆向代理
def initialize_iai_agent(actor, agent_type):

    transf = actor.get_transform()
    vel = actor.get_velocity()
    speed = math.sqrt(vel.x**2. + vel.y**2. +vel.z**2.)

    agent_state = AgentState.fromlist([
                                        transf.location.x,
                                        transf.location.y,
                                        transf.rotation.yaw,
                                        speed
                                    ])

    bb = actor.bounding_box
    length, width = bb.extent.x*2, bb.extent.y*2

    agent_properties = AgentProperties(length=length, width=width, agent_type=agent_type)
    if agent_type=="car":
        agent_properties.rear_axis_offset = length*0.38 # 从InvertedAI初始化中拟合的经验值

    return agent_state, agent_properties

# 从CARLA参与者中初始化逆向行人
def initialize_pedestrians(pedestrians):

    iai_pedestrians_states, iai_pedestrians_properties = [], []
    for actor in pedestrians:
        iai_ped_state, iai_ped_properties = initialize_iai_agent(actor,agent_type="pedestrian")
        iai_pedestrians_states.append(iai_ped_state)
        iai_pedestrians_properties.append(iai_ped_properties)

    return iai_pedestrians_states, iai_pedestrians_properties

# 在模拟中生成由CARLA控制器驱动的行人（不是由invertedai驱动）
def spawn_pedestrians(client, world, num_pedestrians, bps):

    batch = []

    # 获取行人的生成点
    spawn_points = []
    for i in range(num_pedestrians):
        
        loc = world.get_random_location_from_navigation()
        if (loc is not None):
            spawn_point = carla.Transform(location=loc)
            #Apply Offset in vertical to avoid collision spawning
            spawn_point.location.z += 1
            spawn_points.append(spawn_point)

    pedestrians = []
    walkers_list = []

    # 生成行人
    for i in range(len(spawn_points)):
        walker_bp = random.choice(bps)
        if walker_bp.has_attribute('is_invincible'):
            walker_bp.set_attribute('is_invincible', 'false')
        spawn_point = spawn_points[i]
        batch.append(SpawnActor(walker_bp, spawn_point))

    results = client.apply_batch_sync(batch, True)
    pedestrians = world.get_actors().filter('walker.*')
    for i in range(len(results)):
        if results[i].error:
            logging.error(results[i].error)
        else:
            walkers_list.append({"id": results[i].actor_id})

    # 生成CARLA IA控制器用于行人
    batch = []
    walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
    for i in range(len(walkers_list)):
        batch.append(SpawnActor(walker_controller_bp, carla.Transform(), walkers_list[i]["id"]))
    results = client.apply_batch_sync(batch, True)

    world.tick()

    for controller in world.get_actors().filter('controller.ai.walker'):
        controller.start()
        dest = world.get_random_location_from_navigation()
        controller.go_to_location(dest)
        controller.set_max_speed(0.5 + random.random())

    return pedestrians

# 根据给定的过滤器获取
def get_actor_blueprints(world, filter, generation):
    bps = world.get_blueprint_library().filter(filter)

    if generation.lower() == "all":
        return bps

    # 根据给定的过滤器获取
    # 因此，我们忽略代
    if len(bps) == 1:
        return bps

    try:
        int_generation = int(generation)
        # 检查代是否在可用代中
        if int_generation in [1, 2, 3, 4]:
            bps = [x for x in bps if int(x.get_attribute('generation')) == int_generation]
            return bps
        else:
            print("   Warning! Actor Generation is not valid. No actor will be spawned.")
            return []
    except:
        print("   Warning! Actor Generation is not valid. No actor will be spawned.")
        return []

#---------
# InvertedAI - CARLA 同步例程
#---------

# 从 InvertedAI 转换获取 CARLA 转换
def transform_iai_to_carla(agent_state):
    agent_transform = carla.Transform(
        carla.Location(
            agent_state.center.x,
            agent_state.center.y,
            0.
        ),
        carla.Rotation(
            yaw=math.degrees(agent_state.orientation)
        )
    )

    return agent_transform

# 更新由 IAI 驱动的 CARLA 代理的转换并推进世界
def update_transforms(iai2carla,response):
    """
    推进 carla 模拟一个时间步
    假设 carla_actors 是由 IAI 控制的 carla 参与者列表
    """
    for agent_id in iai2carla.keys():
        agentdict = iai2carla[agent_id]
        if agentdict["is_iai"]:            
            agent = response.agent_states[agent_id]
            agent_transform = transform_iai_to_carla(agent)
            try:     
                actor = agentdict["actor"]
                actor.set_transform(agent_transform)
            except:
                pass

# 将现有的 IAI 代理分配给 CARLA 车辆蓝图，并将这些代理添加到 CARLA 模拟中
def assign_carla_blueprints_to_iai_agents(world,vehicle_blueprints,agent_properties,agent_states,recurrent_states,is_iai,noniai_actors):

    agent_properties_new = []
    agent_states_new = []
    recurrent_states_new = []
    new_agent_id = 0
    iai2carla = {}

    for agent_id, state in enumerate(agent_states):

        if not is_iai[agent_id]:
            agent_properties_new.append(agent_properties[agent_id])
            agent_states_new.append(agent_states[agent_id])
            recurrent_states_new.append(recurrent_states[agent_id])
            actor = noniai_actors[agent_id]
            new_agent_id += 1
            iai2carla[len(iai2carla)] = {"actor":actor, "is_iai":False, "type":agent_properties[agent_id].agent_type}
             
        else:

            blueprint = random.choice(vehicle_blueprints)
            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)
            agent_transform = transform_iai_to_carla(state)

            actor = world.try_spawn_actor(blueprint,agent_transform)
            
            if actor is not None:
                bb = actor.bounding_box.extent

                agent_attr = agent_properties[agent_id]

                agent_attr.length = 2*bb.x
                agent_attr.width = 2*bb.y
                agent_attr.rear_axis_offset = 2*bb.x/3

                new_agent_id += 1

                agent_properties_new.append(agent_attr)
                agent_states_new.append(agent_states[agent_id])
                recurrent_states_new.append(recurrent_states[agent_id])

                actor.set_simulate_physics(False)

                iai2carla[len(iai2carla)] = {"actor":actor, "is_iai":True, "type":agent_properties[agent_id].agent_type}

    if len(agent_properties_new) == 0:
        raise Exception("No vehicles could be placed in Carla environment.")
    
    return agent_properties_new, agent_states_new, recurrent_states_new, iai2carla

# 初始化 InvertedAI 协同模拟
def initialize_simulation(args, world, agent_states=None, agent_properties=None):

    iai_seed = args.seed if args.seed is not None else random.randint(1,10000)
    traffic_lights_states, carla2iai_tl = initialize_tl_states(world)

    #################################################################################################
    # 初始化逆向智能代理
    map_center = args.map_center
    print(f"Call location info.")
    location_info_response = iai.location_info(
        location = args.location,
        rendering_center = map_center
    )
    print(f"Begin initialization.") 
    # 获取一个100x100米区域的网格，以便初始化逆向的车辆。
    regions = iai.get_regions_default(
        location = args.location,
        total_num_agents = args.number_of_vehicles,
        area_shape = (int(args.width/2),int(args.height/2)),
        map_center = map_center, 
    )
    # 在指定区域内放置车辆时，需要考虑附近区域车辆的相对状态。
    response = iai.large_initialize(
        location = args.location,
        regions = regions,
        traffic_light_state_history = [traffic_lights_states],
        agent_states = agent_states,
        agent_properties = agent_properties,
        random_seed = iai_seed
    )

    return response, carla2iai_tl, location_info_response

#---------
#同步 InvertedAI 和 CARLA 交通灯
#---------

# CARLA 和 IAI 交通灯 ID 之间的映射
def get_traffic_lights_mapping(world):
    tls = world.get_actors().filter('traffic.traffic_light*')
    tl_ids = sorted([tl.id for tl in list(tls)])
    carla2iai_tl = {}
    # IAI交通灯的ID，目前仅适用于Town10地图（包括UE4和UE5版本的地图）。
    iai_tl_id = 4364
    for carla_tl_id in tl_ids:
        carla2iai_tl[str(carla_tl_id)] = [str(iai_tl_id), str(iai_tl_id+1000)]
        iai_tl_id+=1

    return carla2iai_tl

# 根据 CARLA 交通灯状态返回 IAI 交通灯状态
def get_traffic_light_state_from_carla(carla_tl_state):

    if carla_tl_state == carla.TrafficLightState.Red:
        return TrafficLightState.red

    elif carla_tl_state == carla.TrafficLightState.Yellow:
        return TrafficLightState.yellow

    elif carla_tl_state == carla.TrafficLightState.Green:
        return TrafficLightState.green

    else:  # 未知状态，关闭交通灯。
        return TrafficLightState.Off

# 根据 CARLA 交通灯分配 IAI 交通灯
def assign_iai_traffic_lights_from_carla(world, iai_tl, carla2iai_tl):

    traffic_lights = world.get_actors().filter('traffic.traffic_light*')
    
    carla_tl_dict = {}
    for tl in traffic_lights:
        carla_tl_dict[str(tl.id)]=tl.state

    for carla_tl_id, carla_state in carla_tl_dict.items():
        iai_tl_id_pair = carla2iai_tl[carla_tl_id]
        for iai_tl_id in iai_tl_id_pair:
            iai_tl[iai_tl_id] = get_traffic_light_state_from_carla(carla_state)

    return iai_tl

# 初始化交通灯状态
def initialize_tl_states(world):
    carla2iai_tl = get_traffic_lights_mapping(world)
    iai_tl_states = {}
    for tlpair in carla2iai_tl.values():
        for tl in tlpair:
            iai_tl_states[tl] = TrafficLightState.red # Initialize to given value

    iai_tl_states = assign_iai_traffic_lights_from_carla(world, iai_tl_states, carla2iai_tl)
    return iai_tl_states, carla2iai_tl

#---------
# 主函数
#---------
def main():

    args = argument_parser()

    # 设置CARLA客户端和世界。
    client, world = setup_carla_environment(args.host, args.port)

    # 指定IAI API密钥
    try:
        iai.add_apikey(args.iai_key)  
    except:
        print("\n\tYou need to indicate the InvertedAI API key with the argument --iai-key. To obtain one, please go to https://www.inverted.ai \n")

    num_pedestrians = args.number_of_walkers

    FPS = int(1./world.get_settings().fixed_delta_seconds)

    if args.record:
        logfolder = os.getcwd()+"/logs/"
        if not os.path.exists(logfolder):
            os.system("mkdir "+logfolder)
        logfile = logfolder+"carla_record.log"
        client.start_recorder(logfile)
        print("Recording on file: %s" % logfile)

    seed = args.seed

    if seed:
        random.seed(seed)
    
    vehicle_blueprints = get_actor_blueprints(world, args.filterv, args.generationv)
    if args.safe:
        vehicle_blueprints = [x for x in vehicle_blueprints if x.get_attribute('base_type') == 'car']   

    agent_states, agent_properties = [], []
    is_iai = []
    noniai_actors = []
        
    # 添加行人（不受IAI控制）。
    if num_pedestrians>0:
        if seed:
            world.set_pedestrians_seed(seed)
        blueprintsWalkers = get_actor_blueprints(world, args.filterw, args.generationw)
        if not blueprintsWalkers:
            raise ValueError("Couldn't find any walkers with the specified filters")
        pedestrians = spawn_pedestrians(client, world, num_pedestrians, blueprintsWalkers)
        iai_pedestrians_states, iai_pedestrians_properties = initialize_pedestrians(pedestrians)
        agent_states.extend(iai_pedestrians_states)
        agent_properties.extend(iai_pedestrians_properties)
        is_iai.extend( [False]*len(iai_pedestrians_states) )
        noniai_actors.extend(pedestrians)
    
    else:
        pedestrians = []
    
    num_noniai = len(agent_properties)

    # 初始化InvertedAI协同仿真。
    response, carla2iai_tl, location_info_response = initialize_simulation(args, world, agent_states=agent_states, agent_properties=agent_properties)
    agent_properties = response.agent_properties
    is_iai.extend( [True]*(len(agent_properties)-num_noniai) )

    # 编写InvertedAI日志文件，之后可以打开它来可视化gif图像并进行进一步分析。
    # 查看这里的例子：https://github.com/inverted-ai/invertedai/blob/master/examples/scenario_log_example.py
    if args.iai_log:

        log_writer = iai.LogWriter()
        log_writer.initialize(
            location=args.location,
            location_info_response=location_info_response,
            init_response=response
        )
        iailog_path = os.path.join(os.getcwd(),f"iai_log.json")

    # 将IAI代理映射到CARLA角色，并更新响应属性和状态。
    agent_properties, agent_states_new, recurrent_states_new, iai2carla = assign_carla_blueprints_to_iai_agents(world,vehicle_blueprints,agent_properties,response.agent_states,response.recurrent_states,is_iai,noniai_actors)
    traffic_lights_states = assign_iai_traffic_lights_from_carla(world,response.traffic_lights_states, carla2iai_tl)
    response.agent_states = agent_states_new
    response.recurrent_states = recurrent_states_new
    response.traffic_lights_states = traffic_lights_states

    # 执行第一次CARLA模拟刻。
    world.tick()

    try:

        vehicles = world.get_actors().filter('vehicle.*')
        print("Total number of agents:",len(agent_properties),"Vehicles",len(vehicles), "Pedestrians:",len(pedestrians))

        # 获取主车辆。
        hero_v = None
        if args.hero:
            hero_v = vehicles[0]

        for frame in range(args.sim_length * FPS):

            response.traffic_lights_states = assign_iai_traffic_lights_from_carla(world, response.traffic_lights_states, carla2iai_tl)

            # IAI更新步骤。
            response = iai.large_drive(
                location = args.location,
                agent_states = response.agent_states,
                agent_properties = agent_properties,
                recurrent_states = response.recurrent_states,
                traffic_lights_states = response.traffic_lights_states,
                light_recurrent_states = None,
                single_call_agent_limit = args.capacity,
                async_api_calls = args.iai_async,
                api_model_version = args.api_model,
                random_seed = seed
            )

            if args.iai_log:
                log_writer.drive(drive_response=response)

            # 用IAI代理的新变换更新CARLA参与者
            update_transforms(iai2carla,response)

            # 执行CARLA模拟刻。
            world.tick()

            # 在IAI协同仿真中更新逆向控制的代理，例如行人。
            for agent_id in iai2carla.keys():
                agentdict = iai2carla[agent_id]

                if not agentdict["is_iai"]:
                    actor = agentdict["actor"]
                    state, properties = initialize_iai_agent(actor, agentdict["type"])
                    response.agent_states[agent_id] = state
                    agent_properties[agent_id] = properties

            # 包括来自其他客户端的可能的新角色（车辆），例如使用`automatic_control.py`或`manual_control.py`。
            actors_all = world.get_actors().filter('vehicle.*')
            actsids = [act["actor"].id for act in iai2carla.values()]
            for actor in actors_all:
                if not (actor.id in actsids):
                    state, properties = initialize_iai_agent(actor, "car")
                    response.agent_states.append( state )
                    agent_properties.append( properties )
                    response.recurrent_states.append( response.recurrent_states[-1] )   # temporal fix
                    iai2carla[len(iai2carla)] = {"actor":actor, "is_iai":False, "type":properties.agent_type}

            # 如果存在主车辆，则更新观众视角。
            if hero_v is not None:
                set_spectator(world, hero_v)

            

    finally:

        vehicles_list = world.get_actors().filter('vehicle.*')
        print('\ndestroying %d vehicles' % len(vehicles_list))
        client.apply_batch([carla.command.DestroyActor(x) for x in vehicles_list])

        walkercontrollers_list = world.get_actors().filter('controller.*')
        for control in walkercontrollers_list:
            control.stop()
            control.destroy()

        walkers_list = world.get_actors().filter('walker.*')
        print('\ndestroying %d walkers' % len(walkers_list))
        client.apply_batch([carla.command.DestroyActor(x) for x in walkers_list])

        time.sleep(0.5)

        if args.record:
            client.stop_recorder()

        if args.iai_log:
            log_writer.export_to_file(log_path=iailog_path)

if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
