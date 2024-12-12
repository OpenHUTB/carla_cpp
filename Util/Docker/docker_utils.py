#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import tarfile
import os

BLUE = '\033[94m'
GREEN = '\033[92m'
RED = '\033[91m'
ENDC = '\033[0m'
BOLD = '\033[1m'
UNDERLINE = '\033[4m'


class ReadableStream():

    def __init__(self, generator):
        self._generator = generator

    def read(self):
        return next(self._generator)


def get_container_name(container):
    return str(container.attrs['Config']['Image'])


def exec_command(container, command, user="root",
                 silent=False, verbose=False, ignore_error=True):
    command_prefix = "bash -c '"
    if not silent:
        print(''.join([BOLD, BLUE,
                       user, '@', get_container_name(container),
                       ENDC, '$ ', str(command)]))

    command_result = container.exec_run(
        command_prefix + command + "'",
        user=user)
    if not silent and verbose and command_result.exit_code:
        print(''.join([RED, 'Command: "', command,
                       '" exited with error: ', str(command_result.exit_code),
                       ENDC]))
        print('Error:')
    if not silent:
        out = command_result.output.decode().strip()
        if out:
            print(out)
    if not ignore_error and command_result.exit_code:
        exit(1)
    return command_result


def get_file_paths(container, path, user="root",
                   absolute_path=True, hidden_files=True, verbose=False):
    # command = "bash -c 'ls -ad $PWD/* "
    command = "ls "
    if hidden_files:
        command += "-a "
    if absolute_path:
        command += "-d "
    result = exec_command(container, command + path, user=user, silent=True)
    if result.exit_code:
        if verbose:
            print(RED + "No files found in " + path + ENDC)
        return []
    file_list = [x for x in result.output.decode('utf-8').split('\n') if x]
    if verbose:
        print("Found files: " + str(file_list))
    return file_list


def extract_files(container, file_list, out_path):
    # 遍历文件列表
    for file in file_list:
        # 打印正在复制的文件信息
        print('Copying "' + file + '" to ' + out_path)
        # 从容器中获取文件的存档流和信息
        strm, _ = container.get_archive(file)
        # 以二进制写入模式打开文件 result.tar.gz
        f = open("%s/result.tar.gz" % out_path, "wb")
        # 将存档流的数据写入文件
        for d in strm:
            f.write(d)
        # 关闭文件
        f.close()
        # 打开 result.tar.gz 文件作为 TarFile 对象
        pw_tar = tarfile.TarFile("%s/result.tar.gz" % out_path)
        # 解压文件到指定输出路径
        pw_tar.extractall(out_path)
        # 删除生成的临时压缩文件
        os.remove("%s/result.tar.gz" % out_path)
