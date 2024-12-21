# 0. 选择行人（walkers）的蓝图
world = client.get_world()
blueprintsWalkers = world.get_blueprint_library().filter("walker.pedestrian.*")
walker_bp = random.choice(blueprintsWalkers)

# 1. 获取所有用于生成行人的随机位置
spawn_points = []
for i in range(50):
    spawn_point = carla.Transform()
    spawn_point.location = world.get_random_location_from_navigation()
    if (spawn_point.location != None):
        spawn_points.append(spawn_point)

# 2. 构建用于生成行人的批量命令
batch = []
for spawn_point in spawn_points:
    walker_bp = random.choice(blueprintsWalkers)
    batch.append(carla.command.SpawnActor(walker_bp, spawn_point))

# 2.1 执行批量命令来生成行人
results = client.apply_batch_sync(batch, True)
for i in range(len(results)):
    if results[i].error:
        logging.error(results[i].error)
    else:
        walkers_list.append({"id": results[i].actor_id})

# 3. 为每个行人生成AI控制器
batch = []
walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
for i in range(len(walkers_list)):
    batch.append(carla.command.SpawnActor(walker_controller_bp, carla.Transform(), walkers_list[i]["id"]))

# 3.1 执行批量命令来生成AI控制器
results = client.apply_batch_sync(batch, True)
for i in range(len(results)):
    if results[i].error:
        logging.error(results[i].error)
    else:
        walkers_list[i]["con"] = results[i].actor_id
        
4. 整合行人与控制器的id信息
for i in range(len(walkers_list)):
    all_id.append(walkers_list[i]["con"])
    all_id.append(walkers_list[i]["id"])
all_actors = world.get_actors(all_id)

# 等待世界进行一次更新（tick）操作，确保客户端接收到我们刚刚创建的行人的最新位置等变换信息
world.wait_for_tick()

# 5. 初始化每个控制器，并设置目标行走位置
for i in range(0, len(all_actors), 2):
    # 启动行人
    all_actors[i].start()
    # 设置行人行走的目标位置
    all_actors[i].go_to_location(world.get_random_location_from_navigation())
    # 设置行人的随机最大速度
    all_actors[i].set_max_speed(1 + random.random())    # max speed between 1 and 2 (default is 1.4 m/s)
