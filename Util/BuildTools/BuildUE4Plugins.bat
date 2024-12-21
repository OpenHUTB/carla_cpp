@REM @echo off
setlocal enabledelayedexpansion

rem 通过启用x64 Visual C++工具集的cmd运行此脚本。

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0]：

rem 打印批处理参数（调试目的）
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- 解析参数 ---------------------------------------------------------
rem ============================================================================

set DOC_STRING=Build LibCarla.
set USAGE_STRING=Usage: %FILE_N% [-h^|--help] [--rebuild] [--build] [--clean] [--no-pull]

set BUILD_STREETMAP=false
set GIT_PULL=true
set CURRENT_STREETMAP_COMMIT=260273d6b7c3f28988cda31fd33441de7e272958
set STREETMAP_BRANCH=master
set STREETMAP_REPO=https://github.com/carla-simulator/StreetMap.git 

:arg-parse
if not "%1"=="" (
    if "%1"=="--rebuild" (
        set REMOVE_INTERMEDIATE=true
        set BUILD_STREETMAP=true
    )
    if "%1"=="--build" (
        set BUILD_STREETMAP=true
    )
    if "%1"=="--no-pull" (
        set GIT_PULL=false
    )
    if "%1"=="--clean" (
        set REMOVE_INTERMEDIATE=true
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

rem ============================================================================
rem -- 本地变量 ---------------------------------------------------------
rem ============================================================================

rem Set the visual studio solution directory
rem
set CARLA_PLUGINS_PATH=%ROOT_PATH:/=\%Unreal\CarlaUE4\Plugins\
set CARLA_STREETMAP_PLUGINS_PATH=%ROOT_PATH:/=\%Unreal\CarlaUE4\Plugins\StreetMap\

rem 构建STREETMAP

if  %GIT_PULL% == true (
    if not exist "%CARLA_STREETMAP_PLUGINS_PATH%" git clone -b %STREETMAP_BRANCH% %STREETMAP_REPO% %CARLA_STREETMAP_PLUGINS_PATH%
    cd "%CARLA_STREETMAP_PLUGINS_PATH%"
    git fetch
    git checkout %CURRENT_STREETMAP_COMMIT%
)

rem 如果需要执行构建操作，则跳转到成功消息
goto success

rem ============================================================================
rem -- 消息和错误 -----------------------------------------------------
rem ============================================================================

:success
    if %BUILD_STREETMAP% == true echo %FILE_N% STREETMAP has been successfully installed in "%CARLA_PLUGINS_PATH%"!
    goto good_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b %errorlevel%
