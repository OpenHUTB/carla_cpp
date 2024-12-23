@echo off
setlocal

rem 这是一个批处理脚本（BAT script），其功能是下载并安装所有插件（Plugins）。
rem 需要在启用了x64 Visual C++工具集的命令提示符（cmd）环境中运行该脚本。

set LOCAL_PATH=%~dp0
set "FILE_N=    -[%~n0]:"

rem 打印批处理传入的参数（用于调试目的），%*表示所有传入的命令行参数，通过这种方式可以查看实际运行脚本时传入了哪些参数。
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

set RELEASE=false

:arg-parse
if not "%1"=="" (
    if "%1"=="--release" (
        rem 如果传入的第一个参数是 --release，意味着要以发布（release）模式进行相关操作，
        rem 这里将 RELEASE 变量设置为 true，表示后续操作按照发布模式的逻辑执行，然后使用 shift 命令将参数指针向后移动一位，以便处理后续参数。
        set RELEASE=true
        shift
    )
    shift
    goto :arg-parse
)

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

rem 判断插件（Plugins）所在的目录（通过 %ROOT_PATH%Plugins 来指定路径，不过这里没看到 %ROOT_PATH% 的定义，应该是在外部环境或者后续执行环境中有相应赋值）是否不存在。
if not exist "%ROOT_PATH%Plugins" (
    rem 如果 RELEASE 变量为 true，即处于发布模式，输出提示信息表示正在为发布进行克隆操作，
    rem 然后使用 git clone 命令从指定的 GitHub 仓库（https://github.com/carla-simulator/carla-plugins.git）克隆插件代码到 %ROOT_PATH%Plugins 目录下，
    rem 这里使用了 --depth=1 参数表示只克隆最近一次提交的内容（浅克隆，可减少克隆的数据量），--recursive 参数表示递归克隆子模块（如果仓库包含子模块的话）。
    if %RELEASE% == true (
        echo Cloning for release...
        call git clone --depth=1 --recursive https://github.com/carla-simulator/carla-plugins.git "%ROOT_PATH%Plugins"
    ) else (
        rem 如果不是发布模式（即常规的构建等模式），同样执行 git clone 命令进行克隆操作，但不使用浅克隆的限制（没有 --depth=1 参数），完整克隆整个仓库内容。
        echo Cloning for build...
        call git clone --recursive https://github.com/carla-simulator/carla-plugins.git "%ROOT_PATH%Plugins"
    )
    rem 如果执行 git clone 命令出现错误（通过判断errorlevel是否不等于0来确定），则跳转到 error_git 标签处，执行相应的 Git 操作错误处理逻辑。
    if %errorlevel% neq 0 goto error_git
) else (
    rem 如果插件目录已经存在，则直接跳转到 already 标签处，执行相应的已存在处理逻辑。
    goto already
)
goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
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
