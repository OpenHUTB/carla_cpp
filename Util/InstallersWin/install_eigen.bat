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
    if "%1"=="--build-dir" (
        set BUILD_DIR=%~dpn2
        shift
    )
    if "%1"=="-h" (
        goto help
    )
    if "%1"=="--help" (
        goto help
    )
    shift
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
rem 以下是名为:already_build的代码标签部分，大概率用于处理已经存在相关安装（这里推测是Eigen库安装）的情况，告知用户相应的信息以及后续如果要强制重新构建需要做的操作
:already_build
    rem 输出提示信息，告知用户已经存在一个Eigen库的安装（这里根据代码文本推测，实际情况需结合更多上下文判断）
    echo %FILE_N% A xerces installation already exists.
    rem     :: 进一步输出提示信息，告诉用户如果想要强制重新构建Eigen库，需要删除EIGEN_INSTALL_DIR所指定的目录，引导用户进行后续操作
    echo %FILE_N% Delete "%EIGEN_INSTALL_DIR%" if you want to force a rebuild.
    rem 跳转到名为:good_exit的代码标签处，和上面类似，去执行统一的正常退出脚本的相关处理逻辑
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
