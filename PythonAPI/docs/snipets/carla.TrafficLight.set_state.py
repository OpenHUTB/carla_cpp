
# This recipe changes from red to green the traffic light that affects the vehicle. 
# This is done by detecting if the vehicle actor is at a traffic light.

# ...
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

