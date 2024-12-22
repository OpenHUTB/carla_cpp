# ==============================================================================
# 车辆路径追踪器
# 修改自lane_explorer.py示例，用于绘制和监控车辆在世界中的运动路径
# ==============================================================================

def track_vehicle_path(map, vehicle, debug, args):
    """
    追踪并可视化车辆的运动路径
    
    Args:
        map: CARLA地图对象
        vehicle: 要追踪的车辆对象
        debug: 调试绘图工具对象
        args: 配置参数
    """
    # 颜色定义
    COLORS = {
        'junction': cyan,      # 路口颜色
        'sidewalk': red,       # 人行道颜色
        'normal_road': green,  # 普通道路颜色
        'velocity': orange,    # 速度显示颜色
        'transform': white     # 变换标记颜色
    }
    
    # 要监控的车道类型
    MONITORED_LANES = (
        carla.LaneType.Driving |  # 行车道
        carla.LaneType.Shoulder | # 路肩
        carla.LaneType.Sidewalk   # 人行道
    )
    
    def calculate_velocity(vector):
        """计算车辆速度（km/h）"""
        return 3.6 * math.sqrt(vector.x**2 + vector.y**2 + vector.z**2)
    
    def draw_path_segment(current_w, next_w, is_sidewalk):
        """绘制路径段"""
        # 确定路段颜色
        color = (COLORS['junction'] if current_w.is_junction else 
                COLORS['sidewalk'] if is_sidewalk else 
                COLORS['normal_road'])
        
        # 绘制路径连接
        draw_waypoint_union(debug, current_w, next_w, color, 60)
        
        # 绘制当前位置的变换标记
        draw_transform(debug, current_w.transform, COLORS['transform'], 60)
    
    def draw_velocity_info(waypoint, velocity):
        """绘制速度信息"""
        speed_text = f'{calculate_velocity(velocity):15.0f} km/h'
        debug.draw_string(
            waypoint.transform.location,
            speed_text,
            False,
            COLORS['velocity'],
            60
        )
    
    # 初始化当前路点
    current_waypoint = map.get_waypoint(vehicle.get_location())
    
    # 主循环：持续追踪车辆路径
    while True:
        # 获取下一个路点
        next_waypoint = map.get_waypoint(
            vehicle.get_location(),
            lane_type=MONITORED_LANES
        )
        
        # 检查车辆是否移动到新位置
        if next_waypoint.id != current_waypoint.id:
            # 获取当前速度
            velocity = vehicle.get_velocity()
            
            # 绘制路径可视化
            draw_path_segment(
                current_waypoint,
                next_waypoint,
                current_waypoint.lane_type == carla.LaneType.Sidewalk
            )
            
            # 显示速度信息
            draw_velocity_info(current_waypoint, velocity)
            
            # 更新当前路点
            current_waypoint = next_waypoint
        
        # 按指定时间间隔暂停
        time.sleep(args.tick_time)

# 使用示例：
# track_vehicle_path(world.get_map(), vehicle, world.debug, args)
