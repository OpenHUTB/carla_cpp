#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Lidar投影到RGB相机示例
"""

import glob
import os
import sys

# 将CARLA PythonAPI模块的路径添加到系统路径
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

import argparse
from queue import Queue
from queue import Empty
from matplotlib import cm

# 确保已安装numpy
try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')

# 确保已安装PIL
try:
    from PIL import Image
except ImportError:
    raise RuntimeError('cannot import PIL, make sure "Pillow" package is installed')

# 用于lidar强度可视化的颜色映射
VIRIDIS = np.array(cm.get_cmap('viridis').colors)
VID_RANGE = np.linspace(0.0, 1.0, VIRIDIS.shape[0])

def sensor_callback(data, queue):
    """
   传感器数据的回调函数。它将数据放入线程安全的队列中。
    """
    queue.put(data)


def tutorial(args):
    """
    本函数旨在作为如何以同步方式获取数据，并将3D点从lidar投影到2D相机的教程。
    """
    # 连接到服务器
    client = carla.Client(args.host, args.port)
    client.set_timeout(2.0)
     # 获取当前世界的引用以及蓝图库（用于创建actor）
    world = client.get_world()
    bp_lib = world.get_blueprint_library()

    # 获取交通管理器并配置为同步模式
    traffic_manager = client.get_trafficmanager(8000)
    traffic_manager.set_synchronous_mode(True)

     # 保存原始世界设置并配置新的同步模式设置
    original_settings = world.get_settings()
    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 3.0
    world.apply_settings(settings)

    vehicle = None
    camera = None
    lidar = None

    try:
        if not os.path.isdir('_out'):
            os.mkdir('_out')
        #  搜索所需的蓝图
        vehicle_bp = bp_lib.filter("vehicle.lincoln.mkz_2017")[0]
        camera_bp = bp_lib.filter("sensor.camera.rgb")[0]
        lidar_bp = bp_lib.filter("sensor.lidar.ray_cast")[0]

        # 配置蓝图
        camera_bp.set_attribute("image_size_x", str(args.width))
        camera_bp.set_attribute("image_size_y", str(args.height))

        if args.no_noise:
            lidar_bp.set_attribute('dropoff_general_rate', '0.0')
            lidar_bp.set_attribute('dropoff_intensity_limit', '1.0')
            lidar_bp.set_attribute('dropoff_zero_intensity', '0.0')
        lidar_bp.set_attribute('upper_fov', str(args.upper_fov))
        lidar_bp.set_attribute('lower_fov', str(args.lower_fov))
        lidar_bp.set_attribute('channels', str(args.channels))
        lidar_bp.set_attribute('range', str(args.range))
        lidar_bp.set_attribute('points_per_second', str(args.points_per_second))

        # 生成蓝图
        vehicle = world.spawn_actor(
            blueprint=vehicle_bp,
            transform=world.get_map().get_spawn_points()[0])
        vehicle.set_autopilot(True)
        camera = world.spawn_actor(
            blueprint=camera_bp,
            transform=carla.Transform(carla.Location(x=1.6, z=1.6)),
            attach_to=vehicle)
        lidar = world.spawn_actor(
            blueprint=lidar_bp,
            transform=carla.Transform(carla.Location(x=1.0, z=1.8)),
            attach_to=vehicle)

        # 构建K投影矩阵：
        # K = [[Fx,  0, image_w/2],
        #      [ 0, Fy, image_h/2],
        #      [ 0,  0,         1]]
        image_w = camera_bp.get_attribute("image_size_x").as_int()
        image_h = camera_bp.get_attribute("image_size_y").as_int()
        fov = camera_bp.get_attribute("fov").as_float()
        focal = image_w / (2.0 * np.tan(fov * np.pi / 360.0))

        # 在这种情况下，Fx和Fy是相同的，因为像素宽高比是1

        K = np.identity(3)
        K[0, 0] = K[1, 1] = focal
        K[0, 2] = image_w / 2.0
        K[1, 2] = image_h / 2.0

        # 传感器数据将被保存在线程安全的队列中
        image_queue = Queue()
        lidar_queue = Queue()

        camera.listen(lambda data: sensor_callback(data, image_queue))
        lidar.listen(lambda data: sensor_callback(data, lidar_queue))

        for frame in range(args.frames):#frame从0到arg.frames-1执行循环
            world.tick()
            world_frame = world.get_snapshot().frame

            try:
                # 一旦接收到数据就获取它。
                image_data = image_queue.get(True, 1.0)
                lidar_data = lidar_queue.get(True, 1.0)
            except Empty:
                print("[Warning] Some sensor data has been missed")
                continue

            assert image_data.frame == lidar_data.frame == world_frame
            # 在这一点上，我们有了两个传感器的同步信息。
            sys.stdout.write("\r(%d/%d) Simulation: %d Camera: %d Lidar: %d" %
                (frame, args.frames, world_frame, image_data.frame, lidar_data.frame) + ' ')
            sys.stdout.flush()

            # 获取原始BGRA缓冲区并将其转换为RGB数组，形状为（image_data.height，image_data.width，3）。
            im_array = np.copy(np.frombuffer(image_data.raw_data, dtype=np.dtype("uint8")))
            im_array = np.reshape(im_array, (image_data.height, image_data.width, 4))
            im_array = im_array[:, :, :3][:, :, ::-1]

            # 获取lidar数据并将其转换为numpy数组。
            p_cloud_size = len(lidar_data)
            p_cloud = np.copy(np.frombuffer(lidar_data.raw_data, dtype=np.dtype('f4')))
            p_cloud = np.reshape(p_cloud, (p_cloud_size, 4))

            # Lidar强度数组形状为（p_cloud_size，），但目前，我们关注3D点。
            intensity = np.array(p_cloud[:, 3])

            # 点云在lidar传感器空间数组形状为（3，p_cloud_size）。
            local_lidar_points = np.array(p_cloud[:, :3]).T

            # 为每个3D点添加额外的1.0，使其成为形状（4，p_cloud_size）的数组，以便它可以乘以（4，4）矩阵。
            local_lidar_points = np.r_[
                local_lidar_points, [np.ones(local_lidar_points.shape[1])]]

            # 这个（4，4）矩阵将点从lidar空间转换到世界空间。
            lidar_2_world = lidar.get_transform().get_matrix()

            # 将点从lidar空间转换到世界空间。
            world_points = np.dot(lidar_2_world, local_lidar_points)

            # 这个（4，4）矩阵将点从世界坐标转换到相机坐标。
            world_2_camera = np.array(camera.get_transform().get_inverse_matrix())

            # 将点从世界空间转换到相机空间。
            sensor_points = np.dot(world_2_camera, world_points)

            #现在我们必须从UE4的坐标系更改为“标准”
            # 相机坐标系（与OpenCV使用的相同）：

            # ^ z                       . z
            # |                        /
            # |              转换为： +-------> x
            # | . x                   |
            # |/                      |
            # +-------> y             v y

            # 这可以通过乘以以下矩阵实现：
            # [[ 0,  1,  0 ],
            #  [ 0,  0, -1 ],
            #  [ 1,  0,  0 ]]

            # 或者，在这种情况下，等同于交换：
            # (x, y ,z) -> (y, -z, x)
            point_in_camera_coords = np.array([
                sensor_points[1],
                sensor_points[2] * -1,
                sensor_points[0]])

            #最后，我们可以使用我们的K矩阵进行实际的3D -> 2D转换。
            points_2d = np.dot(K, point_in_camera_coords)

            # 记得通过第三值归一化x，y值。
            points_2d = np.array([
                points_2d[0, :] / points_2d[2, :],
                points_2d[1, :] / points_2d[2, :],
                points_2d[2, :]])

            # 此时，points_2d[0, :] 包含所有点的 x 坐标，points_2d[1, :] 包含所有点的 y 坐标。
            # 为了在屏幕上正确地可视化所有内容，必须丢弃屏幕外的点，以及相机投影平面后的点。
            points_2d = points_2d.T
            intensity = intensity.T
           # 创建一个掩码，用于筛选出在画布内的点
            points_in_canvas_mask = \
                (points_2d[:, 0] > 0.0) & (points_2d[:, 0] < image_w) & \
                (points_2d[:, 1] > 0.0) & (points_2d[:, 1] < image_h) & \
                (points_2d[:, 2] > 0.0)
             # 应用掩码，筛选出画布内的点
            points_2d = points_2d[points_in_canvas_mask]
            intensity = intensity[points_in_canvas_mask]

           # 提取屏幕坐标（uv）作为整数。
            u_coord = points_2d[:, 0].astype(np.int)
            v_coord = points_2d[:, 1].astype(np.int)

            # 由于在创建此脚本时，强度函数返回的值较高，需要调整以便更好地进行可视化。
            intensity = 4 * intensity - 3
            # 根据强度值，使用viridis颜色映射来为每个点着色
            color_map = np.array([
                np.interp(intensity, VID_RANGE, VIRIDIS[:, 0]) * 255.0,
                np.interp(intensity, VID_RANGE, VIRIDIS[:, 1]) * 255.0,
                np.interp(intensity, VID_RANGE, VIRIDIS[:, 2]) * 255.0]).astype(np.int).T

            if args.dot_extent <= 0:
              # 使用numpy将2D点绘制为单个像素。
                im_array[v_coord, u_coord] = color_map
            else:
                # 将2D点绘制为边长为args.dot_extent的正方形。
                for i in range(len(points_2d)):
                    # 我不是NumPy专家，不知道如何在不使用这个循环的情况下设置更大的点，
                    # 所以如果有人有更好的解决方案，请务必更新这个脚本。与此同时，它足够快 :)
                    im_array[
                        v_coord[i]-args.dot_extent : v_coord[i]+args.dot_extent,
                        u_coord[i]-args.dot_extent : u_coord[i]+args.dot_extent] = color_map[i]

            # 使用Pillow模块保存图像。
            image = Image.fromarray(im_array)
            image.save("_out/%08d.png" % image_data.frame)

    finally:
        # 最后，退出时恢复原始设置。
        world.apply_settings(original_settings)

        # 销毁场景中的actor。
        if camera:
            camera.destroy()
        if lidar:
            lidar.destroy()
        if vehicle:
            vehicle.destroy()


def main():
        """
    主函数，用于解析命令行参数并启动相关教程（tutorial）操作。
    它通过argparse模块来定义和解析一系列的命令行参数，然后调用tutorial函数执行具体任务，
    同时对可能出现的用户中断操作（通过键盘中断）进行了异常处理。
    """
    # 创建一个参数解析器对象，用于定义和解析命令行参数
    argparser = argparse.ArgumentParser(
        description='CARLA Sensor sync and projection tutorial')
# 添加'--host'参数，指定主机服务器的IP地址，默认值为'127.0.0.1'，用于连接对应的服务器
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
# 添加'-p'或'--port'参数，指定要监听的TCP端口号，类型为整数，默认值为2000
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
# 添加'--res'参数，指定窗口分辨率，格式为'WIDTHxHEIGHT'，默认值为'1280x720'
    argparser.add_argument(
        '--res',
        metavar='WIDTHxHEIGHT',
        default='680x420',
        help='window resolution (default: 1280x720)')
# 添加'-f'或'--frames'参数，指定要记录的帧数，类型为整数，默认值为500
    argparser.add_argument(
        '-f', '--frames',
        metavar='N',
        default=500,
        type=int,
        help='number of frames to record (default: 500)')
# 添加'-d'或'--dot-extent'参数，指定可视化点的范围（以像素为单位），类型为整数，默认值为2，推荐范围是[1-4]
    argparser.add_argument(
        '-d', '--dot-extent',
        metavar='SIZE',
        default=2,
        type=int,
        help='visualization dot extent in pixels (Recomended [1-4]) (default: 2)')
# 添加'--no-noise'参数，这是一个布尔型参数（action='store_true'表示如果命令行中指定了该参数，则其值为True），用于移除普通（非语义）激光雷达的衰减和噪声
    argparser.add_argument(
        '--no-noise',
        action='store_true',
        help='remove the drop off and noise from the normal (non-semantic) lidar')
# 添加'--upper-fov'参数，指定激光雷达的上视野角度（以度为单位），类型为浮点数，默认值为30.0
    argparser.add_argument(
        '--upper-fov',
        metavar='F',
        default=30.0,
        type=float,
        help='lidar\'s upper field of view in degrees (default: 15.0)')
# 添加'--lower-fov'参数，指定激光雷达的下视野角度（以度为单位），类型为浮点数，默认值为-25.0
    argparser.add_argument(
        '--lower-fov',
        metavar='F',
        default=-25.0,
        type=float,
        help='lidar\'s lower field of view in degrees (default: -25.0)')
# 添加'-c'或'--channels'参数，指定激光雷达的通道数，类型为浮点数，默认值为64.0
    argparser.add_argument(
        '-c', '--channels',
        metavar='C',
        default=64.0,
        type=float,
        help='lidar\'s channel count (default: 64)')
# 添加'-r'或'--range'参数，指定激光雷达的最大探测范围（以米为单位），类型为浮点数，默认值为100.0
    argparser.add_argument(
        '-r', '--range',
        metavar='R',
        default=100.0,
        type=float,
        help='lidar\'s maximum range in meters (default: 100.0)')
# 添加'--points-per-second'参数，指定激光雷达每秒的点数，类型为整数，默认值为100000
    argparser.add_argument(
        '--points-per-second',
        metavar='N',
        default='100000',
        type=int,
        help='lidar points per second (default: 100000)')
# 解析命令行参数，得到包含所有参数值的命名空间对象args
    args = argparser.parse_args()
# 将分辨率参数（字符串形式如'WIDTHxHEIGHT'）拆分成宽度和高度两个整数，并分别赋值给args的width和height属性
    args.width, args.height = [int(x) for x in args.res.split('x')]
# 将可视化点的范围参数减1（具体作用可能和后续可视化相关逻辑有关）
    args.dot_extent -= 1

    try:
        tutorial(args)
 # 调用tutorial函数，并传入解析好的参数args，执行具体的教程相关操作（此处假设tutorial函数在别处定义且实现了相关功能）
    except KeyboardInterrupt:
        # 如果用户通过键盘中断（比如按下Ctrl+C）操作中断程序执行，捕获该异常并打印相应提示信息
        print('\nCancelled by user. Bye!')


if __name__ == '__main__':

    main()
