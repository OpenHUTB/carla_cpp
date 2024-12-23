#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Download big files from Google Drive."""

import argparse  # 导入 argparse 库，用于解析命令行参数。
import shutil    # 导入 shutil 库，提供对文件和文件集的高级操作，比如复制和移动文件。
import sys       # 导入 sys 库，用于访问与 Python 解释器交互的一些变量和功能，比如处理脚本退出。
import requests   # 导入 requests 库，方便进行 HTTP 请求，很适合用于文件下载等。


# 将字节大小的数字转换为合适的带单位的格式化字符串表示（如KB、MB等）
# 参考自：https://stackoverflow.com/a/1094933/5308925
# 此函数将文件大小数字转换为更易读的格式，添加适当的单位后缀（B、K、M、G等）
def sizeof_fmt(num, suffix='B'):
    for unit in ['', 'K', 'M', 'G', 'T', 'P', 'E', 'Z']:
        if abs(num) < 1000.0:
            # 当数字小于1000时，使用当前单位并保留两位小数
            return "%3.2f%s%s" % (num, unit, suffix)
        num /= 1000.0
    # 对于非常大的数字，使用Yi作为单位
    return "%.2f%s%s" % (num, 'Yi', suffix)


# 在终端打印下载状态，包括目标文件路径和已下载的大小格式化后的信息
# 打印下载状态信息，包括下载的目标文件和已下载的大小
# 获取终端的宽度，并计算需要填充的空格数量，然后打印状态信息，并使用空格填充，确保不会换行
def print_status(destination, progress):
    message = "Downloading %s...    %s" % (destination, sizeof_fmt(progress))
    empty_space = shutil.get_terminal_size((80, 20)).columns - len(message)
    sys.stdout.write('\r' + message + empty_space * ' ')
    sys.stdout.flush()


# 从Google Drive下载文件的主函数，根据给定的文件ID和目标保存路径来下载文件
# 参考自：https://stackoverflow.com/a/39225039/5308925
def download_file_from_google_drive(id, destination):
    # 内部函数，用于将响应内容保存到指定的目标文件中，以32768字节为块大小下载文件
    def save_response_content(response, destination):
        chunk_size = 32768
        written_size = 0
        # 以二进制写入模式打开文件
        with open(destination, "wb") as f:
            # 迭代下载内容，每次读取一个块，过滤掉keep-alive新块，将块写入文件，更新已写入大小并打印下载状态
            for chunk in response.iter_content(chunk_size):
                if chunk:
                    f.write(chunk)
                    written_size += chunk_size
                    print_status(destination, written_size)
        print('Done.')

    # 内部函数，从响应的cookies中获取确认令牌（用于处理Google Drive下载需要确认的情况），从响应的cookie中查找下载警告标记并返回其值
    def get_confirm_token(response):
        for key, value in response.cookies.items():
            if key.startswith('download_warning'):
                return value
        return None

    url = "https://docs.google.com/uc?export=download"
    # 创建一个会话对象，发起请求，使用流模式下载文件
    session = requests.Session()
    response = session.get(url, params={'id': id}, stream=True)
    # 获取确认令牌（如果需要）
    token = get_confirm_token(response)
    if token:
        params = {'id': id, 'confirm': token}
        # 重新发起请求，包含确认令牌
        response = session.get(url, params=params, stream=True)
    # 保存下载的文件内容
    save_response_content(response, destination)


if __name__ == "__main__":
    try:
        # 创建命令行参数解析器
        argparser = argparse.ArgumentParser(description=__doc__)
        # 添加一个参数，用于接收Google Drive文件的ID
        argparser.add_argument(
            'id',
            help='Google Drive\'s file id')
        # 添加一个参数，用于接收文件的目标存储路径
        argparser.add_argument(
            'destination',
            help='destination file path')
        # 解析命令行参数
        args = argparser.parse_args()
        # 调用下载函数，传入文件ID和存储路径
        download_file_from_google_drive(args.id, args.destination)
    # 捕获用户的键盘中断（Ctrl+C），打印取消信息
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
