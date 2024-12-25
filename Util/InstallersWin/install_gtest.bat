@echo off
setlocal

rem BAT script that downloads and installs a ready to use
rem Google Test build for CARLA (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

set DEL_SRC=false

:arg-parse  :: 这是一个标签，用于goto命令跳转，以循环解析命令行参数。

if not "%1"=="" (  :: 检查第一个参数（%1）是否不为空。
    :: 如果第一个参数是"--build-dir"，则执行以下操作：
    if "%1"=="--build-dir" (
        :: 注意：这里有一个潜在的问题。%~dpn2 实际上是引用第二个参数（%2）的盘符、路径和文件名（不包括扩展名）。
        :: 如果"--build-dir"后面没有参数，这将不会按预期工作。正确的做法可能是检查%2是否存在并有效。
        set BUILD_DIR=%~dpn2  :: 设置BUILD_DIR环境变量为紧随"--build-dir"之后的参数的路径（去除扩展名）。
        shift  :: 将参数列表左移一位，即原来的第二个参数现在成为新的第一个参数。
    )
    
    :: 如果第一个参数是"--generator"，则执行以下操作：
    if "%1"=="--generator" (
        :: 设置GENERATOR环境变量为紧随"--generator"之后的参数。
        :: 注意：这里假设"--generator"后面确实跟着一个有效的参数（%2）。
        set GENERATOR=%2
        shift  :: 将参数列表再次左移一位。
    )
    
    :: 如果第一个参数是"--delete-src"，则执行以下操作：
    if "%1"=="--delete-src" (
        :: 设置DEL_SRC环境变量为true，表示需要删除源文件。
        set DEL_SRC=true
    )
    
    :: 无论是否处理了当前参数，都再次使用shift命令将参数列表左移一位。
    shift
    
    :: 使用goto命令跳转到标签:arg-parse的位置，以继续循环解析剩余的参数。
    goto :arg-parse
)
:: 判断环境变量GENERATOR是否为空字符串，如果为空，则将其设置为指定的值"Visual Studio 16 2019"
if %GENERATOR% == "" set GENERATOR="Visual Studio 16 2019"

rem If not set set the build dir to the current dir
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

set GT_VERSION=release-1.8.1
set GT_SRC=gtest-src
set GT_SRC_DIR=%BUILD_DIR%%GT_SRC%\
set GT_INSTALL=gtest-install
set GT_INSTALL_DIR=%BUILD_DIR%%GT_INSTALL%\
set GT_BUILD_DIR=%GT_SRC_DIR%build\

if exist "%GT_INSTALL_DIR%" (
    goto already_build
)

if not exist "%GT_SRC_DIR%" (
    echo %FILE_N% Cloning Google Test - version "%GT_VERSION%"...

    call git clone --depth=1 -b "%GT_VERSION%" https://github.com/google/googletest.git "%GT_SRC_DIR:~0,-1%"
    if %errorlevel% neq 0 goto error_git
) else (
    echo %FILE_N% Not cloning Google Test because already exists a folder called "%GT_SRC%".
)

if not exist "%GT_BUILD_DIR%" (
    echo %FILE_N% Creating "%GT_BUILD_DIR%"
    mkdir "%GT_BUILD_DIR%"
)
rem 进入构建目录
cd "%GT_BUILD_DIR%"
echo %FILE_N% Generating build...

echo.%GENERATOR% | findstr /C:"Visual Studio" >nul && (
    set PLATFORM=-A x64
) || (
    set PLATFORM=
)

cmake .. -G %GENERATOR% %PLATFORM%^
    -DCMAKE_BUILD_TYPE=Release^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DCMAKE_INSTALL_PREFIX="%GT_INSTALL_DIR:\=/%"^
    -DCMAKE_CXX_FLAGS=/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING^
    "%GT_SRC_DIR%"
if %errorlevel%  neq 0 goto error_cmake

echo %FILE_N% Building...
cmake --build . --config Release --target install

if %errorlevel% neq 0 goto error_install

rem Remove the downloaded Google Test source because is no more needed
if %DEL_SRC% == true (
    rd /s /q "%GT_SRC_DIR%"
)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo.
    echo %FILE_N% Google Test has been successfully installed in "%GT_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A Google Test installation already exists.
    echo %FILE_N% Delete "%GT_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_git
    echo.
    echo %FILE_N% [GIT ERROR] An error ocurred while executing the git.
    echo %FILE_N% [GIT ERROR] Possible causes:
    echo %FILE_N%              - Make sure "git" is installed.
    echo %FILE_N%              - Make sure it is available on your Windows "path".
    goto bad_exit

:error_cmake
    echo.
    echo %FILE_N% [CMAKE ERROR] An error ocurred while executing the cmake.
    echo %FILE_N% [CMAKE ERROR] Possible causes:
    echo %FILE_N%                - Make sure "CMake" is installed.
    echo %FILE_N%                - Make sure it is available on your Windows "path".
    goto bad_exit

:error_install
    echo.
    echo %FILE_N% [%GENERATOR% Win64 ERROR] An error ocurred while installing using %GENERATOR% Win64.
    echo %FILE_N% [%GENERATOR% Win64 ERROR] Possible causes:
    echo %FILE_N%                - Make sure you have Visual Studio installed.
    echo %FILE_N%                - Make sure you have the "x64 Visual C++ Toolset" in your path.
    echo %FILE_N%                  For example using the "Visual Studio x64 Native Tools Command Prompt",
    echo %FILE_N%                  or the "vcvarsall.bat".
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    endlocal
    set install_gtest=done
    exit /b 0

:bad_exit
    if exist "%GT_INSTALL_DIR%" rd /s /q "%GT_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
