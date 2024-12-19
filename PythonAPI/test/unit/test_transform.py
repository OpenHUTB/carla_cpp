# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla

import unittest
# 导入Carla模块，用于操作Carla相关的类和功能。
# 导入Python标准库中的unittest模块，用于编写单元测试用例。

class TestLocation(unittest.TestCase):
    def test_default_values(self):
        location = carla.Location()
        # 验证默认构造的Location对象，其x、y、z属性是否都为0.0。
        self.assertEqual(location.x, 0.0)
        self.assertEqual(location.y, 0.0)
        self.assertEqual(location.z, 0.0)
        location = carla.Location(1.0)
        # 验证使用一个参数构造Location对象时，x属性被赋值为传入参数，y和z属性为0.0。
        self.assertEqual(location.x, 1.0)
        self.assertEqual(location.y, 0.0)
        self.assertEqual(location.z, 0.0)
        location = carla.Location(1.0, 2.0)
        # 验证使用两个参数构造Location对象时，x和y属性被正确赋值，z属性为0.0。
        self.assertEqual(location.x, 1.0)
        self.assertEqual(location.y, 2.0)
        self.assertEqual(location.z, 0.0)
        location = carla.Location(1.0, 2.0, 3.0)
        # 验证使用三个参数构造Location对象时，x、y、z属性都能被正确赋值。
        self.assertEqual(location.x, 1.0)
        self.assertEqual(location.y, 2.0)
        self.assertEqual(location.z, 3.0)

    def test_named_args(self):
        location = carla.Location(x=42.0)
        # 验证使用命名参数x构造Location对象时，只有x属性被赋值为传入参数，y和z属性为0.0。
        self.assertEqual(location.x, 42.0)
        self.assertEqual(location.y, 0.0)
        self.assertEqual(location.z, 0.0)
        location = carla.Location(y=42.0)
        # 验证使用命名参数y构造Location对象时，只有y属性被赋值为传入参数，x和z属性为0.0。
        self.assertEqual(location.x, 0.0)
        self.assertEqual(location.y, 42.0)
        self.assertEqual(location.z, 0.0)
        location = carla.Location(z=42.0)
        # 验证使用命名参数z构造Location对象时，只有z属性被赋值为传入参数，x和y属性为0.0。
        self.assertEqual(location.x, 0.0)
        self.assertEqual(location.y, 0.0)
        self.assertEqual(location.z, 42.0)
        location = carla.Location(z=3.0, x=1.0, y=2.0)
        # 验证使用多个命名参数构造Location对象时，属性按参数名正确赋值。
        self.assertEqual(location.x, 1.0)
        self.assertEqual(location.y, 2.0)
        self.assertEqual(location.z, 3.0)


class TestRotation(unittest.TestCase):
    def test_default_values(self):
        rotation = carla.Rotation()
         # 验证默认构造的Rotation对象，其pitch、yaw、roll属性是否都为0.0。
        self.assertEqual(rotation.pitch, 0.0)
        self.assertEqual(rotation.yaw, 0.0)
        self.assertEqual(rotation.roll, 0.0)
        rotation = carla.Rotation(1.0)
        # 验证使用一个参数构造Rotation对象时，pitch属性被赋值为传入参数，yaw和roll属性为0.0。
        self.assertEqual(rotation.pitch, 1.0)
        self.assertEqual(rotation.yaw, 0.0)
        self.assertEqual(rotation.roll, 0.0)
        rotation = carla.Rotation(1.0, 2.0)
        # 验证使用两个参数构造Rotation对象时，pitch和yaw属性被正确赋值，roll属性为0.0。
        self.assertEqual(rotation.pitch, 1.0)
        self.assertEqual(rotation.yaw, 2.0)
        self.assertEqual(rotation.roll, 0.0)
        rotation = carla.Rotation(1.0, 2.0, 3.0)
         # 验证使用三个参数构造Rotation对象时，pitch、yaw、roll属性都能被正确赋值。
        self.assertEqual(rotation.pitch, 1.0)
        self.assertEqual(rotation.yaw, 2.0)
        self.assertEqual(rotation.roll, 3.0)

    def test_named_args(self):
        rotation = carla.Rotation(pitch=42.0)
         # 验证使用命名参数pitch构造Rotation对象时，只有pitch属性被赋值为传入参数，yaw和roll属性为0.0。
        self.assertEqual(rotation.pitch, 42.0)
        self.assertEqual(rotation.yaw, 0.0)
        self.assertEqual(rotation.roll, 0.0)
        rotation = carla.Rotation(yaw=42.0)
        # 验证使用命名参数yaw构造Rotation对象时，只有yaw属性被赋值为传入参数，pitch和roll属性为0.0。
        self.assertEqual(rotation.pitch, 0.0)
        self.assertEqual(rotation.yaw, 42.0)
        self.assertEqual(rotation.roll, 0.0)
        rotation = carla.Rotation(roll=42.0)
         # 验证使用命名参数roll构造Rotation对象时，只有roll属性被赋值为传入参数，pitch和yaw属性为0.0。
        self.assertEqual(rotation.pitch, 0.0)
        self.assertEqual(rotation.yaw, 0.0)
        self.assertEqual(rotation.roll, 42.0)
        rotation = carla.Rotation(roll=3.0, pitch=1.0, yaw=2.0)
        # 验证使用多个命名参数构造Rotation对象时，属性按参数名正确赋值。
        self.assertEqual(rotation.pitch, 1.0)
        self.assertEqual(rotation.yaw, 2.0)
        self.assertEqual(rotation.roll, 3.0)


class TestTransform(unittest.TestCase):
    def test_values(self):
        t = carla.Transform()
        # 验证默认构造的Transform对象，其内部的Location对象的x、y、z属性以及Rotation对象的pitch、yaw、roll属性是否都为0.0。
        self.assertEqual(t.location.x, 0.0)
        self.assertEqual(t.location.y, 0.0)
        self.assertEqual(t.location.z, 0.0)
        self.assertEqual(t.rotation.pitch, 0.0)
        self.assertEqual(t.rotation.yaw, 0.0)
        self.assertEqual(t.rotation.roll, 0.0)
        t = carla.Transform(carla.Location(y=42.0))
        # 验证使用Location对象构造Transform对象时，其内部的Location对象属性被正确赋值，Rotation对象属性保持默认值0.0。
        self.assertEqual(t.location.x, 0.0)
        self.assertEqual(t.location.y, 42.0)
        self.assertEqual(t.location.z, 0.0)
        self.assertEqual(t.rotation.pitch, 0.0)
        self.assertEqual(t.rotation.yaw, 0.0)
        self.assertEqual(t.rotation.roll, 0.0)
        t = carla.Transform(rotation=carla.Rotation(yaw=42.0))
        # 验证使用Rotation对象通过命名参数构造Transform对象时，其内部的Rotation对象属性被正确赋值，Location对象属性保持默认值0.0。
        self.assertEqual(t.location.x, 0.0)
        self.assertEqual(t.location.y, 0.0)
        self.assertEqual(t.location.z, 0.0)
        self.assertEqual(t.rotation.pitch, 0.0)
        self.assertEqual(t.rotation.yaw, 42.0)
        self.assertEqual(t.rotation.roll, 0.0)

    def test_print(self):
        t = carla.Transform(
            carla.Location(x=1.0, y=2.0, z=3.0),
            carla.Rotation(pitch=4.0, yaw=5.0, roll=6.0))
        s = 'Transform(Location(x=1.000000, y=2.000000, z=3.000000), Rotation(pitch=4.000000, yaw=5.000000, roll=6.000000))'
        # 验证Transform对象转换为字符串后的内容与预期的格式化字符串是否一致。
        self.assertEqual(str(t), s)

    def test_translation(self):
        error = .001
        t = carla.Transform(
            carla.Location(x=8.0, y=19.0, z=20.0),
            carla.Rotation(pitch=0.0, yaw=0.0, roll=0.0))
        point = carla.Location(x=0.0, y=0.0, z=0.0)
        t.transform(point)
        # 验证经过Transform对象（仅包含平移信息，无旋转）变换后的坐标点的x、y、z属性值与预期的平移值是否在误差范围内一致。
        self.assertTrue(abs(point.x - 8.0) <= error)
        self.assertTrue(abs(point.y - 19.0) <= error)
        self.assertTrue(abs(point.z - 20.0) <= error)

    def test_rotation(self):
        error = .001
        t = carla.Transform(
            carla.Location(x=0.0, y=0.0, z=0.0),
            carla.Rotation(pitch=180.0, yaw=0.0, roll=0.0))
        point = carla.Location(x=0.0, y=0.0, z=1.0)
        t.transform(point)
        # 验证经过Transform对象（仅包含绕x轴旋转180度信息，无平移）变换后的坐标点的z坐标值是否变为-1.0（绕x轴旋转180度的预期结果），在误差范围内进行比较。
        self.assertTrue(abs(point.x - 0.0) <= error)
        self.assertTrue(abs(point.x - 0.0) <= error)
        self.assertTrue(abs(point.y - 0.0) <= error)
        self.assertTrue(abs(point.z - (-1.0)) <= error)

    def test_rotation_and_translation(self):
        error = .001
        t = carla.Transform(
            carla.Location(x=0.0, y=0.0, z=-1.0),
            carla.Rotation(pitch=90.0, yaw=0.0, roll=0.0))
        point = carla.Location(x=0.0, y=0.0, z=2.0)
        t.transform(point)
        # 验证经过Transform对象（包含绕x轴旋转90度和平移信息）变换后的坐标点的x、y、z坐标值是否与预期的旋转和平移后的结果在误差范围内一致。
        self.assertTrue(abs(point.x - (-2.0)) <= error)
        self.assertTrue(abs(point.y - 0.0) <= error)
        self.assertTrue(abs(point.z - (-1.0)) <= error)
    #定义一个测试函数，用于测试点列表的旋转和平移
    def test_list_rotation_and_translation_location(self):
        error = .001
        t = carla.Transform(  
            carla.Location(x=0.0, y=0.0, z=-1.0),
            carla.Rotation(pitch=90.0, yaw=0.0, roll=0.0))
        #定义一个点列表
        point_list = [carla.Location(x=0.0, y=0.0, z=2.0),
                      carla.Location(x=0.0, y=10.0, z=1.0),
                      carla.Location(x=0.0, y=18.0, z=2.0)
                      ]
        t.transform(point_list)

        solution_list = [carla.Location(-2.0, 0.0, -1.0),
                         carla.Location(-1.0, 10.0, -1.0),
                         carla.Location(-2.0, 18.0, -1.0)
                         ]

        for i in range(len(point_list)):
             # 遍历坐标点列表，逐个验证每个坐标点经过变换后的坐标值与预期结果是否在误差范围内一致。
            self.assertTrue(abs(point_list[i].x - solution_list[i].x) <= error)
            self.assertTrue(abs(point_list[i].y - solution_list[i].y) <= error)
            self.assertTrue(abs(point_list[i].z - solution_list[i].z) <= error)

    def test_list_rotation_and_translation_vector3d(self):
        error = .001
        t = carla.Transform(
            carla.Location(x=0.0, y=0.0, z=-1.0),
            carla.Rotation(pitch=90.0, yaw=0.0, roll=0.0))

        point_list = [carla.Vector3D(0.0, 0.0, 2.0),
                      carla.Vector3D(0.0, 10.0, 1.0),
                      carla.Vector3D(0.0, 18.0, 2.0)
                      ]
        t.transform(point_list)
        #定义预期的变化后点列表
        solution_list = [carla.Vector3D(-2.0, 0.0, -1.0),
                         carla.Vector3D(-1.0, 10.0, -1.0),
                         carla.Vector3D(-2.0, 18.0, -1.0)
                         ]

        for i in range(len(point_list)):
            # 遍历坐标点列表，逐个验证每个坐标点经过变换后的坐标值与预期结果是否在误差范围内一致。
            self.assertTrue(abs(point_list[i].x - solution_list[i].x) <= error)
            self.assertTrue(abs(point_list[i].x - solution_list[i].x) <= error)
            self.assertTrue(abs(point_list[i].y - solution_list[i].y) <= error)
            self.assertTrue(abs(point_list[i].z - solution_list[i].z) <= error)
