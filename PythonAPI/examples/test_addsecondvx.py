import glob
import os
import sys

# 尝试将Carla库对应的模块路径添加到Python的系统路径中，这里指定了Carla库的版本为0.9.15，
# 根据当前Python版本（主版本号sys.version_info.major和次版本号sys.version_info.minor）以及操作系统类型（通过os.name判断，'nt'表示Windows，'linux-x86_64'表示Linux），
# 去查找符合特定命名格式（carla-0.9.15-py*%d.%d-%s.egg）的.egg文件（Carla库的一种打包格式），使用glob.glob获取匹配的文件路径列表，取第一个路径（[0]）添加到sys.path中。
# 如果没找到匹配文件（触发IndexError异常），则直接跳过添加操作，继续往下执行代码。
try:
    sys.path.append(glob.glob('../carla/dist/carla-0.9.15-py*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla
import random
import weakref

# 定义一个函数，用于获取符合特定条件的演员（actor，在Carla中可以是车辆、传感器等各种实体）蓝图（blueprint，相当于创建实体的模板，包含实体的各种属性配置信息）。
# 参数world是Carla中的模拟世界对象，filter是用于筛选蓝图的字符串，generation用于指定演员的生成版本相关条件。
def get_actor_blueprints(world, filter, generation):
    # 首先根据传入的筛选字符串filter从世界对象的蓝图库中获取符合条件的蓝图列表。
    bps = world.get_blueprint_library().filter(filter)

    # 如果generation参数设置为"all"，表示获取所有符合filter筛选条件的蓝图，直接返回该列表。
    if generation.lower() == "all":
        return bps

    # 如果通过筛选后只得到一个蓝图，那就认为这就是所需的蓝图，忽略generation条件，直接返回这个唯一的蓝图。
    if len(bps) == 1:
        return bps

    try:
        # 尝试将generation参数转换为整数，因为后续要根据整数版本号来进一步筛选蓝图。
        int_generation = int(generation)
        # 检查转换后的版本号是否在有效的版本列表（这里假设只有1和2是有效的版本号）中。
        if int_generation in [1, 2]:
            # 从蓝图列表bps中筛选出属性中'generation'值等于指定版本号的蓝图，生成一个新的符合版本要求的蓝图列表。
            bps = [x for x in bps if int(x.get_attribute('generation')) == int_generation]
            return bps
        else:
            # 如果传入的版本号不在有效范围内，打印警告信息，表示演员生成版本无效，不会生成相应的演员，然后返回空列表。
            print("   Warning! Actor Generation is not valid. No actor will be spawned.")
            return []
    except:
        # 如果在将generation转换为整数或进行相关操作时出现异常，同样打印警告信息，不会生成演员，返回空列表。
        print("   Warning! Actor Generation is not valid. No actor will be spawned.")
        return []


# 定义一个名为V2XSensor的类，用于表示一种V2X传感器（Vehicle-to-Everything，车与外界通信的一种传感器概念）相关的功能封装。
class V2XSensor(object):
    # 类的初始化方法，接收一个parent_actor参数，表示这个传感器所属的父级演员（例如传感器附着的车辆等）。
    def __init__(self, parent_actor):
        self.sensor = None
        self._parent = parent_actor
        world = self._parent.get_world()
        # 从世界对象的蓝图库中查找名为'sensor.other.v2x'的传感器蓝图，这里原本有查找'sensor.other.v2x_custom'的代码但被注释掉了，可能根据实际需求选择了标准的'v2x'传感器蓝图。
        bp = world.get_blueprint_library().find('sensor.other.v2x')
        # 在世界中根据找到的传感器蓝图以及默认的坐标变换（carla.Transform()，即初始位置和姿态）创建传感器实例，并将其附着到父级演员上，然后将创建好的传感器对象赋值给self.sensor。
        self.sensor = world.spawn_actor(
            bp, carla.Transform(), attach_to=self._parent)
        # 为了避免循环引用问题（Python中对象之间相互引用可能导致内存无法正确回收的情况），使用weakref创建对自身的弱引用，以便在回调函数中使用。
        weak_self = weakref.ref(self)
        # 为传感器注册一个回调函数，当传感器获取到数据时会触发该回调函数。回调函数传入了弱引用weak_self以及传感器数据sensor_data，
        # 回调函数实际调用的是类中的静态方法_V2X_callback，传入弱引用和传感器数据进行后续处理。
        self.sensor.listen(
            lambda sensor_data: V2XSensor._V2X_callback(weak_self, sensor_data))

    # 定义一个方法用于销毁传感器，先停止传感器（可能是停止其数据采集等操作），然后销毁传感器实例，释放相关资源。
    def destroy(self):
        self.sensor.stop()
        self.sensor.destroy()

    # 定义一个静态方法，作为传感器数据的回调处理函数。接收弱引用weak_self（指向V2XSensor实例）和传感器数据sensor_data作为参数。
    @staticmethod
    def _V2X_callback(weak_self, sensor_data):
        # 通过弱引用获取实际的V2XSensor实例对象，如果获取不到（可能实例已经被销毁了），则直接返回，不进行后续处理。
        self = weak_self()
        if not self:
            return
        # 遍历传感器数据列表（可能传感器一次返回多个数据项）。
        for data in sensor_data:
            msg = data.get()
            # 原本有获取消息中"Header"里"Station ID"的代码但被注释掉了，可能根据实际情况不需要或者后续再处理这个信息。
            # stationId = msg["Header"]["Station ID"]
            power = data.power
            print(msg)
            # 原本有根据Station ID打印相关信息的代码被注释掉了，替换为打印接收到消息的功率信息。
            # print('Cam message received from %s ' % stationId)
            print('Cam message received with power %f ' % power)


# 创建一个Carla客户端对象，连接到本地主机（"localhost"）的2000端口，这是与Carla服务器进行通信的入口点。
client = carla.Client("localhost", 2000)
# 设置客户端的超时时间为2000.0秒，用于控制客户端向服务器发送请求后等待响应的最长时间，避免长时间无响应等待。
client.set_timeout(2000.0)

# 通过客户端获取Carla模拟世界（world）对象，这个世界对象包含了模拟场景中的各种实体、设置等信息，后续操作大多基于这个世界对象展开。
world = client.get_world()
# 从世界对象中获取地图（map）对象，用于后续获取地图相关信息，比如生成点等。
smap = world.get_map()
# 以下两行代码有一行被注释掉了，原本可能是想获取某个特定ID（这里是28）的演员并向其发送"test"消息，但目前未执行该操作，可能后续根据需求再启用。
# acl = world.get_actor(28)
# acl.send("test")

# 从地图对象中获取所有的生成点（spawn_points，即可以生成演员的位置点）列表，然后随机选择一个作为初始生成点，
# 如果没有获取到生成点列表（可能地图没有合适的生成点），则使用默认的坐标变换（carla.Transform()）作为生成点。
spawn_points = smap.get_spawn_points()
spawn_point = random.choice(spawn_points) if spawn_points else carla.Transform()
# 通过调用前面定义的get_actor_blueprints函数，从世界的蓝图库中筛选出符合"vehicle.*"（表示所有车辆相关蓝图）且版本为"2"的蓝图，然后随机选择一个蓝图作为要生成的车辆蓝图。
blueprint = random.choice(get_actor_blueprints(world, "vehicle.*", "2"))
# 设置车辆蓝图的'role_name'属性为"test"，可能用于在模拟场景中标识该车辆的角色等用途。
blueprint.set_attribute('role_name', "test")
# 尝试在世界中根据选择的车辆蓝图和生成点来生成车辆演员，如果生成成功则返回创建好的车辆对象，否则返回None（这里假设成功生成了车辆）。
player = world.try_spawn_actor(blueprint, spawn_point)
# 使用创建好的车辆对象作为父级演员，创建一个V2XSensor实例，即给车辆附着一个V2X传感器，用于后续获取相关传感器数据。
v2x_sensor = V2XSensor(player)

# 让世界对象等待一次时间推进（tick），确保世界状态更新等操作完成，这一步可能是为了让初始状态准备好，比如车辆、传感器等都处于合适的初始状态。
world.wait_for_tick()
try:
    # 进入一个无限循环，只要程序不被中断，就会持续让世界对象等待时间推进，这样模拟世界会不断运行，传感器也会持续获取数据并触发回调函数进行处理。
    while True:
        world.wait_for_tick()
finally:
    # 无论在循环过程中是否出现异常等情况，最终都要销毁创建的V2X传感器实例和车辆实例，释放相关资源，避免内存泄漏等问题，保持模拟世界的整洁。
    v2x_sensor.destroy()
    player.destroy()
