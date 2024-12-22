@echo off
setlocal

rem 这是一个批处理脚本（BAT script），用于下载并安装Recast和Detour库。
rem 需要在启用了x64 Visual C++工具集的命令提示符（cmd）中运行该脚本。

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem 打印批处理参数（用于调试目的），%*表示所有传入的命令行参数
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

set DEL_SRC=false
rem 设置一个变量DEL_SRC，初始值为false，用于后续判断是否删除下载的源码

:arg-parse
if not "%1"=="" (
    if "%1"=="--build-dir" (
        rem 如果命令行参数是--build-dir，将下一个参数（%2）作为构建目录路径赋值给BUILD_DIR变量，并将参数指针向后移动一位（shift）
        set BUILD_DIR=%~dpn2
        shift
    )

    if "%1"=="--delete-src" (
        rem 如果命令行参数是--delete-src，将DEL_SRC变量设置为true，表示要删除源码
        set DEL_SRC=true
    )
    if "%1"=="--generator" (
        rem 如果参数是--generator，将下一个参数（%2）作为生成器名称赋值给GENERATOR变量，并将参数指针向后移动一位（shift）
        set GENERATOR=%2
        shift
    )
    shift
    goto :arg-parse
)

rem 如果GENERATOR变量为空，设置默认的生成器为"Visual Studio 16 2019"
if %GENERATOR% == "" set GENERATOR="Visual Studio 16 2019"

rem 如果未设置构建目录，将其设置为当前目录，并且确保目录路径最后有反斜杠（\）
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

rem 设置Recast相关的一些变量，包括源码目录、安装目录、构建目录以及基础名称等
set RECAST_SRC=recast-src
set RECAST_SRC_DIR=%BUILD_DIR%%RECAST_SRC%\
set RECAST_INSTALL=recast-install
set RECAST_INSTALL_DIR=%BUILD_DIR%%RECAST_INSTALL%\
set RECAST_BUILD_DIR=%RECAST_SRC_DIR%build\
set RECAST_BASENAME=%RECAST_SRC%

rem 如果Recast已经安装（安装目录存在），则跳转到already_build标签处
if exist "%RECAST_INSTALL_DIR%" (
    goto already_build
)

rem 如果Recast源码目录不存在
if not exist "%RECAST_SRC_DIR%" (
    echo %FILE_N% Cloning "Recast & Detour"
    rem 使用git命令克隆Recast & Detour库的代码到指定的源码目录（去除最后一个字符，可能是去除末尾的反斜杠）
    call git clone https://github.com/carla-simulator/recastnavigation.git "%RECAST_SRC_DIR:~0,-1%"
    cd "%RECAST_SRC_DIR%"
    rem 切换到名为"carla"的分支（可能是特定版本或定制分支）
    call git checkout carla
    cd..
    rem 如果git操作出现错误（errorlevel非0），跳转到error_git标签处处理git相关错误情况
    if %errorlevel% neq 0 goto error_git
) else (
    echo %FILE_N% Not cloning "Recast & Detour" because already exists a folder called "%RECAST_SRC%".
)

rem 如果构建目录不存在，创建该目录，用于后续的构建操作
if not exist "%RECAST_BUILD_DIR%" (
    echo %FILE_N% Creating "%RECAST_BUILD_DIR%"
    mkdir "%RECAST_BUILD_DIR%"
)

rem 切换到构建目录，后续的生成构建文件等操作在此目录下进行
cd "%RECAST_BUILD_DIR%"
echo %FILE_N% Generating build...

rem 判断生成器名称中是否包含"Visual Studio"，如果包含则设置PLATFORM为-A x64，用于指定64位平台构建，否则PLATFORM为空
echo.%GENERATOR% | findstr /C:"Visual Studio" >nul && (
    set PLATFORM=-A x64
) || (
    set PLATFORM=
)

rem 使用cmake配置构建项目，指定生成器、平台、构建类型、C++编译Release模式的标志、安装前缀以及其他编译相关的标志和源码目录等参数
cmake.. -G %GENERATOR% %PLATFORM%^
    -DCMAKE_BUILD_TYPE=Release^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DCMAKE_INSTALL_PREFIX="%RECAST_INSTALL_DIR:\=/%"^
    -DCMAKE_CXX_FLAGS=/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING^
    "%RECAST_SRC_DIR%"
rem 如果cmake配置过程出现错误（errorlevel非0），跳转到error_cmake标签处处理配置错误情况
if %errorlevel%  neq 0 goto error_cmake

echo %FILE_N% Building...
rem 使用cmake进行构建并安装，指定构建配置为Release并执行安装目标
cmake --build. --config Release --target install

rem 如果构建安装过程出现错误（errorlevel非0），跳转到error_install标签处处理安装错误情况
if errorlevel  neq 0 goto error_install

rem 根据DEL_SRC变量的值判断是否删除下载的Recast & Detour源码，如果为true则删除源码目录
rem 因为源码在安装完成后可能不再需要，可节省磁盘空间
if %DEL_SRC% == true (
    rd /s /q "%RECAST_SRC_DIR%"
)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo.
    echo %FILE_N% "Recast & Detour" has been successfully installed in "%RECAST_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A "Recast & Detour" installation already exists.
    echo %FILE_N% Delete "%RECAST_INSTALL_DIR%" if you want to force a rebuild.
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
    rem 设置环境变量install_recast为Recast的安装目录路径，并返回0表示成功退出
    endlocal & set install_recast=%RECAST_INSTALL_DIR%
    exit /b 0

:bad_exit
    rem 如果安装目录存在，删除该安装目录（可能是构建安装出现错误，清理残留）
    if exist "%RECAST_INSTALL_DIR%" rd /s /q "%RECAST_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
