#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Run unit tests."

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
)

GDB=
XML_OUTPUT=false
GTEST_ARGS=`sed -e 's/#.*$//g' ${CARLA_ROOT_FOLDER}/.gtest | sed -e '/^[[:space:]]*$/!s/^/--/g' | sed -e ':a;N;$!ba;s/\n/ /g'`
LIBCARLA_RELEASE=false
LIBCARLA_DEBUG=false
SMOKE_TESTS=false
PYTHON_API=false
RUN_BENCHMARK=false

OPTS=`getopt -o h --long help,gdb,xml,gtest_args:,all,libcarla-release,libcarla-debug,python-api,smoke,benchmark,python-version:, -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

source $(dirname "$0")/Environment.sh

PY_VERSION_LIST=3

# 这是一个while循环，它会一直执行，直到所有的命令行参数都被处理完毕（$# -gt 0 表示参数个数大于0）。
while [[ $# -gt 0 ]]; do
  # case语句用于根据$1（第一个参数）的值执行不同的代码块。
  case "$1" in
    # 如果参数是--gdb，则设置GDB变量为"gdb --args"，并移除第一个参数以便处理下一个。
    --gdb )
      GDB="gdb --args";
      shift ;;
    
    # 如果参数是--xml，则设置XML_OUTPUT为true，并创建一个用于存储测试结果的文件夹。
    --xml )
      XML_OUTPUT=true;
      # 使用mkdir -p命令创建测试结果文件夹，如果文件夹已存在则不会报错。
      mkdir -p "${CARLA_TEST_RESULTS_FOLDER}"
      shift ;;
    
    # 如果参数是--gtest_args，则把下一个参数（$2）赋值给GTEST_ARGS，并移除这两个参数。
    --gtest_args )
      GTEST_ARGS="$2";
      shift 2 ;;
    
    # 如果参数是--all，则设置LIBCARLA_RELEASE、LIBCARLA_DEBUG和PYTHON_API为true。
    --all )
      LIBCARLA_RELEASE=true;
      LIBCARLA_DEBUG=true;
      PYTHON_API=true;
      shift ;;
    
    # 如果参数是--libcarla-release，则设置LIBCARLA_RELEASE为true。
    --libcarla-release )
      LIBCARLA_RELEASE=true;
      shift ;;
    
    # 如果参数是--libcarla-debug，则设置LIBCARLA_DEBUG为true。
    --libcarla-debug )
      LIBCARLA_DEBUG=true;
      shift ;;
    
    # 如果参数是--smoke，则设置SMOKE_TESTS为true。
    --smoke )
      SMOKE_TESTS=true;
      shift ;;
    
    # 如果参数是--python-api，则设置PYTHON_API为true。
    --python-api )
      PYTHON_API=true;
      shift ;;
    
    # 如果参数是--benchmark，则设置LIBCARLA_RELEASE和RUN_BENCHMARK为true，并设置GTEST_ARGS为"--gtest_filter=benchmark*"。
    --benchmark )
      LIBCARLA_RELEASE=true;
      RUN_BENCHMARK=true;
      GTEST_ARGS="--gtest_filter=benchmark*";
      shift ;;
    
    # 如果参数是--python-version，则把下一个参数（$2）赋值给PY_VERSION_LIST，并移除这两个参数。
    --python-version )
      PY_VERSION_LIST="$2"
      shift 2 ;;
    
    # 如果参数是-h或--help，则打印帮助信息并退出脚本。
    -h | --help )
      echo "$DOC_STRING"
      echo -e "$USAGE_STRING"
      exit 1
      ;;
    
    # 如果参数不匹配上述任何一个，则简单地移除第一个参数以便处理下一个（这里可能是一个错误处理或忽略未知参数的机制）。
    * )
      shift ;;
  esac
done

if ! { ${LIBCARLA_RELEASE} || ${LIBCARLA_DEBUG} || ${PYTHON_API} || ${SMOKE_TESTS}; }; then
  fatal_error "Nothing selected to be done."
fi

# Convert comma-separated string to array of unique elements.
IFS="," read -r -a PY_VERSION_LIST <<< "${PY_VERSION_LIST}"

# ==============================================================================
# -- Download Content need it by the tests -------------------------------------
# ==============================================================================

if { ${LIBCARLA_RELEASE} || ${LIBCARLA_DEBUG}; }; then

  CONTENT_TAG=0.1.4

  mkdir -p ${LIBCARLA_TEST_CONTENT_FOLDER}
  pushd "${LIBCARLA_TEST_CONTENT_FOLDER}" >/dev/null

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
