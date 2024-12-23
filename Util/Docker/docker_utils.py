#!/usr/bin/env python
"""
容器文件处理工具

提供一组用于容器内文件操作的实用工具，包括：
- 命令执行
- 文件路径获取
- 文件提取
- 流式数据处理

Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
de Barcelona (UAB).

This work is licensed under the terms of the MIT license.
For a copy, see <https://opensource.org/licenses/MIT>.
"""

import tarfile
import os
from typing import List, Generator, Tuple, Optional

# ==============================================================================
# 终端颜色常量
# ==============================================================================
class TerminalColors:
    """终端ANSI颜色代码"""
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    RED = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

# ==============================================================================
# 流处理类
# ==============================================================================
class ReadableStream:
    """提供可读取接口的数据流包装器"""
    
    def __init__(self, generator: Generator) -> None:
        """
        初始化流对象
        
        Args:
            generator: 数据生成器
        """
        self._generator = generator
        
    def read(self) -> bytes:
        """
        读取流中的下一个数据块
        
        Returns:
            bytes: 读取的数据
        """
        return next(self._generator)

# ==============================================================================
# 容器操作函数
# ==============================================================================
def get_container_name(container) -> str:
    """
    获取容器名称
    
    Args:
        container: Docker容器对象
        
    Returns:
        str: 容器名称
    """
    return str(container.attrs['Config']['Image'])

def exec_command(
    container,
    command: str,
    user: str = "root",
    silent: bool = False,
    verbose: bool = False,
    ignore_error: bool = True
) -> Tuple[int, str]:
    """
    在容器中执行命令
    
    Args:
        container: Docker容器对象
        command: 要执行的命令
        user: 执行命令的用户名
        silent: 是否静默执行
        verbose: 是否显示详细信息
        ignore_error: 是否忽略错误
        
    Returns:
        Tuple[int, str]: (退出码, 输出内容)
    """
    # 构建命令
    command_prefix = "bash -c '"
    full_command = f"{command_prefix}{command}'"
    
    # 打印命令信息
    if not silent:
        container_info = f"{TerminalColors.BOLD}{TerminalColors.BLUE}{user}@{get_container_name(container)}"
        print(f"{container_info}{TerminalColors.ENDC}$ {command}")
    
    # 执行命令
    result = container.exec_run(full_command, user=user)
    
    # 处理输出
    output = result.output.decode().strip()
    if not silent:
        if verbose and result.exit_code:
            error_msg = (f"{TerminalColors.RED}Command: \"{command}\" "
                        f"exited with error: {result.exit_code}{TerminalColors.ENDC}")
            print(error_msg)
            print("Error:")
        if output:
            print(output)
    
    # 错误处理
    if not ignore_error and result.exit_code:
        raise RuntimeError(f"Command failed with exit code {result.exit_code}")
        
    return result.exit_code, output

def get_file_paths(
    container,
    path: str,
    user: str = "root",
    absolute_path: bool = True,
    hidden_files: bool = True,
    verbose: bool = False
) -> List[str]:
    """
    获取容器中指定路径下的文件列表
    
    Args:
        container: Docker容器对象
        path: 要查找的路径
        user: 执行命令的用户名
        absolute_path: 是否返回绝对路径
        hidden_files: 是否包含隐藏文件
        verbose: 是否显示详细信息
        
    Returns:
        List[str]: 文件路径列表
    """
    # 构建ls命令
    ls_options = []
    if hidden_files:
        ls_options.append("-a")
    if absolute_path:
        ls_options.append("-d")
    
    command = f"ls {' '.join(ls_options)} {path}"
    
    # 执行命令
    exit_code, output = exec_command(container, command, user=user, silent=True)
    
    if exit_code:
        if verbose:
            print(f"{TerminalColors.RED}No files found in {path}{TerminalColors.ENDC}")
        return []
    
    # 处理结果
    file_list = [x for x in output.split('\n') if x]
    if verbose:
        print(f"Found files: {file_list}")
        
    return file_list

def extract_files(container, file_list: List[str], out_path: str) -> None:
    """
    从容器中提取文件
    
    Args:
        container: Docker容器对象
        file_list: 要提取的文件列表
        out_path: 输出目录路径
    """
    temp_archive = os.path.join(out_path, "result.tar.gz")
    
    try:
        for file in file_list:
            print(f'Copying "{file}" to {out_path}')
            
            # 获取文件存档
            archive_stream, _ = container.get_archive(file)
            
            # 写入临时文件
            with open(temp_archive, "wb") as f:
                for chunk in archive_stream:
                    f.write(chunk)
            
            # 解压文件
            with tarfile.TarFile(temp_archive) as tar:
                tar.extractall(out_path)
                
    finally:
        # 清理临时文件
        if os.path.exists(temp_archive):
            os.remove(temp_archive)
