#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys

# 尝试将Carla库对应的模块路径添加到Python的系统路径中，以便后续能正确导入carla模块。
# 它会根据当前Python版本（主版本号sys.version_info.major和次版本号sys.version_info.minor）以及操作系统类型（通过os.name判断，'nt'表示Windows，'linux-x86_64'表示Linux），
# 去查找符合特定命名格式（carla-*%d.%d-%s.egg）的.egg文件（Carla库的一种打包格式），使用glob.glob获取匹配的文件路径列表，取第一个路径（[0]）添加到sys.path中。
# 如果没找到匹配文件（触发IndexError异常），则直接跳过添加操作，继续往下执行代码。
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla
import math
import random

# 定义一个函数，用于获取特定的坐标变换（Transform）信息，这个变换通常用于在Carla模拟环境中设置物体的位置和姿态。
# 参数vehicle_location表示车辆的位置（carla.Location类型），angle表示角度（单位可能是角度制，后续会转换为弧度制），d是一个距离参数，默认值为6.4，用于计算相对位置。
def get_transform(vehicle_location, angle, d=6.4):
    # 将输入的角度从角度制转换为弧度制，因为在后续的三角函数计算中需要使用弧度制。
    a = math.radians(angle)
    # 根据给定的距离d、角度a（已转换为弧度制）以及车辆的位置vehicle_location，计算出一个新的位置location。
    # 这里通过三角函数计算在平面上相对于车辆位置的偏移量，然后加上车辆位置得到新位置，并且将新位置的z轴坐标设置为2.0（可能是为了让物体处于某个合适的高度）。
    location = carla.Location(d * math.cos(a), d * math.sin(a), 2.0) + vehicle_location
    # 创建并返回一个carla.Transform对象，它包含了新计算出的位置location以及一个旋转信息（Rotation），旋转角度在yaw方向上设置为180 + angle（可能是特定的朝向要求），pitch方向设置为 -15（同样可能是特定姿态需求）。
    return carla.Transform(location, carla.Rotation(yaw=180 + angle, pitch=-15))


def main():
    # 创建一个Carla客户端对象，连接到本地主机（'localhost'）的2000端口，这是与Carla服务器进行通信的入口点。
    client = carla.Client('localhost', 2000)
    # 设置客户端的超时时间为2.0秒，意味着如果客户端向服务器发送请求后，在2.0秒内没有收到服务器的响应，就会判定此次操作超时，避免程序长时间无响应地等待。
    client.set_timeout(2.0)
    # 通过客户端获取Carla模拟世界（world）对象，这个世界对象包含了模拟场景中的各种实体、设置等信息，后续操作大多基于这个世界对象展开。
    world = client.get_world()
    # 从世界对象中获取观察者（spectator）对象，观察者通常用于控制虚拟摄像机的位置和视角，决定在模拟场景中看到的画面内容。
    spectator = world.get_spectator()
    # 从世界对象的蓝图库（blueprint_library）中筛选出所有代表车辆（'vehicle'）的蓝图（blueprint），蓝图可以理解为创建具体车辆实例的模板，包含了车辆的各种属性和配置信息。
    vehicle_blueprints = world.get_blueprint_library().filter('vehicle')

    # 从世界地图（world.get_map()）的所有出生点（spawn_points）中随机选择一个出生点的位置（location），这个出生点就是后续创建车辆的初始位置。
    location = random.choice(world.get_map().get_spawn_points()).location

    # 遍历筛选出的所有车辆蓝图，意味着会为每个车辆蓝图创建对应的车辆实例。
    for blueprint in vehicle_blueprints:
        # 创建一个坐标变换（Transform）对象，将位置设置为前面随机选到的位置location，旋转角度在yaw方向上设置为 -45.0度，这个变换用于指定车辆创建时的初始位置和姿态。
        transform = carla.Transform(location, carla.Rotation(yaw=-45.0))
        # 使用世界对象的spawn_actor方法，根据当前的车辆蓝图blueprint和坐标变换transform，在模拟世界中创建一个车辆实例，返回创建好的车辆对象。
        vehicle = world.spawn_actor(blueprint, transform)

        try:
            # 打印出当前创建的车辆的类型ID（type_id），可以通过这个ID识别车辆的具体类型，比如是轿车、卡车等不同类型的车辆。
            print(vehicle.type_id)

            # 将角度angle初始化为0，这个角度后续会用于控制观察者的视角旋转等操作。
            angle = 0
            # 开始一个循环，只要角度angle小于356度（接近一圈但不到一圈，可能是为了在一圈内进行相关操作），就持续执行循环体内容。
            while angle < 356:
                # 让世界对象等待一次时间推进（tick），并获取此次tick对应的时间戳（timestamp）信息，时间戳包含了时间相关的数据，比如时间间隔等。
                timestamp = world.wait_for_tick().timestamp
                # 根据时间戳中的时间间隔（delta_seconds）信息，以一定的速率更新角度angle，这里乘以60.0可能是为了让角度按照某种期望的速度变化（具体速度取决于模拟世界的时间设置等因素）。
                angle += timestamp.delta_seconds * 60.0
                # 使用前面定义的get_transform函数，根据车辆的当前位置（vehicle.get_location()）和更新后的角度（angle - 90，可能是为了调整到合适的相对角度关系），获取一个新的坐标变换信息，
                # 然后使用这个坐标变换信息来设置观察者（spectator）的位置和姿态，从而实现从不同角度观察车辆的效果，让观察者围绕车辆旋转观察。
                spectator.set_transform(get_transform(vehicle.get_location(), angle - 90))

        finally:
            # 无论在try块中是否发生异常，最终都要销毁创建的车辆实例，释放相关资源，避免内存泄漏等问题，保持模拟世界的整洁。
            vehicle.destroy()


if __name__ == '__main__':
    main()
