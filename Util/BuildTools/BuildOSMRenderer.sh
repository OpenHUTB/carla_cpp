#! /bin/bash
# 导入当前脚本所在目录下的 Environment.sh 文件中的环境变量
source $(dirname "$0")/Environment.sh
# 定义 libosmscout 和 lunasvg 的 GitHub 仓库地址
LIBOSMSCOUT_REPO=https://github.com/Framstag/libosmscout
LUNASVG_REPO=https://github.com/sammycage/lunasvg
# 定义 libosmscout 的源代码和构建文件夹路径
LIBOSMSCOUT_SOURCE_FOLDER=${CARLA_BUILD_FOLDER}/libosmscout-source
LIBOSMSCOUT_BUILD_FOLDER=${CARLA_BUILD_FOLDER}/libosmscout-build
LIBOSMSCOUT_COMMIT=e83e4881a4adc69c5a4bcc05de5e1f23ebf06238
# 定义 lunasvg 的源代码和构建文件夹路径
LUNASVG_SOURCE_FOLDER=${CARLA_BUILD_FOLDER}/lunasvg-source
LUNASVG_BUILD_FOLDER=${CARLA_BUILD_FOLDER}/lunasvg-build
# 定义 osm-world-renderer 的源代码和构建文件夹路径
OSM_RENDERER_SOURCE=${CARLA_ROOT_FOLDER}/osm-world-renderer
OSM_RENDERER_BUILD=${CARLA_BUILD_FOLDER}/osm-world-renderer-build
# 定义第三方库的安装路径
INSTALLATION_PATH=${OSM_RENDERER_SOURCE}/ThirdParties



# ==============================================================================
# -- Download and build libosmscout --------------------------------------------
# ==============================================================================
echo "Cloning libosmscout."
echo ${CARLA_BUILD_FOLDER}
# 如果 libosmscout 的源代码文件夹不存在，则从 GitHub 克隆
if [ ! -d ${LIBOSMSCOUT_SOURCE_FOLDER} ] ; then
  git clone ${LIBOSMSCOUT_REPO} ${LIBOSMSCOUT_SOURCE_FOLDER}
fi
# 进入 libosmscout 的源代码文件夹
cd ${LIBOSMSCOUT_SOURCE_FOLDER}
# 拉取最新的代码
git fetch
# 检出特定的提交版本
git checkout ${LIBOSMSCOUT_COMMIT}
# 返回上级目录
cd ..
# 创建并进入 libosmscout 的构建文件夹
mkdir -p ${LIBOSMSCOUT_BUILD_FOLDER}
cd ${LIBOSMSCOUT_BUILD_FOLDER}
# 使用 CMake 配置 libosmscout 项目，并指定安装路径
cmake ${LIBOSMSCOUT_SOURCE_FOLDER} \
    -DCMAKE_INSTALL_PREFIX=${INSTALLATION_PATH}
# 编译和安装 libosmscout
make
make install

# ==============================================================================
# -- Download and build lunasvg ------------------------------------------------
# ==============================================================================
echo "Cloning luna-svg"
# 如果 lunasvg 的源代码文件夹不存在，则从 GitHub 克隆
if [ ! -d ${LUNASVG_SOURCE_FOLDER} ] ; then
  git clone ${LUNASVG_REPO} ${LUNASVG_SOURCE_FOLDER}
fi
# 创建并进入 lunasvg 的构建文件夹
mkdir -p ${LUNASVG_BUILD_FOLDER}
cd ${LUNASVG_BUILD_FOLDER}
# 使用 CMake 配置 lunasvg 项目，并指定安装路径
cmake ${LUNASVG_SOURCE_FOLDER} \
    -DCMAKE_INSTALL_PREFIX=${INSTALLATION_PATH}
# 编译和安装 lunasvg
make
make install


# ==============================================================================
# -- Build osm-map-renderer tool -----------------------------------------------
# ==============================================================================
echo "Building osm-map-renderer"
# 创建并进入 osm-map-renderer 的构建文件夹
mkdir -p ${OSM_RENDERER_BUILD}
cd ${OSM_RENDERER_BUILD}
# 使用 CMake 配置 osm-map-renderer 项目，设置 C++ 编译标志，并指定源代码路径
cmake -DCMAKE_CXX_FLAGS="-std=c++17 -g -pthread -I${CARLA_BUILD_FOLDER}/boost-1.80.0-c10-install/include" \
    ${OSM_RENDERER_SOURCE}# 编译 osm-map-renderer
make

echo "SUCCESS! Finishing setting up renderer."
