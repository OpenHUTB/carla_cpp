# 此代码片段用于更改不同类型蓝图（blueprint）角色的属性。
# 蓝图在CARLA等仿真环境中通常用于定义各类可生成对象（如行人、车辆、传感器等）的初始配置信息，通过修改蓝图属性可以定制生成对象的具体特性。

# 获取世界（world）对象中的蓝图库，并从其中筛选出名为'walker.pedestrian.0002'的行人蓝图（walker blueprint）。
# 这里的世界对象通常指代仿真场景的整体环境，包含了所有可生成的角色、物体等的蓝图信息。
walker_bp = world.get_blueprint_library().filter('walker.pedestrian.0002')
# 将筛选出的行人蓝图的'is_invincible'属性设置为True，这意味着基于该蓝图生成的行人在仿真场景中可能会具有无敌（不会受到伤害等影响）的特性，
# 具体效果取决于该属性在整个仿真系统中的定义和使用方式。
walker_bp.set_attribute('is_invincible', True)

# ...
# 从世界对象的蓝图库中筛选出所有以'vehicle.bmw.'开头的车辆蓝图，这里可能会匹配到多种不同型号的宝马（BMW）车辆蓝图。
# 注意此处代码中变量名'wolrd'应该是拼写错误，正确的应为'world'，以下按照正确变量名含义进行注释。
vehicle_bp = wolrd.get_blueprint_library().filter('vehicle.bmw.*')
# 从车辆蓝图的'color'属性的推荐值列表中随机选择一个颜色值。
# 通常蓝图的某些属性会有推荐的取值范围或者取值列表，这里获取颜色属性的推荐值列表后随机选取一个，用于后续设置车辆的颜色。
color = random.choice(vehicle_bp.get_attribute('color').recommended_values)
# 将筛选出的车辆蓝图的'color'属性设置为前面随机选取的颜色值，这样基于该蓝图生成的车辆将会呈现出所选的颜色。
vehicle_bp.set_attribute('color', color)

# ...
# 从世界对象的蓝图库中筛选出名为'sensor.camera.rgb'的摄像头蓝图，一般用于在仿真场景中生成能捕捉彩色图像的摄像头传感器。
camera_bp = world.get_blueprint_library().filter('sensor.camera.rgb')
# 将摄像头蓝图的'image_size_x'属性设置为600，该属性通常用于定义摄像头拍摄图像的水平分辨率大小，这里设置为600像素。
camera_bp.set_attribute('image_size_x', 600)
# 将摄像头蓝图的'image_size_y'属性设置为600，与'image_size_x'类似，此属性用于定义摄像头拍摄图像的垂直分辨率大小，这里同样设置为600像素，
# 使得基于该蓝图生成的摄像头拍摄的图像将具有600x600的分辨率规格。
camera_bp.set_attribute('image_size_y', 600)
# ...
