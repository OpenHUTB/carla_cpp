#! /bin/bash# 这是一个Bash脚本，用于下载和构建Unreal Engine 4的StreetMap插件。
DOC_STRING="Download StreetMapUE4 Plugin."# 脚本的文档字符串，描述脚本的功能

USAGE_STRING=$(cat <<- END
Usage: $0 [-h|--help]

commands

    [--clean]    Clean intermediate files.
    [--rebuild]  Clean and rebuild both configurations.
END
)# 脚本的使用说明

REMOVE_INTERMEDIATE=false# 是否移除中间文件的标志
BUILD_STREETMAP=false# 是否构建StreetMap插件的标志
GIT_PULL=true# 是否从GitHub拉取代码的标志
CURRENT_STREETMAP_COMMIT=260273d6b7c3f28988cda31fd33441de7e272958# StreetMap的当前commit哈希
STREETMAP_BRANCH=master# StreetMap的分支名
STREETMAP_REPO=https://github.com/carla-simulator/StreetMap.git# StreetMap的GitHub仓库地址

# 使用getopt解析命令行参数
OPTS=`getopt -o h --long build,rebuild,clean, -n 'parse-options' -- "$@"`

eval set -- "$OPTS"# 重新设置参数列表

# 遍历解析后的参数列表
while [[ $# -gt 0 ]]; do
  case "$1" in
    --rebuild )
      REMOVE_INTERMEDIATE=true;# 设置移除中间文件标志为true
      BUILD_STREETMAP=true;# 设置构建StreetMap插件标志为true
      shift ;;
    --build )
      BUILD_STREETMAP=true;# 设置构建StreetMap插件标志为true
      shift ;;
    --no-pull )
      GIT_PULL=false# 设置不从GitHub拉取代码
      shift ;;
    --clean )
      REMOVE_INTERMEDIATE=true;# 设置移除中间文件标志为true
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

# 导入环境设置脚本
source $(dirname "$0")/Environment.sh

# 如果没有选择任何操作，则报错退出
if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_STREETMAP}; }; then
  fatal_error "Nothing selected to be done."
fi

# ==============================================================================
# -- Clean intermediate files --------------------------------------------------
# ==============================================================================

# 如果REMOVE_INTERMEDIATE为true，清理中间文件和文件夹
if ${REMOVE_INTERMEDIATE} ; then

  log "Cleaning intermediate files and folders."

  UE4_INTERMEDIATE_FOLDERS="Binaries Build Intermediate DerivedDataCache"

  pushd "${CARLAUE4_STREETMAP_FOLDER}" >/dev/null

  rm -Rf ${UE4_INTERMEDIATE_FOLDERS}

  popd >/dev/null

fi

# ==============================================================================
# -- Build library -------------------------------------------------------------
# ==============================================================================

# 如果BUILD_STREETMAP为true，下载StreetMap插件
if ${BUILD_STREETMAP} ; then
  log "Downloading STREETMAP plugin."
  if ${GIT_PULL} ; then
    if [ ! -d ${CARLAUE4_STREETMAP_FOLDER} ] ; then
      git clone -b ${STREETMAP_BRANCH} ${STREETMAP_REPO} ${CARLAUE4_STREETMAP_FOLDER}# 克隆仓库
    fi
    cd ${CARLAUE4_STREETMAP_FOLDER}
    git fetch# 获取最新代码
    git checkout ${CURRENT_STREETMAP_COMMIT}# 检出指定的commit
  fi
fi

log "StreetMap Success!"# 打印成功信息
