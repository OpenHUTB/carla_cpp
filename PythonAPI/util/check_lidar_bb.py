#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""

Lidar/BB check for CARLA
This script obtains the LiDAR's point cloud corresponding to all the vehicles
of the scene and make sure that they are inside the bounding box of the 
corresponding actor.
This is done in a predefined route in Town03 with a high speed and several agressive 
turns.

In a nutshell, the script have a queue that is filled in each frame with a lidar point 
cloud and an structure for storing the Bounding Boxes. This last one is emulated as a 
sensor filling the queue in the on_tick callback of the carla.world. In this way, we make
sure that we are correctly syncronizing the lidar point cloud and BB/actor transformations.
Then, we select the points corresponding to each actor (car) in the scene and check they
are inside the bounding boxes of that actor, all in each vehicle frame of reference.

Important Data structure description:
  + Lidar data structure: four element tuple with:
      - [0] Frame
      - [1] Sensor name: 'semlidar'
      - [2] Point cloud in the form of a numpy dictionary with all semantic lidar information
      - [3] Global transformation of the sensor
  + Bounding box data structure: four element tuple with:
      - [0] Frame
      - [1] Sensor name: 'bb'
      - [2] List of actor information: each a tuple with:
            - [0] Actor id
            - [1] Actor type (blueprint's name)
            - [0] Actor's global transformation
            - [0] Actor's bounding box
 + ActorTrace class: Takes the Lidar data structure and one actor information and
    check if all the data points related with this actor are inside its BB.
    This is done in the local coordinate frame of the actor and should be done like:
        trace = ActorTrace(actor_info, lidar_data)
        trace.process()
        trace.check_lidar_data()


"""

import glob
import os
import sys
import numpy as np
from queue import Queue
from queue import Empty

try:
    # 使用 `sys.path.append()` 函数尝试将一个路径添加到Python的模块搜索路径列表 `sys.path` 中。Python在导入模块时，会按照 `sys.path` 中列出的路径顺序去查找对应的模块文件，
    # 通过添加新的路径，可以让Python找到位于该路径下的自定义模块或者第三方库模块等，以便在代码中正常导入和使用它们。
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla


class ActorTrace(object):
    """Class that store and process information about an actor at certain moment."""
    def __init__(self, actor, lidar):
        self.set_lidar(lidar)
        self.set_actor(actor)
# 初始化本地坐标系下的激光雷达点云数据为空数组
        self._lidar_pc_local = np.array([])
# 初始化边界框顶点为空数组
        self._bb_vertices = np.array([])
# 初始化边界框的最小和最大坐标限制
        self._bb_minlimits = [0, 0, 0] # 定义边界框的最小坐标，初始值为（0，0，0）
        self._bb_maxlimits = [0, 0, 0] # 定义边界框的最大坐标，初始值为（0，0，0）

    def set_lidar(self, lidar):
        self._frame = lidar[0]                      # 帧信息
        self._lidar_data = lidar[2]              # 点云数据
        self._lidar_transf = lidar[3]            # 激光雷达的全局变换

    def set_actor(self, actor):
        self._actor_id = actor[0]              # 行为者ID
        self._actor_type = actor[1]             # 行为者类型
        self._actor_transf = actor[2]            # 行为者的全局变换
        self._actor_bb = actor[3]             # 行为者的边界框

    def process(self):
 # 过滤出与当前行为者ID对应的激光雷达点云数据
        # Filter lidar points that correspond to my actor id
        data_actor = self._lidar_data[self._lidar_data['ObjIdx'] == self._actor_id]
  # 将点云数据从传感器坐标系转换到世界坐标系，再转换到行为者本地坐标系
        # Take the xyz point cloud data and transform it to actor's frame
        points = np.array([data_actor['x'], data_actor['y'], data_actor['z']]).T                                    # 添加齐次坐标
        points = np.append(points, np.ones((points.shape[0], 1)), axis=1)
        points = np.dot(self._lidar_transf.get_matrix(), points.T).T         # sensor -> world                 # 传感器 -> 世界
        points = np.dot(self._actor_transf.get_inverse_matrix(), points.T).T # world -> actor                 
        points = points[:, :-1]                      # 移除齐次坐标

        # 保存转换后的点云数据到本地坐标系
        self._lidar_pc_local = points

        # 计算边界框在本地坐标系下的顶点
        vertices = self._actor_bb.get_local_vertices()
        ver_py = []
        for v in vertices:
            ver_py.append([v.x, v.y, v.z])
        ver_np = np.array(ver_py)

        self._bb_vertices = ver_np

 # 计算边界框的最小和最大坐标限制（加减一个小数以避免浮点误差）
        self._bb_minlimits = ver_np.min(axis=0) - 0.001
        self._bb_maxlimits = ver_np.max(axis=0) + 0.001

    def print(self, print_if_empty = False):
        if self._lidar_pc_local.shape[0] > 0 or print_if_empty:
            np.savetxt("veh_data_%d_%s_%d.out" % (self._frame, self._actor_type, self._actor_id), self._lidar_pc_local)
            np.savetxt("bb_data_%d_%s_%d.out"  % (self._frame, self._actor_type, self._actor_id), self._bb_vertices)

    def lidar_is_outside_bb(self, check_axis=[True, True, True]):
        """
        检查激光雷达点云是否超出了边界框的范围。
 
        参数:
            check_axis (list of bool): 一个包含三个布尔值的列表，分别表示是否检查x、y、z轴。
 
        返回:
            bool: 如果点云在任一检查的轴上超出了边界框，则返回True；否则返回False。
        """
        # 获取本地激光雷达点云数据
        lidar_pc = self._lidar_pc_local
 
        # 检查x轴
        if check_axis[0]:
            # 获取边界框在x轴上的最小和最大限制
            xmin, xmax = self._bb_minlimits[0], self._bb_maxlimits[0]
            # 检查点云中是否有任何点在x轴上超出了边界框的范围
            if np.any((lidar_pc[:, 0] > xmax) | (lidar_pc[:, 0] < xmin)):
                print("Problem with x axis")
                return True
 
        # 检查y轴（逻辑与x轴相同）
        if check_axis[1]:
            ymin, ymax = self._bb_minlimits[1], self._bb_maxlimits[1]
            if np.any((lidar_pc[:, 1] > ymax) | (lidar_pc[:, 1] < ymin)):
                print("Problem with y axis")
                return True
 
        # 检查z轴（逻辑与x轴和y轴相同）
        if check_axis[2]:
            zmin, zmax = self._bb_minlimits[2], self._bb_maxlimits[2]
            if np.any((lidar_pc[:, 2] > zmax) | (lidar_pc[:, 2] < zmin)):
                print("Problem with z axis")
                return True
 
        # 如果点云在所有检查的轴上都没有超出边界框，则返回False
        return False
    
    def check_lidar_data(self):
        if self.lidar_is_outside_bb():
            print("Error!!! Points of lidar point cloud are outside its BB for car %d: %s " % (self._actor_id, self._actor_type))
            self.print()
            return False
        else:
            return True

def wait(world, frames=100, queue = None, slist = None):
    for i in range(0, frames):
        world.tick()

        if queue != None and slist != None:
            try:
                for _i in range (0, len(slist)):
                    s_frame = queue.get(True, 1.0)
            except Empty:
                print("    Some of the sensor information is missed")

# Sensor callback.
# This is where you receive the sensor data and 
# process it as you liked and the important part is that,
# at the end, it should include an element into the sensor queue.
def lidar_callback(sensor_data, sensor_queue, sensor_name):
    sensor_pc_local = np.frombuffer(sensor_data.raw_data, dtype=np.dtype([
        ('x', np.float32), ('y', np.float32), ('z', np.float32),
        ('CosAngle', np.float32), ('ObjIdx', np.uint32), ('ObjTag', np.uint32)]))
    sensor_transf = sensor_data.transform
    sensor_queue.put((sensor_data.frame, sensor_name, sensor_pc_local, sensor_transf))

# 定义一个回调函数，用于处理车辆的边界框（bounding box）数据
def bb_callback(snapshot, world, sensor_queue, sensor_name):
    data_array = []

  # 获取所有车辆
    vehicles = world.get_actors().filter('vehicle.*')
    for actor in vehicles:
      # 将车辆的id、类型、变换和边界框信息添加到数组中
        data_array.append((actor.id, actor.type_id, actor.get_transform(), actor.bounding_box))

  # 将数据放入传感器队列中
    sensor_queue.put((snapshot.frame, sensor_name, data_array))

# 定义一个函数，用于移动观察者的位置
def move_spectator(world, actor):
    actor_tr = actor.get_transform()
    spectator_transform = carla.Transform(actor_tr.location, actor_tr.rotation)
  # 将观察者的位置向后移动5个单位，向上移动3个单位
    spectator_transform.location -= actor_tr.get_forward_vector() * 5
    spectator_transform.location -= actor_tr.get_up_vector() * 3
    spectator = world.get_spectator()
    spectator.set_transform(spectator_transform)

# 定义一个回调函数，用于处理世界信息和边界框数据
def world_callback(snapshot, world, sensor_queue, sensor_name, actor):
  # 移动观察者的位置
    move_spectator(world, actor)
    bb_callback(snapshot, world, sensor_queue, sensor_name)

# 定义一个函数，用于处理传感器数据
def process_sensors(w_frame, sensor_queue, sensor_number):
    if sensor_number != 2:
        print("Error!!! Sensor number should be two")

    sl_data = None
    bb_data = None

    try:
        for i in range (0, sensor_number):
            s_frame = sensor_queue.get(True, 1.0)
            while s_frame[0] != w_frame:
                print("Warning! Missmatch for sensor %s in the frame timestamp (w: %d, s: %d)" % (s_frame[1], w_frame, s_frame[0]))
                print("This could be due to accumulated data for previous steps")
                s_frame = sensor_queue.get(True, 1.0)

            if s_frame[1] == "semlidar":
                sl_data = s_frame
            elif s_frame[1] == "bb":
                bb_data = s_frame
            #print("    Frame: %d   Sensor: %s Len: %d " % (s_frame[0], s_frame[1], len(s_frame[2])))
    except Empty:
        print("Error!!! The needeinformation is not here!!!")
        return

    if sl_data == None or bb_data == None:
        print("Error!!! Missmatch for sensor %s in the frame timestamp (w: %d, s: %d)" % (s_frame[1], w_frame, s_frame[0]))

  # 处理边界框数据和激光雷达数据
    for actor_data in bb_data[2]:
        trace_vehicle = ActorTrace(actor_data, sl_data)
        trace_vehicle.process()
        trace_vehicle.check_lidar_data()

# 定义一个类，用于生成车辆
class SpawnCar(object):
    def __init__(self, location, rotation, filter="vehicle.*", autopilot = False, velocity = None):
        self._filter = filter
        self._transform = carla.Transform(location, rotation)
        self._autopilot = autopilot
        self._velocity = velocity
        self._actor = None
        self._world = None
    
    def spawn(self, world):
        self._world = world
        actor_BP = world.get_blueprint_library().filter(self._filter)[0]
        self._actor = world.spawn_actor(actor_BP, self._transform)
        self._actor.set_autopilot(True)

        return self._actor

    def destroy(self):
        if self._actor != None:
            self._actor.destroy()


# 定义一个车辆属性列表，用于生成多种类型的车辆
CarPropList = [
    SpawnCar(carla.Location(x=83,  y= -40, z=5),  carla.Rotation(yaw=-90),  filter= "mkz_2017", autopilot=True),
    SpawnCar(carla.Location(x=83,  y= -30, z=3),  carla.Rotation(yaw=-90),  filter= "ambulance", autopilot=True),
    SpawnCar(carla.Location(x=83,  y= -20, z=3),  carla.Rotation(yaw=-90),  filter= "etron", autopilot=True),
    SpawnCar(carla.Location(x=120, y= -3.5, z=2), carla.Rotation(yaw=+180), filter= "microlino", autopilot=True),
    SpawnCar(carla.Location(x=100, y= -3.5, z=2), carla.Rotation(yaw=+180), filter= "coupe_2020", autopilot=True),
    SpawnCar(carla.Location(x=140, y= -3.5, z=2), carla.Rotation(yaw=+180), filter= "model3", autopilot=True),
    SpawnCar(carla.Location(x=160, y= -3.5, z=2), carla.Rotation(yaw=+180), filter= "impala", autopilot=False),
    SpawnCar(carla.Location(x=180, y= -3.5, z=2), carla.Rotation(yaw=+180), filter= "a2", autopilot=True),
    SpawnCar(carla.Location(x=60,  y= +6, z=2),   carla.Rotation(yaw=+00),  filter= "sprinter", autopilot=True),
    SpawnCar(carla.Location(x=80,  y= +6, z=2),   carla.Rotation(yaw=+00),  filter= "t2", autopilot=True),
    SpawnCar(carla.Location(x=100, y= +6, z=2),   carla.Rotation(yaw=+00),  filter= "mustang", autopilot=True),
    SpawnCar(carla.Location(x=120, y= +6, z=2),   carla.Rotation(yaw=+00),  filter= "patrol_2021", autopilot=True),
    SpawnCar(carla.Location(x=140, y= +6, z=2),   carla.Rotation(yaw=+00),  filter= "impala", autopilot=True),
    SpawnCar(carla.Location(x=160, y= +6, z=2),   carla.Rotation(yaw=+00),  filter= "prius", autopilot=True),
    SpawnCar(carla.Location(x=234, y= +20,z=2),   carla.Rotation(yaw=+90),  filter= "charger_police_2020", autopilot=True),
    SpawnCar(carla.Location(x=234, y= +40,z=2),   carla.Rotation(yaw=+90),  filter= "microlino", autopilot=True),
    SpawnCar(carla.Location(x=234, y= +80,z=2),   carla.Rotation(yaw=+90),  filter= "tt", autopilot=True),
    SpawnCar(carla.Location(x=243, y= -40,z=2),   carla.Rotation(yaw=-90),  filter= "etron", autopilot=True),
    SpawnCar(carla.Location(x=243, y= -20,z=2),   carla.Rotation(yaw=-90),  filter= "mkz_2017", autopilot=True),
    SpawnCar(carla.Location(x=243, y= +00,z=2),   carla.Rotation(yaw=-90),  filter= "mustang", autopilot=True),
    SpawnCar(carla.Location(x=243, y= +20,z=2),   carla.Rotation(yaw=-90),  filter= "cooper_s_2021", autopilot=True),
    SpawnCar(carla.Location(x=243, y= +40,z=2),   carla.Rotation(yaw=-90),  filter= "charger_2020", autopilot=True),
    SpawnCar(carla.Location(x=243, y= +60,z=2),   carla.Rotation(yaw=-90),  filter= "mkz_2020", autopilot=True),
    SpawnCar(carla.Location(x=243, y= +80,z=2),   carla.Rotation(yaw=-90),  filter= "tt", autopilot=True),
    SpawnCar(carla.Location(x=243, y=+100,z=2),   carla.Rotation(yaw=-90),  filter= "a2", autopilot=True),
    SpawnCar(carla.Location(x=243, y=+120,z=2),   carla.Rotation(yaw=-90),  filter= "wrangler_rubicon", autopilot=True),
    SpawnCar(carla.Location(x=243, y=+140,z=2),   carla.Rotation(yaw=-90),  filter= "c3", autopilot=True)
]

# 定义一个函数，用于生成所有车辆
def spawn_prop_vehicles(world):
    for car in CarPropList:
        car.spawn(world)

# 定义一个函数，用于销毁所有车辆
def destroy_prop_vehicles():
    for car in CarPropList:
        car.destroy()


def main():
  # 创建CARLA客户端
    # We start creating the client
    client = carla.Client('localhost', 2000)
    client.set_timeout(2.0)
    world = client.get_world()

    try:
      # 保存原始设置以便恢复
        # We need to save the settings to be able to recover them at the end
        # of the script to leave the server in the same state that we found it.
        original_settings = world.get_settings()
        settings = world.get_settings()

        # We set CARLA syncronous mode
        settings.fixed_delta_seconds = 0.05
        settings.synchronous_mode = True
        world.apply_settings(settings)

        traffic_manager = client.get_trafficmanager(8000)
        traffic_manager.set_synchronous_mode(True)

        # We create the sensor queue in which we keep track of the information
        # already received. This structure is thread safe and can be
        # accessed by all the sensors callback concurrently without problem.
        sensor_queue = Queue()

        # Spawning ego vehicle
        actor_BP = world.get_blueprint_library().filter("vehicle.lincoln.mkz_2017")[0]
        car_tr = carla.Transform(carla.Location(x=239, y=125, z=0.9), carla.Rotation(yaw=-88.5))
        actor = world.spawn_actor(actor_BP, car_tr)

        world.tick()
        move_spectator(world, actor)

        spawn_prop_vehicles(world)

        wait(world, 10)

        # We create all the sensors and keep them in a list for convenience.
        sensor_list = []

        lidar_bp = world.get_blueprint_library().find('sensor.lidar.ray_cast_semantic')
        lidar_bp.set_attribute('channels', '64')
        lidar_bp.set_attribute('points_per_second', '500000')
        lidar_bp.set_attribute('range', '300')
        lidar_bp.set_attribute('upper_fov', '10.0')
        lidar_bp.set_attribute('lower_fov', '-90.0')
        lidar_tr = carla.Transform(carla.Location(z=3), carla.Rotation(yaw=0))
        lidar = world.spawn_actor(lidar_bp, lidar_tr, attach_to=actor)
        lidar.listen(lambda data: lidar_callback(data, sensor_queue, "semlidar"))
        world.on_tick(lambda snapshot: world_callback(snapshot, world, sensor_queue, "bb", actor))
        sensor_list.append(lidar)
        sensor_list.append(actor) # actor acts as a 'sensor' to simplify bb-lidar data comparison 
        
        # Set autopilot for main vehicle
        actor.enable_constant_velocity(carla.Vector3D(20, 0, 0))

        for _i in range(0, 100):
            # Tick the server
            world.tick()
            w_frame = world.get_snapshot().frame
            process_sensors(w_frame, sensor_queue, len(sensor_list))

        actor.disable_constant_velocity()

    finally:
        world.apply_settings(original_settings)

        # Destroy all the actors
        destroy_prop_vehicles()
        for sensor in sensor_list:
            sensor.destroy()
        

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print(' - Exited by user.')
