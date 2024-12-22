#! /bin/bash

# ==============================================================================
# -- 脚本说明 ----------------------------------------------------------------
# ==============================================================================
# 这是一个用于构建和启动CarlaUE4项目的Bash脚本
# 支持多种构建选项和可选模块的配置

# -- 帮助信息 ----------------------------------------------------------------
DOC_STRING="Build and launch CarlaUE4."
USAGE_STRING="Usage: $0 [-h|--help] [--build] [--rebuild] [--launch] [--clean] [--hard-clean] [--opengl]"

# ==============================================================================
# -- 初始化变量 --------------------------------------------------------------
# ==============================================================================

# 基本构建选项
REMOVE_INTERMEDIATE=false  # 是否清理中间文件
HARD_CLEAN=false          # 是否进行完整清理
BUILD_CARLAUE4=false      # 是否构建项目
LAUNCH_UE4_EDITOR=false   # 是否启动编辑器

# 可选模块配置
USE_CARSIM=false    # CarSim物理引擎
USE_CHRONO=false    # Chrono物理引擎
USE_PYTORCH=false   # PyTorch支持
USE_UNITY=true      # Unity集成
USE_ROS2=false      # ROS2支持

# 编辑器配置
EDITOR_FLAGS=""     # 编辑器额外参数
GDB=""             # GDB调试支持
RHI="-vulkan"      # 默认使用Vulkan渲染

# ==============================================================================
# -- 参数解析 ----------------------------------------------------------------
# ==============================================================================

OPTS=`getopt -o h --long help,build,rebuild,launch,clean,hard-clean,gdb,opengl,carsim,pytorch,chrono,ros2,no-unity,editor-flags: -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    # 编辑器相关选项
    --editor-flags )
      EDITOR_FLAGS=$2
      shift ;;
    --gdb )
      GDB="gdb --args"
      shift ;;
    
    # 构建相关选项
    --build )
      BUILD_CARLAUE4=true
      shift ;;
    --rebuild )
      REMOVE_INTERMEDIATE=true
      BUILD_CARLAUE4=true
      shift ;;
    --launch )
      LAUNCH_UE4_EDITOR=true
      shift ;;
    
    # 清理相关选项
    --clean )
      REMOVE_INTERMEDIATE=true
      shift ;;
    --hard-clean )
      REMOVE_INTERMEDIATE=true
      HARD_CLEAN=true
      shift ;;
    
    # 图形相关选项
    --opengl )
      RHI="-opengl"
      shift ;;
    
    # 可选模块开关
    --carsim )
      USE_CARSIM=true
      shift ;;
    --chrono )
      USE_CHRONO=true
      shift ;;
    --pytorch )
      USE_PYTORCH=true
      shift ;;
    --ros2 )
      USE_ROS2=true
      shift ;;
    --no-unity )
      USE_UNITY=false
      shift ;;
    
    # 帮助信息
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      shift ;;
  esac
done

# ==============================================================================
# -- 环境设置 ----------------------------------------------------------------
# ==============================================================================

# 导入环境配置
source $(dirname "$0")/Environment.sh

# 验证UE4环境
if [ ! -d "${UE4_ROOT}" ]; then
  fatal_error "UE4_ROOT is not defined, or points to a non-existant directory, please set this environment variable."
else
  log "Using Unreal Engine at '$UE4_ROOT'"
fi

# 参数验证
if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_CARLAUE4} || ${LAUNCH_UE4_EDITOR}; }; then
  fatal_error "Nothing selected to be done."
fi

# 切换到项目目录
pushd "${CARLAUE4_ROOT_FOLDER}" >/dev/null

# ==============================================================================
# -- 清理操作 ----------------------------------------------------------------
# ==============================================================================

# 执行完整清理
if ${HARD_CLEAN} ; then
  if [ ! -f Makefile ]; then
    fatal_error "The project wasn't built before!"
  fi
  log "Doing a \"hard\" clean of the Unreal Engine project."
  make CarlaUE4Editor ARGS=-clean
fi

# 清理中间文件
if ${REMOVE_INTERMEDIATE} ; then
  log "Cleaning intermediate files and folders."
  UE4_INTERMEDIATE_FOLDERS="Binaries Build Intermediate DerivedDataCache"
  
  # 清理主项目
  rm -Rf ${UE4_INTERMEDIATE_FOLDERS}
  rm -f Makefile
  
  # 清理插件
  pushd "${CARLAUE4_PLUGIN_ROOT_FOLDER}" >/dev/null
  rm -Rf ${UE4_INTERMEDIATE_FOLDERS}
  cd Plugins
  rm -Rf HoudiniEngine
  cd ..
  popd >/dev/null
fi

# ==============================================================================
# -- 构建项目 ----------------------------------------------------------------
# ==============================================================================

if ${BUILD_CARLAUE4} ; then
  # 配置可选模块
  OPTIONAL_MODULES_TEXT=""
  
  # CarSim配置
  if ${USE_CARSIM} ; then
    python ${PWD}/../../Util/BuildTools/enable_carsim_to_uproject.py -f="CarlaUE4.uproject" -e
    OPTIONAL_MODULES_TEXT="CarSim ON"$'\n'"${OPTIONAL_MODULES_TEXT}"
  else
    python ${PWD}/../../Util/BuildTools/enable_carsim_to_uproject.py -f="CarlaUE4.uproject"
    OPTIONAL_MODULES_TEXT="CarSim OFF"$'\n'"${OPTIONAL_MODULES_TEXT}"
  fi
  
  # 其他模块配置
  [[ ${USE_CHRONO} == true ]] && OPTIONAL_MODULES_TEXT="Chrono ON"$'\n'"${OPTIONAL_MODULES_TEXT}" \
                               || OPTIONAL_MODULES_TEXT="Chrono OFF"$'\n'"${OPTIONAL_MODULES_TEXT}"
  [[ ${USE_PYTORCH} == true ]] && OPTIONAL_MODULES_TEXT="Pytorch ON"$'\n'"${OPTIONAL_MODULES_TEXT}" \
                                || OPTIONAL_MODULES_TEXT="Pytorch OFF"$'\n'"${OPTIONAL_MODULES_TEXT}"
  [[ ${USE_ROS2} == true ]] && OPTIONAL_MODULES_TEXT="Ros2 ON"$'\n'"${OPTIONAL_MODULES_TEXT}" \
                             || OPTIONAL_MODULES_TEXT="Ros2 OFF"$'\n'"${OPTIONAL_MODULES_TEXT}"
  [[ ${USE_UNITY} == true ]] && OPTIONAL_MODULES_TEXT="Unity ON"$'\n'"${OPTIONAL_MODULES_TEXT}" \
                              || OPTIONAL_MODULES_TEXT="Unity OFF"$'\n'"${OPTIONAL_MODULES_TEXT}"
  
  # 添加Fast_dds支持
  OPTIONAL_MODULES_TEXT="Fast_dds ON"$'\n'"${OPTIONAL_MODULES_TEXT}"
  echo ${OPTIONAL_MODULES_TEXT} > ${PWD}/Config/OptionalModules.ini

  # 生成项目文件
  if [ ! -f Makefile ]; then
    set +e
    log "Generate Unreal project files."
    ${UE4_ROOT}/GenerateProjectFiles.sh -project="${PWD}/CarlaUE4.uproject" -game -engine -makefiles
    set -e
  fi

  # 构建项目
  log "Build CarlaUE4 project."
  make CarlaUE4Editor

  # 创建地图导出目录
  mkdir -p "${CARLAUE4_ROOT_FOLDER}/Content/Carla/ExportedMaps"
fi

# ==============================================================================
# -- 启动编辑器 --------------------------------------------------------------
# ==============================================================================

if ${LAUNCH_UE4_EDITOR} ; then
  log "Launching UE4Editor..."
  ${GDB} ${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${PWD}/CarlaUE4.uproject" ${RHI} ${EDITOR_FLAGS}
else
  log "Success!"
fi

# ==============================================================================
# -- 清理工作 ----------------------------------------------------------------
# ==============================================================================

popd >/dev/null
