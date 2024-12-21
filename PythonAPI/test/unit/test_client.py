# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla  # 导入CARLA模块，CARLA是一个开源的自动驾驶模拟器，通过该模块可以与CARLA模拟器进行交互，例如创建模拟场景、控制车辆、获取模拟器内各种对象及环境信息等操作
import sys  # 导入系统模块，该模块提供了一些与Python解释器和运行环境相关的变量及函数，在这里主要用于获取Python的版本信息，后续会根据Python版本来处理数据格式相关问题
import unittest  # 导入unittest模块，它是Python内置的标准测试框架，用于编写和组织单元测试用例，方便对代码中的各个功能模块进行单独的测试，确保功能的正确性和稳定性
from subprocess import check_output
# 从subprocess模块中导入check_output函数，subprocess模块用于在Python程序中启动新进程并与其进行通信，check_output函数可执行指定的外部命令，并返回命令执行后的输出结果（以字节流形式返回）

# 定义一个名为TestClient的类，它继承自unittest.TestCase类，按照unittest框架的规则，此类中的方法可以作为独立的测试用例来对相关功能进行单元测试
class TestClient(unittest.TestCase):
    # 定义一个名为test_client_version的方法，按照unittest框架的命名规范，以'test_'开头的方法会被识别为测试用例方法，该方法主要用于测试客户端版本相关的功能是否符合预期
    def test_client_version(self):
        # 创建一个carla.Client实例，尝试连接到本地主机（'localhost'表示本地回环地址，也就是当前运行代码的这台计算机自身）的8080端口。
        # 在CARLA模拟器的常规配置中，模拟器通常会在本地的8080端口监听来自客户端的连接请求，通过这个客户端实例，后续就能向模拟器发送请求、调用模拟器提供的各种服务以及获取相关信息了
        c = carla.Client('localhost', 8080)
        # 调用刚创建的carla.Client实例的get_client_version方法，来获取客户端版本信息。
        # 注意，这里获取到的版本信息的具体格式、内容等完全取决于CARLA模拟器内部是如何实现版本信息的记录和返回的，不同版本的CARLA模拟器可能会返回不同形式的版本相关数据
        v = c.get_client_version()
        # 使用subprocess模块的check_output函数执行一个外部命令，此处执行的命令是'git describe --tags --dirty --always'。
        # 这个git命令常用于获取当前代码库的版本标签相关信息，比如可以获取到最近一次打标签（tag）的情况，以及代码库是否处于“脏”（dirty，表示有未提交的修改）状态等信息。
        # 该命令执行后会返回相应的输出结果，这个结果是以字节类型（bytes）的数据形式呈现的
        out = check_output(['git', 'describe', '--tags', '--dirty', '--always', ])
        # 通过sys.version_info获取当前Python的版本信息，并判断其是否大于(3, 0)，也就是判断当前Python版本是否是Python 3及以上版本。
        # 这是因为Python 3对字符串的处理方式相较于Python 2有了较大改变，在Python 3中，字节类型（bytes）的数据不能直接像Python 2那样当作普通字符串使用，需要进行解码操作才能转换为正常的字符串类型，以便后续进行诸如比较等操作
        if sys.version_info > (3, 0):
            # 如果当前Python版本大于3.0，就使用decode方法将获取到的字节类型的输出结果（out）按照'utf8'编码格式进行解码，将其转换为Python 3中通用的字符串类型，这样后续才能与其他字符串类型的数据进行比较等操作
            out = out.decode('utf8')
            # 使用unittest框架提供的assertEqual断言方法，对从CARLA客户端获取到的版本信息（v）和经过处理后的通过git命令获取到的版本标签信息（out）进行比较，判断它们是否相等。
            # 在进行比较之前，先将两者都转换为字符串类型（确保数据类型一致以便比较），并且使用strip方法去除out字符串两端可能存在的空白字符（比如空格、换行符等），
            # 通过这样的比较操作，可以验证客户端所报告的版本信息与代码库的版本标签信息是否一致，以此来检测相关版本功能是否正确
        self.assertEqual(str(v), str(out.strip()))

