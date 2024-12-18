#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Helper script to generate consumables for Carla in Docker
"""

# 引入Python 3的print函数特性，确保即使在Python 2环境下也能使用Python 3的print语法
from __future__ import print_function
 
# 导入必要的库
import argparse  # 用于解析命令行参数
import docker  # Docker SDK，用于操作Docker容器
import docker_utils  # 自定义模块，可能包含与Docker交互的辅助函数
import os  # 用于操作系统功能，如获取当前工作目录
 
 
def print_formated_dict(dic):
    """打印格式化的字典"""
    for k, v in dic.items():
        print(' - "' + str(k) + '"' + ": " + str(v))
    print()  # 空行分隔
 
 
def bold(text):
    """将文本加粗"""
    return ''.join([docker_utils.BOLD, text, docker_utils.ENDC])
 
 
def bold_underline(text):
    """将文本加粗并加下划线"""
    return ''.join([docker_utils.UNDERLINE, bold(text), docker_utils.ENDC])  # 注意ENDC的添加位置


def parse_args():
    """解析命令行参数"""
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '-i', '--input',
        type=str,
        help='Path of all the assets to convert')
    argparser.add_argument(
        '-o', '--output',
        type=str,
        help='Path where all the assets will be after conversion. Default: current directory.')
    argparser.add_argument(
        '--packages',
        type=str,
        help='(Optional) Packages to generate. Usage: "--packages=PkgeName1,PkgeName2"')
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        default=False,
        help='Prints extra information')
    argparser.add_argument(
        '--image',
        type=str,
        help='Use a specific Carla image. Default: "carla:latest"',
        default='carla:latest',
    )
    args = argparser.parse_args()

    if not args.output:
        args.output = os.getcwd()  # 如果没有指定输出路径，则使用当前工作目录
 
    if args.packages and args.packages.strip() and not args.input:
        # 如果指定了包但没有指定输入路径，则报错并退出
        print(
            docker_utils.RED +
            "[Error] The Input Path [-i|--input] must be specified "
            "if you are processing individual packages." + docker_utils.ENDC)
        exit(1)

    print()

    print(bold("- ") + bold_underline("Params:"))
    print(" - Output path: " + str(args.output))
    print(" - Packages:    " + str(args.packages))
    print(" - Input path:  " + str(args.input))
    print(" - Verbose:     " + str(args.verbose))
    print()

    return args


def main():

    args = parse_args()
    carla_image_name = args.image
    inbox_assets_path = '/home/carla/carla/Import'  # Docker容器内的输入资产路径
    client = docker.from_env()  # 创建Docker客户端

    # All possible Docker arguments are here:
    # https://docker-py.readthedocs.io/en/stable/containers.html
    # 设置Docker容器的启动参数
    container_args = {
        "image": carla_image_name,
        "user": 'carla',
        "auto_remove": True,
        "stdin_open": True,
        "tty": True,
        "detach": True}

    if args.packages:
        # 如果指定了包，则设置卷以挂载输入路径到容器内的指定位置
        container_args["volumes"] = {
            args.input: {'bind': inbox_assets_path, 'mode': 'rw'}}

    print(bold("- ") + bold_underline("Docker arguments:"))
    print_formated_dict(container_args)

    try:

        print("Running Docker...")
        carla_container = client.containers.run(**container_args)

        if args.packages:
            # If there is packages, import them first and package them
            docker_utils.exec_command(
                carla_container,
                'make import',
                user='carla', verbose=args.verbose, ignore_error=False)

            docker_utils.exec_command(
                carla_container,
                'make package ARGS="--packages=' + str(args.packages) + '"',
                user='carla', verbose=args.verbose, ignore_error=False)
        else:
            # Just create a package of the whole project
            docker_utils.exec_command(
                carla_container,
                'make package',
                user='carla', verbose=args.verbose, ignore_error=False)

        # Get the files routes to export
        files_to_copy = docker_utils.get_file_paths(
            carla_container,
            '/home/carla/carla/Dist/*.tar.gz',
            user='carla', verbose=args.verbose)

        # Copy these files to the output folder
        docker_utils.extract_files(carla_container, files_to_copy, args.output)

    finally:
        # 无论是否发生异常，都关闭容器
        print("Closing container " + carla_image_name)
        carla_container.stop()


if __name__ == '__main__':
    main()
