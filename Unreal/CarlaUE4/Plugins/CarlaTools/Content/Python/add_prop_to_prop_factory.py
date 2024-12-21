#!/usr/bin/env python
"""
Script to add new props to the prop factory and json file
"""
import unreal#导入unreal模块
import argparse#导入argparse模块
import json#导入json模块

# 此函数将 PropSize 枚举值转换为字符串表示
def size_to_str(prop_size):
    # 判断传入的 prop_size 是否为特定的枚举值，并返回对应的字符串
    if prop_size == unreal.PropSize.TINY:
        return 'Tiny'
    elif prop_size == unreal.PropSize.SMALL:
        return 'Small'
    elif prop_size == unreal.PropSize.MEDIUM:
        return 'Medium'
    elif prop_size == unreal.PropSize.BIG:
        return 'Big'
    elif prop_size == unreal.PropSize.HUGE:
        return 'Huge'
    else:
        return 'Medium'

# 此函数将字符串表示的道具大小转换为 PropSize 枚举值
def str_to_size(prop_size):
    if prop_size == "tiny":
        return unreal.PropSize.TINY
    elif prop_size == "small":
        return unreal.PropSize.SMALL
    elif prop_size == "medium":
        return unreal.PropSize.MEDIUM
    elif prop_size == "big":
        return unreal.PropSize.BIG
    elif prop_size == "huge":
        return unreal.PropSize.HUGE
    else:
        return unreal.PropSize.MEDIUM

argparser = argparse.ArgumentParser()
# 添加命令行参数 -s/--static_mesh_path，用于指定新道具的静态网格路径
argparser.add_argument(
    '-s', '--static_mesh_path',
    metavar='S',
    default='',
    type=str,
    help='Path to add to static mesh')
# 添加命令行参数 -n/--name，用于指定新道具的名称
argparser.add_argument(
    '-n', '--name',
    metavar='N',
    default='',
    type=str,
    help='prop name')
# 添加命令行参数 --size，用于指定新道具的大小
argparser.add_argument(
    '--size',
    metavar='Z',
    default='',
    type=str,
    help='prop size')
# 解析命令行参数
args = argparser.parse_args()

# 加载道具工厂对象和获取其默认对象
prop_factory_path = '/Game/Carla/Blueprints/Props/PropFactory.PropFactory_C'
prop_factory_class = unreal.load_object(None, prop_factory_path)
prop_factory_default_object = unreal.get_default_object(prop_factory_class)
# 加载静态网格对象
static_mesh = unreal.load_object(None, args.static_mesh_path)
# 从道具工厂的默认对象中获取定义映射
definitions_map = prop_factory_default_object.get_editor_property("DefinitionsMap")
# 生成新的道具参数对象
new_prop_parameters = unreal.PropParameters()
new_prop_parameters.name = args.name
new_prop_parameters.mesh = static_mesh
new_prop_parameters.size = str_to_size(args.size)
# 生成新道具的唯一标识
prop_id = 'static.prop.' + args.name
# 检查新道具是否已经在定义映射中
if prop_id in definitions_map:
    print("The prop is already present in the DefinitionsMap")
else:
    # 将新道具的参数添加到定义映射中
    definitions_map[prop_id] = new_prop_parameters
    # 定义 JSON 配置文件的路径
    unreal.EditorAssetLibrary.save_asset(prop_factory_path, False)
    # 定义 JSON 配置文件的路径
    prop_config_file_path = unreal.Paths.project_content_dir() + "Carla/Config/Default.Package.json"
    # 打开 JSON 配置文件并读取内容
    json_file = open(prop_config_file_path, 'r')
    config_json = json.load(json_file)
    json_file.close()
    # 获取配置文件中的道具列表
    prop_list = config_json['props']
    # 将新道具的信息添加到道具列表中
    prop_list.append(
        {'name' : new_prop_parameters.name,
        'path' : args.static_mesh_path,
        'size' : size_to_str(new_prop_parameters.size)})
    # 打开 JSON 配置文件并写入更新后的内容
    json_file = open(prop_config_file_path, 'w')
    json_file.write(json.dumps(config_json, indent = 4, sort_keys=False))
    json_file.close()
