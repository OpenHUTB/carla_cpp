# 这个脚本示例用于在我们名为“_Opt”的地图中切换开启多个图层。

# 加载城镇1（Town01_Opt）地图，并且只加载最基础的布局元素（包括道路、人行道、交通信号灯以及交通标识）
world = client.load_world('Town01_Opt', carla.MapLayer.None)

# 切换开启所有建筑物图层
world.load_map_layer(carla.MapLayer.Buildings)

# 切换开启所有植被（ foliage ）图层
world.load_map_layer(carla.MapLayer.Foliage)

# 切换开启所有停放车辆图层
world.load_map_layer(carla.MapLayer.ParkedVehicles)
