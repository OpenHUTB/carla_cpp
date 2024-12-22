# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
# 导入CARLA模块和unittest测试框架
import carla

import unittest

# 测试VehicleControl类的默认值和通过参数设置的值
class TestVehicleControl(unittest.TestCase):
    def test_default_values(self):
 # 创建一个VehicleControl对象，使用默认值
        c = carla.VehicleControl()
 # 验证默认值
        self.assertEqual(c.throttle, 0.0)                                     # 油门应为0
        self.assertEqual(c.steer, 0.0)                                         # 方向盘转角应为0
        self.assertEqual(c.brake, 0.0)                                       # 刹车应为0
        self.assertEqual(c.hand_brake, False)                             # 手刹应为False
        self.assertEqual(c.reverse, False)                                     # 倒车应为False
      # 使用参数创建一个VehicleControl对象
        c = carla.VehicleControl(1.0, 2.0, 3.0, True, True)
   # 验证通过参数设置的值  
        self.assertEqual(c.throttle, 1.0)
        self.assertEqual(c.steer, 2.0)
        self.assertEqual(c.brake, 3.0)
        self.assertEqual(c.hand_brake, True)
        self.assertEqual(c.reverse, True)
# 使用命名参数创建一个VehicleControl对象
    def test_named_args(self):
        c = carla.VehicleControl(
            throttle=1.0,
            steer=2.0,
            brake=3.0,
            hand_brake=True,
            reverse=True)
# 验证通过命名参数设置的值
        self.assertEqual(c.throttle, 1.0)
        self.assertEqual(c.steer, 2.0)
        self.assertEqual(c.brake, 3.0)
        self.assertEqual(c.hand_brake, True)
        self.assertEqual(c.reverse, True)

# 测试VehiclePhysicsControl类的命名参数设置
class TestVehiclePhysicsControl(unittest.TestCase):
    def test_named_args(self):

 # 定义扭矩曲线，表示发动机转速与扭矩的关系
        torque_curve = [[0, 400],
                        [24, 56],
                        [24, 56],
                        [1315.47, 654.445],
                        [5729, 400]]

# 定义方向盘转角与转向比的曲线
        steering_curve = [carla.Vector2D(x=0, y=1),
                          carla.Vector2D(x=20.0, y=0.9),
                          carla.Vector2D(x=63.0868, y=0.703473),
                          carla.Vector2D(x=119.12, y=0.573047)]

 # 定义每个车轮的物理参数
        wheels = [carla.WheelPhysicsControl(tire_friction=2, damping_rate=0, max_steer_angle=30, radius=10),
                  carla.WheelPhysicsControl(tire_friction=3, damping_rate=1, max_steer_angle=40, radius=20),
                  carla.WheelPhysicsControl(tire_friction=4, damping_rate=2, max_steer_angle=50, radius=30),
                  carla.WheelPhysicsControl(tire_friction=5, damping_rate=3, max_steer_angle=60, radius=40)]

# 使用命名参数创建一个VehiclePhysicsControl对象
        pc = carla.VehiclePhysicsControl(
            torque_curve=torque_curve,                                            # 扭矩曲线
            max_rpm=5729,                                                                # 最大转速
            moi=1,                                                                             # 转动惯量
            damping_rate_full_throttle=0.15,                                     # 全油门时的阻尼率
            damping_rate_zero_throttle_clutch_engaged=2,                 # 离合器接合时零油门的阻尼率
            damping_rate_zero_throttle_clutch_disengaged=0.35,        # 离合器断开时零油门的阻尼率

            use_gear_autobox=1,           # 是否使用自动变速箱
            gear_switch_time=0.5,           # 换挡时间
            clutch_strength=10,              # 离合器强度
                                                
            mass=5500,                      # 车辆质量
            drag_coefficient=0.3,      # 空气阻力系数

            center_of_mass=carla.Vector3D(x=0.5, y=1, z=1),        # 质心位置
            steering_curve=steering_curve,                               # 方向盘转角与转向比的曲线
            wheels=wheels)                                             # 车轮物理参数


 # 定义一个允许的误差范围，用于浮点数比较
        error = .001
# 验证扭矩曲线的每个点
        for i in range(0, len(torque_curve)):
            self.assertTrue(abs(pc.torque_curve[i].x - torque_curve[i][0]) <= error)
            self.assertTrue(abs(pc.torque_curve[i].y - torque_curve[i][1]) <= error)

 # 验证其他物理参数
        self.assertTrue(abs(pc.max_rpm - 5729) <= error)
        self.assertTrue(abs(pc.moi - 1) <= error)
        self.assertTrue(abs(pc.damping_rate_full_throttle - 0.15) <= error)
        self.assertTrue(abs(pc.damping_rate_zero_throttle_clutch_engaged - 2) <= error)
        self.assertTrue(abs(pc.damping_rate_zero_throttle_clutch_disengaged - 0.35) <= error)

        self.assertTrue(abs(pc.use_gear_autobox - 1) <= error)
        self.assertTrue(abs(pc.gear_switch_time - 0.5) <= error)
        self.assertTrue(abs(pc.clutch_strength - 10) <= error)

        self.assertTrue(abs(pc.mass - 5500) <= error)
        self.assertTrue(abs(pc.drag_coefficient - 0.3) <= error)

        self.assertTrue(abs(pc.center_of_mass.x - 0.5) <= error)
        self.assertTrue(abs(pc.center_of_mass.y - 1) <= error)
        self.assertTrue(abs(pc.center_of_mass.z - 1) <= error)

 # 验证方向盘转角与转向比的曲线的每个点
        for i in range(0, len(steering_curve)):
            self.assertTrue(abs(pc.steering_curve[i].x - steering_curve[i].x) <= error)
            self.assertTrue(abs(pc.steering_curve[i].y - steering_curve[i].y) <= error)

 # 验证每个车轮的物理参数
        for i in range(0, len(wheels)):
            self.assertTrue(abs(pc.wheels[i].tire_friction - wheels[i].tire_friction) <= error)
            self.assertTrue(abs(pc.wheels[i].damping_rate - wheels[i].damping_rate) <= error)
            self.assertTrue(abs(pc.wheels[i].max_steer_angle - wheels[i].max_steer_angle) <= error)
            self.assertTrue(abs(pc.wheels[i].radius - wheels[i].radius) <= error)

            self.assertTrue(abs(pc.wheels[i].position.x - wheels[i].position.x) <= error)
            self.assertTrue(abs(pc.wheels[i].position.y - wheels[i].position.y) <= error)
            self.assertTrue(abs(pc.wheels[i].position.z - wheels[i].position.z) <= error)
      