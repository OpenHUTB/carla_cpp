#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Vehicle physics example for CARLA
Small example that shows the effect of different impulse and force application
methods to a vehicle.
"""

import glob
import os
import sys
import argparse

try:
    # 将 CARLA 库的路径添加到系统路径中，根据操作系统和 Python 版本选择正确的文件
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla


def print_step_info(world, vehicle):
    """
    打印世界快照和车辆的信息
    :param world: CARLA 世界对象
    :param vehicle: 车辆对象
    """
    snapshot = world.get_snapshot()
    # 打印当前帧、经过的时间、车辆的加速度、速度和位置
    print("%d %06.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f %+8.03f" %
          (snapshot.frame, snapshot.timestamp.elapsed_seconds, \
           vehicle.get_acceleration().x, vehicle.get_acceleration().y, vehicle.get_acceleration().z, \
           vehicle.get_velocity().x, vehicle.get_velocity().y, vehicle.get_velocity().z, \
           vehicle.get_location().x, vehicle.get_location().y, vehicle.get_location().z))


def wait(world, frames=100):
    """
    使世界模拟前进指定的帧数
    :param world: CARLA 世界对象
    :param frames: 要模拟的帧数，默认为 100
    """
    for i in range(0, frames):
        world.tick()


def main(arg):
    """Main function of the script"""
    # 创建一个客户端，连接到指定的主机和端口
    client = carla.Client(arg.host, arg.port)
    client.set_timeout(2.0)
    # 获取世界对象
    world = client.get_world()

    try:
        # 获取世界的原始设置
        original_settings = world.get_settings()
        settings = world.get_settings()

        # 设置固定的时间步长和同步模式
        delta = 0.1
        settings.fixed_delta_seconds = delta
        settings.synchronous_mode = True
        world.apply_settings(settings)

        # 获取蓝图库并根据过滤器筛选车辆蓝图
        blueprint_library = world.get_blueprint_library()
        vehicle_bp = blueprint_library.filter(arg.filter)[0]

        # 获取初始的车辆生成点并将其抬高 3 个单位
        vehicle_transform = world.get_map().get_spawn_points()[0]
        vehicle_transform.location.z += 3
        # 在世界中生成车辆
        vehicle = world.spawn_actor(vehicle_bp, vehicle_transform)

        # 获取车辆的物理控制信息并存储车辆质量
        physics_vehicle = vehicle.get_physics_control()
        car_mass = physics_vehicle.mass

        # 设置观察者的位置和朝向，使其在车辆后方 20 个单位并旋转 180 度
        spectator_transform = carla.Transform(vehicle_transform.location, vehicle_transform.rotation)
        spectator_transform.location += vehicle_transform.get_forward_vector() * 20
        spectator_transform.rotation.yaw += 180
        spectator = world.get_spectator()
        spectator.set_transform(spectator_transform)


        # 等待一段时间让车辆稳定，并保存初始变换
        wait(world)
        vehicle.set_target_velocity(carla.Vector3D(0, 0, 0))
        vehicle_transform = vehicle.get_transform()
        wait(world)


        # 在物体质心处施加一个冲量
        impulse = 10 * car_mass
        print("# Adding an Impulse of %f N s" % impulse)
        vehicle.add_impulse(carla.Vector3D(0, 0, impulse))

        wait(world)
        vehicle.set_transform(vehicle_transform)
        vehicle.set_target_velocity(carla.Vector3D(0, 0, 0))
        wait(world)

        print("# Adding a Force of %f N" % (impulse / delta))
        # 这里指出 add_force 方法不适合这种瞬时力，但为了展示 add_impulse 和 add_force 的等效性，进行了适当的缩放
        # 因为力将在整个时间步长 delta 内施加，所以力约等于冲量除以时间步长
        vehicle.add_force(carla.Vector3D(0, 0, impulse / delta))

        wait(world)
        vehicle.set_transform(vehicle_transform)
        vehicle.set_target_velocity(carla.Vector3D(0, 0, 0))
        wait(world)

        wait(world, 500)


    finally:
        # 恢复世界的原始设置
        world.apply_settings(original_settings)
        # 销毁车辆
        vehicle.destroy()


if __name__ == "__main__":

    argparser = argparse.ArgumentParser(
        description=__doc__)
    # 命令行参数：主机 IP 地址，默认为 localhost
    argparser.add_argument(
        '--host',
        metavar='H',
        default='localhost',
        help='IP of the host CARLA Simulator (default: localhost)')
    # 命令行参数：端口号，默认为 2000
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port of CARLA Simulator (default: 2000)')
    # 命令行参数：演员过滤器，默认为 "model3"
    argparser.add_argument(
        '--filter',
        metavar='PATTERN',
        default='model3',
        help='actor filter (default: "vehicle.*")')
    args = argparser.parse_args()

    try:
        main(args)
    except KeyboardInterrupt:
        print(' - Exited by user.')
