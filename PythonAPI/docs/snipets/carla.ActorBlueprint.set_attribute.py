
#此代码片段用于更改不同类型蓝图（blueprint）角色的属性。

#获取名为'walker.pedestrian.0002'的行人蓝图对象
walker_bp = world.get_blueprint_library().filter('walker.pedestrian.0002')
# 将行人蓝图对象的'is_invincible'（无敌）属性设置为True
walker_bp.set_attribute('is_invincible', True)

# 从世界（world）的蓝图库中筛选出名称匹配'vehicle.bmw.*'的车辆蓝图对象
# Changes attribute randomly by the recommended value
vehicle_bp = wolrd.get_blueprint_library().filter('vehicle.bmw.*')
# 从车辆蓝图对象的'color'（颜色）属性的推荐值列表中随机选择一个颜色值
# 先获取'color'属性的推荐值列表，然后通过random.choice随机从中选一个
color = random.choice(vehicle_bp.get_attribute('color').recommended_values)
# 将选中的颜色值设置为车辆蓝图对象的'color'属性
vehicle_bp.set_attribute('color', color)

# ...
# 从世界（world）的蓝图库中筛选出名为'sensor.camera.rgb'的相机蓝图对象
camera_bp = world.get_blueprint_library().filter('sensor.camera.rgb')
# 将相机蓝图对象的'image_size_x'（图像水平尺寸）属性设置为600像素，用于定义相机拍摄图像的水平分辨率大小
camera_bp.set_attribute('image_size_x', 600)
# 将相机蓝图对象的'image_size_y'（图像垂直尺寸）属性设置为600像素，用于定义相机拍摄图像的垂直分辨率大小
camera_bp.set_attribute('image_size_y', 600)
# ...
