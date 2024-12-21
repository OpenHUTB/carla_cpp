@echo off
setlocal

rem 这是一个批处理脚本，用于下载并安装所有插件
rem 通过启用x64 Visual C++工具集的cmd运行此脚本。

rem 设置脚本所在目录为本地路径
set LOCAL_PATH=%~dp0
rem 设置日志前缀，用于打印信息
set "FILE_N=    -[%~n0]:"

rem 打印批处理参数（调试目的）
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- 解析参数 ---------------------------------------------------------
rem ============================================================================

rem 初始化RELEASE变量为false，表示默认不是发布版本
set RELEASE=false

:arg-parse
rem 循环解析参数
if not "%1"=="" (
    rem 如果参数是--release，则将RELEASE设置为true
    if "%1"=="--release" (
        set RELEASE=true
        shift
    )
    rem 无论是否匹配到--release，都移除当前参数并继续解析下一个
    shift
    goto :arg-parse
)

rem ============================================================================
rem -- 克隆插件仓库 ---------------------------------------------------------
rem ============================================================================

rem 检查ROOT_PATH环境变量下是否存在Plugins目录
if not exist "%ROOT_PATH%Plugins" (
    rem 根据RELEASE变量决定是深度克隆还是普通克隆
    if %RELEASE% == true (
        echo Cloning for release...
        call git clone --depth=1 --recursive https://github.com/carla-simulator/carla-plugins.git  "%ROOT_PATH%Plugins"
    ) else (
        echo Cloning for build...
        call git clone --recursive https://github.com/carla-simulator/carla-plugins.git  "%ROOT_PATH%Plugins"
    )
    rem 如果git命令执行失败，则跳转到错误处理
    if %errorlevel% neq 0 goto error_git
) else (
    rem 如果Plugins目录已存在，则跳转到已存在处理
    goto already
)
rem 如果以上步骤都成功，则跳转到成功处理
goto success

rem ============================================================================
rem -- 消息和错误处理 -----------------------------------------------------
rem ============================================================================

:success
    echo.
    echo %FILE_N% "Plugins" has been successfully installed in "%ROOT_PATH%Plugins!"
    goto good_exit

:already
    echo.
    echo %FILE_N% "Plugins" already exists in "%ROOT_PATH%Plugins!"
    goto good_exit

:error_git
    echo.
    echo %FILE_N% [GIT ERROR] An error ocurred while executing the git.
    echo %FILE_N% [GIT ERROR] Possible causes:
    echo %FILE_N%              - Make sure "git" is installed.
    echo %FILE_N%              - Make sure it is available on your Windows "path".
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    endlocal
    exit /b 0

:bad_exit
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
