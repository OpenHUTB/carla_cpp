#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys
#尝试将Carla的Python包路径添加到sys.path，以便可以导入Carla模块。如果失败（例如，如果没有找到匹配的.egg文件），则忽略。
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

import argparse
import random
import time

#定义一些Carla颜色，用于在仿真中绘制不同的图形。
red = carla.Color(255, 0, 0)
green = carla.Color(0, 255, 0)
blue = carla.Color(47, 210, 231)
cyan = carla.Color(0, 255, 255)
yellow = carla.Color(255, 255, 0)
orange = carla.Color(255, 162, 0)
white = carla.Color(255, 255, 255)

#定义轨迹生命周期（trail_life_time）和路径点间隔（waypoint_separation）。
trail_life_time = 10
waypoint_separation = 4


   #定义一个函数draw_transform，用于在仿真中绘制变换（位置和方向）。
def draw_transform(debug, trans, col=carla.Color(255, 0, 0), lt=-1):
    #在draw_transform函数中，使用debug.draw_arrow绘制一个箭头，表示给定变换的方向。
    debug.draw_arrow(
        trans.location, trans.location + trans.get_forward_vector(),
        thickness=0.05, arrow_size=0.1, color=col, life_time=lt)


 #定义一个函数draw_waypoint_union，用于绘制两个路径点之间的连线。
def draw_waypoint_union(debug, w0, w1, color=carla.Color(255, 0, 0), lt=5):
   #在draw_waypoint_union函数中，绘制一条线和一个点，表示路径点的位置。
    debug.draw_line(
        w0.transform.location + carla.Location(z=0.25),
        w1.transform.location + carla.Location(z=0.25),
        thickness=0.1, color=color, life_time=lt, persistent_lines=False)
    debug.draw_point(w1.transform.location + carla.Location(z=0.25), 0.1, color, lt, False)


    #定义一个函数draw_waypoint_info，用于在仿真中显示路径点的相关信息。
def draw_waypoint_info(debug, w, lt=5):
#在draw_waypoint_info函数中，绘制字符串，显示路径点的车道ID、道路ID和车道变化信息。
    w_loc = w.transform.location
    debug.draw_string(w_loc + carla.Location(z=0.5), "lane: " + str(w.lane_id), False, yellow, lt)
    debug.draw_string(w_loc + carla.Location(z=1.0), "road: " + str(w.road_id), False, blue, lt)
    debug.draw_string(w_loc + carla.Location(z=-.5), str(w.lane_change), False, red, lt)

 #定义一个函数draw_junction，用于绘制路口的边界框和每个车道的起始和结束路径点。
def draw_junction(debug, junction, l_time=10):
   #在draw_junction函数中，绘制路口的边界框和每个车道的起始和结束路径点
    """Draws a junction bounding box and the initial and final waypoint of every lane."""
    # draw bounding box
    box = junction.bounding_box
    point1 = box.location + carla.Location(x=box.extent.x, y=box.extent.y, z=2)
    point2 = box.location + carla.Location(x=-box.extent.x, y=box.extent.y, z=2)
    point3 = box.location + carla.Location(x=-box.extent.x, y=-box.extent.y, z=2)
    point4 = box.location + carla.Location(x=box.extent.x, y=-box.extent.y, z=2)
    debug.draw_line(
        point1, point2,
        thickness=0.1, color=orange, life_time=l_time, persistent_lines=False)
    debug.draw_line(
        point2, point3,
        thickness=0.1, color=orange, life_time=l_time, persistent_lines=False)
    debug.draw_line(
        point3, point4,
        thickness=0.1, color=orange, life_time=l_time, persistent_lines=False)
    debug.draw_line(
        point4, point1,
        thickness=0.1, color=orange, life_time=l_time, persistent_lines=False)
    # draw junction pairs (begin-end) of every lane
    junction_w = junction.get_waypoints(carla.LaneType.Any)
    for pair_w in junction_w:
        draw_transform(debug, pair_w[0].transform, orange, l_time)
        debug.draw_point(
            pair_w[0].transform.location + carla.Location(z=0.75), 0.1, orange, l_time, False)
        draw_transform(debug, pair_w[1].transform, orange, l_time)
        debug.draw_point(
            pair_w[1].transform.location + carla.Location(z=0.75), 0.1, orange, l_time, False)
        debug.draw_line(
            pair_w[0].transform.location + carla.Location(z=0.75),
            pair_w[1].transform.location + carla.Location(z=0.75), 0.1, white, l_time, False)


def main():
    argparser = argparse.ArgumentParser()# 创建 ArgumentParser 对象，用于处理命令行参数
    argparser.add_argument(
        '--host',#添加 --host 参数，用于指定主机IP地址
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',# 参数的短名称和长名称
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-i', '--info', # 添加显示信息的参数，若指定该参数则显示相关文本信息，布尔类型
        action='store_true',
        help='Show text information')  
    argparser.add_argument(
        '-x',
        default=0.0,
        type=float,
        help='X start position (default: 0.0)')# 添加 -x 参数，用于指定X轴起始位置 ，如果不指定，则默认为 0.0
    argparser.add_argument(
        '-y',
        default=0.0,
        type=float,
        help='Y start position (default: 0.0)')
    argparser.add_argument(
        '-z', # 添加Z轴起始位置参数，默认值是0.0，类型为浮点数
        default=0.0,
        type=float,
        help='Z start position (default: 0.0)')
    argparser.add_argument(
        '-s', '--seed',
        metavar='S',
        default=os.getpid(),
        type=int,
        help='Seed for the random path (default: program pid)')
    argparser.add_argument(
        '-t', '--tick-time',
        metavar='T',
        default=0.2,
        type=float,
        help='Tick time between updates (forward velocity) (default: 0.2)')
    args = argparser.parse_args()

    try:
        # 创建Carla客户端，连接到指定IP和端口的服务器
        client = carla.Client(args.host, args.port)
        client.set_timeout(2.0)

        # 获取Carla世界对象、地图对象以及调试工具对象
        world = client.get_world()
        m = world.get_map()
        debug = world.debug

        # 设置随机种子，方便复现随机结果，并打印种子值
        random.seed(args.seed)
        print("Seed: ", args.seed)

        # 根据传入的起始坐标创建位置对象，并打印初始位置
        loc = carla.Location(args.x, args.y, args.z)
        print("Initial location: ", loc)

        # 获取初始位置对应的路径点
        current_w = m.get_waypoint(loc)

        # 主循环
        while True:
            # list of potential next waypoints
            potential_w = list(current_w.next(waypoint_separation))

            # 检查可用的右侧行车道
            if current_w.lane_change & carla.LaneChange.Right:
                right_w = current_w.get_right_lane()
                if right_w and right_w.lane_type == carla.LaneType.Driving:
                    potential_w += list(right_w.next(waypoint_separation))

            # 检查可用的左侧行车道。
            if current_w.lane_change & carla.LaneChange.Left:
                left_w = current_w.get_left_lane()
                if left_w and left_w.lane_type == carla.LaneType.Driving:
                    potential_w += list(left_w.next(waypoint_separation))

            #选择一个随机的路径点作为下一个目标。
            next_w = random.choice(potential_w)
            potential_w.remove(next_w)

            #渲染一些有用的信息，请注意，如果您使用的是编辑器相机，您将无法看到这些字符串。
            if args.info:
                draw_waypoint_info(debug, current_w, trail_life_time)
            draw_waypoint_union(debug, current_w, next_w, cyan if current_w.is_junction else green, trail_life_time)
            draw_transform(debug, current_w.transform, white, trail_life_time)

            #打印剩余的路径点。
            for p in potential_w:
                draw_waypoint_union(debug, current_w, p, red, trail_life_time)
                draw_transform(debug, p.transform, white, trail_life_time)

            # 绘制所有的路口路径点和边界框。
            if next_w.is_junction:
                junction = next_w.get_junction()
                draw_junction(debug, junction, trail_life_time)

            #更新当前路径点并暂停一段时间。
            current_w = next_w
            time.sleep(args.tick_time)

    finally:
        pass


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('\nExit by user.')
    finally:
        print('\nExit.')
