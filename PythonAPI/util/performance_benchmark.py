#!/usr/bin/env python3

# 版权所有（c）2019 英特尔实验室。
# authors: German Ros (german.ros@intel.com)
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This is a benchmarking script for CARLA. It serves to analyze the performance of CARLA in different scenarios and
conditions, for both sensors and traffic.

Please, make sure you install the following dependencies:

    * python -m pip install -U py-cpuinfo
    * python -m pip install psutil
    * python -m pip install python-tr
    * python -m pip install gpuinfo

"""

# @todo Include this file in the Pylint checks.
# pylint: skip-file
#导入了sys模块，然后检查 Python 的主版本号
import sys
#如果小于 3，说明脚本是在 Python 2 环境下运行，脚本会输出一条提示信息
#告知用户该脚本只能在 Python 3 下运行，然后通过sys.exit(1)退出脚本
if sys.version_info[0] < 3:
    print('This script is only available for Python 3')
    sys.exit(1)

from tr import tr
import argparse
import cpuinfo
import glob
import math
import numpy as np
import os
import psutil
import pygame
import shutil
import GPUtil
import threading
import time
import logging
#尝试将carla模块的路径添加到sys.path中
try:
    #使用glob.glob来查找符合特定模式的carla模块的.egg文件路径，并将其添加到sys.path中
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
#如果在查找过程中出现IndexError
except IndexError:
    #捕获异常并pass
    pass
#添加carla模块的路径以便能够导入carla模块
import carla

# ======================================================================================================================
# -- Global variables. So sorry... -------------------------------------------------------------------------------------
# ======================================================================================================================
sensors_callback = []
#定义一个函数，目的是定义一个天气列表
def define_weather():
  #初始化了一个空列表list_weather
  list_weather = []
  #如果args.tm为True
  if args.tm:
    #只将ClearNoon天气添加到list_weather中
    weather00 = { 'parameter' : carla.WeatherParameters.ClearNoon, 'name': 'ClearNoon'}
     
    list_weather.append(weather00)
  #如果args.tm为False
  else:
    #ClearNoon、CloudyNoon和SoftRainSunset三种天气添加到list_weather中
    weather00 = { 'parameter' : carla.WeatherParameters.ClearNoon, 'name' : 'ClearNoon'}
    weather01 = { 'parameter' : carla.WeatherParameters.CloudyNoon, 'name' : 'CloudyNoon'}
    weather02 = { 'parameter' : carla.WeatherParameters.SoftRainSunset, 'name' : 'SoftRainSunset'}

    list_weather.append(weather00)
    list_weather.append(weather01)
    list_weather.append(weather02)
    #如果args.weather不为None
    if args.weather is not None:
      #尝试根据args.weather中的索引来筛选list_weather中的天气
      try:
        new_list = [list_weather[int(i)] for i in args.weather]
        list_weather = new_list
      #如果在索引过程中出现IndexError
      except IndexError as error:
        #打印一条警告信息，并使用原始的list_weather
        print("Warning!! The list of types of weather introduced is not valid. Using all available.")
  #函数返回list_weather列表
  return list_weather


def define_sensors():
  list_sensor_specs = []

  if args.tm:
    sensors00 = [{'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                'width': 300, 'height': 200, 'fov': 100, 'label': '0. cam-300x200'}]

    list_sensor_specs.append(sensors00)

  else:
    sensors00 = [{'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                  'width': 1920, 'height': 1080, 'fov': 100, 'label': '0. cam-1920x1080'}]

    sensors01 = [{'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                  'width': 1920, 'height': 1080, 'fov': 100, 'label': '1. cam-1920x1080'},
                  {'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 180.0,
                  'width': 1920, 'height': 1080, 'fov': 100, 'label': 'cam-1920x1080'}]

    sensors02 = [{'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                  'width': 1920, 'height': 1080, 'fov': 100, 'label': '2. cam-1920x1080'},
                  {'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 90.0,
                  'width': 1920, 'height': 1080, 'fov': 100, 'label': 'cam-1920x1080'},
                  {'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 180.0,
                  'width': 1920, 'height': 1080, 'fov': 100, 'label': 'cam-1920x1080'}]

    sensors03 = [{'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                  'width': 1920, 'height': 1080, 'fov': 100, 'label': '3. cam-1920x1080'},
                  {'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 90.0,
                  'width': 1920, 'height': 1080, 'fov': 100, 'label': 'cam-1920x1080'},
                  {'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 180.0,
                  'width': 1920, 'height': 1080, 'fov': 100, 'label': 'cam-1920x1080'},
                  {'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 270.0,
                  'width': 1920, 'height': 1080, 'fov': 100, 'label': 'cam-1920x1080'}]

    sensors04 = [{'type': 'sensor.lidar.ray_cast', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'yaw': 0.0, 'pitch': 0.0, 'roll': 0.0,
                  'pts_per_sec': '100000', 'label': '4. LIDAR: 100k'}]

    sensors05 = [{'type': 'sensor.lidar.ray_cast', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'yaw': 0.0, 'pitch': 0.0, 'roll': 0.0,
                  'pts_per_sec': '500000', 'label': '5. LIDAR: 500k'}]

    sensors06 = [{'type': 'sensor.lidar.ray_cast', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'yaw': 0.0, 'pitch': 0.0, 'roll': 0.0,
                  'pts_per_sec': '1000000', 'label': '6. LIDAR: 1M'}]

    list_sensor_specs.append(sensors00)
    list_sensor_specs.append(sensors01)
    list_sensor_specs.append(sensors02)
    list_sensor_specs.append(sensors03)
    list_sensor_specs.append(sensors04)
    list_sensor_specs.append(sensors05)
    list_sensor_specs.append(sensors06)

    if args.sensors is not None:
      try:
        new_list = [list_sensor_specs[int(i)] for i in args.sensors]
        list_sensor_specs = new_list
      except IndexError as error:
        print("Warning!! The list of sensors introduced is not valid. Using all available.")

  return list_sensor_specs
#定义一个函数，目的是创建一个空列表list_env_specs，用于存储环境规格
def define_environments():
  list_env_specs = []
  #如果args.tm为True
  if args.tm:
    #定义四种不同的环境规格
    env00 = {'vehicles': 10, 'walkers': 0}
    env01 = {'vehicles': 50, 'walkers': 50}
    env02 = {'vehicles': 250, 'walkers': 0}
    env03 = {'vehicles': 150, 'walkers': 50}
    #将这四种环境规格依次添加到list_env_specs列表中
    list_env_specs.append(env00)
    list_env_specs.append(env01)
    list_env_specs.append(env02)
    list_env_specs.append(env03)
  #如果args.tm为False
  else:
    #只定义一种环境规格env00，有 1 辆车，0 个行人，并将其添加到list_env_specs列表中
    env00 = {'vehicles': 1, 'walkers': 0}

    list_env_specs.append(env00)
  #函数返回list_env_specs列表
  return list_env_specs
#定义一个函数，主要功能是处理地图列表
def define_maps(client):
  #获取可用地图的完整路径列表，然后使用列表推导式将每个路径中的/Game/Carla/Maps/替换为空字符串
  maps = [m.replace('/Game/Carla/Maps/', '') for m in client.get_available_maps()]
  #对处理后的地图名称列表进行排序
  maps = sorted(maps)
  #如果args.maps不为None
  if args.maps is not None:
    #检查args.maps中的所有元素是否都在maps列表中
    all_good = all(elem in maps for elem in args.maps)
    #如果是，则将maps列表设置为args.maps并进行排序
    if all_good:
      maps = sorted(args.maps)
    #如果不是，则打印警告信息，表示传入的地图列表无效，将继续使用所有可用地图
    else:
      print("Warning!! The list of maps introduced is not valid. Using all available.")
  #函数返回处理后的地图列表
  return maps

#定义一个类，继承自object
class CallBack(object):
    #类的构造函数，在创建类的实例时会被调用
    def __init__(self):
        #线程锁，通过threading.Lock()创建，用于在多线程环境下确保数据的一致性
        self._lock = threading.Lock()
        #pygame的时钟对象，通过pygame.time.Clock()创建，用于跟踪时间
        self._pygame_clock = pygame.time.Clock()
        #整数变量，初始化为 0，用于存储当前的帧率（Frames Per Second
        self._current_fps = 0
    #使类的实例可以像函数一样被调用
    def __call__(self, data):
        #更新时钟，并通过self._pygame_clock.get_fps()获取当前的帧率
        self._pygame_clock.tick()
        #将其存储在self._current_fps中
        self._current_fps = self._pygame_clock.get_fps()
    #用于获取当前的帧率
    def get_fps(self):
        #确保在获取帧率时线程安全
        with self._lock:
            #返回self._current_fps的值，即当前的帧率
            return self._current_fps


def create_environment(world, sensors, n_vehicles, n_walkers, spawn_points, client, tick):
  global sensors_callback
  sensors_ret = []
  blueprint_library = world.get_blueprint_library()

  # setup sensors
  for sensor_spec in sensors:
    bp = blueprint_library.find(sensor_spec['type'])

    if sensor_spec['type'].startswith('sensor.camera'):
      bp.set_attribute('image_size_x', str(sensor_spec['width']))
      bp.set_attribute('image_size_y', str(sensor_spec['height']))
      bp.set_attribute('fov', str(sensor_spec['fov']))
      sensor_location = carla.Location(
        x=sensor_spec['x'],
        y=sensor_spec['y'],
        z=sensor_spec['z'])
      sensor_rotation = carla.Rotation(
        pitch=sensor_spec['pitch'],
        roll=sensor_spec['roll'],
        yaw=sensor_spec['yaw'])

    elif sensor_spec['type'].startswith('sensor.lidar'):
      bp.set_attribute('range', '200')
      bp.set_attribute('rotation_frequency', '10')
      bp.set_attribute('channels', '32')
      bp.set_attribute('upper_fov', '15')
      bp.set_attribute('lower_fov', '-30')
      bp.set_attribute('points_per_second', str(sensor_spec['pts_per_sec']))
      sensor_location = carla.Location(
        x=sensor_spec['x'],
        y=sensor_spec['y'],
        z=sensor_spec['z'])
      sensor_rotation = carla.Rotation(
        pitch=sensor_spec['pitch'],
        roll=sensor_spec['roll'],
        yaw=sensor_spec['yaw'])

    elif sensor_spec['type'].startswith('sensor.other.gnss'):
      sensor_location = carla.Location(x=sensor_spec['x'], y=sensor_spec['y'], z=sensor_spec['z'])
      sensor_rotation = carla.Rotation()

    # create sensor
    sensor_transform = carla.Transform(sensor_location, sensor_rotation)
    sensor = world.spawn_actor(bp, sensor_transform)

    # add callbacks
    sc = CallBack()
    sensor.listen(sc)

    sensors_callback.append(sc)
    sensors_ret.append(sensor)

  vehicles_list = []
  walkers_list = []
  all_id = []

  blueprint = world.get_blueprint_library().filter('vehicle.audi.a2')[0]
  walker_bp = world.get_blueprint_library().filter("walker.pedestrian.0001")[0]

  # @todo cannot import these directly.
  SpawnActor = carla.command.SpawnActor
  SetAutopilot = carla.command.SetAutopilot
  FutureActor = carla.command.FutureActor

  # --------------
  # Spawn vehicles
  # --------------
  batch = []
  for num, transform in enumerate(spawn_points):
    if num >= n_vehicles:
      break
    blueprint.set_attribute('role_name', 'autopilot')
    batch.append(SpawnActor(blueprint, transform).then(SetAutopilot(FutureActor, True)))

  for response in client.apply_batch_sync(batch, False):
    if response.error:
      logging.error(response.error)
    else:
      vehicles_list.append(response.actor_id)

  # -------------
  # Spawn Walkers
  # -------------
  # some settings
  percentagePedestriansRunning = 0.0      # how many pedestrians will run
  percentagePedestriansCrossing = 0.0     # how many pedestrians will walk through the road
  if n_walkers > 0:
    # 1. take all the random locations to spawn
    spawn_points = []
    for i in range(n_walkers):
      spawn_point = carla.Transform()
      loc = world.get_random_location_from_navigation()
      if (loc != None):
        spawn_point.location = loc
        spawn_points.append(spawn_point)
    # 2. we spawn the walker object
    batch = []
    walker_speed = []
    for spawn_point in spawn_points:
      # set as not invincible
      if walker_bp.has_attribute('is_invincible'):
        walker_bp.set_attribute('is_invincible', 'false')
      # set the max speed
      if walker_bp.has_attribute('speed'):
        # walking
        walker_speed.append(walker_bp.get_attribute('speed').recommended_values[1])
      else:
        print("Walker has no speed")
        walker_speed.append(0.0)
      batch.append(SpawnActor(walker_bp, spawn_point))
    results = client.apply_batch_sync(batch, True)
    walker_speed2 = []
    for i in range(len(results)):
      if results[i].error:
        logging.error(results[i].error)
      else:
        walkers_list.append({"id": results[i].actor_id})
        walker_speed2.append(walker_speed[i])
    walker_speed = walker_speed2
    # 3. we spawn the walker controller
    batch = []
    walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
    for i in range(len(walkers_list)):
      batch.append(SpawnActor(walker_controller_bp, carla.Transform(), walkers_list[i]["id"]))
    results = client.apply_batch_sync(batch, True)
    for i in range(len(results)):
      if results[i].error:
        logging.error(results[i].error)
      else:
        walkers_list[i]["con"] = results[i].actor_id
      # 4. we put altogether the walkers and controllers id to get the objects from their id
    for i in range(len(walkers_list)):
      all_id.append(walkers_list[i]["con"])
      all_id.append(walkers_list[i]["id"])

  all_actors = world.get_actors(all_id)

  # ensures client has received the last transform of the walkers we have just created
  tick()

  # 5. initialize each controller and set target to walk to (list is [controler, actor, controller, actor ...])
  # set how many pedestrians can cross the road
  world.set_pedestrians_cross_factor(percentagePedestriansCrossing)
  for i in range(0, len(all_id), 2):
    # start walker
    all_actors[i].start()
    # set walk to random point
    all_actors[i].go_to_location(world.get_random_location_from_navigation())
    # max speed
    all_actors[i].set_max_speed(float(walker_speed[int(i/2)]))

  print('Spawned %d vehicles and %d walkers.' % (len(vehicles_list), len(walkers_list)))


  return vehicles_list, walkers_list, all_id, all_actors, sensors_ret


# ======================================================================================================================
# -- Benchmarking functions --------------------------------------------------------------------------------------------
# ======================================================================================================================

def set_world_settings(world, args = None):

  if args == None:
    settings = world.get_settings()
    settings.synchronous_mode = False
    settings.fixed_delta_seconds = 0.0
    settings.no_rendering_mode = False
    world.apply_settings(settings)
  else:
    settings = world.get_settings()
    settings.synchronous_mode = args.sync
    settings.fixed_delta_seconds = args.fixed_dt if args.sync else 0.0
    settings.no_rendering_mode = args.no_render_mode
    world.apply_settings(settings)

def run_benchmark(world, sensors, n_vehicles, n_walkers, client, debug=False):
  global sensors_callback

  spawn_points = world.get_map().get_spawn_points()
  n = min(n_vehicles, len(spawn_points))
  list_fps = []
  sensor_list = None

  tick = world.tick if args.sync else world.wait_for_tick
  set_world_settings(world, args)

  vehicles_list, walkers_list, all_id, all_actors, sensors_ret = create_environment(world, sensors, n, n_walkers, spawn_points, client, tick)

  if sensors_ret:
    sensor_list = sensors_ret

  # Allow some time for the server to finish the initialization
  for _i in range(0, 50):
    tick()

  ticks = 0
  while ticks < int(args.ticks):
    _ = tick()
    if debug:
      print("== Samples {} / {}".format(ticks + 1, args.ticks))

    min_fps = float('inf')
    for sc in sensors_callback:
      fps = sc.get_fps()
      if fps < min_fps:
        min_fps = fps
    if math.isinf(min_fps):
      min_fps = 0
    list_fps.append(min_fps)

    ticks += 1

  for sensor in sensor_list:
    sensor.stop()
    sensor.destroy()
  sensors_callback.clear()

  print('Destroying %d vehicles.\n' % len(vehicles_list))
  client.apply_batch([carla.command.DestroyActor(x) for x in vehicles_list])

  # stop walker controllers (list is [controller, actor, controller, actor ...])
  for i in range(0, len(all_id), 2):
    all_actors[i].stop()

  print('\ndestroying %d walkers' % len(walkers_list))
  client.apply_batch([carla.command.DestroyActor(x) for x in all_id])

  set_world_settings(world)

  return list_fps


def compute_mean_std(list_values):
    np_values = np.array(list_values)

    mean = np.mean(np_values)
    std = np.std(np_values)

    return mean, std


def serialize_records(records, system_specs, filename):
  with open(filename, 'w+') as fd:
    s = "| Town | Sensors | Weather | # of Vehicles | # of Walkers | Samples | Mean FPS | Std FPS |\n"
    s += "| ----------- | ----------- | ----------- | ----------- | ----------- | ----------- | ----------- |\n"
    fd.write(s)

    for sensor_key in sorted(records.keys()):
      list_records = records[sensor_key]
      for record in list_records:
        s = "| {} | {} | {} | {} | {} | {} | {:03.2f} | {:03.2f} |\n".format(record['town'],
                                                                    record['sensors'],
                                                                    record['weather'],
                                                                    record['n_vehicles'],
                                                                    record['n_walkers'],
                                                                    record['samples'],
                                                                    record['fps_mean'],
                                                                    record['fps_std'])
        fd.write(s)

    s = "\n| Global mean FPS | Global std FPS |\n"
    s += "| **{:03.2f}** | **{:03.2f}** |\n".format(*get_total(records))
    fd.write(s)

    s = "Table: {}.\n".format(system_specs)
    fd.write(s)


def get_total(records):
  record_vals = [item for sublist in records.values() for item in sublist]
  total_mean_fps = sum([r['fps_mean'] for r in record_vals]) / len(record_vals)
  total_mean_std = sum([r['fps_std'] for r in record_vals]) / len(record_vals)
  return total_mean_fps, total_mean_std


def get_system_specs():
  str_system = ""
  cpu_info = cpuinfo.get_cpu_info()
  str_system += "CPU {} {}. ".format(cpu_info.get('brand', 'Unknown'), cpu_info.get('family', 'Unknown'))

  memory_info = psutil.virtual_memory()
  str_system += "{:03.2f} GB RAM memory. ".format(memory_info.total / (1024 * 1024 * 1024))
  nvidia_cmd = shutil.which("nvidia-smi")
  if nvidia_cmd:
    str_system += "GPU "
    gpu_info = GPUtil.getGPUs()
    for gpu in gpu_info:
      str_system += "{} ".format(gpu.name)

  return str_system


def show_benchmark_scenarios(maps):
  print("Available maps")
  for map in sorted(maps):
    print("  - %s" % map)
  print("Available sensors")
  for i,sensors in enumerate(define_sensors()):
    sensor_str = ""
    for sensor in sensors:
      sensor_str += (sensor['label'] + " ")
    print('  - %s' % (sensor_str))
  print("Available types of weather")
  for i,weather in enumerate(define_weather()):
    print('  - %i: %s' % (i, weather['name']))
  print("Available Enviroments")
  for i,env in enumerate(define_environments()):
    print('  - %i: %s' % (i, str(env)))


def main(args):

  try:
    client = carla.Client(args.host, int(args.port))
    client.set_timeout(150.0)
    pygame.init()

    records = {}
    maps = define_maps(client)

    if args.show_scenarios:
      show_benchmark_scenarios(maps)
      return

    #maps = ["Town04_Opt"]

    for town in maps:
      world = client.load_world(town)
      time.sleep(5)

      # set to async mode
      set_world_settings(world)

      # spectator pointing to the sky to reduce rendering impact
      spectator = world.get_spectator()
      spectator.set_transform(carla.Transform(carla.Location(z=500), carla.Rotation(pitch=90)))

      for weather in define_weather():
        world.set_weather(weather["parameter"])
        for env in define_environments():
          for sensors in define_sensors():
            list_fps = run_benchmark(world, sensors, env["vehicles"], env["walkers"], client)
            mean, std = compute_mean_std(list_fps)
            sensor_str = ""
            for sensor in sensors:
              sensor_str += (sensor['label'] + " ")

            record = {
              'town': town,
              'sensors': sensor_str,
              'weather': weather["name"],
              'n_vehicles': env["vehicles"],
              'n_walkers': env["walkers"],
              'samples': args.ticks,
              'fps_mean': mean,
              'fps_std': std
            }

            env_str = str(env["vehicles"]) + str(env["walkers"])

            if env_str not in records:
              records[env_str] = []
            records[env_str].append(record)
            print(record)

    system_specs = get_system_specs()
    serialize_records(records, system_specs, args.file)
    pygame.quit()

  except KeyboardInterrupt:
      set_world_settings(world)
      client.reload_world()
      print('\nCancelled by user. Bye!')


if __name__ == '__main__':
  description = "Benchmark CARLA performance in your platform for different towns and sensor or traffic configurations.\n"

  parser = argparse.ArgumentParser(description=description)
  parser.add_argument('--host', default='localhost', help='IP of the host server (default: localhost)')
  parser.add_argument('--port', default='2000', help='TCP port to listen to (default: 2000)')
  parser.add_argument('--file', type=str, help='Write results into a txt file', default="benchmark.md")
  parser.add_argument('--tm', action='store_true', help='Switch to traffic manager benchmark')
  parser.add_argument('--ticks', default=100, help='Number of ticks for each scenario (default: 100)')
  parser.add_argument('--sync', default=True, action='store_true', help='Synchronous mode execution (default)')
  parser.add_argument('--async', dest='sync', action='store_false', help='Asynchronous mode execution')
  parser.add_argument('--fixed_dt', type=float, default=0.05, help='Time interval for the simulator in synchronous mode (default: 0.05)')
  parser.add_argument('--render_mode', dest='no_render_mode', action='store_false', help='Execute with spectator')
  parser.add_argument('--no_render_mode', default=True, action='store_true', help='Execute in no rendering mode (default)')
  parser.add_argument('--show_scenarios', default=False, action='store_true', help='Show the scenarios to benchmark and return (default=False)')
  parser.add_argument('--sensors', nargs="+", default=None, help='List of sensors to benchmark, by default all defined ones')
  parser.add_argument('--maps', nargs="+", default=None, help='List of maps to benchmark, by default all defined ones')
  parser.add_argument('--weather', nargs="+", default=None, help='List of weather types to benchmark, by default all defined ones')

  args = parser.parse_args()

  main(args)

