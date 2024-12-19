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

    import numpy as np
import networkx as nx
import carla
from typing import List, Optional, Tuple, Dict

# 假设这里有适当的RoadOption等相关类和类型定义，以下是示例示意
class RoadOption:
    LANEFOLLOW = 1
    CHANGELANERIGHT = 2
    CHANGELANELEFT = 3

class EdgeDict:
    pass

# 这里假设是某个包含道路拓扑相关处理的类中的方法（因为有self参数），以下是类的示例示意
class RoadTopologyHandler:
    def __init__(self):
        self._sampling_resolution = 1.0  # 示例采样分辨率，实际应该根据具体需求初始化
        self._topology = []  # 存储道路拓扑的相关信息，例如道路段信息等
        self._road_id_to_edge = {}  # 用于映射道路ID到对应边信息的字典
        self._id_map = {}  # 用于节点ID映射的相关字典
        self._graph = nx.Graph()  # 用于构建道路拓扑图

    def _find_loose_ends(self):
        """
        This method finds road segments that have an unconnected end, and
        adds them to the internal graph representation
        """
        count_loose_ends = 0
        hop_resolution = self._sampling_resolution
        # 遍历所有道路段信息
        for segment in self._topology:
            end_wp = segment['exit']  # 获取道路段的出口路点
            exit_xyz = segment['exitxyz']  # 获取出口的坐标信息（可能是xyz坐标元组之类）
            road_id, section_id, lane_id = end_wp.road_id, end_wp.section_id, end_wp.lane_id
            # 判断当前道路段出口对应的道路、路段、车道是否已经在边映射中存在
            if road_id in self._road_id_to_edge \
                    and section_id in self._road_id_to_edge[road_id] \
                    and lane_id in self._road_id_to_edge[road_id][section_id]:
                pass  # 如果存在则跳过后续处理（可能之前已经处理过该连接情况）
            else:
                count_loose_ends += 1
                # 如果当前道路ID不在映射字典中，初始化对应的值为一个空字典
                if road_id not in self._road_id_to_edge:
                    self._road_id_to_edge[road_id] = dict()
                # 如果当前路段ID不在对应道路的字典中，初始化对应的值为一个空字典
                if section_id not in self._road_id_to_edge[road_id]:
                    self._road_id_to_edge[road_id][section_id] = dict()
                n1 = self._id_map[exit_xyz]
                n2 = -1 * count_loose_ends
                # 将当前道路段出口对应的节点信息存入映射字典
                self._road_id_to_edge[road_id][section_id][lane_id] = (n1, n2)
                next_wp = end_wp.next(hop_resolution)  # 获取下一个路点（按照一定分辨率跳跃获取）
                path = []  # 初始化路径列表，用于存储后续连续的同车道路点
                # 循环获取同车道的路点，直到条件不满足（例如换车道、到道路尽头等）
                while next_wp is not None and next_wp \
                        and next_wp[0].road_id == road_id \
                        and next_wp[0].section_id == section_id \
                        and next_wp[0].lane_id == lane_id:
                    path.append(next_wp[0])
                    next_wp = next_wp[0].next(hop_resolution)
                if path:
                    n2_xyz = (path[-1].transform.location.x,
                              path[-1].transform.location.y,
                              path[-1].transform.location.z)
                    # 在图中添加节点，传入节点ID和坐标信息
                    self._graph.add_node(n2, vertex=n2_xyz)
                    # 在图中添加边，连接两个节点，并传入相关属性信息，如长度、路径、入口路点、出口路点等
                    self._graph.add_edge(
                        n1, n2,
                        length=len(path) + 1, path=path,
                        entry_waypoint=end_wp, exit_waypoint=path[-1],
                        entry_vector=None, exit_vector=None, net_vector=None,
                        intersection=end_wp.is_junction, type=RoadOption.LANEFOLLOW)

    def _lane_change_link(self):
        """
        This method places zero cost links in the topology graph
        representing availability of lane changes.
        """

        for segment in self._topology:
            left_found, right_found = False, False

            for waypoint in segment['path']:
                if not segment['entry'].is_junction:
                    next_waypoint, next_road_option, next_segment = None, None, None

                    # 判断是否可以向右变道
                    if waypoint.right_lane_marking and waypoint.right_lane_marking.lane_change & carla.LaneChange.Right and not right_found:
                        next_waypoint = waypoint.get_right_lane()
                        if next_waypoint is not None \
                                and next_waypoint.lane_type == carla.LaneType.Driving \
                                and waypoint.road_id == next_waypoint.road_id:
                            next_road_option = RoadOption.CHANGELANERIGHT
                            next_segment = self._localize(next_waypoint.transform.location)
                            if next_segment is not None:
                                # 在图中添加表示向右变道的边，传入相关属性信息，如起点节点、终点节点、入口路点、出口路点等
                                self._graph.add_edge(
                                    self._id_map[segment['entryxyz']], next_segment[0], entry_waypoint=waypoint,
                                    exit_waypoint=next_waypoint, intersection=False, exit_vector=None,
                                    path=[], length=0, type=next_road_option, change_waypoint=next_waypoint)
                                right_found = True
                    # 判断是否可以向左变道
                    if waypoint.left_lane_marking and waypoint.left_lane_marking.lane_change & carla.LaneChange.Left and not left_found:
                        next_waypoint = waypoint.get_left_lane()
                        if next_waypoint is not None \
                                and next_waypoint.lane_type == carla.LaneType.Driving \
                                and waypoint.road_id == next_waypoint.road_id:
                            next_road_option = RoadOption.CHANGELANELEFT
                            next_segment = self._localize(next_waypoint.transform.location)
                            if next_segment is not None:
                                # 在图中添加表示向左变道的边，传入相关属性信息，如起点节点、终点节点、入口路点、出口路点等
                                self._graph.add_edge(
                                    self._id_map[segment['entryxyz']], next_segment[0], entry_waypoint=waypoint,
                                    exit_waypoint=next_waypoint, intersection=False, exit_vector=None,
                                    path=[], length=0, type=next_road_option, change_waypoint=next_waypoint)
                                left_found = True
                if left_found and right_found:
                    break

    def _localize(self, location):
        # type: (carla.Location) -> Optional[Tuple[int, int]]
        """
        This function finds the road segment that a given location
        is part of, returning the edge it belongs to
        """
        waypoint = self._wmap.get_waypoint(location)  # 根据给定位置获取对应的路点
        edge = None  # 初始化边信息为None
        try:
            # 尝试从映射字典中获取对应路点所在的边信息（道路、路段、车道对应的边）
            edge = self._road_id_to_edge[waypoint.road_id][waypoint.section_id][waypoint.lane_id]
        except KeyError:
            pass
        return edge

    def _distance_heuristic(self, n1, n2):
        """
        Distance heuristic calculator for path searching
        in self._graph
        """
        l1 = np.array(self._graph.nodes[n1]['vertex'])  # 获取起点节点的坐标信息（转换为numpy数组形式）
        l2 = np.array(self._graph.nodes[n2]['vertex'])  # 获取终点节点的坐标信息（转换为numpy数组形式）
        return np.linalg.norm(l1 - l2)  # 计算两点间的欧几里得距离作为启发式距离

    def _path_search(self, origin, destination):
        # type: (carla.Location, carla.Location) -> List[int]
        """
        This function finds the shortest path connecting origin and destination
        using A* search with distance heuristic.
        origin      :   carla.Location object of start position
        destination :   carla.Location object of of end position
        return      :   path as list of node ids (as int) of the graph self._graph
        connecting origin and destination
        """
        start, end = self._localize(origin), self._localize(destination)
        # 使用A*算法在图中搜索从起点到终点的最短路径，传入图、起点、终点、启发式函数、边权重属性等信息
        route = nx.astar_path(
            self._graph, source=start[0], target=end[0],
            heuristic=self._distance_heuristic, weight='length')
        route.append(end[1])  # 将终点的另一个节点ID添加到路径中（根据具体逻辑，可能是完整路径需要的操作）
        return route

    def _successive_last_intersection_edge(self, index, route):
        # type: (int, List[int]) -> Tuple[Optional[int], Optional[EdgeDict]]
        """
        This method returns the last successive intersection edge
        from a starting index on the route.
        This helps moving past tiny intersection edges to calculate
        proper turn decisions.
        """

        last_intersection_edge = None  # 初始化最后交叉路口边信息为None
        last_node = None
        # 遍历路径上的节点对（从给定索引开始，到倒数第二个节点为止）
        for node1, node2 in [(route[i], route[i + 1]) for i in range(index, len(route) - 1)]:
            candidate_edge = self._graph.edges[node1, node2]  # 获取两个节点间的边信息
            if node1 == route[index]:
                last_intersection_edge = candidate_edge
            # 如果边类型是车道跟随且处于交叉路口，则更新最后交叉路口边信息以及最后节点
            if candidate_edge['type'] == RoadOption.LANEFOLLOW and candidate_edge['intersection']:
                last_intersection_edge = candidate_edge
                last_node = node2
            else:
                break

        return last_node, last_intersection_edge

    def _turn_decision(self, index, route, threshold=math.radians(35)):
        # type: (int, list[int], float) -> RoadOption
        """
        此方法根据路线（route）列表中当前索引（index）位置附近的边对信息，返回对应的转弯决策（RoadOption类型）。
        通过分析当前边与下一条边的属性以及它们之间的向量关系等来判断是左转、右转还是直行等情况。

        参数：
        index：路线列表（route）中的索引位置，指明要分析的边对在路线中的位置。
        route：由节点ID组成的列表，表示从起点到终点的完整路线。
        threshold：角度阈值（以弧度为单位），用于判断两条边之间的夹角是否足够小以认定为直行情况，默认值为35度对应的弧度值。

        返回值：
        返回一个RoadOption类型的值，表示做出的转弯决策（例如RoadOption.LEFT表示左转等）。
        """

        decision = None
        # 获取路线中当前索引前一个节点的ID，用于后续与当前边等信息结合分析
        previous_node = route[index - 1]
        # 获取路线中当前索引对应的节点的ID，作为当前边的起始节点
        current_node = route[index]
        # 获取路线中当前索引后一个节点的ID，作为当前边的结束节点（即下一条边的起始节点）
        next_node = route[index + 1]
        # 从表示道路拓扑图（假设为self._graph）的对象中获取当前节点与下一个节点之间的边信息，类型标注为EdgeDict，其中包含边的各种属性
        next_edge = self._graph.edges[current_node, next_node]  # type: EdgeDict

        if index > 0:
            # 当当前索引大于0时（即不是路线中的第一个节点，存在前序边可供参考对比），进行以下判断：
            # 如果满足以下多个条件，则直接采用上一次的决策作为本次的转弯决策：
            # 1. 上一次的决策不是VOID，说明之前已经有过有效的决策记录。
            # 2. 交叉路口结束节点编号大于0，表示之前已经确定过交叉路口相关的有效结束节点（可能在之前的处理中标记过）。
            # 3. 交叉路口结束节点编号不等于前一个节点的编号，说明当前的边对情况符合参考上一次决策的条件。
            # 4. 下一条边的类型是车道跟随（LANEFOLLOW），表示正常沿着车道行驶的情况。
            # 5. 下一条边处于交叉路口（intersection属性为True），说明即将进入或处于交叉路口场景。
            if self._previous_decision!= RoadOption.VOID \
                    and self._intersection_end_node > 0 \
                    and self._intersection_end_node!= previous_node \
                    and next_edge['type'] == RoadOption.LANEFOLLOW \
                    and next_edge['intersection']:
                decision = self._previous_decision
            else:
                self._intersection_end_node = -1
                # 获取前一个节点与当前节点之间的边信息，同样类型为EdgeDict，包含边的相关属性，用于后续分析
                current_edge = self._graph.edges[previous_node, current_node]  # type: EdgeDict
                # 判断是否需要进行详细的转弯决策计算的条件：
                # 当前边的类型是车道跟随（LANEFOLLOW）且当前边不在交叉路口（not current_edge['intersection']为True），
                # 同时下一条边的类型是车道跟随（LANEFOLLOW）且下一条边处于交叉路口（next_edge['intersection']为True），
                # 这种情况意味着车辆即将进入交叉路口，需要通过进一步分析来确定转弯方向等决策。
                calculate_turn = current_edge['type'] == RoadOption.LANEFOLLOW and not current_edge[
                    'intersection'] and next_edge['type'] == RoadOption.LANEFOLLOW and next_edge['intersection']
                if calculate_turn:
                    # 调用另一个方法（_successive_last_intersection_edge）获取从当前索引开始的路线上最后连续交叉路口边缘相关信息，
                    # 包括最后一个处于连续交叉路口情况的节点编号以及对应的边信息，这有助于更准确地处理连续经过交叉路口小段边的场景，避免误判转弯情况。
                    last_node, tail_edge = self._successive_last_intersection_edge(index, route)
                    self._intersection_end_node = last_node
                    if tail_edge is not None:
                        # 如果获取到了最后连续交叉路口对应的边信息（tail_edge），则用它替换原本获取的下一条边信息（next_edge），
                        # 以便后续基于更准确的边信息进行转弯决策相关的计算和判断。
                        next_edge = tail_edge
                    # 获取当前边的出口向量（exit_vector），用于后续计算两条边之间的角度关系等，可能表示边的方向信息（具体含义根据实际情况而定）。
                    # 获取下一条边的出口向量，同样用于角度关系等计算，以判断车辆行驶方向的变化情况。
                    cv, nv = current_edge['exit_vector'], next_edge['exit_vector']
                    if cv is None or nv is None:
                        # 如果当前边或者下一条边的出口向量为None（可能是数据缺失或未正确初始化等原因），
                        # 则直接返回下一条边的类型作为本次的转弯决策（无法进行更准确的角度等相关分析了）。
                        return next_edge['type']
                    cross_list = []
                    # 遍历当前节点在图中的所有后继节点（即邻居节点），目的是收集与当前边相关的交叉向量信息，
                    # 用于后续与当前边和下一条边的向量关系进行对比分析，辅助判断转弯方向。
                    for neighbor in self._graph.successors(current_node):
                        select_edge = self._graph.edges[current_node, neighbor]
                        if select_edge['type'] == RoadOption.LANEFOLLOW:
                            if neighbor!= route[index + 1]:
                                # 对于类型为车道跟随（LANEFOLLOW）且不是下一条边（避免重复计算下一条边相关情况）的邻居节点对应的边，
                                # 获取该边的网络向量（net_vector，具体含义根据实际应用场景而定，可能与道路网络的方向布局等有关），
                                # 并计算该网络向量与当前边出口向量的叉积的z分量，将结果添加到交叉向量列表（cross_list）中，
                                # 这些叉积结果可以帮助判断当前边与其他相邻边在二维平面上的相对位置关系，进而辅助确定转弯方向。
                                sv = select_edge['net_vector']
                                cross_list.append(np.cross(cv, sv)[2])
                    # 计算当前边出口向量和下一条边出口向量的叉积的z分量，同样用于判断两条边在二维平面上的相对位置关系，
                    # 结合叉积的方向特性（在二维平面上可以辅助判断左右关系等）来分析转弯情况。
                    next_cross = np.cross(cv, nv)[2]
                    # 计算当前边出口向量和下一条边出口向量的夹角（通过向量点积公式计算夹角的余弦值，再使用math.acos函数获取夹角弧度值），
                    # 同时使用np.clip函数确保点积结果除以向量模长乘积得到的余弦值在[-1, 1]范围内，避免出现数学计算错误，
                    # 这个夹角可以直观反映两条边方向上的偏差程度，用于判断是直行还是转弯等情况。
                    deviation = math.acos(np.clip(
                        np.dot(cv, nv) / (np.linalg.norm(cv) * np.linalg.norm(nv)), -1.0, 1.0))
                    if not cross_list:
                        cross_list.append(0)
                    # 根据计算得到的夹角与设定的阈值比较以及交叉向量列表中的相关信息来确定最终的转弯决策：
                    if deviation < threshold:
                        # 如果两条边的夹角小于设定的阈值（threshold），认为车辆基本是沿着直线行驶，决策为直行（RoadOption.STRAIGHT）。
                        decision = RoadOption.STRAIGHT
                    elif cross_list and next_cross < min(cross_list):
                        # 如果交叉向量列表不为空，且当前边与下一条边叉积的z分量小于交叉向量列表中的最小值，
                        # 说明当前边相对于其他相邻边更偏向左边，决策为左转（RoadOption.LEFT）。
                        decision = RoadOption.LEFT
                    elif cross_list and next_cross > max(cross_list):
                        # 如果交叉向量列表不为空，且当前边与下一条边叉积的z分量大于交叉向量列表中的最大值，
                        # 说明当前边相对于其他相邻边更偏向右边，决策为右转（RoadOption.RIGHT）。
                        decision = RoadOption.RIGHT
                    elif next_cross < 0:
                        # 如果当前边与下一条边叉积的z分量小于0，基于叉积在二维平面上判断左右方向的常规逻辑，决策为左转（RoadOption.LEFT）。
                        decision = RoadOption.LEFT
                    elif next_cross > 0:
                        # 如果当前边与下一条边叉积的z分量大于0，基于叉积在二维平面上判断左右方向的常规逻辑，决策为右转（RoadOption.RIGHT）。
                        decision = RoadOption.RIGHT
                else:
                    # 如果不满足需要进行详细转弯决策计算的条件（calculate_turn为False），则直接采用下一条边的类型作为本次的转弯决策。
                    decision = next_edge['type']

        else:
            # 如果当前索引为0（即处理路线中的第一个边对，不存在前序边可供参考对比），直接采用下一条边的类型作为转弯决策。
            decision = next_edge['type']

        self._previous_decision = decision
        return decision

    def _find_closest_in_list(self, current_waypoint, waypoint_list):
        """
        在给定的路点列表（waypoint_list）中，查找距离当前路点（current_waypoint）最近的路点，并返回其在列表中的索引。
        通过遍历列表，计算每个路点与当前路点之间的距离，比较得出距离最小的路点对应的索引。

        参数：
        current_waypoint：作为参考的当前路点，用于与列表中的路点计算距离进行比较。
        waypoint_list：包含多个路点的列表，要在其中查找距离当前路点最近的路点。

        返回值：
        返回waypoint_list中距离current_waypoint最近的路点的索引值，如果列表为空，则返回 -1。
        """
        min_distance = float('inf')
        closest_index = -1
        for i, waypoint in enumerate(waypoint_list):
            # 计算当前路点（current_waypoint）与列表中每个路点（waypoint）之间的距离，
            # 这里通过路点的位置坐标（假设transform.location表示路点的位置信息）来计算两点间的距离，
            # 具体的距离计算方式可能是欧几里得距离或者其他符合实际场景需求的距离度量方式。
            distance = waypoint.transform.location.distance(
                current_waypoint.transform.location)
            if distance < min_distance:
                min_distance = distance
                closest_index = i

        return closest_index
