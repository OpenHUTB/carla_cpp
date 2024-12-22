import carla
import random
import logging

# 获取CARLA客户端连接的世界对象，这是后续操作的基础，世界对象包含了场景中的各种元素，比如地图、车辆、行人等相关信息
world = client.get_world()

# 1. 选择行人蓝图
# 从世界对象的蓝图库中筛选出所有行人相关的蓝图（以"walker.pedestrian.*"模式匹配），这些蓝图定义了行人的各种属性和外观等信息
blueprintsWalkers = world.get_blueprint_library().filter("walker.pedestrian.*")
# 从筛选出的行人蓝图列表中随机选择一个作为后续生成行人的基础蓝图
walker_bp = random.choice(blueprintsWalkers)

# 2. 获取行人的随机生成位置
# 创建一个空列表，用于存放所有有效的行人生成位置信息（以carla.Transform对象表示，包含位置和姿态）
spawn_points = []
# 循环50次，尝试获取50个不同的行人生成位置
for _ in range(50):
    # 创建一个用于表示位置和姿态的变换对象，初始化为默认值
    spawn_point = carla.Transform()
    # 从世界的导航地图中获取一个随机位置，这个位置将作为行人的生成位置
    spawn_point.location = world.get_random_location_from_navigation()
    # 如果成功获取到了有效的随机位置（不为None），则将这个位置信息添加到生成位置列表中
    if spawn_point.location is not None:
        spawn_points.append(spawn_point)

# 3. 批量生成行人
# 创建一个空列表，用于存放批量生成行人的命令
batch_spawn_walkers = []
# 遍历每个获取到的有效生成位置
for spawn_point in spawn_points:
    # 为了增加生成行人的多样性，每次都重新从行人蓝图列表中随机选择一个蓝图作为当前要生成的行人蓝图
    walker_bp = random.choice(blueprintsWalkers)
    # 创建一个生成行人的命令，传入当前选择的行人蓝图和对应的生成位置信息
    spawn_command = carla.command.SpawnActor(walker_bp, spawn_point)
    # 将生成行人的命令添加到批量命令列表中
    batch_spawn_walkers.append(spawn_command)

# 应用批量生成行人的命令，并获取执行结果
results_spawn_walkers = client.apply_batch_sync(batch_spawn_walkers, True)
# 创建一个列表，用于存放已成功生成的行人信息（以字典形式，包含行人的actor_id）
walkers_list = []
# 遍历每个生成行人命令的执行结果
for index, result in enumerate(results_spawn_walkers):
    # 如果执行结果有错误信息，表示行人生成出现问题
    if result.error:
        # 使用日志记录模块记录下错误信息，方便后续排查问题
        logging.error(result.error)
    else:
        # 如果没有错误，说明行人成功生成，将包含该行人actor_id的字典添加到行人列表中
        walkers_list.append({"id": result.actor_id})

# 4. 批量生成行人AI控制器
# 创建一个空列表，用于存放批量生成行人AI控制器的命令
batch_spawn_controllers = []
# 从世界的蓝图库中查找名为"controller.ai.walker"的AI控制器蓝图，这个蓝图定义了控制行人行为的AI逻辑等信息
walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
# 遍历已成功生成的每个行人信息（通过walkers_list长度确定循环次数）
for walker_info in walkers_list:
    # 创建一个生成AI控制器的命令，传入控制器蓝图、默认的位置姿态变换对象（可能后续有其他关联位置的机制）以及对应的行人actor_id
    spawn_controller_command = carla.command.SpawnActor(walker_controller_bp, carla.Transform(), walker_info["id"])
    # 将生成AI控制器的命令添加到批量命令列表中
    batch_spawn_controllers.append(spawn_controller_command)

# 应用批量生成AI控制器的命令，并获取执行结果
results_spawn_controllers = client.apply_batch_sync(batch_spawn_controllers, True)
# 遍历每个生成AI控制器命令的执行结果，将对应的控制器actor_id关联到对应的行人信息字典中
for index, result in enumerate(results_spawn_controllers):
    if result.error:
        logging.error(result.error)
    else:
        walkers_list[index]["con"] = result.actor_id

# 5. 整理所有行人与控制器的ID，并获取对应的actor对象
# 创建一个列表，用于存放所有行人与控制器的actor_id，方便后续统一操作它们
all_id = []
# 遍历每个行人信息字典，将对应的控制器和行人的actor_id添加到all_id列表中
for walker_info in walkers_list:
    all_id.append(walker_info["con"])
    all_id.append(walker_info["id"])
# 通过世界对象获取所有对应actor_id的actor对象，后续可对这些对象进行操作
all_actors = world.get_actors(all_id)

# 等待一个时间步（tick），确保客户端接收到刚创建的行人的最新状态（比如位置等变换信息）
world.wait_for_tick()

# 6. 初始化每个行人AI控制器并设置行为
# 遍历所有的actor对象，步长为2是因为每两个对象依次为一个控制器和对应的行人
for index in range(0, len(all_actors), 2):
    # 获取当前的AI控制器对象
    controller = all_actors[index]
    # 获取当前控制器对应的行人对象
    walker = all_actors[index + 1]
    # 启动行人，使其开始活动（可能开始执行AI控制的行为等）
    controller.start()
    # 设置行人要前往的目标位置为世界中的一个随机位置（从导航地图获取），这样行人就会朝着该随机位置移动
    controller.go_to_location(world.get_random_location_from_navigation())
    # 为行人设置一个随机的最大速度，速度范围在1到2之间（默认速度可能是1.4 m/s，这里重新随机设置增加多样性）
    controller.set_max_speed(1 + random.random())
