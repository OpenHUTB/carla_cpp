import glob
import os
import sys
# 尝试将Carla库的路径添加到系统路径中，通过查找符合特定格式的Carla库文件（.egg格式）来实现
# 根据当前Python版本（主版本号和次版本号）以及操作系统类型（Windows是'win-amd64'，Linux是'linux-x86_64'）来确定具体文件名
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass


# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

# 导入Carla库，后续将使用其中的类和函数来与Carla模拟器进行交互
import carla
# 创建一个Carla客户端对象，连接到本地运行的Carla服务器，默认端口号为2000
# 'localhost'表示本地主机，也就是当前运行代码的这台机器
client = carla.Client('localhost', 2000)
# 通过客户端对象获取Carla世界对象，这个世界对象代表了整个模拟场景，包含了地图、车辆、行人等各种元素
world = client.get_world()
# 创建一个Carla中的位置对象（Location），用于指定在三维空间中的坐标位置
# 这里坐标设置为 (200.0, 200.0, 200.0)，分别对应 x、y、z 轴的坐标值
location = carla.Location(200.0, 200.0, 200.0)
# 创建一个Carla中的旋转对象（Rotation），用于指定物体在三维空间中的旋转角度
# 这里角度都初始化为0.0，表示没有旋转，三个参数分别对应绕 yaw（偏航）、pitch（俯仰）、roll（翻滚）轴的旋转角度
rotation = carla.Rotation(0.0, 0.0, 0.0)
# 创建一个Carla中的变换对象（Transform），它结合了位置（Location）和旋转（Rotation）信息
# 用于描述物体在世界中的姿态（位置和朝向），这里将前面创建的位置和旋转对象传入构造
transform = carla.Transform(location, rotation)
# 获取世界对象中的蓝图库（Blueprint Library），蓝图库包含了可以在模拟场景中生成的各种对象的蓝图（类似于模板）
bp_library = world.get_blueprint_library()
# 从蓝图库中查找名为'vehicle.audi.tt'的车辆蓝图，这个蓝图定义了要生成的奥迪TT车辆的各种属性和配置
bp_audi = bp_library.find('vehicle.audi.tt')
 #在世界中根据找到的奥迪TT车辆蓝图以及指定的变换（位置和朝向）信息，生成一个实际的车辆演员（Actor）对象
# 也就是在模拟场景中创建出一辆处于特定位置和朝向的奥迪TT车辆
audi = world.spawn_actor(bp_audi, transform)
# 获取刚刚生成的奥迪TT车辆（audi）上名为'front-blinker-r-1'这个组件在世界坐标系下的变换信息
# 这个变换信息包含了该组件在世界中的位置和朝向等信息，可用于了解该组件相对于世界或车辆的姿态
component_transform = audi.get_component_world_transform('front-blinker-r-1')
# 打印出获取到的组件的世界变换信息，方便查看其在模拟场景中的具体位置和姿态情况
print(component_transform)

