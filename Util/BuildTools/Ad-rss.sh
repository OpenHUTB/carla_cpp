#!/bin/bash

# ==============================================================================
# -- 配置和参数解析 ----------------------------------------------------------
# ==============================================================================

# 命令行参数处理
OPTS=$(getopt -o h --long python-version: -n 'parse-options' -- "$@")
eval set -- "$OPTS"

# 默认配置
PY_VERSION_LIST=3  # 默认Python版本
CXX_TAG=c10       # C++编译器标签

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case "$1" in
        --python-version )
            PY_VERSION_LIST="$2"
            shift 2 
            ;;
        * )
            shift 
            ;;
    esac
done

# ==============================================================================
# -- 环境设置 ---------------------------------------------------------------
# ==============================================================================

# 载入环境配置
source $(dirname "$0")/Environment.sh

# 将逗号分隔的Python版本列表转换为数组
IFS="," read -r -a PY_VERSION_LIST <<< "${PY_VERSION_LIST}"

# ==============================================================================
# -- AD-RSS配置 ------------------------------------------------------------
# ==============================================================================

# 版本和目录配置
ADRSS_VERSION=4.4.4
ADRSS_BASENAME="ad-rss-${ADRSS_VERSION}"
ADRSS_COLCON_WORKSPACE="${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}"
ADRSS_SRC_DIR="${ADRSS_COLCON_WORKSPACE}/src"
ADRSS_INSTALL_DIR="${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/install"

# ==============================================================================
# -- 源代码获取和准备 -------------------------------------------------------
# ==============================================================================

function prepare_source_code() {
    log "Retrieving ${ADRSS_BASENAME}."
    mkdir -p "${ADRSS_SRC_DIR}"

    pushd "${ADRSS_SRC_DIR}" >/dev/null
        # 克隆和配置仓库
        git clone -b v${ADRSS_VERSION} https://github.com/intel/ad-rss-lib.git && \
        cd ad-rss-lib && \
        git submodule update --init --recursive && \
        rm -rf dependencies/map/dependencies/PROJ4 && \
        cd ..

        # 更新代码以支持新版Boost
        grep -rl "boost::array" | xargs sed -i 's/boost::array/std::array/g'
        grep -rl "find_package(Boost" | xargs sed -i 's/find_package(Boost/find_package(Boost 1.80/g'
    popd >/dev/null

    # 创建colcon配置文件
    create_colcon_meta
}

# ==============================================================================
# -- Colcon配置生成 --------------------------------------------------------
# ==============================================================================

function create_colcon_meta() {
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
}

# ==============================================================================
# -- 构建函数 --------------------------------------------------------------
# ==============================================================================

function build_adrss() {
    local PY_VERSION=$1
    local ADRSS_BUILD_DIR="${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/build-python${PY_VERSION}"

    # 检查是否已经构建
    if [[ -d "${ADRSS_INSTALL_DIR}" && -d "${ADRSS_BUILD_DIR}" ]]; then
        log "${ADRSS_BASENAME} for python${PY_VERSION} already installed."
        return 0
    fi

    log "Building ${ADRSS_BASENAME} for python${PY_VERSION}"

    pushd "${ADRSS_COLCON_WORKSPACE}" >/dev/null
        # 设置CMAKE路径
        setup_cmake_prefix_path

        # 获取Python版本信息
        local PYTHON_VERSION=$(/usr/bin/env python${PY_VERSION} -V 2>&1)
        local PYTHON_BINDING_VERSIONS=${PYTHON_VERSION:7}
        PYTHON_BINDING_VERSIONS=${PYTHON_BINDING_VERSIONS%.*}
        echo "PYTHON_BINDING_VERSIONS=${PYTHON_BINDING_VERSIONS}"

        # 执行构建
        if ! run_colcon_build "${ADRSS_BUILD_DIR}" "${PYTHON_BINDING_VERSIONS}"; then
            rm -rf "${ADRSS_INSTALL_DIR}"
            log "Failed!"
            return 1
        fi

        log "Success!"
    popd >/dev/null
}

# ==============================================================================
# -- 辅助函数 --------------------------------------------------------------
# ==============================================================================

function setup_cmake_prefix_path() {
    local boost_path="${CARLA_BUILD_FOLDER}/boost-1.80.0-$CXX_TAG-install"
    local proj_path="${CARLA_BUILD_FOLDER}/proj-install"
    
    if [[ -z "${CMAKE_PREFIX_PATH}" ]]; then
        CMAKE_PREFIX_PATH="${boost_path};${proj_path}"
    else
        CMAKE_PREFIX_PATH="${boost_path};${proj_path};${CMAKE_PREFIX_PATH}"
    fi
}

function run_colcon_build() {
    local build_dir=$1
    local python_version=$2

    colcon build \
        --executor sequential \
        --packages-up-to ad_rss_map_integration \
        --cmake-args \
            -DCMAKE_BUILD_TYPE=RelWithDebInfo \
            -DCMAKE_TOOLCHAIN_FILE="${CARLA_BUILD_FOLDER}/LibStdCppToolChain.cmake" \
            -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}" \
            -DPYTHON_BINDING_VERSIONS="${python_version}" \
        --build-base ${build_dir} \
        --install-base ${ADRSS_INSTALL_DIR}
    
    return $?
}

# ==============================================================================
# -- 主程序 ----------------------------------------------------------------
# ==============================================================================

# 准备源代码
[[ ! -d "${ADRSS_SRC_DIR}" ]] && prepare_source_code

# 为每个Python版本执行构建
for PY_VERSION in ${PY_VERSION_LIST[@]}; do
    build_adrss ${PY_VERSION}
done
