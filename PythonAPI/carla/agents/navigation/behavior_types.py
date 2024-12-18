# 本作品在MIT许可证的条款下授权。
# 副本请查看 <https://opensource.org/licenses/MIT>。

"""
这个模块包含了每种行为的不同参数集。
"""

class Cautious:
    """谨慎型代理的类。"""
    max_speed = 40  # 最大速度
    speed_lim_dist = 6  # 速度限制距离
    speed_decrease = 12  # 速度减少量
    safety_time = 3  # 安全时间
    min_proximity_threshold = 12  # 最小接近阈值
    braking_distance = 6  # 制动距离
    tailgate_counter = 0  # 尾随计数器

class Normal:
    """普通型代理的类。"""
    max_speed = 50  # 最大速度
    speed_lim_dist = 3  # 速度限制距离
    speed_decrease = 10  # 速度减少量
    safety_time = 3  # 安全时间
    min_proximity_threshold = 10  # 最小接近阈值
    braking_distance = 5  # 制动距离
    tailgate_counter = 0  # 尾随计数器

class Aggressive:
    """激进型代理的类。"""
    max_speed = 70  # 最大速度
    speed_lim_dist = 1  # 速度限制距离
    speed_decrease = 8  # 速度减少量
    safety_time = 3  # 安全时间
    min_proximity_threshold = 8  # 最小接近阈值
    braking_distance = 4  # 制动距离
    tailgate_counter = -1  # 尾随计数器
