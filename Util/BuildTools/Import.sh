#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================
# 描述脚本的功能：导入地图
DOC_STRING="Import maps"
# 使用Here文档定义使用说明字符串，并格式化输出
USAGE_STRING=$(cat <<- END
Usage: $0 [-h|--help] [--python-version=VERSION]
END
)
# 使用getopt命令解析短选项和长选项
OPTS=`getopt -o h --long batch:,package:,no-carla-materials,json-only,python-version:, -n 'parse-options' -- "$@"`
# 初始化ARGS变量，用于存储非选项参数
ARGS=""
# 将getopt的结果赋值给OPTS变量，并将其设置为位置参数
eval set -- "$OPTS"
# 导入环境变量和函数，假设有一个名为Environment.sh的脚本在同一目录下
source $(dirname "$0")/Environment.sh
# 初始化Python版本列表，这里默认使用Python 3
PY_VERSION_LIST=3
# 循环处理传入的参数
while [[ $# -gt 0 ]]; do
  case "$1" in
    --python-version )
    # 如果参数是--python-version，则将PY_VERSION_LIST设置为第二个参数的值
      PY_VERSION_LIST="$2"
      shift 2 ;;
    -h | --help )
    # 如果参数是-h或--help，则打印帮助信息并退出
      echo "$DOC_STRING"
      echo -e "$USAGE_STRING"
      exit 1
      ;;
    --batch )
    # 如果参数是--batch，则将该参数及其值添加到ARGS变量中
      ARGS="${ARGS} $1 $2"
      shift 2 ;;
    --package )
    # 如果参数是--package，则将该参数及其值添加到ARGS变量中
      ARGS="${ARGS} $1 $2"
      shift 2 ;;
    -- )
    # 遇到--则之后的参数作为非选项参数处理
      shift ;;
    * )
    # 如果参数不匹配任何已知选项，则添加到ARGS变量中
      ARGS="${ARGS} $1"
      shift ;;
  esac
done

# Convert comma-separated string to array of unique elements.
IFS="," read -r -a PY_VERSION_LIST <<< "${PY_VERSION_LIST}"

/usr/bin/env python${PY_VERSION_LIST[0]} ${CARLA_BUILD_TOOLS_FOLDER}/Import.py ${ARGS}
