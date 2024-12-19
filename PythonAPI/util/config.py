#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Configure and inspect an instance of CARLA Simulator.

For further details, visit
https://carla.readthedocs.io/en/latest/configuring_the_simulation/
"""

import glob
import os
import sys
# 尝试将CARLA相关的模块路径添加到系统路径中，以便能够正确导入CARLA模块。
# 根据当前Python版本以及操作系统类型（Windows的win-amd64或者Linux的linux-x86_64）来匹配对应的CARLA egg文件路径，并添加到系统路径。
try:
    # 使用glob模块来查找符合特定模式的文件路径。
    # 这里构造的查找模式是基于当前Python的版本号以及操作系统类型来生成的。
    # 通过格式化字符串的方式，构造出类似'../carla/dist/carla-<Python主版本号>.<Python次版本号>-<操作系统类型>.egg'这样的模式。
    # 例如在Python 3.8的Windows系统下，模式就是'../carla/dist/carla-3.8-win-amd64.egg'。
    # glob.glob会返回所有匹配该模式的文件路径列表，由于期望只有一个匹配的CARLA egg文件路径，所以取列表中的第一个元素（通过[0]索引获取）添加到系统路径中。
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

import argparse
import datetime
import re
import socket
import textwrap


def get_ip(host):
    """
        获取给定主机名对应的IP地址。
        如果主机名是'localhost'或者'127.0.0.1'，则尝试通过创建UDP套接字连接到一个特定的IP地址（10.255.255.255:1）来获取本地实际IP地址，
        如果在获取过程中出现运行时错误则忽略，最后关闭套接字并返回IP地址。

        参数:
        host: 要获取IP地址的主机名，可能是字符串形式的IP地址或者像'localhost'这样的特殊标识。

        返回值:
        返回对应的IP地址（字符串形式）。
        """
    if host in ['localhost', '127.0.0.1']:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            sock.connect(('10.255.255.255', 1))
            host = sock.getsockname()[0]
        except RuntimeError:
            pass
        finally:
            sock.close()
    return host


def find_weather_presets():
    """
        查找CARLA中所有天气预设的相关信息。
        通过反射获取`carla.WeatherParameters`类中所有符合特定命名规范（以大写字母开头的属性名）的属性，
        然后将这些属性及其对应的名称组成元组列表返回，这些属性代表了不同的天气预设，名称则是对应的预设名称字符串。

        返回值:
        返回一个包含元组的列表，每个元组包含一个`carla.WeatherParameters`类中的天气预设属性和对应的预设名称字符串。
        """
    presets = [x for x in dir(carla.WeatherParameters) if re.match('[A-Z].+', x)]
    return [(getattr(carla.WeatherParameters, x), x) for x in presets]


def list_options(client):
    """
       列出CARLA服务器端可用的一些选项信息，包括天气预设和可用地图。

       参数:
       client: CARLA客户端对象，通过它可以与CARLA服务器进行交互，获取服务器端的相关资源和信息。

       该函数首先获取服务器端可用的地图列表，然后通过`find_weather_presets`函数获取天气预设信息，
       接着将这些信息进行格式化输出，每行缩进一定空格，方便查看。
       """
     # 获取所有可用的地图名称，并去掉路径前缀'/Game/Carla/Maps/'以简化显示
    maps = [m.replace('/Game/Carla/Maps/', '') for m in client.get_available_maps()]
    # 设置缩进格式，每个层次缩进4个空格
    indent = 4 * ' '
     # 定义一个辅助函数 wrap，用来对文本进行格式化处理，使其适合输出
    def wrap(text):
        return '\n'.join(textwrap.wrap(text, initial_indent=indent, subsequent_indent=indent))
        # 输出天气预设信息。这里假设有一个名为 find_weather_presets 的函数，
    # 它返回一系列天气预设，其中包含两个元素的元组，但我们只对第二个元素感兴趣（即预设的名字）
    print('weather presets:\n')
    print(wrap(', '.join(x for _, x in find_weather_presets())) + '.\n')# 使用辅助函数 wrap 对天气预设列表进行格式化后输出
     # 输出可用地图信息。首先对地图名称进行排序，然后使用辅助函数 wrap 进行格式化后输出
    print('available maps:\n')
    print(wrap(', '.join(sorted(maps))) + '.\n') # 格式化并打印排序后的地图名称


def list_blueprints(world, bp_filter):
    """
       列出符合给定过滤器条件的CARLA蓝图（blueprint）信息。
       参数:
       world: CARLA世界对象，通过它可以获取世界中的蓝图库等资源。
       bp_filter: 蓝图过滤器字符串，用于筛选蓝图库中的蓝图。
       该函数从世界对象获取蓝图库，然后根据给定的过滤器筛选出符合条件的蓝图，获取它们的ID并进行排序后输出，
       输出时每个蓝图ID前有一定的缩进，方便查看。
       """
    blueprint_library = world.get_blueprint_library()
    blueprints = [bp.id for bp in blueprint_library.filter(bp_filter)]
    print('available blueprints (filter %r):\n' % bp_filter)
    for bp in sorted(blueprints):
        print('    ' + bp)
    print('')


def inspect(args, client):
    """
       检查CARLA服务器端的详细信息并进行输出展示，包括服务器地址、版本、地图名称、天气情况、时间、帧率、渲染状态、同步模式以及各类演员数量等信息。

       参数:
       args: 命令行参数对象，包含了像主机名、端口号等配置信息。
       client: CARLA客户端对象，用于获取服务器端的各种资源和信息来进行检查。

       该函数首先获取服务器端的IP地址和端口号组成地址字符串，然后获取世界对象、时间、各种设置以及天气等相关信息，
       接着根据获取到的信息进行格式化输出，展示服务器端的详细运行状态和资源情况。
       """
    address = '%s:%d' % (get_ip(args.host), args.port)

    world = client.get_world()
    elapsed_time = world.get_snapshot().timestamp.elapsed_seconds
    elapsed_time = datetime.timedelta(seconds=int(elapsed_time))

    actors = world.get_actors()
    s = world.get_settings()

    weather = 'Custom'
    current_weather = world.get_weather()
    for preset, name in find_weather_presets():
        if current_weather == preset:
            weather = name

    if s.fixed_delta_seconds is None:
        frame_rate = 'variable'
    else:
        frame_rate = '%.2f ms (%d FPS)' % (
            1000.0 * s.fixed_delta_seconds,
            1.0 / s.fixed_delta_seconds)

    print('-' * 34)
    print('address:% 26s' % address)
    print('version:% 26s\n' % client.get_server_version())
    print('map:        % 22s' % world.get_map().name)
    print('weather:    % 22s\n' % weather)
    print('time:       % 22s\n' % elapsed_time)
    print('frame rate: % 22s' % frame_rate)
    print('rendering:  % 22s' % ('disabled' if s.no_rendering_mode else 'enabled'))
    print('sync mode:  % 22s\n' % ('disabled' if not s.synchronous_mode else 'enabled'))
    print('actors:     % 22d' % len(actors))
    print('  * spectator:% 20d' % len(actors.filter('spectator')))
    print('  * static:   % 20d' % len(actors.filter('static.*')))
    print('  * traffic:  % 20d' % len(actors.filter('traffic.*')))
    print('  * vehicles: % 20d' % len(actors.filter('vehicle.*')))
    print('  * walkers:  % 20d' % len(actors.filter('walker.*')))
    print('-' * 34)

# 主函数，整个脚本的核心逻辑入口，负责解析命令行参数并根据参数执行相应的CARLA模拟器相关操作
def main():
    """
       主函数，用于解析命令行参数并执行相应的CARLA操作，如加载地图、设置渲染、同步模式、帧率等相关配置，以及查看各类信息等。
       """
    # 创建一个命令行参数解析器对象，用于定义和解析脚本接收的各种命令行参数，description参数用于描述脚本功能（通常会自动取文档字符串内容）
    argparser = argparse.ArgumentParser(
        description=__doc__)
    # 创建一个命令行参数解析器对象，用于定义和解析脚本接收的各种命令行参数，description参数用于描述脚本功能（通常会自动取文档字符串内容）
    argparser.add_argument(
        '--host',
        metavar='H',
        default='localhost',
        help='IP of the host CARLA Simulator (default: localhost)')
    # 添加命令行参数'-p'（'--port'的缩写形式），用于指定连接CARLA模拟器的TCP端口号，默认值为2000，类型为整数，并提供相应的帮助信息
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port of CARLA Simulator (default: 2000)')
    # 添加命令行参数'-d'（'--default'的缩写形式），若指定该参数（其值为True），则会设置一些默认的配置项，后续会根据此参数进行相关默认设置操作
    argparser.add_argument(
        '-d', '--default',
        action='store_true',
        help='set default settings')
    # 添加命令行参数'-m'（'--map'的缩写形式），用于指定要加载的新地图名称，同时提示可以使用'--list'参数查看可用地图列表
    argparser.add_argument(
        '-m', '--map',
        help='load a new map, use --list to see available maps')
    # 添加命令行参数'-r'（'--reload-map'的缩写形式），若指定该参数（其值为True），则会重新加载当前地图
    argparser.add_argument(
        '-r', '--reload-map',
        action='store_true',
        help='reload current map')
    # 添加命令行参数'--fps'，用于设置固定的每秒帧数（FPS），若值为0则表示使用可变帧率（和'--delta-seconds'功能类似），类型为浮点数，用于控制模拟的帧率相关设置
    argparser.add_argument(
        '--delta-seconds',
        metavar='S',
        type=float,
        help='set fixed delta seconds, zero for variable frame rate')
    # 添加命令行参数'--rendering'，若指定该参数（其值为True），则会启用渲染功能
    argparser.add_argument(
        '--fps',
        metavar='N',
        type=float,
        help='set fixed FPS, zero for variable FPS (similar to --delta-seconds)')
    # 添加命令行参数'--no-rendering'，若指定该参数（其值为True），则会禁用渲染功能，与'--rendering'参数作用相反
    argparser.add_argument(
        '--rendering',
        action='store_true',
        help='enable rendering')
    # 添加命令行参数'--no-sync'，若指定该参数（其值为True），则会禁用同步模式
    argparser.add_argument(
        '--no-rendering',
        action='store_true',
        help='disable rendering')
    # 添加命令行参数'--weather'，用于设置天气预设，同时提示可以使用'--list'参数查看可用的天气预设列表
    argparser.add_argument(
        '--no-sync',
        action='store_true',
        help='disable synchronous mode')
    # 添加命令行参数'-i'（'--inspect'的缩写形式），若指定该参数（其值为True），则会检查模拟的相关详细信息并输出展示
    argparser.add_argument(
    argparser.add_argument(
        '--weather',
        help='set weather preset, use --list to see available presets')
    # 添加命令行参数'-l'（'--list'的缩写形式），若指定该参数（其值为True），则会列出可用的各种选项信息，例如可用地图、天气预设等
    argparser.add_argument(
        '-i', '--inspect',
        action='store_true',
        help='inspect simulation')
    # 添加命令行参数'-b'（'--list-blueprints'的缩写形式），用于列出符合给定过滤器条件的可用蓝图信息，过滤器条件通过参数值指定（使用'*'可列出所有蓝图）
    argparser.add_argument(
        '-l', '--list',
        action='store_true',
        help='list available options')
    # 添加命令行参数'-x'（'--xodr-path'的缩写形式），用于指定OpenDRIVE文件的路径，以便加载一个具有该文件所描述的道路表示的新地图
    argparser.add_argument(
        '-b', '--list-blueprints',
        metavar='FILTER',
        help='list available blueprints matching FILTER (use \'*\' to list them all)')
    # 添加命令行参数'--osm-path'，用于指定OpenStreetMaps文件的路径，以便加载一个基于该文件转换后的道路表示的新地图
    argparser.add_argument(
        '-x', '--xodr-path',
        metavar='XODR_FILE_PATH',
        help='load a new map with a minimum physical road representation of the provided OpenDRIVE')
    # 添加命令行参数'--tile-stream-distance'，用于设置瓦片流距离（针对大型地图有效），类型为浮点数，控制地图相关的加载显示等特性
    argparser.add_argument(
        '--osm-path',
        metavar='OSM_FILE_PATH',
        help='load a new map with a minimum physical road representation of the provided OpenStreetMaps')
    argparser.add_argument(
    # 添加命令行参数'--actor-active-distance'，用于设置演员（如车辆、行人等实体）的活动距离（针对大型地图有效），类型为浮点数，控制实体相关的显示等特性
        '--tile-stream-distance',
        metavar='N',
        type=float,
        help='Set tile streaming distance (large maps only)')
    argparser.add_argument(
        '--actor-active-distance',
        metavar='N',
        type=float,
        help='Set actor active distance (large maps only)')
    # 如果命令行参数数量小于2（即除了脚本名称外没有其他参数传入），则打印帮助信息并返回，不执行后续操作
    if len(sys.argv) < 2:
        argparser.print_help()
        return
    # 解析命令行参数，将解析后的参数对象赋值给args变量，后续根据这些参数来执行具体的CARLA相关操作
    args = argparser.parse_args()
    # 创建一个CARLA客户端对象，用于连接到CARLA模拟器，传入主机IP、端口号以及指定工作线程数为1，并设置连接超时时间为10秒
    client = carla.Client(args.host, args.port, worker_threads=1)
    client.set_timeout(10.0)
    # 如果命令行中指定了'--default'参数（即设置默认配置），则进行以下默认设置操作
    if args.default:
        args.rendering = True
        args.delta_seconds = 0.0
        args.weather = 'Default'
        args.no_sync = True
    # 如果命令行中指定了'--map'参数（即要加载新地图），则执行以下操作
    if args.map is not None:
        print('load map %r.' % args.map)
        # 通过客户端对象加载指定名称的地图，并获取对应的世界对象（CARLA中的虚拟世界）赋值给world变量
        world = client.load_world(args.map)
        # 如果命令行中指定了'--reload-map'参数（即重新加载当前地图），则执行以下操作
    elif args.reload_map:
        print('reload map.')
        # 通过客户端对象重新加载当前地图，并获取对应的世界对象赋值给world变量
        world = client.reload_world()
        # 如果命令行中指定了'--xodr-path'参数（即要基于OpenDRIVE文件加载地图），则执行以下操作
    elif args.xodr_path is not None:
        # 首先检查指定的OpenDRIVE文件路径是否存在
        if os.path.exists(args.xodr_path):
            with open(args.xodr_path, encoding='utf-8') as od_file:
                try:
                    data = od_file.read()
                except OSError:
                    print('file could not be readed.')
                    sys.exit()
            print('load opendrive map %r.' % os.path.basename(args.xodr_path))
            # 设置生成地图时的一些参数，如顶点距离（单位：米），用于控制道路等几何图形的精度相关特性
            vertex_distance = 2.0  # in meters
            # 设置最大道路长度（单位：米），用于控制地图生成时道路的长度相关特性
            max_road_length = 500.0 # in meters
            # 设置墙壁高度（单位：米），用于地图中相关建筑物等结构的高度设置
            wall_height = 1.0      # in meters
            # 设置额外宽度（单位：米），用于道路等结构的宽度相关扩展设置
            extra_width = 0.6      # in meters
            # 通过客户端对象基于读取的OpenDRIVE文件数据以及设定的地图生成参数来生成对应的OpenDRIVE格式的世界（地图），并获取对应的世界对象赋值给world变量
            world = client.generate_opendrive_world(
                data, carla.OpendriveGenerationParameters(
                    vertex_distance=vertex_distance,
                    max_road_length=max_road_length,
                    wall_height=wall_height,
                    additional_width=extra_width,
                    smooth_junctions=True,
                    enable_mesh_visibility=True))
        else:
            print('file not found.')
            # 如果命令行中指定了'--osm-path'参数（即要基于OpenStreetMaps文件加载地图），则执行以下操作
    elif args.osm_path is not None:
        # 首先检查指定的OpenStreetMaps文件路径是否存在
        if os.path.exists(args.osm_path):
            with open(args.osm_path, encoding='utf-8') as od_file:
                try:
                    data = od_file.read()
                    # 读取文件内容到data变量中
                except OSError:
                    print('file could not be readed.')
                    sys.exit()
            print('Converting OSM data to opendrive')
            # 将读取的OpenStreetMaps数据转换为OpenDRIVE格式的数据，调用相关转换函数完成转换操作
            xodr_data = carla.Osm2Odr.convert(data)
            print('load opendrive map.')
            # 设置生成地图时的一些参数，如顶点距离（单位：米），用于控制道路等几何图形的精度相关特性
            vertex_distance = 2.0  # in meters
            # 设置最大道路长度（单位：米），用于控制地图生成时道路的长度相关特性
            max_road_length = 500.0 # in meters
            # 设置墙壁高度（单位：米），当前设置为0米，用于地图中相关建筑物等结构的高度设置
            wall_height = 0.0      # in meters
            # 设置额外宽度（单位：米），用于道路等结构的宽度相关扩展设置
            extra_width = 0.6      # in meters
            # 通过客户端对象基于转换后的OpenDRIVE格式数据以及设定的地图生成参数来生成对应的OpenDRIVE格式的世界（地图），并获取对应的世界对象赋值给world变量
            world = client.generate_opendrive_world(
                xodr_data, carla.OpendriveGenerationParameters(
                    vertex_distance=vertex_distance,
                    max_road_length=max_road_length,
                    wall_height=wall_height,
                    additional_width=extra_width,
                    smooth_junctions=True,
                    enable_mesh_visibility=True))
        else:
            print('file not found.')

    else:
        # 如果没有上述加载新地图等相关操作的参数指定，则获取当前的世界对象赋值给world变量，即保持当前地图不变
        world = client.get_world()

    settings = world.get_settings()
    # 获取当前世界的设置对象，用于后续对世界的各种设置项进行修改操作，如渲染模式、同步模式、帧率等相关设置
    if args.no_rendering:
        print('disable rendering.')
        settings.no_rendering_mode = True
    elif args.rendering:
        print('enable rendering.')
        settings.no_rendering_mode = False
    # 如果命令行中指定了'--no-sync'参数（即禁用同步模式），则执行以下操作
    if args.no_sync:
        print('disable synchronous mode.')
        settings.synchronous_mode = False
        # 如果命令行中指定了'--delta-seconds'参数（即设置了固定时间步长），则将对应的时间步长值赋给设置对象中的固定时间步长属性
    if args.delta_seconds is not None:
    if args.delta_seconds is not None:
        settings.fixed_delta_seconds = args.delta_seconds
        # 如果命令行中指定了'--fps'参数（即设置了固定帧率），则根据帧率值计算对应的时间步长（帧率的倒数）并赋给设置对象中的固定时间步长属性，若帧率为0则设置时间步长为0（表示可变帧率）
    elif args.fps is not None:
        settings.fixed_delta_seconds = (1.0 / args.fps) if args.fps > 0.0 else 0.0
    # 如果命令行中指定了'--delta-seconds'参数或者'--fps'参数（即涉及到帧率相关设置操作），则根据最终设置的固定时间步长值来输出相应的帧率信息或者提示设置了可变帧率
    if args.delta_seconds is not None or args.fps is not None:
        if settings.fixed_delta_seconds > 0.0:
            print('set fixed frame rate %.2f milliseconds (%d FPS)' % (
                1000.0 * settings.fixed_delta_seconds,
                1.0 / settings.fixed_delta_seconds))
        else:
            print('set variable frame rate.')
            settings.fixed_delta_seconds = None
    # 如果命令行中指定了'--tile-stream-distance'参数（即设置了瓦片流距离），则将对应的距离值赋给设置对象中的瓦片流距离属性
    if args.tile_stream_distance is not None:
        settings.tile_stream_distance = args.tile_stream_distance
        # 如果命令行中指定了'--actor-active-distance'参数（即设置了演员活动距离），则将对应的距离值赋给设置对象中的演员活动距离属性
    if args.actor_active_distance is not None:
        settings.actor_active_distance = args.actor_active_distance
    # 将修改后的设置对象应用到世界中，使设置生效
    world.apply_settings(settings)
    # 如果命令行中指定了'--weather'参数（即设置天气预设），则执行以下操作
    if args.weather is not None:
        if not hasattr(carla.WeatherParameters, args.weather):
            # 首先检查指定的天气预设名称是否存在于CARLA的天气预设参数中，如果不存在则输出错误信息
            print('ERROR: weather preset %r not found.' % args.weather)
        else:
            print('set weather preset %r.' % args.weather)
            # 如果天气预设名称存在，则获取对应的天气预设参数，并设置到世界对象中，改变世界的天气状态
            world.set_weather(getattr(carla.WeatherParameters, args.weather))
    # 如果命令行中指定了'--inspect'参数（即检查模拟相关信息），则调用inspect函数来输出展示模拟的详细信息，传入命令行参数对象和客户端对象作为参数
    if args.inspect:
        inspect(args, client)
        # 如果命令行中指定了'--list'参数（即列出可用选项信息），则调用list_options函数来输出展示可用的各种选项信息，传入客户端对象作为参数
    if args.list:
        list_options(client)
        # 如果命令行中指定了'--list-blueprints'参数（即列出符合条件的蓝图信息），则调用list_blueprints函数来输出展示符合给定过滤器条件的蓝图信息，传入世界对象和过滤器参数作为参数
    if args.list_blueprints:
        list_blueprints(world, args.list_blueprints)


if __name__ == '__main__':

    try:

        main()

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
    except RuntimeError as e:
        print(e)
