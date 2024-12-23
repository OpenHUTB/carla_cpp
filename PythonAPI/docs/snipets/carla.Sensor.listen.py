
import carla

# 获取CARLA世界对象，这是后续操作的基础，世界对象包含了场景中的各种元素和资源，例如地图、车辆、传感器蓝图等
world = client.get_world()

# 1. 获取语义分割相机蓝图
# 从世界的蓝图库中筛选出用于语义分割相机的蓝图
# 语义分割相机能够捕捉场景图像，并将图像中的不同物体按照语义类别进行区分，例如道路、车辆、行人等
camera_blueprint = world.get_blueprint_library().filter('sensor.camera.semantic_segmentation')[0]
# 这里假设筛选结果只有一个合适的语义分割相机蓝图，所以取[0]，如果可能有多个，需要根据实际情况选择合适的

# 2. 设置相机图像的处理与保存逻辑
# 定义颜色转换模式为CityScapesPalette
# CityScapesPalette是CARLA中预定义的一种颜色转换方式，它遵循CityScapes数据集的调色板规范
# 该规范为不同的语义类别（如建筑物、道路、植被等）分配特定的颜色，以便于直观地分辨图像中的物体类别
color_converter = carla.ColorConverter.CityScapesPalette

# 为相机设置图像捕获后的回调函数
# 当相机捕获到新的图像时，会自动调用这个回调函数进行处理
def process_and_save_image(image):
    # 将图像按照指定的颜色转换模式进行转换
    converted_image = image.convert(color_converter)
    # 定义图像保存的文件名格式，使用6位数字的帧编号作为文件名的一部分，确保文件名的顺序性和唯一性
    # 这样在保存多个图像时，能够方便地按照顺序进行查看和管理
    file_name = f'output/{image.frame:06d}.png'
    # 将转换后的图像保存到磁盘上
    converted_image.save_to_disk(file_name)

# 让相机开始监听图像事件，并在捕获图像时调用process_and_save_image函数进行处理
camera.listen(process_and_save_image)
