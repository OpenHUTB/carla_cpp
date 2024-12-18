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
import random
import time

def main():
    # 创建一个列表，用于存储在模拟过程中创建的所有演员（actors，在Carla中可以是车辆、传感器等各种实体），方便后续统一销毁它们。
    actor_list = []

    # 以下是这段脚本功能的描述，即在这个教程脚本中，我们要向模拟环境中添加一辆车，让它自动驾驶，同时创建一个附着在车辆上的相机，
    # 并将相机生成的所有图像保存到磁盘，此外，还会保存每一帧的GBuffer纹理信息。

    try:
        # 首先，我们需要创建一个客户端（client）对象，它将向模拟器发送各种请求。这里假设模拟器在本地主机（'127.0.0.1'）的2000端口接受请求。
        client = carla.Client('127.0.0.1', 2000)
        # 设置客户端的超时时间为2.0秒，意味着如果客户端向服务器发送请求后，在2.0秒内没有收到服务器的响应，就会判定此次操作超时，避免程序长时间无响应地等待。
        client.set_timeout(2.0)

        # 一旦创建了客户端，我们就可以获取当前正在运行的模拟世界（world）对象，这个世界对象包含了模拟场景中的各种实体、设置等信息，后续操作大多基于这个世界对象展开。
        world = client.get_world()

        # 世界对象包含了蓝图库（blueprint_library），其中存放着各种可用于向模拟环境中添加新演员的蓝图，蓝图可以理解为创建具体实体实例的模板，定义了实体的各种属性和配置信息。
        blueprint_library = world.get_blueprint_library()

        # 现在从蓝图库中筛选出所有类型为'vehicle'（车辆）的蓝图，并随机选择其中一个，用于后续创建车辆实例。
        bp = random.choice(blueprint_library.filter('vehicle'))

        # 一个蓝图包含了定义车辆实例的一系列属性列表，我们可以读取这些属性并修改其中一些。例如，这里我们随机设置车辆的颜色。
        if bp.has_attribute('color'):
            # 获取颜色属性的推荐值列表（可能是预定义的一些可选颜色），并从中随机选择一个颜色值。
            color = random.choice(bp.get_attribute('color').recommended_values)
            # 使用选择的颜色值来设置车辆蓝图的颜色属性，这样创建出的车辆实例就会具有该随机颜色。
            bp.set_attribute('color', color)

        # 接下来要给车辆设置一个初始的坐标变换（transform），我们从地图推荐的出生点列表中随机选择一个作为车辆的初始位置和姿态信息。
        # 这里暂时只取了列表中的第一个出生点，你也可以根据实际需求随机选择其他出生点等方式来设置初始位置。
        transform = world.get_map().get_spawn_points()[0]

        # 让世界对象根据选择的车辆蓝图（bp）和设置的坐标变换（transform）来创建车辆实例，返回创建好的车辆对象。
        vehicle = world.spawn_actor(bp, transform)

        # 需要注意的是，我们创建的演员如果不调用它们的“destroy”函数进行销毁，它们将一直留在模拟环境中，即使我们退出了Python脚本也是如此。
        # 因此，我们将创建的所有演员都存储到actor_list列表中，以便后续可以统一销毁它们。
        actor_list.append(vehicle)
        print('created %s' % vehicle.type_id)

        # 让创建的车辆开启自动驾驶模式，这样车辆就会在模拟环境中按照预设的自动驾驶逻辑自动行驶。
        vehicle.set_autopilot(True)

        # 现在添加一个“rgb”相机（用于获取彩色图像的传感器），并将其附着在车辆上。注意这里给定的坐标变换（transform）是相对于车辆的，也就是以车辆为参考系来设置相机的位置和姿态。
        camera_bp = blueprint_library.find('sensor.camera.rgb')
        # 设置相机图像的水平分辨率为1920像素，通过修改蓝图的属性来配置相机的相关参数。
        camera_bp.set_attribute('image_size_x', '1920')
        # 设置相机图像的垂直分辨率为1080像素，同样是对相机蓝图属性的修改。
        camera_bp.set_attribute('image_size_y', '1080')
        # 创建一个相对于车辆的坐标变换，将相机放置在车辆的某个位置（x坐标为1.5，z坐标为2.4，这里y坐标默认为0，可能是在车辆上方且靠前一点的位置），用于确定相机相对于车辆的具体位置和朝向等姿态信息。
        camera_transform = carla.Transform(carla.Location(x=1.5, z=2.4))
        # 使用世界对象在指定位置（根据camera_transform）并附着到指定车辆（vehicle）上创建相机实例，返回创建好的相机对象。
        camera = world.spawn_actor(camera_bp, camera_transform, attach_to=vehicle)
        actor_list.append(camera)
        print('created %s' % camera.type_id)

        # 为相机注册一个回调函数，当有新的图像帧可用时，这个回调函数就会被触发。目前，这一步对于正确接收GBuffer纹理是必需的，
        # 因为它用于确定传感器是否处于活动状态（可能内部机制通过这种回调来判断何时可以获取相关纹理数据等情况）。
        # 这里的回调函数将每一帧图像保存到磁盘，文件名格式为'_out/FinalColor-%06d.png'，其中%06d会根据图像的帧编号进行格式化，保证文件名的唯一性和顺序性，便于后续查看和处理图像。
        camera.listen(lambda image: image.save_to_disk('_out/FinalColor-%06d.png' % image.frame))

        # 在这里为每个GBuffer纹理注册相应的回调函数。函数“listen_to_gbuffer”的行为类似于常规的“listen”函数，
        # 但需要先传入想要获取的GBuffer纹理的ID，然后指定对应的回调函数，用于将相应的GBuffer纹理图像保存到磁盘，每个纹理都有其对应的文件名格式，同样包含帧编号用于区分不同帧的纹理图像。

        camera.listen_to_gbuffer(carla.GBufferTextureID.SceneColor, lambda image: image.save_to_disk('_out/GBuffer-SceneColor-%06d.png' % image.frame))
        camera.listen_to_gbuffer(carla.GBufferTextureID.SceneDepth, lambda image: image.save_to_disk('_out/GBuffer-SceneDepth-%06d.png' % image.frame))
        camera.listen_to_gbuffer(carla.GBufferTextureID.SceneStencil, lambda image: image.save_to_disk('_out/GBuffer-SceneStencil-%06d.png' % image.frame))
        camera.listen_to_gbuffer(carla.GBufferTextureID.GBufferA, lambda image: image.save_to_disk('_out/GBuffer-A-%06d.png' % image.frame))
        camera.listen_to_gbuffer(carla.GBufferTextureID.GBufferB, lambda image: image.save_to_disk('_out/GBuffer-B-%06d.png' % image.frame))
        camera.listen_to_gbuffer(carla.GBufferTextureID.GBufferC, lambda image: image.save_to_disk('_out/GBuffer-C-%06d.png' % image.frame))
        camera.listen_to_gbuffer(carla.GBufferTextureID.GBufferD, lambda image: image.save_to_disk('_out/GBuffer-D-%06d.png' % image.frame))
        # 注意，某些GBuffer纹理可能在特定场景中不可用。例如，在这个示例中，纹理E和F可能不可用，这将导致它们被保存为黑色图像（因为没有实际有效的数据）。
        camera.listen_to_gbuffer(carla.GBufferTextureID.GBufferE, lambda image: image.save_to_disk('_out/GBuffer-E-%06d.png' % image.frame))
        camera.listen_to_gbuffer(carla.GBufferTextureID.GBufferF, lambda image: image.save_to_disk('_out/GBuffer-F-%06d.png' % image.frame))
        camera.listen_to_gbuffer(carla.GBufferTextureID.Velocity, lambda image: image.save_to_disk('_out/GBuffer-Velocity-%06d.png' % image.frame))
        camera.listen_to_gbuffer(carla.GBufferTextureID.SSAO, lambda image: image.save_to_disk('_out/GBuffer-SSAO-%06d.png' % image.frame))
        camera.listen_to_gbuffer(carla.GBufferTextureID.CustomDepth, lambda image: image.save_to_disk('_out/GBuffer-CustomDepth-%06d.png' % image.frame))
        camera.listen_to_gbuffer(carla.GBufferTextureID.CustomStencil, lambda image: image.save_to_disk('_out/GBuffer-CustomStencil-%06d.png' % image.frame))

        # 让程序休眠10秒，在这10秒内，车辆会自动驾驶，相机会不断捕获图像并触发回调函数保存图像及相关GBuffer纹理数据到磁盘，模拟场景会持续运行一段时间。
        time.sleep(10)

    finally:
        print('destroying actors')
        # 首先销毁相机实例，释放相关资源。
        camera.destroy()
        # 使用客户端对象批量销毁存储在actor_list列表中的所有演员（包括车辆等），通过创建一个包含销毁每个演员命令的列表来实现批量操作，确保模拟环境清理干净，避免资源泄漏等问题。
        client.apply_batch([carla.command.DestroyActor(x) for x in actor_list])
        print('done.')


if __name__ == '__main__':
    main()
