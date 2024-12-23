""" CARLA map spawn points extractor """

from __future__ import print_function#是Python中的一个特性导入语句
import argparse#argparse  是Python内置的用于命令行参数解析的标准库模块。
import logging#logging  同样是Python内置的标准库模块，用于记录程序运行过程中的日志信息。
import glob#glob  模块提供了一种简单的方式来查找符合特定模式的文件路径。
import os#os  模块是Python与操作系统进行交互的重要接口，它封装了很多操作系统相关的功能
import sys#sys  是Python内置的处理系统相关功能的模块

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])#sys.version_info.major  和  sys.version_info.minor  获取Python当前的主版本号和次版本号
except IndexError:
    pass

import carla

#定义一个函数，从 CARLA 模拟器中提取生成点（spawn points）并保存到 CSV 文件中
def extract(args):
    #尝试创建一个 CARLA 客户端并设置超时时间
    try:
        #创建一个与 CARLA 模拟器通信的客户端
        client = carla.Client(args.host, args.port, worker_threads=1)
        #设置客户端操作的超时时间为 2 秒
        client.set_timeout(2.0)
        #获取当前的世界对象
        world = client.get_world()
        #尝试获取当前世界的地图对象，并处理可能出现的运行时错误
        try:
            #获取当前世界的地图对象
            _map = world.get_map()
        #如果出现RuntimeError，则记录错误信息并退出程序
        except RuntimeError as error:
            logging.info('RuntimeError: %s', error)
            sys.exit(1)
        #检查地图是否有生成点，如果没有则记录错误并退出程序
        if not _map.get_spawn_points():
            logging.info('There are no spawn points available in your map/town.')
            logging.info('Please add some Vehicle Spawn Point to your UE4 scene.')
            sys.exit(1)
        #如果有生成点，则获取这些生成点
        spawn_points = _map.get_spawn_points()
        #打开指定路径下的 CSV 文件，以写入模式（"w"）和 UTF - 8 编码打开
        with open(args.output_dir + "/spawn_points.csv", "w", encoding='utf8') as file:
            index = 0
            #遍历生成点列表，将每个生成点的索引、x 坐标、y 坐标和 z 坐标写入文件
            for index, spawn_point in enumerate(spawn_points):
                file.write(f'{index},{spawn_point.location.x},{spawn_point.location.y},{spawn_point.location.z}\n')
    #最后，将world对象设置为None
    finally:
        world = None

# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================

#定义一个函数，功能是配置命令行参数并调用extract函数来提取 CARLA 地图中的生成点（spawn points）
def main():
    #创建了一个ArgumentParser对象来处理命令行参数
    argparser = argparse.ArgumentParser(
        description='CARLA map spawn points extractor')
    #--host：指定主机服务器的 IP 地址，默认为127.0.0.1
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    #-p或--port：指定要监听的 TCP 端口，默认为2000
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    #-o或--output - dir：指定提取结果的输出目录路径，这个参数是必需的
    argparser.add_argument(
        '-o', '--output-dir',
        required=True,
        help='Output directory path for extraction result')
    args = argparser.parse_args()
    #如果输出目录未指定或不存在，则打印错误信息
    if args.output_dir is None or not os.path.exists(args.output_dir):
        print('output directory not found.')
    #设置日志级别为INFO，并记录正在监听的服务器地址和端口
    logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    logging.info('listening to server %s:%s', args.host, args.port)

    print(__doc__)
    #调用extract函数来提取生成点
    try:
        extract(args)
    #如果在提取过程中发生错误，则打印错误信息
    except:
        print('\nAn error has occurred in extraction.')

#判断当前脚本是否作为主程序运行
if __name__ == '__main__':
    #尝试调用main()函数。这意味着在这个脚本中，main()函数应该是主要的执行逻辑所在
    try:
        main()
    #如果用户在程序运行时按下Ctrl + C（KeyboardInterrupt）
    except KeyboardInterrupt:
        #捕获这个异常并打印'Cancelled by user. Bye!'
        print('\nCancelled by user. Bye!')
    #如果在main()函数执行过程中出现RuntimeError
    except RuntimeError as e:
        #捕获这个异常并打印出具体的错误信息e
        print(e)
