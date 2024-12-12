#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import tarfile
import os

# 定义不同颜色的 ANSI 转义序列，用于终端输出颜色设置
BLUE = '\033[94m'
GREEN = '\033[92m'
RED = '\033[91m'
ENDC = '\033[0m'
BOLD = '\033[1m'
UNDERLINE = '\033[4m'


class ReadableStream():
    def __init__(self, generator):
        # 初始化函数，接收一个生成器并存储在实例变量中
        self._generator = generator

    def read(self):
        # 读取生成器的下一个元素
        return next(self._generator)


def get_container_name(container):
    # 从容器的属性中获取容器的名称
    return str(container.attrs['Config']['Image'])


def exec_command(container, command, user="root",
                silent=False, verbose=False, ignore_error=True):
    # 定义命令前缀，用于在容器中执行 bash 命令
    command_prefix = "bash -c '"
    if not silent:
        # 如果不是静默模式，打印命令执行的用户和命令信息
        print(''.join([BOLD, BLUE,
                     user, '@', get_container_name(container),
                     ENDC, '$ ', str(command)]))

    # 在容器中执行命令，使用用户和命令前缀
    command_result = container.exec_run(
        command_prefix + command + "'",
        user=user)
    if not silent and verbose and command_result.exit_code:
        # 如果不是静默模式且开启了详细输出，并且命令执行出错，打印错误信息
        print(''.join([RED, 'Command: "', command,
                     '" exited with error: ', str(command_result.exit_code),
                     ENDC]))
        print('Error:')
    if not silent:
        # 如果不是静默模式，打印命令输出
        out = command_result.output.decode().strip()
        if out:
            print(out)
    if not ignore_error and command_result.exit_code:
        # 如果不忽略错误且命令执行出错，退出程序
        exit(1)
    return command_result


def get_file_paths(container, path, user="root",
                  absolute_path=True, hidden_files=True, verbose=False):
    # 定义初始的 ls 命令，可根据参数进行修改
    command = "ls "
    if hidden_files:
        # 如果需要包含隐藏文件，添加 -a 参数
        command += "-a "
    if absolute_path:
        # 如果需要绝对路径，添加 -d 参数
        command += "-d "
    # 执行命令获取文件列表
    result = exec_command(container, command + path, user=user, silent=True)
    if result.exit_code:
        if verbose:
            # 如果命令执行出错且开启了详细输出，打印错误信息
            print(RED + "No files found in " + path + ENDC)
        return []
    # 将命令输出按行分割并过滤空行，得到文件列表
    file_list = [x for x in result.output.decode('utf-8').split('\n') if x]
    if verbose:
        # 如果开启了详细输出，打印找到的文件列表
        print("Found files: " + str(file_list))
    return file_list


def extract_files(container, file_list, out_path):
    # 遍历文件列表中的每个文件
    for file in file_list:
        # 打印正在复制的文件和目标路径
        print('Copying "' + file + '" to ' + out_path)
        # 从容器中获取文件的存档流和元数据
        strm, _ = container.get_archive(file)
        # 以二进制写入模式打开文件 result.tar.gz
        f = open("%s/result.tar.gz" % out_path, "wb")
        # 将存档流中的数据写入文件
        for d in strm:
            f.write(d)
        # 关闭文件
        f.close()
        # 打开生成的 tar.gz 文件
        pw_tar = tarfile.TarFile("%s/result.tar.gz" % out_path)
        # 解压文件到指定的输出路径
        pw_tar.extractall(out_path)
        # 删除生成的临时 tar.gz 文件
        os.remove("%s/result.tar.gz" % out_path)
