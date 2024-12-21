""" CARLA map spawn points extractor """

from __future__ import print_function
import argparse
import logging
import glob
import os
import sys

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla


def extract(args):
    try:
        client = carla.Client(args.host, args.port, worker_threads=1)
        client.set_timeout(2.0)

        world = client.get_world()
        try:
            _map = world.get_map()
        except RuntimeError as error:
            logging.info('RuntimeError: %s', error)
            sys.exit(1)

        if not _map.get_spawn_points():
            logging.info('There are no spawn points available in your map/town.')
            logging.info('Please add some Vehicle Spawn Point to your UE4 scene.')
            sys.exit(1)
        spawn_points = _map.get_spawn_points()
        with open(args.output_dir + "/spawn_points.csv", "w", encoding='utf8') as file:
            index = 0
            for index, spawn_point in enumerate(spawn_points):
                file.write(f'{index},{spawn_point.location.x},{spawn_point.location.y},{spawn_point.location.z}\n')

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


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
    except RuntimeError as e:
        print(e)
