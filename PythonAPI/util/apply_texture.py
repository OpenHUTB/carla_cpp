#!/usr/bin/env python
""" TL info printer
"""
# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import glob
import os
import sys
import argparse
import math
import time
import queue
import imageio
# 导入多个Python标准库模块，分别用于文件路径查找（glob）、操作系统相关操作（os）、系统相关功能（sys）、
# 命令行参数解析（argparse）、数学运算（math）、时间处理（time）、队列操作（queue）以及图像读取（imageio）等功能。
# ==============================================================================
# -- find carla module ---------------------------------------------------------
# ==============================================================================

try:#尝试将特定的路径添加到系统路径sys.path中
    # 尝试查找Carla模块所在的.egg文件路径，并将其添加到系统路径sys.path中，以便后续能正确导入carla模块。
    # 根据Python版本信息（major表示主版本号，minor表示次版本号）以及操作系统类型（win-amd64表示Windows，
    # linux-x86_64表示Linux）来构建通配符模式，查找对应的.egg文件。
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    # 如果没有找到匹配的.egg文件（即glob.glob返回的列表为空，引发IndexError），则跳过添加路径操作，继续执行后续代码。
    pass

import carla
# 导入Carla模块，用于与Carla模拟器进行交互，实现各种场景相关操作，如获取场景对象、更新纹理等功能。
def get_8bit_texture(image):
    if image is None:
        return carla.TextureFloatColor(0,0)
    height = len(image)
    width = len(image[0])
    texture = carla.TextureColor(width,height)
    # 遍历图像的每个像素点，获取其颜色信息（RGBA值），并将其设置到创建的纹理对象中。
    # 注意这里y坐标的处理是height - y - 1，可能是因为Carla纹理坐标系统与常规图像坐标系统在y方向上有所不同。
    for x in range(0,width):
        for y in range(0,height):
            color = image[y][x]
            r = int(color[0])
            g = int(color[1])
            b = int(color[2])
            a = int(color[3])
            texture.set(x, height - y - 1, carla.Color(r,g,b,a))
    return texture

def get_float_texture(image):
    if image is None:
        return carla.TextureFloatColor(0,0)
    height = len(image)
    width = len(image[0])
    texturefloat = carla.TextureFloatColor(width,height)
    for x in range(0,width):
        for y in range(0,height):
            color = image[y][x]
            r = int(color[0])/255.0 * 5
            g = int(color[1])/255.0 * 5
            b = int(color[2])/255.0 * 5
            a = 1.0
            texturefloat.set(x, height - y - 1, carla.FloatColor(r,g,b,a))
    return texturefloat

def main():
    argparser = argparse.ArgumentParser()
    # 添加命令行参数'--host'，用于指定Carla服务器的IP地址，默认值为'127.0.0.1'。
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    # 添加命令行参数'-p'或'--port'，用于指定连接Carla服务器的TCP端口号，默认值为2000，类型为整数。
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    # 添加命令行参数'-d'或'--diffuse'，用于指定要更新的漫反射图像文件的路径，默认值为空字符串。
    argparser.add_argument(
        '-d', '--diffuse',
        type=str,
        default='',
        help='Path to diffuse image to update')
    # 添加命令行参数'-o'或'--object-name'，用于指定要应用纹理的对象名称，类型为字符串。
    argparser.add_argument(
        '-o', '--object-name',
        type=str,
        help='Object name')
     # 添加命令行参数'-l'或'--list'，若指定该参数（action='store_true'表示只要出现该参数就设为True），
    # 则会打印出场景中所有对象的名称，然后程序结束。
    argparser.add_argument(
        '-l', '--list',
        action='store_true',
        help='Prints names of all objects in the scene')
     # 添加命令行参数'-n'或'--normal'，用于指定要更新的法线贴图图像文件的路径，默认值为空字符串。
    argparser.add_argument(
        '-n', '--normal',
        type=str,
        default='',
        help='Path to normal map to update')
    # 添加命令行参数'--ao_roughness_metallic_emissive'，用于指定另一种类型的图像（可能是与环境光遮蔽、粗糙度、
    # 金属度、自发光等相关的纹理图像）文件的路径，默认值为空字符串。
    argparser.add_argument(
        '--ao_roughness_metallic_emissive',
        type=str,
        default='',
        help='Path to normal map to update')
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(20.0)
    # 创建一个Carla客户端对象，使用指定的服务器IP地址和端口号进行连接，并设置连接超时时间为20秒。

    world = client.get_world()
# 通过客户端对象获取当前Carla服务器中的世界（world）对象，后续可以通过该对象获取场景中的各种信息和执行相关操作。
    if args.list:
        names = world.get_names_of_all_objects()
        for name in names:
            print(name)
        return
# 如果命令行中指定了'-l'或'--list'参数，就获取场景中所有对象的名称并逐个打印出来，然后结束程序。
    if args.object_name is '':
        print('Error: missing object name to apply texture')
        return

    diffuse = None
    normal = None
    ao_r_m_e = None
    if args.diffuse is not '':
        diffuse = imageio.imread(args.diffuse)
    if args.normal is not '':
        normal = imageio.imread(args.normal)
    if args.ao_roughness_metallic_emissive is not '':
        ao_r_m_e = imageio.imread(args.ao_roughness_metallic_emissive)
# 根据命令行中指定的图像文件路径参数，使用imageio库读取相应的图像数据（如果路径不为空），
    # 分别存储到diffuse、normal、ao_r_m_e变量中，用于后续纹理转换和应用操作。
    tex_diffuse = get_8bit_texture(diffuse)
    tex_normal = get_float_texture(normal)
    tex_ao_r_m_e = get_float_texture(ao_r_m_e)
# 将读取到的图像数据分别转换为对应的Carla纹理格式，得到漫反射纹理、法线纹理以及另一种相关纹理对象。
    world.apply_textures_to_object(args.object_name, tex_diffuse, carla.TextureFloatColor(0,0), tex_normal, tex_ao_r_m_e)

if __name__ == '__main__':
    main()
