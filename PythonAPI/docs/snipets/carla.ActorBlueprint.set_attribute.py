# This recipe changes attributes of different type of blueprint actors.

# 获取名为 'walker.pedestrian.0002' 的蓝图，并将其存储在 walker_bp 变量中
walker_bp = world.get_blueprint_library().filter('walker.pedestrian.0002')
# 将 walker_bp 的 'is_invincible' 属性设置为 True
walker_bp.set_attribute('is_invincible', True)


#...
# 以下代码存在错误，将 'wolrd' 改为 'world'
# 获取以 'vehicle.bmw.*' 开头的蓝图，并将其存储在 vehicle_bp 变量中
vehicle_bp = world.get_blueprint_library().filter('vehicle.bmw.*')
# 从 vehicle_bp 的 'color' 属性的推荐值中随机选择一个颜色
color = random.choice(vehicle_bp.get_attribute('color').recommended_values)
# 将 vehicle_bp 的 'color' 属性设置为随机选择的颜色
vehicle_bp.set_attribute('color', color)


#...
# 获取名为 'sensor.camera.rgb' 的蓝图，并将其存储在 camera_bp 变量中
camera_bp = world.get_blueprint_library().filter('sensor.camera.rgb')
# 将 camera_bp 的 'image_size_x' 属性设置为 600
camera_bp.set_attribute('image_size_x', 600)
# 将 camera_bp 的 'image_size_y' 属性设置为 600
camera_bp.set_attribute('image_size_y', 600)
#...
