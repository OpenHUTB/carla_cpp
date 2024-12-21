@REM @echo off
setlocal enabledelayedexpansion

rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

#获取批处理脚本所在的路径，并将其存储在LOCAL_PATH变量中
set LOCAL_PATH=%~dp0
#批处理脚本自身的文件名
set FILE_N=-[%~n0]:

rem Print batch params (debug purpose)
#调试目的
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

#设置描述性的字符串变量DOC_STRING
set DOC_STRING=Build LibCarla.
#定义一个用法字符串
set USAGE_STRING=Usage: %FILE_N% [-h^|--help] [--rebuild] [--build] [--clean] [--no-pull]

#设置为假
set BUILD_STREETMAP=false
#设置为真
set GIT_PULL=true
#设置当前街道地图的提交编号
set CURRENT_STREETMAP_COMMIT=260273d6b7c3f28988cda31fd33441de7e272958
#将街道地图的分支设置为master
set STREETMAP_BRANCH=master
#设置街道地图的仓库地址
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
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

rem Set the visual studio solution directory
rem
set CARLA_PLUGINS_PATH=%ROOT_PATH:/=\%Unreal\CarlaUE4\Plugins\
set CARLA_STREETMAP_PLUGINS_PATH=%ROOT_PATH:/=\%Unreal\CarlaUE4\Plugins\StreetMap\

rem Build STREETMAP

if  %GIT_PULL% == true (
    if not exist "%CARLA_STREETMAP_PLUGINS_PATH%" git clone -b %STREETMAP_BRANCH% %STREETMAP_REPO% %CARLA_STREETMAP_PLUGINS_PATH%
    cd "%CARLA_STREETMAP_PLUGINS_PATH%"
    git fetch
    git checkout %CURRENT_STREETMAP_COMMIT%
)


goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
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
