# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from . import SyncSmokeTest

import carla

class TestCollisionSensor(SyncSmokeTest):
     # 等待指定帧数的函数
    def wait(self, frames=100):
        for _i in range(0, frames):
            self.world.tick()
# 碰撞回调函数，用于记录碰撞事件
    def collision_callback(self, event, event_list):
        event_list.append(event)
 # 运行单辆车与墙碰撞的测试
    def run_collision_single_car_against_wall(self, bp_vehicle):
        # 设置车辆的初始位置和朝向
        veh_transf = carla.Transform(carla.Location(30, -6, 1), carla.Rotation(yaw=-90))
         # spawn车辆
        vehicle = self.world.spawn_actor(bp_vehicle, veh_transf)
# 获取碰撞传感器的蓝图并生成碰撞传感器
        bp_col_sensor = self.world.get_blueprint_library().find('sensor.other.collision')
        col_sensor = self.world.spawn_actor(bp_col_sensor, carla.Transform(), attach_to=vehicle)
 # 初始化事件列表
        event_list = []
         # 注册碰撞传感器的监听回调
        col_sensor.listen(lambda data: self.collision_callback(data, event_list))
# 等待100帧
        self.wait(100)
          # 设置车辆的目标速度
        vehicle.set_target_velocity(10.0*veh_transf.rotation.get_forward_vector())
# 再等待100帧
        self.wait(100)
  # 销毁碰撞传感器和车辆
        col_sensor.destroy()
        vehicle.destroy()
 # 返回碰撞事件列表
        return event_list
 # 测试单辆车的碰撞检测
    def test_single_car(self):
        print("TestCollisionSensor.test_single_car")
 # 获取所有车辆蓝图
        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")
         # 过滤适用于旧城镇的车辆蓝图
        bp_vehicles = self.filter_vehicles_for_old_towns(bp_vehicles)
        cars_failing = ""
        for bp_veh in bp_vehicles:
                # 对每辆车运行与墙的碰撞测试
            # Run collision agains wall
            event_list = self.run_collision_single_car_against_wall(bp_veh)
 # 如果没有检测到碰撞事件，则记录车辆ID
            if len(event_list) == 0:
                cars_failing += " %s" % bp_veh.id

        # Check result events
         # 检查结果事件
        if cars_failing != "":
             # 如果有车辆未能检测到碰撞，则测试失败
            self.fail("The collision sensor have failed for the cars: %s" % cars_failing)
