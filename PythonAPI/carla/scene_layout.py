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
        left_marking = [_lateral_shift(w.transform, -w.lane_width * 0.5) for w in waypoints]
        right_marking = [_lateral_shift(w.transform, w.lane_width * 0.5) for w in waypoints]
        
        # 将路径点及其左右车道线信息添加到map_dict中
        lane = {"waypoints": waypoints, "left_marking": left_marking, "right_marking": right_marking}
        if map_dict.get(waypoint.road_id) is None:
            map_dict[waypoint.road_id] = {}
        map_dict[waypoint.road_id][waypoint.lane_id] = lane
    
    # 生成路径点图
    waypoints_graph = dict()
    for road_key in map_dict:
        for lane_key in map_dict[road_key]:
            lane = map_dict[road_key][lane_key]
            for i in range(len(lane["waypoints"])):
                # 获取当前路径点的后续路径点ID
                next_ids = [w.id for w in lane["waypoints"][i + 1:]]
                
                # 计算左右车道键
                left_lane_key = lane_key - 1 if lane_key - 1 else lane_key - 2
                right_lane_key = lane_key + 1 if lane_key + 1 else lane_key + 2
                
                # 获取左右车道路径点的ID（如果存在）
                left_lane_waypoint_id = -1
                if left_lane_key in map_dict[road_key]:
                    left_lane_waypoints = map_dict[road_key][left_lane_key]["waypoints"]
                    if i < len(left_lane_waypoints):
                        left_lane_waypoint_id = left_lane_waypoints[i].id
                
                right_lane_waypoint_id = -1
                if right_lane_key in map_dict[road_key]:
                    right_lane_waypoints = map_dict[road_key][right_lane_key]["waypoints"]
                    if i < len(right_lane_waypoints):
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

# 定义一个函数，用于将传入的actors列表根据类型分割成不同的列表
def _split_actors(actors):
    vehicles = []  # 存储车辆的列表
    traffic_lights = []  # 存储交通灯的列表
    speed_limits = []  # 存储速度限制的列表（注意：在CARLA中，速度限制可能作为actor存在）
    walkers = []  # 存储行人的列表
    stops = []  # 存储停车点的列表
    static_obstacles = []  # 存储静态障碍物的列表
    
    for actor in actors:  # 遍历每个actor
        if 'vehicle' in actor.type_id:  # 如果actor的类型ID包含'vehicle'，则认为是车辆
            vehicles.append(actor)
        elif 'traffic_light' in actor.type_id:  # 如果actor的类型ID包含'traffic_light'，则认为是交通灯
            traffic_lights.append(actor)
        elif 'speed_limit' in actor.type_id:  # 如果actor的类型ID包含'speed_limit'，则认为是速度限制
            speed_limits.append(actor)
        elif 'walker' in actor.type_id:  # 如果actor的类型ID包含'walker'，则认为是行人
            walkers.append(actor)
        elif 'stop' in actor.type_id:  # 如果actor的类型ID包含'stop'，则认为是停车点
            stops.append(actor)
        elif 'static.prop' in actor.type_id:  # 如果actor的类型ID包含'static.prop'，则认为是静态障碍物
            static_obstacles.append(actor)
    
    # 返回分割后的列表元组
    return (vehicles, traffic_lights, speed_limits, walkers, stops, static_obstacles)
 
# 公共函数部分
 
# 获取停车点的信号信息
def get_stop_signals(stops):
    stop_signals_dict = dict()  # 创建一个字典来存储停车点的信息
    for stop in stops:  # 遍历每个停车点
        st_transform = stop.get_transform()  # 获取停车点的变换信息
        location_gnss = carla_map.transform_to_geolocation(st_transform.location)  # 将位置转换为GNSS坐标
        # 创建一个字典来存储停车点的ID、位置和触发体积信息
        st_dict = {
            "id": stop.id,
            "position": [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
            "trigger_volume": [[v.longitude, v.latitude, v.altitude] for v in _get_trigger_volume(stop)]  # 获取触发体积的坐标列表
        }
        stop_signals_dict[stop.id] = st_dict  # 将停车点信息添加到字典中
    return stop_signals_dict  # 返回包含所有停车点信息的字典
 
# 获取交通灯的信息
def get_traffic_lights(traffic_lights):
    traffic_lights_dict = dict()  # 创建一个字典来存储交通灯的信息
    for traffic_light in traffic_lights:  # 遍历每个交通灯
        tl_transform = traffic_light.get_transform()  # 获取交通灯的变换信息
        location_gnss = carla_map.transform_to_geolocation(tl_transform.location)  # 将位置转换为GNSS坐标
        # 创建一个字典来存储交通灯的ID、状态、位置和触发体积信息
        tl_dict = {
            "id": traffic_light.id,
            "state": int(traffic_light.state),  # 交通灯的状态（整数表示）
            "position": [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
            "trigger_volume": [[v.longitude, v.latitude, v.altitude] for v in _get_trigger_volume(traffic_light)]  # 获取触发体积的坐标列表
        }
        traffic_lights_dict[traffic_light.id] = tl_dict  # 将交通灯信息添加到字典中
    return traffic_lights_dict  # 返回包含所有交通灯信息的字典
 
# 获取车辆的信息
def get_vehicles(vehicles):
    vehicles_dict = dict()  # 创建一个字典来存储车辆的信息
    for vehicle in vehicles:  # 遍历每个车辆
        v_transform = vehicle.get_transform()  # 获取车辆的变换信息
        location_gnss = carla_map.transform_to_geolocation(v_transform.location)  # 将位置转换为GNSS坐标
        # 创建一个字典来存储车辆的ID、位置、朝向和边界框信息
        v_dict = {
            "id": vehicle.id,
            "position": [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
            "orientation": [v_transform.rotation.roll, v_transform.rotation.pitch, v_transform.rotation.yaw],  # 车辆的朝向（横滚角、俯仰角、偏航角）
            "bounding_box": [[v.longitude, v.latitude, v.altitude] for v in _get_bounding_box(vehicle)]  # 获取边界框的坐标列表
        }
        vehicles_dict[vehicle.id] = v_dict  # 将车辆信息添加到字典中
    return vehicles_dict  # 返回包含所有车辆信息的字典
 
# 获取主角车辆的信息
def get_hero_vehicle(hero_vehicle):
    if hero_vehicle is None:  # 如果主角车辆为空，则直接返回
        return hero_vehicle
    
    hero_waypoint = carla_map.get_waypoint(hero_vehicle.get_location())  # 获取主角车辆所在的路点信息
    hero_transform = hero_vehicle.get_transform()  # 获取主角车辆的变换信息
    location_gnss = carla_map.transform_to_geolocation(hero_transform.location)  # 将位置转换为GNSS坐标
    
    # 创建一个字典来存储主角车辆的ID、位置、道路ID和车道ID信息
    hero_vehicle_dict = {
        "id": hero_vehicle.id,
        "position": [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
        "road_id": hero_waypoint.road_id,
        "lane_id": hero_waypoint.lane_id
    }
    return hero_vehicle_dict  # 返回包含主角车辆信息的字典

# 定义一个函数，用于获取行人的信息并将其存储在一个字典中
def get_walkers(walkers):
    # 初始化一个空字典，用于存储行人的信息
    walkers_dict = dict()
    # 遍历传入的行人列表
    for walker in walkers:
        # 获取行人的位置转换信息
        w_transform = walker.get_transform()
        # 将行人的位置从CARLA坐标转换为地理坐标（GPS）
        location_gnss = carla_map.transform_to_geolocation(w_transform.location)
        # 创建一个字典，用于存储单个行人的信息
        w_dict = {
            # 行人的唯一标识符
            "id": walker.id,
            # 行人的位置，包括纬度、经度和海拔
            "position": [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
            # 行人的朝向，包括横滚角、俯仰角和偏航角
            "orientation": [w_transform.rotation.roll w,_transform.rotation.pitch, w_transform.rotation.yaw],
            # 行人的边界框，这里假设有一个函数_get_bounding_box用于获取行人的边界框顶点
            "bounding_box": [[v.longitude, v.latitude, v.altitude] for v in _get_bounding_box(walker)]
        }
        # 将单个行人的信息添加到字典中，以行人的id作为键
        walkers_dict[walker.id] = w_dict
    # 返回包含所有行人信息的字典
    return walkers_dict
 
# 定义一个函数，用于获取速度限制的信息并将其存储在一个字典中
def get_speed_limits(speed_limits):
    # 初始化一个空字典，用于存储速度限制的信息
    speed_limits_dict = dict()
    # 遍历传入的速度限制列表
    for speed_limit in speed_limits:
        # 获取速度限制的位置转换信息
        sl_transform = speed_limit.get_transform()
        # 将速度限制的位置从CARLA坐标转换为地理坐标（GPS）
        location_gnss = carla_map.transform_to_geolocation(sl_transform.location)
        # 创建一个字典，用于存储单个速度限制的信息
        sl_dict = {
            # 速度限制的唯一标识符
            "id": speed_limit.id,
            # 速度限制的位置，包括纬度、经度和海拔
            "position": [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
            # 速度限制的值，这里假设速度限制的id中包含速度值，通过分割字符串获取
            "speed": int(speed_limit.type_id.split('.')[2])
        }
        # 将单个速度限制的信息添加到字典中，以速度限制的id作为键
        speed_limits_dict[speed_limit.id] = sl_dict
    # 返回包含所有速度限制信息的字典
    return speed_limits_dict
 
# 定义一个函数，用于获取静态障碍物的信息并将其存储在一个字典中
def get_static_obstacles(static_obstacles):
    # 初始化一个空字典，用于存储静态障碍物的信息
    static_obstacles_dict = dict()
    # 遍历传入的静态障碍物列表
    for static_prop in static_obstacles:
        # 获取静态障碍物的位置转换信息
        sl_transform = static_prop.get_transform()
        # 将静态障碍物的位置从CARLA坐标转换为地理坐标（GPS）
        location_gnss = carla_map.transform_to_geolocation(sl_transform.location)
        # 创建一个字典，用于存储单个静态障碍物的信息
        sl_dict = {
            # 静态障碍物的唯一标识符
            "id": static_prop.id,
            # 静态障碍物的位置，包括纬度、经度和海拔
            "position": [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude]
        }
        # 将单个静态障碍物的信息添加到字典中，以静态障碍物的id作为键
        static_obstacles_dict[static_prop.id] = sl_dict
    # 返回包含所有静态障碍物信息的字典
    return static_obstacles_dict
 
# 获取CARLA世界中的所有元素（演员）
actors = carla_world.get_actors()
# 将演员分类为车辆、交通灯、速度限制、行人、停车标志和静态障碍物
vehicles, traffic_lights, speed_limits, walkers, stops, static_obstacles = _split_actors(actors)
 
# 从车辆中筛选出英雄车辆（通常用于模拟驾驶的主车辆）
hero_vehicles = [vehicle for vehicle in vehicles if
                 'vehicle' in vehicle.type_id and vehicle.attributes['role_name'] == 'hero']
# 如果有英雄车辆，则随机选择一个作为hero，否则设置为None
hero = None if len(hero_vehicles) == 0 else random.choice(hero_vehicles)
 
# 返回一个包含所有元素信息的字典
return {
    # 车辆的信息，这里假设有一个函数get_vehicles用于获取车辆的信息
    'vehicles': get_vehicles(vehicles),
    # 英雄车辆的信息，这里假设有一个函数get_hero_vehicle用于获取英雄车辆的信息
    'hero_vehicle': get_hero_vehicle(hero),
    # 行人的信息，使用上面定义的get_walkers函数获取
    'walkers': get_walkers(walkers),
    # 交通灯的信息，这里假设有一个函数get_traffic_lights用于获取交通灯的信息
    'traffic_lights': get_traffic_lights(traffic_lights),
    # 停车标志的信息，这里假设有一个函数get_stop_signals用于获取停车标志的信息
    'stop_signs': get_stop_signals(stops),
    # 速度限制的信息，使用上面定义的get_speed_limits函数获取
    'speed_limits': get_speed_limits(speed_limits),
    # 静态障碍物的信息，使用上面定义的get_static_obstacles函数获取
    'static_obstacles': get_static_obstacles(static_obstacles)
}
