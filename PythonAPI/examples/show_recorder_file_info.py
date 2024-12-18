#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys

# 尝试将Carla库对应的模块路径添加到Python的系统路径中，以便后续能正确导入carla模块。
# 具体操作是根据当前Python版本（主版本号sys.version_info.major和次版本号sys.version_info.minor）以及操作系统类型（通过os.name判断，'nt'表示Windows，'linux-x86_64'表示Linux），
# 去查找符合特定命名格式（carla-*%d.%d-%s.egg）的.egg文件（Carla库的一种打包格式），使用glob.glob获取匹配的文件路径列表，取第一个路径（[0]）添加到sys.path中。
# 如果没找到匹配文件（触发IndexError异常），则直接跳过添加操作，继续往下执行代码。
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

# 导入Carla库，这是用于自动驾驶模拟等相关功能的库，后续会基于它与Carla服务器进行交互、获取相关信息等操作。
import carla

# 导入argparse库，用于方便地解析命令行传入的参数，使得程序可以通过命令行来灵活配置不同的运行参数。
import argparse


def main():
    # 创建一个argparse.ArgumentParser对象，用于定义和解析命令行参数。
    # 使用脚本开头的文档字符串（__doc__）作为参数解析器的描述信息，这样在命令行中使用--help选项查看帮助时，会展示这段文档字符串内容，帮助用户了解程序的大致功能和参数用法。
    argparser = argparse.ArgumentParser(
        description=__doc__)
    # 添加一个名为'--host'的命令行参数，用于指定Carla服务器所在主机的IP地址。
    # metavar='H'表示在帮助信息中显示该参数时使用的变量名为'H'，主要是为了让帮助信息格式更规范、美观且易读。
    # default='127.0.0.1'设定了该参数的默认值为本地主机地址，意味着如果用户运行程序时没有通过命令行指定主机IP，程序将默认连接本地的Carla服务器。
    # help参数提供了对该参数的详细说明，告知用户这个参数的作用以及默认值情况，方便用户按需进行配置。
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    # 添加一个名为'-p'或'--port'的命令行参数，用于指定客户端要监听的TCP端口号。
    # metavar='P'用于帮助信息展示变量名。
    # default=2000将默认端口号设为2000，即用户未指定端口时，程序默认使用2000端口与服务器通信。
    # type=int指定了该参数的数据类型必须是整数，确保传入的值能正确转换并参与后续操作。
    # help参数说明了参数作用及默认值设定，方便用户知晓如何根据实际情况调整端口号。
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    # 添加一个名为'-f'或'--recorder_filename'的命令行参数，用于指定记录器的文件名，这个记录器文件可能包含了Carla模拟场景中的各类数据记录，例如车辆轨迹、场景状态变化等信息。
    # metavar='F'用于帮助信息展示变量名。
    # default="test1.rec"设定了默认的记录器文件名是"test1.rec"，当用户未指定文件名时，程序会默认操作这个名为"test1.rec"的文件。
    # help参数对默认文件名情况做了说明，方便用户了解如何指定不同的记录器文件名。
    argparser.add_argument(
        '-f', '--recorder_filename',
        metavar='F',
        default="test1.rec",
        help='recorder filename (test1.rec)')
    # 添加一个名为'-a'或'--show_all'的命令行参数，用于控制是否展示所有帧内容的详细信息。
    # action='store_true'表示当用户在命令行中指定了这个参数（如--show_all或-a）时，该参数的值会被设置为True，否则为False，这种参数常用于开关型的功能控制。
    # help参数对该参数的作用进行了说明，让用户明白它用于决定是否显示详细的帧内容信息。
    argparser.add_argument(
        '-a', '--show_all',
        action='store_true',
        help='show detailed info about all frames content')
    # 添加一个名为'-s'或'--save_to_file'的命令行参数，用于指定将结果保存到的文件名（需要包含扩展名）。
    # metavar='S'用于在帮助信息中展示变量名。
    # help参数说明了该参数的作用是让用户指定保存结果的文件名，以便将程序运行得到的相关结果保存到文件中，方便后续查看或分析。
    argparser.add_argument(
        '-s', '--save_to_file',
        metavar='S',
        help='save result to file (specify name and extension)')

    # 调用argparse.ArgumentParser对象的parse_args()方法，解析从命令行传入的参数，并将解析结果保存在args对象中。
    # 后续可以通过访问args的不同属性（如args.host、args.port等）来获取各个参数的值，进而依据这些值在程序中执行相应的操作。
    args = argparser.parse_args()

    try:
        # 使用通过命令行参数指定的主机IP地址（args.host）和端口号（args.port）创建一个Carla客户端对象。
        # 这个客户端对象是与Carla服务器进行通信交互的关键接口，通过它可以向服务器发送请求、获取服务器端的数据等操作。
        client = carla.Client(args.host, args.port)
        # 为客户端设置超时时间为60.0秒，意味着如果客户端向服务器发送请求后，在60秒内没有收到服务器的响应，就会判定此次操作超时，避免程序长时间无响应地等待，提升程序的稳定性和交互友好性。
        client.set_timeout(60.0)

        # 判断args.save_to_file参数的值是否为真（即用户是否通过命令行指定了要保存结果到文件）。
        if args.save_to_file:
            # 如果用户指定了保存文件，以写入模式（"w+"，若文件不存在则创建，若存在则覆盖原有内容）打开指定的文件，这里的文件名通过args.save_to_file获取。
            doc = open(args.save_to_file, "w+")
            # 调用客户端的show_recorder_file_info方法，传入记录器文件名（args.recorder_filename）和是否展示所有帧详细信息的参数（args.show_all），
            # 该方法可能用于获取记录器文件相关的信息（根据args.show_all决定是否包含所有帧详细内容），并将返回的结果写入到打开的文件中。
            doc.write(client.show_recorder_file_info(args.recorder_filename, args.show_all))
            # 操作完成后关闭文件，释放相关资源。
            doc.close()
        else:
            # 如果用户没有指定保存文件（即只是想在终端查看结果），则直接调用客户端的show_recorder_file_info方法，传入相应参数，
            # 然后将该方法返回的结果打印输出到终端，让用户可以直接看到记录器文件相关的信息（同样根据args.show_all决定是否包含详细内容）。
            print(client.show_recorder_file_info(args.recorder_filename, args.show_all))

    finally:
        pass


if __name__ == '__main__':
    try:
        main()
    # 捕获键盘中断异常（比如用户按下Ctrl+C），在捕获到该异常时不做任何具体处理操作，只是简单地让程序能够相对优雅地结束运行，避免异常崩溃退出，给用户一个相对友好的交互体验。
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
