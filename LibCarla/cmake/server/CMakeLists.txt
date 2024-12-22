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

# 收集 carla/road/element 目录下的所有头文件，并将它们存储在变量 libcarla_carla_road_element_headers 中
file(GLOB libcarla_carla_road_element_headers "${libcarla_source_path}/carla/road/element/*.h")
// 使用install命令将存储在变量${libcarla_carla_road_element_headers}中的头文件列表里的每一个头文件，
// 安装到指定的目标目录“include/carla/road/element”下。
// 这样在项目整体安装时，相应的头文件就能被准确放置在对应的目录位置，便于其他代码引用这些头文件中的定义来实现相关功能。
install(FILES ${libcarla_carla_road_element_headers} DESTINATION include/carla/road/element)

# 类似地为其他模块收集头文件，并指定了各自的安装路径
file(GLOB libcarla_carla_road_general_headers "${libcarla_source_path}/carla/road/general/*.h")#这里使用file(GLOB...命令来查找${libcarla_source_path}/carla/road/general/目录下的所有头文件（*.h）。GLOB操作会将匹配的文件路径存储到变量libcarla_carla_road_general_headers中。
install(FILES ${libcarla_carla_road_general_headers} DESTINATION include/carla/road/general)#install(FILES...命令用于将前面通过GLOB操作获取到的文件（存储在libcarla_carla_road_general_headers变量中的文件）安装到目标目录include/carla/road/general。这样做的目的可能是为了在构建项目时，确保这些头文件被正确放置在项目结构中的相应位置，以便其他部分的代码可以引用它们。

file(GLOB libcarla_carla_road_object_headers "${libcarla_source_path}/carla/road/object/*.h")#结果存储在libcarla_carla_road_object_headers变量中。
install(FILES ${libcarla_carla_road_object_headers} DESTINATION include/carla/road/object)#将libcarla_carla_road_object_headers中的文件安装到include/carla/road/object目录。

file(GLOB libcarla_carla_road_signal_headers "${libcarla_source_path}/carla/road/signal/*.h")#查找${libcarla_source_path}/carla/road/signal/下的头文件并存入libcarla_carla_road_signal_headers。
install(FILES ${libcarla_carla_road_signal_headers} DESTINATION include/carla/road/signal)#将文件安装到include/carla/road/signal目录。

file(GLOB libcarla_carla_rpc_headers "${libcarla_source_path}/carla/rpc/*.h")#查找${libcarla_source_path}/carla/rpc/目录下的头文件，结果存入libcarla_carla_rpc_headers。
install(FILES ${libcarla_carla_rpc_headers} DESTINATION include/carla/rpc)#将头文件安装到include/carla/rpc目录。

file(GLOB libcarla_carla_sensor_headers "${libcarla_source_path}/carla/sensor/*.h")#查找${libcarla_source_path}/carla/sensor/目录下的头文件，存入libcarla_carla_sensor_headers。
install(FILES ${libcarla_carla_sensor_headers} DESTINATION include/carla/sensor)#将头文件安装到include/carla/sensor目录。

file(GLOB libcarla_carla_sensor_data_headers "${libcarla_source_path}/carla/sensor/data/*.h")#查找${libcarla_source_path}/carla/sensor/data/目录下的头文件，存入libcarla_carla_sensor_data_headers。
install(FILES ${libcarla_carla_sensor_data_headers} DESTINATION include/carla/sensor/data)#将头文件安装到include/carla/sensor/data目录。

file(GLOB libcarla_carla_sensor_s11n_headers "${libcarla_source_path}/carla/sensor/s11n/*.h")#查找${libcarla_source_path}/carla/sensor/s11n/目录下的头文件，存入libcarla_carla_sensor_s11n_headers。
install(FILES ${libcarla_carla_sensor_s11n_headers} DESTINATION include/carla/sensor/s11n)#将头文件安装到include/carla/sensor/s11n目录。

file(GLOB libcarla_carla_streaming_headers "${libcarla_source_path}/carla/streaming/*.h")#查找${libcarla_source_path}/carla/streaming/目录下的头文件，存入libcarla_carla_streaming_headers。
install(FILES ${libcarla_carla_streaming_headers} DESTINATION include/carla/streaming)#将头文件安装到include/carla/streaming目录。

file(GLOB libcarla_carla_streaming_detail_headers "${libcarla_source_path}/carla/streaming/detail/*.h")#这里的GLOB是file命令的一个操作。
install(FILES ${libcarla_carla_streaming_detail_headers} DESTINATION include/carla/streaming/detail)#file(GLOB libcarla_carla_streaming_detail_tcp_headers "${libcarla_source_path}/carla/streaming/detail/tcp/*.h"会在${libcarla_source_path}/carla/streaming/detail/tcp/目录下查找所有.h文件，并将其路径存储到libcarla_carla_streaming_detail_tcp_headers变量中。

file(GLOB libcarla_carla_streaming_detail_tcp_headers "${libcarla_source_path}/carla/streaming/detail/tcp/*.h")#使用file(GLOB...)命令。GLOB是CMake中的一个操作，它会根据指定的通配符模式查找文件。
install(FILES ${libcarla_carla_streaming_detail_tcp_headers} DESTINATION include/carla/streaming/detail/tcp)#使用install(FILES...)命令。install命令用于指定在安装项目时要执行的操作。这里它将第104行找到的头文件（存储在${libcarla_carla_streaming_detail_tcp_headers}变量中的文件）安装到include/carla/streaming/detail/tcp目录下。

file(GLOB libcarla_carla_streaming_low_level_headers "${libcarla_source_path}/carla/streaming/low_level/*.h")#查找${libcarla_source_path}/carla/streaming/low_level/目录下所有的.h文件，并将文件路径存储到变量libcarla_carla_streaming_low_level_headers中。
install(FILES ${libcarla_carla_streaming_low_level_headers} DESTINATION include/carla/streaming/low_level)#将头文件安装到include/carla/streaming/low_level目录下。

file(GLOB libcarla_carla_multigpu_headers "${libcarla_source_path}/carla/multigpu/*.h")#file(GLOB libcarla_carla_multigpu_headers "${libcarla_source_path}/carla/multigpu/*.h"查找${libcarla_source_path}/carla/multigpu/目录下的所有.h文件，并将文件路径存储到变量libcarla_carla_multigpu_headers中。
install(FILES ${libcarla_carla_multigpu_headers} DESTINATION include/carla/multigpu)#将头文件安装到include/carla/multigpu目录下。

file(GLOB libcarla_carla_ros2_headers "${libcarla_source_path}/carla/ros2/*.h")#查找${libcarla_source_path}/carla/ros2/目录下的所有.h文件，并将文件路径存储到变量libcarla_carla_ros2_headers中。
install(FILES ${libcarla_carla_ros2_headers} DESTINATION include/carla/ros2)#将头文件安装到include/carla/ros2目录下。

# 安装 Boost 头文件到目标 'include' 目录
install(DIRECTORY "${BOOST_INCLUDE_PATH}/boost" DESTINATION include)

# 如果是在 Windows 平台上编译，则进一步处理特定版本的 Boost 库文件
if(WIN32)
# 定义一个名为 boostlibs 的变量，用于存储要安装的 Boost 库文件路径列表
  file(GLOB boostlibs# 定义一个名为boostlibs的变量，用于存储要安装的Boost库文件路径列表。
      "${BOOST_LIB_PATH}/libboost_date_time-*-mt-*.lib"
      "${BOOST_LIB_PATH}/libboost_system-*-mt-*.lib"
      "${BOOST_LIB_PATH}/libboost_filesystem-*-mt-*.lib")
  install(FILES ${boostlibs} DESTINATION lib)
endif()

# 使用GLOB命令收集libcarla_server库所需的源文件和头文件
file(GLOB libcarla_server_sources
    # 收集${libcarla_source_path}/carla/目录下的Buffer.cpp和Exception.cpp源文件
    "${libcarla_source_path}/carla/Buffer.cpp"
    "${libcarla_source_path}/carla/Exception.cpp"
    
    # 收集${libcarla_source_path}/carla/目录下所有的.h头文件
    "${libcarla_source_path}/carla/*.h"
    
    # 收集${libcarla_source_path}/carla/geom/目录下所有的.cpp源文件和.h头文件
    "${libcarla_source_path}/carla/geom/*.cpp"
    "${libcarla_source_path}/carla/geom/*.h"
    
    # 收集${libcarla_source_path}/carla/opendrive/目录下所有的.cpp源文件和.h头文件
    # 注意：您重复列出了.h文件路径，这里只列出一次
    "${libcarla_source_path}/carla/opendrive/*.cpp"
    "${libcarla_source_path}/carla/opendrive/*.h"
    
    # 收集${libcarla_source_path}/carla/opendrive/parser/目录下的所有.cpp源文件和.h头文件
    "${libcarla_source_path}/carla/opendrive/parser/*.cpp"
    "${libcarla_source_path}/carla/opendrive/parser/*.h"
    
    # 收集${libcarla_source_path}/carla/road/及其子目录下的所有.cpp源文件和.h头文件
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
    
    # 收集${libcarla_source_path}/carla/rpc/目录下的所有.cpp源文件和.h头文件
    "${libcarla_source_path}/carla/rpc/*.cpp"
    "${libcarla_source_path}/carla/rpc/*.h"
    
    # 收集${libcarla_source_path}/carla/sensor/及其子目录下的所有.h头文件和指定的.cpp文件
    "${libcarla_source_path}/carla/sensor/*.h"
    "${libcarla_source_path}/carla/sensor/s11n/*.h"
    "${libcarla_source_path}/carla/sensor/s11n/SensorHeaderSerializer.cpp"
    
    # 收集${libcarla_source_path}/carla/streaming/及其子目录下的所有.h头文件和.cpp源文件
    "${libcarla_source_path}/carla/streaming/*.h"
    "${libcarla_source_path}/carla/streaming/detail/*.cpp"
    "${libcarla_source_path}/carla/streaming/detail/*.h"
    "${libcarla_source_path}/carla/streaming/detail/tcp/*.cpp"
    "${libcarla_source_path}/carla/streaming/low_level/*.h"
    
    # 收集${libcarla_source_path}/carla/multigpu/目录下的所有.h头文件和.cpp源文件
    "${libcarla_source_path}/carla/multigpu/*.h"
    "${libcarla_source_path}/carla/multigpu/*.cpp"
    
    # 收集${libcarla_source_path}/carla/ros2/目录下的所有.h头文件和.cpp源文件
    "${libcarla_source_path}/carla/ros2/*.h"
    "${libcarla_source_path}/carla/ros2/*.cpp"
    
    # 收集第三方库目录下的所有源文件和头文件
    # 注意：这里假设第三方库位于一个单独的路径下
    "${libcarla_source_thirdparty_path}/odrSpiral/*.cpp"
    "${libcarla_source_thirdparty_path}/odrSpiral/*.h"
    "${libcarla_source_thirdparty_path}/moodycamel/*.cpp"
    "${libcarla_source_thirdparty_path}/moodycamel/*.h"
    "${libcarla_source_thirdparty_path}/pugixml/*.cpp"
    "${libcarla_source_thirdparty_path}/pugixml/*.hpp"
)

# ==============================================================================
# 在同一构建类型中创建调试和发布的目标
# ==============================================================================

if (LIBCARA_BUILD_RELEASE) #设置目标服务器属性，包括编译选项和链接选项


  add_library(carla_server STATIC ${libcarla_server_sources})

# 为名为carla_server的目标（即前面创建的静态库）添加头文件包含目录。
# 使用target_include_directories命令。
target_include_directories(carla_server SYSTEM PRIVATE
    "${BOOST_INCLUDE_PATH}"  # 添加Boost库的头文件路径。
    "${RPCLIB_INCLUDE_PATH}" # 添加rpclib库的头文件路径。
)
# SYSTEM关键字指示这些头文件被视为系统头文件，可能会影响编译器的警告行为。
# PRIVATE表示这些包含目录仅对carla_server目标可见，不会传递给其依赖项。

# 使用install命令将名为carla_server的目标（静态库）安装到目标路径下的lib目录中。
# OPTIONAL关键字表示如果目标不存在，安装过程不会失败。
install(TARGETS carla_server DESTINATION lib OPTIONAL)

# 为carla_server目标设置编译属性，使用set_target_properties命令。
# 这里设置编译标志为CMAKE_CXX_FLAGS_RELEASE，即用于发布版本的C++编译标志。
set_target_properties(carla_server PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

# 判断是否定义了LIBCARLA_BUILD_DEBUG变量，这个变量通常用于标识是否构建调试版本。
if (LIBCARLA_BUILD_DEBUG)

    # 如果定义了LIBCARLA_BUILD_DEBUG，则创建一个名为carla_server_debug的静态库。
    add_library(carla_server_debug STATIC ${libcarla_server_sources})

    # 为carla_server_debug目标添加头文件包含目录，与carla_server目标类似。
    target_include_directories(carla_server_debug SYSTEM PRIVATE
        "${BOOST_INCLUDE_PATH}"
        "${RPCLIB_INCLUDE_PATH}"
    )

    # 将生成的carla_server_debug静态库安装到目标路径下的lib目录中。
    install(TARGETS carla_server_debug DESTINATION lib OPTIONAL)

    # 为carla_server_debug目标设置编译属性，使用CMAKE_CXX_FLAGS_DEBUG，即用于调试版本的C++编译标志。
    set_target_properties(carla_server_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")

    # 为carla_server_debug目标添加编译定义，使用target_compile_definitions命令。
    # 这里添加了一个宏定义BOOST_ASIO_ENABLE_BUFFER_DEBUGGING，通常用于启用Boost.Asio的缓冲区调试功能。
    target_compile_definitions(carla_server_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

endif() # 结束对LIBCARLA_BUILD_DEBUG变量的判断。
