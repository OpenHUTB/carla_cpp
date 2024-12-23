# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
# Provides map data for users.

import glob  # 用于文件路径的模式匹配
import os  # 提供了许多与操作系统交互的功能
import sys  # 提供了一些变量和函数，用来操纵Python运行时环境
 
try:
    # 尝试将符合特定模式的CARLA egg文件路径添加到系统路径中，以便导入CARLA模块
    # 该模式基于Python版本（major.minor）和操作系统（Windows或Linux）
    sys.path.append(glob.glob('dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    # 如果没有找到符合条件的egg文件，则忽略此异常
    pass
 
import carla  # 导入CARLA模块
import random  # 导入random模块，尽管在这段代码中未直接使用
 
def get_scene_layout(carla_map):
    """
    提取完整的场景布局，作为完整的场景描述返回给用户。
    
    :param carla_map: CARLA地图对象
    :return: 描述场景的字典
    """
    
    def _lateral_shift(transform, shift):
        # 将变换的旋转yaw增加90度，然后沿着变换的前向向量移动指定的偏移量
        transform.rotation.yaw += 90
        return transform.location + shift * transform.get_forward_vector()
    
    # 获取并排序地图的拓扑结构，按道路的高度排序
    topology = [x[0] for x in carla_map.get_topology()]
    topology = sorted(topology, key=lambda w: w.transform.location.z)
    
    # 初始化一个字典来存储地图信息
    map_dict = dict()
    precision = 0.05  # 设置精度
    
    # 遍历拓扑结构中的每个waypoint（路径点）
    for waypoint in topology:
        waypoints = [waypoint]
        nxt = waypoint.next(precision)  # 获取下一个路径点
        while nxt and nxt[0].road_id == waypoint.road_id:  # 如果下一个路径点在同一道路上
            waypoints.append(nxt[0])  # 添加到当前路径点的列表中
            nxt = nxt[0].next(precision)  # 继续获取下一个路径点
        
        # 计算左右车道线的位置
        left_marking = [_lateral_shift(w.transform, -w.lane_width * 0.5) for w in waypoints]# 使用_lateral_shift函数，将每个路径点的transform（位置和朝向）沿横向移动半个车道宽度的距离
        right_marking = [_lateral_shift(w.transform, w.lane_width * 0.5) for w in waypoints]# 同样使用_lateral_shift函数，将每个路径点的transform沿横向移动半个车道宽度的距离，但方向相反
        
        # 将路径点及其左右车道线信息添加到map_dict中
        lane = {"waypoints": waypoints, "left_marking": left_marking, "right_marking": right_marking}# 创建一个字典，包含路径点、左车道线和右车道线的信息
        if map_dict.get(waypoint.road_id) is None:# 检查map_dict中是否存在当前路径点的道路ID
            map_dict[waypoint.road_id] = {}# 如果不存在，初始化一个空字典
        map_dict[waypoint.road_id][waypoint.lane_id] = lane# 将车道信息添加到对应道路ID的字典中
    
    # 生成路径点图
    waypoints_graph = dict()# 初始化一个空字典，用于存储路径点图
    for road_key in map_dict:# 遍历map_dict中的每个道路键（road_key）
        for lane_key in map_dict[road_key]:# 遍历当前道路下的所有车道键（lane_key）
            lane = map_dict[road_key][lane_key]
            for i in range(len(lane["waypoints"])):
                # 获取当前路径点的后续路径点ID
                next_ids = [w.id for w in lane["waypoints"][i + 1:]]
                
                # 计算左右车道键
                left_lane_key = lane_key - 1 if lane_key - 1 else lane_key - 2# 如果lane_key减1的结果是非零值，则left_lane_key为lane_key减1，否则为lane_key减2
                right_lane_key = lane_key + 1 if lane_key + 1 else lane_key + 2# 如果lane_key加1的结果是非零值，则right_lane_key为lane_key加1，否则为lane_key加2
                
                # 获取左右车道路径点的ID（如果存在）
                left_lane_waypoint_id = -1# 初始化左右车道的路径点ID为-1，表示默认不存在
                if left_lane_key in map_dict[road_key]:# 如果左车道key存在于地图字典中，则获取左车道的路径点
                    left_lane_waypoints = map_dict[road_key][left_lane_key]["waypoints"]
                    if i < len(left_lane_waypoints):# 如果索引i小于左车道路径点列表的长度，则获取第i个路径点的ID
                        left_lane_waypoint_id = left_lane_waypoints[i].id
                
                right_lane_waypoint_id = -1# 初始化左右车道的路径点ID为-1，表示默认不存在
                if right_lane_key in map_dict[road_key]:# 如果右车道key存在于地图字典中，则获取右车道的路径点
                    right_lane_waypoints = map_dict[road_key][right_lane_key]["waypoints"]
                    if i < len(right_lane_waypoints):# 如果索引i小于右车道路径点列表的长度，则获取第i个路径点的ID
                        right_lane_waypoint_id = right_lane_waypoints[i].id
                
                # 获取左右边界（车道线）和当前路径点的地理位置和朝向
                lm = carla_map.transform_to_geolocation(lane["left_marking"][i])
                rm = carla_map.transform_to_geolocation(lane["right_marking"][i])
                wl = carla_map.transform_to_geolocation(lane["waypoints"][i].transform.location)
                wo = lane["waypoints"][i].transform.rotation
                
                # 创建当前路径点的字典
                waypoint_dict = {
                    "road_id": road_key,
                    "lane_id": lane_key,
                    "position": [wl.latitude, wl.longitude, wl.altitude],
                    "orientation": [wo.roll, wo.pitch, wo.yaw],
                    "left_margin_position": [lm.latitude, lm.longitude, lm.altitude],
                    "right_margin_position": [rm.latitude, rm.longitude, rm.altitude],
                    "next_waypoints_ids": next_ids,
                    "left_lane_waypoint_id": left_lane_waypoint_id,
                    "right_lane_waypoint_id": right_lane_waypoint_id
                }
                waypoints_graph[map_dict[road_key][lane_key]["waypoints"][i].id] = waypoint_dict
    
    return waypoints_graph  # 返回路径点图
 
def get_dynamic_objects(carla_world, carla_map):
    """
    获取CARLA世界中的动态对象信息。
    
    :param carla_world: CARLA世界对象
    :param carla_map: CARLA地图对象
    :return: 包含不同类型动态对象的元组
    """
    
    def _get_bounding_box(actor):
        # 获取演员的包围盒的角点坐标（经纬度海拔）
        bb = actor.bounding_box.extent
        corners = [carla.Location(x=-bb.x, y=-bb.y), carla.Location(x=bb.x, y=-bb.y),
                   carla.Location(x=bb.x, y=bb.y), carla.Location(x=-bb.x, y=bb.y)]
        t = actor.get_transform()
        t.transform(corners)
        corners = [carla_map.transform_to_geolocation(p) for p in corners]
        return corners
    
    def _get_trigger_volume(actor):
        # 获取演员的触发体积的角点坐标（经纬度海拔）
        bb = actor.trigger_volume.extent
        corners = [carla.Location(x=-bb.x, y=-bb.y), carla.Location(x=bb.x, y=-bb.y),
                   carla.Location(x=bb.x, y=bb.y), carla.Location(x=-bb.x, y=bb.y),
                   carla.Location(x=-bb.x, y=-bb.y)]  # 注意这里最后一个点是重复的，形成一个闭合的多边形
        corners = [x + actor.trigger_volume.location for x in corners]
        t = actor.get_transform()
        t.transform(corners)
        corners = [carla_map.transform_to_geolocation(p) for p in corners]
        return corners

    def _split_actors(actors):
     #定义一个函数 _split_actors，它接受一个包含所有actor的列表 actors 作为参数。
        vehicles = []
        traffic_lights = []
        speed_limits = []
        walkers = []
        stops = []
        static_obstacles = []
     #初始化六个空列表，用于存储不同类型的actor。
        for actor in actors:
         #遍历传入的 actors 列表中的每个actor。
            if 'vehicle' in actor.type_id:
                vehicles.append(actor)
             #检查actor的 type_id 是否包含字符串 "vehicle"，如果是，则将该actor添加到 vehicles 列表中。
            elif 'traffic_light' in actor.type_id:
                traffic_lights.append(actor)
             #检查actor的 type_id 是否包含字符串 "traffic_light"，如果是，则将该actor添加到 traffic_lights 列表中。
            elif 'speed_limit' in actor.type_id:
                speed_limits.append(actor)
             #检查actor的 type_id 是否包含字符串 "speed_limit"，如果是，则将该actor添加到 speed_limits 列表中。
            elif 'walker' in actor.type_id:
                walkers.append(actor)
             #检查actor的 type_id 是否包含字符串 "walker"，如果是，则将该actor添加到 walkers 列表中。
            elif 'stop' in actor.type_id:
                stops.append(actor)
             #检查actor的 type_id 是否包含字符串 "stop"，如果是，则将该actor添加到 stops 列表中。
            elif 'static.prop' in actor.type_id:
                static_obstacles.append(actor)
             #检查actor的 type_id 是否包含字符串 "static.prop"，如果是，则将该actor添加到 static_obstacles 列表中。


        return (vehicles, traffic_lights, speed_limits, walkers, stops, static_obstacles)
        #函数返回六个列表，包含分类后的actors。

    #定义一个函数 get_stop_signals，它接受一个包含所有stop actor的列表 stops 作为参数。
    def get_stop_signals(stops):
        stop_signals_dict = dict()
     #初始化一个空字典 stop_signals_dict，用于存储stop信号的信息。
        for stop in stops:
         #遍历传入的 stops 列表中的每个stop actor。
            st_transform = stop.get_transform()
            location_gnss = carla_map.transform_to_geolocation(st_transform.location)
         #获取stop actor的变换信息 st_transform，然后使用 carla_map.transform_to_geolocation 方法将变换位置转换为GNSS坐标 location_gnss。
            st_dict = {
                "id": stop.id,
                "position": [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
                "trigger_volume": [[v.longitude, v.latitude, v.altitude] for v in _get_trigger_volume(stop)]
            }
         #创建一个字典 st_dict，包含stop actor的ID、位置和触发体积信息。触发体积信息是通过调用 _get_trigger_volume 函数获得的。
            stop_signals_dict[stop.id] = st_dict
         #将 st_dict 添加到 stop_signals_dict 字典中，以stop actor的ID作为键。
        return stop_signals_dict
     #函数返回包含所有stop信号信息的字典 stop_signals_dict。

    # 定义一个函数，用于获取交通灯信息
    def get_traffic_lights(traffic_lights):
        # 初始化一个空字典，用于存储交通灯信息
        traffic_lights_dict = dict()
        # 遍历传入的交通灯列表
        for traffic_light in traffic_lights:
            # 获取当前交通灯的变换信息
            tl_transform = traffic_light.get_transform()
             # 将交通灯的位置从游戏世界坐标转换为全球导航卫星系统坐标
            location_gnss = carla_map.transform_to_geolocation(tl_transform.location)
            # 创建一个字典，用于存储当前交通灯的详细信息
            tl_dict = {
                # 交通灯的ID
                "id": traffic_light.id,
                # 交通灯的状态，转换为整数
                "state": int(traffic_light.state),
                # 交通灯的位置信息，包括纬度、经度和高度
                "position": [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
                # 交通灯触发区域的坐标列表，通过调用_get_trigger_volume函数获取
                "trigger_volume": [[v.longitude, v.latitude, v.altitude] for v in _get_trigger_volume(traffic_light)]
            }
            # 将当前交通灯的信息字典存储到交通灯字典中，以交通灯ID为键
            traffic_lights_dict[traffic_light.id] = tl_dict
        # 返回包含所有交通灯信息的字典
        return traffic_lights_dict

    # 定义一个函数，用于获取车辆信息
    def get_vehicles(vehicles):
        # 初始化一个空字典，用于存储车辆信息
        vehicles_dict = dict()
        # 遍历传入的车辆列表
        for vehicle in vehicles:
            # 获取当前车辆的变换信息，包括位置和旋转
            v_transform = vehicle.get_transform()
            # 将车辆的位置从游戏世界坐标转换为全球导航卫星系统坐标
            location_gnss = carla_map.transform_to_geolocation(v_transform.location)
            # 创建一个字典，用于存储当前车辆的详细信息
            v_dict = {
                "id": vehicle.id,
                 # 车辆的位置信息，包括纬度、经度和高度
                "position": [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
                # 车辆的朝向信息，包括横滚角、俯仰角和偏航角
                "orientation": [v_transform.rotation.roll, v_transform.rotation.pitch, v_transform.rotation.yaw],
                 # 车辆的包围盒信息，通过调用_get_bounding_box函数获取，包含四个角点的坐标
                "bounding_box": [[v.longitude, v.latitude, v.altitude] for v in _get_bounding_box(vehicle)]
            }
             # 将当前车辆的信息字典存储到车辆字典中，以车辆ID为键
            vehicles_dict[vehicle.id] = v_dict
        return vehicles_dict

    #定义了一个用于接受hero_vehicle的函数并实现对其的操作
    def get_hero_vehicle(hero_vehicle):
        if hero_vehicle is None:
            return hero_vehicle

        #获取与hero_vehicle所在位置相关的路点信息
        hero_waypoint = carla_map.get_waypoint(hero_vehicle.get_location())
        #获取hero_vehicle的变换信息
        hero_transform = hero_vehicle.get_transform()
        #将hero_vehicle的变换位置转换为地理位置信息（经纬度高度）
        location_gnss = carla_map.transform_to_geolocation(hero_transform.location)

       #创建一个新字典 其中包含hero_vehicle的id信息
        hero_vehicle_dict = {
            #使用之前转换获取的地理位置信息 并在字典中添加hero_waypoint的id信息
            "id": hero_vehicle.id,
            "position": [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
            "road_id": hero_waypoint.road_id,
            "lane_id": hero_waypoint.lane_id
        }
        return hero_vehicle_dict

    #定义了一个叫get_walkers的函数 用于获取与walkers相关的一些信息并进行处理
    def get_walkers(walkers):
       #创建一个新字典 用于存储与每个walkers相关的信息
        walkers_dict = dict()
        for walker in walkers:
           #调用获取walkers的变换信息
            w_transform = walker.get_transform()
            #将walkers的位置信息转换为地理的地位信息形式
            location_gnss = carla_map.transform_to_geolocation(w_transform.location)
           
            w_dict = {
                "id": walker.id,
                #在字典中添加名为position的键值对 其值为一个包含location_gnss的经纬度高度的列表 把walkers的位置信息以特定形式存储在字典中
                "position": [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
                #添加名为orientation的键值对 值为一个包含w_transform的旋转信息的列表 存储walkers的方向信息
                "orientation": [w_transform.rotation.roll, w_transform.rotation.pitch, w_transform.rotation.yaw],
                #添加名为bounding_box的键值对 构建walkers的包围盒信息
                "bounding_box": [[v.longitude, v.latitude, v.altitude] for v in _get_bounding_box(walker)]
            }
            #把w_dict字典添加到walkers_dict字典中 便以通过walkers的id来获取信息
            walkers_dict[walker.id] = w_dict
        return walkers_dict

    def get_speed_limits(speed_limits):
        speed_limits_dict = dict()
        for speed_limit in speed_limits:
            sl_transform = speed_limit.get_transform()
            location_gnss = carla_map.transform_to_geolocation(sl_transform.location)
            sl_dict = {
                "id": speed_limit.id,
                "position": [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
                "speed": int(speed_limit.type_id.split('.')[2])
            }
            speed_limits_dict[speed_limit.id] = sl_dict
        return speed_limits_dict
#遍历静态障碍物列表，将每个障碍物的位置信息提取出来并存储在一个字典里
    def get_static_obstacles(static_obstacles):
        static_obstacles_dict = dict()
        for static_prop in static_obstacles:
            sl_transform = static_prop.get_transform()
            location_gnss = carla_map.transform_to_geolocation(sl_transform.location)
            sl_dict = {
                "id": static_prop.id,
                "position": [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude]
            }
            static_obstacles_dict[static_prop.id] = sl_dict
        return static_obstacles_dict
#从一个更大的actors列表中分离出不类型的actors
    actors = carla_world.get_actors()
    vehicles, traffic_lights, speed_limits, walkers, stops, static_obstacles = _split_actors(actors)

    hero_vehicles = [vehicle for vehicle in vehicles if
                     'vehicle' in vehicle.type_id and vehicle.attributes['role_name'] == 'hero']
    hero = None if len(hero_vehicles) == 0 else random.choice(hero_vehicles)
#将信息整合到一个字典中，并返回
    return {
        'vehicles': get_vehicles(vehicles),
        'hero_vehicle': get_hero_vehicle(hero),
        'walkers': get_walkers(walkers),
        'traffic_lights': get_traffic_lights(traffic_lights),
        'stop_signs': get_stop_signals(stops),
        'speed_limits': get_speed_limits(speed_limits),
        'static_obstacles': get_static_obstacles(static_obstacles)
    }
