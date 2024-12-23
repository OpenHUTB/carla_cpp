# ==============================================================================
# LibCarla的单元测试配置
# ==============================================================================
cmake_minimum_required(VERSION 3.5.1)
project(libcarla-unit-tests)

# ==============================================================================
# 构建类型配置
# ==============================================================================

# 进行客户端的测试
if (CMAKE_BUILD_TYPE STREQUAL "Client")
  set(carla_config client)
# 进行服务端的测试
elseif (CMAKE_BUILD_TYPE STREQUAL "Server")
  set(carla_config server)
endif()

# 是否构建责任敏感安全库
# RSS (Responsibility-Sensitive Safety)变体构建配置
if (BUILD_RSS_VARIANT)
  set(carla_target_postfix "_rss")
else()
  set(carla_target_postfix "")
endif()

# ==============================================================================
# 依赖库路径配置
# ==============================================================================

# 需要添加到链接库搜索路径的目录：
# 远程过程调用(Remote Procedure Call, RPC)、Google Test 库
link_directories(
    ${RPCLIB_LIB_PATH}     # RPC库路径
    ${GTEST_LIB_PATH})     # Google Test库路径

# ==============================================================================
# 源文件收集配置
# ==============================================================================

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

# ==============================================================================
# 构建目标配置
# ==============================================================================

# 如果LibCarla使用调试版本，则libcarla_test也使用调试版本
if (LIBCARLA_BUILD_DEBUG)
  list(APPEND build_targets libcarla_test_${carla_config}_debug)
endif()

# 如果LibCarla使用发布版本，则libcarla_test也使用发布版本
if (LIBCARLA_BUILD_RELEASE)
  list(APPEND build_targets libcarla_test_${carla_config}_release)
endif()

# 以相同的构建类型创建调试和发布的目标
foreach(target ${build_targets})
  # 创建可执行文件目标
  add_executable(${target} ${libcarla_test_sources})

  # 使用target_compile_definitions命令为目标添加编译时的定义
  # PUBLIC关键字表示这些定义不仅在当前目标内可用，而且对于依赖这个目标的其他目标也是可见的
  target_compile_definitions(${target} PUBLIC
      -DLIBCARLA_ENABLE_PROFILER    # 启用性能分析器
      -DLIBCARLA_WITH_GTEST)        # 启用Google Test支持

  # 为指定的目标添加系统头文件搜索路径，这些路径仅对当前目标私有
  # SYSTEM关键字告诉编译器这些是系统头文件，可以避免某些编译警告
  # PRIVATE关键字表示这些包含路径只对当前目标有效，不会传播给依赖它的其他目标
  target_include_directories(${target} SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"        # Boost库头文件路径
      "${RPCLIB_INCLUDE_PATH}"       # RPC库头文件路径
      "${GTEST_INCLUDE_PATH}"        # Google Test头文件路径
      "${LIBPNG_INCLUDE_PATH}")      # PNG库头文件路径
      
  # 添加测试源码目录到包含路径
  target_include_directories(${target} PRIVATE
      "${libcarla_source_path}/test")

  # 根据操作系统类型选择不同的链接库方式
  if (WIN32)
    # 如果是在Windows平台上编译，则直接链接预编译好的静态库文件
    target_link_libraries(${target} 
        "gtest_main.lib"
        "gtest.lib"
        "rpc.lib")
  else()
    # 如果不是在Windows平台（例如Linux或macOS），则使用链接器命令行参数来链接库
    target_link_libraries(${target} 
        "-lrpc"
        "-lgtest_main"
        "-lgtest")
  endif()

  # 定义安装规则，用于当执行`make install`或等效命令时将构建结果安装到系统中
  # OPTIONAL表示如果安装过程中出现问题，安装过程不会报错停止
  install(TARGETS ${target} DESTINATION test OPTIONAL)
endforeach(target)

# ==============================================================================
# 调试版本特定配置
# ==============================================================================

# 构建调试版本
if (LIBCARLA_BUILD_DEBUG)
  # 设置调试版本特定的编译选项
  set_target_properties(libcarla_test_${carla_config}_debug 
      PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  
  # 链接调试版本的Carla库
  target_link_libraries(libcarla_test_${carla_config}_debug 
      "carla_${carla_config}${carla_target_postfix}_debug")
  
  # 启用Boost.Asio的调试功能
  target_compile_definitions(libcarla_test_${carla_config}_debug 
      PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)
  
  # 客户端特定库链接
  if (CMAKE_BUILD_TYPE STREQUAL "Client")
      target_link_libraries(libcarla_test_${carla_config}_debug 
          "${BOOST_LIB_PATH}/libboost_filesystem.a")
  endif()
endif()

# ==============================================================================
# 发布版本特定配置
# ==============================================================================

# 构建发布版本
if (LIBCARLA_BUILD_RELEASE)
  # 设置发布版本特定的编译选项
  set_target_properties(libcarla_test_${carla_config}_release 
      PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
  
  # 链接发布版本的Carla库
  target_link_libraries(libcarla_test_${carla_config}_release 
      "carla_${carla_config}${carla_target_postfix}")
  
  # 客户端特定库链接
  # 检查CMAKE_BUILD_TYPE是否等于"Client"
  if (CMAKE_BUILD_TYPE STREQUAL "Client")# 如果等于"Client"，则将libboost_filesystem.a库链接到libcarla_test_${carla_config}_release目标上
    # 这里的${BOOST_LIB_PATH}应该是指向Boost库文件所在路径的变量
      target_link_libraries(libcarla_test_${carla_config}_release 
          "${BOOST_LIB_PATH}/libboost_filesystem.a")
  endif()
endif()
