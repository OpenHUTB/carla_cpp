#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Unpack and copy over CarlaUE4's Exported Assets"
//定义一个字符串常量，说明了脚本的使用方式，包括可接受的命令行参数格式

USAGE_STRING="Usage: $0 [-h|--help] [-d|--dir] <outdir>"
//用于存储输出目录的变量，初始化为空字符串

OUTPUT_DIRECTORY=""

OPTS=`getopt -o h,d:: --long help,dir:: -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2; fi

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

#Tar.gz the stuff
for filepath in `find Import/ -type f -name "*.tar.gz"`; do
  tar --keep-newer-files -xvf ${filepath}
done

