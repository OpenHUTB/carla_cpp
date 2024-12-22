# 指定 CMake 最低版本要求为 3.5.1，意味着该 CMakeLists.txt 文件需要在这个版本及以上的 CMake 环境中才能正确运行，
# 其目的在于确保所使用的 CMake 版本具备该文件中用到的各种功能特性，避免因版本过低而出现语法不兼容等问题。
cmake_minimum_required(VERSION 3.5.1)
# 定义项目名称为 libcarla-server，后续构建过程中诸如添加库、配置编译选项、指定文件安装路径等操作都围绕此项目展开，
# 该名称将作为整个项目在 CMake 构建体系中的唯一标识，便于进行相关的构建管理与配置。
project(libcarla-server)

# 以下部分主要涉及各种库和头文件的安装相关操作，通过这些操作来确保项目依赖的各类资源能正确放置在合适的位置，
# 以满足项目在编译、链接以及后续使用过程中对这些资源的引用需求。

# 安装 libc++ 共享库。
# 使用 file(GLOB...) 命令查找指定路径（由 ${LLVM_LIB_PATH} 变量指定路径）下所有以 libc++ 开头的文件，
# 这里的 GLOB 命令是 CMake 中一种强大的文件路径匹配工具，能够依据给定的通配符模式在指定目录下收集符合条件的文件路径，
# 然后将找到的文件路径列表存储在 LibCXX_Libraries 变量中，以便后续安装使用，为项目正确链接 libc++ 库做准备。
file(GLOB LibCXX_Libraries "${LLVM_LIB_PATH}/libc++*")
# install 命令用于将指定的文件安装到目标位置。
# 当 DESTINATION 给出相对路径时，它将相对于 CMAKE_INSTALL_PREFIX 变量（此处其值为 Unreal\CarlaUE4\Plugins\Carla\CarlaDependencies）所确定的基础路径来确定最终的安装位置，
# 在此处就是将之前收集到的 LibCXX_Libraries 变量所代表的 libc++ 相关文件安装到目标位置的 lib 目录下，确保库文件可被正确访问。
install(FILES ${LibCXX_Libraries} DESTINATION lib)

# 安装远程过程调用库 rpclib
# 首先，通过 install(DIRECTORY...) 命令将 ${RPCLIB_INCLUDE_PATH}/rpc 目录下的所有文件安装到目标位置的 include 目录下，
# 这一步操作的目的在于将 rpclib 的头文件放置到合适的位置，使得项目代码在编译时能够顺利找到并包含这些头文件，从而正确调用远程过程调用相关的功能接口。
install(DIRECTORY "${RPCLIB_INCLUDE_PATH}/rpc" DESTINATION include)
# 使用 file(GLOB...) 命令查找 ${RPCLIB_LIB_PATH} 目录下的所有文件（通配符 *.* 表示匹配任意文件名及扩展名的文件），
# 该命令会按照此模式在指定目录中搜索文件，并将找到的文件路径列表存储在 libcarla_carla_rpclib 变量中，方便后续进行相应的安装操作。
file(GLOB libcarla_carla_rpclib "${RPCLIB_LIB_PATH}/*.*")
# 接着，利用 install(FILES...) 命令将之前找到并存储在 libcarla_carla_rpclib 变量中的文件安装到目标路径下的 lib 目录中，
# 如此一来，在项目进行链接阶段时，就能准确找到并使用该远程过程调用库的实现文件，保证相关功能的正常运行。
install(FILES ${libcarla_carla_rpclib} DESTINATION lib)

# 安装头文件

# 使用 install(DIRECTORY...) 命令将 ${libcarla_source_path}/compiler 目录下的所有文件安装到目标位置的 include 目录下，
# 推测此处该目录下的文件可能包含了与项目编译相关的一些头文件或者其他辅助资源，将它们安装到合适位置有助于后续编译过程的顺利进行。
install(DIRECTORY "${libcarla_source_path}/compiler" DESTINATION include)

# 使用 file(GLOB...) 命令查找 ${libcarla_source_path}/carla/ 目录下所有以.h 为扩展名的头文件，
# 这里的 GLOB 操作基于文件扩展名进行匹配查找，会把符合条件的头文件路径收集起来，并将其存储在 libcarla_carla_headers 变量中，
# 后续可依据此变量来对这些头文件进行进一步的安装操作，以确保它们处于项目可正确引用的位置。
file(GLOB libcarla_carla_headers "${libcarla_source_path}/carla/*.h")
# 使用 install(FILES...) 命令将存储在 libcarla_carla_headers 变量中的头文件安装到目标路径下的 include/carla 目录中，
# 这样做的好处是，在项目其他代码中需要使用这些头文件时（例如通过 #include 指令引用），编译器能够按照既定的目录结构准确找到并处理它们，实现相应的功能逻辑。
install(FILES ${libcarla_carla_headers} DESTINATION include/carla)

# 使用 file(GLOB...) 命令查找 ${libcarla_source_path}/carla/geom/ 目录下所有以.h 为扩展名的头文件，
# 此 GLOB 命令的作用是按照指定的文件扩展名模式在特定目录下进行文件查找，将查找到的所有符合条件的头文件路径列表存储在变量 libcarla_carla_geom_headers 中，
# 便于后续围绕这些头文件开展诸如安装到指定目录等相关操作，以此保证项目在构建过程中能够正确引用这些头文件所声明的内容。
file(GLOB libcarla_carla_geom_headers "${libcarla_source_path}/carla/geom/*.h")
# 使用 install(FILES...) 命令将通过 GLOB 获取到的头文件列表（即存储在变量 ${libcarla_carla_geom_headers} 中的文件）安装到目标目录 include/carla/geom 下，
# 在项目进行安装等操作时，这样的安装设置能够确保这些头文件被精准地放置在指定的安装路径中，方便其他项目或者模块通过正确的路径来引用它们，进而实现相关的功能扩展或交互操作。
install(FILES ${libcarla_carla_geom_headers} DESTINATION include/carla/geom)

# 使用 GLOB 命令在指定的路径（${libcarla_source_path}/carla/opendrive/）下查找所有以.h 为后缀的文件，
# 该命令依据后缀名匹配规则收集相应文件，并把查找到的这些文件的路径列表存放到名为 libcarla_carla_opendrive 的变量当中，
# 这种基于 GLOB 的文件路径匹配和收集方式，为后续基于这些收集到的文件进行如安装等其他操作提供了便利，确保项目对这些头文件的有效管理与使用。
file(GLOB libcarla_carla_opendrive "${libcarla_source_path}/carla/opendrive/*.h")
# 通过 install(FILES...) 命令将变量 libcarla_carla_opendrive 所代表的文件列表中的所有文件，
# 安装到目标安装目录（include/carla/opendrive）下，
# 如此操作后，在整个项目进行安装过程时，相应的头文件就能被准确放置在期望的位置，
# 便于其他可能依赖该项目的代码在编译等操作时，能够顺利地找到并包含这些头文件，从而实现基于这些头文件所定义的功能逻辑，例如与 OpenDrive 相关的功能实现。
install(FILES ${libcarla_carla_opendrive} DESTINATION include/carla/opendrive)

# 使用 file(GLOB) 命令查找位于 ${libcarla_source_path}/carla/opendrive/parser/ 目录下所有的.h 文件，
# 这里的 GLOB 作为一种有效的文件通配符查找机制，在特定目录下按照指定后缀（.h）快速定位符合条件的文件，
# 并将查找到的这些.h 文件的路径列表存储到变量 libcarla_carla_opendrive_parser 中，为后续针对性的安装操作提供准确的文件路径信息。
file(GLOB libcarla_carla_opendrive_parser "${libcarla_source_path}/carla/opendrive/parser/*.h")
# 使用 install 命令将之前通过 file(GLOB) 查找到并存储在变量 libcarla_carla_opendrive_parser 中的头文件（.h 文件），
# 安装到目标目录 include/carla/opendrive/parser 下，
# 通过这样的安装步骤，在整个项目进行安装构建的过程中，这些头文件就能被准确无误地放置在正确的位置，
# 方便后续其他代码在编译等操作时能够顺利找到并引用它们，进而实现与该解析器（parser）相关的业务逻辑，例如解析 OpenDrive 文件等功能。
install(FILES ${libcarla_carla_opendrive_parser} DESTINATION include/carla/opendrive/parser)

# 使用 CMake 中的 GLOB 命令来查找符合特定模式的文件，
# 具体是在路径 ${libcarla_source_path}/carla/profiler/ 下查找所有以.h 为后缀的头文件，
# 此 GLOB 命令依据扩展名模式进行文件查找，能够快速定位目标文件，并将查找到的这些头文件的路径信息存储到变量 libcarla_carla_profiler_headers 中，
# 为后续的安装等操作提前准备好所需的文件路径集合，方便对这些头文件进行统一管理与部署。
file(GLOB libcarla_carla_profiler_headers "${libcarla_source_path}/carla/profiler/*.h")
# 使用 install 命令将存储在变量 ${libcarla_carla_profiler_headers} 中的头文件安装到目标目录 include/carla/profiler 下，
# 这样在项目安装完成后，这些头文件就能处于正确的位置，便于项目内其他模块或者外部依赖项目引用它们来实现相应的功能逻辑，例如性能分析相关的功能实现与数据统计等操作。
install(FILES ${libcarla_carla_profiler_headers} DESTINATION include/carla/profiler)

// 利用 CMake 中的 file 函数与 GLOB 命令来查找符合特定模式的文件。
// 这里是在由变量 ${libcarla_source_path} 所指定的基础路径下的“carla/road/”子目录中（即完整路径为 ${libcarla_source_path}/carla/road/ ），
// 查找所有以“.h”为后缀的头文件，然后把找到的这些头文件的路径列表存储到名为 libcarla_carla_road_headers 的变量中。
// 在 CMake 里，GLOB 命令常用于按文件扩展名等特定模式收集一批文件，这种收集方式便于后续围绕这些文件开展进一步操作，
// 比如将它们安装到特定目录、依据其进行编译配置等相关操作，以此确保项目在构建过程中能够正确使用这些头文件所定义的内容，满足项目功能实现的需求。
file(GLOB libcarla_carla_road_headers "${libcarla_source_path}/carla/road/*.h")
// 使用 install 命令来设置文件的安装规则，
// 该命令会将存储在变量 ${libcarla_carla_road_headers} 里的头文件列表中的每一个头文件，
// 安装到指定的目标目录“include/carla/road”下。
// 通过这样的安装设置，在整个项目进行安装的过程中，这些头文件就能按照预期被准确放置在对应的目录位置，
// 方便其他需要使用该库相关功能的代码（例如基于这个库开发的外部应用程序代码等）可以顺利地找到并包含这些头文件，
// 进而能够使用头文件中所声明的各种类型、函数等内容来实现相应的业务逻辑，比如道路相关的创建、查询、修改等功能操作。
install(FILES ${libcarla_carla_road_headers} DESTINATION include/carla/road)

// 收集 carla/road/element 目录下的所有头文件，并将它们存储在变量 libcarla_carla_road_element_headers 中，
// 这一操作的目的在于对该特定目录下的头文件进行统一管理，方便后续一次性将它们安装到指定的目录位置，供项目内其他代码进行引用。
file(GLOB libcarla_carla_road_element_headers "${libcarla_source_path}/carla/road/element/*.h")
// 使用 install 命令将存储在变量 ${libcarla_carla_road_element_headers} 中的头文件列表里的每一个头文件，
// 安装到指定的目标目录“include/carla/road/element”下。
// 如此一来，在项目整体安装时，相应的头文件就能被准确放置在对应的目录位置，便于其他代码引用这些头文件中的定义（例如结构体定义、函数声明等）来实现相关功能，
// 比如道路元素相关的属性设置、关系处理等具体业务逻辑操作。
install(FILES ${libcarla_carla_road_element_headers} DESTINATION include/carla/road/element)

// 以下类似地为其他模块收集头文件，并指定了各自的安装路径，这样做的目的是确保每个模块的头文件在项目安装后都能处于正确的目录结构中，
// 方便整个项目内不同模块之间以及外部依赖项目对这些头文件进行准确引用，进而实现各个模块对应的功能逻辑，保证项目整体功能的完整性和正确性。

// 使用 file(GLOB...) 命令来查找 ${libcarla_source_path}/carla/road/general/ 目录下的所有头文件（通配符 *.h 表示匹配所有以.h 为扩展名的文件），
// GLOB 操作会将匹配的文件路径存储到变量 libcarla_carla_road_general_headers 中，以便后续进行安装操作，
// 通过这种方式对该目录下的头文件进行统一收集管理，为后续将它们正确部署到项目结构中做好准备。
file(GLOB libcarla_carla_road_general_headers "${libcarla_source_path}/carla/road/general/*.h")
// 使用 install(FILES...) 命令将前面通过 GLOB 操作获取到的文件（存储在 libcarla_carla_road_general_headers 变量中的文件）安装到目标目录 include/carla/road/general，
// 这样在构建项目时，能够保证这些头文件被正确放置在项目结构中的相应位置，使得其他部分的代码可以顺利引用它们来实现相关功能，
// 例如道路通用属性相关的功能逻辑实现，如道路类型判断、基础属性获取等操作。
install(FILES ${libcarla_carla_road_general_headers} DESTINATION include/carla/road/general)

// 使用 file(GLOB...) 命令查找 ${libcarla_source_path}/carla/road/object/ 目录下的所有头文件（*.h），
// 并将查找到的文件路径结果存储在 libcarla_carla_road_object_headers 变量中，后续将基于此变量进行头文件的安装操作，
// 以此对该目录下的头文件进行有效管理，便于将它们准确安装到合适的位置供项目使用。
file(GLOB libcarla_carla_road_object_headers "${libcarla_source_path}/carla/road/object/*.h")
// 使用 install 命令将 libcarla_carla_road_object_headers 中的文件安装到 include/carla/road/object 目录，
// 使得项目内涉及道路对象相关功能的代码能够找到并引用这些头文件，进而实现如道路对象的创建、属性修改、与其他对象关系处理等相关业务逻辑操作。
install(FILES ${libcarla_carla_road_object_headers} DESTINATION include/carla/road/object)

// 使用 file(GLOB...) 命令查找 ${libcarla_source_path}/carla/road/signal/ 目录下的所有头文件（*.h），
// 将查找到的头文件路径存入 libcarla_carla_road_signal_headers 变量，为后续安装做准备，
// 通过这种方式对该目录下的头文件进行集中管理，方便后续将它们安装到正确位置供项目使用。
file(GLOB libcarla_carla_road_signal_headers "${libcarla_source_path}/carla/road/signal/*.h")
// 使用 install 命令将文件安装到 include/carla/road/signal 目录，
// 以便项目中关于道路信号相关的代码能够正确引用这些头文件，实现如道路信号的检测、处理、显示等相关功能逻辑操作。
install(FILES ${libcarla_carla_road_signal_headers} DESTINATION include/carla/road/signal)

// 使用 file(GLOB...) 命令查找 ${libcarla_source_path}/carla/rpc/ 目录下的所有头文件（*.h），
// 并将查找到的头文件路径结果存入 libcarla_carla_rpc_headers 变量，方便后续安装操作，
// 这样可以对该目录下的头文件进行统一管理，确保它们能被正确安装到合适位置供项目进行远程过程调用相关功能的实现。
file(GLOB libcarla_carla_rpc_headers "${libcarla_source_path}/carla/rpc/*.h")
// 使用 install 命令将头文件安装到 include/carla/rpc 目录，
// 确保项目在使用远程过程调用（RPC）相关功能时，对应的代码能够顺利找到并包含这些头文件，进而实现诸如远程函数调用、数据传输等 RPC 相关的业务逻辑操作。
install(FILES ${libcarla_carla_rpc_headers} DESTINATION include/carla/rpc)
// 使用 file(GLOB...) 命令查找 ${libcarla_source_path}/carla/sensor/ 目录下的所有头文件（*.h），
// 将查找到的头文件路径存入 libcarla_carla_sensor_headers 变量，用于后续的安装处理，
// 这样便于对传感器相关的头文件进行统一管理，为后续将它们放置到合适的项目目录结构中做准备，以保障传感器相关功能在项目中的正确实现。
file(GLOB libcarla_carla_sensor_headers "${libcarla_source_path}/carla/sensor/*.h")
// 使用 install 命令将头文件安装到 include/carla/sensor 目录，
// 使得项目中传感器相关的代码能够正确引用这些头文件，进而实现如传感器数据的采集、预处理、传输等相关功能逻辑，满足项目对传感器功能运用的需求。
install(FILES ${libcarla_carla_sensor_headers} DESTINATION include/carla/sensor)

// 使用 file(GLOB...) 命令查找 ${libcarla_source_path}/carla/sensor/data/ 目录下的所有头文件（*.h），
// 把查找到的头文件路径存储到 libcarla_carla_sensor_data_headers 变量中，为后续安装到相应目录做准备，
// 这有助于对传感器数据相关的头文件进行集中管控，确保它们能被准确安装到指定位置，方便后续代码与之交互操作。
file(GLOB libcarla_carla_sensor_data_headers "${libcarla_source_path}/carla/sensor/data/*.h")
// 使用 install 命令将头文件安装到 include/carla/sensor/data 目录，
// 便于项目内涉及传感器数据相关的功能代码能够顺利引用这些头文件，从而实现像传感器数据存储格式定义、数据读取与解析等具体业务逻辑，保障传感器数据处理环节的正常运行。
install(FILES ${libcarla_carla_sensor_data_headers} DESTINATION include/carla/sensor/data)

// 使用 file(GLOB...) 命令查找 ${libcarla_source_path}/carla/sensor/s11n/ 目录下的所有头文件（*.h），
// 将查找到的头文件路径存入 libcarla_carla_sensor_s11n_headers 变量，后续将据此进行安装操作，
// 这样能统一管理传感器序列化相关的头文件，便于后续将它们正确安装到合适的项目目录位置，以供相关代码使用。
file(GLOB libcarla_carla_sensor_s11n_headers "${libcarla_source_path}/carla/sensor/s11n/*.h")
// 使用 install 命令将头文件安装到 include/carla/sensor/s11n 目录，
// 确保项目中与传感器序列化相关的代码能够找到并引用这些头文件，进而实现传感器数据序列化、反序列化等相关功能逻辑，便于传感器数据在不同模块间的传递与存储。
install(FILES ${libcarla_carla_sensor_s11n_headers} DESTINATION include/carla/sensor/s11n)

// 使用 file(GLOB...) 命令查找 ${libcarla_source_path}/carla/streaming/ 目录下的所有头文件（*.h），
// 将查找到的头文件路径存入 libcarla_carla_streaming_headers 变量，为后续安装操作提供依据，
// 这是对数据流式处理相关头文件的收集管理过程，方便后续将它们部署到正确的项目目录结构中，保障流式处理相关功能代码能正确引用。
file(GLOB libcarla_carla_streaming_headers "${libcarla_source_path}/carla/streaming/*.h")
// 使用 install 命令将头文件安装到 include/carla/streaming 目录，
// 使得项目中涉及数据流式处理的代码能够顺利找到并引用这些头文件，进而实现如数据流式传输、处理逻辑编排等相关功能逻辑，满足项目对数据流式处理的需求。
install(FILES ${libcarla_carla_streaming_headers} DESTINATION include/carla/streaming)

// 使用 file(GLOB...) 命令查找 ${libcarla_source_path}/carla/streaming/detail/ 目录下的所有头文件（*.h），
// 这里的 GLOB 是 file 命令的一个操作，用于在指定子目录下按照扩展名匹配头文件并收集其路径，
// 后续将依据收集到的头文件路径进行相应的安装操作，保障该细节层面的头文件能被正确放置到项目结构中供使用。
file(GLOB libcarla_carla_streaming_detail_headers "${libcarla_source_path}/carla/streaming/detail/*.h")
// 使用 install 命令将通过 GLOB 获取到的头文件（存储在变量 ${libcarla_carla_streaming_detail_headers} 中的文件）安装到目标目录 include/carla/streaming/detail 下，
// 使得项目在涉及到数据流式处理细节相关功能实现时，对应的代码能够准确找到并引用这些头文件，以保障流式处理细节功能（如具体协议实现、底层交互逻辑等）的正确实现。
install(FILES ${libcarla_carla_streaming_detail_headers} DESTINATION include/carla/streaming/detail)

// 使用 file(GLOB libcarla_carla_streaming_detail_tcp_headers "${libcarla_source_path}/carla/streaming/detail/tcp/*.h" 会在 ${libcarla_source_path}/carla/streaming/detail/tcp/ 目录下查找所有.h 文件，
// 并将其路径存储到 libcarla_carla_streaming_detail_tcp_headers 变量中，这是专门针对流式处理中 TCP 相关头文件的收集操作，便于后续统一管理与安装。
file(GLOB libcarla_carla_streaming_detail_tcp_headers "${libcarla_source_path}/carla/streaming/detail/tcp/*.h")
// 使用 install(FILES...) 命令将第 104 行找到的头文件（存储在 ${libcarla_carla_streaming_detail_tcp_headers} 变量中的文件）安装到 include/carla/streaming/detail/tcp 目录下，
// 如此一来，项目中涉及到数据流式处理里 TCP 相关功能实现的代码就能顺利找到并引用这些头文件，比如基于 TCP 的流式数据传输、连接管理等功能逻辑实现。
install(FILES ${libcarla_carla_streaming_detail_tcp_headers} DESTINATION include/carla/streaming/detail/tcp)

// 使用 file(GLOB...) 命令查找 ${libcarla_source_path}/carla/streaming/low_level/ 目录下所有的.h 文件，并将文件路径存储到变量 libcarla_carla_streaming_low_level_headers 中，
// 这一步是收集流式处理底层相关的头文件，方便后续将它们安装到合适位置，供项目中涉及底层流式处理功能实现的代码引用。
file(GLOB libcarla_carla_streaming_low_level_headers "${libcarla_source_path}/carla/streaming/low_level/*.h")
// 使用 install 命令将头文件安装到 include/carla/streaming/low_level 目录下，
// 确保在项目需要运用到流式处理底层功能（如底层数据封装、传输优化等）时，相关代码能够准确找到并使用这些头文件所定义的内容来实现相应逻辑。
install(FILES ${libcarla_carla_streaming_low_level_headers} DESTINATION include/carla/streaming/low_level)

// 使用 file(GLOB...) 命令查找 ${libcarla_source_path}/carla/multigpu/ 目录下的所有.h 文件，并将文件路径存储到变量 libcarla_carla_multigpu_headers 中，
// 此操作旨在收集多 GPU 相关的头文件，便于后续统一管理并安装到项目结构中合适的位置，以保障多 GPU 相关功能代码能正确引用它们。
file(GLOB libcarla_carla_multigpu_headers "${libcarla_source_path}/carla/multigpu/*.h")
// 使用 install 命令将头文件安装到 include/carla/multigpu 目录下，
// 使得项目中涉及多 GPU 功能（如多 GPU 并行计算、资源分配管理等）的代码能够顺利找到并引用这些头文件，进而实现相应的多 GPU 功能逻辑。
install(FILES ${libcarla_carla_multigpu_headers} DESTINATION include/carla/multigpu)

// 使用 file(GLOB...) 命令查找 ${libcarla_source_path}/carla/ros2/ 目录下的所有.h 文件，并将文件路径存储到变量 libcarla_carla_ros2_headers 中，
// 这是对 ROS2 相关头文件的收集过程，方便后续将它们正确安装到项目结构中，供项目内涉及 ROS2 功能集成的代码进行引用。
file(GLOB libcarla_carla_ros2_headers "${libcarla_source_path}/carla/ros2/*.h")
// 使用 install 命令将头文件安装到 include/carla/ros2 目录下，
// 确保项目在与 ROS2 进行交互、集成 ROS2 相关功能（如消息传递、节点通信等）时，对应的代码能够准确找到并包含这些头文件来实现相应的 ROS2 功能逻辑。
install(FILES ${libcarla_carla_ros2_headers} DESTINATION include/carla/ros2)

// 安装 Boost 头文件到目标 'include' 目录，
// 这一步操作是将 Boost 库的头文件放置到项目可引用的合适位置，使得项目在需要使用 Boost 相关功能（如智能指针、线程管理等）时，能够顺利找到并包含相应头文件进行功能实现。
install(DIRECTORY "${BOOST_INCLUDE_PATH}/boost" DESTINATION include)

// 如果是在 Windows 平台上编译，则进一步处理特定版本的 Boost 库文件，
// 因为在 Windows 环境下，Boost 库的一些特定版本配置及文件处理可能与其他平台有所不同，需要进行针对性的操作来确保库文件正确安装和使用。
if(WIN32)
    // 定义一个名为 boostlibs 的变量，用于存储要安装的 Boost 库文件路径列表，
    // 通过下面的 GLOB 操作来收集符合特定模式的 Boost 库文件路径，方便后续进行统一的安装处理。
    file(GLOB boostlibs
        "${BOOST_LIB_PATH}/libboost_date_time-*-mt-*.lib"
        "${BOOST_LIB_PATH}/libboost_system-*-mt-*.lib"
        "${BOOST_LIB_PATH}/libboost_filesystem-*-mt-*.lib")
    // 使用 install(FILES...) 命令将收集到的 Boost 库文件（存储在 boostlibs 变量中的文件）安装到目标位置的 lib 目录中，
    // 这样在 Windows 平台编译项目时，就能正确链接到这些特定的 Boost 库文件，保障 Boost 相关功能在项目中的正常运行。
    install(FILES ${boostlibs} DESTINATION lib)
endif()

// carla_server 库相关操作部分，以下代码主要涉及收集该库的源文件以及针对不同构建类型（发布版和调试版）创建相应的目标，并配置相关属性。

// 使用 file(GLOB...) 命令收集构成 carla_server 库的各类源文件路径，
// 这些源文件涵盖了项目多个不同模块下的.cpp 和.h 文件，收集它们是为后续创建库以及编译链接操作做准备，确保所有相关代码都能参与到库的构建过程中。
file(GLOB libcarla_server_sources
    "${libcarla_source_path}/carla/*.h" // 收集${libcarla_source_path}/carla/目录下名为 Buffer.cpp 的源文件路径，以及其他以.h 为扩展名的头文件路径，共同作为库的一部分资源。
    "${libcarla_source_path}/carla/Buffer.cpp" // 收集${libcarla_source_path}/carla/目录下名为 Buffer.cpp 的源文件路径，该文件是构成 carla_server 库功能实现的重要代码文件之一。
    "${libcarla_source_path}/carla/Exception.cpp" // 收集${libcarla_source_path}/carla/目录下名为 Exception.cpp 的源文件路径，其包含了项目中特定异常处理相关的代码逻辑，对库的稳定性和错误处理有重要作用。
    "${libcarla_source_path}/carla/geom/*.cpp" // 收集${libcarla_source_path}/carla/geom/目录下所有以.cpp 为扩展名的源文件路径，这些文件实现了与几何相关的功能逻辑，是 carla_server 库功能的一部分。
    "${libcarla_source_path}/carla/geom/*.h" // 收集${libcarla_source_path}/carla/geom/目录下所有以.h 为扩展名的头文件路径，为几何相关功能的代码实现提供了必要的类型定义、函数声明等内容，供其他代码引用。
    "${libcarla_source_path}/carla/opendrive/*.cpp" // 收集${libcarla_source_path}/carla/opendrive/目录下所有以.cpp 为扩展名的源文件路径，包含了与 OpenDrive 相关功能实现的代码，是库功能的重要组成部分。
    "${libcarla_source_path}/carla/opendrive/*.h" // 收集${libcarla_source_path}/carla/opendrive/目录下所有以.h 为扩展名的头文件路径，为 OpenDrive 相关功能的代码提供必要的声明，便于在库中正确实现和调用相关功能。
    "${libcarla_source_path}/carla/opendrive/parser/*.cpp" // 收集 carla/opendrive/parser 目录下的所有.cpp 文件路径，这些文件实现了对 OpenDrive 文件解析相关的功能逻辑，对处理特定格式的数据很关键。
    "${libcarla_source_path}/carla/opendrive/parser/*.h" // 收集 carla/opendrive/parser 目录下的所有.h 文件路径，提供了 OpenDrive 文件解析相关功能的声明内容，供解析代码实现时使用。
    "${libcarla_source_path}/carla/road/*.cpp" // 收集 carla/road 目录下的所有.cpp 文件路径，实现了道路相关功能的具体代码逻辑，是 carla_server 库在道路处理方面的重要组成部分。
    "${libcarla_source_path}/carla/road/*.h" // 收集 carla/road 目录下的所有.h 文件路径，定义了道路相关的各种类型、函数等，便于其他代码在处理道路相关业务时引用和调用。
    "${libcarla_source_path}/carla/road/element/*.cpp" // 收集 carla/road/element 目录下的所有.cpp 文件路径，包含了道路元素相关功能实现的代码，是细化道路处理功能的一部分代码资源。
    "${libcarla_source_path}/carla/road/element/*.h" // 收集 carla/road/element 目录下的所有.h 文件路径，为道路元素相关功能提供必要的声明内容，辅助具体代码实现。
    "${libcarla_source_path}/carla/road/general/*.cpp" // 收集 carla/road/general 目录下的所有.cpp 文件路径，实现了道路通用属性相关的功能逻辑，有助于完善道路相关的整体功能。
    "${libcarla_source_path}/carla/road/general/*.h" // 收集 carla/road/general 目录下的所有.h 文件路径，提供道路通用属性相关的类型定义、函数声明等，供其他道路相关代码使用。
    "${libcarla_source_path}/carla/road/object/*.cpp" // 收集 carla/road/object 目录下的所有.cpp 文件路径，包含了道路对象相关功能实现的代码，用于处理道路上各类对象相关的业务逻辑。
    "${libcarla_source_path}/carla/road/object/*.h" // 收集 carla/road/object 目录下的所有.h 文件路径，为道路对象相关功能提供声明内容，方便代码对道路对象进行操作。
    "${libcarla_source_path}/carla/road/signal/*.cpp" // 收集 carla/road/signal 目录下的所有.cpp 文件路径，实现了道路信号相关功能的具体代码逻辑，用于处理道路信号相关事务。
    "${libcarla_source_path}/carla/road/signal/*.h" // 收集 carla/road/signal 目录下的所有.h 文件路径，提供道路信号相关的类型定义、函数声明等，辅助道路信号相关功能实现。
    "${libcarla_source_path}/carla/rpc/*.cpp" // 收集 carla/rpc 目录下的所有.cpp 文件路径，实现了远程过程调用相关功能的具体代码逻辑，是库中实现 RPC 功能的关键部分。
    "${libcarla_source_path}/carla/rpc/*.h" // 收集 carla/rpc 目录下的所有.h 文件路径，提供远程过程调用相关的类型定义、函数声明等，供其他代码进行 RPC 操作时引用。
    "${libcarla_source_path}/carla/sensor/*.h" // 收集 carla/sensor 目录下的所有.h 文件路径，定义了传感器相关的各种内容，便于传感器相关代码使用。
    "${libcarla_source_path}/carla/sensor/s11n/*.h" // 收集 carla/sensor/s11n 目录下的所有.h 文件路径，为传感器序列化相关功能提供声明内容，辅助序列化相关代码实现。
    "${libcarla_source_path}/carla/sensor/s11n/SensorHeaderSerializer.cpp" // 收集 carla/sensor/s11n 目录下的 SensorHeaderSerializer.cpp 文件路径，该文件实现了传感器头部信息序列化相关的具体逻辑，是传感器序列化功能的一部分。
    "${libcarla_source_path}/carla/streaming/*.h" // 收集 carla/streaming 目录下的所有.h 文件路径，定义了数据流式处理相关的内容，方便流式处理相关代码引用。
    "${libcarla_source_path}/carla/streaming/detail/*.cpp" // 收集 carla/streaming/detail 目录下的所有.cpp 文件路径，这些文件包含了数据流式处理细节方面功能实现的具体代码逻辑，例如具体的流式处理流程控制、底层交互细节等操作相关的代码，是完善流式处理功能的重要组成部分。
    "${libcarla_source_path}/carla/streaming/detail/*.h" // 收集 carla/streaming/detail 目录下的所有.h 文件路径，为数据流式处理细节相关功能提供类型定义、函数声明等内容，便于在具体代码中进行相应的调用，辅助流式处理细节功能的正确实现。
    "${libcarla_source_path}/carla/streaming/detail/tcp/*.cpp" // 收集 carla/streaming/detail/tcp 目录下的所有.cpp 文件路径，这些文件实现了基于TCP协议的数据流式处理相关的具体逻辑，比如TCP连接建立、数据在TCP通道上的传输等功能相关代码，对基于TCP的流式处理功能起着关键作用。
    "${libcarla_source_path}/carla/streaming/low_level/*.h" // 收集 carla/streaming/low_level 目录下的所有.h 文件路径，定义了数据流式处理底层相关的各种类型、函数等内容，为流式处理底层功能实现提供必要的基础，比如底层数据的封装格式、优化传输机制相关的声明等。
    "${libcarla_source_path}/carla/multigpu/*.h" // 收集 carla/multigpu 目录下的所有.h 文件路径，为多GPU相关功能提供了必要的类型定义、函数声明等内容，方便项目中涉及多GPU并行计算、资源分配等功能代码的编写与调用。
    "${libcarla_source_path}/carla/multigpu/*.cpp" // 收集 carla/multigpu 目录下的所有.cpp 文件路径，包含了多GPU相关功能实现的具体代码逻辑，例如如何在多个GPU上分配任务、进行数据并行处理等操作相关的代码，是实现多GPU功能的核心代码部分。
    "${libcarla_source_path}/carla/ros2/*.h" // 收集 carla/ros2 目录下的所有.h 文件路径，提供了与ROS2集成相关的各种类型定义、函数声明等内容，便于项目在与ROS2进行交互、实现ROS2相关功能（如节点通信、消息传递等）时进行代码编写与调用。
    "${libcarla_source_path}/carla/ros2/*.cpp" // 收集 carla/ros2 目录下的所有.cpp 文件路径，实现了与ROS2集成相关功能的具体代码逻辑，比如如何与ROS2节点进行对接、发送和接收ROS2消息等功能相关的代码，对项目集成ROS2功能起着重要作用。
    "${libcarla_source_thirdparty_path}/odrSpiral/*.cpp" // 收集第三方库odrSpiral目录下的所有.cpp文件路径，这些文件包含了来自第三方库odrSpiral的功能实现代码，可能涉及到特定的算法、数据处理逻辑等，为项目整体功能补充了额外的能力。
    "${libcarla_source_thirdparty_path}/odrSpiral/*.h" // 收集第三方库odrSpiral目录下的所有.h 文件路径，为来自该第三方库的功能提供了相应的类型定义、函数声明等内容，便于在项目中正确调用和使用其功能。
    "${libcarla_source_thirdparty_path}/moodycamel/*.cpp" // 收集第三方库moodycamel目录下的所有.cpp文件路径，包含了该第三方库所提供功能的具体实现代码，或许是与并发队列、高效数据结构等相关的代码逻辑，为项目在相关方面提供支持。
    "${libcarla_source_thirdparty_path}/moodycamel/*.h" // 收集第三方库moodycamel目录下的所有.h 文件路径，定义了该第三方库相关功能的类型、函数等内容，方便项目代码在需要使用其提供的功能时进行正确引用。
    "${libcarla_source_thirdparty_path}/pugixml/*.cpp" // 收集第三方库pugixml目录下的所有.cpp文件路径，这些文件实现了与XML解析相关的功能逻辑，比如如何读取、解析XML文件等操作相关的代码，有助于项目处理XML格式的数据。
    "${libcarla_source_thirdparty_path}/pugixml/*.hpp" // 收集第三方库pugixml目录下的所有.hpp文件路径，为XML解析相关功能提供了相应的类型定义、函数声明等内容，便于在项目中正确运用XML解析功能来处理相关数据。

// ==============================================================================
// 在同一构建类型中创建调试和发布的目标，针对不同的构建需求（如开发调试阶段和正式发布阶段）分别进行相应的配置，以满足不同场景下对项目构建的要求。
// ==============================================================================

if (LIBCARA_BUILD_RELEASE) // 判断是否定义了LIBCARA_BUILD_RELEASE这个变量，通常该变量用于标识构建发布版本，若此变量为真，表示当前是构建发布版本的场景，以下代码块将被执行来进行相应的配置操作。

    // 使用add_library命令创建一个名为carla_server的静态库，将之前通过GLOB收集到的所有源文件（存储在libcarla_server_sources变量中）作为该静态库的组成部分，这些源文件中的代码逻辑共同构成了carla_server静态库的功能实现。
    add_library(carla_server STATIC ${libcarla_server_sources})

    // 使用target_include_directories命令为名为carla_server的目标（也就是前面创建的静态库）添加头文件包含目录，以便在编译该静态库时能正确找到对应的头文件。
    // SYSTEM关键字表示这些目录下的头文件被视为系统头文件，在一些编译器行为上可能会有区别对待，比如抑制某些警告等情况，有助于按照更合适的编译规则来处理这些头文件。
    // PRIVATE表示这些包含目录仅对该目标（carla_server）本身可见，不会传递给依赖它的其他目标，这样可以更好地控制头文件的作用范围，避免不必要的头文件暴露和潜在的命名冲突等问题。
    // 后面跟着的是具体的头文件包含目录路径，这里分别添加了BOOST_INCLUDE_PATH和RPCLIB_INCLUDE_PATH这两个路径，它们对应的头文件对于carla_server静态库的正确编译是必需的。
    target_include_directories(carla_server SYSTEM PRIVATE
        "${BOOST_INCLUDE_PATH}"
        "${RPCLIB_INCLUDE_PATH}")

    // 使用install命令将生成的carla_server静态库安装到目标路径下的lib目录中，这样在项目安装后，其他项目或者外部模块如果需要使用该库，就能在对应的lib目录下找到它，从而方便地进行链接使用，实现相应的功能扩展和交互。
    install(TARGETS carla_server DESTINATION lib OPTIONAL)

    // 使用set_target_properties命令为carla_server目标设置编译相关的属性，此处将编译标志设置为CMAKE_CXX_FLAGS_RELEASE，这意味着在构建发布版本时，会应用与发布相关的编译选项，比如优化级别、代码生成方式等，确保生成的库在性能、稳定性等方面符合发布要求，达到较好的运行效果。
    set_target_properties(carla_server PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

endif()

if (LIBCARLA_BUILD_DEBUG) // 判断是否定义了LIBCARLA_BUILD_DEBUG这个变量，该变量通常用于标识当前是否处于构建调试版本的场景，如果为真，则执行以下代码块来针对调试版本进行相应的配置操作。

    // 使用add_library命令创建一个名为carla_server_debug的静态库，同样以之前收集到的libcarla_server_sources变量中的所有源文件作为其组成部分，通过这些源文件里的代码逻辑构建出调试版本的静态库，方便在调试阶段进行问题排查和功能验证等操作。
    add_library(carla_server_debug STATIC ${libcarla_server_sources})

    // 使用target_include_directories命令为名为carla_server_debug的目标（即刚创建的调试版本静态库）添加头文件包含目录，作用和之前类似，确保在编译该调试库时能准确找到所需的头文件。
    // SYSTEM关键字表示将这些目录下的头文件视为系统头文件，有着特定的编译器处理方式，比如减少一些不必要的警告显示，利于调试时专注于关键代码逻辑。
    // PRIVATE表示这些包含目录仅对本目标（carla_server_debug）自身可见，不会传递给依赖它的其他目标，以此限定头文件作用范围，便于在调试阶段更精准地控制代码依赖和避免一些潜在的干扰因素。
    // 这里添加的BOOST_INCLUDE_PATH和RPCLIB_INCLUDE_PATH路径对应的头文件对于调试版本静态库的正确编译也是不可或缺的。
    target_include_directories(carla_server_debug SYSTEM PRIVATE
        "${BOOST_INCLUDE_PATH}"
        "${RPCLIB_INCLUDE_PATH}")

    // 使用install命令将生成的carla_server_debug静态库安装到目标路径下的lib目录中，这样在调试版本的项目安装后，也能在相应的lib目录找到该调试库，便于在调试过程中进行替换、链接使用等操作，辅助开发人员查找和修复代码中的问题。
    install(TARGETS carla_server_debug DESTINATION lib OPTIONAL)

    // 使用set_target_properties命令为carla_server_debug目标设置编译相关的属性，首先将编译标志设置为CMAKE_CXX_FLAGS_DEBUG，意味着在构建调试版本时，会应用适合调试的编译选项，例如保留调试符号、开启一些有助于调试的优化级别等，方便开发人员在调试过程中进行代码追踪、变量查看等操作，更好地定位和解决问题。
    set_target_properties(carla_server_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")

    // 使用set_target_properties命令为carla_server_debug这个静态库目标设置编译相关的属性，通过定义BOOST_ASIO_ENABLE_BUFFER_DEBUGGING宏，开启Boost.Asio库中与缓冲区相关的调试功能，在调试涉及网络通信等使用到该库的部分时，能够更细致地查看缓冲区的数据变化、错误情况等，有助于排查相关的潜在问题，进一步辅助调试工作的开展。
    target_compile_definitions(carla_server_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

endif() // 判断是否定义了LIBCARLA_BUILD_DEBUG这个变量结束标志，至此完成了针对不同构建类型（发布版本和调试版本）的相应配置操作，使得项目在不同阶段都能按照合适的构建方式生成对应的版本，满足开发、测试以及最终发布使用等各种需求。



