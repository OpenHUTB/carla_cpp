#!/usr/bin/env python

"""
Script to control weather parameters in simulations
This script interfaces with the Carla simulator, allowing the user to modify environmental conditions such as weather, lighting, and sun position. 
It offers functionality to apply presets or custom values to various simulation parameters like cloudiness, precipitation, sun altitude, wind intensity, and vehicle light states.
"""

import glob
import os
import sys
import argparse
# 尝试将Carla库文件路径添加到系统路径中，以便后续能正确导入carla库
# 根据当前Python版本（主版本号和次版本号）以及操作系统类型（Windows或Linux）来构建路径通配符模式
# 查找符合模式的.egg文件，并将其路径添加到系统路径sys.path中，这样Python就能找到并导入carla库了
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass
# 导入carla库，用于与Carla模拟器进行交互，比如获取世界场景、车辆、天气等相关对象及操作它们
import carla

# 定义一个字典，用于存储不同日照预设情况对应的太阳高度角和方位角数值
# 'day'、'night'、'sunset'是预设的场景名称，对应的元组中第一个元素是太阳高度角，第二个元素是太阳方位角
SUN_PRESETS = {
    'day': (45.0, 0.0),  # 白天：太阳高度角45.0°，太阳方位角0.0°
    'night': (-90.0, 0.0),  # 夜晚：太阳高度角 -90.0°，太阳方位角0.0°（即太阳不可见）
    'sunset': (0.5, 0.0)}  # 日落：太阳高度角0.5°，太阳方位角0.0°

# 定义一个字典，用于存储不同天气预设情况对应的各种天气参数数值
# 每个列表中的元素依次对应cloudiness（云量）、precipitation（降水量）等不同天气参数的值
WEATHER_PRESETS = {
    'clear': [10.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0331, 0.0],
    'overcast': [80.0, 0.0, 0.0, 50.0, 2.0, 0.75, 0.1, 10.0, 0.0, 0.03, 0.0331, 0.0],
    'rain': [100.0, 80.0, 90.0, 100.0, 7.0, 0.75, 0.1, 100.0, 0.0, 0.03, 0.0331, 0.0]}

# 定义一个字典，用于表示车辆灯光的不同状态选项
# 键名是不同灯光状态的描述，值是对应的carla.VehicleLightState枚举值组成的列表
CAR_LIGHTS = {
    'None': [carla.VehicleLightState.NONE],  # 所有灯光关闭
    'Position': [carla.VehicleLightState.Position],  # 位置灯
    'LowBeam': [carla.VehicleLightState.LowBeam],  # 近光灯
    'HighBeam': [carla.VehicleLightState.HighBeam],  # 高光灯
    'Brake': [carla.VehicleLightState.Brake],  # 刹车灯
    'RightBlinker': [carla.VehicleLightState.RightBlinker],  # 右转向灯
    'LeftBlinker': [carla.VehicleLightState.LeftBlinker],  # 左转向灯
    'Reverse': [carla.VehicleLightState.Reverse],  # 倒车灯
    'Fog': [carla.VehicleLightState.Fog],  # 雾灯
    'Interior': [carla.VehicleLightState.Interior],  # 内饰灯
    'Special1': [carla.VehicleLightState.Special1],  # 特殊灯光1
    'Special2': [carla.VehicleLightState.Special2],  # 特殊灯光2
    'All': [carla.VehicleLightState.All]}  # 所有灯光

# 定义一个字典，用于表示灯光组的不同选项
# 键名是不同灯光组的描述，值是对应的carla.LightGroup枚举值组成的列表
LIGHT_GROUP = {
    'None': [carla.LightGroup.NONE],  # 无灯光组
    # 'Vehicle' : [carla.LightGroup.Vehicle], 
    'Street': [carla.LightGroup.Street],  # 街道灯光组
    'Building': [carla.LightGroup.Building],  # 建筑灯光组
    'Other': [carla.LightGroup.Other]}  # 其他灯光组

# 函数功能：根据传入的命令行参数args和天气对象weather，应用日照预设来设置太阳位置
# 如果命令行中指定的日照预设（通过args.sun获取）在SUN_PRESETS字典中有定义，则将对应的太阳高度角和方位角赋值给天气对象的相应属性
# 如果指定的预设不存在，则打印错误信息并退出程序
def apply_sun_presets(args, weather):
    """Uses sun presets to set the sun position"""
    if args.sun is not None:
        if args.sun in SUN_PRESETS:
            # 设置太阳高度角，从SUN_PRESETS字典中获取对应预设的太阳高度角数值并赋值给天气对象的sun_altitude_angle属性
            weather.sun_altitude_angle = SUN_PRESETS[args.sun][0]
            # 设置太阳方位角，从SUN_PRESETS字典中获取对应预设的太阳方位角数值并赋值给天气对象的sun_azimuth_angle属性
            weather.sun_azimuth_angle = SUN_PRESETS[args.sun][1]
        else:
            print("[ERROR]: Command [--sun | -s] '" + args.sun + "' not known")
            sys.exit(1)

# 函数功能：根据传入的命令行参数args和天气对象weather，应用天气预设来设置各种天气参数
# 如果命令行中指定的天气预设（通过args.weather获取）在WEATHER_PRESETS字典中有定义，则依次将列表中对应的各个天气参数值赋值给天气对象的相应属性
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

# 函数功能：根据传入的命令行参数args和天气对象weather，逐个设置天气参数的值
# 检查每个对应的命令行参数是否有值（不为None），如果有值则将其赋值给天气对象的相应属性，实现对天气各参数的单独设置
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

# 函数功能：根据传入的命令行参数args和世界对象world，将指定的车辆灯光状态应用到世界中的所有车辆上
# 如果命令行参数args.cars为None（即没有指定车辆灯光相关设置），则直接返回不做操作
# 否则，通过循环遍历args.cars中的选项，将对应的车辆灯光状态掩码进行或运算合并，然后获取世界中的所有车辆，为符合条件的车辆设置相应的灯光状态
def apply_lights_to_cars(args, world):
    if args.cars is None:
        return

    light_mask = carla.VehicleLightState.NONE
    for option in args.cars:
        light_mask |= CAR_LIGHTS[option][0]

    # 获取世界场景中的所有车辆，这里通过world.get_actors()方法获取所有的参与者（包括车辆等各种实体）
    all_vehicles = world.get_actors()
    for ve in all_vehicles:
        # 判断当前参与者是否是车辆（通过类型ID中包含"vehicle."来判断）
        if "vehicle." in ve.type_id:
            # 为车辆设置灯光状态，将合并后的灯光状态掩码赋值给车辆的灯光状态属性
            ve.set_light_state(carla.VehicleLightState(light_mask))

# 函数功能：根据传入的命令行参数args和灯光管理器对象light_manager，对相应灯光组的灯光进行操作
# 如果命令行参数args.lights为None（即没有指定灯光相关操作），则直接返回不做操作
# 首先确定要操作的灯光组（根据args.lightgroup获取，默认为'None'），然后获取该灯光组下的所有灯光
# 接着循环遍历args.lights中的操作选项，根据不同选项（如'on'、'off'、'intensity'、'color'）执行相应的灯光操作，如打开、关闭、设置强度、设置颜色等
def apply_lights_manager(args, light_manager):
    if args.lights is None:
        return

    # 根据命令行参数中指定的灯光组（默认为'None'）来确定要操作的灯光组，从LIGHT_GROUP字典中获取对应的枚举值列表
    light_group = 'None'
    if args.lightgroup is not None:
        light_group = args.lightgroup

    # 根据确定的灯光组获取该组下的所有灯光，通过灯光管理器的get_all_lights方法获取
    lights = light_manager.get_all_lights(LIGHT_GROUP[light_group][0])

    i = 0
    while (i < len(args.lights)):
        option = args.lights[i]

        if option == "on":
            # 打开指定灯光组下的所有灯光，调用灯光管理器的turn_on方法
            light_manager.turn_on(lights)
        elif option == "off":
            # 关闭指定灯光组下的所有灯光，调用灯光管理器的turn_off方法
            light_manager.turn_off(lights)
        elif option == "intensity":
            # 设置指定灯光组下所有灯光的强度，通过命令行参数中紧跟在'intensity'后的数值来设置，调用set_intensity方法，并将索引i向后移动一位，跳过已处理的'intensity'参数
            light_manager.set_intensity(lights, int(args.lights[i + 1]))
            i += 1
        elif option == "color":
            # 设置指定灯光组下所有灯光的颜色，通过命令行参数中紧跟在'color'后的三个数值（分别表示红、绿、蓝分量）来设置颜色，调用set_color方法，并将索引i向后移动三位，跳过已处理的表示颜色的三个参数
            r = int(args.lights[i + 1])
            g = int(args.lights[i + 2])
            b = int(args.lights[i + 3])
            light_manager.set_color(lights, carla.Color(r, g, b))
            i += 3

        i += 1


def main():
    """Start function"""
    # 创建一个命令行参数解析器对象，用于解析用户在命令行输入的参数
    # description参数用于设置在命令行帮助信息中显示的描述内容（这里使用了文档字符串__doc__，通常是模块开头的说明信息）
    argparser = argparse.ArgumentParser(
        description=__doc__)
    # 添加命令行参数'--host'，用于指定主机服务器的IP地址，默认值为'127.0.0.1'，并提供相应的帮助信息
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    # 添加命令行参数'-p'或'--port'，用于指定要监听的TCP端口号，默认值为2000，类型为整数，并提供相应的帮助信息
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    # 添加命令行参数'--sun'，用于指定日照预设情况，类型为字符串，默认值为None，并提供可选择的预设值列表作为帮助信息
    argparser.add_argument(
        '--sun',
        default=None,
        type=str,
        help='Sun position presets [' + ' | '.join([i for i in SUN_PRESETS]) + ']')
    # 添加命令行参数'--weather'，用于指定天气预设情况，类型为字符串，默认值为None，并提供可选择的预设值列表作为帮助信息
    argparser.add_argument(
        '--weather',
        default=None,
        type=str,
        help='Weather condition presets [' + ' | '.join([i for i in WEATHER_PRESETS]) + ']')
    # 添加命令行参数'--altitude'或'-alt'，用于指定太阳高度角，类型为浮点数，默认值为None，并提供相应的取值范围帮助信息
    argparser.add_argument(
        '--altitude', '-alt',
        metavar='A',
        default=None,
        type=float,
        help='Sun altitude [-90.0, 90.0]')
    # 添加命令行参数'--azimuth'或'-azm'，用于指定太阳方位角，类型为浮点数，默认值为None，并提供相应的取值范围帮助信息
    argparser.add_argument(
        '--azimuth', '-azm',
        metavar='A',
        default=None,
        type=float,
        help='Sun azimuth [0.0, 360.0]')
    # 添加命令行参数'--clouds'或'-c'，用于指定云量，类型为浮点数，默认值为None，并提供相应的取值范围帮助信息
    argparser.add_argument(
        '--clouds', '-c',
        metavar='C',
        default=None,
        type=float,
        help='Clouds amount [0.0, 100.0]')
    # 添加命令行参数'--rain'或'-r'，用于指定降水量，类型为浮点数，默认值为None，并提供相应的取值范围帮助信息
    argparser.add_argument(
        '--rain', '-r',
        metavar='R',
        default=None,
        type=float,
        help='Rain amount [0.0, 100.0]')
    # 添加命令行参数'--puddles'或'-pd'，用于指定积水量，类型为浮点数，默认
