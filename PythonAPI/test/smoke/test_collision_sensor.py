# ==============================================================================
# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
# ==============================================================================

from . import SyncSmokeTest
import carla

class TestCollisionSensor(SyncSmokeTest):
    """碰撞传感器测试类
    
    该类用于测试CARLA中车辆的碰撞传感器功能，包括：
    - 单车对墙碰撞测试
    - 多车型碰撞检测验证
    """

    def wait(self, frames=100):
        """等待指定帧数
        
        Args:
            frames (int): 需要等待的帧数，默认为100帧
        """
        for _i in range(0, frames):
            self.world.tick()

    def collision_callback(self, event, event_list):
        """碰撞事件回调函数
        
        Args:
            event: 碰撞事件数据
            event_list (list): 用于存储碰撞事件的列表
        """
        event_list.append(event)

    def run_collision_single_car_against_wall(self, bp_vehicle):
        """运行单车对墙碰撞测试
        
        该方法执行以下步骤：
        1. 在指定位置生成测试车辆
        2. 安装碰撞传感器
        3. 设置车辆运动
        4. 收集碰撞数据
        
        Args:
            bp_vehicle: 车辆蓝图
            
        Returns:
            list: 碰撞事件列表
        """
        # 设置车辆初始状态
        veh_transform = carla.Transform(
            carla.Location(30, -6, 1),   # x=30, y=-6, z=1
            carla.Rotation(yaw=-90)      # 朝向左侧
        )
        
        try:
            # 生成车辆
            vehicle = self.world.spawn_actor(bp_vehicle, veh_transform)
            
            # 配置碰撞传感器
            bp_collision_sensor = self.world.get_blueprint_library().find('sensor.other.collision')
            collision_sensor = self.world.spawn_actor(
                bp_collision_sensor, 
                carla.Transform(),    # 使用默认变换
                attach_to=vehicle     # 附加到车辆上
            )
            
            # 设置碰撞检测
            collision_events = []
            collision_sensor.listen(
                lambda data: self.collision_callback(data, collision_events)
            )
            
            # 执行碰撞测试
            self.wait(100)  # 等待初始化
            
            # 设置车辆运动
            forward_vector = veh_transform.rotation.get_forward_vector()
            target_velocity = 10.0 * forward_vector  # 设置10.0的速度
            vehicle.set_target_velocity(target_velocity)
            
            self.wait(100)  # 等待碰撞发生
            
            return collision_events
            
        finally:
            # 清理资源
            if 'collision_sensor' in locals():
                collision_sensor.destroy()
            if 'vehicle' in locals():
                vehicle.destroy()

    def test_single_car(self):
        """测试所有车型的碰撞检测功能
        
        该测试会：
        1. 获取所有可用的车辆蓝图
        2. 对每种车型进行碰撞测试
        3. 验证碰撞检测的有效性
        """
        print("TestCollisionSensor.test_single_car")

        # 获取并过滤车辆蓝图
        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")
        bp_vehicles = self.filter_vehicles_for_old_towns(bp_vehicles)
        
        # 记录测试失败的车辆
        failed_vehicles = []
        
        # 测试每种车型
        for bp_vehicle in bp_vehicles:
            collision_events = self.run_collision_single_car_against_wall(bp_vehicle)
            
            # 检查碰撞检测结果
            if len(collision_events) == 0:
                failed_vehicles.append(bp_vehicle.id)
        
        # 验证测试结果
        if failed_vehicles:
            self.fail(
                "碰撞传感器在以下车型中检测失败: {}".format(
                    ", ".join(failed_vehicles)
                )
            )
