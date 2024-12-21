@echo off
setlocal

rem BAT script that downloads and installs a ready to use
rem x64 zlib build for CARLA (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set MAKEFLAGS=
set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================
rem 定义名为:arg-parse的代码标签，这里应该是用于进行命令行参数解析相关的主要逻辑部分
:arg-parse
rem 判断第一个命令行参数是否不为空字符串，若不为空则意味着有传入的参数，需要进一步解析这些参数来执行相应操作，进入下面的条件判断分支
if not "%1"=="" (
    rem :: 判断第一个命令行参数是否等于"--build-dir"，通常这个参数可能用于指定构建相关的目录路径，比如项目构建输出目录等
    if "%1"=="--build-dir" (
        rem 如果第一个参数是"--build-dir"，则将第二个参数（%~dpn2 会获取去掉文件名只保留路径的形式）的值赋给BUILD_DIR变量，以便后续在构建操作中使用该指定的目录路径
        set BUILD_DIR=%~dpn2
        rem 使用shift命令，将命令行参数列表整体左移一位，这样原本的第二个参数就变成了第一个参数，方便后续继续判断新的第一个参数情况，进行持续的参数解析
        shift
    )
    rem 判断第一个命令行参数是否等于"--toolset"，此参数大概率是用于指定所使用的工具集相关信息，比如特定的编译工具版本等
    if "%1"=="--toolset" (
        rem 如果第一个参数是"--toolset"，则将第二个参数的值赋给TOOLSET变量，后续可以依据该变量来确定具体使用哪种工具集进行相关操作
        set TOOLSET=%~2
        rem 同样使用shift命令将命令行参数列表整体左移一位，准备解析后续的参数
        shift
    )
    rem 判断第一个命令行参数是否等于"-h"，这是一种比较简洁常见的用于请求显示帮助信息的命令行选项格式
    if "%1"=="-h" (
        rem 如果参数是"-h"，则跳转到名为help的代码标签处执行相应帮助相关的代码逻辑，比如显示脚本的使用说明、参数介绍等内容
        goto help
    )
    rem 判断第一个命令行参数是否等于"--help"，这也是常见的用于请求显示帮助信息的命令行选项格式，功能和"-h"类似
    if "%1"=="--help" (
        rem 如果参数是"--help"，同样跳转到help代码标签处去执行帮助相关的操作逻辑
        goto help
    )
    rem 使用shift命令将命令行参数列表整体左移一位，为后续可能的其他参数解析等操作做准备，比如再次循环判断新的第一个参数是否符合其他条件等情况
    shift
    rem  跳转到名为:arg-parse的代码标签处，形成一个循环，继续解析剩下的命令行参数，直至所有参数都被解析处理完
    goto :arg-parse
)

rem If not set set the build dir to the current dir
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

rem If not defined, use Visual Studio 2019 as tool set
if "%TOOLSET%" == "" set TOOLSET=""

rem ============================================================================
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

set ZLIB_BASENAME=zlib
set ZLIB_VERSION=1.2.13

set ZLIB_TEMP_FOLDER=%ZLIB_BASENAME%-%ZLIB_VERSION%
set ZLIB_TEMP_FILE=%ZLIB_TEMP_FOLDER%.zip
set ZLIB_TEMP_FILE_DIR=%BUILD_DIR%%ZLIB_TEMP_FILE%

set ZLIB_REPO=https://github.com/madler/zlib/archive/refs/tags/v%ZLIB_VERSION%.zip
set ZLIB_BACKUP_REPO=https://carla-releases.s3.us-east-005.backblazeb2.com/Backup/zlib%ZLIB_VERSION:.=%.zip
set ZLIB_SRC_DIR=%BUILD_DIR%%ZLIB_BASENAME%-source\
set ZLIB_INSTALL_DIR=%BUILD_DIR%%ZLIB_BASENAME%-install\

rem ============================================================================
rem -- Get zlib ---------------------------------------------------------------
rem ============================================================================

if exist "%ZLIB_INSTALL_DIR%" (
    goto already_build
)

if not exist "%ZLIB_SRC_DIR%" (
    if not exist "%ZLIB_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %ZLIB_BASENAME%.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%ZLIB_REPO%', '%ZLIB_TEMP_FILE_DIR%')"
    )
    if not exist "%ZLIB_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %ZLIB_BASENAME% from backup.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%ZLIB_BACKUP_REPO%', '%ZLIB_TEMP_FILE_DIR%')"
    )
    if %errorlevel% neq 0 goto error_download
    rem Extract the downloaded library
    echo %FILE_N% Extracting zlib from "%ZLIB_TEMP_FILE%".
    powershell -Command "Expand-Archive '%ZLIB_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%'"
    if %errorlevel% neq 0 goto error_extracting

    rem Remove the no longer needed downloaded file
    echo %FILE_N% Removing "%ZLIB_TEMP_FILE%"
    del "%ZLIB_TEMP_FILE_DIR%"
    rename "%BUILD_DIR%%ZLIB_TEMP_FOLDER%" "%ZLIB_BASENAME%-source"
) else (
    echo %FILE_N% Not downloading zlib because already exists the folder "%ZLIB_SRC_DIR%".
)

if not exist "%ZLIB_SRC_DIR%build" (
    echo %FILE_N% Creating "%ZLIB_SRC_DIR%build"
    mkdir "%ZLIB_SRC_DIR%build"
)

cd "%ZLIB_SRC_DIR%build"

rem -DCMAKE_BUILD_TYPE=Release^
rem -DCMAKE_CONFIGURATION_TYPES=Release^
cmake -G "NMake Makefiles"^
 -DCMAKE_INSTALL_PREFIX="%ZLIB_INSTALL_DIR:\=/%"^
 -DCMAKE_BUILD_TYPE=Release^
 "%ZLIB_SRC_DIR%"
if %errorlevel% neq 0 goto error_cmake

rem https://stackoverflow.com/questions/601970/how-do-i-utilise-all-the-cores-for-nmake
set CL=/MP

nmake install
if %errorlevel% neq 0 goto error_install

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    echo %FILE_N% Download and install a zlib.
    echo "Usage: %FILE_N% [-h^|--help] [--toolset] [--build-dir]"
    goto eof

:success
    echo.
    echo %FILE_N% zlib has been successfully installed in "%ZLIB_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A zlib installation already exists.
    echo %FILE_N% Delete "%ZLIB_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_download
    echo.
    echo %FILE_N% [DOWNLOAD ERROR] An error ocurred while downloading zlib.
    echo %FILE_N% [DOWNLOAD ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%ZLIB_REPO%"
    echo %FILE_N% [DOWNLOAD ERROR] Workaround:
    echo %FILE_N%              - Download the zlib's source code and
    echo %FILE_N%                extract the content in
    echo %FILE_N%                "%ZLIB_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_extracting
    echo.
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the libpng's source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%ZLIB_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_cmake
    echo.
    echo %FILE_N% [CMAKE ERROR] An error ocurred while executing cmake command.
    echo %FILE_N% [CMAKE ERROR] Possible causes:
    echo %FILE_N%                - Make sure "CMake" is installed.
    echo %FILE_N%                - Make sure it is available on your Windows "path".
    echo %FILE_N%                - Make sure you have cmake 3.12.4 or higher installed.
    goto bad_exit

:error_install
    echo.
    echo %FILE_N% [NMAKE ERROR] An error ocurred while compiling and installing using nmake.
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    rem A return value used for checking for errors
    copy %ZLIB_INSTALL_DIR%\lib\zlibstatic.lib %CARLA_DEPENDENCIES_FOLDER%\lib
    endlocal & set install_zlib=%ZLIB_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%ZLIB_INSTALL_DIR%" rd /s /q "%ZLIB_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
