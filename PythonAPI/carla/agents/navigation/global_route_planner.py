# Copyright (c) # Copyright (c) 2018-2020 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


"""
This module provides GlobalRoutePlanner implementation.
"""

import math
import numpy as np
import networkx as nx

import carla
from agents.navigation.local_planner import RoadOption

# Python 2 compatibility
# 用于类型检查相关的标记，后续根据Python版本来决定如何导入特定的类型相关模块
TYPE_CHECKING = False
if TYPE_CHECKING:
    import sys
    # 判断Python版本是否大于等于3.11
    if sys.version_info >= (3, 11):
        # 从typing模块导入TypedDict和NotRequired，用于类型注解（Python 3.11及以上版本）
        from typing import TypedDict, NotRequired
    # 判断Python版本是否大于等于3.8且小于3.11
    elif sys.version_info >= (3, 8):
        # 从typing模块导入TypedDict
        from typing import TypedDict
        # 从typing_extensions模块导入NotRequired，用于在对应Python版本下补充类型注解功能
        from typing_extensions import NotRequired
    else:
        # 如果Python版本小于3.8，从typing_extensions模块同时导入TypedDict和NotRequired来处理类型注解相关操作
        from typing_extensions import TypedDict, NotRequired

    # 定义一个名为TopologyDict的类型字典（TypedDict），用于描述拓扑结构相关信息
    # 它包含了以下键值对的类型规范：
    # 'entry': 类型为carla.Waypoint，表示入口的路点信息
    # 'exit': 类型为carla.Waypoint，表示出口的路点信息
    # 'entryxyz': 类型为包含三个浮点数的元组，表示入口的坐标信息（x, y, z）
    # 'exitxyz': 类型为包含三个浮点数的元组，表示出口的坐标信息（x, y, z）
    # 'path': 类型为包含carla.Waypoint元素的列表，表示路径上的一系列路点
    TopologyDict = TypedDict('TopologyDict',
        {
            'entry': carla.Waypoint,
            'exit': carla.Waypoint,
            'entryxyz': tuple[float, float, float],
            'exitxyz': tuple[float, float, float],
            'path': list[carla.Waypoint]
        })

    # 定义一个名为EdgeDict的类型字典（TypedDict），用于描述边相关的信息
    # 它包含了以下键值对的类型规范：
    # 'length': 类型为整数，表示边的长度（具体含义可能根据应用场景而定）
    # 'path': 类型为包含carla.Waypoint元素的列表，表示这条边所经过的路点路径
    # 'entry_waypoint': 类型为carla.Waypoint，表示边的入口路点
    # 'exit_waypoint': 类型为carla.Waypoint，表示边的出口路点
    # 'entry_vector': 类型为numpy.ndarray，可能表示入口处的向量信息（比如方向向量等，依具体情况而定）
    # 'exit_vector': 类型为numpy.ndarray，类似地表示出口处的向量信息
    # 'net_vector': 类型为包含浮点数的列表，具体含义根据实际应用场景确定，可能与网络相关向量有关
    # 'intersection': 类型为布尔值，用于判断是否为交叉路口相关的边
    # 'type': 类型为RoadOption，可能表示边对应的道路选项类型（比如直行、转弯等不同道路行为类型）
    # 'change_waypoint': 类型为carla.Waypoint，不过是可选的（NotRequired），可能在某些路径变化的场景下用到的路点信息
    # 定义EdgeDict类型字典，用于描述边相关的各种属性类型
   EdgeDict = TypedDict('EdgeDict',
        {
            'length': int,
            'path': list[carla.Waypoint],
            'entry_waypoint': carla.Waypoint,
            'exit_waypoint': carla.Waypoint,
            'entry_vector': np.ndarray,
            'exit_vector': np.ndarray,
            'net_vector': list[float],
            'intersection': bool,
            'type': RoadOption,
            'change_waypoint': NotRequired[carla.Waypoint]
        })

# 定义GlobalRoutePlanner类，用于提供高层次的路线规划功能
class GlobalRoutePlanner:
    """
    This class provides a very high level route plan.
    """
    # 类的初始化方法，接收地图对象和采样分辨率作为参数
    def __init__(self, wmap, sampling_resolution):
        # type: (carla.Map, float) -> None
        # 保存采样分辨率，可能用于后续路径规划中距离相关的计算等操作
        self._sampling_resolution = sampling_resolution
        # 保存传入的地图对象，后续会基于此地图进行拓扑结构构建、路径搜索等操作
        self._wmap = wmap
        # 用于存储拓扑结构信息，元素类型为TopologyDict（之前定义的拓扑结构类型字典）
        self._topology = []    # type: list[TopologyDict]
        # 用于存储构建的有向图（networkx的DiGraph类型），初始化为None，后续会进行构建
        self._graph = None     # type: nx.DiGraph # type: ignore[assignment]
        # 用于将坐标（以三元组形式表示，可能是xyz坐标）映射到一个整数标识，初始化为None，后续构建和赋值
        self._id_map = None    # type: dict[tuple[float, float, float], int] # type: ignore[assignment]
        # 用于将道路相关标识（道路ID、路段ID、车道ID等组合）映射到边相关信息的嵌套字典，初始化为None
        self._road_id_to_edge = None  # type: dict[int, dict[int, dict[int, tuple[int, int]]]] # type: ignore[assignment]

        # 用于标记交叉路口的结束节点，初始化为 -1，具体含义和使用场景在后续路径处理相关逻辑中体现
        self._intersection_end_node = -1
        # 用于记录上一次的决策（类型为RoadOption，可能是不同道路行驶选择如直行、转弯等），初始化为RoadOption.VOID
        self._previous_decision = RoadOption.VOID

        # 构建拓扑结构，这是初始化过程中进行的一系列准备工作之一
        self._build_topology()
        # 基于拓扑结构构建图，用于后续的路径搜索等操作
        self._build_graph()
        # 查找图中松散的端点（可能是孤立的节点等情况）并进行相应处理
        self._find_loose_ends()
        # 处理车道变更相关的连接情况
        self._lane_change_link()

    # 用于追踪从起点到终点的路线，返回包含路点和道路选项的元组列表
    def trace_route(self, origin, destination):
        # type: (carla.Location, carla.Location) -> list[tuple[carla.Waypoint, RoadOption]]
        """
        This method returns list of (carla.Waypoint, RoadOption)
        from origin to destination
        """
        # 用于存储最终的路线追踪结果，初始为空列表，元素类型为包含路点和道路选项的元组
        route_trace = []  # type: list[tuple[carla.Waypoint, RoadOption]]
        # 通过路径搜索方法获取从起点到终点的路径（以节点编号等形式表示的序列）
        route = self._path_search(origin, destination)
        # 获取起点对应的路点信息
        current_waypoint = self._wmap.get_waypoint(origin)
        # 获取终点对应的路点信息
        destination_waypoint = self._wmap.get_waypoint(destination)

        # 遍历路径中的每一段（除了最后一段，因为是到终点了）
        for i in range(len(route) - 1):
            # 根据当前路径段的索引等信息确定道路选项（比如该转弯还是直行等决策）
            road_option = self._turn_decision(i, route)
            # 获取当前路径段对应的边信息（类型为EdgeDict，包含边的各种属性）
            edge = self._graph.edges[route[i], route[i + 1]]  # type: EdgeDict
            path = []  # type: list[carla.Waypoint]

            # 如果边的类型不是车道跟随（正常沿着车道行驶）且不是无效类型（可能表示特殊情况等）
            if edge['type']!= RoadOption.LANEFOLLOW and edge['type']!= RoadOption.VOID:
                # 将当前路点和道路选项添加到路线追踪结果中
                route_trace.append((current_waypoint, road_option))
                # 获取边的出口路点
                exit_wp = edge['exit_waypoint']
                # 通过道路相关标识从_road_id_to_edge字典中获取对应的边的节点编号（可能用于后续查找下一段边等操作）
                n1, n2 = self._road_id_to_edge[exit_wp.road_id][exit_wp.section_id][exit_wp.lane_id]
                # 获取下一段边的信息（类型同样为EdgeDict）
                next_edge = self._graph.edges[n1, n2]  # type: EdgeDict
                # 如果下一段边的路径列表不为空（即存在路点路径）
                if next_edge['path']:
                    # 在该路径列表中查找与当前路点距离最近的路点索引
                    closest_index = self._find_closest_in_list(current_waypoint, next_edge['path'])
                    # 对索引进行调整，限制最大索引值（防止越界等情况），这里加5可能是适当扩展范围等考虑
                    closest_index = min(len(next_edge['path']) - 1, closest_index + 5)
                    # 更新当前路点为下一段边路径中调整后的最近路点
                    current_waypoint = next_edge['path'][closest_index]
                else:
                    # 如果下一段边没有路径列表（可能是特殊情况），则直接将出口路点作为当前路点
                    current_waypoint = next_edge['exit_waypoint']
                # 再次将更新后的当前路点和道路选项添加到路线追踪结果中
                route_trace.append((current_waypoint, road_option))

            else:
                # 如果边的类型是车道跟随或者无效类型，则进行以下操作
                path = path + [edge['entry_waypoint']] + edge['path'] + [edge['exit_waypoint']]
                # 在完整的路径中查找与当前路点距离最近的路点索引
                closest_index = self._find_closest_in_list(current_waypoint, path)
                # 遍历从最近路点开始的后续路点
                for waypoint in path[closest_index:]:
                    # 更新当前路点为遍历到的路点
                    current_waypoint = waypoint
                    # 将当前路点和道路选项添加到路线追踪结果中
                    route_trace.append((current_waypoint, road_option))
                    # 判断是否接近终点（根据剩余路径段数量以及与终点的距离等条件判断）
                    if len(route) - i <= 2 and waypoint.transform.location.distance(
                            destination) < 2 * self._sampling_resolution:
                        break
                    # 或者判断是否已经处于和终点相同的道路、路段、车道等情况
                    #首先检查route的长度为确保有元素可进行后续的操作 若route为空 则后续操作无意义 然后比较当前路点的道路id是否等于目的路点
                    elif len(
                            route) - i <= 2 and current_waypoint.road_id == destination_waypoint.road_id and current_waypoint.section_id == destination_waypoint.section_id and current_waypoint.lane_id == destination_waypoint.lane_id:
                        # 查找终点路点在当前路径中的最近索引
                        destination_index = self._find_closest_in_list(destination_waypoint, path)
                        if closest_index > destination_index:
                            break

        return route_trace
        #定义一个_build_topology函数 函数目的是构建拓朴结构：从服务器获取道路拓扑信息 然后将其处理包含特定属性的字典对象列表
    def _build_topology(self):
        """
        This function retrieves topology from the server as a list of
        road segments as pairs of waypoint objects, and processes the
        topology into a list of dictionary objects with the following attributes

        - entry (carla.Waypoint): waypoint of entry point of road segment
        - entryxyz (tuple): (x,y,z) of entry point of road segment
        - exit (carla.Waypoint): waypoint of exit point of road segment
        - exitxyz (tuple): (x,y,z) of exit point of road segment
        - path (list of carla.Waypoint):  list of waypoints between entry to exit, separated by the resolution
        """
        #创建一个空列表 用于存储最终构建的拓扑结构信息
        self._topology = []
        # Retrieving waypoints to construct a detailed topology
        #从地图中获取拓扑信息
        for segment in self._wmap.get_topology():
            wp1, wp2 = segment[0], segment[1]
            #获取路点的位置并进行舍入（舍入的目的是为了后续避免点数精度问题，减少一定的误差）
            l1, l2 = wp1.transform.location, wp2.transform.location
            # 舍入以避免浮点不精确
            x1, y1, z1, x2, y2, z2 = np.round([l1.x, l1.y, l1.z, l2.x, l2.y, l2.z], 0)
            wp1.transform.location, wp2.transform.location = l1, l2
            #创建字典并填充信息 这个字典将用于存储当前道路段的拓扑信息
            seg_dict = dict()  # type: TopologyDict # type: ignore[assignment]
            #将路点分别作为入口和出口路点添加到字典中
            seg_dict['entry'], seg_dict['exit'] = wp1, wp2
            #将入口和出口路点的坐标以元组坐标形式添加到字典中
            seg_dict['entryxyz'], seg_dict['exitxyz'] = (x1, y1, z1), (x2, y2, z2)
            #创建一个空的路径列表 用于存储道路段中入口到出口的路点列表
            seg_dict['path'] = []
            #关于endloc以及路径构建逻辑
            endloc = wp2.transform.location
            #计算两地之间的距离进行比较
            if wp1.transform.location.distance(endloc) > self._sampling_resolution:
                #如果满足上述距离条件 获取wp1的下一点
                w = wp1.next(self._sampling_resolution)[0]
                #只要w.transform.location 和endloc两地间距离大于self._sampling_resolution就会进入该while循环
                while w.transform.location.distance(endloc) > self._sampling_resolution:
                    #不断构建路径
                    seg_dict['path'].append(w)
                    #获取w的下一个点序列
                    next_ws = w.next(self._sampling_resolution)
                    #如果next_ws的长度为0则跳出该while循环
                    if len(next_ws) == 0:
                        break
                    w = next_ws[0]
            else:
                #如果最初距离不大于self._sampling_resolution则进入else部分 首先获取wp1的下一个点序列next_wps
                next_wps = wp1.next(self._sampling_resolution)
                #若next_wps的长度为0则跳过当前循环
                if len(next_wps) == 0:
                    continue
                    #如果next_wps不为空 就next_wps[0]将添加到seg_dict['path']字典中
                seg_dict['path'].append(next_wps[0])
                #将字典添加到拓扑函数中
            self._topology.append(seg_dict)

    #定义一个_build_graph函数 函数功能是构建一个network有向图来表示拓扑结构同时还构建了两个字典用于映射相关信息
    def _build_graph(self):
        """
        This function builds a networkx graph representation of topology, creating several class attributes:
        - graph (networkx.DiGraph): networkx graph representing the world map, with:
            Node properties:
                vertex: (x,y,z) position in world map
            Edge properties:
                entry_vector: unit vector along tangent at entry point
                exit_vector: unit vector along tangent at exit point
                net_vector: unit vector of the chord from entry to exit
                intersection: boolean indicating if the edge belongs to an  intersection
        - id_map (dictionary): mapping from (x,y,z) to node id
        - road_id_to_edge (dictionary): map from road id to edge in the graph
        """

        #创建并初始化图和字典
        self._graph = nx.DiGraph()
        self._id_map = dict()  # Map with structure {(x,y,z): id, ... }
        self._road_id_to_edge = dict()  # Map with structure {road_id: {lane_id: edge, ... }, ... }

        #遍历拓扑结构中的每个路段
        for segment in self._topology:
            #获取路段的入口和出口的坐标
            entry_xyz, exit_xyz = segment['entryxyz'], segment['exitxyz']
            #获取路段的路径
            path = segment['path']
            #获取路段入口和出口的更多相关信息
            entry_wp, exit_wp = segment['entry'], segment['exit']
            #判断入口是否是路口
            intersection = entry_wp.is_junction
            #获取道路id路段id和车道id
            road_id, section_id, lane_id = entry_wp.road_id, entry_wp.section_id, entry_wp.lane_id

            #处理节点和映射关系
            #对入口和出口坐标进行处理
            for vertex in entry_xyz, exit_xyz:
                # Adding unique nodes and populating id_map
                #如果坐标不在_id_map这个字典中则为这个新坐标分配一个新id即当前字典的长度
                if vertex not in self._id_map:
                    new_id = len(self._id_map)
                    #将坐标和新的id添加到._id_map字典中
                    self._id_map[vertex] = new_id
                    #在图中添加一个新的节点
                    self._graph.add_node(new_id, vertex=vertex)
            #获取入口坐标对应的节点id
            n1 = self._id_map[entry_xyz]
            #获取出口坐标对应的节点id
            n2 = self._id_map[exit_xyz]
            #处理边和相关字典映射
            #如果road_id不在_road_id_to_edge字典中就为road_id创建一个新的空子字典
            if road_id not in self._road_id_to_edge:
                self._road_id_to_edge[road_id] = dict()
            #如果ection_id也不在_road_id_to_edge这个字典中就为section_id创建一个新的空子字典
            if section_id not in self._road_id_to_edge[road_id]:
                self._road_id_to_edge[road_id][section_id] = dict()
                #将_road_id映射到由入口和出口节点id组成的元组 表示图中的一条边
            self._road_id_to_edge[road_id][section_id][lane_id] = (n1, n2)

            #获取出入口点的旋转信息和前向向量 将结果存储在ntry_carla_vector变量中
            entry_carla_vector = entry_wp.transform.rotation.get_forward_vector()
            #获取出出口点的旋转信息和前向向量 将结果存储在ntry_carla_vector变量中
            exit_carla_vector = exit_wp.transform.rotation.get_forward_vector()
            #计算出口点和入口点的位置差 并转换为单位向量将其存储在net_carla_vector变量中
            net_carla_vector = (exit_wp.transform.location - entry_wp.transform.location).make_unit_vector()

            # Adding edge with attributes
            self._graph.add_edge(
                n1, n2,
                length=len(path) + 1, path=path,
                entry_waypoint=entry_wp, exit_waypoint=exit_wp,
                entry_vector=np.array(
                    [entry_carla_vector.x, entry_carla_vector.y, entry_carla_vector.z]),
                exit_vector=np.array(
                    [exit_carla_vector.x, exit_carla_vector.y, exit_carla_vector.z]),
                net_vector=[net_carla_vector.x, net_carla_vector.y, net_carla_vector.z],
                intersection=intersection, type=RoadOption.LANEFOLLOW)
    #定义一个函数，用于查找未连接的端点
    def _find_loose_ends(self):
        """
        This method finds road segments that have an unconnected end, and
        adds them to the internal graph representation
        """
        #初始化一个计数器，用于纪录未连接端点的数量
        count_loose_ends = 0
        #获取采样分辨率，可能用于后续的计算
        hop_resolution = self._sampling_resolution
        #循环遍历拓扑结构
        for segment in self._topology:
            #获取路段的出口端点
            end_wp = segment['exit']
            #获取出口端点的坐标
            exit_xyz = segment['exitxyz']
            #从出口端点获取道路ID、路段ID和车道ID
            road_id, section_id, lane_id = end_wp.road_id, end_wp.section_id, end_wp.lane_id
            #检查端点连接情况
            #如果存在，则不进行任何操作
            if road_id in self._road_id_to_edge \
                    and section_id in self._road_id_to_edge[road_id] \
                    and lane_id in self._road_id_to_edge[road_id][section_id]:
                pass
            #如果端点没有连接，则将count_loose_ends加1
            else:
                count_loose_ends += 1
                #初始化数据结构
                #如果道路 ID 不在self.road_id_to_edge中，则初始化一个空字典
                if road_id not in self._road_id_to_edge:
                    self._road_id_to_edge[road_id] = dict()
                #如果路段 ID 不在对应道路 ID 的字典中，则初始化一个空字典
                if section_id not in self._road_id_to_edge[road_id]:
                    self._road_id_to_edge[road_id][section_id] = dict()
                #获取出口坐标对应的节点 ID
                n1 = self._id_map[exit_xyz]
                #计算未连接端点的节点 ID
                n2 = -1 * count_loose_ends
                #将节点 ID 存储在self.road_id_to_edge数据结构中
                self._road_id_to_edge[road_id][section_id][lane_id] = (n1, n2)
                #获取下一个端点
                next_wp = end_wp.next(hop_resolution)
                #初始化路径列表
                path = []  # type: list[carla.Waypoint]
                #构建路径和图
                #循环直到下一个端点不存在或不匹配当前的道路 ID、路段 ID 和车道 ID
                while next_wp is not None and next_wp \
                        and next_wp[0].road_id == road_id \
                        and next_wp[0].section_id == section_id \
                        and next_wp[0].lane_id == lane_id:
                    #将下一个端点添加到路径列表中
                    path.append(next_wp[0])
                    #获取下一个端点
                    next_wp = next_wp[0].next(hop_resolution)
                #如果路径列表不为空,获取路径最后一个端点的坐标
                if path:
                    n2_xyz = (path[-1].transform.location.x,
                              path[-1].transform.location.y,
                              path[-1].transform.location.z)
                    #在图中添加节点
                    self._graph.add_node(n2, vertex=n2_xyz)
                    #在图中添加边，连接两个节点，并设置边的属性
                    self._graph.add_edge(
                        n1, n2,
                        length=len(path) + 1, path=path,
                        entry_waypoint=end_wp, exit_waypoint=path[-1],
                        entry_vector=None, exit_vector=None, net_vector=None,
                        intersection=end_wp.is_junction, type=RoadOption.LANEFOLLOW)
    #定义一个函数，用于添加车道变更链接
    def _lane_change_link(self):
        """
        This method places zero cost links in the topology graph
        representing availability of lane changes.
        """
        #遍历self.topology中的每个路段（segment）
        for segment in self._topology:
            #初始化两个布尔变量，用于跟踪是否找到了向左和向右的车道变更
            left_found, right_found = False, False
            #遍历路段中的每个路点（waypoint）
            for waypoint in segment['path']:
                #如果路段的入口不是交叉路口
                if not segment['entry'].is_junction:
                    #初始化下一个路点、下一个道路选项和下一个路段为None
                    next_waypoint, next_road_option, next_segment = None, None, None
                    #如果路点有右车道标记、允许右车道变更且尚未找到右车道变更
                    if waypoint.right_lane_marking and waypoint.right_lane_marking.lane_change & carla.LaneChange.Right and not right_found:
                        #获取右车道的下一个路点
                        next_waypoint = waypoint.get_right_lane()
                        #检查下一个路点是否存在、车道类型是否为行驶车道、道路 ID 是否相同等
                        if next_waypoint is not None \
                                and next_waypoint.lane_type == carla.LaneType.Driving \
                                and waypoint.road_id == next_waypoint.road_id:
                            #设置下一个道路选项为向右变更车道
                            next_road_option = RoadOption.CHANGELANERIGHT
                            #获取下一个路段
                            next_segment = self._localize(next_waypoint.transform.location)
                            #如果下一个路段存在，则在图中添加一条边，表示向右车道变更
                            if next_segment is not None:
                                self._graph.add_edge(
                                    self._id_map[segment['entryxyz']], next_segment[0], entry_waypoint=waypoint,
                                    exit_waypoint=next_waypoint, intersection=False, exit_vector=None,
                                    path=[], length=0, type=next_road_option, change_waypoint=next_waypoint)
                                #标记已经找到了向右车道变更
                                right_found = True
                    #如果路点有左车道标记、允许左车道变更且尚未找到左车道变更
                    #类似的操作，获取左车道的下一个路点、设置下一个道路选项为向左变更车道、获取下一个路段，并在图中添加边
                    if waypoint.left_lane_marking and waypoint.left_lane_marking.lane_change & carla.LaneChange.Left and not left_found:
                        next_waypoint = waypoint.get_left_lane()
                        if next_waypoint is not None \
                                and next_waypoint.lane_type == carla.LaneType.Driving \
                                and waypoint.road_id == next_waypoint.road_id:
                            next_road_option = RoadOption.CHANGELANELEFT
                            next_segment = self._localize(next_waypoint.transform.location)
                            if next_segment is not None:
                                self._graph.add_edge(
                                    self._id_map[segment['entryxyz']], next_segment[0], entry_waypoint=waypoint,
                                    exit_waypoint=next_waypoint, intersection=False, exit_vector=None,
                                    path=[], length=0, type=next_road_option, change_waypoint=next_waypoint)
                                #标记已经找到了向左车道变更
                                left_found = True
                #如果已经找到了向左和向右的车道变更，则跳出循环
                if left_found and right_found:
                    break
    #定义一个函数，用于定位给定位置的道路段
    def _localize(self, location):
        # type: (carla.Location) -> None | tuple[int, int]
        """
        This function finds the road segment that a given location
        is part of, returning the edge it belongs to
        """
        #使用self._wmap（可能是地图对象）的get_waypoint方法获取给定位置的路点（waypoint）
        waypoint = self._wmap.get_waypoint(location)
        #初始化edge变量为None，它可能最终存储一个包含两个整数的元组
        edge = None  # type: None | tuple[int, int]
        #查找道路段所属的边
        #开始一个try块，用于捕获可能的键错误（KeyError）
        try:
            #尝试从self._road_id_to_edge字典中获取给定路点的道路 ID、路段 ID 和车道 ID 对应的边
            edge = self._road_id_to_edge[waypoint.road_id][waypoint.section_id][waypoint.lane_id]
        #如果发生键错误（即找不到对应的边）
        except KeyError:
            #不做任何操作，直接跳过
            pass
        return edge

    def _distance_heuristic(self, n1, n2):
        """
        Distance heuristic calculator for path searching
        in self._graph
        """
        l1 = np.array(self._graph.nodes[n1]['vertex'])
        l2 = np.array(self._graph.nodes[n2]['vertex'])
        return np.linalg.norm(l1 - l2)
    #定义一个函数，用于在图中搜索路径
    def _path_search(self, origin, destination):
        # type: (carla.Location, carla.Location) -> list[int]
        """
        This function finds the shortest path connecting origin and destination
        using A* search with distance heuristic.
        origin      :   carla.Location object of start position
        destination :   carla.Location object of of end position
        return      :   path as list of node ids (as int) of the graph self._graph
        connecting origin and destination
        """
        #使用self._localize方法将起点和终点的位置转换为图中的节点
        start, end = self._localize(origin), self._localize(destination)
        #使用networkx的astar_path函数来寻找最短路径
        #图是self._graph，起点是start[0]，终点是end[0]
        #使用self._distance_heuristic作为启发式函数，边的权重是'length'
        route = nx.astar_path(
            self._graph, source=start[0], target=end[0],
            heuristic=self._distance_heuristic, weight='length')
        #将终点的第二个元素添加到路径中（可能是终点的另一个属性）
        route.append(end[1])
        #返回找到的路径
        return route
    #定义一个函数，从路线中的一个起始索引开始，返回最后一个连续的交叉路口边。这有助于跳过微小的交叉路口边，以便正确计算转弯决策。
    def _successive_last_intersection_edge(self, index, route):
        # type: (int, list[int]) -> tuple[int | None, EdgeDict | None]
        """
        This method returns the last successive intersection edge
        from a starting index on the route.
        This helps moving past tiny intersection edges to calculate
        proper turn decisions.
        """
        #初始化一个变量last_intersection_edge，用于存储最后一个交叉路口边，初始值为None
        last_intersection_edge = None  # type: EdgeDict | None
        #初始化一个变量last_node，用于存储最后一个节点，初始值为None
        last_node = None
        #遍历从index开始到route列表末尾的节点对
        #node1和node2分别代表路线中的两个连续节点
        for node1, node2 in [(route[i], route[i + 1]) for i in range(index, len(route) - 1)]:
            #获取self._graph中连接node1和node2的边，存储在candidate_edge中
            candidate_edge = self._graph.edges[node1, node2]  # type: EdgeDict
            #如果node1等于route列表中的index位置的节点
            if node1 == route[index]:
                #将candidate_edge赋值给last_intersection_edge
                last_intersection_edge = candidate_edge
            #如果candidate_edge的类型是RoadOption.LANEFLLOW且是一个交叉路口
            if candidate_edge['type'] == RoadOption.LANEFOLLOW and candidate_edge['intersection']:
                #将candidate_edge赋值给last_intersection_edge
                last_intersection_edge = candidate_edge
                #将node2赋值给last_node
                last_node = node2
            #如果上述条件不满足
            else:
                #跳出循环
                break

        return last_node, last_intersection_edge

    def _turn_decision(self, index, route, threshold=math.radians(35)):
         """
        函数功能：
        该方法用于返回路线列表（route）中当前索引（index）位置前后两条边对应的转向决策（RoadOption类型）。

        参数说明：
        - index: 当前在路线列表中的索引位置，用于确定要分析的边对所处位置，类型为int。
        - route: 表示路线的节点列表，其中每个元素是一个节点的标识（假设为int类型），类型为list[int]。
        - threshold: 用于判断转向决策的角度阈值，默认值是将35度转换为弧度后的值，类型为float。

        返回值：
        返回对应的转向决策，类型为RoadOption。
        """
        # type: (int, list[int], float) -> RoadOption
        """
        This method returns the turn decision (RoadOption) for pair of edges
        around current index of route list
        """

        decision = None
        previous_node = route[index - 1]
        current_node = route[index]
        next_node = route[index + 1]
        # 获取当前节点与下一个节点之间的边信息（假设EdgeDict是存储边属性的字典类型）
        next_edge = self._graph.edges[current_node, next_node]  # type: EdgeDict
        if index > 0:
            # 如果满足以下一系列条件：
            # 1. 之前的决策不是RoadOption.VOID（表示之前有有效的决策）。
            # 2. 交叉路口结束节点大于0（表示存在有效的交叉路口结束节点）。
            # 3. 交叉路口结束节点不等于前一个节点。
            # 4. 下一条边的类型是RoadOption.LANEFOLLOW（表示沿着车道行驶）。
            # 5. 下一条边处于交叉路口。
            if self._previous_decision != RoadOption.VOID \
                    and self._intersection_end_node > 0 \
                    and self._intersection_end_node != previous_node \
                    and next_edge['type'] == RoadOption.LANEFOLLOW \
                    and next_edge['intersection']:
                decision = self._previous_decision
            else:
                # 重置交叉路口结束节点为 -1，表示当前不在特定的交叉路口相关情境中
                self._intersection_end_node = -1
                # 获取前一个节点与当前节点之间的边信息
                current_edge = self._graph.edges[previous_node, current_node]  # type: EdgeDict
                # 判断是否需要计算转向，条件为：
                # 1. 当前边类型是RoadOption.LANEFOLLOW且不在交叉路口。
                # 2. 下一条边类型是RoadOption.LANEFOLLOW且在交叉路口。
                calculate_turn = current_edge['type'] == RoadOption.LANEFOLLOW and not current_edge[
                    'intersection'] and next_edge['type'] == RoadOption.LANEFOLLOW and next_edge['intersection']
                if calculate_turn:
                    # 查找当前索引位置往后连续的最后一个处于交叉路口的边相关信息（包括节点和边本身）
                    last_node, tail_edge = self._successive_last_intersection_edge(index, route)
                    self._intersection_end_node = last_node
                    if tail_edge is not None:
                        next_edge = tail_edge
                    # 获取当前边的出口向量和下一条边的出口向量（假设边的属性中有这些向量相关信息）
                    cv, nv = current_edge['exit_vector'], next_edge['exit_vector']
                    if cv is None or nv is None:
                        return next_edge['type']
                    cross_list = []
                    # 遍历当前节点的所有后继节点（邻居节点）
                    for neighbor in self._graph.successors(current_node):
                        # 获取当前节点到邻居节点的边信息
                        select_edge = self._graph.edges[current_node, neighbor]
                        if select_edge['type'] == RoadOption.LANEFOLLOW:
                            if neighbor != route[index + 1]:
                                # 获取该边的网络向量（假设边属性中有此向量），并计算与当前边出口向量的叉积的z分量，添加到叉积列表中
                                sv = select_edge['net_vector']
                                cross_list.append(np.cross(cv, sv)[2])
                    # 计算当前边出口向量与下一条边出口向量的叉积的z分量
                    next_cross = np.cross(cv, nv)[2]
                    # 计算当前边出口向量与下一条边出口向量的夹角偏差（通过向量点积和向量模长来计算夹角的余弦值，再反余弦得到夹角）
                    deviation = math.acos(np.clip(
                        np.dot(cv, nv) / (np.linalg.norm(cv) * np.linalg.norm(nv)), -1.0, 1.0))
                    if not cross_list:
                        cross_list.append(0)
                    # 根据夹角偏差和叉积结果来确定转向决策：
                    # 1. 如果夹角偏差小于阈值，决策为直走（RoadOption.STRAIGHT）。
                    # 2. 如果叉积列表不为空且当前边与下一条边的叉积小于叉积列表中的最小值，决策为左转（RoadOption.LEFT）。
                    # 3. 如果叉积列表不为空且当前边与下一条边的叉积大于叉积列表中的最大值，决策为右转（RoadOption.RIGHT）。
                    # 4. 如果当前边与下一条边的叉积小于0，决策为左转。
                    # 5. 如果当前边与下一条边的叉积大于0，决策为右转。
                    if deviation < threshold:
                        decision = RoadOption.STRAIGHT
                    elif cross_list and next_cross < min(cross_list):
                        decision = RoadOption.LEFT
                    elif cross_list and next_cross > max(cross_list):
                        decision = RoadOption.RIGHT
                    elif next_cross < 0:
                        decision = RoadOption.LEFT
                    elif next_cross > 0:
                        decision = RoadOption.RIGHT
                else:
                    decision = next_edge['type']

        else:
            decision = next_edge['type']

        # 更新之前的决策为当前决策，方便后续使用
        self._previous_decision = decision
        return decision

    def _find_closest_in_list(self, current_waypoint, waypoint_list):
         """
        函数功能：
        在给定的路点列表（waypoint_list）中，查找距离当前路点（current_waypoint）最近的路点的索引。

        参数说明：
        - current_waypoint: 当前要作为参考的路点，其具有位置等相关属性（假设通过transform.location获取位置信息），类型根据实际情况而定。
        - waypoint_list: 包含多个路点的列表，每个路点同样有位置等相关属性，类型为List[某种路点类型]（这里用某种路点类型表示实际对应的路点类或结构体等）。

        返回值：
        返回距离当前路点最近的路点在waypoint_list中的索引，类型为int，如果没有找到更近的路点（理论上不会出现此情况，除非列表为空），则返回 -1。
        """
        # 初始化最小距离为正无穷大，方便后续比较找到真正的最小距离
        min_distance = float('inf')
        closest_index = -1
        # 初始化最小距离为正无穷大，方便后续比较找到真正的最小距离
        for i, waypoint in enumerate(waypoint_list):
            distance = waypoint.transform.location.distance(
                current_waypoint.transform.location)
            if distance < min_distance:
                min_distance = distance
                closest_index = i

        return closest_index
