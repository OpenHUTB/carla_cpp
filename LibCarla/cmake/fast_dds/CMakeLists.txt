cmake_minimum_required(VERSION 3.5.1)  # 设置CMake的最低版本要求
project(libcarla_fastdds)  # 定义项目名称为libcarla_fastdds

# 搜索指定路径下的所有头文件，并将其存储在libcarla_carla_fastdds_headers变量中
file(GLOB libcarla_carla_fastdds_headers
  "${libcarla_source_pathcarla/ros2/publishers/*.h"    # 查找发布者相关的头文件
  "${libcarla_source_path}/carla/ros2/subscribers/*.h"   # 查找订阅者相关的头文件
  "${libcarla_source_pathcarla/ros2/listeners/*.h"     # 查找监听者相关的头文件
  "${libcarla_source_pathcarla/ros2/types/*.h"         # 查找类型相关的头文件
)

# 将找到的头文件安装到指定的目录
install(FILES ${libcarla_carla_fastdds_headers DESTINATION include/carla/ros2)

# 查找Fast DDS动态库的文件，并将其存储在fast_dds_dependencies变量中
file(GLOB fast_dds_dependencies "${FASTDDS_LIB_PATH/*.so*")  # 匹配所有以.so开头的文件

# 将找到的动态库文件安装到lib目录
install(FILES ${fast_dds_dependencies DESTINATION lib)

# 使用GLOB命令查找指定目录下的所有源文件，并将其存储在libcarla_fastdds_sources变量中
file(GLOB libcarla_fastdds_sources
  "${libcarla_source_pathcarla/ros2/publishers/*.cpp"   # 查找发布者相关的源文件
  "${libcarla_source_pathcarla/ros2/subscribers/*.cpp"  # 查找订阅者相关的源文件
  "${libcarla_source_pathcarla/ros2/listeners/*.cpp"    # 查找监听者相关的源文件
  "${libcarla_source_pathcarla/ros2/types/*.cpp"        # 查找类型相关的源文件
)

# 设置C++标准库头文件的路径
set(FASTDDS_CPP_STD_INCLUDES "/usr/include/c++/7")

# 设置CMake的编译选项
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -I${FASTDDS_CPP_STD_INCLUDES CACHE STRING "" FORCE)
# -O3: 优化等级，表示进行最高级别的优化
# -fPIC: 生成位置无关代码，适用于共享库
# -D_GLIBCXX_USE_CXX11_ABI=0: 使用旧的C++ ABI，兼容性设置
# -I${FASTDDS_CPP_STD_INCLUDES 指定头文件搜索的路径
# CACHE STRING "" FORCE: 强制更新缓存中的值，确保使用这个选项
if (LIBCARLA_BUILD_RELEASE)
  # 添加一个静态库目标，命名为carla_fastdds，使用之前定义的源文件
  add_library(carla_fastdds STATIC ${libcarla_fastdds_sources  # 设置包含目录，标记为SYSTEM减少警告，不支持该路径的许可
  target_include_directories(carla_fastdds SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH         # 引入Boost库的包含目录
      "${RPCLIB_INCLUDE_PATH       # 引入RPC库的包含目录

  # 添加Fast DDS的包含目录，标记为私有，只能被当前目标使用
  target_include_directories(carla_fastdds PRIVATE "${FASTDDS_INCLUDE_PATH  # 添加libcarla源代码的ros2子目录的包含目录，标记为私有
  target_include_directories(carla_fastdds PRIVATE "${libcarla_source_pathcarla/ros2")

  # 链接依赖库，包含Fast DDS相关的库
  target_link_libraries(carla_fastdds fastrtps fastcdr "${FAST_DDS_LIBRARIES  # 设置安装规则，将carla_fastdds库安装到目标目录lib中
  install(TARGETS carla_fastdds DESTINATION lib)

  # 为目标设置编译选项，使用之前定义的CMAKE_CXX_FLAGS_RELEASE
  set_target_properties(carla_fastdds PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE```


endif()  # 结束之前的条件判断块（LIBCARLA_BUILD_RELEASE）

# 如果启用了调试版本构建
if (LIBCARLA_BUILD_DEBUG)
  # 添加一个静态库目标，命名为carla_fastdds_debug，使用之前定义的源文件
  add_library(carla_fastdds_debug STATIC ${libcarla_fastdds_sources  # 设置包含目录，标记为SYSTEM以减少编译警告
  target_include_directories(carla_fastdds_debug SYSTEM PRIVATE# SYSTEM 关键字在这里有特殊含义，当指定了这个关键字后，告诉 CMake 和编译器将添加的这些包含目录视为系统头文件所在的目录。
  # 这样做的好处在于，编译器可能会对来自这些“系统头文件目录”的头文件在处理警告等方面采用不同的策略。
      "${BOOST_INCLUDE_PATH         # 引入Boost库的包含目录
      "${RPCLIB_INCLUDE_PATH       # 引入RPC库的包含目录

  # 将调试版的库安装到目标目录lib中
  install(TARGETS carla_fastdds_debug DESTINATION lib)

  # 设置目标的编译选项，使用调试版本的编译标志
  set_target_properties(carla_fastdds_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG  # 定义编译时的预处理器宏，启用Boost库的ASIO调试模式
  target_compile_definitions(carla_fastdds_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)# 这里定义了 -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING 宏，它是针对 Boost 库中的 ASIO 模块（常用于网络编程等异步 I/O 操作场景）的，
  # 当定义了这个宏后，在编译时会开启 ASIO 模块中与缓冲区相关的调试功能，比如可能会输出更多关于缓冲区操作的日志、进行额外的错误检查等，
  # 方便在调试涉及到 ASIO 模块使用的代码时，更清晰地了解缓冲区层面的运行情况以及排查可能出现的问题。
endif()  # 结束调试版本构建的条件判断块
