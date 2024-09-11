cmake_minimum_required(VERSION 3.5.1)# 指定CMake的最低版本要求
project(libcarla_fastdds)# 设置项目名称和版本


# 安装头文件  
# 使用file(GLOB)命令收集所有相关的头文件，并将它们安装到include/carla/ros2目录下
file(GLOB libcarla_carla_fastdds_headers
  "${libcarla_source_path}/carla/ros2/publishers/*.h"
  "${libcarla_source_path}/carla/ros2/subscribers/*.h"
  "${libcarla_source_path}/carla/ros2/listeners/*.h"
  "${libcarla_source_path}/carla/ros2/types/*.h"
  )
install(FILES ${libcarla_carla_fastdds_headers} DESTINATION include/carla/ros2)
# 安装Fast DDS的动态链接库文件
file(GLOB fast_dds_dependencies "${FASTDDS_LIB_PATH}/*.so*")
install(FILES ${fast_dds_dependencies} DESTINATION lib)

# 收集libcarla_fastdds的源文件
file(GLOB libcarla_fastdds_sources
  "${libcarla_source_path}/carla/ros2/publishers/*.cpp"
  "${libcarla_source_path}/carla/ros2/subscribers/*.cpp"
  "${libcarla_source_path}/carla/ros2/listeners/*.cpp"
  "${libcarla_source_path}/carla/ros2/types/*.cpp")
# 设置C++标准库头文件路径和编译选项
set(FASTDDS_CPP_STD_INCLUDES "/usr/include/c++/7")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -I${FASTDDS_CPP_STD_INCLUDES}" CACHE STRING "" FORCE)
# 根据是否启用发布或调试构建，创建相应的目标
# ==============================================================================
# Create targets for debug and release in the same build type.
# ==============================================================================
# 如果启用了发布版本构建
if (LIBCARLA_BUILD_RELEASE)
# 添加一个静态库目标
  add_library(carla_fastdds STATIC ${libcarla_fastdds_sources})
   # 设置包含目录
  target_include_directories(carla_fastdds SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")
   # 添加Fast DDS和其他私有包含目录
  target_include_directories(carla_fastdds PRIVATE "${FASTDDS_INCLUDE_PATH}")
  target_include_directories(carla_fastdds PRIVATE "${libcarla_source_path}/carla/ros2")
  # 链接依赖库
  target_link_libraries(carla_fastdds fastrtps fastcdr "${FAST_DDS_LIBRARIES}")
  # 安装库到lib目录
  install(TARGETS carla_fastdds DESTINATION lib)
  set_target_properties(carla_fastdds PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

endif()
# 如果启用了调试版本构建
if (LIBCARLA_BUILD_DEBUG)
 # 添加一个静态库目标（调试版）
  add_library(carla_fastdds_debug STATIC ${libcarla_fastdds_sources})# 设置包含目录
 target_include_directories(carla_fastdds_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")
      # 安装调试版库到lib目录 
  install(TARGETS carla_fastdds_debug DESTINATION lib)
  set_target_properties(carla_fastdds_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")# 设置编译标志
  target_compile_definitions(carla_fastdds_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)# 启用Boost的ASIO调试

endif()
