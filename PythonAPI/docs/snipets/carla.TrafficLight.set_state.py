# （这段注释说明了这段代码的目的：是要改变影响车辆的交通信号灯颜色，使其从红色变为绿色，实现方式是检测车辆角色是否处于交通信号灯处。）
# This recipe changes from red to green the traffic light that affects the vehicle. 
# This is done by detecting if the vehicle actor is at a traffic light.

# ...
# 获取游戏或模拟世界的实例，这里的 'client' 应该是与该世界进行交互的客户端对象，通过它可以获取到整个世界的相关信息，
# 比如场景中的各种实体、地图信息等，后续操作都基于这个获取到的世界对象展开。
world = client.get_world()
spectator = world.get_spectator()

vehicle_bp = random.choice(world.get_blueprint_library().filter('vehicle.bmw.*'))
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

