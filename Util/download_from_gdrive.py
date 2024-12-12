#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Download big files from Google Drive."""

import argparse
import shutil
import sys

import requests


# 将字节大小的数字转换为合适的带单位的格式化字符串表示（如KB、MB等）
# 参考自：https://stackoverflow.com/a/1094933/5308925
def sizeof_fmt(num, suffix='B'):
    for unit in ['', 'K', 'M', 'G', 'T', 'P', 'E', 'Z']:
        if abs(num) < 1000.0:
            return "%3.2f%s%s" % (num, unit, suffix)
        num /= 1000.0
    return "%.2f%s%s" % (num, 'Yi', suffix)


# 在终端打印下载状态，包括目标文件路径和已下载的大小格式化后的信息
def print_status(destination, progress):
    message = "Downloading %s...    %s" % (destination, sizeof_fmt(progress))
    # 获取终端宽度，计算需要填充的空白字符数量，用于覆盖之前的打印内容，实现动态更新显示效果
    empty_space = shutil.get_terminal_size((80, 20)).columns - len(message)
    sys.stdout.write('\r' + message + empty_space * ' ')
    sys.stdout.flush()


# 从Google Drive下载文件的主函数，根据给定的文件ID和目标保存路径来下载文件
def download_file_from_google_drive(id, destination):
    # 参考自：https://stackoverflow.com/a/39225039/5308925
    # 内部函数，用于将响应内容保存到指定的目标文件中
    def save_response_content(response, destination):
        # 每次读取的块大小（字节）
        chunk_size = 32768
        written_size = 0

        with open(destination, "wb") as f:
            # 逐块读取响应内容并写入文件，同时更新已写入的大小并打印下载状态
            for chunk in response.iter_content(chunk_size):
                if chunk:  # 过滤掉keep-alive等新的空块
                    f.write(chunk)
                    written_size += chunk_size
                    print_status(destination, written_size)
        print('Done.')

    # 内部函数，从响应的cookies中获取确认令牌（用于处理Google Drive下载需要确认的情况）
    def get_confirm_token(response):
        for key, value in response.cookies.items():
            if key.startswith('download_warning'):
                return value

        return None

    url = "https://docs.google.com/uc?export=download"

    session = requests.Session()

    # 首先发送GET请求获取文件信息，设置stream=True以便后续逐块处理响应内容
    response = session.get(url, params={'id': id}, stream=True)
    token = get_confirm_token(response)

    # 如果获取到确认令牌，说明需要进行额外的确认操作，重新发送带确认参数的GET请求
    if token:
        params = {'id': id, 'confirm': token}
        response = session.get(url, params=params, stream=True)

    # 调用函数保存响应内容到目标文件
    save_response_content(response, destination)


if __name__ == "__main__":
    try:
        # 创建命令行参数解析器，用于解析传入的参数，描述信息取自模块文档字符串
        argparser = argparse.ArgumentParser(description=__doc__)
        # 添加一个参数 'id'，用于接收Google Drive文件的ID，帮助信息提示用户传入对应的ID
        argparser.add_argument(
            'id',
            help='Google Drive\'s file id')
        # 添加一个参数 'destination'，用于接收下载后保存文件的路径，帮助信息提示用户传入目标路径
        argparser.add_argument(
            'destination',
            help='destination file path')
        args = argparser.parse_args()

        # 根据解析得到的文件ID和目标路径，调用函数下载文件
        download_file_from_google_drive(args.id, args.destination)
    # 捕获用户通过键盘中断（比如Ctrl+C）操作引发的异常，打印相应提示信息
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
