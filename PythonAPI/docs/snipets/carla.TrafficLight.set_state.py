# （这段注释说明了这段代码的目的：是要改变影响车辆的交通信号灯颜色，使其从红色变为绿色，实现方式是检测车辆角色是否处于交通信号灯处。）
# This recipe changes from red to green the traffic light that affects the vehicle. 
# This is done by detecting if the vehicle actor is at a traffic light.

# ...
# 获取游戏或模拟世界的实例，这里的 'client' 应该是与该世界进行交互的客户端对象，通过它可以获取到整个世界的相关信息，
# 比如场景中的各种实体、地图信息等，后续操作都基于这个获取到的世界对象展开。
world = client.get_world()
spectator = world.get_spectator()
# 从世界的蓝图库（blueprint_library）中筛选出所有名称以 'vehicle.bmw.*' 模式匹配的车辆蓝图（blueprint），
# 蓝图在这里可以理解为创建某种类型实体的模板，包含了车辆的各种属性、外观等定义信息。然后使用 random.choice 随机选择其中一个宝马品牌相关的车辆蓝图，
# 用于后续创建具体的车辆实例，这样每次运行代码时创建的车辆类型可能会有所不同（只要蓝图库中有多个符合条件的蓝图）。
vehicle_bp = random.choice(world.get_blueprint_library().filter('vehicle.bmw.*'))
# 从世界的地图对象中获取所有的生成点（spawn points），生成点通常是地图上预先定义好的可以放置实体（如车辆）的位置，
# 然后同样使用 random.choice 随机选择其中一个生成点，这个生成点将作为即将创建的车辆的初始位置。
transform = random.choice(world.get_map().get_spawn_points())
vehicle = world.try_spawn_actor(vehicle_bp, transform)

# 等待世界进行一次更新（tick）操作，确保世界状态已经将车辆这个新的参与者纳入考虑范围
world.tick()

world_snapshot = world.wait_for_tick()
actor_snapshot = world_snapshot.find(vehicle.id)

# 将观察者（spectator）的位置和朝向设置为给定的变换信息（这里使用的是车辆的变换信息）
spectator.set_transform(actor_snapshot.get_transform())
# ...# ...
if vehicle_actor.is_at_traffic_light():
    traffic_light = vehicle_actor.get_traffic_light()
    if traffic_light.get_state() == carla.TrafficLightState.Red:
       # 提示交通信号灯已改变，可以通行
        traffic_light.set_state(carla.TrafficLightState.Green)
# ...

