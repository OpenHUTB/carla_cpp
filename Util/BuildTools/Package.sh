#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Makes a packaged version of CARLA and other content packages ready for distribution."

USAGE_STRING="Usage: $0 [-h|--help] [--config={Debug,Development,Shipping}] [--no-zip] [--clean-intermediate] [--packages=Name1,Name2,...] [--target-archive=] [--archive-sufix=]"

# 定义一个变量PACKAGES，并将其值设置为"Carla"。
PACKAGES="Carla"

# 定义一个变量DO_TARBALL，并将其值设置为true。
DO_TARBALL=true

# 定义一个变量DO_CLEAN_INTERMEDIATE，并将其值设置为false。
DO_CLEAN_INTERMEDIATE=false

# 定义一个变量PROPS_MAP_NAME，并将其值设置为"PropsMap"。
PROPS_MAP_NAME=PropsMap

# 定义一个变量PACKAGE_CONFIG，并将其值设置为"Shipping"。
PACKAGE_CONFIG=Shipping

# 定义一个变量USE_CARSIM，并将其值设置为false。
USE_CARSIM=false

# 定义一个变量SINGLE_PACKAGE，并将其值设置为false。
SINGLE_PACKAGE=false

# 定义一个变量ARCHIVE_SUFIX，并将其值设置为空字符串。
ARCHIVE_SUFIX=""
# 使用 getopt 解析命令行参数
OPTS=`getopt -o h --long help,config:,no-zip,clean-intermediate,carsim,packages:,python-version,target-archive:,archive-sufix:, -n 'parse-options' -- "$@"`

eval set -- "$OPTS"
# 循环处理每个参数
while [[ $# -gt 0 ]]; do
  case "$1" in
    --config )
      PACKAGE_CONFIG="$2"
      shift 2 ;;
    --no-zip )
      DO_TARBALL=false
      shift ;;
    --clean-intermediate )
      DO_CLEAN_INTERMEDIATE=true
      shift ;;
    --packages )
      PACKAGES="$2"
      shift 2 ;;
    --target-archive )
      SINGLE_PACKAGE=true
      TARGET_ARCHIVE="$2"
      shift 2 ;;
    --archive-sufix )
      ARCHIVE_SUFIX="$2"
      shift 2 ;;
    --carsim )
      USE_CARSIM=true;
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

# ==============================================================================
# -- Prepare environment -------------------------------------------------------
# ==============================================================================
# 导入环境设置脚本
source $(dirname "$0")/Environment.sh
# 检查 UE4_ROOT 环境变量是否设置并指向存在的目录
if [ ! -d "${UE4_ROOT}" ]; then
  fatal_error "UE4_ROOT is not defined, or points to a non-existent directory, please set this environment variable."
fi
# 如果 PACKAGES 变量为空，则报错退出
if [ ! -n "${PACKAGES}" ] ; then
  fatal_error "Nothing to be done."
fi

# 将逗号分隔的字符串转换为数组
PACKAGES="$(echo "${PACKAGES}" | tr ',' '\n' | sort -u | tr '\n' ',')"
IFS=',' read -r -a PACKAGES <<< "${PACKAGES}"

# 如果数组包含 "Carla" 元素，则设置 DO_CARLA_RELEASE 为 true
if [[ "${PACKAGES[@]}" =~ "Carla" ]] ; then
  DO_CARLA_RELEASE=true
else
  DO_CARLA_RELEASE=false
fi
# 获取 git 仓库版本号
REPOSITORY_TAG=$(get_git_repository_version)
# 获取 git 仓库版本号
if [[ ${ARCHIVE_SUFIX} != "" ]] ; then
  RELEASE_BUILD_FOLDER=${CARLA_DIST_FOLDER}/CARLA_${PACKAGE_CONFIG}_${REPOSITORY_TAG}_${ARCHIVE_SUFIX}
else
  RELEASE_BUILD_FOLDER=${CARLA_DIST_FOLDER}/CARLA_${PACKAGE_CONFIG}_${REPOSITORY_TAG}
fi
# 根据 PACKAGE_CONFIG 变量设置发布包路径
if [[ ${PACKAGE_CONFIG} == "Shipping" ]] ; then
  RELEASE_PACKAGE_PATH=${CARLA_DIST_FOLDER}/CARLA_${REPOSITORY_TAG}
else
  RELEASE_PACKAGE_PATH=${CARLA_DIST_FOLDER}/CARLA_${PACKAGE_CONFIG}_${REPOSITORY_TAG}
fi
# 如果设置了 ARCHIVE_SUFIX，则添加到发布包路径
if [[ ${ARCHIVE_SUFIX} != "" ]] ; then
  RELEASE_PACKAGE_PATH=${RELEASE_PACKAGE_PATH}_${ARCHIVE_SUFIX}
fi
# 设置发布包路径的文件名
RELEASE_PACKAGE_PATH=${RELEASE_PACKAGE_PATH}.tar.gz

log "Packaging version '${REPOSITORY_TAG}' (${PACKAGE_CONFIG})."

# ==============================================================================
# -- Cook CARLA project --------------------------------------------------------
# ==============================================================================
# 如果设置了 DO_CARLA_RELEASE，则执行 CARLA 项目的构建
if ${DO_CARLA_RELEASE} ; then

  pushd "${CARLAUE4_ROOT_FOLDER}" >/dev/null
# 如果设置了 USE_CARSIM，则启用 CarSim
  if ${USE_CARSIM} ; then
    python ${PWD}/../../Util/BuildTools/enable_carsim_to_uproject.py -f="CarlaUE4.uproject" -e
    echo "CarSim ON" > ${PWD}/Config/CarSimConfig.ini
  else
    python ${PWD}/../../Util/BuildTools/enable_carsim_to_uproject.py -f="CarlaUE4.uproject"
    echo "CarSim OFF" > ${PWD}/Config/CarSimConfig.ini
  fi

  log "Cooking CARLA project."
# 删除旧的发布构建文件夹并创建新的
  rm -Rf ${RELEASE_BUILD_FOLDER}
  mkdir -p ${RELEASE_BUILD_FOLDER}
# 使用 Unreal Engine 4 的 UAT 工具构建项目
  ${UE4_ROOT}/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun \
      -project="${PWD}/CarlaUE4.uproject" \
      -nocompileeditor -nop4 -cook -stage -archive -package -iterate \
      -clientconfig=${PACKAGE_CONFIG} -ue4exe=UE4Editor \
      -prereqs -targetplatform=Linux -build -utf8output \
      -archivedirectory="${RELEASE_BUILD_FOLDER}"

  popd >/dev/null
# 如果构建失败，则报错退出
  if [[ ! -d ${RELEASE_BUILD_FOLDER}/LinuxNoEditor ]] ; then
    fatal_error "Failed to cook the project!"
  fi

fi

# ==============================================================================
# -- Copy files (Python API, README, etc) --------------------------------------
# ==============================================================================
# 如果设置了 DO_CARLA_RELEASE，则执行额外文件的复制
if ${DO_CARLA_RELEASE} ; then

  DESTINATION=${RELEASE_BUILD_FOLDER}/LinuxNoEditor

  log "Adding extra files to CARLA package."

  pushd ${CARLA_ROOT_FOLDER} >/dev/null
# 创建Import文件夹并复制版本号
  mkdir -p "${DESTINATION}/Import"

  echo "${REPOSITORY_TAG}" > ${DESTINATION}/VERSION
# 复制文件到发布构建文件夹
  copy_if_changed "./LICENSE" "${DESTINATION}/LICENSE"
  copy_if_changed "./CHANGELOG.md" "${DESTINATION}/CHANGELOG"
  copy_if_changed "./Docs/release_readme.md" "${DESTINATION}/README"
  copy_if_changed "./Docs/python_api.md" "${DESTINATION}/PythonAPI/python_api.md"
  copy_if_changed "./Util/Docker/Release.Dockerfile" "${DESTINATION}/Dockerfile"
  copy_if_changed "./Util/ImportAssets.sh" "${DESTINATION}/ImportAssets.sh"
  copy_if_changed "./Util/DockerUtils/dist/RecastBuilder" "${DESTINATION}/Tools/"
 # 复制Python API文件
  copy_if_changed "./PythonAPI/carla/dist/*.egg" "${DESTINATION}/PythonAPI/carla/dist/"
  copy_if_changed "./PythonAPI/carla/dist/*.whl" "${DESTINATION}/PythonAPI/carla/dist/"
  copy_if_changed "./PythonAPI/carla/agents/" "${DESTINATION}/PythonAPI/carla/agents"
  copy_if_changed "./PythonAPI/carla/scene_layout.py" "${DESTINATION}/PythonAPI/carla/"
  copy_if_changed "./PythonAPI/carla/requirements.txt" "${DESTINATION}/PythonAPI/carla/"
# 复制Python API示例文件
  copy_if_changed "./PythonAPI/examples/*.py" "${DESTINATION}/PythonAPI/examples/"
  copy_if_changed "./PythonAPI/examples/rss/*.py" "${DESTINATION}/PythonAPI/examples/rss/"
  copy_if_changed "./PythonAPI/examples/requirements.txt" "${DESTINATION}/PythonAPI/examples/"
 # 复制Python API工具文件
  copy_if_changed "./PythonAPI/util/*.py" "${DESTINATION}/PythonAPI/util/"
  copy_if_changed "./PythonAPI/util/opendrive/" "${DESTINATION}/PythonAPI/util/opendrive/"
  copy_if_changed "./PythonAPI/util/requirements.txt" "${DESTINATION}/PythonAPI/util/"
# 复制Co-Simulation文件夹
  copy_if_changed "./Co-Simulation/" "${DESTINATION}/Co-Simulation/"
# 复制插件文件夹
  if [ -d "./Plugins/" ] ; then
    copy_if_changed "./Plugins/" "${DESTINATION}/Plugins/"
  fi
# 复制Carla依赖库
  if [ -d "./Unreal/CarlaUE4/Plugins/Carla/CarlaDependencies/lib" ] ; then
    cp -r "./Unreal/CarlaUE4/Plugins/Carla/CarlaDependencies/lib" "${DESTINATION}/CarlaUE4/Plugins/Carla/CarlaDependencies"
  fi
# 复制HDMaps文件夹
  copy_if_changed "./Unreal/CarlaUE4/Content/Carla/HDMaps/*.pcd" "${DESTINATION}/HDMaps/"
  copy_if_changed "./Unreal/CarlaUE4/Content/Carla/HDMaps/Readme.md" "${DESTINATION}/HDMaps/README"

  popd >/dev/null

fi

# ==============================================================================
# -- Zip the project -----------------------------------------------------------
# ==============================================================================

#确定源文件的位置和目标文件的存放路径
if ${DO_CARLA_RELEASE} && ${DO_TARBALL} ; then

  DESTINATION=${RELEASE_PACKAGE_PATH}
  SOURCE=${RELEASE_BUILD_FOLDER}/LinuxNoEditor

  #将当前目录切换到指定的目录
  pushd "${SOURCE}" >/dev/null

  #记录正在进行的CARLA版本的打包操作
  log "Packaging CARLA release."

  #进行了一系列的文件删除操作
  rm -f ./Manifest_NonUFSFiles_Linux.txt
  rm -f ./Manifest_UFSFiles_Linux.txt
  rm -Rf ./CarlaUE4/Saved
  rm -Rf ./Engine/Saved

  #用于打包和压缩文件
  tar -czf ${DESTINATION} *

  #从目录栈中弹出一个目录，并将当前目录切换到弹出的目录
  popd >/dev/null

fi

# ==============================================================================
# -- Remove intermediate files -------------------------------------------------
# ==============================================================================

#条件判断
if ${DO_CARLA_RELEASE} && ${DO_CLEAN_INTERMEDIATE} ; then

  #记录一条日志信息
  log "Removing intermediate build."

  #删除与RELEASE_BUILD_FOLDER变量所指定路段相关的中间构建文件或目录
  rm -Rf ${RELEASE_BUILD_FOLDER}

fi

# ==============================================================================
# -- Cook other packages -------------------------------------------------------
# ==============================================================================

PACKAGE_PATH_FILE=${CARLAUE4_ROOT_FOLDER}/Content/PackagePath.txt
MAP_LIST_FILE=${CARLAUE4_ROOT_FOLDER}/Content/MapPathsLinux.txt

for PACKAGE_NAME in "${PACKAGES[@]}" ; do if [[ ${PACKAGE_NAME} != "Carla" ]] ; then

  log "Preparing environment for cooking '${PACKAGE_NAME}'."

  if ${SINGLE_PACKAGE} ; then
      BUILD_FOLDER_TARGET=${CARLA_DIST_FOLDER}/${TARGET_ARCHIVE}_${REPOSITORY_TAG}
  else
      BUILD_FOLDER_TARGET=${CARLA_DIST_FOLDER}/${PACKAGE_NAME}_${REPOSITORY_TAG}
  fi

  if [[ ${ARCHIVE_SUFIX} != "" ]] ; then
    BUILD_FOLDER_TARGET=${BUILD_FOLDER_TARGET}_${ARCHIVE_SUFIX}
  fi

  if [[ ${ARCHIVE_SUFIX} != "" ]] ; then
    BUILD_FOLDER=${CARLA_DIST_FOLDER}/${PACKAGE_NAME}_${REPOSITORY_TAG}_${ARCHIVE_SUFIX}
  else
    BUILD_FOLDER=${CARLA_DIST_FOLDER}/${PACKAGE_NAME}_${REPOSITORY_TAG}
  fi

  DESTINATION=${BUILD_FOLDER_TARGET}.tar
  PACKAGE_PATH=${CARLAUE4_ROOT_FOLDER}/Content/${PACKAGE_NAME}

  mkdir -p ${BUILD_FOLDER}

  log "Cooking package '${PACKAGE_NAME}'..."

  pushd "${CARLAUE4_ROOT_FOLDER}" > /dev/null

  # Prepare cooking of package
  ${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${CARLAUE4_ROOT_FOLDER}/CarlaUE4.uproject" \
      -run=PrepareAssetsForCooking -PackageName=${PACKAGE_NAME} -OnlyPrepareMaps=false

  PACKAGE_FILE=$(<${PACKAGE_PATH_FILE})
  MAPS_TO_COOK=$(<${MAP_LIST_FILE})


  # Cook maps in batches
  MAX_STRINGLENGTH=1000
  IFS="+" read -ra MAP_LIST <<< $MAPS_TO_COOK
  TOTAL=0
  MAP_STRING=""
  for MAP in "${MAP_LIST[@]}"; do
    if (($(($TOTAL+${#MAP})) > $MAX_STRINGLENGTH)); then
      echo "Cooking $MAP_STRING"
      ${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${CARLAUE4_ROOT_FOLDER}/CarlaUE4.uproject" \
          -run=cook -map="${MAP_STRING}" -cooksinglepackage -targetplatform="LinuxNoEditor" \
          -OutputDir="${BUILD_FOLDER}" -iterate
      MAP_STRING=""
      TOTAL=0
    fi
    MAP_STRING=$MAP_STRING+$MAP
    TOTAL=$(($TOTAL+${#MAP}))
  done
  if (($TOTAL > 0)); then
    ${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${CARLAUE4_ROOT_FOLDER}/CarlaUE4.uproject" \
        -run=cook -map="${MAP_STRING}" -cooksinglepackage -targetplatform="LinuxNoEditor" \
        -OutputDir="${BUILD_FOLDER}" -iterate
  fi

  PROP_MAP_FOLDER="${PACKAGE_PATH}/Maps/${PROPS_MAP_NAME}"

  if [ -d ${PROP_MAP_FOLDER} ] ; then
    rm -Rf ${PROP_MAP_FOLDER}
  fi

  popd >/dev/null

  pushd "${BUILD_FOLDER}" > /dev/null

  SUBST_PATH="${BUILD_FOLDER}/CarlaUE4"
  SUBST_FILE="${PACKAGE_FILE/${CARLAUE4_ROOT_FOLDER}/${SUBST_PATH}}"

  # Copy the package config file to package
  mkdir -p "$(dirname ${SUBST_FILE})" && cp "${PACKAGE_FILE}" "$_"

  # Copy the OpenDRIVE .xodr files to package
  IFS='+' # set delimiter
  # MAPS_TO_COOK is read into an array as tokens separated by IFS
  read -ra ADDR <<< "$MAPS_TO_COOK"
  for i in "${ADDR[@]}"; do # access each element of array

    XODR_FILE_PATH="${CARLAUE4_ROOT_FOLDER}/Content${i:5}"
    MAP_NAME=${XODR_FILE_PATH##*/}
    XODR_FILE=$(find "${CARLAUE4_ROOT_FOLDER}/Content" -name "${MAP_NAME}.xodr" -print -quit)

    if [ -f "${XODR_FILE}" ] ; then

      SUBST_FILE="${XODR_FILE/${CARLAUE4_ROOT_FOLDER}/${SUBST_PATH}}"

      # Copy the package config file to package
      mkdir -p "$(dirname ${SUBST_FILE})" && cp "${XODR_FILE}" "$_"

    fi

    # binary files for navigation and traffic manager
    BIN_FILE_PATH="${CARLAUE4_ROOT_FOLDER}/Content${i:5}"
    MAP_NAME=${BIN_FILE_PATH##*/}
    find "${CARLAUE4_ROOT_FOLDER}/Content" -name "${MAP_NAME}.bin" -print0 | while read -d $'\0' BIN_FILE
    do
      if [ -f "${BIN_FILE}" ] ; then

        SUBST_FILE="${BIN_FILE/${CARLAUE4_ROOT_FOLDER}/${SUBST_PATH}}"

        # Copy the package config file to package
        mkdir -p "$(dirname ${SUBST_FILE})" && cp "${BIN_FILE}" "$_"
    fi
    done
  done

  rm -Rf "./CarlaUE4/Metadata"
  rm -Rf "./CarlaUE4/Plugins"
  rm -Rf "./CarlaUE4/Content/${PACKAGE_NAME}/Maps/${PROPS_MAP_NAME}"
  rm -f "./CarlaUE4/AssetRegistry.bin"

  if ${DO_TARBALL} ; then

    if ${SINGLE_PACKAGE} ; then
      tar -rf ${DESTINATION} *
    else
      tar -czf ${DESTINATION}.gz *
    fi

    popd >/dev/null

  fi

  if ${DO_CLEAN_INTERMEDIATE} ; then

    log "Removing intermediate build."

    rm -Rf ${BUILD_FOLDER}

  fi

fi ; done

# compress the TAR if it is a single package
if ${SINGLE_PACKAGE} ; then
  gzip -f ${DESTINATION}
fi

# ==============================================================================
# -- Log paths of generated packages -------------------------------------------
# ==============================================================================

# for PACKAGE_NAME in "${PACKAGES[@]}" ; do if [[ ${PACKAGE_NAME} != "Carla" ]] ; then
#   FINAL_PACKAGE=${CARLA_DIST_FOLDER}/${PACKAGE_NAME}_${REPOSITORY_TAG}.tar.gz
#   log "Package '${PACKAGE_NAME}' created at ${FINAL_PACKAGE}"
# fi ; done

if ${DO_CARLA_RELEASE} ; then
  if ${DO_TARBALL} ; then
    log "CARLA release created at ${RELEASE_PACKAGE_PATH}"
  else
    log "CARLA release created at ${RELEASE_BUILD_FOLDER}"
  fi
fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
