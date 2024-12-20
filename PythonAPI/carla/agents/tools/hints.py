"""
Module to add high-level semantic return types for obstacle and traffic light detection results via named tuples.

The code is compatible with Python 2.7, <3.6 and >=3.6. The later uses the typed version of named tuples.
"""


import sys
# 检查 Python 版本
if sys.version_info < (3, 6):
    # Python 2.7 或较低版本使用的方式
    from collections import namedtuple
    # 定义命名元组 ObstacleDetectionResult，用于表示障碍物检测结果
    # 包含以下字段：
    # - obstacle_was_found: 布尔值，表示是否发现了障碍物
    # - obstacle: 存储障碍物的对象，如果发现障碍物则存储对应的 Actor 对象，若无则为 None
    # - distance: 到障碍物的距离，单位为米
    ObstacleDetectionResult = namedtuple('ObstacleDetectionResult', ['obstacle_was_found', 'obstacle', 'distance'])
    
     # 定义命名元组 TrafficLightDetectionResult，用于表示交通灯检测结果
    # 包含以下字段：
    # - traffic_light_was_found: 布尔值，表示是否发现了交通灯
    # - traffic_light: 存储交通灯对象，如果发现交通灯则存储对应的 TrafficLight 对象，若无则为 None
    TrafficLightDetectionResult = namedtuple('TrafficLightDetectionResult', ['traffic_light_was_found', 'traffic_light'])
else:
    # Python 3.6+ 使用类型注解的命名元组
    from typing import NamedTuple, Union, TYPE_CHECKING
    from carla import Actor, TrafficLight

    """
    # Python 3.6+ 版本的代码，使用 NamedTuple 和类型注解进行定义
    class ObstacleDetectionResult(NamedTuple):
        obstacle_was_found : bool  # 是否发现障碍物
        obstacle : Union[Actor, None]  # 如果发现障碍物，则为 Actor 对象，否则为 None
        distance : float  # 到障碍物的距离，单位为米
        
    class TrafficLightDetectionResult(NamedTuple):
        traffic_light_was_found : bool  # 是否发现交通灯
        traffic_light : Union[TrafficLight, None]  # 如果发现交通灯，则为 TrafficLight 对象，否则为 None
    """
    
    # 为类型检查做准备
    if TYPE_CHECKING:
        # 导入 Literal，用于标注可能的特定值（如 Python 3.8+）
        from typing import Literal
        # 定义命名元组 ObstacleDetectionResult，使用更精细的类型注解，包括可能的 Literal 类型
        # - obstacle_was_found: 布尔值，表示是否发现障碍物
        # - obstacle: 如果发现障碍物，则为 Actor 对象，否则为 None
        # - distance: 如果有障碍物，表示距离的浮动值；如果没有障碍物，则可能为 -1
        ObstacleDetectionResult = NamedTuple('ObstacleDetectionResult', [('obstacle_was_found', bool), ('obstacle', Union[Actor, None]), ('distance', Union[float, Literal[-1]])])
    else:
        # 定义命名元组 ObstacleDetectionResult，适用于 Python 3.6 及以上版本
        ObstacleDetectionResult = NamedTuple('ObstacleDetectionResult', [('obstacle_was_found', bool), ('obstacle', Union[Actor, None]), ('distance', float)])
     # 定义命名元组 TrafficLightDetectionResult，表示交通灯检测结果
     # - traffic_light_was_found: 布尔值，表示是否发现交通灯
     # - traffic_light: 如果发现交通灯，则为 TrafficLight 对象，否则为 None
     TrafficLightDetectionResult = NamedTuple('TrafficLightDetectionResult', [('traffic_light_was_found', bool), ('traffic_light', Union[TrafficLight, None])])
