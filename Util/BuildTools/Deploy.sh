#! /bin/bash

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================



# 设置REPLACE_LATEST变量为false，可能表示在上传或发布新版本时不自动替换最新版本的文件。
REPLACE_LATEST=false

# 设置DOCKER_PUSH变量为false，可能表示不自动将Docker镜像推送到远程仓库。
DOCKER_PUSH=false

# 设置AWS_COPY变量为"aws s3 cp"命令，用于后续操作中将文件复制到Amazon S3存储桶。
# 注意：这里使用的是AWS CLI的s3 cp命令，但实际上并未指定源和目标路径。
AWS_COPY="aws s3 cp"

# 设置DOCKER变量为"docker"命令，用于后续操作中与Docker进行交互。
DOCKER="docker"

# 设置UNTAR变量为"tar -xvzf"命令，用于解压以.tar.gz格式存储的文件。
# -x表示解压，-v表示显示解压过程，-z表示处理gzip压缩的文件，-f表示指定文件名。
UNTAR="tar -xvzf"

# 设置UPLOAD_MAPS变量为true，可能表示需要将地图数据上传到某个存储位置。
UPLOAD_MAPS=true

# 设置PROFILE变量为"--profile Jenkins-CVC"，这个值可能用于指定AWS CLI使用的配置文件或角色。
# 在使用AWS CLI与AWS服务交互时，可以通过--profile选项指定使用的配置。
PROFILE="--profile Jenkins-CVC"

# 设置ENDPOINT变量为"--endpoint-url=https://s3.us-east-005.backblazeb2.com/"，
# 这个值可能用于指定S3兼容存储服务的端点URL。
# 在这里，它看起来像是指向Backblaze B2 Cloud Storage的URL，而不是Amazon S3的标准端点。
ENDPOINT="--endpoint-url=https://s3.us-east-005.backblazeb2.com/"

# 设置TEST变量为false，可能表示当前不是在进行测试操作。
TEST=false


# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Upload latest build to S3."

USAGE_STRING="Usage: $0 [-h|--help] [--replace-latest] [--docker-push] [--dry-run]"

OPTS=`getopt -o h --long help,replace-latest,docker-push,dry-run,test -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

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

source $(dirname "$0")/Environment.sh

REPOSITORY_TAG=$(get_git_repository_version)

LATEST_PACKAGE=CARLA_${REPOSITORY_TAG}.tar.gz
LATEST_PACKAGE_PATH=${CARLA_DIST_FOLDER}/${LATEST_PACKAGE}
LATEST_PACKAGE2=AdditionalMaps_${REPOSITORY_TAG}.tar.gz
LATEST_PACKAGE_PATH2=${CARLA_DIST_FOLDER}/${LATEST_PACKAGE2}

S3_PREFIX=s3://carla-releases/Linux

LATEST_DEPLOY_URI=${S3_PREFIX}/Dev/CARLA_Latest.tar.gz
LATEST_DEPLOY_URI2=${S3_PREFIX}/Dev/AdditionalMaps_Latest.tar.gz

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

DEPLOY_URI=${S3_PREFIX}/${DEPLOY_NAME}
DEPLOY_URI2=${S3_PREFIX}/${DEPLOY_NAME2}

${AWS_COPY} ${LATEST_PACKAGE_PATH} ${DEPLOY_URI} ${ENDPOINT} ${PROFILE}
log "Latest build uploaded to ${DEPLOY_URI}."

${AWS_COPY} ${LATEST_PACKAGE_PATH2} ${DEPLOY_URI2} ${ENDPOINT} ${PROFILE}
log "Latest build uploaded to ${DEPLOY_URI2}."

# ==============================================================================
# -- Replace Latest ------------------------------------------------------------
# ==============================================================================

if ${REPLACE_LATEST} ; then

  ${AWS_COPY} ${DEPLOY_URI} ${LATEST_DEPLOY_URI} ${ENDPOINT} ${PROFILE}
  log "Latest build uploaded to ${LATEST_DEPLOY_URI}."
  
  ${AWS_COPY} ${DEPLOY_URI2} ${LATEST_DEPLOY_URI2} ${ENDPOINT} ${PROFILE}
  log "Latest build uploaded to ${LATEST_DEPLOY_URI2}."

fi

# ==============================================================================
# -- Docker build and push -----------------------------------------------------
# ==============================================================================

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
