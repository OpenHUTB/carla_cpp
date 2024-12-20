@echo off
setlocal enabledelayedexpansion

:: BAT脚本，用于为Carla (carla.org)创建二进制文件。
:: 需要在启用了x64 Visual C++工具集的cmd中运行。

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0%]

:: 打印批处理参数（调试目的）
echo %FILE_N% [批处理参数]: %*

:: ============================================================================
:: -- 解析参数 ---------------------------------------------------------
:: ============================================================================

set BUILD_UE4_EDITOR=false
set LAUNCH_UE4_EDITOR=false
set REMOVE_INTERMEDIATE=false
set USE_CARSIM=false
set USE_CHRONO=false
set USE_UNITY=true
set CARSIM_STATE="CarSim OFF"
set CHRONO_STATE="Chrono OFF"
set UNITY_STATE="Unity ON"
set AT_LEAST_WRITE_OPTIONALMODULES=false
set EDITOR_FLAGS=
set USE_ROS2=false
set ROS2_STATE="Ros2 OFF"

:arg-parse
echo %1
if not "%1"=="" (
    if "%1"=="--editor-flags" (
        set EDITOR_FLAGS=%2
        shift
    )
    if "%1"=="--build" (
        set BUILD_UE4_EDITOR=true
    )
    if "%1"=="--launch" (
        set LAUNCH_UE4_EDITOR=true
    )
    if "%1"=="--clean" (
        set REMOVE_INTERMEDIATE=true
    )
    if "%1"=="--carsim" (
        set USE_CARSIM=true
    )
    if "%1"=="--chrono" (
        set USE_CHRONO=true
    )
    if "%1"=="--ros2" (
        set USE_ROS2=true
    )
    if "%1"=="--no-unity" (
        set USE_UNITY=false
    )
    if "%1"=="--at-least-write-optionalmodules" (
        set AT_LEAST_WRITE_OPTIONALMODULES=true
    )
    if "%1"=="-h" (
        goto help
    )
    if "%1"=="--help" (
        goto help
    )
    shift
    goto arg-parse
)

:: 从参数中移除引号
set EDITOR_FLAGS=%EDITOR_FLAGS:"=%

if %REMOVE_INTERMEDIATE% == false (
    if %LAUNCH_UE4_EDITOR% == false (
        if %BUILD_UE4_EDITOR% == false (
            if %AT_LEAST_WRITE_OPTIONALMODULES% == false (
                goto help
            )
        )
    )
)

:: 获取Unreal Engine根路径
if not defined UE4_ROOT (
    set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine"
    set VALUE_NAME=InstalledDirectory
    for /f "usebackq tokens=1,2,*" %%A in (`reg query !KEY_NAME! /s /reg:64`) do (
        if "%%A" == "!VALUE_NAME!" (
            set UE4_ROOT=%%C
        )
    )
    if not defined UE4_ROOT goto error_unreal_no_found
)
if not "%UE4_ROOT:~-1%"=="\" set UE4_ROOT=%UE4_ROOT%\

:: 设置Visual Studio解决方案目录
set UE4_PROJECT_FOLDER=%ROOT_PATH:/=\%Unreal\CarlaUE4\
pushd "%UE4_PROJECT_FOLDER%"

:: 清除构建系统生成的二进制文件和中间文件
if %REMOVE_INTERMEDIATE% == true (
    :: 删除目录
    for %%G in (
        "%UE4_PROJECT_FOLDER%Binaries",
        "%UE4_PROJECT_FOLDER%Build",
        "%UE4_PROJECT_FOLDER%Saved",
        "%UE4_PROJECT_FOLDER%Intermediate",
        "%UE4_PROJECT_FOLDER%Plugins\Carla\Binaries",
        "%UE4_PROJECT_FOLDER%Plugins\Carla\Intermediate",
        "%UE4_PROJECT_FOLDER%Plugins\HoudiniEngine\",
        "%UE4_PROJECT_FOLDER%.vs"
    ) do (
        if exist %%G (
            echo %FILE_N% 清理 %%G
            rmdir /s/q %%G
        )
    )

    :: 删除文件
    for %%G in (
        "%UE4_PROJECT_FOLDER%CarlaUE4.sln"
    ) do (
        if exist %%G (
            echo %FILE_N% 清理 %%G
            del %%G
        )
    )
)

:: 构建Carla编辑器
set OMNIVERSE_PATCH_FOLDER=%ROOT_PATH%Util\Patches\omniverse_4.26\
set OMNIVERSE_PLUGIN_FOLDER=%UE4_ROOT%Engine\Plugins\Marketplace\NVIDIA\Omniverse\
if exist %OMNIVERSE_PLUGIN_FOLDER% (
    set OMNIVERSE_PLUGIN_INSTALLED=Omniverse ON
    xcopy /Y /S /I "%OMNIVERSE_PATCH_FOLDER%USDCARLAInterface.h" "%OMNIVERSE_PLUGIN_FOLDER%Source\OmniverseUSD\Public\" > NUL
    xcopy /Y /S /I "%OMNIVERSE_PATCH_FOLDER%USDCARLAInterface.cpp" "%OMNIVERSE_PLUGIN_FOLDER%Source\OmniverseUSD\Private\" > NUL
) else (
    set OMNIVERSE_PLUGIN_INSTALLED=Omniverse OFF
)

if %USE_CARSIM% == true (
    python %ROOT_PATH%Util/BuildTools/enable_carsim_to_uproject.py -f="%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject" -e
    set CARSIM_STATE=CarSim ON
) else (
    python %ROOT_PATH%Util/BuildTools/enable_carsim_to_uproject.py -f="%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"
    set CARSIM_STATE=CarSim OFF
)
if %USE_CHRONO% == true (
    set CHRONO_STATE=Chrono ON
) else (
    set CHRONO_STATE=Chrono OFF
)
if %USE_ROS2% == true (
    set ROS2_STATE=Ros2 ON
) else (
    set ROS2_STATE=Ros2 OFF
)
if %USE_UNITY% == true (
    set UNITY_STATE=Unity ON
) else (
    set UNITY_STATE=Unity OFF
)
set OPTIONAL_MODULES_TEXT=%CARSIM_STATE% %CHRONO_STATE% %ROS2_STATE% %OMNIVERSE_PLUGIN_INSTALLED% %UNITY_STATE%
echo %OPTIONAL_MODULES_TEXT% > "%ROOT_PATH%Unreal/CarlaUE4/Config/OptionalModules.ini"

if %BUILD_UE4_EDITOR% == true (
    echo %FILE_N% 构建Unreal编辑器...
    call "%UE4_ROOT%Engine\Build\BatchFiles\Build.bat" ^
        CarlaUE4Editor ^
        Win64 ^
        Development ^
        -WaitMutex ^
        -FromMsBuild ^
        "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"
    if errorlevel 1 goto bad_exit

    call "%UE4_ROOT%Engine\Build\BatchFiles\Build.bat" ^
        CarlaUE4 ^
        Win64 ^
        Development ^
        -WaitMutex ^
        -FromMsBuild ^
        "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"
    if errorlevel 1 goto bad_exit
)

:: 启动Carla编辑器
if %LAUNCH_UE4_EDITOR% == true (
    echo %FILE_N% 启动Unreal编辑器...
    call "%UE4_ROOT%\Engine\Binaries\Win64\UE4Editor.exe" ^
        "%UE4_PROJECT_FOLDER%CarlaUE4.uproject" %EDITOR_FLAGS%
    if %errorlevel% neq 0 goto error_build
)

goto good_exit

:: ============================================================================
:: -- 消息和错误 -----------------------------------------------------
:: ============================================================================

:help
    echo 构建LibCarla。
    echo "用法：%FILE_N% [-h^|--help] [--build] [--launch] [--clean]"
    goto good_exit

:error_build
    echo.
    echo %FILE_N% [错误] 构建CarlaUE4时出现问题。
    echo %FILE_N%         请转到 "Carla\Unreal\CarlaUE4"，右键点击
    echo %FILE_N%         "CarlaUE4.uproject" 并选择：
    echo %FILE_N%         "生成Visual Studio项目文件"
    echo %FILE_N%         打开生成的 "CarlaUE4.sln" 并尝试手动编译它
    echo %FILE_N%         并检查是什么导致了错误。
    goto bad_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b %errorlevel%

:error_unreal_no_found
    echo.
    echo %FILE_N% [错误] 未检测到Unreal Engine
    goto bad_exit
