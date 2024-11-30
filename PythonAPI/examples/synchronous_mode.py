#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
#在CARLA仿真环境中控制一辆车辆，并使用pygame库显示车辆的RGB和语义分割图像。


import glob
import os
import sys

# 尝试将CARLA的.egg文件路径添加到sys.path，如果失败则忽略。
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

import random

# 尝试导入pygame模块，用于创建图形用户界面,如果导入失败，抛出运行时错误。
try:
    import pygame
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')

# 尝试导入queue模块,如果导入失败（在Python 2中），从Queue模块导入。
try:
    import queue
except ImportError:
    import Queue as queue


class CarlaSyncMode(object):# 定义一个名为CarlaSyncMode的上下文管理器类。
    """
    Context manager to synchronize output from different sensors. Synchronous
    mode is enabled as long as we are inside this context

        with CarlaSyncMode(world, sensors) as sync_mode:
            while True:
                data = sync_mode.tick(timeout=1.0)

    """
    
 # 类构造函数，初始化world对象和传感器列表。
    def __init__(self, world, *sensors, **kwargs):
        self.world = world
        self.sensors = sensors
        self.frame = None
        self.delta_seconds = 1.0 / kwargs.get('fps', 20)# 默认FPS为20。
        self._queues = []                             # 用于存储传感器数据的队列。
        self._settings = None                         # 用于存储world的当前设置。
        
# 进入上下文管理器时执行的操作。
    def __enter__(self):
        self._settings = self.world.get_settings()  # 获取当前world设置。
        self.frame = self.world.apply_settings(carla.WorldSettings(# 应用新的world设置。
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=self.delta_seconds))

           # 创建队列并注册事件。
        def make_queue(register_event):
            q = queue.Queue()
            register_event(q.put)
            self._queues.append(q)

        make_queue(self.world.on_tick)        # 为world的tick事件创建队列。
        for sensor in self.sensors:           # 为每个传感器创建队列。
            make_queue(sensor.listen)       
        return self

    def tick(self, timeout):                  # 从传感器队列中获取数据。
        self.frame = self.world.tick()                                
        data = [self._retrieve_data(q, timeout) for q in self._queues]
        assert all(x.frame == self.frame for x in data)         # 确保所有数据帧相同。
        return data

    def __exit__(self, *args, **kwargs):                    # 退出上下文管理器时执行的操作。
        self.world.apply_settings(self._settings)            # 恢复之前的world设置。

    def _retrieve_data(self, sensor_queue, timeout):        # 从传感器队列中检索数据。
        while True:
            data = sensor_queue.get(timeout=timeout)
            if data.frame == self.frame:
                return data


def draw_image(surface, image, blend=False):                        # 函数用于在pygame表面绘制图像。
    array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))  # 从图像的原始数据创建numpy数组。
    array = np.reshape(array, (image.height, image.width, 4))         # 调整数组形状。
    array = array[:, :, :3]                                         # 去除alpha通道。
    array = array[:, :, ::-1]                                       # 反转颜色通道。
    image_surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))# 创建pygame表面。
    if blend:                                                       # 如果需要混合，则设置alpha值。
        image_surface.set_alpha(100)
    surface.blit(image_surface, (0, 0))                             # 将图像绘制到pygame表面。


def get_font():                                                     # 函数用于获取字体。
    fonts = [x for x in pygame.font.get_fonts()]
    default_font = 'ubuntumono'
    font = default_font if default_font in fonts else fonts[0]
    font = pygame.font.match_font(font)
    return pygame.font.Font(font, 14)


def should_quit():
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            return True
        elif event.type == pygame.KEYUP:
            if event.key == pygame.K_ESCAPE:
                return True
    return False


def main():
    actor_list = []
    pygame.init()

    display = pygame.display.set_mode(
        (800, 600),
        pygame.HWSURFACE | pygame.DOUBLEBUF)
    font = get_font()
    clock = pygame.time.Clock()

    client = carla.Client('localhost', 2000)
    client.set_timeout(2.0)

    world = client.get_world()

    try:
        m = world.get_map()
        start_pose = random.choice(m.get_spawn_points())
        waypoint = m.get_waypoint(start_pose.location)

        blueprint_library = world.get_blueprint_library()

        vehicle = world.spawn_actor(
            random.choice(blueprint_library.filter('vehicle.*')),
            start_pose)
        actor_list.append(vehicle)
        vehicle.set_simulate_physics(False)

        camera_rgb = world.spawn_actor(
            blueprint_library.find('sensor.camera.rgb'),
            carla.Transform(carla.Location(x=-5.5, z=2.8), carla.Rotation(pitch=-15)),
            attach_to=vehicle)
        actor_list.append(camera_rgb)

        camera_semseg = world.spawn_actor(
            blueprint_library.find('sensor.camera.semantic_segmentation'),
            carla.Transform(carla.Location(x=-5.5, z=2.8), carla.Rotation(pitch=-15)),
            attach_to=vehicle)
        actor_list.append(camera_semseg)

        # Create a synchronous mode context.
        with CarlaSyncMode(world, camera_rgb, camera_semseg, fps=30) as sync_mode:
            while True:
                if should_quit():
                    return
                clock.tick()

                # Advance the simulation and wait for the data.
                snapshot, image_rgb, image_semseg = sync_mode.tick(timeout=2.0)

                # Choose the next waypoint and update the car location.
                waypoint = random.choice(waypoint.next(1.5))
                vehicle.set_transform(waypoint.transform)

                image_semseg.convert(carla.ColorConverter.CityScapesPalette)
                fps = round(1.0 / snapshot.timestamp.delta_seconds)

                # Draw the display.
                draw_image(display, image_rgb)
                draw_image(display, image_semseg, blend=True)
                display.blit(
                    font.render('% 5d FPS (real)' % clock.get_fps(), True, (255, 255, 255)),
                    (8, 10))
                display.blit(
                    font.render('% 5d FPS (simulated)' % fps, True, (255, 255, 255)),
                    (8, 28))
                pygame.display.flip()

    finally:

        print('destroying actors.')
        for actor in actor_list:
            actor.destroy()

        pygame.quit()
        print('done.')


if __name__ == '__main__':

    try:

        main()

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
