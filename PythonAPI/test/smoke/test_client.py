# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


from . import SmokeTest


class TestClient(SmokeTest):  # 定义一个名为TestClient的类，它继承自SmokeTest类。
    def test_version(self):  # 定义一个名为test_version的测试方法。
        print("TestClient.test_version")  # 打印一条消息，表明正在执行这个测试。
        
        # 调用self.assertEqual方法来断言两个版本是否相等。
        # self.assertEqual 是从 unittest.TestCase 继承而来的方法，用于比较两个值是否相等。
        # 如果不相等，这个测试方法会抛出一个异常，标志着测试失败。
        # self.client.get_client_version() 调用了一个返回客户端版本的方法。
        # self.client.get_server_version() 调用了一个返回服务器版本的方法。
        self.assertEqual(
            self.client.get_client_version(),  # 获取客户端版本
            self.client.get_server_version()   # 获取服务器版本
        )
