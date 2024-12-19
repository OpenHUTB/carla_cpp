cmake_minimum_required(VERSION 3.5.1)# 指定 CMake 最低版本要求为 3.5.1，意味着该 CMakeLists.txt 文件需要在这个版本及以上的 CMake 环境中才能正确运行。
project(libcarla-server)

# 安装 libc++ 共享库。
# 使用 file(GLOB...) 命令查找指定路径（由 ${LLVM_LIB_PATH} 变量指定路径）下所有以 libc++ 开头的文件，
# 将找到的文件路径列表存储在 LibCXX_Libraries 变量中，以便后续安装使用。
file(GLOB LibCXX_Libraries "${LLVM_LIB_PATH}/libc++*")
# DESTINATION给出相对路径时，它将相对于 CMAKE_INSTALL_PREFIX 变量（Unreal\CarlaUE4\Plugins\Carla\CarlaDependencies）
install(FILES ${LibCXX_Libraries} DESTINATION lib){LLVM_LIB_PATH}/libc++*")
# install 命令用于将指定的文件安装到目标位置。

# 安装远程过程调用库 rpclib
install(DIRECTORY "${RPCLIB_INCLUDE_PATH}/rpc" DESTINATION include)# 将找到的文件路径列表存储在 libcarla_carla_rpclib 变量中，用于后续安装。
file(GLOB libcarla_carla_rpclib "${RPCLIB_LIB_PATH}/*.*")# 将之前找到的 libcarla_carla_rpclib 变量中的文件安装到目标路径下的 lib 目录中。
install(FILES ${libcarla_carla_rpclib} DESTINATION lib)

# 安装头文件

install(DIRECTORY "${libcarla_source_path}/compiler" DESTINATION include)

file(GLOB libcarla_carla_headers "${libcarla_source_path}/carla/*.h")# 将之前找到的 libcarla_carla_headers 变量中的头文件安装到目标路径下的 include/carla 目录中，
# 以便在使用该项目时能正确引用这些头文件。
install(FILES ${libcarla_carla_headers} DESTINATION include/carla)# 使用 file(GLOB...) 命令查找 ${libcarla_source_path}/carla/geom 目录下所有以.h 为扩展名的头文件，

file(GLOB libcarla_carla_geom_headers "${libcarla_source_path}/carla/geom/*.h")# 使用GLOB命令查找指定路径下（${libcarla_source_path}/carla/geom/）所有以.h为后缀的头文件，
# 并将找到的文件列表存储在变量libcarla_carla_geom_headers中。
# GLOB命令用于生成符合特定模式的文件列表，方便后续操作使用这些文件。
install(FILES ${libcarla_carla_geom_headers} DESTINATION include/carla/geom)# 使用install命令将之前通过GLOB获取到的头文件列表（${libcarla_carla_geom_headers}）中的文件
# 安装到目标目录（include/carla/geom）下。
# 这样在进行项目安装等操作时，这些头文件就能被正确放置到指定的安装路径中，便于其他项目引用这些头文件。

file(GLOB libcarla_carla_opendrive "${libcarla_source_path}/carla/opendrive/*.h")# 使用GLOB命令在指定的路径（${libcarla_source_path}/carla/opendrive/）下查找所有以.h为后缀的文件，
# 并把查找到的这些文件的列表存放到名为libcarla_carla_opendrive的变量当中。
# GLOB是CMake中用于文件路径匹配和收集的便捷指令，方便后续基于这些收集到的文件进行其他操作。
install(FILES ${libcarla_carla_opendrive} DESTINATION include/carla/opendrive)# 通过install命令将变量libcarla_carla_opendrive所代表的文件列表中的所有文件，
# 安装到目标安装目录（include/carla/opendrive）下。
# 这样在整个项目进行安装过程时，相应的头文件就能被准确放置在期望的位置，
# 便于其他可能依赖该项目的代码在需要时能顺利找到并包含这些头文件。

# 使用file(GLOB)命令查找位于${libcarla_source_path}/carla/opendrive/parser/目录下所有的.h文件，
# 并将查找到的这些.h文件的路径列表存储到变量libcarla_carla_opendrive_parser中。
# GLOB是一种文件通配符查找机制，在这里用于快速定位特定目录下符合指定后缀（.h）的文件。
file(GLOB libcarla_carla_opendrive_parser "${libcarla_source_path}/carla/opendrive/parser/*.h")
# 使用install命令将之前通过file(GLOB)查找到并存储在变量libcarla_carla_opendrive_parser中的头文件（.h文件），
# 安装到目标目录include/carla/opendrive/parser下。这样在整个项目进行安装构建的过程中，
# 这些头文件就能被放置在正确的位置，方便后续其他代码在编译等操作时能够顺利找到并引用它们。
install(FILES ${libcarla_carla_opendrive_parser} DESTINATION include/carla/opendrive/parser)

file(GLOB libcarla_carla_profiler_headers "${libcarla_source_path}/carla/profiler/*.h")# 使用CMake中的GLOB命令来查找符合特定模式的文件。
# 这里是在路径${libcarla_source_path}/carla/profiler/下查找所有以.h为后缀的头文件，
# 并将查找到的这些头文件的路径信息存储到变量libcarla_carla_profiler_headers中。
# GLOB命令可以方便地根据扩展名等模式快速收集一批文件，为后续的安装等操作做准备。
install(FILES ${libcarla_carla_profiler_headers} DESTINATION include/carla/profiler)

// 利用CMake中的file函数与GLOB命令来查找符合特定模式的文件。
// 这里是在由变量${libcarla_source_path}所指定的基础路径下的“carla/road/”子目录中（即完整路径为${libcarla_source_path}/carla/road/ ），
// 查找所有以“.h”为后缀的头文件。
// 然后把找到的这些头文件的路径列表存储到名为libcarla_carla_road_headers的变量中。
// GLOB命令在CMake里常用于按文件扩展名等特定模式收集一批文件，便于后续围绕这些文件开展进一步操作，比如安装、编译配置等相关操作。
file(GLOB libcarla_carla_road_headers "${libcarla_source_path}/carla/road/*.h")
// 使用install命令来设置文件的安装规则。
// 此命令会将存储在变量${libcarla_carla_road_headers}里的头文件列表中的每一个头文件，
// 安装到指定的目标目录“include/carla/road”下。
// 这样在整个项目进行安装的过程中，这些头文件就能按照预期被准确放置在对应的目录位置，
// 方便其他需要使用该库相关功能的代码（例如基于这个库开发的外部应用程序代码等）可以顺利地找到并包含这些头文件，
// 进而能够使用头文件中所声明的各种类型、函数等内容来实现相应的业务逻辑。
install(FILES ${libcarla_carla_road_headers} DESTINATION include/carla/road)

file(GLOB libcarla_carla_road_element_headers "${libcarla_source_path}/carla/road/element/*.h")
// 使用install命令将存储在变量${libcarla_carla_road_element_headers}中的头文件列表里的每一个头文件，
// 安装到指定的目标目录“include/carla/road/element”下。
// 这样在项目整体安装时，相应的头文件就能被准确放置在对应的目录位置，便于其他代码引用这些头文件中的定义来实现相关功能。
install(FILES ${libcarla_carla_road_element_headers} DESTINATION include/carla/road/element)

file(GLOB libcarla_carla_road_general_headers "${libcarla_source_path}/carla/road/general/*.h")
install(FILES ${libcarla_carla_road_general_headers} DESTINATION include/carla/road/general)

file(GLOB libcarla_carla_road_object_headers "${libcarla_source_path}/carla/road/object/*.h")
install(FILES ${libcarla_carla_road_object_headers} DESTINATION include/carla/road/object)

file(GLOB libcarla_carla_road_signal_headers "${libcarla_source_path}/carla/road/signal/*.h")
install(FILES ${libcarla_carla_road_signal_headers} DESTINATION include/carla/road/signal)

file(GLOB libcarla_carla_rpc_headers "${libcarla_source_path}/carla/rpc/*.h")
install(FILES ${libcarla_carla_rpc_headers} DESTINATION include/carla/rpc)

file(GLOB libcarla_carla_sensor_headers "${libcarla_source_path}/carla/sensor/*.h")
install(FILES ${libcarla_carla_sensor_headers} DESTINATION include/carla/sensor)

file(GLOB libcarla_carla_sensor_data_headers "${libcarla_source_path}/carla/sensor/data/*.h")
install(FILES ${libcarla_carla_sensor_data_headers} DESTINATION include/carla/sensor/data)

file(GLOB libcarla_carla_sensor_s11n_headers "${libcarla_source_path}/carla/sensor/s11n/*.h")
install(FILES ${libcarla_carla_sensor_s11n_headers} DESTINATION include/carla/sensor/s11n)

file(GLOB libcarla_carla_streaming_headers "${libcarla_source_path}/carla/streaming/*.h")
install(FILES ${libcarla_carla_streaming_headers} DESTINATION include/carla/streaming)

file(GLOB libcarla_carla_streaming_detail_headers "${libcarla_source_path}/carla/streaming/detail/*.h")
install(FILES ${libcarla_carla_streaming_detail_headers} DESTINATION include/carla/streaming/detail)

file(GLOB libcarla_carla_streaming_detail_tcp_headers "${libcarla_source_path}/carla/streaming/detail/tcp/*.h")
install(FILES ${libcarla_carla_streaming_detail_tcp_headers} DESTINATION include/carla/streaming/detail/tcp)

file(GLOB libcarla_carla_streaming_low_level_headers "${libcarla_source_path}/carla/streaming/low_level/*.h")
install(FILES ${libcarla_carla_streaming_low_level_headers} DESTINATION include/carla/streaming/low_level)

file(GLOB libcarla_carla_multigpu_headers "${libcarla_source_path}/carla/multigpu/*.h")
install(FILES ${libcarla_carla_multigpu_headers} DESTINATION include/carla/multigpu)

file(GLOB libcarla_carla_ros2_headers "${libcarla_source_path}/carla/ros2/*.h")
install(FILES ${libcarla_carla_ros2_headers} DESTINATION include/carla/ros2)

install(DIRECTORY "${BOOST_INCLUDE_PATH}/boost" DESTINATION include)

if(WIN32)
  file(GLOB boostlibs# 定义一个名为boostlibs的变量，用于存储要安装的Boost库文件路径列表。
      "${BOOST_LIB_PATH}/libboost_date_time-*-mt-*.lib"
      "${BOOST_LIB_PATH}/libboost_system-*-mt-*.lib"
      "${BOOST_LIB_PATH}/libboost_filesystem-*-mt-*.lib")
  install(FILES ${boostlibs} DESTINATION lib)
endif()

# carla_server 库。

file(GLOB libcarla_server_sources
    "${libcarla_source_path}/carla/*.h" # 收集${libcarla_source_path}/carla/目录下名为Buffer.cpp的源文件路径
    "${libcarla_source_path}/carla/Buffer.cpp" # 收集${libcarla_source_path}/carla/目录下名为Exception.cpp的源文件路径
    "${libcarla_source_path}/carla/Exception.cpp"# 收集${libcarla_source_path}/carla/geom/目录下所有以.cpp为扩展名的源文件路径
    "${libcarla_source_path}/carla/geom/*.cpp" # 收集${libcarla_source_path}/carla/geom/目录下所有以.h为扩展名的头文件路径
    "${libcarla_source_path}/carla/geom/*.h"# 收集${libcarla_source_path}/carla/opendrive/目录下所有以.cpp为扩展名的源文件路径
    "${libcarla_source_path}/carla/opendrive/*.cpp"# 收集${libcarla_source_path}/carla/opendrive/目录下所有以.h为扩展名的头文件路径
    "${libcarla_source_path}/carla/opendrive/*.h"
    "${libcarla_source_path}/carla/opendrive/parser/*.cpp"
    "${libcarla_source_path}/carla/opendrive/parser/*.h"
    "${libcarla_source_path}/carla/road/*.cpp"
    "${libcarla_source_path}/carla/road/*.h"
    "${libcarla_source_path}/carla/road/element/*.cpp"
    "${libcarla_source_path}/carla/road/element/*.h"
    "${libcarla_source_path}/carla/road/general/*.cpp"
    "${libcarla_source_path}/carla/road/general/*.h"
    "${libcarla_source_path}/carla/road/object/*.cpp"
    "${libcarla_source_path}/carla/road/object/*.h"
    "${libcarla_source_path}/carla/road/signal/*.cpp"
    "${libcarla_source_path}/carla/road/signal/*.h"
    "${libcarla_source_path}/carla/rpc/*.cpp"
    "${libcarla_source_path}/carla/rpc/*.h"
    "${libcarla_source_path}/carla/sensor/*.h"
    "${libcarla_source_path}/carla/sensor/s11n/*.h"
    "${libcarla_source_path}/carla/sensor/s11n/SensorHeaderSerializer.cpp"
    "${libcarla_source_path}/carla/streaming/*.h"
    "${libcarla_source_path}/carla/streaming/detail/*.cpp"
    "${libcarla_source_path}/carla/streaming/detail/*.h"
    "${libcarla_source_path}/carla/streaming/detail/tcp/*.cpp"
    "${libcarla_source_path}/carla/streaming/low_level/*.h"
    "${libcarla_source_path}/carla/multigpu/*.h"
    "${libcarla_source_path}/carla/multigpu/*.cpp"
    "${libcarla_source_path}/carla/ros2/*.h"
    "${libcarla_source_path}/carla/ros2/*.cpp"
    "${libcarla_source_thirdparty_path}/odrSpiral/*.cpp"
    "${libcarla_source_thirdparty_path}/odrSpiral/*.h"
    "${libcarla_source_thirdparty_path}/moodycamel/*.cpp"
    "${libcarla_source_thirdparty_path}/moodycamel/*.h"
    "${libcarla_source_thirdparty_path}/pugixml/*.cpp"
    "${libcarla_source_thirdparty_path}/pugixml/*.hpp")

# ==============================================================================
# 在同一构建类型中创建调试和发布的目标
# ==============================================================================

if (LIBCARA_BUILD_RELEASE) #设置目标服务器属性，包括编译选项和链接选项


  add_library(carla_server STATIC ${libcarla_server_sources})

  target_include_directories(carla_server SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")# 使用target_include_directories命令为名为carla_server的目标（这里就是前面创建的静态库）添加头文件包含目录。
# SYSTEM关键字表示这些目录下的头文件被视为系统头文件（在一些编译器行为上可能会有区别对待，比如抑制警告等情况）。
# PRIVATE表示这些包含目录仅对该目标（carla_server）本身可见，不会传递给依赖它的其他目标。
# 后面跟着的是具体的头文件包含目录路径，这里分别添加了BOOST_INCLUDE_PATH和RPCLIB_INCLUDE_PATH这两个路径，
# 以便在编译carla_server静态库时能正确找到对应的头文件。

  install(TARGETS carla_server DESTINATION lib OPTIONAL)

  set_target_properties(carla_server PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

endif()

if (LIBCARLA_BUILD_DEBUG)# 判断是否定义了LIBCARA_BUILD_RELEASE这个变量（通常用于标识构建发布版本），如果为真，则执行以下代码块。

  add_library(carla_server_debug STATIC ${libcarla_server_sources})  # 使用add_library命令创建一个名为carla_server的静态库

  target_include_directories(carla_server_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")
# 使用install命令将生成的carla_server静态库安装到目标路径下的lib目录中
  install(TARGETS carla_server_debug DESTINATION lib OPTIONAL)

  set_target_properties(carla_server_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  # 使用set_target_properties命令为carla_server这个静态库目标设置编译相关的属性
  target_compile_definitions(carla_server_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

endif()# 判断是否定义了LIBCARA_BUILD_DEBUG这个变量
