@echo off
setlocal enabledelayedexpansion 

rem 设置libosmscout库的仓库地址，从GitHub上获取该库的源码等相关资源
set LIBOSMSCOUT_REPO=https://github.com/Framstag/libosmscout
rem 设置lunasvg库的仓库地址，同样用于从GitHub获取对应源码
set LUNASVG_REPO=https://github.com/sammycage/lunasvg

rem 设置libosmscout库的源代码存放路径，将INSTALLATION_DIR中的斜杠替换为反斜杠，并添加libosmscout-source后缀，用于后续克隆代码等操作确定存放位置
set LIBOSMSCOUT_SOURCE_PATH=%INSTALLATION_DIR:/=\%libosmscout-source\
rem 设置libosmscout库的Visual Studio项目文件存放路径，类似源代码路径的处理方式，添加libosmscout-visualstudio后缀，用于后续生成、存放相关项目文件
set LIBOSMSCOUT_VSPROJECT_PATH=%INSTALLATION_DIR:/=\%libosmscout-visualstudio\

rem 设置lunasvg库的源代码存放路径，处理方式同libosmscout的源代码路径设置逻辑，添加lunasvg-source后缀确定位置
set LUNASVG_SOURCE_PATH=%INSTALLATION_DIR:/=\%lunasvg-source\
rem 设置lunasvg库的Visual Studio项目文件存放路径，添加lunasvg-visualstudio后缀来确定其项目文件存放位置
set LUNASVG_VSPROJECT_PATH=%INSTALLATION_DIR:/=\%lunasvg-visualstudio\

rem 设置osm-world-renderer（可能是一个地图渲染相关工具）的源代码路径，基于ROOT_PATH并替换斜杠为反斜杠，添加osm-world-renderer后缀确定位置
set OSM_RENDERER_SOURCE=%ROOT_PATH:/=\%osm-world-renderer\
rem 设置osm-world-renderer的Visual Studio项目文件存放路径，通过INSTALLATION_DIR相关处理并添加osm-world-renderer-visualstudio后缀来确定
set OSM_RENDERER_VSPROJECT_PATH=%INSTALLATION_DIR:/=\%%osm-world-renderer-visualstudio\

rem 设置服务器依赖项的安装路径，基于osm-world-renderer的源代码路径并添加ThirdParties后缀，用于存放依赖项安装后的文件等
set DEPENDENCIES_INSTALLATION_PATH=%OSM_RENDERER_SOURCE:/=\%ThirdParties\


rem ============================================================================
rem -- Download dependency manager for libosmscout -----------------------------
rem ============================================================================

rem 设置vcpkg（一个C++库管理工具）的仓库地址，用于获取vcpkg来管理后续项目的依赖库
set VCPKG_REPO=https://github.com/microsoft/vcpkg
rem 设置vcpkg在本地的存放路径，同样进行斜杠替换等操作，确定其在本地磁盘的具体位置
set VCPKG_PATH=%INSTALLATION_DIR:/=\%vcpkg\
rem 设置vcpkg的CMake工具链文件路径，用于在CMake构建项目时配置使用vcpkg管理的依赖库，位于vcpkg的scripts/buildsystems目录下
set VCPKG_CMAKE_TOOLCHAIN_PATH=%VCPKG_PATH:/=\%scripts\buildsystems\vcpkg.cmake

rem 判断vcpkg本地路径是否不存在，如果不存在则使用git命令克隆vcpkg仓库到指定的本地路径
if not exist "%VCPKG_PATH%" git clone %VCPKG_REPO% %VCPKG_PATH%

rem 进入vcpkg所在目录（这里原代码可能有点小问题，.\"%VCPKG_PATH:/=\%"这种写法不太准确，一般是直接写"%VCPKG_PATH:/=\%"，以下按正确理解注释）
rem 并执行bootstrap-vcpkg.bat脚本，用于初始化vcpkg，比如下载一些必要的构建工具等，使其可以正常使用
rem （原代码此处可能需要调整，比如去掉前面的点和双引号，如果是想在当前批处理脚本所在目录的相对路径下执行，需要确保路径解析正确）
rem 正确的示例调用可能像这样：
rem cd "%VCPKG_PATH:/=\%"
rem bootstrap-vcpkg.bat
rem （假设当前批处理脚本所在目录能正确解析相对路径找到vcpkg目录下的脚本）


rem ============================================================================
rem -- Download and build libosmscout ------------------------------------------
rem ============================================================================

rem 判断libosmscout源代码路径是否不存在，如果不存在则使用git命令从设置的仓库地址克隆代码到该路径下
if not exist "%LIBOSMSCOUT_SOURCE_PATH%" git clone %LIBOSMSCOUT_REPO% %LIBOSMSCOUT_SOURCE_PATH%

rem 判断libosmscout的Visual Studio项目文件路径是否不存在，如果不存在则创建该目录
if not exist "%LIBOSMSCOUT_VSPROJECT_PATH%" mkdir "%LIBOSMSCOUT_VSPROJECT_PATH%"
rem 切换到libosmscout的Visual Studio项目文件所在目录，后续在此目录下进行构建相关操作
cd "%LIBOSMSCOUT_VSPROJECT_PATH%"

rem 使用cmake命令进行项目配置，指定生成器为"Visual Studio 16 2019"，设置安装前缀为处理过斜杠的依赖项安装路径，同时关闭了一系列libosmscout相关工具、测试、客户端、演示等的构建选项，最后指定源代码路径
cmake -G "Visual Studio 16 2019"^
    -DCMAKE_INSTALL_PREFIX="%DEPENDENCIES_INSTALLATION_PATH:\=/%"^
    -DOSMSCOUT_BUILD_TOOL_STYLEEDITOR=OFF^
    -DOSMSCOUT_BUILD_TOOL_OSMSCOUT2=OFF^
    -DOSMSCOUT_BUILD_TESTS=OFF^
    -DOSMSCOUT_BUILD_CLIENT_QT=OFF^
    -DOSMSCOUT_BUILD_DEMOS=OFF^
    "%LIBOSMSCOUT_SOURCE_PATH%"

rem （原代码此处有一行被注释掉了，可能是之前用于设置Release编译类型下的C++编译标志，比如定义了数学常量PI的值，若需要可以取消注释使用）
rem -DCMAKE_CXX_FLAGS_RELEASE="/DM_PI=3.14159265358979323846"^

rem 使用cmake命令进行构建，指定配置为Release并以安装为目标进行构建，将编译生成的文件安装到指定的安装前缀目录下
cmake --build. --config=Release --target install


rem ============================================================================
rem -- Download and build lunasvg ----------------------------------------------
rem ============================================================================

rem 判断lunasvg源代码路径是否不存在，如果不存在则使用git命令从设置的仓库地址克隆代码到该路径下
if not exist "%LUNASVG_SOURCE_PATH%" git clone %LUNASVG_REPO% %LUNASVG_SOURCE_PATH%

rem 判断lunasvg的Visual Studio项目文件路径是否不存在，如果不存在则创建该目录
if not exist "%LUNASVG_VSPROJECT_PATH%" mkdir "%LUNASVG_VSPROJECT_PATH%"
rem 切换到lunasvg的Visual Studio项目文件所在目录，后续在此目录下进行构建相关操作
cd "%LUNASVG_VSPROJECT_PATH%"

rem 使用cmake命令进行项目配置，指定生成器为"Visual Studio 16 2019"并指定平台为x64，设置安装前缀为处理过斜杠的依赖项安装路径，最后指定源代码路径
cmake -G "Visual Studio 16 2019" -A x64^
    -DCMAKE_INSTALL_PREFIX="%DEPENDENCIES_INSTALLATION_PATH:\=/%"^
    "%LUNASVG_SOURCE_PATH%"

rem 使用cmake命令进行构建，指定配置为Release并以安装为目标进行构建，将编译生成的文件安装到指定的安装前缀目录下
cmake --build. --config Release --target install


rem ===========================================================================
rem -- Build osm-map-renderer tool --------------------------------------------
rem ===========================================================================

rem 判断osm-map-renderer的Visual Studio项目文件路径是否不存在，如果不存在则创建该目录
if not exist "%OSM_RENDERER_VSPROJECT_PATH%" mkdir "%OSM_RENDERER_VSPROJECT_PATH%"
rem 切换到osm-map-renderer的Visual Studio项目文件所在目录，后续在此目录下进行构建相关操作
cd "%OSM_RENDERER_VSPROJECT_PATH%"

rem 使用cmake命令进行项目配置，指定生成器为"Visual Studio 16 2019"并指定平台为x64，设置Release编译类型下的C++编译标志（如指定C++标准为17、忽略特定警告、添加特定头文件包含路径等），最后指定源代码路径
cmake -G "Visual Studio 16 2019" -A x64^
    -DCMAKE_CXX_FLAGS_RELEASE="/std:c++17 /wd4251 /I%INSTALLATION_DIR:/=\%boost-1.80.0-install\include"^
    "%OSM_RENDERER_SOURCE%"

rem 使用cmake命令进行构建，指定配置为Release进行构建，但这里原代码重复写了两次构建命令，可根据实际需求保留一个即可
cmake --build. --config Release
rem cmake --build. --config Release

rem （原代码此处有一行被注释掉了，可能是之前用于设置构建相关的某个选项，比如关闭某个与地图Qt相关的功能构建，若需要可以取消注释使用）
rem    -DOSMSCOUT_BUILD_MAP_QT=OFF^

rem 使用copy命令将依赖项安装路径下的bin目录内容复制到osm-map-renderer的Visual Studio项目文件所在目录下的Release目录中，可能是为了将依赖的可执行文件等复制过来确保工具能正常运行
copy "%DEPENDENCIES_INSTALLATION_PATH:/=\%"bin "%OSM_RENDERER_VSPROJECT_PATH:/=\%"Release\
