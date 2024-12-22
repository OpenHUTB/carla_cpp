#!/usr/bin/env python
"""
Carla资源生成工具

该脚本用于在Docker容器中生成Carla项目的资源包。支持：
- 生成指定包或全部资源
- 自定义输入和输出路径
- 详细的构建过程日志
"""

from __future__ import print_function

import argparse
import docker
import docker_utils
import os
from typing import Dict, Any, Optional, List

# ==============================================================================
# 终端输出格式化
# ==============================================================================

def format_output(text: str, style: Optional[List[str]] = None) -> str:
    """
    格式化终端输出文本
    
    Args:
        text: 要格式化的文本
        style: 样式列表，可以包含 'bold' 和 'underline'
        
    Returns:
        str: 格式化后的文本
    """
    if not style:
        return text
        
    styled_text = text
    if 'bold' in style:
        styled_text = f"{docker_utils.BOLD}{styled_text}"
    if 'underline' in style:
        styled_text = f"{docker_utils.UNDERLINE}{styled_text}"
        
    return f"{styled_text}{docker_utils.ENDC}"

def print_dict(dictionary: Dict[str, Any], title: str = "") -> None:
    """
    打印格式化的字典内容
    
    Args:
        dictionary: 要打印的字典
        title: 可选的标题
    """
    if title:
        print(format_output(f"- {title}:", ['bold', 'underline']))
        
    for key, value in dictionary.items():
        print(f' - "{key}": {value}')
    print()

# ==============================================================================
# 命令行参数解析
# ==============================================================================

def parse_args() -> argparse.Namespace:
    """
    解析命令行参数
    
    Returns:
        argparse.Namespace: 解析后的参数对象
    """
    parser = argparse.ArgumentParser(description=__doc__)
    
    parser.add_argument(
        '-i', '--input',
        type=str,
        help='资源输入路径'
    )
    
    parser.add_argument(
        '-o', '--output',
        type=str,
        default=os.getcwd(),
        help='资源输出路径，默认为当前目录'
    )
    
    parser.add_argument(
        '--packages',
        type=str,
        help='要生成的包列表，格式：PackageName1,PackageName2'
    )
    
    parser.add_argument(
        '-v', '--verbose',
        action='store_true',
        default=False,
        help='显示详细信息'
    )
    
    parser.add_argument(
        '--image',
        type=str,
        default='carla:latest',
        help='使用指定的Carla镜像，默认为carla:latest'
    )
    
    args = parser.parse_args()
    
    # 验证参数
    if args.packages and not args.input:
        raise ValueError(
            format_output(
                "指定包时必须提供输入路径 [-i|--input]",
                ['bold']
            )
        )
    
    # 打印参数信息
    print_dict({
        'Output path': args.output,
        'Packages': args.packages,
        'Input path': args.input,
        'Verbose': args.verbose
    }, "参数配置")
    
    return args

# ==============================================================================
# Docker容器操作
# ==============================================================================

def setup_container_args(image_name: str, input_path: Optional[str] = None) -> Dict[str, Any]:
    """
    配置Docker容器启动参数
    
    Args:
        image_name: Docker镜像名称
        input_path: 可选的输入路径
        
    Returns:
        Dict[str, Any]: 容器配置参数
    """
    args = {
        "image": image_name,
        "user": 'carla',
        "auto_remove": True,
        "stdin_open": True,
        "tty": True,
        "detach": True
    }
    
    if input_path:
        args["volumes"] = {
            input_path: {
                'bind': '/home/carla/carla/Import',
                'mode': 'rw'
            }
        }
    
    return args

def build_packages(container, packages: Optional[str] = None, verbose: bool = False) -> None:
    """
    在容器中构建包
    
    Args:
        container: Docker容器对象
        packages: 要构建的包列表
        verbose: 是否显示详细信息
    """
    # 如果指定了包，先导入资源
    if packages:
        docker_utils.exec_command(
            container,
            'make import',
            user='carla',
            verbose=verbose,
            ignore_error=False
        )
        
        package_cmd = f'make package ARGS="--packages={packages}"'
    else:
        package_cmd = 'make package'
    
    # 执行打包命令
    docker_utils.exec_command(
        container,
        package_cmd,
        user='carla',
        verbose=verbose,
        ignore_error=False
    )

def export_packages(container, output_path: str, verbose: bool = False) -> None:
    """
    导出生成的包文件
    
    Args:
        container: Docker容器对象
        output_path: 输出路径
        verbose: 是否显示详细信息
    """
    files = docker_utils.get_file_paths(
        container,
        '/home/carla/carla/Dist/*.tar.gz',
        user='carla',
        verbose=verbose
    )
    
    docker_utils.extract_files(container, files, output_path)

# ==============================================================================
# 主函数
# ==============================================================================

def main() -> None:
    """主程序入口"""
    try:
        # 解析命令行参数
        args = parse_args()
        
        # 配置并启动容器
        container_args = setup_container_args(args.image, args.input)
        print_dict(container_args, "Docker配置")
        
        print("启动Docker容器...")
        client = docker.from_env()
        container = client.containers.run(**container_args)
        
        try:
            # 构建包
            build_packages(container, args.packages, args.verbose)
            
            # 导出结果
            export_packages(container, args.output, args.verbose)
            
        finally:
            # 确保容器被关闭
            print(f"关闭容器 {args.image}")
            container.stop()
            
    except Exception as e:
        print(format_output(f"错误: {str(e)}", ['bold']))
        raise

if __name__ == '__main__':
    main()
