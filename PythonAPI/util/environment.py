#!/usr/bin/env python

"""
Script to control weather parameters in simulations
"""

import glob
import os
import sys
import argparse
# 尝试将特定路径下符合条件的 Carla 库文件路径添加到系统路径中，以便后续能正确导入 carla 库
# 根据当前 Python 版本（主版本号和次版本号）以及操作系统类型（Windows 或 Linux）来构建路径通配符模式
# 查找符合模式的.egg 文件，并将其路径添加到系统路径 sys.path 中，这样 Python 就能找到并导入 carla 库了
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass
# 导入 carla 库，用于与 Carla 模拟器进行交互，比如获取世界场景、车辆、天气等相关对象及操作它们
import carla
# 定义一个字典，用于存储不同日照预设情况对应的太阳高度角和方位角数值
# 'day'、'night'、'sunset' 是预设的场景名称，对应的元组中第一个元素是太阳高度角，第二个元素是太阳方位角
SUN_PRESETS = {
    'day': (45.0, 0.0),
    'night': (-90.0, 0.0),
    'sunset': (0.5, 0.0)}
# 定义一个字典，用于存储不同天气预设情况对应的各种天气参数数值
# 每个列表中的元素依次对应 cloudiness（云量）、precipitation（降水量）等不同天气参数的值
# 键名如 'clear'、'overcast'、'rain' 代表不同的天气状况预设
WEATHER_PRESETS = {
    'clear': [10.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0331, 0.0],
    'overcast': [80.0, 0.0, 0.0, 50.0, 2.0, 0.75, 0.1, 10.0, 0.0, 0.03, 0.0331, 0.0],
    'rain': [100.0, 80.0, 90.0, 100.0, 7.0, 0.75, 0.1, 100.0, 0.0, 0.03, 0.0331, 0.0]}
# 定义一个字典，用于表示车辆灯光的不同状态选项
# 键名是不同灯光状态的描述，值是对应的 carla.VehicleLightState 枚举值组成的列表（这里每个键对应只有一个枚举值）
# 例如 'None' 表示车辆灯光全部关闭状态，对应的枚举值是 carla.VehicleLightState.NONE
CAR_LIGHTS = {
    'None' : [carla.VehicleLightState.NONE],
    'Position' : [carla.VehicleLightState.Position],
    'LowBeam' : [carla.VehicleLightState.LowBeam],
    'HighBeam' : [carla.VehicleLightState.HighBeam],
    'Brake' : [carla.VehicleLightState.Brake],
    'RightBlinker' : [carla.VehicleLightState.RightBlinker],
    'LeftBlinker' : [carla.VehicleLightState.LeftBlinker],
    'Reverse' : [carla.VehicleLightState.Reverse],
    'Fog' : [carla.VehicleLightState.Fog],
    'Interior' : [carla.VehicleLightState.Interior],
    'Special1' : [carla.VehicleLightState.Special1],
    'Special2' : [carla.VehicleLightState.Special2],
    'All' : [carla.VehicleLightState.All]}
# 定义一个字典，用于表示灯光组的不同选项
# 键名是不同灯光组的描述，值是对应的 carla.LightGroup 枚举值组成的列表（这里每个键对应只有一个枚举值）
# 例如 'Street' 对应 carla.LightGroup.Street，表示街道灯光组等
LIGHT_GROUP = {
    'None' : [carla.LightGroup.NONE],
    # 'Vehicle' : [carla.LightGroup.Vehicle],
    'Street' : [carla.LightGroup.Street],
    'Building' : [carla.LightGroup.Building],
    'Other' : [carla.LightGroup.Other]}
# 函数功能：根据传入的命令行参数 args 和天气对象 weather，应用日照预设来设置太阳位置
# 如果命令行中指定的日照预设（通过 args.sun 获取）在 SUN_PRESETS 字典中有定义，则将对应的太阳高度角和方位角赋值给天气对象的相应属性
# 如果指定的预设不存在，则打印错误信息并退出程序
def apply_sun_presets(args, weather):
    """Uses sun presets to set the sun position"""
    if args.sun is not None:
        if args.sun in SUN_PRESETS:
            weather.sun_altitude_angle = SUN_PRESETS[args.sun][0]
            weather.sun_azimuth_angle = SUN_PRESETS[args.sun][1]
        else:
            print("[ERROR]: Command [--sun | -s] '" + args.sun + "' not known")
            sys.exit(1)
# 函数功能：根据传入的命令行参数 args 和天气对象 weather，应用天气预设来设置各种天气参数
# 如果命令行中指定的天气预设（通过 args.weather 获取）在 WEATHER_PRESETS 字典中有定义，则依次将列表中对应的各个天气参数值赋值给天气对象的相应属性
# 如果指定的预设不存在，则打印错误信息并退出程序
def apply_weather_presets(args, weather):
    """Uses weather presets to set the weather parameters"""
    if args.weather is not None:
        if args.weather in WEATHER_PRESETS:
            weather.cloudiness = WEATHER_PRESETS[args.weather][0]
            weather.precipitation = WEATHER_PRESETS[args.weather][1]
            weather.precipitation_deposits = WEATHER_PRESETS[args.weather][2]
            weather.wind_intensity = WEATHER_PRESETS[args.weather][3]
            weather.fog_density = WEATHER_PRESETS[args.weather][4]
            weather.fog_distance = WEATHER_PRESETS[args.weather][5]
            weather.fog_falloff = WEATHER_PRESETS[args.weather][6]
            weather.wetness = WEATHER_PRESETS[args.weather][7]
            weather.scattering_intensity = WEATHER_PRESETS[args.weather][8]
            weather.mie_scattering_scale = WEATHER_PRESETS[args.weather][9]
            weather.rayleigh_scattering_scale = WEATHER_PRESETS[args.weather][10]
            weather.dust_storm = WEATHER_PRESETS[args.weather][11]
        else:
            print("[ERROR]: Command [--weather | -w] '" + args.weather + "' not known")
            sys.exit(1)

# 函数功能：根据传入的命令行参数 args 和天气对象 weather，逐个设置天气参数的值
# 检查每个对应的命令行参数是否有值（不为 None），如果有值则将其赋值给天气对象的相应属性，实现对天气各参数的单独设置
def apply_weather_values(args, weather):
    """Set weather values individually"""
    if args.azimuth is not None:
        weather.sun_azimuth_angle = args.azimuth
    if args.altitude is not None:
        weather.sun_altitude_angle = args.altitude
    if args.clouds is not None:
        weather.cloudiness = args.clouds
    if args.rain is not None:
        weather.precipitation = args.rain
    if args.puddles is not None:
        weather.precipitation_deposits = args.puddles
    if args.wind is not None:
        weather.wind_intensity = args.wind
    if args.fog is not None:
        weather.fog_density = args.fog
    if args.fogdist is not None:
        weather.fog_distance = args.fogdist
    if args.fogfalloff is not None:
        weather.fog_falloff = args.fogfalloff
    if args.wetness is not None:
        weather.wetness = args.wetness
    if args.scatteringintensity is not None:
        weather.scattering_intensity = args.scatteringintensity
    if args.miescatteringscale is not None:
        weather.mie_scattering_scale = args.miescatteringscale
    if args.rayleighscatteringscale is not None:
        weather.rayleigh_scattering_scale = args.rayleighscatteringscale
    if args.dust_storm is not None:
        weather.dust_storm = args.dust_storm

# 函数功能：根据传入的命令行参数 args 和世界对象 world，将指定的车辆灯光状态应用到世界中的所有车辆上
# 如果命令行参数 args.cars 为 None（即没有指定车辆灯光相关设置），则直接返回不做操作
# 否则，通过循环遍历 args.cars 中的选项，将对应的车辆灯光状态掩码进行或运算合并，然后获取世界中的所有车辆，为符合条件的车辆设置相应的灯光状态
def apply_lights_to_cars(args, world):
    if args.cars is None:
        return

    light_mask = carla.VehicleLightState.NONE
    for option in args.cars:
        light_mask |= CAR_LIGHTS[option][0]
        
    # Get all cars in level
    all_vehicles = world.get_actors()
    for ve in all_vehicles:
        if "vehicle." in ve.type_id:
            ve.set_light_state(carla.VehicleLightState(light_mask))
# 函数功能：根据传入的命令行参数 args 和灯光管理器对象 light_manager，对相应灯光组的灯光进行操作
# 如果命令行参数 args.lights 为 None（即没有指定灯光相关操作），则直接返回不做操作
# 首先确定要操作的灯光组（根据 args.lightgroup 获取，默认为 'None'），然后获取该灯光组下的所有灯光
# 接着循环遍历 args.lights 中的操作选项，根据不同选项（如 'on'、'off'、'intensity'、'color'）执行相应的灯光操作，如打开、关闭、设置强度、设置颜色等
def apply_lights_manager(args, light_manager):
    if args.lights is None:
        return
  # filter by group
    light_group = 'None'
    if args.lightgroup is not None:
        light_group = args.lightgroup

    # filter by group
    lights = light_manager.get_all_lights(LIGHT_GROUP[light_group][0]) # light_group

    i = 0
    while (i < len(args.lights)):
        option = args.lights[i]

        if option == "on":
            light_manager.turn_on(lights)
        elif option == "off":
            light_manager.turn_off(lights)
        elif option == "intensity":
            light_manager.set_intensity(lights, int(args.lights[i + 1]))
            i += 1
        elif option == "color":
            r = int(args.lights[i + 1])
            g = int(args.lights[i + 2])
            b = int(args.lights[i + 3])
            light_manager.set_color(lights, carla.Color(r, g, b))
            i += 3

        i += 1


def main():
    """Start function"""
     # 创建一个命令行参数解析器对象，用于解析用户在命令行输入的参数
    # description 参数用于设置在命令行帮助信息中显示的描述内容（这里使用了文档字符串 __doc__，通常是模块开头的说明信息）
    argparser = argparse.ArgumentParser(
        description=__doc__)
     # 添加命令行参数 '--host'，用于指定主机服务器的 IP 地址，默认值为 '127.0.0.1'，并提供相应的帮助信息
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
      # 添加命令行参数 '-p' 或 '--port'，用于指定要监听的 TCP 端口号，默认值为 2000，类型为整数，并提供相应的帮助信息
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
     # 添加命令行参数 '--sun'，用于指定日照预设情况，类型为字符串，默认值为 None，并提供可选择的预设值列表作为帮助信息
    argparser.add_argument(
        '--sun',
        default=None,
        type=str,
        help='Sun position presets [' + ' | '.join([i for i in SUN_PRESETS]) + ']')
     # 添加命令行参数 '--weather'，用于指定天气预设情况，类型为字符串，默认值为 None，并提供可选择的预设值列表作为帮助信息
    argparser.add_argument(
        '--weather',
        default=None,
        type=str,
        help='Weather condition presets [' + ' | '.join([i for i in WEATHER_PRESETS]) + ']')
     # 添加命令行参数 '--altitude' 或 '-alt'，用于指定太阳高度角，类型为浮点数，默认值为 None，并提供相应的取值范围帮助信息
    argparser.add_argument(
        '--altitude', '-alt',
        metavar='A',
        default=None,
        type=float,
        help='Sun altitude [-90.0, 90.0]')
     # 添加命令行参数 '--azimuth' 或 '-azm'，用于指定太阳方位角，类型为浮点数，默认值为 None，并提供相应的取值范围帮助信息
    argparser.add_argument(
        '--azimuth', '-azm',
        metavar='A',
        default=None,
        type=float,
        help='Sun azimuth [0.0, 360.0]')
     # 添加命令行参数 '--clouds' 或 '-c'，用于指定云量，类型为浮点数，默认值为 None，并提供相应的取值范围帮助信息
    argparser.add_argument(
        '--clouds', '-c',
        metavar='C',
        default=None,
        type=float,
        help='Clouds amount [0.0, 100.0]')
      # 添加命令行参数 '--rain' 或 '-r'，用于指定降水量，类型为浮点数，默认值为 None，并提供相应的取值范围帮助信息
    argparser.add_argument(
        '--rain', '-r',
        metavar='R',
        default=None,
        type=float,
        help='Rain amount [0.0, 100.0]')
     # 添加命令行参数 '--puddles' 或 '-pd'，用于指定积水量，类型为浮点数，默认值为 None，并提供相应的取值范围帮助信息
    argparser.add_argument(
        '--puddles', '-pd',
        metavar='Pd',
        default=None,
        type=float,
        help='Puddles amount [0.0, 100.0]')
     # 添加命令行参数 '--wind' 或 '-w'，用于指定风强度，类型为浮点数，默认值为 None，并提供相应的取值范围帮助信息
    argparser.add_argument(
        '--wind', '-w',
        metavar='W',
        default=None,
        type=float,
        help='Wind intensity [0.0, 100.0]')
     # 添加命令行参数 '--fog' 或 '-f'，用于指定雾强度，类型为浮点数，默认值为 None，并提供相应的取值范围帮助信息
    argparser.add_argument(
        '--fog', '-f',
        metavar='F',
        default=None,
        type=float,
        help='Fog intensity [0.0, 100.0]')
    # 添加命令行参数 '--fogdist' 或 '-fd'，用于指定雾距离，类型为浮点数，默认值为 None，并提供相应的取值范围帮助信息
    argparser.add_argument(
        '--fogdist', '-fd',
        metavar='Fd',
        default=None,
        type=float,
        help='Fog Distance [0.0, 100.0)')
      # 添加命令行参数 '--fogfalloff' 或 '-fo'，用于指定雾消散程度，类型为浮点数，默认值为 None，并提供相应的取值范围帮助信息
    argparser.add_argument(
        '--fogfalloff', '-fo',
        metavar='Fo',
        default=None,
        type=float,
        help='Fog Falloff [0.0, inf)')
    # 添加命令行参数 '--wetness' 或 '-wet'，用于指定湿度强度
# 参数的元数据（在帮助信息等场景展示的相关标识）用 'Wet' 表示，默认值为 None，类型设定为浮点数
# 帮助信息中说明了该参数取值范围是 [0.0, 100.0]，即湿度强度可在此区间内设定
    argparser.add_argument(
        '--wetness', '-wet',
        metavar='Wet',
        default=None,
        type=float,
        help='Wetness intensity [0.0, 100.0]')
    # 添加命令行参数 '--scatteringintensity' 或 '-si'，用于指定散射强度
# 元数据用'si' 表示，默认值为 None，类型为浮点数
# 帮助信息指出其取值范围是 [0.0, +∞)，意味着可以设置大于等于0的任意数值来表示散射强度大小
    argparser.add_argument(
        '--scatteringintensity', '-si',
        metavar='si',
        default=None,
        type=float,
        help='Scattering intensity [0.0, inf]')
    # 添加命令行参数 '--rayleighscatteringscale' 或 '-rss'，用于指定瑞利散射尺度
# 元数据为 'rss'，默认值是 None，类型为浮点数
# 帮助信息提示该参数的取值范围限定在 [0.0, 2.0] 区间内
    argparser.add_argument(
        '--rayleighscatteringscale', '-rss',
        metavar='rss',
        default=None,
        type=float,
        help='Rayleigh scattering scale [0.0, 2.0]')
    # 添加命令行参数 '--miescatteringscale' 或 '-mss'，用于指定米氏散射尺度
# 元数据为'mss'，默认值为 None，类型为浮点数
# 帮助信息表明其取值范围是 [0.0, 5.0]
    argparser.add_argument(
        '--miescatteringscale', '-mss',
        metavar='mss',
        default=None,
        type=float,
        help='Mie scattering scale [0.0, 5.0]')
    # 添加命令行参数 '--dust_storm' 或 '-ds'，用于指定沙尘风暴强度
# 元数据是 'ds'，默认值为 None，类型为浮点数
# 帮助信息显示其取值范围为 [0.0, 100.0]，即用来衡量沙尘风暴强度大小的可设范围
    argparser.add_argument(
        '--dust_storm', '-ds',
        metavar='ds',
        default=None,
        type=float,
        help='Dust storm strength [0.0, 100.0]')
    # 添加命令行参数 '--cars'，用于指定车辆灯光相关设置
# 元数据为 'Cars'，默认值为 None，类型为字符串，且可以接收多个字符串参数（通过 nargs='+' 设定）
# 帮助信息展示了可选的车辆灯光状态选项，这些选项来自于 CAR_LIGHTS 字典中的键，用于对车辆灯光进行不同的设置组合
    argparser.add_argument(
        '--cars',
        metavar='Cars',
        default=None,
        type=str,
        nargs='+',
        help='Light Cars [' + ' | '.join([i for i in CAR_LIGHTS]) + ']')
    # 添加命令行参数 '--lights' 或 '-l'，用于对灯光进行相关操作设置（可能是控制开启、关闭、调整亮度等）
# 元数据为 'Lights'，默认值为 None，类型为字符串，同样可以接收多个字符串参数（nargs='+'）
# 这里帮助信息中的内容为空列表 []，可能后续需要根据具体业务逻辑补充完整相关可选项说明
    argparser.add_argument(
        '--lights', '-l',
        metavar='Lights',
        default=None,
        type=str,
        nargs='+',
        help='Street Lights []')
    # 添加命令行参数 '--lightgroup' 或 '-lg'，用于指定灯光组
# 元数据为 'LightGroup'，默认值为 None，类型为字符串
# 帮助信息展示了可选的灯光组选项，这些选项来自 LIGHT_GROUP 字典中的键，用于区分不同类型的灯光组，以便进行针对性操作
    argparser.add_argument(
        '--lightgroup', '-lg',
        metavar='LightGroup',
        default=None,
        type=str,
        help='Light Group [' + ' | '.join([i for i in LIGHT_GROUP]) + ']')
    # 解析命令行参数，将解析结果保存在 args 变量中，后续可通过 args 的属性来访问各个命令行参数的值
    args = argparser.parse_args()

    # since all the arguments are None by default
    # (except for the first 2, host and port)
    # we can check if all the arguments have been provided
    arg_values = [v for _, v in args.__dict__.items()][2:]
    if all(i is (None and False) for i in arg_values):
        argparser.error('No arguments provided.')
# 创建一个 carla.Client 对象，用于与 Carla 模拟器建立连接
# 使用命令行参数中指定的主机地址（args.host）和端口号（args.port）进行连接初始化
    client = carla.Client(args.host, args.port)
    client.set_timeout(2.0)
    world = client.get_world()

    weather = world.get_weather()

    # apply presets
    apply_sun_presets(args, weather)
    apply_weather_presets(args, weather)

    # apply weather values individually
    apply_weather_values(args, weather)

    world.set_weather(weather)

    # apply car light changes
    apply_lights_to_cars(args, world)

    apply_lights_manager(args, world.get_lightmanager())

    world.wait_for_tick()


if __name__ == '__main__':
    main()
