#! /bin/bash# 这是一个Bash脚本，用于在Docker环境中构建和安装FBX2OBJ工具。

# 导入环境设置脚本
source $(dirname "$0")/Environment.sh

# 定义FBX2OBJ工具的分布、源代码和构建文件夹路径
FBX2OBJ_DIST=${CARLA_DOCKER_UTILS_FOLDER}/dist
FBX2OBJ_FOLDER=${CARLA_DOCKER_UTILS_FOLDER}/fbx
FBX2OBJ_BUILD_FOLDER=${FBX2OBJ_FOLDER}/build
FBX2OBJ_DEP_FOLDER=${FBX2OBJ_FOLDER}/dependencies

# 检查FBX SDK是否已经安装
if [ -f "${FBX2OBJ_DIST}/FBX2OBJ" ]; then
  log "FBX SDK already installed."
  exit
fi

# 定义FBX SDK的名称和下载URL
LIB_NAME=fbx202001_fbxsdk_linux
FBXSDK_URL=https://www.autodesk.com/content/dam/autodesk/www/adn/fbx/2020-0-1/${LIB_NAME}.tar.gz

# 如果依赖文件夹不存在，则下载并安装FBX SDK
if [ ! -d "${FBX2OBJ_DEP_FOLDER}" ]; then
  log "Downloading FBX SDK..."
  wget -c "${FBXSDK_URL}" -P "${CARLA_DOCKER_UTILS_FOLDER}" --user-agent="Mozilla"

  echo "Unpacking..."
  mkdir -p "${FBX2OBJ_DEP_FOLDER}"
  tar -xvzf "${CARLA_DOCKER_UTILS_FOLDER}/${LIB_NAME}.tar.gz" -C "${CARLA_DOCKER_UTILS_FOLDER}" "${LIB_NAME}"
  rm "${CARLA_DOCKER_UTILS_FOLDER}/${LIB_NAME}.tar.gz"

  echo "Installing FBX SDK..."
  echo -e "y\nyes\nn\n" | "${CARLA_DOCKER_UTILS_FOLDER}/${LIB_NAME}" "${FBX2OBJ_DEP_FOLDER}"
  echo
  rm "${CARLA_DOCKER_UTILS_FOLDER}/${LIB_NAME}"
fi
# 编译FBX2OBJ工具
log "Compiling FBX2OBJ..."
mkdir -p "${FBX2OBJ_DIST}"
mkdir -p "${FBX2OBJ_BUILD_FOLDER}"
# 进入构建文件夹
pushd "${FBX2OBJ_BUILD_FOLDER}" >/dev/null
# 使用CMake配置项目
cmake -G "Ninja" \
    -DCMAKE_CXX_FLAGS="-fPIC -std=c++14" \
    ..

# 复制共享对象'libfbxsdk.so'到'dist'文件夹
if [ ! -f "${FBX2OBJ_DIST}/libfbxsdk.so" ]; then
  cp "${FBX2OBJ_DEP_FOLDER}/lib/gcc/x64/release/libfbxsdk.so" "${FBX2OBJ_DIST}"
fi
# 禁用错误立即退出
set +e
# 构建项目
ninja
# 如果构建失败，提示安装libxml2-dev并退出
if [ $? -eq 1 ]; then
  fatal_error "Make sure \"libxml2-dev\" is installed using:\n\n    sudo apt-get install libxml2-dev\n"
  exit 1
fi
# 安装构建结果
ninja install
# 启用错误立即退出
set -e
# 返回到原始目录
popd >/dev/null
# 打印成功信息
log "Success!"
