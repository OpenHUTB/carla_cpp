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


def main():
    argparser = argparse.ArgumentParser(
        description='CARLA map spawn points extractor')
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-o', '--output-dir',
        required=True,
        help='Output directory path for extraction result')
    args = argparser.parse_args()

    if args.output_dir is None or not os.path.exists(args.output_dir):
        print('output directory not found.')

    logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    logging.info('listening to server %s:%s', args.host, args.port)

    print(__doc__)

    try:
        extract(args)
    except:
        print('\nAn error has occurred in extraction.')


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
    except RuntimeError as e:
        print(e)
