#! /bin/bash# 这是一个Bash脚本，用于构建OSM2ODR库，该库可以将OSM（开放街道地图）数据转换为OpenDRIVE格式。
DOC_STRING="Build OSM2ODR."# 脚本的文档字符串，描述脚本的功能

USAGE_STRING=$(cat <<- END
Usage: $0 [-h|--help]

commands

    [--clean]    Clean intermediate files.
    [--rebuild]  Clean and rebuild both configurations.
END
)# 帮助信息，描述了脚本的使用方式

REMOVE_INTERMEDIATE=false# 是否移除中间文件的标志
BUILD_OSM2ODR=false# 是否构建OSM2ODR的标志
GIT_PULL=true# 是否从GitHub拉取代码的标志
CURRENT_OSM2ODR_COMMIT=1835e1e9538d0778971acc8b19b111834aae7261# OSM2ODR的当前commit哈希
OSM2ODR_BRANCH=aaron/defaultsidewalkwidth# OSM2ODR的分支名
OSM2ODR_REPO=https://github.com/carla-simulator/sumo.git# OSM2ODR的GitHub仓库地址
# 使用getopt解析命令行参数
OPTS=`getopt -o h --long help,rebuild,build,clean,carsim,no-pull -n 'parse-options' -- "$@"`

eval set -- "$OPTS"
# 遍历解析后的参数列表
while [[ $# -gt 0 ]]; do
  case "$1" in
    --rebuild )
      REMOVE_INTERMEDIATE=true;
      BUILD_OSM2ODR=true;
      shift ;;
    --build )
      BUILD_OSM2ODR=true;
      shift ;;
    --no-pull )
      GIT_PULL=false
      shift ;;
    --clean )
      REMOVE_INTERMEDIATE=true;
      shift ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      shift ;;
  esac
done
# 导入环境设置脚本
source $(dirname "$0")/Environment.sh
# 定义函数get_source_code_checksum，用于计算源代码的校验和
function get_source_code_checksum {
  local EXCLUDE='*__pycache__*'
  find "${OSM2ODR_SOURCE_FOLDER}"/* \! -path "${EXCLUDE}" -print0 | sha1sum | awk '{print $1}'
}
# 如果没有选择任何操作，则报错退出
if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_OSM2ODR}; }; then
  fatal_error "Nothing selected to be done."
fi

# ==============================================================================
# -- Clean intermediate files --------------------------------------------------
# ==============================================================================
# 如果REMOVE_INTERMEDIATE为true，清理中间文件和文件夹
if ${REMOVE_INTERMEDIATE} ; then

  log "Cleaning intermediate files and folders."

  rm -Rf ${OSM2ODR_BUILD_FOLDER}*

fi

# ==============================================================================
# -- Build library -------------------------------------------------------------
# ==============================================================================
# 如果BUILD_OSM2ODR为true，构建OSM2ODR库
if ${BUILD_OSM2ODR} ; then
  log "Building OSM2ODR."# 如果OSM2ODR源代码文件夹不存在，则从GitHub下载并解压
  if [ ! -d ${OSM2ODR_SOURCE_FOLDER} ] ; then
    cd ${CARLA_BUILD_FOLDER}
    curl --retry 5 --retry-max-time 120 -L -o OSM2ODR.zip https://github.com/carla-simulator/sumo/archive/${CURRENT_OSM2ODR_COMMIT}.zip
    unzip -qq OSM2ODR.zip
    rm -f OSM2ODR.zip
    mv sumo-${CURRENT_OSM2ODR_COMMIT} ${OSM2ODR_SOURCE_FOLDER}
  fi
# 创建并进入构建文件夹
  mkdir -p ${OSM2ODR_BUILD_FOLDER}
  cd ${OSM2ODR_BUILD_FOLDER}

# 设置编译器和路径
  export CC="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang"
  export CXX="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang++"
  export PATH="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin:$PATH"
# 使用CMake配置项目并构建
  cmake ${OSM2ODR_SOURCE_FOLDER} \
      -G "Eclipse CDT4 - Ninja" \
      -DCMAKE_CXX_FLAGS="-stdlib=libstdc++" \
      -DCMAKE_INSTALL_PREFIX=${LIBCARLA_INSTALL_CLIENT_FOLDER} \
      -DPROJ_INCLUDE_DIR=${CARLA_BUILD_FOLDER}/proj-install/include \
      -DPROJ_LIBRARY=${CARLA_BUILD_FOLDER}/proj-install/lib/libproj.a \
      -DXercesC_INCLUDE_DIR=${CARLA_BUILD_FOLDER}/xerces-c-3.2.3-install/include \
      -DXercesC_LIBRARY=${CARLA_BUILD_FOLDER}/xerces-c-3.2.3-install/lib/libxerces-c.a

  ninja osm2odr
  ninja install
# 为服务器构建OSM2ODR
  mkdir -p ${OSM2ODR_SERVER_BUILD_FOLDER}
  cd ${OSM2ODR_SERVER_BUILD_FOLDER}
# 设置LLVM的路径
  LLVM_BASENAME=llvm-8.0
  LLVM_INCLUDE="$UE4_ROOT/Engine/Source/ThirdParty/Linux/LibCxx/include/c++/v1"
  LLVM_LIBPATH="$UE4_ROOT/Engine/Source/ThirdParty/Linux/LibCxx/lib/Linux/x86_64-unknown-linux-gnu"

  echo $LLVM_INCLUDE
  echo $LLVM_LIBPATH
# 使用CMake配置项目并构建服务器版本的OSM2ODR
  cmake ${OSM2ODR_SOURCE_FOLDER} \
      -G "Eclipse CDT4 - Ninja" \
      -DCMAKE_CXX_FLAGS="-fPIC -std=c++14 -stdlib=libc++ -I${LLVM_INCLUDE} -L${LLVM_LIBPATH}" \
      -DCMAKE_INSTALL_PREFIX=${LIBCARLA_INSTALL_SERVER_FOLDER} \
      -DPROJ_INCLUDE_DIR=${CARLA_BUILD_FOLDER}/proj-install-server/include \
      -DPROJ_LIBRARY=${CARLA_BUILD_FOLDER}/proj-install-server/lib/libproj.a \
      -DXercesC_INCLUDE_DIR=${CARLA_BUILD_FOLDER}/xerces-c-3.2.3-install-server/include \
      -DXercesC_LIBRARY=${CARLA_BUILD_FOLDER}/xerces-c-3.2.3-install-server/lib/libxerces-c.a

  ninja osm2odr
  ninja install

  mkdir -p ${OSM2ODR_SERVER_BUILD_FOLDER}
  cd ${OSM2ODR_SERVER_BUILD_FOLDER}

  LLVM_BASENAME=llvm-8.0
  LLVM_INCLUDE="$UE4_ROOT/Engine/Source/ThirdParty/Linux/LibCxx/include/c++/v1"
  LLVM_LIBPATH="$UE4_ROOT/Engine/Source/ThirdParty/Linux/LibCxx/lib/Linux/x86_64-unknown-linux-gnu"

  cmake ${OSM2ODR_SOURCE_FOLDER} \
      -G "Eclipse CDT4 - Ninja" \
      -DCMAKE_CXX_FLAGS="-fPIC -std=c++14 -stdlib=libc++ -I${LLVM_INCLUDE} -L${LLVM_LIBPATH}" \
      -DCMAKE_INSTALL_PREFIX=${LIBCARLA_INSTALL_SERVER_FOLDER} \
      -DPROJ_INCLUDE_DIR=${CARLA_BUILD_FOLDER}/proj-install-server/include \
      -DPROJ_LIBRARY=${CARLA_BUILD_FOLDER}/proj-install-server/lib/libproj.a \
      -DXercesC_INCLUDE_DIR=${CARLA_BUILD_FOLDER}/xerces-c-3.2.3-install-server/include \
      -DXercesC_LIBRARY=${CARLA_BUILD_FOLDER}/xerces-c-3.2.3-install-server/lib/libxerces-c.a

  ninja osm2odr
  ninja install

fi

log " OSM2ODR Success!"# 打印构建成功的日志信息
