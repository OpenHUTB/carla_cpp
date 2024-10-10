cmake_minimum_required(VERSION 3.5.1)# 设置 CMake 最低版本要求为 3.5.1
project(libcarla)# 定义项目名称为 libcarla

# 定义构建选项，用户可以选择是否启用调试、发布和测试构建
option(LIBCARLA_BUILD_DEBUG "Build debug configuration" ON)
option(LIBCARLA_BUILD_RELEASE "Build release configuration" ON)
option(LIBCARLA_BUILD_TEST "Build unit tests" ON)

# 显示当前构建选项的状态
message(STATUS "Build debug:   ${LIBCARLA_BUILD_DEBUG}")
message(STATUS "Build release: ${LIBCARLA_BUILD_RELEASE}")
message(STATUS "Build test:    ${LIBCARLA_BUILD_TEST}")

# 设置源码路径和第三方库路径
set(libcarla_source_path "${PROJECT_SOURCE_DIR}/../source")
set(libcarla_source_thirdparty_path "${libcarla_source_path}/third-party")

# 如果定义了 CARLA_VERSION，则配置文件 Version.h
# 包含源码目录和第三方库目录
include_directories(${libcarla_source_path})
include_directories(${libcarla_source_thirdparty_path})

# 根据构建类型选择要添加的子目录
if (CARLA_VERSION)
  configure_file(${libcarla_source_path}/carla/Version.h.in ${libcarla_source_path}/carla/Version.h)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Client")
  add_subdirectory("client")
elseif (CMAKE_BUILD_TYPE STREQUAL "Server")
  add_subdirectory("server")
elseif (CMAKE_BUILD_TYPE STREQUAL "Pytorch")
add_subdirectory("pytorch")
elseif (CMAKE_BUILD_TYPE STREQUAL "ros2")
add_subdirectory("fast_dds")
else ()
 # 如果构建类型未知，则抛出致命错误
  message(FATAL_ERROR "Unknown build type '${CMAKE_BUILD_TYPE}'")
endif ()

# 如果启用了测试构建且不是 Windows 平台，并且构建类型不是 Pytorch 或 ros2，则添加测试子目录
if ((LIBCARLA_BUILD_TEST) AND (NOT WIN32) AND (NOT (CMAKE_BUILD_TYPE STREQUAL "Pytorch")) AND (NOT (CMAKE_BUILD_TYPE STREQUAL "ros2")))
  add_subdirectory("test")
endif()
