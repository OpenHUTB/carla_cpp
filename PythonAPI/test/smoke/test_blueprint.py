# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
import re

from. import SmokeTest


class TestBlueprintLibrary(SmokeTest):
    def test_blueprint_ids(self):
        """
        这个方法用于测试蓝图库（Blueprint Library）相关的功能，主要是对蓝图的一些筛选以及蓝图ID格式的验证。
        """
        print("TestBlueprintLibrary.test_blueprint_ids")
        # 获取世界对象中的蓝图库
        library = self.client.get_world().get_blueprint_library()
        # 验证蓝图库中存在元素，即至少有一个蓝图
        self.assertTrue([x for x in library])
        # 验证通过'sensor.*'过滤后能得到蓝图元素，意味着库中存在符合该过滤规则的蓝图
        self.assertTrue([x for x in library.filter('sensor.*')])
        # 验证通过'static.*'过滤后能得到蓝图元素，同理检查是否有符合此规则的蓝图
        self.assertTrue([x for x in library.filter('static.*')])
        # 验证通过'vehicle.*'过滤后能得到蓝图元素，检查是否有以'vehicle'开头相关规则的蓝图
        self.assertTrue([x for x in library.filter('vehicle.*')])
        # 验证通过'walker.*'过滤后能得到蓝图元素，查看是否有符合该规则的蓝图
        self.assertTrue([x for x in library.filter('walker.*')])

        # 编译一个正则表达式，用于匹配类似 "xxx.xxx.xxx" 这样格式的字符串（此处用于匹配蓝图ID的格式）
        rgx = re.compile(r'\S+\.\S+\.\S+')
        # 遍历蓝图库中的每个蓝图，验证其ID是否符合上述正则表达式的格式要求
        for bp in library:
            self.assertTrue(rgx.match(bp.id))

        # 编译一个专门针对以'vehicle'开头的蓝图ID格式的正则表达式，格式类似 "vehicle.xxx.xxx"
        rgx = re.compile(r'(vehicle)\.\S+\.\S+')
        # 遍历通过'vehicle.*'过滤后的每个蓝图，验证其ID是否符合这个针对车辆蓝图的正则表达式格式要求
        for bp in library.filter('vehicle.*'):
            self.assertTrue(rgx.match(bp.id))
