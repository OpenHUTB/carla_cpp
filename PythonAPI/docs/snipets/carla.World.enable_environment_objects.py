# 这个脚本示例用于在地图上对两栋特定建筑物开启和关闭其可见性

# 获取世界中的建筑物
world = client.get_world()
env_objs = world.get_environment_objects(carla.CityObjectLabel.Buildings)

# 获取单个建筑物的ID并保存到一个集合中
building_01 = env_objs[0]
building_02 = env_objs[1]
objects_to_toggle = {building_01.id, building_02.id}

# 关闭建筑物（使其不可见）
world.enable_environment_objects(objects_to_toggle, False)
# 开启建筑物（使其可见）
world.enable_environment_objects(objects_to_toggle, True)
