#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Unpack and copy over CarlaUE4's Exported Assets"

USAGE_STRING="Usage: $0 [-h|--help] [-d|--dir] <outdir>"

OUTPUT_DIRECTORY=""

OPTS=`getopt -o h,d:: --long help,dir:: -n 'parse-options' -- "$@"`

# 检查上一个命令的退出状态码是否不为0（即表示上一个命令执行失败）
if [ $? != 0 ] ; then 
  # 如果是，则打印用法字符串（假设$USAGE_STRING是之前定义的包含脚本用法的变量）
  echo "$USAGE_STRING" 
  # 并以状态码2退出脚本，表示发生了特定类型的错误
  exit 2
fi

# 使用eval和set命令将OPTS变量的内容作为参数列表重新赋值给位置参数（$1, $2, ...）
eval set -- "$OPTS"

# 进入一个无限循环，用于处理位置参数
while true; do
  # 使用case语句匹配当前的位置参数（$1）
  case "$1" in
    # 如果当前参数是--dir
    --dir )
      # 将下一个位置参数（$2）赋值给OUTPUT_DIRECTORY变量
      OUTPUT_DIRECTORY="$2"
      # 使用shift命令将位置参数向左移动一位，以便下一个循环迭代处理下一个参数
      shift 
      # 注意：这里缺少对$2是否存在的检查，如果$2为空或不存在，可能会导致脚本出错
      ;;
    # 如果当前参数是-h或--help
    -h | --help )
      # 打印文档字符串（假设$DOC_STRING包含脚本的说明或帮助信息）
      echo "$DOC_STRING"
      # 打印用法字符串
      echo "$USAGE_STRING"
      # 并以状态码1退出脚本，表示用户请求了帮助信息
      exit 1
      ;;
    # 如果当前参数不匹配任何已知选项
    * )
      # 跳出循环，这意味着所有已知的选项都已处理完毕，剩余的参数可能是非选项参数
      break 
      ;;
  esac
done

#Tar.gz the stuff
for filepath in `find Import/ -type f -name "*.tar.gz"`; do
  tar --keep-newer-files -xvf ${filepath}
done

