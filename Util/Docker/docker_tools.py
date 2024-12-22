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
 #遍历字典dic中的每一个键值对 其中k代表键，v代表值
    for k, v in dic.items():
     #它将键和值转换为字符串（使用str函数），然后按照 - "键": 值的格式进行打印。这样做是为了以一种清晰的格式展示字典的内容
        print(' - "' + str(k) + '"' + ": " + str(v))
    print()  # 空行分隔
 

#这个函数目的是将输入的文本加粗
def bold(text):
    """将文本加粗"""
 #返回一个字符串，这个字符串是通过join方法将docker_utils.BOLD、输入的文本text和docker_utils.ENDC 连接起来
    return ''.join([docker_utils.BOLD, text, docker_utils.ENDC])
 

#目的是将输入的文本加粗并加下划线
#首先，它调用了bold函数对文本进行加粗处理 然后，它使用join方法将docker_utils.UNDERLINE加粗后的文本（bold(text)）和docker_utils.ENDC
def bold_underline(text):
    """将文本加粗并加下划线"""
    return ''.join([docker_utils.UNDERLINE, bold(text), docker_utils.ENDC])  # 注意ENDC的添加位置


#这个函数的目的是解析命令行参数
def parse_args():
    """解析命令行参数"""
 #创建了一个argparse.ArgumentParser对象argparser
    argparser = argparse.ArgumentParser(
        description=__doc__)
 #通过argparser.add_argument方法添加命令行参数
    argparser.add_argument(
        '-i', '--input',
        type=str,
        help='Path of all the assets to convert')
    argparser.add_argument(
     #添加了'-o', '--output'参数，用于指定转换后所有资源的存放路径，默认是当前目录
        '-o', '--output',
        type=str,
        help='Path where all the assets will be after conversion. Default: current directory.')
    argparser.add_argument(
     #添加了'--packages'参数，这是一个可选参数，类型为字符串，用于指定要生成的包
        '--packages',
        type=str,
        help='(Optional) Packages to generate. Usage: "--packages=PkgeName1,PkgeName2"')
    argparser.add_argument(
     #添加了'-v', '--verbose'参数，这个参数的action='store_true'表示如果在命令行中指定了这个参数，它的值就为True，默认值是False，help是对这个参数用途的描述，即打印额外信息
        '-v', '--verbose',
        action='store_true',
        default=False,
        help='Prints extra information')
 #定义了一个名为--image的命令行参数，它是字符串类型，默认值为carla:latest，并且有相应的帮助信息
    argparser.add_argument(
        '--image',
        type=str,
        help='Use a specific Carla image. Default: "carla:latest"',
        default='carla:latest',
    )
    args = argparser.parse_args()
# 对于--output参数，如果没有在命令行中指定则将其设置为当前工作目录 
    if not args.output:
        args.output = os.getcwd()  # 如果没有指定输出路径，则使用当前工作目录

 #如果指定了--packages参数但是没有指定--input参数则会打印错误信息并退出程序
    if args.packages and args.packages.strip() and not args.input:
        # 如果指定了包但没有指定输入路径，则报错并退出
     #最后，函数打印出解析后的命令行参数的值，包括--output、--packages、--input、--verbose等参数的值
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

 # 调用parse_args函数来获取命令行参数
    args = parse_args()
    carla_image_name = args.image
 #获取--image参数的值（carla_image_name = args.image），并定义了一个Docker容器内的输入资产路径
    inbox_assets_path = '/home/carla/carla/Import'  # Docker容器内的输入资产路径
    client = docker.from_env()  # 创建Docker客户端

    # All possible Docker arguments are here:
    # https://docker-py.readthedocs.io/en/stable/containers.html
    # 设置Docker容器的启动参数
    #创建一个名为container_args的字典，这个字典将用于存储Docker容器的启动参数
    container_args = {
     #将carla_image_name作为image参数的值。这个参数指定了要运行的Docker镜像
        "image": carla_image_name,
     #设置在容器内执行操作的用户为carla
        "user": 'carla',
     #当容器停止时自动删除容器，这有助于清理资源并防止无用容器的堆积
        "auto_remove": True,
     #允许容器的标准输入保持打开状态，通常用于支持容器内的交互式操作
        "stdin_open": True,
        "tty": True,
     #为容器分配一个伪终端（tty），也是为了支持交互式操作
     #使得容器在启动后在后台运行
        "detach": True}

 #检查args.packages是否存在
 #如果args.packages存在，就在container_args字典中添加一个"volumes"键
 #为了将外部资源挂载到容器内部，以便容器内的操作能够访问这些资源
    if args.packages:
        # 如果指定了包，则设置卷以挂载输入路径到容器内的指定位置
        container_args["volumes"] = {
            args.input: {'bind': inbox_assets_path, 'mode': 'rw'}}

 #使用bold和bold_underline函数来打印一段带有样式的字符串 可能用于在终端输出中突出显示
    print(bold("- ") + bold_underline("Docker arguments:"))
 #调用print_formated_dict函数来打印格式化后的container_args字典
    print_formated_dict(container_args)
#这是一个异常处理块的开始，表明下面的代码可能会引发异常，并且要进行捕获和处理
    try:

        print("Running Docker...")
     #使用client的containers.run方法来运行Docker容器
     #**container_args语法是将container_args字典解包为关键字参数传递给run方法 这样容器将根据之前定义的参数进行启动，并且将启动后的容器对象赋值给carla_container变量
        carla_container = client.containers.run(**container_args)

     #如果args.packages存在
        if args.packages:
            # If there is packages, import them first and package them
         #调用docker_utils中的exec_command函数，在carla_container容器内执行'make import'命令
            docker_utils.exec_command(
                carla_container,
                'make import',
             #user='carla'指定了在容器内执行命令的用户
             #verbose=args.verbose根据命令行参数设置是否输出详细信息
             #ignore_error=False表示如果命令执行出错不忽略错误
                user='carla', verbose=args.verbose, ignore_error=False)

            #再次调用docker_utils.exec_command函数
         #在容器内执行'make package ARGS="--packages=' + str(args.packages) + '"'命令
         #这个命令可能是根据指定的包进行打包操作，并且也设置了用户、详细信息输出和错误处理相关的参数。
            docker_utils.exec_command(
                carla_container,
                'make package ARGS="--packages=' + str(args.packages) + '"',
                user='carla', verbose=args.verbose, ignore_error=False)
        else:
         #如果args.packages不存在，调用docker_utils.exec_command函数在carla_container容器内执行'make package'命令
            # Just create a package of the whole project
         # 这里执行的是不带特定包参数的make package命令，同样是在carla_container容器内以carla用户执行，详细信息输出和错误处理的设置不变
            docker_utils.exec_command(
                carla_container,
                'make package',
                user='carla', verbose=args.verbose, ignore_error=False)

       # 获取要导出的文件路径
        # Get the files routes to export
       #调用docker_utils.get_file_paths函数来获取文件路径 同样是以carla用户执行，详细信息输出取决于args.verbose
        files_to_copy = docker_utils.get_file_paths(
            carla_container,
            '/home/carla/carla/Dist/*.tar.gz',
            user='carla', verbose=args.verbose)

        # 复制文件到输出文件夹
        # Copy these files to the output folder
        #调用docker_utils.extract_files函数，将前面获取到的文件从carla_container容器复制到args.output指定的输出文件夹
        docker_utils.extract_files(carla_container, files_to_copy, args.output)

    finally:
        # 无论是否发生异常，都关闭容器
       #首先打印出关闭容器的信息，容器名称为carla_image_name，然后调用carla_container.stop()方法来停止容器
        print("Closing container " + carla_image_name)
        carla_container.stop()


if __name__ == '__main__':
    main()
