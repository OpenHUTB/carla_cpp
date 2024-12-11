# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla

import sys
import unittest

from subprocess import check_output
# 定义一个继承自unittest.TestCase的测试用例类，用于对相关功能进行单元测试
class TestClient(unittest.TestCase):
    # 定义一个测试方法，用于测试客户端版本相关的功能
    def test_client_version(self):
        # 创建一个carla.Client实例，连接到本地主机（localhost）的8080端口，
        # 这通常是为了与CARLA模拟器建立连接，后续可以通过这个实例调用相关服务和获取信息
        c = carla.Client('localhost', 8080)
        # 通过刚创建的客户端实例获取客户端版本信息，具体的版本信息格式等取决于CARLA模拟器的实现方式
        v = c.get_client_version()
        # 使用subprocess模块的check_output函数执行一个外部命令，
        # 这里执行的是'git describe --tags --dirty --always'命令，
        # 该命令常用于获取当前代码库的版本标签信息（比如最近的tag等情况），
        # 并返回命令执行的输出结果（是字节类型的数据）
        out = check_output(['git', 'describe', '--tags', '--dirty', '--always', ])
        # 判断Python的版本是否大于3.0，因为Python 3改变了字符串的处理方式，
        # 在Python 3中字节类型的数据需要进行解码才能转换为正常的字符串类型，便于后续比较等操作
        if sys.version_info > (3, 0):
            # 如果Python版本大于3.0，将获取到的字节类型的输出结果解码为utf8编码的字符串类型
            out = out.decode('utf8')
            # 使用unittest框架的断言方法，比较从CARLA客户端获取到的版本信息（v）和通过git命令获取到的版本标签信息（out）是否相等，
            # 在比较前先将两者都转换为字符串类型，并去除out字符串两端可能存在的空白字符（如空格、换行等），
            # 以此来验证客户端版本信息与代码库版本标签信息是否一致
        self.assertEqual(str(v), str(out.strip()))
