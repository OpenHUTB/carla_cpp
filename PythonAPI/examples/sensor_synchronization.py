#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Sensor synchronization example for CARLA

The communication model for the syncronous mode in CARLA sends the snapshot
of the world and the sensors streams in parallel.
We provide this script as an example of how to syncrononize the sensor
data gathering in the client.
To to this, we create a queue that is being filled by every sensor when the
client receives its data and the main loop is blocked until all the sensors
have received its data.
This suppose that all the sensors gather information at every tick. It this is
not the case, the clients needs to take in account at each frame how many
sensors are going to tick at each frame.
"""

import glob
import os
import sys
from queue import Queue
from queue import Empty
# 尝试将Carla库的路径添加到系统路径中，根据Python版本和操作系统类型来确定具体的库文件（.egg文件）
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

# Sensor callback.
# 这是传感器的回调函数，用于接收传感器数据并进行处理。
# 重要的是，最后要将包含相关信息的元素放入传感器队列中。
# 参数说明：
# - sensor_data：传感器采集到的数据。
# - sensor_queue：传感器数据要放入的队列，用于后续处理和同步。
# - sensor_name：传感器的名称，用于标识不同的传感器。
def sensor_callback(sensor_data, sensor_queue, sensor_name):
    # 在这里可以对sensor_data进行各种操作，比如将其保存到磁盘等。
    # 然后只需将相关信息添加到队列中即可。
    sensor_queue.put((sensor_data.frame, sensor_name))


def main():
    # 创建Carla客户端，连接到本地主机的2000端口
    client = carla.Client('localhost', 2000)
    # 设置客户端操作的超时时间为2秒，如果在这个时间内没有响应则抛出异常
    client.set_timeout(2.0)
    # 获取Carla世界对象，后续所有的操作都基于这个世界环境
    world = client.get_world()

    try:
        # 保存原始的世界设置，以便在脚本结束时能够恢复，使服务器恢复到初始状态。
        original_settings = world.get_settings()
        settings = world.get_settings()

        # 设置Carla为同步模式，固定时间步长为0.2秒。
        # 在同步模式下，模拟的推进需要手动调用world.tick()来进行。
        settings.fixed_delta_seconds = 0.2
        settings.synchronous_mode = True
        world.apply_settings(settings)

        # 创建一个队列，用于存储传感器接收到的数据。
        # 这个队列是线程安全的，多个传感器的回调函数可以并发地向其添加数据而不会出现问题。
        sensor_queue = Queue()

        # 获取世界中的蓝图库，蓝图用于创建各种对象，比如传感器等。
        blueprint_library = world.get_blueprint_library()
        # 从蓝图库中找到RGB相机传感器的蓝图，用于后续创建相机传感器实例。
        cam_bp = blueprint_library.find('sensor.camera.rgb')
        # 找到激光雷达传感器（射线投射类型）的蓝图。
        lidar_bp = blueprint_library.find('sensor.lidar.ray_cast')
        # 找到雷达传感器的蓝图。
        radar_bp = blueprint_library.find('sensor.other.radar')

        # 创建一个列表，用于存放所有创建的传感器实例，方便后续管理和销毁等操作。
        sensor_list = []
        # 在世界中生成一个相机传感器实例，位置使用默认的变换（原点位置等默认设置）。
        cam01 = world.spawn_actor(cam_bp, carla.Transform())
        # 为相机传感器设置监听函数，当有数据时会调用sensor_callback进行处理，传入对应的数据、队列和传感器名称。
        cam01.listen(lambda data: sensor_callback(data, sensor_queue, "camera01"))
        # 将创建的相机传感器添加到传感器列表中。
        sensor_list.append(cam01)
        # 设置激光雷达传感器的属性，这里设置每秒的点数为100000。
        lidar_bp.set_attribute('points_per_second', '100000')
        # 在世界中生成一个激光雷达传感器实例，位置同样使用默认变换。
        lidar01 = world.spawn_actor(lidar_bp, carla.Transform())
        # 为该激光雷达传感器设置监听函数，传入相应参数用于处理数据。
        lidar01.listen(lambda data: sensor_callback(data, sensor_queue, "lidar01"))
        # 将其添加到传感器列表。
        sensor_list.append(lidar01)
        # 再次设置激光雷达传感器属性，这里设置每秒点数为1000000，用于创建另一个不同配置的激光雷达传感器。
        lidar_bp.set_attribute('points_per_second', '1000000')
        lidar02 = world.spawn_actor(lidar_bp, carla.Transform())
        lidar02.listen(lambda data: sensor_callback(data, sensor_queue, "lidar02"))
        sensor_list.append(lidar02)
        # 在世界中生成一个雷达传感器实例，位置为默认变换。
        radar01 = world.spawn_actor(radar_bp, carla.Transform())
        # 为雷达传感器设置监听函数，传入对应参数用于处理数据。
        radar01.listen(lambda data: sensor_callback(data, sensor_queue, "radar01"))
        # 将其添加到传感器列表。
        sensor_list.append(radar01)
        radar02 = world.spawn_actor(radar_bp, carla.Transform())
        radar02.listen(lambda data: sensor_callback(data, sensor_queue, "radar02"))
        sensor_list.append(radar02)

        # 主循环，用于不断推进模拟并处理传感器数据。
        while True:
            # 推进Carla世界的模拟，按照设置的固定时间步长前进一帧。
            world.tick()
            # 获取当前世界的帧编号，并打印出来，用于展示模拟的进度等信息。
            w_frame = world.get_snapshot().frame
            print("\nWorld's frame: %d" % w_frame)

            # 现在等待传感器数据被接收。
            # 由于队列是阻塞式的，会在queue.get()方法处等待，直到所有信息被处理完，然后继续下一帧。
            # 这里设置了1秒的超时时间，如果在这个时间内没有接收到某些信息，就会继续执行后续代码。
            try:
                for _ in range(len(sensor_list)):
                    s_frame = sensor_queue.get(True, 1.0)
                    print("    Frame: %d   Sensor: %s" % (s_frame[0], s_frame[1]))

            except Empty:
                print("    Some of the sensor information is missed")

    finally:
        # 在脚本结束时，无论是否出现异常，都将世界的设置恢复到原始状态。
        world.apply_settings(original_settings)
        # 遍历传感器列表，销毁所有创建的传感器实例，释放资源。
        for sensor in sensor_list:
            sensor.destroy()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print(' - Exited by user.')
