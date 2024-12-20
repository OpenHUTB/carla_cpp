#! /bin/bash

# Sets the environment for other shell scripts.

set -e

CURDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." && pwd )"
source $(dirname "$0")/Vars.mk
unset CURDIR

# 检查环境变量CARLA_BUILD_NO_COLOR是否已设置且非空。
# 如果已设置，则可能意味着在构建过程中不希望输出带有颜色的日志信息。
if [ -n "${CARLA_BUILD_NO_COLOR}" ]; then

  # 定义log函数，用于打印日志信息。
  # 这里使用`basename "$0"`来获取当前脚本的基本名称（即不包含路径的文件名），
  # 并将其与传递给log函数的消息一起打印出来。
  # 注意：由于CARLA_BUILD_NO_COLOR的影响并未直接体现在此函数内部，
  # 因此这里的日志输出可能默认不包含颜色，或者颜色处理在脚本的其他部分实现。
  function log {
      echo "`basename "$0"`: $1"
  }

  # 定义fatal_error函数，用于打印错误信息并退出脚本。
  # 与log函数类似，它使用`basename "$0"`来获取当前脚本的基本名称，
  # 并将其与错误消息一起打印出来。
  # 错误信息被重定向到标准错误输出（stderr，文件描述符2）。
  # 打印完错误信息后，脚本以退出码2退出，表示发生了致命错误。
  function fatal_error {
    echo -e >&2 "`basename "$0"`: ERROR: $1"
    exit 2
  }

# 注意：此代码段没有包含结束if语句的fi，
# 但根据上下文推断，if语句应该在此处结束。
# 在实际脚本中，这两个函数定义应该被包含在if语句的代码块内，
# 除非有其他逻辑需要它们在任何情况下都被定义。
# 然而，由于CARLA_BUILD_NO_COLOR的影响并未直接体现在这两个函数内部，
# 因此将它们放在if语句内可能是为了某种形式的条件性包含或初始化。
# 如果CARLA_BUILD_NO_COLOR的意图是控制日志输出的颜色，
# 那么相关的颜色处理逻辑可能需要在脚本的其他部分实现。

else

  function log {
    echo -e "\033[1;35m`basename "$0"`: $1\033[0m"
  }

  function fatal_error {
    echo -e >&2 "\033[0;31m`basename "$0"`: ERROR: $1\033[0m"
    exit 2
  }

fi

function get_git_repository_version {
  git describe --tags --dirty --always
}

function copy_if_changed {
  mkdir -p $(dirname $2)
  rsync -cIr --out-format="%n" $1 $2
}

function move_if_changed {
  copy_if_changed $1 $2
  rm -f $1
}

CARLA_BUILD_CONCURRENCY=`nproc --all`
