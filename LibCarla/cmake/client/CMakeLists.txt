#设置CMake最低版本要求
cmake_minimum_required(VERSION 3.5.1)
#定义项目名称
project(libcarla-client)

# 启用/安装 RSS VARIANT
#默认是禁用
option (BUILD_RSS_VARIANT "Enables ad-rss-lib based RSS components" OFF)
# 设置目标后缀为空字符串，这是默认情况
set(carla_target_postfix "")
# 如果用户启用了 BUILD_RSS_VARIANT 选项，则执行以下操作
if (BUILD_RSS_VARIANT)
  # 更新目标后缀以包含 "_rss" 字符串，这可能用于区分不同变体的目标文件或库
  set(carla_target_postfix "_rss")
   # 初始化 ADRSS_INCLUDE_DIRS 变量为空列表
  # 这个变量将用来存储 RSS 变体所需的所有包含路径
  set(ADRSS_INCLUDE_DIRS)
    # 初始化 ADRSS_LIBS 变量为空列表
  # 这个变量将用来存储 RSS 变体所需的所有链接库
  set(ADRSS_LIBS)

  # 快速C++日志库：spdlog
  #寻找并配置外部库spdlog
  find_package(spdlog CONFIG REQUIRED
    HINTS ${ADRSS_INSTALL_DIR}/spdlog/)
    #获得和处理spdlog的属性
  get_target_property(spdlog_include_dirs spdlog::spdlog INTERFACE_INCLUDE_DIRECTORIES)
  get_target_property(spdlog_file spdlog::spdlog LOCATION)
  foreach(dir ${spdlog_include_dirs})
  # 遍历 spdlog_include_dirs 列表中的每个目录
    file(GLOB spdlog_includes "${dir}/*")
    install(DIRECTORY ${spdlog_includes} DESTINATION include/system)
    list(APPEND ADRSS_INCLUDE_DIRS ${dir})
  endforeach()
  # DESTINATION给出相对路径时，它将相对于 CMAKE_INSTALL_PREFIX 变量（PythonAPI\carla\dependencies）
  #将spdlog库安装到目标位置
  install(FILES ${spdlog_file} DESTINATION lib)
  list(APPEND ADRSS_LIBS ${spdlog_file})
  #完成项目的包含目录和库目录的路径设置 安装位置指定以及相关列表的添加操作
  set(proj_include_dir ${ADRSS_INSTALL_DIR}/../../proj-install/include)#在CMake中，set命令用于设置变量。
  set(proj_lib ${ADRSS_INSTALL_DIR}/../../proj-install/lib/libproj.a)#定义了名为proj_lib的变量，其值为${ADRSS_INSTALL_DIR}/../../proj - install/lib/libproj.a。这很可能是proj项目的库文件（静态库，因为后缀是.a）的路径，同样依赖于ADRSS_INSTALL_DIR变量。
  install(DIRECTORY ${proj_include_dir} DESTINATION include/system)#在CMake构建系统中，install命令用于指定安装规则。这里是将proj_include_dir所指向的目录安装到目标目录include/system下。这意味着在安装过程中，proj项目的头文件将会被复制到include/system目录中，以便其他项目可以找到这些头文件。
  list(APPEND ADRSS_INCLUDE_DIRS ${proj_include_dir})#list命令用于操作列表。这里的APPEND操作是将proj_include_dir的值添加到ADRSS_INCLUDE_DIRS列表中。这可能是为了在后续的构建过程中，方便地管理所有需要包含的头文件目录。
  install(FILES ${proj_lib} DESTINATION lib)#这是将proj_lib所指向的库文件（libproj.a）安装到目标目录lib下。
  list(APPEND ADRSS_LIBS ${proj_lib})#将proj_lib的值添加到ADRSS_LIBS列表中。这有助于在构建过程中管理所有需要链接的库文件。

  # 只要 libosm2odr 使用相同的 odrSpiral 接口，复制构建的静态库就足够了
  set(odr_lib ${ADRSS_INSTALL_DIR}/../src/ad-rss-lib/dependencies/map/dependencies/odrSpiral/lib/libodrSpiral.a)#定义了一个名为odr_lib的变量，它指向了odrSpiral库的路径。这里使用${ADRSS_INSTALL_DIR}等变量（假设这些变量在之前的CMake脚本中已经定义）来构建库文件的完整路径。
  install(FILES ${odr_lib} DESTINATION lib)#install(FILES${odr_lib} DESTINATION_lib)将odrSpiral库安装到名为_lib的目标位置（这里看起来DESTINATION_lib可能是一个笔误，也许是DESTINATION lib的意思，表示安装到名为lib的目标文件夹下）。
  list(APPEND ADRSS_LIBS ${odr_lib})#list(APPEND ADRSS_LIBS${odr_lib})把odr_lib所指向的库文件添加到ADRSS_LIBS列表中，以便后续在项目构建中使用。

  foreach(ad_lib ad_physics ad_rss ad_map_access ad_map_opendrive_reader ad_rss_map_integration)
  set(${ad_lib}_file ${ADRSS_INSTALL_DIR}/${ad_lib}/lib/lib${ad_lib}.a)#设置每个库的文件路径
  # 使用install命令将上一步中设置的静态库文件安装到目标位置，
  # 这里的目标位置是安装目录下的"lib"文件夹。
  # ${${ad_lib}_file}是CMake的变量间接引用语法，它首先解析${ad_lib}_file得到变量名（如ad_physics_file），
  # 然后再解析该变量名对应的值（如/path/to/adrss_install/ad_physics/lib/libad_physics.a）。
    install(FILES ${${ad_lib}_file} DESTINATION lib)#安装库文件
    list(APPEND ADRSS_LIBS ${${ad_lib}_file})#将库添加到ADRSS_LIBS列表

    # 如果可用，请安装相应的 Python 库
    file(GLOB ${ad_lib}_python_libs "${ADRSS_INSTALL_DIR}/${ad_lib}/lib/lib${ad_lib}_python*.a")
    # 遍历找到的所有Python库文件
    foreach(ad_python_lib ${${ad_lib}_python_libs})
     # 检查每个文件是否存在
      if (EXISTS ${ad_python_lib})
        # 如果存在，则安装该文件到目标系统的 'lib' 目录中
        install(FILES ${ad_python_lib} DESTINATION lib)
      endif()
    endforeach()
# 设置包含路径为ADRSS_INSTALL_DIR下的对应库的include目录
    set(${ad_lib}_include_dir ${ADRSS_INSTALL_DIR}/${ad_lib}/include)
    # 使用 file(GLOB ...) 查找上述包含路径下的所有文件和子目录
    file(GLOB ${ad_lib}_includes "${${ad_lib}_include_dir}/*")
    # 安装找到的所有头文件和子目录到目标系统的 'include/system' 目录下
    install(DIRECTORY ${${ad_lib}_includes} DESTINATION include/system)
    # 将当前库的包含路径添加到 ADRSS_INCLUDE_DIRS 列表中，以便后续使用
    list(APPEND ADRSS_INCLUDE_DIRS ${${ad_lib}_include_dir})
  endforeach()
endif()

# 安装 Recast&Detour 库
install(DIRECTORY "${RECAST_INCLUDE_PATH}/recast" DESTINATION include/system)#这行代码使用 install 命令将Recast库的头文件安装到指定目录。
file(GLOB libcarla_carla_recastlib "${RECAST_LIB_PATH}/*.*")#file(GLOB libcarla_carla_recastlib "${RECAST_LIB_PATH}/*.*") 使用 GLOB 命令来获取 RECAST_LIB_PATH 下的所有文件。
install(FILES ${libcarla_carla_recastlib} DESTINATION lib)#install(FILES${libcarla_carla_recastlib} DESTINATION lib) 这行将上一步获取的Recast库文件安装到项目的 lib 目录下。

#安装 rpclib
install(DIRECTORY "${RPCLIB_INCLUDE_PATH}/rpc" DESTINATION include/system)#install(DIRECTORY "${RPCLIB_INCLUDE_PATH}/rpc" DESTINATION include/system) 将 RPCLIB_INCLUDE_PATH 下名为 rpc 的目录中的头文件安装到项目的 include/system 目录下。这里的 RPCLIB_INCLUDE_PATH 是指向rpclib头文件所在目录的预定义变量。
file(GLOB libcarla_carla_rpclib "${RPCLIB_LIB_PATH}/*.*")#file(GLOB libcarla_carla_rpclib "${RPCLIB_LIB_PATH}/*.*") 使用 GLOB 命令获取 RPCLIB_LIB_PATH 下的所有文件，并存储到变量 libcarla_carla_rpclib 中。这里的 RPCLIB_LIB_PATH 是指向rpclib库文件所在目录的预定义变量。
install(FILES ${libcarla_carla_rpclib} DESTINATION lib)#install(FILES${libcarla_carla_rpclib} DESTINATION lib) 将获取的rpclib库文件安装到项目的 lib 目录下。

# 安装 Boost 头文件
# 待办：从 LibCarla.client 的公共接口中移除 Boost
install(DIRECTORY "${BOOST_INCLUDE_PATH}/boost" DESTINATION include/system)#install(DIRECTORY "${BOOST_INCLUDE_PATH}/boost" DESTINATION include/system) 将 BOOST_INCLUDE_PATH 下名为 boost 的目录中的头文件安装到项目的 include/system 目录下。这里的 BOOST_INCLUDE_PATH 是指向Boost头文件所在目录的预定义变量。
file(GLOB boost_libraries "${BOOST_LIB_PATH}/*.*")#file(GLOB boost_libraries "${BOOST_LIB_PATH}/*.*") 使用 GLOB 命令获取 BOOST_LIB_PATH 下的所有文件，并存储到变量 boost_libraries 中。这里的 BOOST_LIB_PATH 是指向Boost库文件所在目录的预定义变量。
install(FILES ${boost_libraries} DESTINATION lib)#install(FILES${boost_libraries} DESTINATION lib) 将获取的Boost库文件安装到项目的 lib 目录下。

# Windows 需要安装 libpng 以及 zlib。
if (WIN32)#if (WIN32) 是一个条件判断，如果当前构建系统是Windows平台（WIN32 是CMake中的一个预定义变量，在Windows平台下为真），则执行下面的代码块。
    # 安装 zlib 头文件
    file(GLOB zlib_includes "${ZLIB_INCLUDE_PATH}/*")#file(GLOB zlib_includes "${ZLIB_INCLUDE_PATH}/*") 使用 GLOB 命令获取 ZLIB_INCLUDE_PATH 下的所有文件，并存储到变量 zlib_includes 中。这里的 ZLIB_INCLUDE_PATH 是指向zlib头文件所在目录的预定义变量。

    install(FILES ${zlib_includes} DESTINATION include)#install(FILES${zlib_includes} DESTINATION include) 将获取的zlib头文件安装到项目的 include 目录下。
    # 安装 zlib 库
    file(GLOB zlib_libraries "${ZLIB_LIB_PATH}/*")#file(GLOB zlib_libraries "${ZLIB_LIB_PATH}/*") 使用 GLOB 命令获取 ZLIB_LIB_PATH 下的所有文件，并存储到变量 zlib_libraries 中。这里的 ZLIB_LIB_PATH 是指向zlib库文件所在目录的预定义变量。
    install(FILES ${zlib_libraries} DESTINATION lib)#install(FILES${zlib_libraries} DESTINATION lib) 将获取的zlib库文件安装到项目的 lib 目录下。

    # 安装 libpng 头文件
    file(GLOB pnglib_includes "${LIBPNG_INCLUDE_PATH}/*")#查找${LIBPNG_INCLUDE_PATH}目录下的所有文件，并将结果存储在pnglib_includes变量中。GLOB命令用于生成符合特定模式的文件列表。
    install(FILES ${pnglib_includes} DESTINATION include)#将pnglib_includes变量中的文件安装到include目录下。这意味着将libpng库的头文件安装到项目的include目录。
    # 安装 zlib 库.
    file(GLOB libpng_libraries "${LIBPNG_LIB_PATH}/*")
    install(FILES ${libpng_libraries} DESTINATION lib)
    #使用file(GLOB...)命令查找${LIBPNG_LIB_PATH}目录下的所有文件，并将结果存储在libpng_libraries变量中。然后使用install(FILES...)命令将这些文件安装到lib目录下。这意味着将libpng库的库文件安装到项目的lib目录。
else ()
  # 安装图片库（libpng）
  install(DIRECTORY "${LIBPNG_INCLUDE_PATH}" DESTINATION include/system)#使用install(DIRECTORY...)命令将${LIBPNG_INCLUDE_PATH}目录安装到include/system目录下，这可能是为了将libpng库的头文件安装到特定的系统级别的include目录。
  file(GLOB libcarla_carla_libpnglib "${LIBPNG_LIB_PATH}/*.*")#命令查找${LIBPNG_LIB_PATH}目录下的所有文件，并将结果存储在libcarla_carla_libpnglib变量中。
  install(FILES ${libcarla_carla_libpnglib} DESTINATION lib)#使用install(FILES...)命令将libcarla_carla_libpnglib变量中的文件安装到lib目录下。
endif (WIN32)


# 添加和libcarla客户端相关的源代码
# 添加 LibCarla 目录（LibCarla/source/carla）下的*.cpp和.h文件
file(GLOB libcarla_carla_sources#使用file(GLOB...)命令查找${libcarla_source_path}/carla/目录下的所有.cpp和.h文件，并将结果存储在libcarla_carla_sources变量中。
    "${libcarla_source_path}/carla/*.cpp"
    "${libcarla_source_path}/carla/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_sources}")#将libcarla_carla_sources变量中的文件路径添加到libcarla_sources变量中。
install(FILES ${libcarla_carla_sources} DESTINATION include/carla)#将libcarla_carla_sources变量中的文件安装到include/carla目录下。

# 添加客户端目录（LibCarla/source/carla/client）下的*.cpp和.h文件
file(GLOB libcarla_carla_client_sources#使用file(GLOB...)命令来查找${libcarla_source_path}/carla/client/目录下的所有.cpp和.h文件，并将结果存储在libcarla_carla_client_sources变量中。
    "${libcarla_source_path}/carla/client/*.cpp"
    "${libcarla_source_path}/carla/client/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_client_sources}")#将libcarla_carla_client_sources变量中的文件路径添加到libcarla_sources变量中。
install(FILES ${libcarla_carla_client_sources} DESTINATION include/carla/client)#将libcarla_carla_client_sources变量中的文件安装到include/carla/client目录下。

# 添加客户端详细目录（LibCarla/source/carla/client/detail）下的*.cpp和.h文件
file(GLOB libcarla_carla_client_detail_sources#使用file(GLOB...)命令来查找${libcarla_source_path}/carla/client/detail/目录下的所有.cpp和.h文件，并将结果存储在libcarla_carla_client_detail_sources变量中。
    "${libcarla_source_path}/carla/client/detail/*.cpp"
    "${libcarla_source_path}/carla/client/detail/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_client_detail_sources}")#将libcarla_carla_client_detail_sources变量中的文件路径添加到libcarla_sources变量中。
install(FILES ${libcarla_carla_client_detail_sources} DESTINATION include/carla/client/detail)#将libcarla_carla_client_detail_sources变量中的文件安装到include/carla/client/detail目录下。

# 添加几何（LibCarla/source/carla/geom）相关代码
file(GLOB libcarla_carla_geom_sources#使用file(GLOB...)命令来查找${libcarla_source_path}/carla/geom/目录下的所有.cpp和.h文件，并将结果存储在libcarla_carla_geom_sources变量中。
    "${libcarla_source_path}/carla/geom/*.cpp"
    "${libcarla_source_path}/carla/geom/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_geom_sources}")#将libcarla_carla_geom_sources变量中的文件路径添加到libcarla_sources变量中。
install(FILES ${libcarla_carla_geom_sources} DESTINATION include/carla/geom)#将libcarla_carla_geom_sources变量中的文件安装到include/carla/geom目录下。

# 添加图像（LibCarla/source/carla/image）相关代码
file(GLOB libcarla_carla_image_sources#使用file(GLOB...)命令来查找${libcarla_source_path}/carla/image/目录下的所有.cpp和.h文件，并将结果存储在libcarla_carla_image_sources变量中。
    "${libcarla_source_path}/carla/image/*.cpp"
    "${libcarla_source_path}/carla/image/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_image_sources}")#将libcarla_carla_image_sources变量中的文件路径添加到libcarla_sources变量中。
install(FILES ${libcarla_carla_image_sources} DESTINATION include/carla/image)将libcarla_carla_image_sources变量中的文件安装到include/carla/image目录下。

# 添加导航（LibCarla/source/carla/nav）相关代码
file(GLOB libcarla_carla_nav_sources#查找${libcarla_source_path}/carla/nav/目录下的所有.cpp和.h文件，并将结果存储在libcarla_carla_nav_sources变量中。
    "${libcarla_source_path}/carla/nav/*.cpp"
    "${libcarla_source_path}/carla/nav/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_nav_sources}")#将libcarla_carla_nav_sources变量中的文件路径添加到libcarla_sources变量中。
install(FILES ${libcarla_carla_nav_sources} DESTINATION include/carla/nav)#将libcarla_carla_nav_sources变量中的文件安装到include/carla/nav目录下。

# 添加开源路网（LibCarla/source/carla/opendrive/）相关代码
file(GLOB libcarla_carla_opendrive_sources#
    "${libcarla_source_path}/carla/opendrive/*.cpp"
    "${libcarla_source_path}/carla/opendrive/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_opendrive_sources}")
install(FILES ${libcarla_carla_opendrive_sources} DESTINATION include/carla/opendrive)

# 添加开源路网解析（LibCarla/source/carla/parser/）相关代码
file(GLOB libcarla_carla_opendrive_parser_sources
    "${libcarla_source_path}/carla/opendrive/parser/*.cpp"
    "${libcarla_source_path}/carla/opendrive/parser/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_opendrive_parser_sources}")# 使用 set 命令对变量 libcarla_sources 进行赋值操作，将原有的 libcarla_sources 变量内容（如果有的话）与新收集到的 OpenDrive 解析器相关文件路径（变量 libcarla_carla_opendrive_parser_sources 的值）进行拼接（通过分号分隔）后重新赋值给 libcarla_sources 变量。
# 这样做的目的可能是将不同功能模块的源文件路径汇总到一起，方便后续统一处理，例如用于创建库或者可执行文件等操作，确保各个模块的代码都能参与到整体的项目构建中。
install(FILES ${libcarla_carla_opendrive_parser_sources} DESTINATION include/carla/opendrive/parser)# 使用 install 命令进行文件安装操作，指定要安装的文件就是存储在变量 ${libcarla_carla_opendrive_parser_sources} 中的那些 OpenDrive 解析器相关的源文件和头文件（前面通过 file(GLOB) 收集到的）。
# DESTINATION 参数指定了安装的目标目录为 include/carla/opendrive/parser，也就是在安装过程中，会把收集到的与 OpenDrive 解析器相关的文件复制到指定的 include/carla/opendrive/parser 目录下。
# 这样做的好处是，当其他代码想要使用 Carla 项目中 OpenDrive 解析器相关的功能时，可以方便地在这个安装目录下找到对应的头文件进行包含，以顺利进行编译等操作，规范了项目接口文件的安装位置，便于项目的对外使用和分发。

# 添加点云（LibCarla/source/carla/pointcloud/）相关代码
file(GLOB libcarla_carla_pointcloud_sources
    "${libcarla_source_path}/carla/pointcloud/*.cpp"
    "${libcarla_source_path}/carla/pointcloud/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_pointcloud_sources}")
install(FILES ${libcarla_carla_pointcloud_sources} DESTINATION include/carla/pointcloud)

# 添加性能分析器（LibCarla/source/carla/profiler/）的头文件
file(GLOB libcarla_carla_profiler_headers
    "${libcarla_source_path}/carla/profiler/*.h")# 使用 file(GLOB) 命令收集文件路径，将匹配的文件路径列表存储在变量 libcarla_carla_profiler_headers 中。
# 具体是收集 ${libcarla_source_path}/carla/profiler/ 目录下所有以.h 为后缀的头文件路径，
# 这意味着会把 carla 项目中与性能分析器（profiler）相关的头文件都找出来，方便后续进行其他操作，比如安装到指定目录等。
file(GLOB libcarla_carla_profiler_headers
install(FILES ${libcarla_carla_profiler_headers} DESTINATION include/carla/profiler)# 使用 install 命令进行文件安装操作，这里指定要安装的文件就是存储在变量 ${libcarla_carla_profiler_headers} 中的那些头文件（前面通过 file(GLOB) 收集到的）。
# DESTINATION 参数指定了安装的目标目录为 include/carla/profiler，也就是在安装过程中，会把收集到的与 carla 性能分析器相关的头文件复制到指定的 include/carla/profiler 目录下，
# 这样做的目的通常是为了让使用该项目的其他代码（比如其他模块或者外部项目想要调用 carla 相关功能时）能够方便地找到并包含这些头文件，以正确地进行编译链接等操作，使项目的接口头文件处于合理的安装位置便于被引用。

# 添加道路（LibCarla/source/carla/road/）相关代码
file(GLOB libcarla_carla_road_sources
    "${libcarla_source_path}/carla/road/*.cpp"
    "${libcarla_source_path}/carla/road/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_road_sources}")
install(FILES ${libcarla_carla_road_sources} DESTINATION include/carla/road)

# 添加道路元素（LibCarla/source/carla/road/element/）相关代码
file(GLOB libcarla_carla_road_element_sources
    "${libcarla_source_path}/carla/road/element/*.cpp"
    "${libcarla_source_path}/carla/road/element/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_road_element_sources}")
install(FILES ${libcarla_carla_road_element_sources} DESTINATION include/carla/road/element)

# 没有general这个文件夹
file(GLOB libcarla_carla_road_general_sources
    "${libcarla_source_path}/carla/road/general/*.cpp"
    "${libcarla_source_path}/carla/road/general/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_road_general_sources}")
install(FILES ${libcarla_carla_road_general_sources} DESTINATION include/carla/road/general)

# 添加道路对象（LibCarla/source/carla/road/object/）相关代码
file(GLOB libcarla_carla_road_object_sources
    "${libcarla_source_path}/carla/road/object/*.cpp"
    "${libcarla_source_path}/carla/road/object/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_road_object_sources}")
install(FILES ${libcarla_carla_road_object_sources} DESTINATION include/carla/road/object)

# 没有signal这个文件夹
file(GLOB libcarla_carla_road_signal_sources
    "${libcarla_source_path}/carla/road/signal/*.cpp"
    "${libcarla_source_path}/carla/road/signal/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_road_signal_sources}")
install(FILES ${libcarla_carla_road_signal_sources} DESTINATION include/carla/road/signal)

# 添加远程过程调用（LibCarla/source/carla/rpc/）相关代码
file(GLOB libcarla_carla_rpc_sources
    "${libcarla_source_path}/carla/rpc/*.cpp"
    "${libcarla_source_path}/carla/rpc/*.h")# 使用 CMake 中的 file(GLOB) 命令进行文件查找与收集操作。
# 这里是收集符合特定模式的文件路径，并将这些路径存储到变量 libcarla_carla_rpc_sources 中。
# 具体查找的是在 ${libcarla_source_path}/carla/rpc/ 目录下，所有以.cpp 为后缀的 C++ 源文件，以及同样目录下以.h 为后缀的头文件路径。
# 这表明收集的是与 Carla 项目中 RPC（Remote Procedure Call，远程过程调用）相关的源文件和头文件，这些文件包含了实现 RPC 功能的代码，为后续项目构建等环节使用该功能模块做准备。
set(libcarla_sources "${libcarla_sources};${libcarla_carla_rpc_sources}")
install(FILES ${libcarla_carla_rpc_sources} DESTINATION include/carla/rpc)

# 是否构建责任敏感安全变体（传感器需要调用它，所以放在传感器构建之前）
if (BUILD_RSS_VARIANT)
  # 添加责任敏感安全（LibCarla/source/carla/rss/）相关代码
  file(GLOB libcarla_carla_rss_sources
      "${libcarla_source_path}/carla/rss/*.cpp"
      "${libcarla_source_path}/carla/rss/*.h")
  set(libcarla_sources "${libcarla_sources};${libcarla_carla_rss_sources}")
  install(FILES ${libcarla_carla_rss_sources} DESTINATION include/carla/rss)
endif()

# 添加传感器（LibCarla/source/carla/sensor/）相关代码
file(GLOB libcarla_carla_sensor_sources
    "${libcarla_source_path}/carla/sensor/*.cpp"
    "${libcarla_source_path}/carla/sensor/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_sensor_sources}")
install(FILES ${libcarla_carla_sensor_sources} DESTINATION include/carla/sensor)

# 添加传感器数据（LibCarla/source/carla/sensor/data/）相关代码
file(GLOB libcarla_carla_sensor_data_sources
    "${libcarla_source_path}/carla/sensor/data/*.cpp"
    "${libcarla_source_path}/carla/sensor/data/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_sensor_data_sources}")
install(FILES ${libcarla_carla_sensor_data_sources} DESTINATION include/carla/sensor/data)

# 添加传感器序列化（LibCarla/source/carla/sensor/s11n/）相关代码
file(GLOB libcarla_carla_sensor_s11n_sources
    "${libcarla_source_path}/carla/sensor/s11n/*.cpp"
    "${libcarla_source_path}/carla/sensor/s11n/*.h")# 这里具体的匹配模式是查找在 ${libcarla_source_path}/carla/sensor/s11n/ 目录下的所有以.cpp 为后缀的 C++ 源文件，以及同样目录下以.h 为后缀的头文件。
# 也就是说，该命令会遍历指定目录（${libcarla_source_path}/carla/sensor/s11n/），把其中所有的 C++ 源文件和头文件的路径都找出来
# 将找到的源文件添加到libcarla_sources变量中
set(libcarla_sources "${libcarla_sources};${libcarla_carla_sensor_s11n_sources}")
install(FILES ${libcarla_carla_sensor_s11n_sources} DESTINATION include/carla/sensor/s11n)

# 添加流式传输（LibCarla/source/carla/streaming/）相关代码
file(GLOB libcarla_carla_streaming_sources
    "${libcarla_source_path}/carla/streaming/*.cpp"
    "${libcarla_source_path}/carla/streaming/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_streaming_sources}")
install(FILES ${libcarla_carla_streaming_sources} DESTINATION include/carla/streaming)

# 添加详细流式传输（LibCarla/source/carla/streaming/detail/）相关代码
file(GLOB libcarla_carla_streaming_detail_sources
    "${libcarla_source_path}/carla/streaming/detail/*.cpp"
    "${libcarla_source_path}/carla/streaming/detail/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_streaming_detail_sources}")
install(FILES ${libcarla_carla_streaming_detail_sources} DESTINATION include/carla/streaming/detail)

# 添加详细TCP流式传输（LibCarla/source/carla/streaming/detail/tcp/）相关代码
file(GLOB libcarla_carla_streaming_detail_tcp_sources
    "${libcarla_source_path}/carla/streaming/detail/tcp/*.cpp"
    "${libcarla_source_path}/carla/streaming/detail/tcp/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_streaming_detail_tcp_sources}")
install(FILES ${libcarla_carla_streaming_detail_tcp_sources} DESTINATION include/carla/streaming/detail/tcp)

# 添加低层流式传输（LibCarla/source/carla/streaming/detail/tcp/）相关代码
file(GLOB libcarla_carla_streaming_low_level_sources
    "${libcarla_source_path}/carla/streaming/low_level/*.cpp"
    "${libcarla_source_path}/carla/streaming/low_level/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_streaming_low_level_sources}")
install(FILES ${libcarla_carla_streaming_low_level_sources} DESTINATION include/carla/streaming/low_level)

# 添加多GPU（LibCarla/source/carla/multigpu/）相关代码
file(GLOB libcarla_carla_multigpu_sources
    "${libcarla_source_path}/carla/multigpu/*.cpp"
    "${libcarla_source_path}/carla/multigpu/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_multigpu_sources}")
install(FILES ${libcarla_carla_multigpu_sources} DESTINATION include/carla/multigpu)

# 添加开源道路中计算螺旋线（LibCarla/source/third-party/odrSpiral）相关代码
file(GLOB libcarla_odr_spiral_sources
    "${libcarla_source_thirdparty_path}/odrSpiral/*.cpp"
    "${libcarla_source_thirdparty_path}/odrSpiral/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_odr_spiral_sources}")
install(FILES ${libcarla_odr_spiral_sources} DESTINATION include/odrSpiral)

# 添加多生产者多消费者无锁队列（LibCarla/source/third-party/moodycamel）相关代码
file(GLOB libcarla_moodycamel_sources
    "${libcarla_source_thirdparty_path}/moodycamel/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_moodycamel_sources}")
install(FILES ${libcarla_moodycamel_sources} DESTINATION include/moodycamel)

# 添加轻量级的 C++ XML 处理库（LibCarla/source/third-party/pugixml）相关代码
file(GLOB libcarla_pugixml_sources
    "${libcarla_source_thirdparty_path}/pugixml/*.cpp"
    "${libcarla_source_thirdparty_path}/pugixml/*.hpp"
    "${libcarla_source_thirdparty_path}/pugixml/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_pugixml_sources}")
install(FILES ${libcarla_pugixml_sources} DESTINATION include/pugixml)

# 添加交通管理器（LibCarla/source/carla/trafficmanager/）相关代码
file(GLOB libcarla_carla_trafficmanager_sources
    "${libcarla_source_path}/carla/trafficmanager/*.cpp"
    "${libcarla_source_path}/carla/trafficmanager/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_trafficmanager_sources}")
install(FILES ${libcarla_carla_trafficmanager_sources} DESTINATION include/carla/trafficmanager)

# 添加机器人操作系统（LibCarla/source/carla/ros2/）相关代码
file(GLOB libcarla_carla_ros2_sources
    "${libcarla_source_path}/carla/ros2/*.cpp"
    "${libcarla_source_path}/carla/ros2/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_ros2_sources}")
install(FILES ${libcarla_carla_ros2_sources} DESTINATION include/carla/ros2)

# ==============================================================================
# 在相同的构建类型中创建调试和发布的目标。
# ==============================================================================


# 构建发布版
if (LIBCARLA_BUILD_RELEASE)
  add_library(carla_client${carla_target_postfix} STATIC ${libcarla_sources})  #使用CMake的add_library命令来创建一个静态库 且其源文件由变量$决定

#设置目标包含目录：使用target_include_directories命令为carla_client库设置包含目录
  target_include_directories(carla_client${carla_target_postfix} SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}"
      "${RECAST_INCLUDE_PATH}"
      "${LIBPNG_INCLUDE_PATH}")
# 如果定义了BUILD_RSS_VARIANT这个构建选项
  if (BUILD_RSS_VARIANT)# 为carla_client（带上carla_target_postfix后缀）目标添加编译定义，启用RSS相关功能（RSS_ENABLED和RSS_USE_TBB）
    target_compile_definitions(carla_client${carla_target_postfix} PRIVATE RSS_ENABLED RSS_USE_TBB)# 将ADRSS_LIBS里的库以及tbb库链接到carla_client（带上carla_target_postfix后缀）目标上
    target_link_libraries(carla_client${carla_target_postfix} ${ADRSS_LIBS} tbb)# 将ADRSS_INCLUDE_DIRS里的目录作为系统头文件包含目录添加到carla_client（带上carla_target_postfix后缀）目标中
    target_include_directories(carla_client${carla_target_postfix} SYSTEM PRIVATE
      ${ADRSS_INCLUDE_DIRS})
  endif()

#使用install命令来安装carla_client库
# 这样在其他项目中就可以通过链接该库来使用它提供的功能
  install(TARGETS carla_client${carla_target_postfix} DESTINATION lib)

#windows平台特定处理：使用set_target_postfix命令设置carla_client库的编译标准
# @todo 修复在Windows上的PythonAPI构建漏洞，这里是待办事项注释，提示后续需要解决该问题
  if (WIN32) # @todo 修复在 Windows 上的 PythonAPI 构建漏洞。
    # 设置carla_client${carla_target_postfix}目标的编译标志
    # 将其编译标志设置为CMAKE_CXX_FLAGS_RELEASE，通常意味着在Windows平台下以发布版本的编译标准进行编译
    # 例如会包含合适的优化等编译相关设置
    set_target_properties(carla_client${carla_target_postfix} PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
 # 链接Recast及其相关库的Windows版本静态库到carla_client${carla_target_postfix}目标。
    target_link_libraries(carla_client${carla_target_postfix} "${RECAST_LIB_PATH}/Recast.lib")
    target_link_libraries(carla_client${carla_target_postfix} "${RECAST_LIB_PATH}/Detour.lib")
    target_link_libraries(carla_client${carla_target_postfix} "${RECAST_LIB_PATH}/DetourCrowd.lib")
  else ()         #非Windows平台处理：使用set命令设置CMAKE_CXX_FLAGS_RELEASE_CLIENT变量 为了在非Windows平台上设置合适的编译标准
    if (NOT DEFINED CMAKE_CXX_FLAGS_RELEASE_CLIENT)
      set(CMAKE_CXX_FLAGS_RELEASE_CLIENT ${CMAKE_CXX_FLAGS_RELEASE})
    endif()
 # 设置carla_client${carla_target_postfix}目标的编译标志为CMAKE_CXX_FLAGS_RELEASE_CLIENT中定义的值
    # 同样地，这里也使用了已废弃的COMPILE_FLAGS属性
    set_target_properties(carla_client${carla_target_postfix} PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE_CLIENT}")
 # 链接Recast及其相关库的非Windows版本静态库到carla_client${carla_target_postfix}目标。
    target_link_libraries(carla_client${carla_target_postfix} "${RECAST_LIB_PATH}/libRecast.a")
    target_link_libraries(carla_client${carla_target_postfix} "${RECAST_LIB_PATH}/libDetour.a")
    target_link_libraries(carla_client${carla_target_postfix} "${RECAST_LIB_PATH}/libDetourCrowd.a")

  endif (WIN32)

endif()

# 构建调试版
if (LIBCARLA_BUILD_DEBUG)

 # 添加一个静态库目标，名为 carla_client${carla_target_postfix}_debug，包含libcarla_sources中列出的所有源文件
  add_library(carla_client${carla_target_postfix}_debug STATIC ${libcarla_sources})
  
 # 指定编译此静态库时需要包含的系统头文件路径，并且这些路径只对这个目标可见（PRIVATE）
  target_include_directories(carla_client${carla_target_postfix}_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}"
      "${RECAST_INCLUDE_PATH}"
      "${LIBPNG_INCLUDE_PATH}")

  # 如果启用了RSS_VARIANT构建选项，则进行以下额外设置
  if (BUILD_RSS_VARIANT)
  # 定义预处理器宏，以启用RSS功能并指定使用TBB线程库
    target_compile_definitions(carla_client${carla_target_postfix}_debug PRIVATE RSS_ENABLED RSS_USE_TBB)
    # 链接AD-RSS库和Intel TBB库到carla_client_debug库
    target_link_libraries(carla_client${carla_target_postfix}_debug ${ADRSS_LIBS} tbb)
    # 添加AD-RSS库的头文件路径
    target_include_directories(carla_client${carla_target_postfix}_debug SYSTEM PRIVATE
      ${ADRSS_INCLUDE_DIRS})
  endif()

 # 安装命令，将构建的carla_client_debug库安装到系统的lib目录下
  install(TARGETS carla_client${carla_target_postfix}_debug DESTINATION lib)

 # 如果是在Windows平台构建，则应用特定于Windows的设置
  if (WIN32) # @todo Fix PythonAPI build on Windows.
      # 设置编译标志为调试模式下的C++编译标志（这可能包括优化关闭、调试信息等）
    set_target_properties(carla_client${carla_target_postfix}_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
 # 链接Recast及其相关库的Windows版本静态库
    target_link_libraries(carla_client${carla_target_postfix}_debug "${RECAST_LIB_PATH}/Recast.lib")
    target_link_libraries(carla_client${carla_target_postfix}_debug "${RECAST_LIB_PATH}/Detour.lib")
    target_link_libraries(carla_client${carla_target_postfix}_debug "${RECAST_LIB_PATH}/DetourCrowd.lib")
  else ()
   # 如果未定义CMAKE_CXX_FLAGS_DEBUG_CLIENT，则将其设置为与CMAKE_CXX_FLAGS_DEBUG相同
    if (NOT DEFINED CMAKE_CXX_FLAGS_DEBUG_CLIENT)
      set(CMAKE_CXX_FLAGS_DEBUG_CLIENT ${CMAKE_CXX_FLAGS_DEBUG})
    endif()

    set_target_properties(carla_client${carla_target_postfix}_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG_CLIENT}")#    # 在非DEBUG模式下，将Recast库（以.a格式）链接到carla_client目标（带有特定后缀的debug版本）
#设置编译标志
    target_link_libraries(carla_client${carla_target_postfix}_debug "${RECAST_LIB_PATH}/libRecast.a")#    # 在非DEBUG模式下，将Detour库（以.a格式）链接到carla_client目标（带有特定后缀的debug版本）
    target_link_libraries(carla_client${carla_target_postfix}_debug "${RECAST_LIB_PATH}/libDetour.a") # 在非DEBUG模式下，将DetourCrowd库（以.a格式，此处可能存在书写错误，应为libDetourCrowd.a）
    target_link_libraries(carla_client${carla_target_postfix}_debug "${RECAST_LIB_PATH}/libDetourCrowd.a")#链接更多库

  endif (WIN32)
#条件结束和编译定义
  target_compile_definitions(carla_client${carla_target_postfix}_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

endif()
