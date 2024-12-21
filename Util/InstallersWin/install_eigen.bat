REM @echo off
setlocal

rem BAT script that downloads and installs a ready to use
rem x64 eigen for CARLA (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

:arg-parse
if not "%1"=="" (
rem 判断第一个命令行参数是否等于"--build-dir"，可能用于指定构建相关的目录路径等操作
    if "%1"=="--build-dir" (
        rem 如果第一个参数是"--build-dir"，则将第二个参数（%~dpn2会获取去掉文件名只保留路径的形式）的值赋给BUILD_DIR变量，用于后续构建目录相关的操作
        set BUILD_DIR=%~dpn2
        rem 使用shift命令，将命令行参数列表整体左移一位，这样原本的第二个参数就变成了第一个参数，方便后续继续判断新的第一个参数情况
        shift
    )
    rem 判断第一个命令行参数是否等于"-h"，通常这是一种简洁的帮助命令选项格式
    if "%1"=="-h" (
        rem 如果参数是"-h"，则跳转到名为help的代码标签处执行相应帮助相关的代码逻辑，一般用于显示使用说明等
        goto help
    )
    rem 判断第一个命令行参数是否等于"--help"，这也是常见的用于请求显示帮助信息的命令行选项格式
    if "%1"=="--help" (
        rem 如果参数是"--help"，同样跳转到help代码标签处去执行帮助相关的操作逻辑
        goto help
    )
    rem 使用shift命令将命令行参数列表整体左移一位，为后续可能的其他参数解析等操作做准备，比如再次循环判断新的第一个参数
    shift
    rem :: 跳转到名为:arg-parse的代码标签处继续执行后续的参数解析或者基于参数的其他处理逻辑
    goto :arg-parse
)

rem If not set set the build dir to the current dir
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

rem ============================================================================
rem -- Get Eigen (CARLA dependency) -------------------------------------------
rem ============================================================================

set EIGEN_VERSION=3.1.0
set EIGEN_REPO=https://gitlab.com/libeigen/eigen/-/archive/%EIGEN_VERSION%/eigen-%EIGEN_VERSION%.zip
set EIGEN_BASENAME=eigen-%EIGEN_VERSION%

set EIGEN_SRC_DIR=%BUILD_DIR%%EIGEN_BASENAME%
set EIGEN_INSTALL_DIR=%BUILD_DIR%eigen-install
set EIGEN_INCLUDE=%EIGEN_INSTALL_DIR%\include
set EIGEN_TEMP_FILE=eigen-%EIGEN_VERSION%.zip
set EIGEN_TEMP_FILE_DIR=%BUILD_DIR%eigen-%EIGEN_VERSION%.zip

if not exist "%EIGEN_SRC_DIR%" (
    if not exist "%EIGEN_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %EIGEN_TEMP_FILE_DIR%.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%EIGEN_REPO%', '%EIGEN_TEMP_FILE_DIR%')"
    )
    if %errorlevel% neq 0 goto error_download_eigen
    rem Extract the downloaded library
    echo %FILE_N% Extracting eigen from "%EIGEN_TEMP_FILE%".
    powershell -Command "Expand-Archive '%EIGEN_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%'"
    if %errorlevel% neq 0 goto error_extracting
    echo %EIGEN_SRC_DIR%

    del %EIGEN_TEMP_FILE_DIR%

)

if not exist "%EIGEN_INSTALL_DIR%" (
    mkdir %EIGEN_INSTALL_DIR%
    mkdir %EIGEN_INCLUDE%
    mkdir %EIGEN_INCLUDE%\unsupported
    mkdir %EIGEN_INCLUDE%\Eigen
)

xcopy /q /Y /S /I /d "%EIGEN_SRC_DIR%\Eigen" "%EIGEN_INCLUDE%\Eigen"
xcopy /q /Y /S /I /d "%EIGEN_SRC_DIR%\unsupported\Eigen" "%EIGEN_INCLUDE%\unsupported\Eigen"
copy "%BUILD_DIR%..\Util\Patches\Eigen3.1.0\Macros.h" "%EIGEN_INCLUDE%\Eigen\src\Core\util\Macros.h"
copy "%BUILD_DIR%..\Util\Patches\Eigen3.1.0\VectorBlock.h" "%EIGEN_INCLUDE%\Eigen\src\Core\VectorBlock.h"

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    echo %FILE_N% Download and install a the Chrono library.
    echo "Usage: %FILE_N% [-h^|--help] [--build-dir] [--zlib-install-dir]"
    goto eof

:success
    echo.
    echo %FILE_N% Eigen has been successfully installed in "%EIGEN_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A xerces installation already exists.
    echo %FILE_N% Delete "%EIGEN_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_download_eigen
    echo.
    echo %FILE_N% [DOWNLOAD ERROR] An error ocurred while downloading xerces.
    echo %FILE_N% [DOWNLOAD ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%EIGEN_REPO%"
    echo %FILE_N% [DOWNLOAD ERROR] Workaround:
    echo %FILE_N%              - Download the xerces's source code and
    echo %FILE_N%                extract the content in
    echo %FILE_N%                "%EIGEN_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_download_chrono
    echo.
    echo %FILE_N% [DOWNLOAD ERROR] An error ocurred while downloading xerces.
    echo %FILE_N% [DOWNLOAD ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%XERCESC_REPO%"
    echo %FILE_N% [DOWNLOAD ERROR] Workaround:
    echo %FILE_N%              - Download the xerces's source code and
    echo %FILE_N%                extract the content in
    echo %FILE_N%                "%EIGEN_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_extracting
    echo.
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the xerces's source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%EIGEN_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_compiling
    echo.
    echo %FILE_N% [COMPILING ERROR] An error ocurred while compiling with cl.exe.
    echo %FILE_N%              Possible causes:
    echo %FILE_N%              - Make sure you have Visual Studio installed.
    echo %FILE_N%              - Make sure you have the "x64 Visual C++ Toolset" in your path.
    echo %FILE_N%                For example, using the "Visual Studio x64 Native Tools Command Prompt",
    echo %FILE_N%                or the "vcvarsall.bat".
    goto bad_exit

:error_generating_lib
    echo.
    echo %FILE_N% [NMAKE ERROR] An error ocurred while compiling and installing using nmake.
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    rem A return value used for checking for errors
    endlocal & set install_chrono=%CHRONO_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%EIGEN_INSTALL_DIR%" rd /s /q "%EIGEN_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
