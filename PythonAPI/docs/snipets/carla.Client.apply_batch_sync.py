# 0. 为行人选择蓝图​
# 获取客户端连接的当前世界对象，这个世界对象包含了场景中的各种元素，如地图、车辆、行人等，是后续操作的基础。​
world = client.get_world() ​
# 从世界对象的蓝图库中筛选出所有与行人相关的蓝图，使用 "walker.pedestrian.*" 作为过滤器，​
# 这样可以获取到各种类型的行人模型蓝图，为后续创建不同外观、行为的行人提供选择。​
blueprintsWalkers = world.get_blueprint_library().filter("walker.pedestrian.*") ​
# 从筛选出的行人蓝图列表中随机选择一个作为要创建的行人的蓝图模板，这样每次运行代码时创建的行人可能具有不同的外观或属性。​
walker_bp = random.choice(blueprintsWalkers) ​
​
# 1. 获取所有随机生成点​
# 初始化一个空列表，用于存储后续生成行人的位置信息，这些位置将决定行人在仿真场景中的初始出现地点。​
spawn_points = [] ​
# 循环 50 次，目的是生成 50 个行人的生成位置（如果都有效）。这里可以根据实际需求调整数量。​
for i in range(50): ​
    # 创建一个用于表示位置和姿态的变换对象，初始化为默认值，后续会为其赋予具体的位置信息。​
    spawn_point = carla.Transform() ​
    # 从世界对象的导航系统中获取一个随机的位置，这个位置通常是在可通行的道路或人行道等区域，​
    # 确保生成的行人初始位置是合理的，符合现实场景逻辑。​
    spawn_point.location = world.get_random_location_from_navigation() ​
    # 检查获取到的随机位置是否有效，即不为 None，只有有效的位置才会被添加到生成点列表中，​
    # 避免在无效位置尝试生成行人导致错误。​
    if (spawn_point.location!= None): ​
        spawn_points.append(spawn_point) ​
​
# 2. 构建生成行人的命令批次​
# 初始化一个空列表，用于存储即将要执行的生成行人的命令，这些命令会一次性批量发送给服务器执行。​
batch = [] ​
# 遍历之前获取的所有有效生成点​
for spawn_point in spawn_points: ​
    # 再次随机选择一个行人蓝图，这里每次循环重新选择可以增加生成行人的多样性，​
    # 比如不同外观、服饰的行人可能会被随机创建。​
    walker_bp = random.choice(blueprintsWalkers) ​
    # 使用选定的行人蓝图和对应的生成点构建一个生成行人的命令，并添加到命令批次列表中，​
    # 这个命令告诉服务器在指定位置创建指定类型的行人。​
    batch.append(carla.command.SpawnActor(walker_bp, spawn_point)) ​
​
# 2.1 执行命令批次​
# 将构建好的生成行人的命令批次发送给客户端，并要求同步执行，即等待所有命令执行完成后再返回结果。​
# True 参数表示同步执行模式，这样可以确保在继续后续操作之前，行人已经被成功创建。​
results = client.apply_batch_sync(batch, True) ​
# 遍历命令执行的结果列表，检查每个结果是否有错误信息。​
for i in range(len(results)): ​
    # 如果某个结果包含错误信息，使用日志记录模块记录下错误详细内容，以便调试时排查问题，​
    # 例如可能是由于资源不足、位置冲突等原因导致行人创建失败。​
    if results[i].error: ​
        logging.error(results[i].error) ​
    # 如果结果没有错误，说明行人成功创建，将创建的行人的唯一标识符（actor_id）添加到一个列表中，​
    # 这个列表用于后续关联行人及其控制器等操作。​
    else: ​
        walkers_list.append({"id": results[i].actor_id}) ​
​
# 3. 为每个行人生成 AI 控制器​
# 初始化一个空列表，用于存储生成行人 AI 控制器的命令，这些控制器将赋予行人智能行为，使其能在场景中自主移动等。​
batch = [] ​
# 从世界对象的蓝图库中查找特定的行人 AI 控制器蓝图，这里使用 'controller.ai.walker' 作为标识，​
# 找到的这个蓝图将用于创建控制行人行为的控制器。​
walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker') ​
# 遍历之前成功创建行人的列表，为每个行人创建对应的 AI 控制器。​
for i in range(len(walkers_list)): ​
    # 使用找到的控制器蓝图、默认的变换（位置和姿态）信息以及对应的行人标识符构建生成控制器的命令，​
    # 并添加到命令批次列表中，这个命令告诉服务器为指定的行人创建对应的控制器。​
    batch.append(carla.command.SpawnActor(walker_controller_bp, carla.Transform(), walkers_list[i]["id"])) ​
​
# 3.1 执行命令批次​
# 再次将生成控制器的命令批次发送给客户端并同步执行，确保控制器都能成功创建。​
results = client.apply_batch_sync(batch, True) ​
# 遍历控制器创建结果列表，检查是否有错误。​
for i in range(len(results)): ​
    # 如果有错误，记录错误信息以便排查。​
    if results[i].error: ​
        logging.error(results[i].error) ​
    # 如果没有错误，将创建的控制器的标识符与对应的行人标识符关联起来，存储在之前的行人列表中，​
    # 方便后续统一管理和操作行人和控制器。​
    else: ​
        walkers_list[i]["con"] = results[i].actor_id ​
​
# 4. 汇总行人与控制器标识符​
# 遍历之前存储行人信息的列表，将每个行人的控制器标识符和行人自身标识符依次添加到一个新的列表中，​
# 这个新列表将用于获取所有相关的 actor（行人和控制器）对象。​
for i in range(len(walkers_list)): ​
    all_id.append(walkers_list[i]["con"]) ​
    all_id.append(walkers_list[i]["id"]) ​
# 根据汇总的标识符列表，从世界对象中获取对应的所有 actor（行人和控制器）对象，​
# 这些对象将用于后续直接操作行人的行为，如启动、设置目标等。​
all_actors = world.get_actors(all_id) ​
​
# 等待一个时间步，确保客户端接收到刚刚创建的行人的最新状态（位置、姿态等变换信息），​
# 这一步很重要，因为在创建行人及其控制器后，需要给服务器一些时间来更新相关状态信息，​
# 以便后续操作基于最新的状态进行，避免出现不一致的情况。​
world.wait_for_tick() ​
​
# 5. 初始化每个控制器并设置行走目标​
# 以步长为 2 遍历所有获取到的 actor（行人和控制器交替排列），因为每两个连续的 actor 分别是一个控制器和对应的行人。​
for i in range(0, len(all_actors), 2): ​
    # 启动对应的行人，使其开始按照控制器设定的逻辑行动，例如开始行走、避让等，​
    # 这一步激活了行人的自主行为能力。​
    all_actors[i].start() ​
    # 为行人设置一个随机的行走目标点，这个目标点是从世界的导航系统中随机获取的，​
    # 使得每个行人都有不同的行走方向，模拟现实中行人的随机性，走向不同的目的地。​
    all_actors[i].go_to_location(world.get_random_location_from_navigation()) ​
    # 为行人设置一个随机的最大速度，速度范围在 1 到 2 之间（默认速度是 1.4 m/s），​
    # 通过随机设置速度，进一步增加行人行为的多样性，使其看起来更像真实场景中的不同个体。​
    all_actors[i].set_max_speed(1 + random.random())
