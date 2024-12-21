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
