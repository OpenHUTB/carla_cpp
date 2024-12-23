# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
#在运行时动态地将Carla库的路径添加到系统路径中，以便可以导入Carla模块

import glob
import os
import sys

try:
    #将指定路径添加到sys.path列表中,查找特定模式的文件路径
    sys.path.append(glob.glob('../../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
    #检查当前操作系统是否为Windows

except IndexError:
#如果glob.glob没有找到任何匹配的文件路径，将抛出IndexError异常
    pass
    #如果发生IndexError异常，不执行任何操作
