# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import re
from . import SmokeTest

class TestBlueprintLibrary(SmokeTest):
    """测试CARLA世界中蓝图库的功能类。
    
    主要测试：
    1. 蓝图库是否包含内容
    2. 不同类型蓝图的存在性
    3. 蓝图ID的格式是否符合规范
    """

    def test_blueprint_ids(self):
        """测试蓝图ID的格式和内容的有效性。"""
        
        print("TestBlueprintLibrary.test_blueprint_ids")
        
        # 获取并验证蓝图库
        library = self.client.get_world().get_blueprint_library()
        
        # 测试蓝图库的基本内容
        self._test_library_contents(library)
        
        # 测试蓝图ID的格式
        self._test_blueprint_id_format(library)
        
        # 测试车辆蓝图的特定格式
        self._test_vehicle_blueprint_format(library)

    def _test_library_contents(self, library):
        """测试蓝图库是否包含各类必要的蓝图。"""
        
        # 验证蓝图库非空
        self.assertTrue(
            [x for x in library], 
            "蓝图库为空"
        )
        
        # 验证各类型蓝图的存在性
        blueprint_types = ['sensor.*', 'static.*', 'vehicle.*', 'walker.*']
        for bp_type in blueprint_types:
            self.assertTrue(
                [x for x in library.filter(bp_type)],
                f"未找到{bp_type}类型的蓝图"
            )

    def _test_blueprint_id_format(self, library):
        """测试所有蓝图ID是否符合'category.type.specific'格式。"""
        
        # 创建通用蓝图ID格式的正则表达式
        id_pattern = re.compile(r'\S+\.\S+\.\S+')
        
        # 验证每个蓝图的ID格式
        for blueprint in library:
            self.assertTrue(
                id_pattern.match(blueprint.id),
                f"蓝图ID '{blueprint.id}' 格式不正确"
            )

    def _test_vehicle_blueprint_format(self, library):
        """测试车辆蓝图ID是否符合'vehicle.type.specific'格式。"""
        
        # 创建车辆蓝图ID格式的正则表达式
        vehicle_pattern = re.compile(r'(vehicle)\.\S+\.\S+')
        
        # 验证每个车辆蓝图的ID格式
        for vehicle_bp in library.filter('vehicle.*'):
            self.assertTrue(
                vehicle_pattern.match(vehicle_bp.id),
                f"车辆蓝图ID '{vehicle_bp.id}' 格式不正确"
            )
