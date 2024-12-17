"""
Module to add high-level semantic return types for obstacle and traffic light detection results via named tuples.

The code is compatible with Python 2.7, <3.6 and >=3.6. The later uses the typed version of named tuples.
"""


import sys
# 检查 Python 版本，如果小于 3.6
if sys.version_info < (3, 6):
    from collections import namedtuple
    # 定义一个名为 ObstacleDetectionResult 的命名元组，包含三个字段
    # obstacle_was_found: 表示是否发现障碍物
    # obstacle: 存储发现的障碍物（如果有）
    # distance: 表示到障碍物的距离
    ObstacleDetectionResult = namedtuple('ObstacleDetectionResult', ['obstacle_was_found', 'obstacle', 'distance'])
    # 定义一个名为 TrafficLightDetectionResult 的命名元组，包含两个字段
    # traffic_light_was_found: 表示是否发现交通灯
    # traffic_light: 存储发现的交通灯（如果有）
    TrafficLightDetectionResult = namedtuple('TrafficLightDetectionResult', ['traffic_light_was_found', 'traffic_light'])
else:
    from typing import NamedTuple, Union, TYPE_CHECKING
    from carla import Actor, TrafficLight
    """
    # Python 3.6+, incompatible with Python 2.7 syntax
    class ObstacleDetectionResult(NamedTuple):
        obstacle_was_found : bool
        obstacle : Union[Actor, None]
        distance : float
        # distance : Union[float, Literal[-1]] # Python 3.8+ only
    class TrafficLightDetectionResult(NamedTuple):
        traffic_light_was_found : bool
        traffic_light : Union[TrafficLight, None]
    """
    # 用于类型检查的条件
    if TYPE_CHECKING:
        from typing import Literal
        # 定义 ObstacleDetectionResult 命名元组，使用更详细的类型注解，包括可能的 Literal 类型（Python 3.8+）
        ObstacleDetectionResult = NamedTuple('ObstacleDetectionResult', [('obstacle_was_found', bool), ('obstacle', Union[Actor, None]), ('distance', Union[float, Literal[-1]])])
    else:
        # 定义 ObstacleDetectionResult 命名元组，使用基本类型注解
        ObstacleDetectionResult = NamedTuple('ObstacleDetectionResult', [('obstacle_was_found', bool), ('obstacle', Union[Actor, None]), ('distance', float)])
    # 定义 TrafficLightDetectionResult 命名元组，包含交通灯是否被发现和交通灯对象（如果有）
    TrafficLightDetectionResult = NamedTuple('TrafficLightDetectionResult', [('traffic_light_was_found', bool), ('traffic_light', Union[TrafficLight, None])])
