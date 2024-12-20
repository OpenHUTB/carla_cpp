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
  target_include_directories(carla_fastdds PRIVATE "${libcarla_source_path}/carla/ros2")# 将指定路径 "${libcarla_source_path}/carla/ros2" 添加为名为 carla_fastdds 这个目标的私有包含目录。
# 这样在编译与 carla_fastdds 相关代码时，编译器就能在该目录下查找头文件等，PRIVATE 表示此包含目录仅对 carla_fastdds 自身的编译起作用。
  # 链接依赖库
  target_link_libraries(carla_fastdds fastrtps fastcdr "${FAST_DDS_LIBRARIES}")
  # 安装库到lib目录
  install(TARGETS carla_fastdds DESTINATION lib)// 使用CMake的install命令来进行安装相关的配置操作。
// TARGETS关键字表明后面跟着的是要安装的目标，此处目标为carla_fastdds，通常指的是一个编译生成的库（比如动态库或静态库等）。
// DESTINATION关键字用于指定安装的目标路径，这里将carla_fastdds安装到lib目录下，意味着在执行安装过程后，carla_fastdds这个目标文件会被放置到系统或指定环境中的lib目录中，方便后续其他项目引用等操作。
  set_target_properties(carla_fastdds PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

endif()
# 如果启用了调试版本构建
if (LIBCARLA_BUILD_DEBUG)
 # 添加一个静态库目标（调试版）
  add_library(carla_fastdds_debug STATIC ${libcarla_fastdds_sources})# 设置包含目录
 target_include_directories(carla_fastdds_debug SYSTEM PRIVATE# 使用target_include_directories命令，针对名为carla_fastdds_debug的目标（通常是一个库或者可执行文件目标，在Debug配置下）进行操作。
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")
      # 安装调试版库到lib目录 
  install(TARGETS carla_fastdds_debug DESTINATION lib)
  set_target_properties(carla_fastdds_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")# 设置编译标志
  target_compile_definitions(carla_fastdds_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)# 启用Boost的ASIO调试

endif()
