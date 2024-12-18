import carla
from agents.navigation.basic_agent import BasicAgent

class ConstantVelocityAgent(BasicAgent):
    """
    ConstantVelocityAgent实现了一个以固定速度在场景中导航的智能体。
    如果要求该智能体以期望速度执行不可能的转弯（包括变道），它将会失败。
    当检测到碰撞时，恒定速度行为将停止，等待一段时间后再重新启动。
    """
    def __init__(self, vehicle, target_speed=20, opt_dict={}, map_inst=None, grp_inst=None):
        """
        初始化智能体参数、本地规划器和全局规划器。

        :param vehicle: 应用智能体逻辑的车辆actor
        :param target_speed: 车辆移动的速度（单位：Km/h）
        :param opt_dict: 用于更改部分参数的字典，也适用于与本地规划器相关的参数
        :param map_inst: carla.Map实例，避免重复获取的开销
        :param grp_inst: GlobalRoutePlanner实例，避免重复获取的开销
        """
        super().__init__(vehicle, target_speed, opt_dict=opt_dict, map_inst=map_inst, grp_inst=grp_inst)

        # 是否在恒定速度行为失效时使用BasicAgent的行为
        self._use_basic_behavior = False  
        # 将目标速度从km/h转换为m/s，方便后续计算
        self._target_speed = target_speed / 3.6  
        # 获取车辆当前速度（单位：m/s）
        self._current_speed = vehicle.get_velocity().length()  
        self._constant_velocity_stop_time = None
        self._collision_sensor = None

        # 碰撞后再次启动恒定速度行为之前等待的时间，初始化为正无穷
        self._restart_time = float('inf')  
        # 如果在opt_dict中指定了'restart_time'，则更新等待时间
        if 'restart_time' in opt_dict:
            self._restart_time = opt_dict['restart_time']
        # 如果在opt_dict中指定了'use_basic_behavior'，则更新对应标志
        if 'use_basic_behavior' in opt_dict:
            self._use_basic_behavior = opt_dict['use_basic_behavior']

        self.is_constant_velocity_active = True
        # 设置碰撞传感器
        self._set_collision_sensor()
        # 设置恒定速度
        self._set_constant_velocity(target_speed)

    def set_target_speed(self, speed):
        """
        更改智能体的目标速度（单位：km/h）
        """
        self._target_speed = speed / 3.6
        self._local_planner.set_speed(speed)

    def stop_constant_velocity(self):
        """
        停止恒定速度行为
        """
        self.is_constant_velocity_active = False
        self._vehicle.disable_constant_velocity()
        # 记录停止时的时间戳（从世界开始到当前的经过秒数）
        self._constant_velocity_stop_time = self._world.get_snapshot().timestamp.elapsed_seconds

    def restart_constant_velocity(self):
        """
        公开方法，用于重新启动恒定速度行为
        """
        self.is_constant_velocity_active = True
        self._set_constant_velocity(self._target_speed)

    def _set_constant_velocity(self, speed):
        """
        强制智能体以指定速度行驶
        """
        self._vehicle.enable_constant_velocity(carla.Vector3D(speed, 0, 0))

    def run_step(self):
        """
        执行一步导航操作。
        """
        if not self.is_constant_velocity_active:
            # 判断是否超过了重新启动的等待时间
            if self._world.get_snapshot().timestamp.elapsed_seconds - self._constant_velocity_stop_time > self._restart_time:
                self.restart_constant_velocity()
                self.is_constant_velocity_active = True
            # 如果允许使用基本行为且当前未达到重启条件，则执行基本行为的一步操作
            elif self._use_basic_behavior:
                return super(ConstantVelocityAgent, self).run_step()
            # 如果不使用基本行为且未达到重启条件，则返回空的车辆控制指令（车辆保持静止）
            else:
                return carla.VehicleControl()

        hazard_detected = False

        # 获取世界中的所有actor
        actor_list = self._world.get_actors()
        # 过滤出所有车辆actor
        vehicle_list = actor_list.filter("*vehicle*")
        # 过滤出所有交通信号灯actor
        lights_list = actor_list.filter("*traffic_light*")

        vehicle_speed = self._vehicle.get_velocity().length()

        # 根据车辆速度计算最大车辆检测距离
        max_vehicle_distance = self._base_vehicle_threshold + vehicle_speed
        # 检测车辆是否受到其他车辆影响，并获取相关信息
        affected_by_vehicle, adversary, _ = self._vehicle_obstacle_detected(vehicle_list, max_vehicle_distance)
        if affected_by_vehicle:
            vehicle_velocity = self._vehicle.get_velocity()
            if vehicle_velocity.length() == 0:
                hazard_speed = 0
            else:
                # 计算与障碍物车辆相对速度的相关值，用于判断危险程度
                hazard_speed = vehicle_velocity.dot(adversary.get_velocity()) / vehicle_velocity.length()
            hazard_detected = True

        # 根据车辆速度计算最大交通信号灯检测距离
        max_tlight_distance = self._base_tlight_threshold + 0.3 * vehicle_speed
        # 检测车辆是否受到交通信号灯影响
        affected_by_tlight, _ = self._affected_by_traffic_light(lights_list, max_tlight_distance)
        if affected_by_tlight:
            hazard_speed = 0
            hazard_detected = True

        # 纵向PID虽然被恒定速度覆盖，但应用它可以避免车辆车轮静止不动
        control = self._local_planner.run_step()
        if hazard_detected:
            self._set_constant_velocity(hazard_speed)
        else:
            self._set_constant_velocity(self._target_speed)

        return control

    def _set_collision_sensor(self):
        """
        设置碰撞传感器。
        找到碰撞传感器的蓝图，在世界中生成对应的actor并附着到车辆上，
        为其添加监听事件，当发生碰撞时调用stop_constant_velocity方法停止恒定速度行为。
        """
        blueprint = self._world.get_blueprint_library().find('sensor.other.collision')
        self._collision_sensor = self._world.spawn_actor(blueprint, carla.Transform(), attach_to=self._vehicle)
        self._collision_sensor.listen(lambda event: self.stop_constant_velocity())

    def destroy_sensor(self):
        """
        销毁碰撞传感器（如果存在的话），将对应的传感器对象设置为None。
        """
        if self._collision_sensor:
            self._collision_sensor.destroy()
            self._collision_sensor = None
