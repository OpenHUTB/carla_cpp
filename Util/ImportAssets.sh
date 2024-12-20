#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Unpack and copy over CarlaUE4's Exported Assets"
#定义了一个名为 DOC_STRING 的变量，存储了脚本的描述信息，即“解压缩并复制CarlaUE4的导出资产”。
USAGE_STRING="Usage: $0 [-h|--help] [-d|--dir] <outdir>"
#定义了一个名为 USAGE_STRING 的变量，存储了脚本的使用说明。$0 是脚本的名称，[-h|--help] 表示帮助选项，[-d|--dir] <outdir> 表示指定输出目录的选项。
OUTPUT_DIRECTORY=""
#初始化一个名为 OUTPUT_DIRECTORY 的变量，用于存储用户指定的输出目录路径。
OPTS=`getopt -o h,d:: --long help,dir:: -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2; fi
#检查 getopt 命令的退出状态。如果状态不为0（表示参数解析失败），则打印使用说明并退出脚本，退出状态码为2。
eval set -- "$OPTS"

while true; do
  case "$1" in
    --dir )
      OUTPUT_DIRECTORY="$2"
      shift ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      break ;;
  esac
done
#如果参数是 --dir，则将第二个参数（即用户指定的输出目录）赋值给 OUTPUT_DIRECTORY 变量，并使用 shift 命令跳过已处理的参数。
#如果参数是 -h 或 --help，则打印脚本的描述信息和使用说明，然后退出脚本，退出状态码为1。
如果参数不匹配任何已知选项，则退出循环。
#Tar.gz the stuff
for filepath in `find Import/ -type f -name "*.tar.gz"`; do
  tar --keep-newer-files -xvf ${filepath}
done

