#! /bin/bash
# 这是一个Bash脚本，用于构建和打包CARLA Python API。

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Build and package CARLA Python API."# 脚本的文档字符串，描述脚本的功能

USAGE_STRING="Usage: $0 [-h|--help] [--rebuild] [--clean] [--python-version=VERSION] [--target-wheel-platform=PLATFORM]"# 脚本的使用说明

REMOVE_INTERMEDIATE=false# 是否移除中间文件的标志
BUILD_RSS_VARIANT=false# 是否构建RSS变体的标志
BUILD_PYTHONAPI=true# 是否构建Python API的标志

# 使用getopt解析命令行参数
OPTS=`getopt -o h --long help,config:,rebuild,clean,rss,carsim,python-version:,target-wheel-platform:,packages:,clean-intermediate,all,xml,target-archive:, -n 'parse-options' -- "$@"`

eval set -- "$OPTS"# 重新设置参数列表

PY_VERSION_LIST=3# Python版本列表
TARGET_WHEEL_PLATFORM=# 目标wheel平台

# 遍历解析后的参数列表
while [[ $# -gt 0 ]]; do
  case "$1" in
    --rebuild )
      REMOVE_INTERMEDIATE=true;# 设置移除中间文件标志为true
      BUILD_PYTHONAPI=true;# 设置构建Python API标志为true
      shift ;;
    --python-version )
      PY_VERSION_LIST="$2"# 设置Python版本列表
      shift 2 ;;
    --target-wheel-platform )
      TARGET_WHEEL_PLATFORM="$2"# 设置目标wheel平台
      shift 2 ;;
    --rss )
      BUILD_RSS_VARIANT=true;# 设置构建RSS变体标志为true
      shift ;;
    --clean )
      REMOVE_INTERMEDIATE=true;# 设置移除中间文件标志为true
      BUILD_PYTHONAPI=false;# 设置构建Python API标志为false
      shift ;;
    -h | --help )
      echo "$DOC_STRING"# 打印文档字符串
      echo "$USAGE_STRING"# 打印使用说明
      exit 1# 退出脚本
      ;;
    * )
      shift ;;
  esac
done

# 设置编译器和路径
export CC="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang"
export CXX="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang++"
export PATH="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin:$PATH"

# 导入环境设置脚本
source $(dirname "$0")/Environment.sh

# 如果没有选择任何操作，则报错退出
if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_PYTHONAPI} ; }; then
  fatal_error "Nothing selected to be done."
fi

# 将逗号分隔的字符串转换为数组
# Convert comma-separated string to array of unique elements.
IFS="," read -r -a PY_VERSION_LIST <<< "${PY_VERSION_LIST}"

# 进入CARLA Python API源代码文件夹
pushd "${CARLA_PYTHONAPI_SOURCE_FOLDER}" >/dev/null

# ==============================================================================
# -- Clean intermediate files --------------------------------------------------
# ==============================================================================

# 如果REMOVE_INTERMEDIATE为true，清理中间文件和文件夹
if ${REMOVE_INTERMEDIATE} ; then

  log "Cleaning intermediate files and folders."

  rm -Rf build dist source/carla.egg-info

  find source -name "*.so" -delete
  find source -name "__pycache__" -type d -exec rm -r "{}" \;

fi

# ==============================================================================
# -- Build API -----------------------------------------------------------------
# ==============================================================================

# 如果BUILD_RSS_VARIANT为true，设置环境变量
if ${BUILD_RSS_VARIANT} ; then
  export BUILD_RSS_VARIANT=${BUILD_RSS_VARIANT}
fi

# 如果BUILD_PYTHONAPI为true，构建Python API
if ${BUILD_PYTHONAPI} ; then
  # 添加patchelf到路径，auditwheel依赖patchelf修复ELF文件
  export PATH="${LIBCARLA_INSTALL_CLIENT_FOLDER}/bin:${PATH}"
  # 获取操作系统版本代码名称
  CODENAME=$(cat /etc/os-release | grep VERSION_CODENAME)
  # 如果指定了目标平台但不是兼容的Linux发行版，则跳过wheel修复步骤
  if [[ ! -z ${TARGET_WHEEL_PLATFORM} ]] && [[ ${CODENAME#*=} != "bionic" ]] ; then
    log "A target platform has been specified but you are not using a compatible linux distribution. The wheel repair step will be skipped"
    TARGET_WHEEL_PLATFORM=
  fi

  # 遍历Python版本列表，构建Python API
  for PY_VERSION in ${PY_VERSION_LIST[@]} ; do
    log "Building Python API for Python ${PY_VERSION}."

    if [[ -z ${TARGET_WHEEL_PLATFORM} ]] ; then
      /usr/bin/env python${PY_VERSION} setup.py bdist_egg bdist_wheel --dist-dir dist/.tmp
      cp dist/.tmp/$(ls dist/.tmp | grep .whl) dist
    else
      /usr/bin/env python${PY_VERSION} setup.py bdist_egg bdist_wheel --dist-dir dist/.tmp --plat ${TARGET_WHEEL_PLATFORM}
      /usr/bin/env python${PY_VERSION} -m auditwheel repair --plat ${TARGET_WHEEL_PLATFORM} --wheel-dir dist dist/.tmp/$(ls dist/.tmp | grep .whl)
    fi

    rm -rf dist/.tmp

  done

fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

# 返回到原始目录
popd >/dev/null
# 打印成功信息
log "Success!"
