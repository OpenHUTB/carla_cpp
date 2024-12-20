@echo off
setlocal enabledelayedexpansion

rem BAT script that creates the library for conversion from OSM to OpenDRIVE (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0]:
# 设置 LOCAL_PATH 变量为当前批处理脚本所在的目录路径（%~dp0 表示获取当前批处理文件所在的驱动器和路径）。
# 设置 FILE_N 变量，格式为 [-脚本文件名:]，用于后续在输出信息中标识是哪个脚本产生的内容（%~n0 表示获取当前批处理文件的文件名，不包含扩展名和路径）。

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*
# 输出批处理脚本接收到的参数（用于调试目的），格式为 [脚本文件名:] [批处理参数列表]，%* 表示获取传递给脚本的所有参数
  
rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

set DOC_STRING=Build LibCarla.
set USAGE_STRING=Usage: %FILE_N% [-h^|--help] [--rebuild] [--build] [--clean] [--no-pull]
# 设置 DOC_STRING 变量，描述了脚本的主要功能（构建 LibCarla 库）。
# 设置 USAGE_STRING 变量，定义了脚本的使用帮助信息，说明了脚本接受的参数选项及格式。

set REMOVE_INTERMEDIATE=false
set BUILD_OSM2ODR=false
set GIT_PULL=true
set CURRENT_OSM2ODR_COMMIT=1835e1e9538d0778971acc8b19b111834aae7261
set OSM2ODR_BRANCH=aaron/defaultsidewalkwidth
set OSM2ODR_REPO=https://github.com/carla-simulator/sumo.git
# 设置多个变量的初始值：
# REMOVE_INTERMEDIATE 用于控制是否移除中间文件，初始设为 false，表示默认不移除。
# BUILD_OSM2ODR 用于控制是否构建 OSM2ODR 相关内容，初始设为 false，表示默认不构建。
# GIT_PULL 用于控制是否执行 Git 拉取操作，初始设为 true，表示默认执行拉取。
# CURRENT_OSM2ODR_COMMIT 记录了 OSM2ODR 相关的一个提交哈希值（具体用途需结合相关代码逻辑确定）。
# OSM2ODR_BRANCH 指定了 OSM2ODR 对应的 Git 仓库分支名称。
# OSM2ODR_REPO 定义了 OSM2ODR 对应的 Git 仓库的 URL 地址。

:arg-parse
if not "%1"=="" (
    if "%1"=="--rebuild" (
        set REMOVE_INTERMEDIATE=true
        set BUILD_OSM2ODR=true
    )
    if "%1"=="--build" (
        set BUILD_OSM2ODR=true
    )
    if "%1"=="--no-pull" (
        set GIT_PULL=false
    )
    if "%1"=="--clean" (
        set REMOVE_INTERMEDIATE=true
    )
    if "%1"=="--generator" (
        set GENERATOR=%2
        shift
    )
    if "%1"=="-h" (
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
    )
    if "%1"=="--help" (
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
    )
    shift
    goto :arg-parse
)
# 定义了一个标签为 :arg-parse 的代码块，用于解析传递给脚本的参数：
# 首先判断第一个参数（%1）是否为空，如果不为空，则进入参数匹配判断：
# 如果参数是 "--rebuild"，则将 REMOVE_INTERMEDIATE 和 BUILD_OSM2ODR 都设置为 true，意味着要移除中间文件并且构建 OSM2ODR 相关内容。
# 如果参数是 "--build"，则将 BUILD_OSM2ODR 设置为 true，即执行构建操作。
# 如果参数是 "--no-pull"，则将 GIT_PULL 设置为 false，也就是不执行 Git 拉取操作。
# 如果参数是 "--clean"，则将 REMOVE_INTERMEDIATE 设置为 true，仅移除中间文件。
# 如果参数是 "--generator"，则将第二个参数（%2）的值赋给 GENERATOR 变量，并通过 shift 命令将参数列表向左移动一位（去掉已处理的第一个参数）。
# 如果参数是 "-h" 或者 "--help"，则输出文档说明（DOC_STRING）和使用帮助信息（USAGE_STRING），然后通过 GOTO :eof 跳转到文件末尾结束脚本执行。
# 最后无论哪种情况，执行 shift 命令处理下一个参数，并跳转到 :arg-parse 标签处继续解析参数，直到所有参数都处理完。

if %REMOVE_INTERMEDIATE% == false (
    if %BUILD_OSM2ODR% == false (
        echo Nothing selected to be done.
        goto :eof
    )
)

rem ============================================================================
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

rem Set the visual studio solution directory
rem
set OSM2ODR_VSPROJECT_PATH=%INSTALLATION_DIR:/=\%osm2odr-visualstudio\
set OSM2ODR_SOURCE_PATH=%INSTALLATION_DIR:/=\%osm2odr-source\
set OSM2ODR_INSTALL_PATH=%ROOT_PATH:/=\%PythonAPI\carla\dependencies\
set OSM2ODR__SERVER_INSTALL_PATH=%ROOT_PATH:/=\%Unreal\CarlaUE4\Plugins\Carla\CarlaDependencies
set CARLA_DEPENDENCIES_FOLDER=%ROOT_PATH:/=\%Unreal\CarlaUE4\Plugins\Carla\CarlaDependencies\

if %GENERATOR% == "" set GENERATOR="Visual Studio 16 2019"

if %REMOVE_INTERMEDIATE% == true (
    rem Remove directories
    for %%G in (
        "%OSM2ODR_INSTALL_PATH%",
    ) do (
        if exist %%G (
            echo %FILE_N% Cleaning %%G
            rmdir /s/q %%G
        )
    )
)

rem Build OSM2ODR
if %BUILD_OSM2ODR% == true (
    cd "%INSTALLATION_DIR%"
    if not exist "%OSM2ODR_SOURCE_PATH%" (
        curl --retry 5 --retry-max-time 120 -L -o OSM2ODR.zip https://github.com/carla-simulator/sumo/archive/%CURRENT_OSM2ODR_COMMIT%.zip
        tar -xf OSM2ODR.zip
        del OSM2ODR.zip
        ren sumo-%CURRENT_OSM2ODR_COMMIT% osm2odr-source
    )
    
    cd ..
    if not exist "%OSM2ODR_VSPROJECT_PATH%" mkdir "%OSM2ODR_VSPROJECT_PATH%"
    cd "%OSM2ODR_VSPROJECT_PATH%"

    cmake -G %GENERATOR% -A x64^
        -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
        -DCMAKE_INSTALL_PREFIX="%OSM2ODR_INSTALL_PATH:\=/%"^
        -DPROJ_INCLUDE_DIR=%INSTALLATION_DIR:/=\%\proj-install\include^
        -DPROJ_LIBRARY=%INSTALLATION_DIR:/=\%\proj-install\lib\proj.lib^
        -DXercesC_INCLUDE_DIR=%INSTALLATION_DIR:/=\%\xerces-c-3.2.3-install\include^
        -DXercesC_LIBRARY=%INSTALLATION_DIR:/=\%\xerces-c-3.2.3-install\lib\xerces-c.lib^
        "%OSM2ODR_SOURCE_PATH%"
    if %errorlevel% neq 0 goto error_cmake

    cmake --build . --config Release --target install | findstr /V "Up-to-date:"
    if %errorlevel% neq 0 goto error_install
    copy %OSM2ODR_INSTALL_PATH%\lib\osm2odr.lib %CARLA_DEPENDENCIES_FOLDER%\lib
    copy %OSM2ODR_INSTALL_PATH%\include\OSM2ODR.h %CARLA_DEPENDENCIES_FOLDER%\include
)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    if %BUILD_OSM2ODR% == true echo %FILE_N% OSM2ODR has been successfully installed in "%OSM2ODR_INSTALL_PATH%"!
    goto good_exit

:error_cmake
    echo.
    echo %FILE_N% [ERROR] An error ocurred while executing the cmake.
    echo           [ERROR] Possible causes:
    echo           [ERROR]  - Make sure "CMake" is installed.
    echo           [ERROR]  - Make sure it is available on your Windows "path".
    echo           [ERROR]  - CMake 3.9.0 or higher is required.
    goto bad_exit

:error_install
    echo.
    echo %FILE_N% [ERROR] An error ocurred while installing using %GENERATOR% Win64.
    echo           [ERROR] Possible causes:
    echo           [ERROR]  - Make sure you have Visual Studio installed.
    echo           [ERROR]  - Make sure you have the "x64 Visual C++ Toolset" in your path.
    echo           [ERROR]    For example using the "Visual Studio x64 Native Tools Command Prompt",
    echo           [ERROR]    or the "vcvarsall.bat".
    goto bad_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b %errorlevel%
