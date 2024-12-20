#! /bin/bash
# 这是一个Bash脚本，用于将最新的构建上传到S3存储。
# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================


# 初始化变量
REPLACE_LATEST=false# 是否替换最新版本的标志
DOCKER_PUSH=false# 是否推送Docker镜像的标志
AWS_COPY="aws s3 cp"# AWS S3复制命令
DOCKER="docker"# Docker命令
UNTAR="tar -xvzf"# 解压命令
UPLOAD_MAPS=true# 是否上传地图的标志
PROFILE="--profile Jenkins-CVC"# AWS CLI配置文件
ENDPOINT="--endpoint-url=https://s3.us-east-005.backblazeb2.com/"# S3端点
TEST=false# 是否为测试模式


# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Upload latest build to S3."# 脚本的文档字符串，描述脚本的功能

USAGE_STRING="Usage: $0 [-h|--help] [--replace-latest] [--docker-push] [--dry-run]"# 脚本的使用说明

# 使用getopt解析命令行参数
OPTS=`getopt -o h --long help,replace-latest,docker-push,dry-run,test -n 'parse-options' -- "$@"`

eval set -- "$OPTS"# 重新设置参数列表

while [[ $# -gt 0 ]]; do
  case "$1" in
    --test )
      TEST=true
      shift ;;
    --replace-latest )
      REPLACE_LATEST=true;
      shift ;;
    --docker-push )
      DOCKER_PUSH=true;
      shift ;;
    --dry-run )
      AWS_COPY="echo ${AWS_COPY}";
      DOCKER="echo ${DOCKER}";
      UNTAR="echo ${UNTAR}";
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
# 获取Git仓库版本
REPOSITORY_TAG=$(get_git_repository_version)
# 定义S3存储路径和文件名
LATEST_PACKAGE=CARLA_${REPOSITORY_TAG}.tar.gz
LATEST_PACKAGE_PATH=${CARLA_DIST_FOLDER}/${LATEST_PACKAGE}
LATEST_PACKAGE2=AdditionalMaps_${REPOSITORY_TAG}.tar.gz
LATEST_PACKAGE_PATH2=${CARLA_DIST_FOLDER}/${LATEST_PACKAGE2}

S3_PREFIX=s3://carla-releases/Linux

LATEST_DEPLOY_URI=${S3_PREFIX}/Dev/CARLA_Latest.tar.gz
LATEST_DEPLOY_URI2=${S3_PREFIX}/Dev/AdditionalMaps_Latest.tar.gz
# 根据版本标签设置部署名称和Docker标签
if [[ ${REPOSITORY_TAG} =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  log "Detected tag ${REPOSITORY_TAG}."
  DEPLOY_NAME=CARLA_${REPOSITORY_TAG}.tar.gz
  DEPLOY_NAME2=AdditionalMaps_${REPOSITORY_TAG}.tar.gz
  DOCKER_TAG=${REPOSITORY_TAG}
elif [[ ${REPOSITORY_TAG} =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  log "Detected tag ${REPOSITORY_TAG}."
  DEPLOY_NAME=CARLA_${REPOSITORY_TAG}.tar.gz
  DEPLOY_NAME2=AdditionalMaps_${REPOSITORY_TAG}.tar.gz
  DOCKER_TAG=${REPOSITORY_TAG}
else
  S3_PREFIX=${S3_PREFIX}/Dev
  DEPLOY_NAME=$(git log --pretty=format:'%cd_%h' --date=format:'%Y%m%d' -n 1).tar.gz
  DEPLOY_NAME2=AdditionalMaps_$(git log --pretty=format:'%cd_%h' --date=format:'%Y%m%d' -n 1).tar.gz
  DOCKER_TAG=latest
fi

log "Using package ${LATEST_PACKAGE} as ${DEPLOY_NAME}."
log "Using package ${LATEST_PACKAGE2} as ${DEPLOY_NAME2}."

if [ ! -f ${LATEST_PACKAGE_PATH} ]; then
  fatal_error "Latest package not found, please run 'make package'."
fi


# ==============================================================================
# -- TEST --------------------------------------------------------------------
# ==============================================================================
# 如果设置了测试模式，则创建测试文件
if ${TEST} ; then
  LATEST_PACKAGE=test_CARLA_${REPOSITORY_TAG}.tar.gz
  LATEST_PACKAGE_PATH=./${LATEST_PACKAGE}
  LATEST_PACKAGE2=test_AdditionalMaps_${REPOSITORY_TAG}.tar.gz
  LATEST_PACKAGE_PATH2=./${LATEST_PACKAGE2}

  DEPLOY_NAME=test_CARLA_${REPOSITORY_TAG}.tar.gz
  DEPLOY_NAME2=test_AdditionalMaps_${REPOSITORY_TAG}.tar.gz

  touch ${LATEST_PACKAGE}
  touch ${LATEST_PACKAGE2}

fi

# ==============================================================================
# -- Upload --------------------------------------------------------------------
# ==============================================================================
# 定义上传到S3的URI
DEPLOY_URI=${S3_PREFIX}/${DEPLOY_NAME}
DEPLOY_URI2=${S3_PREFIX}/${DEPLOY_NAME2}
# 上传文件到S3
${AWS_COPY} ${LATEST_PACKAGE_PATH} ${DEPLOY_URI} ${ENDPOINT} ${PROFILE}
log "Latest build uploaded to ${DEPLOY_URI}."

${AWS_COPY} ${LATEST_PACKAGE_PATH2} ${DEPLOY_URI2} ${ENDPOINT} ${PROFILE}
log "Latest build uploaded to ${DEPLOY_URI2}."

# ==============================================================================
# -- Replace Latest ------------------------------------------------------------
# ==============================================================================
# 如果设置了替换最新版本的标志，则替换S3上的"latest"版本文件
if ${REPLACE_LATEST} ; then

  ${AWS_COPY} ${DEPLOY_URI} ${LATEST_DEPLOY_URI} ${ENDPOINT} ${PROFILE}
  log "Latest build uploaded to ${LATEST_DEPLOY_URI}."
  
  ${AWS_COPY} ${DEPLOY_URI2} ${LATEST_DEPLOY_URI2} ${ENDPOINT} ${PROFILE}
  log "Latest build uploaded to ${LATEST_DEPLOY_URI2}."

fi

# ==============================================================================
# -- Docker build and push -----------------------------------------------------
# ==============================================================================
# 如果设置了推送Docker镜像的标志，则构建并推送Docker镜像
if ${DOCKER_PUSH} ; then

  DOCKER_BUILD_FOLDER=${CARLA_BUILD_FOLDER}/${REPOSITORY_TAG}.Docker
  DOCKER_NAME=carlasim/carla:${DOCKER_TAG}

  mkdir -p ${DOCKER_BUILD_FOLDER}

  ${UNTAR} ${LATEST_PACKAGE_PATH} -C ${DOCKER_BUILD_FOLDER}/

  pushd "${DOCKER_BUILD_FOLDER}" >/dev/null

  log "Building Docker image ${DOCKER_NAME}."

  ${DOCKER} build -t ${DOCKER_NAME} -f Dockerfile .

  log "Pushing Docker image."

  ${DOCKER} push ${DOCKER_NAME}

  popd >/dev/null

fi;

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
