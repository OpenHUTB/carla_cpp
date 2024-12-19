# LibCarla的单元测试
cmake_minimum_required(VERSION 3.5.1)
project(libcarla-unit-tests)

# 进行客户端的测试
if (CMAKE_BUILD_TYPE STREQUAL "Client")
  set(carla_config client)
# 进行服务端的测试
elseif (CMAKE_BUILD_TYPE STREQUAL "Server")
  set(carla_config server)
endif ()
# 是否构建责任敏感安全库
if (BUILD_RSS_VARIANT)
  set(carla_target_postfix "_rss")
else()
  set(carla_target_postfix "")
endif()

# 需要添加到链接库搜索路径的目录：
# 远程过程调用(Remote Procedure Call, RPC)、Google Test 库
link_directories(
    ${RPCLIB_LIB_PATH}
    ${GTEST_LIB_PATH})

# GLOB 会产生一个由所有匹配表达式的文件组成的列表，并将其保存到libcarla_test_sources变量中
# 包括 性能剖析器 和 测试模块（客户端/服务端）中的所有C++代码
file(GLOB libcarla_test_sources
# 包含性能剖析器目录下的所有 C++ 源文件和头文件
    "${libcarla_source_path}/carla/profiler/*.cpp"
    "${libcarla_source_path}/carla/profiler/*.h"
    # 包含测试目录下的所有 C++ 源文件和头文件
    "${libcarla_source_path}/test/*.cpp"
    "${libcarla_source_path}/test/*.h"
     # 根据当前配置 ${carla_config} 包含特定配置下的测试源文件和头文件
    "${libcarla_source_path}/test/${carla_config}/*.cpp"
    "${libcarla_source_path}/test/${carla_config}/*.h"
    # 包含测试通用目录下的所有 C++ 源文件和头文件，这些可能是共享的测试工具或通用测试代码
    "${libcarla_source_path}/test/common/*.cpp"
    "${libcarla_source_path}/test/common/*.h")

file(GLOB libcarla_test_client_sources "")

# 如果LibCarla使用调试版本，则libcarla_test也是用调试版本
if (LIBCARLA_BUILD_DEBUG)
  list(APPEND build_targets libcarla_test_${carla_config}_debug)
endif()

# 如果LibCarla使用发布版本，则libcarla_test也是用发布版本
if (LIBCARLA_BUILD_RELEASE)
  list(APPEND build_targets libcarla_test_${carla_config}_release)
endif()

# 以相同的构建类型创建调试和发布的对象
foreach(target ${build_targets})

  add_executable(${target} ${libcarla_test_sources})

  target_compile_definitions(${target} PUBLIC
      -DLIBCARLA_ENABLE_PROFILER
      -DLIBCARLA_WITH_GTEST)# 使用target_compile_definitions命令为名为${target}的目标（也就是前面创建的可执行文件目标）添加编译时的定义。
# PUBLIC关键字表示这些定义不仅在当前目标内可用，而且对于依赖这个目标的其他目标也是可见的。
# 这里添加了两个编译定义：
# -DLIBCARLA_ENABLE_PROFILER：可能用于启用性能分析相关的功能代码，在编译时通过这个定义来控制相关代码的编译与否。
# -DLIBCARLA_WITH_GTEST：表示该项目会使用Google Test框架，让编译器知晓并相应地处理涉及GTest相关的代码逻辑。

  target_include_directories(${target} SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}"
      "${GTEST_INCLUDE_PATH}"
      "${LIBPNG_INCLUDE_PATH}")

  target_include_directories(${target} PRIVATE
      "${libcarla_source_path}/test")

  if (WIN32)
      target_link_libraries(${target} "gtest_main.lib")
      target_link_libraries(${target} "gtest.lib")
      target_link_libraries(${target} "rpc.lib")
  else()
      target_link_libraries(${target} "-lrpc")
      target_link_libraries(${target} "-lgtest_main")
      target_link_libraries(${target} "-lgtest")
  endif()

  install(TARGETS ${target} DESTINATION test OPTIONAL)# 使用install命令来指定名为${target}的目标（可执行文件目标）的安装规则。
# 表示在进行项目安装操作时，将该可执行文件安装到目标目录test下。
# OPTIONAL关键字意味着如果在安装过程中出现某些问题（比如找不到这个目标等情况），安装过程不会报错停止，而是继续进行其他部分的安装。
endforeach(target)

# 构建调试版本
if (LIBCARLA_BUILD_DEBUG)
  # Specific options for debug.
  set_target_properties(libcarla_test_${carla_config}_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  target_link_libraries(libcarla_test_${carla_config}_debug "carla_${carla_config}${carla_target_postfix}_debug")
  target_compile_definitions(libcarla_test_${carla_config}_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)
  if (CMAKE_BUILD_TYPE STREQUAL "Client")
      target_link_libraries(libcarla_test_${carla_config}_debug "${BOOST_LIB_PATH}/libboost_filesystem.a")
  endif()
endif()

# 构建发布版本
if (LIBCARLA_BUILD_RELEASE)
  # Specific options for release.
  set_target_properties(libcarla_test_${carla_config}_release PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
  target_link_libraries(libcarla_test_${carla_config}_release "carla_${carla_config}${carla_target_postfix}")
  if (CMAKE_BUILD_TYPE STREQUAL "Client")
      target_link_libraries(libcarla_test_${carla_config}_release "${BOOST_LIB_PATH}/libboost_filesystem.a")
  endif()
endif()
