#!/bin/bash
# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================
# 描述脚本的功能：检索CARLA的插件
DOC_STRING="Retrieve the plugins for CARLA"
# 使用说明字符串，显示如何使用这个脚本
USAGE_STRING="Usage: $0 [-h|--help] [--release]"
# 定义一个变量来标记是否为发布版本
RELEASE=false
# 循环处理传入的参数
while [[ $# -gt 0 ]]; do
  case "$1" in
    --release )
    # 如果参数是--release，则将RELEASE变量设置为true
      RELEASE=true
      shift ;;
    -h | --help )
    # 如果参数是-h或--help，则打印帮助信息并退出
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
    # 如果参数不匹配任何已知选项，则忽略它
      shift ;;
  esac
done

# ==============================================================================
# -- Get Plugins ---------------------------------------------------------------
# ==============================================================================
# 打印日志信息，表示正在检索插件
log "Retrieving Plugins"
# 检查CARLA_ROOT_FOLDER路径下是否存在Plugins目录
if [[ -d "${CARLA_ROOT_FOLDER}Plugins" ]] ; then
# 如果Plugins目录已经存在，则打印日志信息表示插件已经安装
  log "Plugins already installed."
else
# 如果Plugins目录不存在，则根据RELEASE变量的值来克隆插件仓库
  if ${RELEASE} ; then
  # 如果是发布版本，则使用--depth=1参数来克隆，减少克隆的数据量
    git clone --depth=1 --recursive https://github.com/carla-simulator/carla-plugins.git "${CARLA_ROOT_FOLDER}Plugins"
  else
  # 如果不是发布版本，则正常克隆插件仓库
    git clone --recursive https://github.com/carla-simulator/carla-plugins.git "${CARLA_ROOT_FOLDER}Plugins"
  fi
fi
