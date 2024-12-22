@echo off
setlocal enabledelayedexpansion 

rem 设置libosmscout库的GitHub仓库地址
set LIBOSMSCOUT_REPO=https://github.com/Framstag/libosmscout
rem 设置lunasvg库的GitHub仓库地址
set LUNASVG_REPO=https://github.com/sammycage/lunasvg

rem 设置libosmscout源码的存放路径，将INSTALLATION_DIR中的斜杠替换为反斜杠，并添加特定的子目录名，用于后续克隆源码等操作
set LIBOSMSCOUT_SOURCE_PATH=%INSTALLATION_DIR:/=\%libosmscout-source\
rem 设置libosmscout对应的Visual Studio项目路径，同样进行路径格式处理并添加相应子目录名，用于后续构建等操作
set LIBOSMSCOUT_VSPROJECT_PATH=%INSTALLATION_DIR:/=\%libosmscout-visualstudio\

rem 设置lunasvg源码的存放路径，按照特定格式处理路径并添加子目录名
set LUNASVG_SOURCE_PATH=%INSTALLATION_DIR:/=\%lunasvg-source\
rem 设置lunasvg对应的Visual Studio项目路径，进行路径处理及添加对应子目录
set LUNASVG_VSPROJECT_PATH=%INSTALLATION_DIR:/=\%lunasvg-visualstudio\

rem 设置osm-world-renderer的源码路径，将ROOT_PATH中的斜杠替换为反斜杠并添加对应子目录名
set OSM_RENDERER_SOURCE=%ROOT_PATH:/=\%osm-world-renderer\
rem 设置osm-world-renderer对应的Visual Studio项目路径，进行路径处理和添加对应子目录名
set OSM_RENDERER_VSPROJECT_PATH=%INSTALLATION_DIR:/=\%osm-world-renderer-visualstudio\

rem 设置服务器依赖项的安装路径，以osm-world-renderer源码路径为基础添加子目录ThirdParties，并处理路径格式
rem 后续依赖库可能会安装到此目录下
set DEPENDENCIES_INSTALLATION_PATH=%OSM_RENDERER_SOURCE:/=\%ThirdParties\


rem ============================================================================
rem -- Download dependency manager for libosmscout -----------------------------
rem ============================================================================

rem 设置vcpkg的GitHub仓库地址，vcpkg是一个C++库管理工具，可用于下载和管理各种C++依赖库
set VCPKG_REPO=https://github.com/microsoft/vcpkg
rem 设置vcpkg在本地的安装路径，进行路径格式转换（将斜杠换为反斜杠）并添加对应目录名
set VCPKG_PATH=%INSTALLATION_DIR:/=\%vcpkg\
rem 设置vcpkg的CMake工具链文件路径，用于在CMake项目中引入vcpkg管理的依赖库等相关配置
set VCPKG_CMAKE_TOOLCHAIN_PATH=%VCPKG_PATH:/=\%scripts\buildsystems\vcpkg.cmake

rem 如果vcpkg的本地安装路径不存在，则使用git克隆vcpkg仓库到指定的本地路径
rem 此处注释掉了，可能原本是用于首次获取vcpkg工具的代码，后续根据实际情况决定是否启用
rem if not exist "%VCPKG_PATH%" git clone %VCPKG_REPO% %VCPKG_PATH%

rem 进入vcpkg的安装目录并执行bootstrap-vcpkg.bat脚本，该脚本通常用于初始化vcpkg环境等操作
rem 此处同样注释掉了，可能根据实际部署情况决定是否执行此步骤
rem.\"%VCPKG_PATH:/=\%"bootstrap-vcpkg.bat


rem ============================================================================
rem -- Download and build libosmscout ------------------------------------------
rem ============================================================================

rem 如果libosmscout的源码路径不存在，使用git从设置的GitHub仓库地址克隆源码到指定的本地源码路径
if not exist "%LIBOSMSCOUT_SOURCE_PATH%" git clone %LIBOSMSCOUT_REPO% %LIBOSMSCOUT_SOURCE_PATH%

rem 如果libosmscout的Visual Studio项目路径不存在，则创建该目录，用于后续的CMake生成项目文件以及构建操作
if not exist "%LIBOSMSCOUT_VSPROJECT_PATH%" mkdir "%LIBOSMSCOUT_VSPROJECT_PATH%"
rem 切换到libosmscout的Visual Studio项目目录下，后续的CMake操作在此目录中进行
cd "%LIBOSMSCOUT_VSPROJECT_PATH%"

rem 使用CMake配置生成Visual Studio 2019项目文件，设置安装前缀（将反斜杠替换为正斜杠格式用于CMake配置），
rem 并关闭一些libosmscout相关的工具、测试、客户端Qt相关、示例等的构建选项，指定源码路径
cmake -G "Visual Studio 16 2019"^
    -DCMAKE_INSTALL_PREFIX="%DEPENDENCIES_INSTALLATION_PATH:\=/%"^
    -DOSMSCOUT_BUILD_TOOL_STYLEEDITOR=OFF^
    -DOSMSCOUT_BUILD_TOOL_OSMSCOUT2=OFF^
    -DOSMSCOUT_BUILD_TESTS=OFF^
    -DOSMSCOUT_BUILD_CLIENT_QT=OFF^
    -DOSMSCOUT_BUILD_DEMOS=OFF^
    "%LIBOSMSCOUT_SOURCE_PATH%"

rem 以下这行被注释掉了，原本可能是想设置C++编译Release模式下的一些特定编译标志，比如定义数学常量M_PI的值等
rem -DCMAKE_CXX_FLAGS_RELEASE="/DM_PI=3.14159265358979323846"^

rem 使用CMake进行构建，指定构建配置为Release，并执行安装目标，将构建生成的文件安装到指定的安装前缀目录下
cmake --build. --config=Release --target install


rem ============================================================================
rem -- Download and build lunasvg ----------------------------------------------
rem ============================================================================

rem 如果lunasvg的源码路径不存在，使用git从设置的GitHub仓库地址克隆源码到指定的本地源码路径
if not exist "%LUNASVG_SOURCE_PATH%" git clone %LUNASVG_REPO% %LUNASVG_SOURCE_PATH%

rem 如果lunasvg的Visual Studio项目路径不存在，则创建该目录，用于后续的CMake生成项目文件以及构建操作
if not exist "%LUNASVG_VSPROJECT_PATH%" mkdir "%LUNASVG_VSPROJECT_PATH%"
rem 切换到lunasvg的Visual Studio项目目录下，后续的CMake操作在此目录中进行
cd "%LUNASVG_VSPROJECT_PATH%"

rem 使用CMake配置生成Visual Studio 2019项目文件，指定生成器为"Visual Studio 16 2019"并指定平台为x64架构，
rem 设置安装前缀（处理路径格式），指定源码路径，用于后续构建安装lunasvg库
cmake -G "Visual Studio 16 2019" -A x64^
    -DCMAKE_INSTALL_PREFIX="%DEPENDENCIES_INSTALLATION_PATH:\=/%"^
    "%LUNASVG_SOURCE_PATH%"

rem 使用CMake进行构建，指定构建配置为Release，并执行安装目标，将构建生成的文件安装到指定的安装前缀目录下
cmake --build. --config Release --target install


rem ===========================================================================
rem -- Build osm-map-renderer tool --------------------------------------------
rem ===========================================================================

rem 如果osm-map-renderer工具对应的Visual Studio项目路径不存在，则创建该目录，用于后续的CMake生成项目文件以及构建操作
if not exist "%OSM_RENDERER_VSPROJECT_PATH%" mkdir "%OSM_RENDERER_VSPROJECT_PATH%"
rem 切换到osm-map-renderer工具对应的Visual Studio项目目录下，后续的CMake操作在此目录中进行
cd "%OSM_RENDERER_VSPROJECT_PATH%"

rem 使用CMake配置生成Visual Studio 2019项目文件，指定生成器为"Visual Studio 16 2019"并指定平台为x64架构，
rem 设置C++编译Release模式下的一些编译标志，比如指定使用C++17标准、忽略特定的警告（wd4251）以及添加头文件搜索路径（包含boost相关的头文件路径），
rem 指定源码路径，用于后续构建osm-map-renderer工具
cmake -G "Visual Studio 16 2019" -A x64^
    -DCMAKE_CXX_FLAGS_RELEASE="/std:c++17 /wd4251 /I%INSTALLATION_DIR:/=\%boost-1.80.0-install\include"^
    "%OSM_RENDERER_SOURCE%"

rem 使用CMake进行构建，指定构建配置为Release，构建osm-map-renderer工具
cmake --build. --config Release

rem 以下这行重复了上一行的构建命令，可能是多余的，可根据实际情况确认是否需要删除
rem cmake --build. --config Release

rem 以下这行被注释掉了，原本可能是想在构建时设置osm-map-renderer相关的某个构建选项（关闭某个与地图Qt相关的功能构建）
rem    -DOSMSCOUT_BUILD_MAP_QT=OFF^

rem 将依赖项安装目录下的bin文件夹内容复制到osm-map-renderer工具的Release目录下，可能是为了将相关的依赖库二进制文件复制过来以便工具能正常运行
copy "%DEPENDENCIES_INSTALLATION_PATH:/=\%"bin "%OSM_RENDERER_VSPROJECT_PATH:/=\%"Release\
