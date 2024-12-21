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


#从Carla模拟器中提取地图的生成点信息，并将其保存到一个CSV文件中的程序
def extract(args):
    #在 try语句块中，创建一个carla.Client对象
    try:
        client = carla.Client(args.host, args.port, worker_threads=1)
        client.set_timeout(2.0)

        #通过client.get_world获取Carla世界对象
        world = client.get_world()
        #获取地图对象
        try:
            _map = world.get_map()
        #检查地图的生成点是否为空     
        except RuntimeError as error:
            logging.info('RuntimeError: %s', error)
            sys.exit(1)

        #目的是将生成的信息写入这个CSV文件中
        if not _map.get_spawn_points():
            logging.info('There are no spawn points available in your map/town.')
            logging.info('Please add some Vehicle Spawn Point to your UE4 scene.')
            sys.exit(1)
        spawn_points = _map.get_spawn_points()
        with open(args.output_dir + "/spawn_points.csv", "w", encoding='utf8') as file:
            index = 0
            for index, spawn_point in enumerate(spawn_points):
                file.write(f'{index},{spawn_point.location.x},{spawn_point.location.y},{spawn_point.location.z}\n')

    #通常用于无论是否发生用异常都要执行的清理操作
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
