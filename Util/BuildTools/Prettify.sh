#! /bin/bash

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================
# 导入环境变量和函数，假设有一个名为Environment.sh的脚本在同一目录下
source $(dirname "$0")/Environment.sh

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================
# 描述脚本的功能：美化代码文件
DOC_STRING="Prettify code files."
# 使用说明字符串，显示如何使用这个脚本
USAGE_STRING="Usage: $0 [-h|--help] [--all] [-f path|--file=path]"
# 定义变量来标记是否美化所有文件或特定文件
PRETTIFY_ALL=false
PRETTIFY_FILE=false
# 使用getopt命令解析长选项和短选项
OPTS=`getopt -o hf: --long help,all,file: -n 'parse-options' -- "$@"`
# 将getopt的结果赋值给OPTS变量，并将其设置为位置参数
eval set -- "$OPTS"
# 循环处理传入的参数
while [[ $# -gt 0 ]]; do
  case "$1" in
    --all )
     # 如果参数是--all，则将PRETTIFY_ALL变量设置为true
      PRETTIFY_ALL=true;
      shift ;;
    -f | --file )
    # 如果参数是-f或--file，则将PRETTIFY_FILE变量设置为第二个参数的值
      PRETTIFY_FILE="$2";
      shift ;;
    -h | --help )
     # 如果参数是-h或--help，则打印帮助信息并退出
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
    # 如果参数不匹配任何已知选项，则忽略它
      shift ;;
  esac
done
# 检查是否选择了美化所有文件或特定文件
if ! { ${PRETTIFY_ALL} || [ -n "${PRETTIFY_FILE}" ]; } ; then
# 如果没有选择任何操作，则打印错误信息并退出
  fatal_error "Nothing selected to be done."
fi
# 如果选择了美化所有文件，则将PRETTIFY_FILE设置为false
if ${PRETTIFY_ALL} ; then
  PRETTIFY_FILE=false
elif [[ ! -f ${PRETTIFY_FILE} ]] ; then
  pwd
  # 如果选择了特定文件但该文件不存在，则打印错误信息并退出
  fatal_error "\"${PRETTIFY_FILE}\" no such file."
fi

# ==============================================================================
# -- Get latest version of uncrustify ------------------------------------------
# ==============================================================================
# 创建CARLA_BUILD_FOLDER目录
mkdir -p ${CARLA_BUILD_FOLDER}
pushd ${CARLA_BUILD_FOLDER} >/dev/null
# 定义Uncrustify的版本号
UNCRUSTIFY_BASENAME=uncrustify-0.69.0
# 定义Uncrustify的安装路径
UNCRUSTIFY=${PWD}/${UNCRUSTIFY_BASENAME}-install/bin/uncrustify
# 检查是否已经安装了Uncrustify

if [[ -d "${UNCRUSTIFY_BASENAME}-install" ]] ; then
  log "${UNCRUSTIFY_BASENAME} already installed."
else
  rm -Rf ${UNCRUSTIFY_BASENAME}-source ${UNCRUSTIFY_BASENAME}-build
# 如果已经安装，则打印日志信息
  log "Retrieving Uncrustify."
# 如果没有安装，则下载、构建并安装Uncrustify
  git clone --depth=1 -b ${UNCRUSTIFY_BASENAME} https://github.com/uncrustify/uncrustify.git ${UNCRUSTIFY_BASENAME}-source

  log "Building Uncrustify."

  mkdir -p ${UNCRUSTIFY_BASENAME}-build

  pushd ${UNCRUSTIFY_BASENAME}-build >/dev/null

  cmake -G "Ninja" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="../${UNCRUSTIFY_BASENAME}-install" \
      ../${UNCRUSTIFY_BASENAME}-source

  ninja

  ninja install

  popd >/dev/null

  rm -Rf ${UNCRUSTIFY_BASENAME}-source ${UNCRUSTIFY_BASENAME}-build

fi
# 检查Uncrustify是否安装成功
command -v ${UNCRUSTIFY} >/dev/null 2>&1 || {
  fatal_error "Failed to install Uncrustify!";
}

popd >/dev/null
# 检查autopep8是否安装，如果没有安装则使用pip安装
command -v autopep8 >/dev/null 2>&1 || {
  log "Installing autopep8 for this user."
  pip3 install --user autopep8
}

# ==============================================================================
# -- Run uncrustify and/or autopep8 --------------------------------------------
# ==============================================================================
# 定义Uncrustify的配置文件路径
UNCRUSTIFY_CONFIG=${CARLA_BUILD_TOOLS_FOLDER}/uncrustify.cfg
UNCRUSTIFY_UE4_CONFIG=${CARLA_BUILD_TOOLS_FOLDER}/uncrustify-ue4.cfg
UNCRUSTIFY_COMMAND="${UNCRUSTIFY} --no-backup --replace"

AUTOPEP8_COMMAND="autopep8 --jobs 0 --in-place -a"
# 如果选择了美化所有文件，则打印错误信息（当前不支持美化所有文件）
if ${PRETTIFY_ALL} ; then

  fatal_error "Prettify all not yet supported"
# 以下代码被注释掉了，因为美化所有文件的功能尚未支持
  # find ${CARLA_ROOT_FOLDER} -iregex '.*\.\(py\)$' -exec ${AUTOPEP8_COMMAND} {} +
  # find ${LIBCARLA_ROOT_FOLDER} -iregex '.*\.\(h\|cpp\)$' -exec ${UNCRUSTIFY_COMMAND} {} \;

elif [[ -f ${PRETTIFY_FILE} ]] ; then
# 如果选择了特定文件，则根据文件类型运行相应的美化命令
  if [[ ${PRETTIFY_FILE} == *.py ]] ; then
    log "autopep8 ${PRETTIFY_FILE}"
    ${AUTOPEP8_COMMAND} ${PRETTIFY_FILE}
  elif [[ ${PRETTIFY_FILE} == *Unreal/CarlaUE4/* ]] ; then
    log "uncrustify for UE4 ${PRETTIFY_FILE}"
    ${UNCRUSTIFY_COMMAND} -c ${UNCRUSTIFY_UE4_CONFIG} ${PRETTIFY_FILE}
  else
    log "uncrustify ${PRETTIFY_FILE}"
    ${UNCRUSTIFY_COMMAND} -c ${UNCRUSTIFY_CONFIG} ${PRETTIFY_FILE}
  fi

fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================
# 打印日志信息，表示美化操作成功完成
log "Success!"
