#! /bin/bash# 这是一个Bash脚本，用于运行CARLA项目的单元测试。

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Run unit tests."# 脚本的文档字符串，描述脚本的功能

USAGE_STRING=$(cat <<- END
Usage: $0 [-h|--help] [--gdb] [--xml] [--gtest_args=ARGS] [--python-version=VERSION]

Then either ran all the tests

    [--all]

Or choose one or more of the following

    [--libcarla-release] [--libcarla-debug]
    [--benchmark]

You can also set the command-line arguments passed to GTest on a ".gtest"
config file in the Carla project main folder. E.g.

    # Contents of ${CARLA_ROOT_FOLDER}/.gtest
    gtest_shuffle
    gtest_filter=misc*
END
)# 脚本的使用说明

# 初始化变量
GDB=
XML_OUTPUT=false
GTEST_ARGS=`sed -e 's/#.*$//g' ${CARLA_ROOT_FOLDER}/.gtest | sed -e '/^[[:space:]]*$/!s/^/--/g' | sed -e ':a;N;$!ba;s/\n/ /g'`
LIBCARLA_RELEASE=false
LIBCARLA_DEBUG=false
SMOKE_TESTS=false
PYTHON_API=false
RUN_BENCHMARK=false

# 使用getopt解析命令行参数
OPTS=`getopt -o h --long help,gdb,xml,gtest_args:,all,libcarla-release,libcarla-debug,python-api,smoke,benchmark,python-version:, -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

# 导入环境设置脚本
source $(dirname "$0")/Environment.sh

PY_VERSION_LIST=3

# 遍历解析后的参数列表
while [[ $# -gt 0 ]]; do
  case "$1" in
    --gdb )
      GDB="gdb --args";# 设置GDB调试器
      shift ;;
    --xml )
      XML_OUTPUT=true;# 设置XML输出标志
      # 创建测试结果文件夹
      mkdir -p "${CARLA_TEST_RESULTS_FOLDER}"
      shift ;;
    --gtest_args )
      GTEST_ARGS="$2";# 设置GTest参数
      shift 2 ;;
    --all )
      LIBCARLA_RELEASE=true;# 设置LibCarla发布版本测试标志
      LIBCARLA_DEBUG=true;# 设置LibCarla调试版本测试标志
      PYTHON_API=true;# 设置Python API测试标志
      shift ;;
    --libcarla-release )
      LIBCARLA_RELEASE=true;# 设置LibCarla发布版本测试标志
      shift ;;
    --libcarla-debug )
      LIBCARLA_DEBUG=true;# 设置LibCarla调试版本测试标志
      shift ;;
    --smoke )
      SMOKE_TESTS=true;# 设置烟雾测试标志
      shift ;;
    --python-api )
      PYTHON_API=true;# 设置Python API测试标志
      shift ;;
    --benchmark )
      LIBCARLA_RELEASE=true;# 设置LibCarla发布版本测试标志
      RUN_BENCHMARK=true;# 设置基准测试标志
      GTEST_ARGS="--gtest_filter=benchmark*";# 设置GTest基准测试参数
      shift ;;
    --python-version )
      PY_VERSION_LIST="$2"# 设置Python版本
      shift 2 ;;
    -h | --help )
      echo "$DOC_STRING"# 打印文档字符串
      echo -e "$USAGE_STRING"# 打印使用说明
      exit 1# 退出脚本
      ;;
    * )
      shift ;;
  esac
done
# 如果没有选择任何测试，则报错退出
if ! { ${LIBCARLA_RELEASE} || ${LIBCARLA_DEBUG} || ${PYTHON_API} || ${SMOKE_TESTS}; }; then
  fatal_error "Nothing selected to be done."
fi

# 将逗号分隔的字符串转换为数组
IFS="," read -r -a PY_VERSION_LIST <<< "${PY_VERSION_LIST}"

# ==============================================================================
# -- Download Content need it by the tests -------------------------------------
# ==============================================================================
# 如果需要运行LibCarla测试，则下载测试所需的内容
if { ${LIBCARLA_RELEASE} || ${LIBCARLA_DEBUG}; }; then

  CONTENT_TAG=0.1.4

  mkdir -p ${LIBCARLA_TEST_CONTENT_FOLDER}
  pushd "${LIBCARLA_TEST_CONTENT_FOLDER}" >/dev/null
# 如果当前目录的Git版本与所需版本不同，则克隆新的版本
  if [ "$(get_git_repository_version)" != "${CONTENT_TAG}" ]; then
    pushd .. >/dev/null
    rm -Rf ${LIBCARLA_TEST_CONTENT_FOLDER}
    git clone -b ${CONTENT_TAG} https://github.com/carla-simulator/opendrive-test-files.git ${LIBCARLA_TEST_CONTENT_FOLDER}
    popd >/dev/null
  fi

  popd >/dev/null

fi

# ==============================================================================
# -- Run LibCarla tests --------------------------------------------------------
# ==============================================================================
# 如果设置了LibCarla调试版本测试标志，则运行LibCarla调试版本的单元测试
if ${LIBCARLA_DEBUG} ; then

  if ${XML_OUTPUT} ; then
    EXTRA_ARGS="--gtest_output=xml:${CARLA_TEST_RESULTS_FOLDER}/libcarla-debug.xml"
  else
    EXTRA_ARGS=
  fi

  log "Running LibCarla.server unit tests (debug)."
  echo "Running: ${GDB} libcarla_test_server_debug ${GTEST_ARGS} ${EXTRA_ARGS}"
  LD_LIBRARY_PATH=${LIBCARLA_INSTALL_SERVER_FOLDER}/lib ${GDB} ${LIBCARLA_INSTALL_SERVER_FOLDER}/test/libcarla_test_server_debug ${GTEST_ARGS} ${EXTRA_ARGS}

  log "Running LibCarla.client unit tests (debug)."
  echo "Running: ${GDB} libcarla_test_client_debug ${GTEST_ARGS} ${EXTRA_ARGS}"
  ${GDB} ${LIBCARLA_INSTALL_CLIENT_FOLDER}/test/libcarla_test_client_debug ${GTEST_ARGS} ${EXTRA_ARGS}

fi
# 如果设置了LibCarla发布版本测试标志，则运行LibCarla发布版本的单元测试
if ${LIBCARLA_RELEASE} ; then

  if ${XML_OUTPUT} ; then
    EXTRA_ARGS="--gtest_output=xml:${CARLA_TEST_RESULTS_FOLDER}/libcarla-release.xml"
  else
    EXTRA_ARGS=
  fi

  log "Running LibCarla.server unit tests (release)."
  echo "Running: ${GDB} libcarla_test_server_release ${GTEST_ARGS} ${EXTRA_ARGS}"
  LD_LIBRARY_PATH=${LIBCARLA_INSTALL_SERVER_FOLDER}/lib ${GDB} ${LIBCARLA_INSTALL_SERVER_FOLDER}/test/libcarla_test_server_release ${GTEST_ARGS} ${EXTRA_ARGS}

  if ! { ${RUN_BENCHMARK} ; }; then

    log "Running LibCarla.client unit tests (release)."
    echo "Running: ${GDB} libcarla_test_client_debug ${GTEST_ARGS} ${EXTRA_ARGS}"
    ${GDB} ${LIBCARLA_INSTALL_CLIENT_FOLDER}/test/libcarla_test_client_release ${GTEST_ARGS} ${EXTRA_ARGS}

  fi

fi

# ==============================================================================
# -- Run Python API unit tests -------------------------------------------------
# ==============================================================================
# 进入Python API单元测试文件夹
pushd "${CARLA_PYTHONAPI_ROOT_FOLDER}/test/unit" >/dev/null

if ${XML_OUTPUT} ; then
  EXTRA_ARGS="-X"
else
  EXTRA_ARGS=
fi

if ${PYTHON_API} ; then

  for PY_VERSION in ${PY_VERSION_LIST[@]} ; do

    log "Running Python API for Python ${PY_VERSION} unit tests."

    /usr/bin/env python${PY_VERSION} -m nose2 ${EXTRA_ARGS}

  done

  if ${XML_OUTPUT} ; then
    mv test-results.xml ${CARLA_TEST_RESULTS_FOLDER}/python-api-3.xml
  fi

fi
# 返回到原始目录
popd >/dev/null

# ==============================================================================
# -- Run smoke tests -----------------------------------------------------------
# ==============================================================================

if ${SMOKE_TESTS} ; then
  pushd "${CARLA_PYTHONAPI_ROOT_FOLDER}/util" >/dev/null
    log "Checking connection with the simulator."
    for PY_VERSION in ${PY_VERSION_LIST[@]} ; do
      /usr/bin/env python${PY_VERSION} test_connection.py -p 3654 --timeout=60.0
    done
  popd >/dev/null
fi
# 进入Python API测试文件夹         
pushd "${CARLA_PYTHONAPI_ROOT_FOLDER}/test" >/dev/null

if ${XML_OUTPUT} ; then
  EXTRA_ARGS="-c smoke/unittest.cfg -X"
else
  EXTRA_ARGS=
fi

if ${SMOKE_TESTS} ; then
  smoke_list=`cat smoke_test_list.txt`
  for PY_VERSION in ${PY_VERSION_LIST[@]} ; do
    log "Running smoke tests for Python ${PY_VERSION}."
    /usr/bin/env python${PY_VERSION} -m nose2 -v ${EXTRA_ARGS} ${smoke_list}
  done

  if ${XML_OUTPUT} ; then
    mv test-results.xml ${CARLA_TEST_RESULTS_FOLDER}/smoke-tests-3.xml
  fi

fi

popd >/dev/null

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
