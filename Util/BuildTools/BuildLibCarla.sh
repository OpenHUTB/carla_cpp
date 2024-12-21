#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

# 文档字符串，用于描述该脚本的主要功能
DOC_STRING="Build LibCarla."

# 帮助信息，使用了 here-document 来存储多行字符串
USAGE_STRING=$(cat <<- END
Usage: $0 [-h|--help]

选择一个或多个构建配置

    [--server]  构建服务器端配置.
    [--client]   这个选项用于指示构建系统只构建那些与服务器端相关的部分.

选择一个或多个构建选项
    [--debug]    构建调试目标.
    [--release]  构建发布目标.

其他命令

    [--clean]    清理中间文件.
    [--rebuild]  清理并重新构建两个配置.
END
)

# 一些标志变量，用于存储用户的选择，初始化为 false
REMOVE_INTERMEDIATE=false
BUILD_SERVER=false
BUILD_CLIENT=false
BUILD_OPTION_DEBUG=false
BUILD_OPTION_RELEASE=false
BUILD_OPTION_DUMMY=false
BUILD_RSS_VARIANT=false
USE_PYTORCH=false
USE_ROS2=false

# 使用 getopt 命令解析命令行参数，存储在 OPTS 中
OPTS=`getopt -o h --long help,rebuild,server,client,clean,debug,release,rss,pytorch,carsim,ros2 -n 'parse-options' -- "$@"`

# 将解析后的参数列表重新设置为当前参数列表
eval set -- "$OPTS"

# 遍历解析后的参数列表
while [[ $# -gt 0 ]]; do
  case "$1" in
    # 如果是 --rebuild 参数，设置多个标志为 true
    --rebuild )
      REMOVE_INTERMEDIATE=true;
      BUILD_SERVER=true;
      BUILD_CLIENT=true;
      BUILD_OPTION_DEBUG=true;
      BUILD_OPTION_RELEASE=true;
      shift ;;
    # 如果是 --server 参数，设置 BUILD_SERVER 为 true
    --server )
      BUILD_SERVER=true;
      shift ;;
    # 如果是 --client 参数，设置 BUILD_CLIENT 为 true
    --client )
      BUILD_CLIENT=true;
      shift ;;
    # 如果是 --clean 参数，设置 REMOVE_INTERMEDIATE 为 true 和 BUILD_OPTION_DUMMY 为 true
    --clean )
      REMOVE_INTERMEDIATE=true;
      BUILD_OPTION_DUMMY=true;
      shift ;;
    # 如果是 --debug 参数，设置 BUILD_OPTION_DEBUG 为 true
    --debug )
      BUILD_OPTION_DEBUG=true;
      shift ;;
    # 如果是 --release 参数，设置 BUILD_OPTION_RELEASE 为 true
    --release )
      BUILD_OPTION_RELEASE=true;
      shift ;;
    # 如果是 --pytorch 参数，设置 USE_PYTORCH 为 true
    --pytorch )
      USE_PYTORCH=true;
      shift ;;
    # 如果是 --ros2 参数，设置 USE_ROS2 为 true
    --ros2 )
      USE_ROS2=true;
      shift ;;
    # 如果是 --rss 参数，设置 BUILD_RSS_VARIANT 为 true
    --rss )
      BUILD_RSS_VARIANT=true;
      shift ;;
    # 如果是 -h 或 --help 参数，打印文档字符串和帮助信息并退出
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    # 其他情况，跳过当前参数
    * )
      shift ;;
  esac
done

# 从当前脚本所在目录的 Environment.sh 脚本中导入环境变量或函数
source $(dirname "$0")/Environment.sh

# 定义函数 get_source_code_checksum，用于计算源代码的校验和
function get_source_code_checksum {
  # 排除 __pycache__ 目录，使用 find 查找文件并计算 SHA1 校验和
  local EXCLUDE='*__pycache__*'
  find "${LIBCARLA_ROOT_FOLDER}"/* \! -path "${EXCLUDE}" -print0 | sha1sum | awk '{print $1}'
}

# 如果没有选择任何操作，调用 fatal_error 函数（未给出）
if! { ${REMOVE_INTERMEDIATE} || ${BUILD_SERVER} || ${BUILD_CLIENT}; }; then
  fatal_error "Nothing selected to be done."
fi

# 如果没有选择任何构建选项，调用 fatal_error 函数
if! { ${BUILD_OPTION_DUMMY} || ${BUILD_OPTION_DEBUG} || ${BUILD_OPTION_RELEASE} ; }; then
  fatal_error "Choose --debug and/or --release."
fi

# ==============================================================================
# -- Clean intermediate files --------------------------------------------------
# ==============================================================================

# 如果 REMOVE_INTERMEDIATE 为 true，清理中间文件和文件夹
if ${REMOVE_INTERMEDIATE} ; then
  # 打印日志信息
  log "Cleaning intermediate files and folders."
  # 删除服务器和客户端的构建、安装文件夹等
  rm -Rf ${LIBCARLA_BUILD_SERVER_FOLDER}* ${LIBCARLA_BUILD_CLIENT_FOLDER}*
  rm -Rf ${LIBCARLA_BUILD_PYTORCH_FOLDER}* ${LIBCARLA_BUILD_PYTORCH_FOLDER}*
  rm -Rf ${LIBCARLA_INSTALL_SERVER_FOLDER} ${LIBCARLA_INSTALL_CLIENT_FOLDER}
fi

# ==============================================================================
# -- Define build function -----------------------------------------------------
# ==============================================================================

# 定义 build_libcarla 函数，用于构建 LibCarla 库
function build_libcarla {
  # 存储额外的 CMake 选项
  CMAKE_EXTRA_OPTIONS=''

  # 根据不同的构建配置设置工具链文件、构建文件夹和安装文件夹
  if [ $1 == Server ] ; then
    M_TOOLCHAIN=${LIBCPP_TOOLCHAIN_FILE}
    M_BUILD_FOLDER=${LIBCARLA_BUILD_SERVER_FOLDER}.$(echo "$2" | tr '[:upper:]' '[:lower:]')
    M_INSTALL_FOLDER=${LIBCARLA_INSTALL_SERVER_FOLDER}
  elif [ $1 == Client ] ; then
    M_TOOLCHAIN=${LIBSTDCPP_TOOLCHAIN_FILE}
    M_BUILD_FOLDER=${LIBCARLA_BUILD_CLIENT_FOLDER}.$(echo "$2" | tr '[:upper:]' '[:lower:]')
    M_INSTALL_FOLDER=${LIBCARLA_INSTALL_CLIENT_FOLDER}
  elif [ $1 == Pytorch ] ; then
    M_TOOLCHAIN=${LIBSTDCPP_TOOLCHAIN_FILE}
    M_BUILD_FOLDER=${LIBCARLA_BUILD_PYTORCH_FOLDER}.$(echo "$2" | tr '[:upper:]' '[:lower:]')
    M_INSTALL_FOLDER=${LIBCARLA_INSTALL_SERVER_FOLDER}
  elif [ $1 == ros2 ] ; then
    M_TOOLCHAIN=${LIBSTDCPP_TOOLCHAIN_FILE}
    M_BUILD_FOLDER=${LIBCARLA_FASTDDS_FOLDER}.$(echo "$2" | tr '[:upper:]' '[:lower:]')
    M_INSTALL_FOLDER=${LIBCARLA_INSTALL_SERVER_FOLDER}
  elif [ $1 == ClientRSS ] ; then
    BUILD_TYPE='Client'
    M_TOOLCHAIN=${LIBSTDCPP_TOOLCHAIN_FILE}
    M_BUILD_FOLDER=${LIBCARLA_BUILD_CLIENT_FOLDER}.rss.$(echo "$2" | tr '[:upper:]' '[:lower:]')
    M_INSTALL_FOLDER=${LIBCARLA_INSTALL_CLIENT_FOLDER}
    CMAKE_EXTRA_OPTIONS="${CMAKE_EXTRA_OPTIONS:+${CMAKE_EXTRA_OPTIONS} }-DBUILD_RSS_VARIANT=ON -DADRSS_INSTALL_DIR=${CARLA_BUILD_FOLDER}/ad-rss-4.4.4/install"
  else
    fatal_error "Invalid build configuration \"$1\""
  fi

  # 根据不同的构建选项设置调试和发布标志
  if [ $2 == Debug ] ; then
    M_DEBUG=ON
    M_RELEASE=OFF
  elif [ $2 == Release ] ; then
    M_DEBUG=OFF
    M_RELEASE=ON
  else
    fatal_error "Invalid build option \"$2\""
  fi

  # 打印构建信息
  log "Building LibCarla \"$1.$2\" configuration."

  # 创建构建目录
  mkdir -p ${M_BUILD_FOLDER}
  # 进入构建目录
  pushd "${M_BUILD_FOLDER}" >/dev/null

  CHECKSUM_FILE=checksum.txt

  # 检查是否需要重新运行 CMake
  if [! -f "${CHECKSUM_FILE}" ] ; then
    NEEDS_CMAKE=true
  elif [ "$(cat ${CHECKSUM_FILE})"!= "$(get_source_code_checksum)" ] ; then
    log "Re-running cmake, some files were added or removed."
    NEEDS_CMAKE=true
  else
    NEEDS_CMAKE=false
  fi

  # 如果需要运行 CMake
  if ${NEEDS_CMAKE} ; then
    # 运行 CMake 进行配置
    cmake \
        -G "Eclipse CDT4 - Ninja" \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE:-$1} \
        -DLIBCARLA_BUILD_DEBUG=${M_DEBUG} \
        -DLIBCARLA_BUILD_RELEASE=${M_RELEASE} \
        -DCMAKE_TOOLCHAIN_FILE=${M_TOOLCHAIN} \
        -DCMAKE_INSTALL_PREFIX=${M_INSTALL_FOLDER} \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
        ${CMAKE_EXTRA_OPTIONS} \
        ${CARLA_ROOT_FOLDER}
    # 计算并存储源代码的校验和
    get_source_code_checksum > ${CHECKSUM_FILE}
  fi

  # 使用 Ninja 进行构建
  ninja
  # 安装构建结果，并过滤掉 "Up-to-date:" 信息
  ninja install | grep -v "Up-to-date:"

  # 回到上一个目录
  popd >/dev/null
}

# ==============================================================================
# -- Build all possible configurations -----------------------------------------
# ==============================================================================

# 如果选择了服务器构建且是调试模式，调用 build_libcarla 函数进行服务器调试构建
if { ${BUILD_SERVER} && ${BUILD_OPTION_DEBUG}; }; then
  build_libcarla Server Debug
fi

# 如果选择了服务器构建且是发布模式，调用 build_libcarla 函数进行服务器发布构建，并根据情况构建 PyTorch 和 ROS2 部分
if { ${BUILD_SERVER} && ${BUILD_OPTION_RELEASE}; }; then
  build_libcarla Server Release
  if ${USE_PYTORCH} ; then
    build_libcarla Pytorch Release
  fi
  if ${USE_ROS2} ; then
    build_libcarla ros2 Release
  fi
fi

# 客户端的构建变种，根据 BUILD_RSS_VARIANT 标志确定
CLIENT_VARIANT='Client'
if [ $BUILD_RSS_VARIANT == true ] ; then
  CLIENT_VARIANT='ClientRSS'
fi

# 如果选择了客户端构建且是调试模式，调用 build_libcarla 函数进行客户端调试构建
if { ${BUILD_CLIENT} && ${BUILD_OPTION_DEBUG}; }; then
  build_libcarla ${CLIENT_VARIANT} Debug
fi

# 如果选择了客户端构建且是发布模式，调用 build_libcarla 函数进行客户端发布构建
if { ${BUILD_CLIENT} && ${BUILD_OPTION_RELEASE}; }; then
  build_libcarla ${CLIENT_VARIANT} Release
fi

# ==============================================================================
# --...and we are done --------------------------------------------------------
# ==============================================================================

# 打印成功信息
log "Success!"
