cmake_minimum_required(VERSION 3.5.1)  # 设置CMake的最低版本要求为3.5.1
project(libcarla_fastdds)  # 定义项目名称为libcarla_fastdds

# 搜索指定路径下的所有头文件，并将其存储在libcarla_carla_fastdds_headers变量中
# 注意：这里有几个路径中可能存在拼写错误，例如缺少斜杠"/"
file(GLOB libcarla_carla_fastdds_headers
  "${libcarla_source_path}/carla/ros2/publishers/*.h"    # 查找发布者相关的头文件
  "${libcarla_source_path}/carla/ros2/subscribers/*.h"   # 查找订阅者相关的头文件
  "${libcarla_source_path}/carla/ros2/listeners/*.h"     # 查找监听者相关的头文件
  "${libcarla_source_path}/carla/ros2/types/*.h"         # 查找类型相关的头文件
)

# 将找到的头文件安装到指定的目录
install(FILES ${libcarla_carla_fastdds_headers DESTINATION include/carla/ros2)

# 查找Fast DDS动态库的文件，并将其存储在fast_dds_dependencies变量中
file(GLOB fast_dds_dependencies "${FASTDDS_LIB_PATH}/*.so*")  # 匹配所有以.so开头的文件

# 将找到的动态库文件安装到lib目录
install(FILES ${fast_dds_dependencies DESTINATION lib)

# 使用GLOB命令查找指定目录下的所有源文件，并将其存储在libcarla_fastdds_sources变量中
file(GLOB libcarla_fastdds_sources
  "${libcarla_source_path}/carla/ros2/publishers/*.cpp"   # 查找发布者相关的源文件
  "${libcarla_source_path}/carla/ros2/subscribers/*.cpp"  # 查找订阅者相关的源文件
  "${libcarla_source_path}/carla/ros2/listeners/*.cpp"    # 查找监听者相关的源文件
  "${libcarla_source_path}/carla/ros2/types/*.cpp"        # 查找类型相关的源文件
)

# 设置C++标准库头文件的路径，这里指定了特定的GCC版本路径，可能需要根据实际情况调整
set(FASTDDS_CPP_STD_INCLUDES "/usr/include/c++/7")

# 设置CMake的编译选项，用于发布版本
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -I${FASTDDS_CPP_STD_INCLUDES} CACHE STRING "" FORCE)

# 如果定义了LIBCARLA_BUILD_RELEASE，则构建发布版本的静态库
if (LIBCARLA_BUILD_RELEASE)
  add_library(carla_fastdds STATIC ${libcarla_fastdds_sources})  # 添加静态库目标
  # 注意：下面的target_include_directories调用中缺少闭合的括号和引号
  target_include_directories(carla_fastdds SYSTEM PRIVATE "${BOOST_INCLUDE_PATH}" "${RPCLIB_INCLUDE_PATH}")
  target_include_directories(carla_fastdds PRIVATE "${FASTDDS_INCLUDE_PATH}" "${libcarla_source_path}/carla/ros2")
  target_link_libraries(carla_fastdds fastrtps fastcdr "${FAST_DDS_LIBRARIES}")  # 链接依赖库
  install(TARGETS carla_fastdds DESTINATION lib)  # 安装库到lib目录
  set_target_properties(carla_fastdds PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")  # 设置编译选项
endif()

# 如果定义了LIBCARLA_BUILD_DEBUG，则构建调试版本的静态库
if (LIBCARLA_BUILD_DEBUG)
  add_library(carla_fastdds_debug STATIC ${libcarla_fastdds_sources})  # 添加静态库目标
  target_include_directories(carla_fastdds_debug SYSTEM PRIVATE "${BOOST_INCLUDE_PATH}" "${RPCLIB_INCLUDE_PATH}")
  install(TARGETS carla_fastdds_debug DESTINATION lib)  # 安装库到lib目录
  # 注意：这里缺少CMAKE_CXX_FLAGS_DEBUG的定义，需要定义调试版本的编译标志
  set_target_properties(carla_fastdds_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")  # 设置编译选项
  target_compile_definitions(carla_fastdds_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)  # 启用Boost ASIO的调试模式
endif()
