
# This recipe is a modification of lane_explorer.py example.
# It draws the path of an actor through the world, printing information at each waypoint.

# ...
current_w = map.get_waypoint(vehicle.get_location())
while True:
 # 获取下一个路点，指定了要考虑的车道类型，包括行车道、路肩以及人行道类型的车道
    next_w = map.get_waypoint(vehicle.get_location(), lane_type=carla.LaneType.Driving | carla.LaneType.Shoulder | carla.LaneType.Sidewalk )
    # 检查车辆是否在移动，通过比较当前路点和下一个路点的id是否不同来判断
    if next_w.id != current_w.id:
        vector = vehicle.get_velocity()
        # 检查车辆当前是否处于人行道上
        if current_w.lane_type == carla.LaneType.Sidewalk:
            draw_waypoint_union(debug, current_w, next_w, cyan if current_w.is_junction else red, 60)
        else:
            draw_waypoint_union(debug, current_w, next_w, cyan if current_w.is_junction else green, 60)
        debug.draw_string(current_w.transform.location, str('%15.0f km/h' % (3.6 * math.sqrt(vector.x**2 + vector.y**2 + vector.z**2))), False, orange, 60)
        draw_transform(debug, current_w.transform, white, 60)

    # 更新当前路点为下一个路点，并按照给定的时间间隔（args.tick_time）休眠一段时间，以控制循环的节奏
    current_w = next_w
    time.sleep(args.tick_time)
# ...
