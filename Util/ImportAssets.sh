#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

# 定义一个字符串常量，用于描述程序的功能，这里表示解压并复制CarlaUE4导出的资产，通常作为文档字符串来对程序功能做简要说明
DOC_STRING="Unpack and copy over CarlaUE4's Exported Assets"

# 定义一个字符串常量，用于说明程序的使用方法，这里指出使用格式为可执行程序名（用$0表示），后面跟着可选的参数选项，如-h或--help获取帮助，-d或--dir后接输出目录参数，<outdir>表示输出目录是必选参数（实际使用时需替换为具体目录）
USAGE_STRING="Usage: $0 [-h|--help] [-d|--dir] <outdir>"

# 定义一个变量，用于存储输出目录的路径，初始为空字符串，表示尚未指定输出目录
OUTPUT_DIRECTORY=""

# 使用getopt命令来解析命令行参数，-o指定短选项（这里是-h和-d），--long指定长选项（这里是help和dir），-n指定出错时的提示信息前缀，"$@"表示传递所有命令行参数给getopt进行解析
OPTS=`getopt -o h,d:: --long help,dir:: -n 'parse-options' -- "$@"`

# 如果getopt命令执行返回值不为0，说明参数解析出错，此时打印使用方法提示信息（USAGE_STRING）并以退出码2退出程序，表示参数解析错误
if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2; fi

# 使用eval命令结合set命令，将解析后的参数设置为位置参数，以便后续在脚本中方便地通过$1、$2等形式访问各个参数，这是处理命令行参数的一种常见方式
eval set -- "$OPTS"

# 进入一个无限循环，用于逐个处理解析后的命令行参数，通过case语句来匹配不同的参数类型并执行相应操作
while true; do
  case "$1" in
    # 如果参数是--dir（长选项形式，表示指定输出目录）
    --dir )
      # 将下一个参数（$2）赋值给OUTPUT_DIRECTORY变量，作为输出目录路径
      OUTPUT_DIRECTORY="$2"
      # 使用shift命令将参数位置左移一位，去掉已经处理过的--dir及其参数，准备处理下一个参数
      shift ;;
     # 如果参数是-h或--help（短选项或长选项形式，表示请求帮助信息）
    -h | --help )
      # 打印程序功能描述（DOC_STRING）
      echo "$DOC_STRING"
      # 打印程序使用方法提示信息（USAGE_STRING）
      echo "$USAGE_STRING"
      # 以退出码1退出程序，表示正常的请求帮助后退出
      exit 1
      ;;
      # 如果参数不匹配上述任何一种情况，表示已经处理完所有预期的参数，使用break跳出循环
    * )
      break ;;
  esac
done


for filepath in `find Import/ -type f -name "*.tar.gz"`; do
  tar --keep-newer-files -xvf ${filepath}
done

