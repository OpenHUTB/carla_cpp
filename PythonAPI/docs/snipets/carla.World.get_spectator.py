
# 这个脚本示例用于生成一个参与者，并将观察者放置在该参与者所在的位置。

# ...
world = client.get_world()
spectator = world.get_spectator()

vehicle_bp = random.choice(world.get_blueprint_library().filter('vehicle.bmw.*'))
transform = random.choice(world.get_map().get_spawn_points())
vehicle = world.try_spawn_actor(vehicle_bp, transform)

# 让世界进行一次更新（tick）操作
world.tick()

world_snapshot = world.wait_for_tick()
actor_snapshot = world_snapshot.find(vehicle.id)

# 将观察者（spectator）的位置和朝向设置为给定的变换信息
spectator.set_transform(actor_snapshot.get_transform())
# ...
