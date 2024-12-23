#! /bin/bash
# 这是一个Bash脚本，用于构建OSM渲染器所需的库和工具。

# 导入环境设置脚本
source $(dirname "$0")/Environment.sh

# 定义libosmscout和lunasvg的GitHub仓库地址
LIBOSMSCOUT_REPO=https://github.com/Framstag/libosmscout
LUNASVG_REPO=https://github.com/sammycage/lunasvg

# 定义libosmscout的源代码和构建文件夹路径
LIBOSMSCOUT_SOURCE_FOLDER=${CARLA_BUILD_FOLDER}/libosmscout-source
LIBOSMSCOUT_BUILD_FOLDER=${CARLA_BUILD_FOLDER}/libosmscout-build
LIBOSMSCOUT_COMMIT=e83e4881a4adc69c5a4bcc05de5e1f23ebf06238

# 定义lunasvg的源代码和构建文件夹路径
LUNASVG_SOURCE_FOLDER=${CARLA_BUILD_FOLDER}/lunasvg-source
LUNASVG_BUILD_FOLDER=${CARLA_BUILD_FOLDER}/lunasvg-build

# 定义OSM渲染器的源代码和构建文件夹路径
OSM_RENDERER_SOURCE=${CARLA_ROOT_FOLDER}/osm-world-renderer
OSM_RENDERER_BUILD=${CARLA_BUILD_FOLDER}/osm-world-renderer-build

# 定义安装路径
INSTALLATION_PATH=${OSM_RENDERER_SOURCE}/ThirdParties

# ==============================================================================
# -- Download and build libosmscout --------------------------------------------
# ==============================================================================
echo "Cloning libosmscout."
echo ${CARLA_BUILD_FOLDER}
# 如果libosmscout的源代码文件夹不存在，则从GitHub克隆
if [ ! -d ${LIBOSMSCOUT_SOURCE_FOLDER} ] ; then
  git clone ${LIBOSMSCOUT_REPO} ${LIBOSMSCOUT_SOURCE_FOLDER}
fi

# 进入libosmscout的源代码文件夹并检查出指定的commit
cd ${LIBOSMSCOUT_SOURCE_FOLDER}
git fetch
git checkout ${LIBOSMSCOUT_COMMIT}
cd ..

# 创建并进入libosmscout的构建文件夹
mkdir -p ${LIBOSMSCOUT_BUILD_FOLDER}
cd ${LIBOSMSCOUT_BUILD_FOLDER}

# 使用CMake配置libosmscout项目并构建安装
cmake ${LIBOSMSCOUT_SOURCE_FOLDER} \
    -DCMAKE_INSTALL_PREFIX=${INSTALLATION_PATH}

make
make install

# ==============================================================================
# -- Download and build lunasvg ------------------------------------------------
# ==============================================================================
echo "Cloning luna-svg"
# 如果lunasvg的源代码文件夹不存在，则从GitHub克隆
if [ ! -d ${LUNASVG_SOURCE_FOLDER} ] ; then
  git clone ${LUNASVG_REPO} ${LUNASVG_SOURCE_FOLDER}
fi

# 创建并进入lunasvg的构建文件夹
mkdir -p ${LUNASVG_BUILD_FOLDER}
cd ${LUNASVG_BUILD_FOLDER}

# 使用CMake配置lunasvg项目并构建安装
cmake ${LUNASVG_SOURCE_FOLDER} \
    -DCMAKE_INSTALL_PREFIX=${INSTALLATION_PATH}

make
make install


# ==============================================================================
# -- Build osm-map-renderer tool -----------------------------------------------
# ==============================================================================
echo "Building osm-map-renderer"

# 创建并进入osm-map-renderer的构建文件夹
mkdir -p ${OSM_RENDERER_BUILD}
cd ${OSM_RENDERER_BUILD}

# 使用CMake配置osm-map-renderer项目并构建
cmake -DCMAKE_CXX_FLAGS="-std=c++17 -g -pthread -I${CARLA_BUILD_FOLDER}/boost-1.80.0-c10-install/include" \
    ${OSM_RENDERER_SOURCE}
make

echo "SUCCESS! Finishing setting up renderer."# 打印成功信息，表示渲染器设置完成
