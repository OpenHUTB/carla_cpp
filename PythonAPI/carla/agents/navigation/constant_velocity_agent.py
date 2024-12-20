# Copyright (c) # Copyright (c) 2018-2020 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
这个模块实现了一个智能体（agent）类，该智能体能够在赛道上沿着随机生成的路径点漫游，同时具备避开其他车辆以及对交通信号灯做出响应的能力。
此外，它还可以利用全局路径规划器来按照指定的路线行驶。
"""

import carla

from agents.navigation.basic_agent import BasicAgent

class ConstantVelocityAgent(BasicAgent):
    """
   ConstantVelocityAgent类实现了一个以固定速度在场景中导航的智能体。
    当要求该智能体以期望速度执行无法完成的转弯操作（包括变道操作）时，它将会出现故障（无法按预期执行）。
    当检测到碰撞发生时，该智能体的固定速度行驶行为会停止，等待一段时间后，再重新启动固定速度行驶。
    """

    def __init__(self, vehicle, target_speed=20, opt_dict={}, map_inst=None, grp_inst=None):
        """
      初始化智能体的各项参数，以及本地规划器和全局规划器。

        参数说明：
        :param vehicle: 要应用智能体逻辑的车辆角色（actor），也就是该智能体所控制的具体车辆对象，智能体将基于这个车辆在场景中进行导航等操作。
        :param target_speed: 车辆行驶的目标速度，单位为千米/小时（Km/h），用于指定智能体期望车辆运行的速度值。
        :param opt_dict: 一个字典类型的参数，用于在需要更改某些智能体参数时进行相应设置，这个字典同样适用于和本地规划器相关的参数修改。
        :param map_inst: carla.Map实例对象，传入这个实例可以避免重复去获取地图对象的操作，因为获取地图对象可能是一个开销较大的调用，通过传入已有的实例可以提高效率。
        :param grp_inst: GlobalRoutePlanner实例对象，同理，传入这个实例可以避免重复获取全局路径规划器对象时的开销较大的调用，方便后续使用全局路径规划功能。
        """
        super().__init__(vehicle, target_speed, opt_dict=opt_dict, map_inst=map_inst, grp_inst=grp_inst)
# 调用父类（BasicAgent）的构造函数，传入相应参数来初始化一些基础属性和功能，完成继承自父类的初始化工作。
        self._use_basic_behavior = False  # Whether or not to use the BasicAgent behavior when the constant velocity is down
        self._target_speed = target_speed / 3.6  # [m/s]
        self._current_speed = vehicle.get_velocity().length()  # [m/s]
        self._constant_velocity_stop_time = None
        self._collision_sensor = None

        self._restart_time = float('inf')  # Time after collision before the constant velocity behavior starts again

        if 'restart_time' in opt_dict:
            self._restart_time = opt_dict['restart_time']
        if 'use_basic_behavior' in opt_dict:
            self._use_basic_behavior = opt_dict['use_basic_behavior']

        self.is_constant_velocity_active = True
        self._set_collision_sensor()
        self._set_constant_velocity(target_speed)

    def set_target_speed(self, speed):
        """ 改变智能体的目标速度，参数传入的速度单位为千米/小时（km/h）。

        参数：
        :param speed: 新的目标速度值，单位为千米/小时，用于更新智能体期望车辆达到的速度。"""
        self._target_speed = speed / 3.6
        self._local_planner.set_speed(speed)

    def stop_constant_velocity(self):
        """停止固定速度行驶行为，即让车辆不再按照固定速度行驶。"""
        self.is_constant_velocity_active = False
        self._vehicle.disable_constant_velocity()
        self._constant_velocity_stop_time = self._world.get_snapshot().timestamp.elapsed_seconds

    def restart_constant_velocity(self):
        """公开方法，用于重新启动固定速度行驶行为，使车辆恢复按照固定速度行驶的状态。"""
        self.is_constant_velocity_active = True
        self._set_constant_velocity(self._target_speed)

    def _set_constant_velocity(self, speed):
        """Forces the agent to drive at the specified speed"""
        self._vehicle.enable_constant_velocity(carla.Vector3D(speed, 0, 0))

    def run_step(self):
        """强制智能体所控制的车辆以指定的速度行驶。

        参数：
        :param speed: 期望的行驶速度，单位为米/秒，用于设置车辆的固定速度值。"""
        if not self.is_constant_velocity_active:
            if self._world.get_snapshot().timestamp.elapsed_seconds - self._constant_velocity_stop_time > self._restart_time:
                self.restart_constant_velocity()
                self.is_constant_velocity_active = True
            elif self._use_basic_behavior:
                return super(ConstantVelocityAgent, self).run_step()
            else:
                return carla.VehicleControl()

        hazard_detected = False

        # 获取当前世界中所有的角色（actor）列表
        actor_list = self._world.get_actors()
        vehicle_list = actor_list.filter("*vehicle*")
        lights_list = actor_list.filter("*traffic_light*")

        vehicle_speed = self._vehicle.get_velocity().length()

        max_vehicle_distance = self._base_vehicle_threshold + vehicle_speed
        affected_by_vehicle, adversary, _ = self._vehicle_obstacle_detected(vehicle_list, max_vehicle_distance)
        if affected_by_vehicle:
            vehicle_velocity = self._vehicle.get_velocity()
            if vehicle_velocity.length() == 0:
                hazard_speed = 0
            else:
                hazard_speed = vehicle_velocity.dot(adversary.get_velocity()) / vehicle_velocity.length()
            hazard_detected = True

        # 检测当前车辆是否受到交通信号灯的影响
        max_tlight_distance = self._base_tlight_threshold + 0.3 * vehicle_speed
        affected_by_tlight, _ = self._affected_by_traffic_light(lights_list, max_tlight_distance)
        if affected_by_tlight:
            hazard_speed = 0
            hazard_detected = True

        # 纵向PID（比例-积分-微分控制，一种常见的控制算法，这里可能用于车辆速度等方面的控制）虽然会被固定速度行为覆盖，但应用它仍有好处，比如可以避免车辆轮子静止不动等情况，先获取本地规划器执行一步后的控制指令。
        control = self._local_planner.run_step()
        if hazard_detected:
            self._set_constant_velocity(hazard_speed)
        else:
            self._set_constant_velocity(self._target_speed)

        return control

    def _set_collision_sensor(self):
        blueprint = self._world.get_blueprint_library().find('sensor.other.collision')
        self._collision_sensor = self._world.spawn_actor(blueprint, carla.Transform(), attach_to=self._vehicle)
        self._collision_sensor.listen(lambda event: self.stop_constant_velocity())

    def destroy_sensor(self):
        if self._collision_sensor:
            self._collision_sensor.destroy()
            self._collision_sensor = None
