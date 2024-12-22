@echo off
setlocal

rem BAT script that downloads and installs Fast-DDS library
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem 打印批处理参数（用于调试目的），展示传入脚本的所有参数内容
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

set DEL_SRC=false

:arg-parse
:: 检查第一个参数（%1）是否不为空，若不为空则进入参数解析流程
if not "%1"=="" (
    :: 如果第一个参数是"--build-dir"，表示要指定构建目录
    if "%1"=="--build-dir" (
        :: 设置BUILD_DIR环境变量为第二个参数（%~dpn2）的路径（去除扩展名），%~dpn2表示第二个参数所在的盘符、路径和文件名（不包括扩展名），这样就获取到了用户指定的构建目录路径
        set BUILD_DIR=%~dpn2
        :: 使用shift命令将参数列表左移一位，即原来的第二个参数现在成为新的第一个参数，以便后续继续解析下一个参数
        shift
    )

    :: 如果第一个参数是"--delete-src"，表示需要在安装完成后删除源文件
    if "%1"=="--delete-src" (
        :: 设置DEL_SRC环境变量为true，表示需要删除源文件，后续会根据这个变量的值来决定是否执行删除操作
        set DEL_SRC=true
    )

    :: 无论是否处理了当前参数，都使用shift命令将参数列表左移一位，为处理下一个参数做准备
    shift
    :: 使用goto命令跳转到标签:arg-parse的位置继续执行代码（注意：:arg-parse标签应该在脚本的其他部分定义），继续循环解析剩余的参数
    goto :arg-parse
)

rem 如果没有通过参数指定构建目录，就将构建目录设置为当前目录
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

set FASTDDS_SRC=fastDDS-src
set FASTDDS_SRC_DIR=%BUILD_DIR%%FASTDDS_SRC%\
set FASTDDS_INSTALL=fastDDS-install
set FASTDDS_INSTALL_DIR=%BUILD_DIR%%FASTDDS_INSTALL%\
set FASTDDS_BUILD_DIR=%FASTDDS_SRC_DIR%build\
set FASTDDS_BASENAME=%FASTDDS_SRC%

rem 检查Fast-DDS是否已经安装，如果安装目录已存在，则跳转到相应的已安装处理逻辑
if exist "%FASTDDS_INSTALL_DIR%" (
    goto already_build
)

rem 如果Fast-DDS的源文件目录不存在，则执行克隆操作
if not exist "%FASTDDS_SRC_DIR%" (
    echo %FILE_N% Cloning "Fast-DDS"
    :: 使用git命令克隆Fast-DDS仓库到指定的源文件目录（去除末尾的反斜杠），获取项目的源代码
    call git clone https://github.com/eProsima/Fast-DDS.git "%FASTDDS_SRC_DIR:~0,-1%"
    :: 初始化git子模块，用于加载项目中依赖的其他子项目
    call git submodule init
    :: 更新git子模块，确保子模块的代码也是最新的
    call git submodule update
    :: 如果上述git相关操作返回的错误码不为0，说明出现错误，跳转到错误处理逻辑
    if %errorlevel% neq 0 goto error_git
) else (
    echo %FILE_N% Not cloning "Fast-DDS" because already exists a folder called "%FASTDDS_SRC%".
)

echo Compiling fastCDR dependency...

rem 如果fastCDR的构建目录不存在，则创建该目录，用于后续的构建操作
if not exist "%FASTDDS_SRC_DIR%/thirdparty/fastcdr/build" (
    echo %FILE_N% Creating "%FASTDDS_SRC_DIR%/thirdparty/fastcdr/build"
    cd "%FASTDDS_SRC_DIR%/thirdparty/fastcdr"
    mkdir build
    cd ../../
)

cd "%FASTDDS_SRC_DIR%/thirdparty/fastcdr/build"
echo %FILE_N% Generating build...

rem 使用cmake命令生成fastCDR的构建配置，指定生成器为"Visual Studio 16 2019"，平台为x64，构建类型为Release等相关参数，为后续的编译做准备
cmake.. -G "Visual Studio 16 2019" -A x64^
    -DCMAKE_BUILD_TYPE=Release^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DCMAKE_INSTALL_PREFIX="%FASTDDS_INSTALL_DIR:\=/%"^
    -DCMAKE_CXX_FLAGS=/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
if %errorlevel%  neq 0 goto error_cmake

echo %FILE_N% Building...
rem 使用cmake命令根据前面生成的配置进行构建，并安装fastCDR到指定的安装目录
cmake --build. --config Release --target install

if errorlevel  neq 0 goto error_install

cd ../../..

@REM echo Compiling asio dependency...

@REM if not exist "%FASTDDS_SRC_DIR%/thirdparty/asio/asio/build" (
@REM     echo %FILE_N% Creating "%FASTDDS_SRC_DIR%/thirdparty/asio/asio/build"
@REM     cd "%FASTDDS_SRC_DIR%/thirdparty/asio/asio"
@REM     mkdir build
@REM     cd ../../
@REM )

@REM cd "%FASTDDS_SRC_DIR%/thirdparty/asio/asio/build"
@REM echo %FILE_N% Generating build...

@REM cmake.. -G "Visual Studio 16 2019" -A x64^
@REM     -DCMAKE_BUILD_TYPE=Release^
@REM     -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
@REM     -DCMAKE_INSTALL_PREFIX="%FASTDDS_INSTALL_DIR:\=/%"^
@REM     -DCMAKE_CXX_FLAGS=/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
@REM if %errorlevel%  neq 0 goto error_cmake

@REM echo %FILE_N% Building...
@REM cmake --build. --config Release --target install

@REM if errorlevel  neq 0 goto error_install

@REM cd ../../../..

rem 如果Fast-DDS的构建目录不存在，则创建该目录
if not exist "%FASTDDS_BUILD_DIR%" (
    echo %FILE_N% Creating "%FASTDDS_BUILD_DIR%"
    mkdir "%FASTDDS_BUILD_DIR%"
)

cd "%FASTDDS_BUILD_DIR%"
echo %FILE_N% Generating build...

rem 使用cmake命令生成Fast-DDS自身的构建配置，同样指定生成器、平台、构建类型等参数，并指定Fast-DDS的源文件目录作为构建的源，为后续编译做准备
cmake.. -G "Visual Studio 16 2019" -A x64^
    -DCMAKE_BUILD_TYPE=Release^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DCMAKE_INSTALL_PREFIX="%FASTDDS_INSTALL_DIR:\=/%"^
    -DCMAKE_CXX_FLAGS=/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING^
    "%FASTDDS_SRC_DIR%"
if %errorlevel%  neq 0 goto error_cmake

echo %FILE_N% Building...
rem 使用cmake命令根据配置进行Fast-DDS的构建和安装操作
cmake --build. --config Release --target install

if errorlevel  neq 0 goto error_install

rem 根据DEL_SRC变量的值决定是否删除下载的Fast-DDS源文件目录，如果为true则执行删除操作，以节省磁盘空间
rem Remove the downloaded Fast-DDS source because is no more needed
if %DEL_SRC% == true (
    rd /s /q "%FASTDDS_SRC_DIR%"
)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo.
    echo %FILE_N% "Fast-DDS" has been successfully installed in "%FASTDDS_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A "Fast-DDS" installation already exists.
    echo %FILE_N% Delete "%FASTDDS_INSTALL_DIR%" if you want to force a rebuild.
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
    echo %FILE_N% [Visual Studio 16 2019 Win64 ERROR] An error ocurred while installing using Visual Studio 16 2019 Win64.
    echo %FILE_N% [Visual Studio 16 2019 Win64 ERROR] Possible causes:
    echo %FILE_N%                - Make sure you have Visual Studio installed.
    echo %FILE_N%                - Make sure you have the "x64 Visual C++ Toolset" in your path.
    echo %FILE_N%                  For example using the "Visual Studio x64 Native Tools Command Prompt",
    echo %FILE_N%                  or the "vcvarsall.bat".
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    endlocal & set install_recast=%FASTDDS_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%FASTDDS_INSTALL_DIR%" rd /s /q "%FASTDDS_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
