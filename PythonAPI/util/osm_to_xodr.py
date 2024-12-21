""" Convert OpenStreetMap file to OpenDRIVE file. """

import argparse
import glob
import os
import sys# 导入sys模块，用于访问与Python解释器紧密相关的变量和函数

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,# 获取Python的主版本号
        sys.version_info.minor,# 获取Python的次版本号
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla


def convert(args):
    # Read the .osm data
    with open(args.input_path, mode="r", encoding="utf-8") as osmFile:
        osm_data = osmFile.read()

    # Define the desired settings
    settings = carla.Osm2OdrSettings()

    # Set OSM road types to export to OpenDRIVE
    settings.set_osm_way_types([
        "motorway",
        "motorway_link",
        "trunk",
        "trunk_link",
        "primary",
        "primary_link",
        "secondary",
        "secondary_link",
        "tertiary",
        "tertiary_link",
        "unclassified",
        "residential"
    ])
    settings.default_lane_width = args.lane_width
    settings.generate_traffic_lights = args.traffic_lights
    settings.all_junctions_with_traffic_lights = args.all_junctions_lights
    settings.center_map = args.center_map

    # Convert to .xodr
    xodr_data = carla.Osm2Odr.convert(osm_data, settings)

    # save opendrive file
    with open(args.output_path, "w", encoding="utf-8") as xodrFile:
        xodrFile.write(xodr_data)

# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================


def main():
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '-i', '--input-path',
        required=True,
        metavar='OSM_FILE_PATH',
        help='set the input OSM file path')
    argparser.add_argument(
        '-o', '--output-path',
        required=True,
        metavar='XODR_FILE_PATH',
        help='set the output XODR file path')
    argparser.add_argument(
        '--lane-width',
        default=6.0,
        help='width of each road lane in meters')
    argparser.add_argument(
        '--traffic-lights',
        action='store_true',
        help='enable traffic light generation from OSM data')
    argparser.add_argument(
        '--all-junctions-lights',
        action='store_true',
        help='set traffic lights for all junctions')
    argparser.add_argument(
        '--center-map',
        action='store_true',
        help='set center of map to the origin coordinates')

    if len(sys.argv) < 2:
        argparser.print_help()
        return

    args = argparser.parse_args()

    if args.input_path is None or not os.path.exists(args.input_path):
        print('input file not found.')
    if args.output_path is None:
        print('output file path not found.')

    print(__doc__)

    try:
        convert(args)
    except:
        print('\nAn error has occurred in conversion.')


if __name__ == '__main__':#检查这个脚本是否作为主程序运行。如果是，执行下面的代码块。

    try:
        main()
    except KeyboardInterrupt:#捕获 KeyboardInterrupt 异常，这通常是由用户中断程序（如按Ctrl+C）引起的。
        print('\nCancelled by user. Bye!')
    except RuntimeError as e:#捕获 RuntimeError 异常，并将其赋值给变量 e
        print(e)
