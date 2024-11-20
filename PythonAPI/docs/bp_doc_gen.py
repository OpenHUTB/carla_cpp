#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# 导入全局模块，用于文件路径的匹配
import glob
# 导入操作系统接口模块，用于获取操作系统类型等信息
import os
# 导入系统特定的参数和功能模块
import sys
 
try:
    # 根据Python版本和操作系统类型构造CARLA库的文件名模式
    carla_lib_name = 'carla-*%d.%d-%s.egg' % (
        sys.version_info.major,  # Python主版本号
        sys.version_info.minor,  # Python次版本号
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'  # 根据操作系统选择平台标识
    )
    # 将匹配到的CARLA库文件所在的目录添加到系统路径中，以便能够导入CARLA模块
    sys.path.append(glob.glob('../carla/dist/%s' % carla_lib_name)[0])
except IndexError:
    # 如果没有找到匹配的CARLA库文件，则打印错误信息
    print('\n  [ERROR] Could not find "%s"' % carla_lib_name)
    print('          Blueprint library docs will not be generated')
    print("  .---------------------------------------------------.")
    print("  |     Make sure the python client is compiled!      |")
    print("  '---------------------------------------------------'\n")
    # 为了避免Travis检查失败，这里不抛出错误，而是正常退出
    sys.exit(0)
 
# 导入CARLA模块，由于前面已经将CARLA库文件目录添加到系统路径中，这里可以成功导入
import carla
 
# 定义一个颜色常量，用于文本格式化
COLOR_LIST = '#498efc'
 
# 定义一个函数，用于将元素列表用指定的分隔符连接成一个字符串
def join(elem, separator=''):
    return separator.join(elem)
 
# 定义一个函数，用于给文本添加颜色
def color(col, buf):
    return join(['<font color="', col, '">', buf, '</font>'])
 
# 定义一个函数，用于检查字典中是否包含指定的键和值
def valid_dic_val(dic, value):
    return value in dic and dic[value]
 
# 定义一个函数，用于将文本转换为斜体
def italic(buf):
    return join(['_', buf, '_'])
 
# 定义一个函数，用于将文本转换为粗体
def bold(buf):
    return join(['**', buf, '**'])
 
# 定义一个函数，用于将文本用括号括起来
def parentheses(buf):
    return join(['(', buf, ')'])
 
# 定义一个函数，用于将文本转换为下标
def sub(buf):
    return join(['<sub>', buf, '</sub>'])
 
# 定义一个函数，用于将文本转换为代码格式
def code(buf):
    return join(['`', buf, '`'])
 
# 定义一个MarkdownFile类，用于处理Markdown文档的生成
class MarkdownFile:
    def __init__(self):
        self._data = ""  # 用于存储Markdown文档内容的字符串
        self._list_depth = 0  # 用于记录当前列表的深度
        self.endl = '  \n'  # 定义一个换行符，用于在Markdown文档中添加换行

    def data(self):
        return self._data

    def list_push(self, buf=''):
        if buf:
            self.text(join([
                '    ' * self._list_depth if self._list_depth != 0 else '', '- ', buf]))
        self._list_depth = (self._list_depth + 1)

    def list_pushn(self, buf):
        self.list_push(join([buf, self.endl]))

    def list_pop(self):
        self._list_depth = max(self._list_depth - 1, 0)

    def list_popn(self):
        self.list_pop()
        self._data = join([self._data, '\n'])

    def list_depth(self):
        if self._data.strip()[-1:] != '\n' or self._list_depth == 0:
            return ''
        return join(['    ' * self._list_depth])

    def text(self, buf):
        self._data = join([self._data, buf])

    def textn(self, buf):
        self._data = join([self._data, self.list_depth(), buf, self.endl])

    def not_title(self, buf):
        self._data = join([
            self._data, '\n', self.list_depth(), '#', buf, '\n'])

    def title(self, strongness, buf):
        self._data = join([
            self._data, '\n', self.list_depth(), '#' * strongness, ' ', buf, '\n'])

    def new_line(self):
        self._data = join([self._data, self.endl])

    def code_block(self, buf, language=''):
        return join(['```', language, '\n', self.list_depth(), buf, '\n', self.list_depth(), '```\n'])


def generate_pb_docs():
    """Generates markdown file"""

    print('Generating API blueprint documentation...')
    client = carla.Client('127.0.0.1', 2000)
    client.set_timeout(2.0)
    world = client.get_world()

    bp_dict = {}
    blueprints = [bp for bp in world.get_blueprint_library().filter('*')] # Returns list of all blueprints
    blueprint_ids = [bp.id for bp in world.get_blueprint_library().filter('*')] # Returns list of all blueprint ids

    # Creates a dict key = walker, static, prop, vehicle, sensor, controller; value = [bp_id, blueprint]
    for bp_id in sorted(blueprint_ids):
        bp_type = bp_id.split('.')[0]
        value = []
        for bp in blueprints:
            if bp.id == bp_id:
                value = [bp_id, bp]
        if bp_type in bp_dict:
            bp_dict[bp_type].append(value)
        else:
            bp_dict[bp_type] = [value]

    # Actual documentation
    md = MarkdownFile()
    md.not_title('Blueprint Library')
    md.textn(
        "The Blueprint Library ([`carla.BlueprintLibrary`](../python_api/#carlablueprintlibrary-class)) " +
        "is a summary of all [`carla.ActorBlueprint`](../python_api/#carla.ActorBlueprint) " +
        "and its attributes ([`carla.ActorAttribute`](../python_api/#carla.ActorAttribute)) " +
        "available to the user in CARLA.")

    md.textn("\nHere is an example code for printing all actor blueprints and their attributes:")
    md.textn(md.code_block("blueprints = [bp for bp in world.get_blueprint_library().filter('*')]\n"
                        "for blueprint in blueprints:\n"
                        "   print(blueprint.id)\n"
                        "   for attr in blueprint:\n"
                        "       print('  - {}'.format(attr))", "py"))
    md.textn("Check out the [introduction to blueprints](core_actors.md).")

    for key, value in bp_dict.items(): # bp types, bp's
        md.title(3, key) # Key = walker, static, controller, sensor, vehicle
        for bp in sorted(value): # Value = bp[0]= name bp[1]= blueprint
            md.list_pushn(bold(color(COLOR_LIST, bp[0]))) # bp name
            md.list_push(bold('Attributes:') + '\n')
            for attr in sorted(bp[1], key=lambda x: x.id): # for attribute in blueprint
                md.list_push(code(attr.id))
                md.text(' ' + parentheses(italic(str(attr.type))))
                if attr.is_modifiable:
                    md.text(' ' + sub(italic('- Modifiable')))
                md.list_popn()
            md.list_pop()
            md.list_pop()
        md.list_pop()
    return md.data()


def main():

    script_path = os.path.dirname(os.path.abspath(__file__))

    try:
        docs = generate_pb_docs()

    except RuntimeError:
        print("\n  [ERROR] Can't establish connection with the simulator")
        print("  .---------------------------------------------------.")
        print("  |       Make sure the simulator is connected!       |")
        print("  '---------------------------------------------------'\n")
        # We don't provide an error to prvent Travis checks failing
        sys.exit(0)

    with open(os.path.join(script_path, '../../Docs/bp_library.md'), 'w') as md_file:
        md_file.write(docs)
    print("Done!")

if __name__ == '__main__':
    main()
