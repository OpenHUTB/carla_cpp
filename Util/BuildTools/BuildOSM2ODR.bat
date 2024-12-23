@echo off
setlocal enabledelayedexpansion

rem BAT脚本，用于创建从OSM到OpenDRIVE（carla.org）的转换库。
rem 在启用了x64 Visual C++ 工具集的cmd中运行它。

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0]:

rem 打印批处理参数（用于调试）
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- 解析参数 ----------------------------------------------------------------
rem ============================================================================

set DOC_STRING=Build LibCarla.
set USAGE_STRING=Usage: %FILE_N% [-h^|--help] [--rebuild] [--build] [--clean] [--no-pull]

set REMOVE_INTERMEDIATE=false
set BUILD_OSM2ODR=false
set GIT_PULL=true
set CURRENT_OSM2ODR_COMMIT=1835e1e9538d0778971acc8b19b111834aae7261
set OSM2ODR_BRANCH=aaron/defaultsidewalkwidth
set OSM2ODR_REPO=https://github.com/carla-simulator/sumo.git

:arg-parse
if not "%1"=="" (
    if "%1"=="--rebuild" (
        set REMOVE_INTERMEDIATE=true
        set BUILD_OSM2ODR=true
        rem 重新构建，会先清理中间文件
    )
    if "%1"=="--build" (
        set BUILD_OSM2ODR=true
        rem 仅构建，不清理中间文件
    )
    if "%1"=="--no-pull" (
        set GIT_PULL=false
        rem 不从Git拉取最新版本
    )
    if "%1"=="--clean" (
        set REMOVE_INTERMEDIATE=true
        rem 清理中间文件
    )
    if "%1"=="--generator" (
        set GENERATOR=%2
        shift
        rem 设置构建生成器
    )
    if "%1"=="-h" (
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
        rem 显示帮助信息并退出
    )
    if "%1"=="--help" (
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
        rem 显示帮助信息并退出
    )
    shift
    goto :arg-parse
)

if %REMOVE_INTERMEDIATE% == false (
    if %BUILD_OSM2ODR% == false (
        echo Nothing selected to be done.
        goto :eof
        rem 如果没有选择操作，则退出
    )
)

rem ============================================================================
rem -- 本地变量 ----------------------------------------------------------------
rem ============================================================================

rem 设置Visual Studio解决方案目录
rem
set OSM2ODR_VSPROJECT_PATH=%INSTALLATION_DIR:/=\%osm2odr-visualstudio\
set OSM2ODR_SOURCE_PATH=%INSTALLATION_DIR:/=\%osm2odr-source\
set OSM2ODR_INSTALL_PATH=%ROOT_PATH:/=\%PythonAPI\carla\dependencies\
set OSM2ODR__SERVER_INSTALL_PATH=%ROOT_PATH:/=\%Unreal\CarlaUE4\Plugins\Carla\CarlaDependencies
set CARLA_DEPENDENCIES_FOLDER=%ROOT_PATH:/=\%Unreal\CarlaUE4\Plugins\Carla\CarlaDependencies\

if %GENERATOR% == "" set GENERATOR="Visual Studio 16 2019"
rem 如果没有指定生成器，则默认为Visual Studio 16 2019

if %REMOVE_INTERMEDIATE% == true (
    rem 删除目录
    for %%G in (
        "%OSM2ODR_INSTALL_PATH%",
    ) do (
        if exist %%G (
            echo %FILE_N% Cleaning %%G
            rmdir /s/q %%G
            rem 删除指定的安装路径目录
        )
    )
)

rem 构建 OSM2ODR
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
        "%OSM2ODR_SOURCE_PATH%"  ; 此处调用 cmake 生成项目文件
    if %errorlevel% neq 0 goto error_cmake

    cmake --build . --config Release --target install | findstr /V "Up-to-date:"
    if %errorlevel% neq 0 goto error_install  ; 此处编译并安装 OSM2ODR
    copy %OSM2ODR_INSTALL_PATH%\lib\osm2odr.lib %CARLA_DEPENDENCIES_FOLDER%\lib
    copy %OSM2ODR_INSTALL_PATH%\include\OSM2ODR.h %CARLA_DEPENDENCIES_FOLDER%\include  ; 将编译生成的库文件和头文件复制到指定的依赖文件夹
)

goto success

rem ============================================================================
rem -- 消息和错误 -------------------------------------------------------------
rem ============================================================================

:success
    if %BUILD_OSM2ODR% == true echo %FILE_N% OSM2ODR 已在 "%OSM2ODR_INSTALL_PATH%" 成功安装！
    goto good_exit

:error_cmake
    echo.
    echo %FILE_N% [ERROR] 在执行 cmake 时发生错误。
    echo           [ERROR] 可能的原因：
    echo           [ERROR]  - 请确保已安装 "CMake"。
    echo           [ERROR] - 请确保它已在您的 Windows "path" 中可用。
    echo           [ERROR] - 需要 CMake 3.9.0 或更高版本。
    goto bad_exit

:error_install
    echo.
    echo %FILE_N% [ERROR]在使用 %GENERATOR% Win64 安装时发生错误。
    echo           [ERROR] 可能的原因：
    echo           [ERROR]  - 请确保已安装 Visual Studio。
    echo           [ERROR]  - 请确保已在您的路径中安装 "x64 Visual C++ Toolset"。
    echo           [ERROR]    例如，使用 "Visual Studio x64 Native Tools Command Prompt"，
    echo           [ERROR]    或 "vcvarsall.bat"。
    goto bad_exit

:good_exit
    endlocal
    exit /b 0  ; 成功退出脚本

:bad_exit
    endlocal
    exit /b %errorlevel%  ; 失败退出脚本并返回错误代码
