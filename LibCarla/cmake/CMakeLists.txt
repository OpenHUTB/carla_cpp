# 指定最低要求的CMake版本为3.5.1
cmake_minimum_required(VERSION 3.5.1)

# 定义项目名称为libcarla
project(libcarla)

# 定义构建选项，LIBCARLA_BUILD_DEBUG用于控制是否启用调试构建，默认值为ON
option(LCARLA_BUILD_DEBUG "Build debug configuration" ON)

# 定义构建选项，LIBCARLA_BUILD_RELEASE用于控制是否启用发布构建，默认值为ON
option(LIBCARLA_BUILD_RELEASE "Build release configuration" ON)

# 定义构建选项，LIBCARLA_BUILD_TEST用于控制是否构建单元测试，默认值为ON
option(LIBCARLA_BUILD_TEST "Build unit tests" ON)

# 输出构建选项的当前状态信息到控制台
message(STATUS "Build debug:   ${LIBCARLA_BUILD_DEBUGmessage(STATUS "Build release: ${LIBCARLA_BUILD_RELEASEmessage(STATUS "Build test:    ${LIBCARLA_BUILD_TEST# 设置源文件路径，指向项目根目录的上一级的source目录
set(libcarla_source_path "${PROJECT_SOURCE_DIR../source")

# 设置第三方库路径，指向libcarla_source_path的third-party子目录
set(libcarla_source_thirdparty_path "${libcarla_source_paththird-party")

#libcarla_source作为包含目录，以便编译时能够找到头文件
include_direct(${libcarla_source_path# 添加libla_source_third_path作为包含目录，以便编译时能够找到第三方库的头
include_directorieslibcarla_source_thparty_path# 如果定义了CARLA，则生成Version.h文件if (CARLA)
  # 使用configure_file命令将Version.h.in文件配置成Version文件，替换的占位符
 configure_file(${libcarla_source_pathla/Version.h.inlibcarla_source_pathcarla/Version)
endif()
# 根据 CMAKE_BUILD_TYPE 参数的值，添加相应的子目录
if (CMAKE_BUILD_TYPE STREQUAL "Client")
  # 如果构建类型是 Client，则添加 client 子目录
  add_subdirectory("client")
elseif (CMAKE_BUILD_TYPE STREQUAL "Server")
  # 如果构建类型是 Server，则添加 server 子目录
  add_subdirectory("server")
elseif (CMAKE_BUILD_TYPE STREQUAL "Pytorch")
  # 如果构建类型是 Pytorch，则添加 pytorch 子目录
  add_subdirectory("pytorch")
elseif (CMAKE_BUILD_TYPE STREQUAL "ros2")
  # 如果构建类型是 ros2，则添加 fast_dds 子目录
  add_subdirectory("fast_dds")
else ()
  # 如果构建类型不是上述任一类型，则输出错误信息并终止配置
  message(FATAL_ERROR "Unknown build type '${CMAKE_BUILD_TYPE")
endif ()

# 只有在满足特定条件下才添加测试模块
if ((LIBCARLA_BUILD_TEST) AND (NOT WIN32) AND (NOT (CMAKE_BUILD_TYPE STREQUAL "Pytorch")) AND (NOT (CMAKE_BUILD_TYPE STREQUAL "ros2")))
  # 如果启用了 LIBCARLA_BUILD_TEST，并且不是 Windows 平台，且构建类型不是 Pytorch ros2，则添加 子目录
  add_subdirectory("test")
endif()
