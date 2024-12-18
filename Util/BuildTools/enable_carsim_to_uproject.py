#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys
import argparse
import json

def main():
    """
    Edits the uproject file to enable and disable the CarSim plugin
    该函数是程序的主函数，主要功能是编辑uproject文件，实现对CarSim插件的启用或禁用操作
    """
    # 创建一个命令行参数解析器对象，用于解析用户输入的命令行参数
    argparser = argparse.ArgumentParser()
    # 添加一个名为'-f'或'--file'的命令行参数，用于指定uproject文件的路径，参数的默认值为空字符串，类型为字符串，帮助信息提示用户输入文件路径
    argparser.add_argument(
        '-f', '--file',
        metavar='F',
        default="",
        type=str,
        help='Path to the uproject file')
    # 添加一个名为'-e'或'--enable'的命令行参数，该参数为布尔类型，若在命令行中指定此参数，表示要启用CarSim插件
    argparser.add_argument(
        '-e', '--enable',
        action='store_true',
        help='enable carsim')
    # 解析命令行参数，将解析结果保存在args变量中
    args = argparser.parse_args()

    # 尝试以只读模式打开指定的uproject文件（JSON格式），如果文件不存在或无法打开，将会抛出异常
    uproject_file = open(args.file, 'r')
    # 使用json模块的load函数将读取的JSON格式文件内容解析为Python对象（这里应该是字典类型），方便后续操作
    uproject_json = json.load(uproject_file)
    # 关闭打开的文件，释放系统资源
    uproject_file.close()

    # 从解析后的uproject_json（字典类型）中获取"Plugins"键对应的值，通常这应该是一个包含多个插件相关信息的列表，每个元素是一个字典，描述了各个插件的情况
    plugin_list = uproject_json["Plugins"]

    # 初始化两个标志变量，should_do_changes用于标记是否需要对文件进行修改并保存，carsim_found用于标记是否在插件列表中找到了CarSim插件
    should_do_changes = False
    carsim_found = False
    # 遍历插件列表中的每个插件信息字典
    for plugin in plugin_list:
        # 检查当前插件的"Name"键对应的值是否为"CarSim"，以此来判断是否是我们要操作的目标插件
        if plugin['Name'] == 'CarSim':
            # 如果命令行参数指定要启用CarSim插件（args.enable为True）
            if args.enable:
                # 且当前插件处于禁用状态（plugin['Enabled']为False）
                if not plugin['Enabled']:
                    # 则设置should_do_changes为True，表示需要对文件进行修改
                    should_do_changes = True
                    # 将当前插件的"Enabled"键对应的值设置为True，即启用该插件
                    plugin['Enabled'] = True
            # 如果命令行参数没有指定要启用（即可能是要禁用）
            else:
                # 且当前插件处于启用状态（plugin['Enabled']为True）
                if plugin['Enabled']:
                    # 同样设置should_do_changes为True，表示需要对文件进行修改
                    should_do_changes = True
                    # 将当前插件的"Enabled"键对应的值设置为False，即禁用该插件
                    plugin['Enabled'] = False
            # 标记已经在插件列表中找到了CarSim插件
            carsim_found = True
    # 如果没有找到CarSim插件，但命令行参数指定要启用该插件
    if not carsim_found and args.enable:
        # 设置should_do_changes为True，表示需要对文件进行修改
        should_do_changes = True
        # 创建一个表示CarSim插件信息的字典，包含插件名称、市场链接以及初始启用状态为True，并添加到插件列表中
        plugin_list.append({'Name': 'CarSim', 'MarketplaceURL': 'com.epicgames.launcher://ue/marketplace/content/2d712649ca864c80812da7b5252f5608', "Enabled": True})

    # 如果should_do_changes为True，表示有对插件状态进行修改，需要保存文件
    if should_do_changes:
        # 以写入模式打开指定的uproject文件，如果文件不存在将会创建新文件，若存在则会覆盖原有内容
        uproject_file = open(args.file, 'w')
        # 使用json模块的dumps函数将修改后的uproject_json（Python对象）转换回JSON格式字符串，设置缩进为4个空格，并且按照键名排序，使输出的JSON格式更美观易读
        uproject_file.write(json.dumps(uproject_json, indent=4, sort_keys=True))
        # 关闭打开的文件，完成文件保存操作
        uproject_file.close()

if __name__ == '__main__':
    main()
