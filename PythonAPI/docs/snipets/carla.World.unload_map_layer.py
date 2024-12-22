# 这个脚本示例用于在我们名为“_Opt”的地图中切换关闭多个图层。

# 加载城镇1（Town01_Opt）地图，同时加载包含基础布局（如道路、人行道、交通信号灯以及交通标识）以及建筑物和停放车辆的相关图层内容。
# 这里使用按位或操作将建筑物图层和停放车辆图层
world = client.load_world('Town01_Opt', carla.MapLayer.Buildings | carla.MapLayer.ParkedVehicles) 

# 切换关闭所有建筑物图层
world.unload_map_layer(carla.MapLayer.Buildings)

# 切换关闭所有停放车辆图层
world.unload_map_layer(carla.MapLayer.ParkedVehicles)
