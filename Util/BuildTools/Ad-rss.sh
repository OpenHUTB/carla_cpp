#! /bin/bash
# 使用 getopt 处理命令行参数  
# -o h: 短选项 h  
# --long python-version: 长选项 python-version  
# -n 'parse-options': 设置脚本名称，以便在错误信息中显示
OPTS=`getopt -o h --long python-version: -n 'parse-options' -- "$@"`
# 重新设定位置参数 
eval set -- "$OPTS"#使用eval命令来执行set命令
# 默认的 Python 版本列表  
PY_VERSION_LIST=3
# 解析命令行参数
while [[ $# -gt 0 ]]; do
  case "$1" in
    --python-version )
      PY_VERSION_LIST="$2"; # 如果指定了 python-version，更新 PY_VERSION_LIST
      shift 2 ;;            # 移动到下一个参数 
    * )
      shift ;;              # 移动到下一个参数
  esac
done

# ==============================================================================
# -- 设置环境 --------------------------------------------------------------
# ==============================================================================
# 载入环境配置 
source $(dirname "$0")/Environment.sh

# 将以逗号分隔的字符串转换为唯一元素数组
IFS="," read -r -a PY_VERSION_LIST <<< "${PY_VERSION_LIST}"

# ==============================================================================
# -- 获取 ad-rss -------------------------------------------
# ==============================================================================
# 定义 ad-rss 的版本和相关目录
ADRSS_VERSION=4.4.4
ADRSS_BASENAME=ad-rss-${ADRSS_VERSION}
ADRSS_COLCON_WORKSPACE="${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}"
ADRSS_SRC_DIR="${ADRSS_COLCON_WORKSPACE}/src"
# 检查源代码目录是否存在
if [[ ! -d "${ADRSS_SRC_DIR}" ]]; then
  # 如果目录不存在，首先设置工作区
  log "Retrieving ${ADRSS_BASENAME}."

  mkdir -p "${ADRSS_SRC_DIR}"  # 创建源目录

 # 从 GitHub 克隆 ad-rss 仓库，初始化所有子模块，并删除 proj 依赖项
  pushd "${ADRSS_SRC_DIR}" >/dev/null
  git clone -b v${ADRSS_VERSION} https://github.com/intel/ad-rss-lib.git && cd ad-rss-lib && git submodule update --init --recursive && rm -rf dependencies/map/dependencies/PROJ4 && cd ..

 # 更新代码以支持更新的 Boost 库  
  grep -rl "boost::array" | xargs sed -i 's/boost::array/std::array/g'  # 替换 boost::array 为 std::array
  grep -rl "find_package(Boost" | xargs sed -i 's/find_package(Boost/find_package(Boost 1.80/g'
  popd
 # 创建 colcon.meta 文件，定义要构建的包和参数
  cat >"${ADRSS_COLCON_WORKSPACE}/colcon.meta" <<EOL
{
    "names": {
        "ad_physics": {
            "cmake-args": ["-DBUILD_PYTHON_BINDING=ON", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF", "-DDISABLE_WARNINGS_AS_ERRORS=ON"]
        },
        "ad_map_access": {
            "cmake-args": ["-DBUILD_PYTHON_BINDING=ON", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF", "-DDISABLE_WARNINGS_AS_ERRORS=ON"]
        },
        "ad_map_opendrive_reader": {
            "cmake-args": ["-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF", "-DDISABLE_WARNINGS_AS_ERRORS=ON"],
            "dependencies": ["odrSpiral"]
        },
        "ad_rss": {
            "cmake-args": ["-DBUILD_PYTHON_BINDING=ON", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF", "-DDISABLE_WARNINGS_AS_ERRORS=ON"]
        },
        "ad_rss_map_integration": {
            "cmake-args": ["-DBUILD_PYTHON_BINDING=ON", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF", "-DDISABLE_WARNINGS_AS_ERRORS=ON"]
        },
        "spdlog": {
            "cmake-args": ["-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF"]
        }
    }
}

EOL
fi

# ==============================================================================
# -- 构建 ad-rss -------------------------------------------
# ==============================================================================
ADRSS_INSTALL_DIR="${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/install" # 安装目录

# 如果系统中安装的 clang 版本与 Unreal 提供的版本不匹配，自动化过程将无法正常工作。  
# Setup.sh 脚本需要进行相应的调整。  
# 因此，我们需要更改：  
# CARLA_LLVM_VERSION_MAJOR=$(cut -d'.' -f1 <<<"$(clang --version | head -n 1 | sed -r 's/^([^.]+).*$/\1/; s/^[^0-9]*([0-9]+).*$/\1/')")  
# 如果未能获取到 CARLA_LLVM_VERSION_MAJOR， 则：  
#   fatal_error "未能检索到安装的 clang 编译器版本。"  
# 否则，输出使用的 clang 编译器版本：  
#   echo "正在使用 clang-$CARLA_LLVM_VERSION_MAJOR 作为 CARLA 编译器。"
CXX_TAG=c10

#
# 由于 boost-python 不支持同时构建多个 Python 版本（find_package 存在一些 bug），  
# 我们必须为每个版本单独进行 colcon 构建。
#
for PY_VERSION in ${PY_VERSION_LIST[@]} ; do#每次循环将一个python版本赋值给PY_VERSION变量
  ADRSS_BUILD_DIR="${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/build-python${PY_VERSION}" # 为当前 Python 版本设置构建目录
# 如果安装目录和构建目录都已存在，则跳过构建
  if [[ -d "${ADRSS_INSTALL_DIR}" && -d "${ADRSS_BUILD_DIR}" ]]; then
    log "${ADRSS_BASENAME} for python${PY_VERSION} already installed."
  else
    log "Building ${ADRSS_BASENAME} for python${PY_VERSION}"#log函数会输出一条消息，表示正在构建针对python${PY_VERSION}的${ADRSS_BASENAME}

    pushd "${ADRSS_COLCON_WORKSPACE}" >/dev/null # 切换到工作区 
      # 设置 CMake 前缀路径
    if [[ "${CMAKE_PREFIX_PATH}" == "" ]]; then
      CMAKE_PREFIX_PATH="${CARLA_BUILD_FOLDER}/boost-1.80.0-$CXX_TAG-install;${CARLA_BUILD_FOLDER}/proj-install"
    else
      CMAKE_PREFIX_PATH="${CARLA_BUILD_FOLDER}/boost-1.80.0-$CXX_TAG-install;${CARLA_BUILD_FOLDER}/proj-install;${CMAKE_PREFIX_PATH}"
    fi

    # 获取要构建的绑定的 Python 版本，需要查询二进制文件，  
    # 因为可能仅提供了 '3' 作为 PY_VERSION，这样就会调用符号链接的 python3。
    PYTHON_VERSION=$(/usr/bin/env python${PY_VERSION} -V 2>&1)
    PYTHON_BINDING_VERSIONS=${PYTHON_VERSION:7} # 提取 Python 版本信息
    PYTHON_BINDING_VERSIONS=${PYTHON_BINDING_VERSIONS%.*}  # 只保留主版本号
    echo "PYTHON_BINDING_VERSIONS=${PYTHON_BINDING_VERSIONS}"

    # 使用顺序执行器以减少编译所需的内存
    colcon build --executor sequential --packages-up-to ad_rss_map_integration --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE="${CARLA_BUILD_FOLDER}/LibStdCppToolChain.cmake" -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}" -DPYTHON_BINDING_VERSIONS="${PYTHON_BINDING_VERSIONS}" --build-base ${ADRSS_BUILD_DIR} --install-base ${ADRSS_INSTALL_DIR}

    COLCON_RESULT=$? # 获取构建结果
    if (( COLCON_RESULT )); then
      rm -rf "${ADRSS_INSTALL_DIR}"  # 构建失败时，删除安装目录  
      log "Failed !"
    else
      log "Success!"
    fi
    popd >/dev/null  # 返回上级目录
  fi
done
