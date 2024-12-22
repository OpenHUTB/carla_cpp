@echo off
setlocal 

rem BAT script that downloads and installs a ready to use
rem rpclib build for CARLA (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================
rem 设置一个变量DEL_SRC，初始值为false，表示默认情况下不删除源文件
set DEL_SRC=false

:arg-parse
rem 判断传入的第一个命令行参数是否为空字符串，如果不为空则进入参数解析流程
if not "%1"=="" (
rem 判断第一个命令行参数是否等于"--build-dir"
    if "%1"=="--build-dir" (
        rem 如果等于"--build-dir"，则将第二个参数（去掉文件名部分只保留路径，即%~dpn2的含义）的值赋给BUILD_DIR变量，用于指定构建目录相关的路径
        set BUILD_DIR=%~dpn2
    )
rem 判断第一个命令行参数是否等于"--generator"
    if "%1"=="--generator" (
        rem 如果等于"--generator"，则将第二个命令行参数的值赋给GENERATOR变量，用于指定生成器相关的配置
        set GENERATOR=%2
        rem 使用shift命令移除第一个命令行参数，这样后续处理时 %1 就会指向原来的第二个参数，依次类推
        shift
    )
   rem 判断第一个命令行参数是否等于"--delete-src"
    if "%1"=="--delete-src" (
        rem :: 如果等于"--delete-src"，则设置DEL_SRC变量为true，可能用于标记是否要删除源文件之类的操作逻辑
        set DEL_SRC=true
    )
    rem 使用shift命令移除第一个命令行参数，方便后续可能继续进行的参数解析等操作（比如再次循环判断新的第一个参数情况）
    shift  
    rem 跳转到 :arg-parse 标签处继续执行代码逻辑，可能在那里还有其他针对参数解析后的处理代码
    goto :arg-parse
)

if %GENERATOR% == "" set GENERATOR="Visual Studio 16 2019"

rem If not set set the build dir to the current dir
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

set RPC_VERSION=v2.2.1_c5
set RPC_SRC=rpclib-src
set RPC_SRC_DIR=%BUILD_DIR%%RPC_SRC%\
set RPC_INSTALL=rpclib-install
set RPC_INSTALL_DIR=%BUILD_DIR%%RPC_INSTALL%\
set RPC_BUILD_DIR=%RPC_SRC_DIR%build\

set PUSHD_RPC=%RPC_SRC_DIR%

if exist "%RPC_INSTALL_DIR%" (
    goto already_build
)

if not exist "%RPC_SRC_DIR%" (
    echo %FILE_N% Cloning rpclib - version "%RPC_VERSION%"...

    echo git clone -b "%RPC_VERSION%" https://github.com/carla-simulator/rpclib.git "%RPC_SRC_DIR:~0,-1%"
    call git clone -b "%RPC_VERSION%" https://github.com/carla-simulator/rpclib.git "%RPC_SRC_DIR:~0,-1%"
    if %errorlevel% neq 0 goto error_git
) else (
    echo %FILE_N% Not cloning rpclib because already exists a folder called "%RPC_SRC%".
)

if not exist "%RPC_BUILD_DIR%" (
    echo %FILE_N% Creating "%RPC_BUILD_DIR%"
    mkdir "%RPC_BUILD_DIR%"
)

cd "%RPC_BUILD_DIR%"
echo %FILE_N% Generating build...

echo.%GENERATOR% | findstr /C:"Visual Studio" >nul && (
    set PLATFORM=-A x64
) || (
    set PLATFORM=
)


cmake .. -G %GENERATOR% %PLATFORM%^
        -DCMAKE_BUILD_TYPE=Release^
        -DRPCLIB_BUILD_EXAMPLES=OFF^
        -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
        -DCMAKE_INSTALL_PREFIX="%RPC_INSTALL_DIR:\=/%"^
        "%RPC_SRC_DIR%"
if %errorlevel% neq 0 goto error_cmake

echo %FILE_N% Building...
cmake --build . --config Release --target install

if %errorlevel% neq 0 goto error_install

rem Remove the downloaded rpclib source because is no more needed
if %DEL_SRC% == true (
  rd /s /q "%RPC_SRC_DIR%"
)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo.
    echo %FILE_N% rpclib has been successfully installed in "%RPC_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A rpclib installation already exists.
    echo %FILE_N% Delete "%RPC_INSTALL_DIR%" if you want to force a rebuild.
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
    echo %FILE_N%                - CMake 3.9.0 or higher is required.
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
    endlocal & set install_rpclib=done
    exit /b 0

:bad_exit
    if exist "%RPC_INSTALL_DIR%" rd /s /q "%RPC_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
