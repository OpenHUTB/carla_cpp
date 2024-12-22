@echo off
setlocal enabledelayedexpansion

rem 这是一个用于为Carla（carla.org）创建二进制文件的BAT脚本。
rem 请在启用了x64 Visual C++ 工具集的cmd中运行它。

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0]:

rem 打印批处理参数（用于调试）
echo %FILE_N% [Batch params]: %*
rem ============================================================================
rem -- 解析参数 --------------------------------------------------------------
rem ============================================================================

rem 初始化变量
set BUILD_UE4_EDITOR=false        rem 是否构建UE4编辑器
set LAUNCH_UE4_EDITOR=false       rem 是否启动UE4编辑器
set REMOVE_INTERMEDIATE=false     rem 是否移除中间文件
set USE_CARSIM=false              rem 是否使用CarSim
set USE_CHRONO=false              rem 是否使用Chrono
set USE_UNITY=true                rem 是否使用Unity
set CARSIM_STATE="CarSim OFF"     rem CarSim状态
set CHRONO_STATE="Chrono OFF"     rem Chrono状态
set UNITY_STATE="Unity ON"        rem Unity状态
set AT_LEAST_WRITE_OPTIONALMODULES=false rem 是否至少写入可选模块
set EDITOR_FLAGS=""               rem 编辑器标志
set USE_ROS2=false                rem 是否使用ROS2
set ROS2_STATE="Ros2 OFF"         rem ROS2状态

:arg-parse
echo %1
if not "%1"=="" (
    if "%1"=="--editor-flags" (
        set EDITOR_FLAGS=%2       rem 设置编辑器标志
        shift
    )
    if "%1"=="--build" (
        set BUILD_UE4_EDITOR=true rem 设置为构建UE4编辑器
    )
    if "%1"=="--launch" (
        set LAUNCH_UE4_EDITOR=true rem 设置为启动UE4编辑器
    )
    if "%1"=="--clean" (
        set REMOVE_INTERMEDIATE=true rem 设置为移除中间文件
    )
    if "%1"=="--carsim" (
        set USE_CARSIM=true       rem 设置为使用CarSim
    )
    if "%1"=="--chrono" (
        set USE_CHRONO=true       rem 设置为使用Chrono
    )
    if "%1"=="--ros2" (
        set USE_ROS2=true         rem 设置为使用ROS2
    )
    if "%1"=="--no-unity" (
        set USE_UNITY=false       rem 设置为不使用Unity
    )
    if "%1"=="--at-least-write-optionalmodules" (
        set AT_LEAST_WRITE_OPTIONALMODULES=true rem 设置为至少写入可选模块
    )
    if "%1"=="-h" (
        goto help                 rem 跳转到帮助信息
    )
    if "%1"=="--help" (
        goto help                 rem 跳转到帮助信息
    )
    shift                         rem 移动到下一个参数
    goto arg-parse                rem 继续解析参数
)

rem 可以在此处添加实际的构建逻辑和命令
rem 从参数中移除引号
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

rem 获取Unreal Engine的根目录
rem 如果未定义UE4_ROOT，则从注册表中查找
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
rem 确保UE4_ROOT的末尾有反斜杠
if not "%UE4_ROOT:~-1%"=="\" set UE4_ROOT=%UE4_ROOT%\

rem 设置Visual Studio解决方案的目录
rem 将ROOT_PATH中的斜杠替换为反斜杠，并拼接上Unreal\CarlaUE4\路径
set UE4_PROJECT_FOLDER=%ROOT_PATH:/=\%Unreal\CarlaUE4\
pushd "%UE4_PROJECT_FOLDER%"

rem 清除由构建系统生成的二进制文件和中间文件
rem
if %REMOVE_INTERMEDIATE% == true (
    rem 删除以下目录
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
            echo %FILE_N% Cleaning %%G
            rmdir /s/q %%G
        )
    )

    rem 删除以下文件
    for %%G in (
        "%UE4_PROJECT_FOLDER%CarlaUE4.sln"
    ) do (
        if exist %%G (
            echo %FILE_N% Cleaning %%G
            del %%G
        )
    )
)

rem 构建Carla编辑器
rem

set OMNIVERSE_PATCH_FOLDER=%ROOT_PATH%Util\Patches\omniverse_4.26\  ; 设置Omniverse补丁文件夹路径
set OMNIVERSE_PLUGIN_FOLDER=%UE4_ROOT%Engine\Plugins\Marketplace\NVIDIA\Omniverse\  ; 设置Omniverse插件文件夹路径

; 检查Omniverse插件文件夹是否存在
if exist %OMNIVERSE_PLUGIN_FOLDER% (
    set OMNIVERSE_PLUGIN_INSTALLED="Omniverse ON"  ; 如果存在，设置Omniverse为已安装
    ; 复制USDCARLAInterface.h到OmniverseUSD的Public文件夹
    xcopy /Y /S /I "%OMNIVERSE_PATCH_FOLDER%USDCARLAInterface.h" "%OMNIVERSE_PLUGIN_FOLDER%Source\OmniverseUSD\Public\" > NUL
    ; 复制USDCARLAInterface.cpp到OmniverseUSD的Private文件夹
    xcopy /Y /S /I "%OMNIVERSE_PATCH_FOLDER%USDCARLAInterface.cpp" "%OMNIVERSE_PLUGIN_FOLDER%Source\OmniverseUSD\Private\" > NUL
) else (
    set OMNIVERSE_PLUGIN_INSTALLED="Omniverse OFF"  ; 如果不存在，设置Omniverse为未安装
)

; 根据USE_CARSIM环境变量的值启用或禁用CarSim
if %USE_CARSIM% == true (
    python %ROOT_PATH%Util/BuildTools/enable_carsim_to_uproject.py -f="%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject" -e  ; 启用CarSim
    set CARSIM_STATE="CarSim ON"  ; 设置CarSim状态为已启用
) else (
    python %ROOT_PATH%Util/BuildTools/enable_carsim_to_uproject.py -f="%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"  ; 禁用CarSim（不添加-e参数）
    set CARSIM_STATE="CarSim OFF"  ; 设置CarSim状态为未启用
)

; 根据USE_CHRONO环境变量的值设置Chrono状态
if %USE_CHRONO% == true (
    set CHRONO_STATE="Chrono ON"  ; 如果为true，设置Chrono为已启用
) else (
    set CHRONO_STATE="Chrono OFF"  ; 如果为false，设置Chrono为未启用
)

; 根据USE_ROS2环境变量的值设置ROS2状态
if %USE_ROS2% == true (
    set ROS2_STATE="Ros2 ON"  ; 如果为true，设置ROS2为已启用
) else (
    set ROS2_STATE="Ros2 OFF"  ; 如果为false，设置ROS2为未启用
)

; 根据USE_UNITY环境变量的值设置Unity状态
if %USE_UNITY% == true (
    set UNITY_STATE="Unity ON"  ; 如果为true，设置Unity为已启用
) else (
    set UNITY_STATE="Unity OFF"  ; 如果为false，设置Unity为未启用
)

; 设置OptionalModules.ini文件的内容，并写入该文件
set OPTIONAL_MODULES_TEXT=%CARSIM_STATE% %CHRONO_STATE% %ROS2_STATE% %OMNIVERSE_PLUGIN_INSTALLED% %UNITY_STATE%
echo %OPTIONAL_MODULES_TEXT% > "%ROOT_PATH%Unreal/CarlaUE4/Config/OptionalModules.ini"

rem 检查是否构建Unreal Editor
if %BUILD_UE4_EDITOR% == true (
    echo %FILE_N% Building Unreal Editor...

    call "%UE4_ROOT%Engine\Build\BatchFiles\Build.bat"^
        CarlaUE4Editor^
        Win64^
        Development^
        -WaitMutex^
        -FromMsBuild^
        "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"
    if errorlevel 1 goto bad_exit

    call "%UE4_ROOT%Engine\Build\BatchFiles\Build.bat"^
        CarlaUE4^
        Win64^
        Development^
        -WaitMutex^
        -FromMsBuild^
        "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"
    if errorlevel 1 goto bad_exit
)

rem 启动Carla Editor

if %LAUNCH_UE4_EDITOR% == true (
    echo %FILE_N% Launching Unreal Editor...
    call "%UE4_ROOT%\Engine\Binaries\Win64\UE4Editor.exe"^
        "%UE4_PROJECT_FOLDER%CarlaUE4.uproject" %EDITOR_FLAGS%
    if %errorlevel% neq 0 goto error_build
)

goto good_exit

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    echo Build LibCarla.
    echo "Usage: %FILE_N% [-h|--help] [--build] [--launch] [--clean]"
    goto good_exit
; 这个部分打印脚本的用途，并跳转到正常退出标签

:error_build
    echo.
    echo %FILE_N% [ERROR] There was a problem building CarlaUE4.
    echo %FILE_N%         Please go to "Carla\Unreal\CarlaUE4", right click on
    echo %FILE_N%         "CarlaUE4.uproject" and select:
    echo %FILE_N%         "Generate Visual Studio project files"
    echo %FILE_N%         Open the generated "CarlaUE4.sln" and try to manually compile it
    echo %FILE_N%         and check what is causing the error.
    goto bad_exit
; 这个部分处理构建CarlaUE4时出现的错误，并提供了一些手动编译的指示，然后跳转到错误退出标签

:good_exit
    endlocal
    exit /b 0
; 这个部分表示脚本的正常退出，它结束本地变量的作用域，并以退出码0退出脚本

:bad_exit
    endlocal
    exit /b %errorlevel%
; 这个部分表示脚本的错误退出，它也结束本地变量的作用域，但以当前的错误级别退出脚本

:error_unreal_no_found
    echo.
    echo %FILE_N% [ERROR] Unreal Engine not detected
    goto bad_exit
; 这个部分处理未检测到Unreal Engine的情况，并跳转到错误退出标签
