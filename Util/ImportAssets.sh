#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================
# 定义一个文档字符串，用于描述这个脚本的主要功能，即解压并复制CarlaUE4的导出资产
DOC_STRING="Unpack and copy over CarlaUE4's Exported Assets"
# 定义一个使用说明字符串，展示脚本的正确使用方式，提示需要传入参数，其中[-h|--help]用于获取帮助信息，[-d|--dir]用于指定输出目录，后面要跟上具体的输出目录参数<outdir>
USAGE_STRING="Usage: $0 [-h|--help] [-d|--dir] <outdir>"
# 初始化一个变量，用于存储输出目录的路径，初始值为空字符串，后续会根据用户传入的参数进行赋值
OUTPUT_DIRECTORY=""
# 使用getopt命令来解析命令行传入的参数选项。
# -o选项指定了短选项格式，这里有'h'（表示帮助）和'd'（表示目录，后面跟参数时需要双冒号，表示参数可选）。
# --long选项指定了长选项格式，对应'help'和'dir'（同样参数可选，用双冒号表示）。
# -n选项指定了在getopt出现错误时显示的错误信息中的脚本名称部分。
# '$@'表示传入脚本的所有命令行参数。
OPTS=`getopt -o h,d:: --long help,dir:: -n 'parse-options' -- "$@"`
# 检查getopt命令的执行结果，如果返回值不等于0，说明参数解析出现错误，此时打印使用说明字符串并以状态码2退出脚本，表示参数错误。
if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2; fi
# 使用eval命令结合set命令，将解析后的参数重新设置，以便后续在脚本中可以方便地按顺序处理各个参数选项和对应的值。
eval set -- "$OPTS"
# 进入一个循环，用于逐个处理解析后的参数选项。
while true; do
  case "$1" in
  # 当参数为--dir（长选项形式，表示指定输出目录）时，
    --dir )
    # 将下一个参数（即输出目录的路径值）赋值给OUTPUT_DIRECTORY变量，然后通过shift命令将参数列表向左移动一位，去掉已经处理过的参数选项及其对应的值。
      OUTPUT_DIRECTORY="$2"
      shift ;;
       # 当参数为-h或者--help（表示获取帮助信息）时，
    -h | --help )
     # 打印描述脚本功能的DOC_STRING字符串和使用说明的USAGE_STRING字符串，然后以状态码1退出脚本，表示正常的帮助信息展示后退出。
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
       # 当参数不匹配前面定义的任何选项时，跳出循环，意味着参数处理完毕。
    * )
      break ;;
  esac
done

#Tar.gz the stuff
# 使用find命令在Import/目录下查找所有类型为文件（-type f）且文件名匹配*.tar.gz模式的文件，然后遍历找到的每个文件路径（filepath变量）。
for filepath in `find Import/ -type f -name "*.tar.gz"`; do
# 针对每个找到的.tar.gz文件，使用tar命令进行解压。
  # --keep-newer-files选项表示如果解压后的文件比已存在的同名文件更新，则保留新文件（覆盖旧文件），-xvf选项用于指定解压操作，${filepath}表示当前遍历到的.tar.gz文件的路径。
  tar --keep-newer-files -xvf ${filepath}
done

