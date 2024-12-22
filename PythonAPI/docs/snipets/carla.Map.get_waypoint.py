
# This recipe shows the current traffic rules affecting the vehicle. 
# Shows the current lane type and if a lane change can be done in the actual lane or the surrounding ones.

# ...
waypoint = world.get_map().get_waypoint(vehicle.get_location(),project_to_road=True, lane_type=(carla.LaneType.Driving | carla.LaneType.Shoulder | carla.LaneType.Sidewalk))
# 获取车辆所在位置对应的路点（waypoint）信息
print("Current lane type: " + str(waypoint.lane_type))
# 打印当前车道的类型
print("Current Lane change:  " + str(waypoint.lane_change))
# 检查当前车道是否允许变道
print("L lane marking type: " + str(waypoint.left_lane_marking.type))# 打印左侧车道标线的类型
print("L lane marking change: " + str(waypoint.left_lane_marking.lane_change))# 打印左侧车道标线所对应的变道相关情况
print("R lane marking type: " + str(waypoint.right_lane_marking.type))# 打印右侧车道标线的类型
print("R lane marking change: " + str(waypoint.right_lane_marking.lane_change))# 打印右侧车道标线所对应的变道相关情况
# ...
