@echo off
setlocal

rem BAT script that creates the client and the server of LibCarla (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

set DOC_STRING=Build LibCarla.
set USAGE_STRING=Usage: %FILE_N% [-h^|--help] [--rebuild] [--server] [--client] [--clean]

set REMOVE_INTERMEDIATE=false
set BUILD_SERVER=false
set BUILD_CLIENT=false

:arg-parse
if not "%1"=="" (
    if "%1"=="--rebuild" (
        set REMOVE_INTERMEDIATE=true
        set BUILD_SERVER=true
        set BUILD_CLIENT=true
    )
    if "%1"=="--server" (
        set BUILD_SERVER=true
    )
    if "%1"=="--client" (
        set BUILD_CLIENT=true
    )
    if "%1"=="--clean" (
        set REMOVE_INTERMEDIATE=true
    )
    if "%1"=="--generator" (
        set GENERATOR=%2
        shift
    )
    if "%1"=="-h" (
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
    )
    if "%1"=="--help" (
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
    )
    shift
    goto :arg-parse
)

if %REMOVE_INTERMEDIATE% == false (
    if %BUILD_SERVER% == false (
        if %BUILD_CLIENT% == false (
          echo Nothing selected to be done.
          goto :eof
        )
    )
)

rem ============================================================================
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

rem Set the visual studio solution directory
rem
set LIBCARLA_VSPROJECT_PATH=%INSTALLATION_DIR:/=\%libcarla-visualstudio\

if %GENERATOR% == "" set GENERATOR="Visual Studio 16 2019"


set LIBCARLA_SERVER_INSTALL_PATH=%ROOT_PATH:/=\%Unreal\CarlaUE4\Plugins\Carla\CarlaDependencies\
set LIBCARLA_CLIENT_INSTALL_PATH=%ROOT_PATH:/=\%PythonAPI\carla\dependencies\

if %REMOVE_INTERMEDIATE% == true (
    rem Remove directories
    for %%G in (
        "%LIBCARLA_SERVER_INSTALL_PATH%",
        "%LIBCARLA_CLIENT_INSTALL_PATH%",
    ) do (
        if exist %%G (
            echo %FILE_N% Cleaning %%G
            rmdir /s/q %%G
        )
    )

    rem Remove files
    for %%G in (
        "%ROOT_PATH:/=\%LibCarla\source\carla\Version.h"
    ) do (
        if exist %%G (
            echo %FILE_N% Cleaning %%G
            del %%G
        )
    )
)

if not exist "%LIBCARLA_VSPROJECT_PATH%" mkdir "%LIBCARLA_VSPROJECT_PATH%"
cd "%LIBCARLA_VSPROJECT_PATH%"

echo.%GENERATOR% | findstr /C:"Visual Studio" >nul && (
    set PLATFORM=-A x64
) || (
    set PLATFORM=
)

rem For some reason the findstr above sets an errorlevel even if it finds the string in this batch file.
set errorlevel=0


rem Build libcarla server
rem
rem 这一行是注释，说明接下来的代码块是用于构建libcarla服务器的。

if %BUILD_SERVER% == true (
    rem 检查环境变量BUILD_SERVER是否被设置为true，如果是，则执行以下构建步骤。

    cmake -G %GENERATOR% %PLATFORM%^
      -DCMAKE_BUILD_TYPE=Server^
      -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
      -DCMAKE_INSTALL_PREFIX="%LIBCARLA_SERVER_INSTALL_PATH:\=/%"^
      "%ROOT_PATH%"

    rem 使用cmake命令配置libcarla服务器的构建。
    rem -G %GENERATOR% 指定生成器（如Visual Studio、Makefiles等）。
    rem %PLATFORM% 可能是一个指定平台（如Win32、x64等）的变量，但在此脚本中未明确其值。
    rem -DCMAKE_BUILD_TYPE=Server 设置构建类型为Server。
    rem -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP" 为Release构建设置C++编译器标志，/MD使用DLL版本的运行时库，/MP启用并行编译。
    rem -DCMAKE_INSTALL_PREFIX="%LIBCARLA_SERVER_INSTALL_PATH:\=/%" 设置安装前缀，将路径中的反斜杠替换为正斜杠以适应Unix-like系统（尽管这在Windows批处理中可能不是必需的，除非cmake脚本有特定要求）。
    rem "%ROOT_PATH%" 指定cmake项目的根目录。

    rem 注意：^ 符号在这里可能是尝试在批处理文件中进行行继续的尝试，但在大多数情况下，这不是必需的，除非命令本身包含特殊字符或需要在同一行上继续。然而，在提供的代码段中，由于每行命令后都紧跟了^，这实际上可能导致语法错误，除非这是从更大的脚本中摘录出来的，并且原始脚本中某些行的^后面紧跟着其他命令或参数。

    if %errorlevel% neq 0 goto error_cmake

    rem 检查上一个命令（cmake配置）的退出代码。如果不为0（表示出错），则跳转到标签error_cmake（该标签应在脚本的其他地方定义）。

    cmake --build . --config Release --target install | findstr /V "Up-to-date:"

    rem 使用cmake构建并安装项目。--build . 表示在当前目录构建，--config Release 指定构建配置为Release，--target install 指定构建目标为安装。
    rem | findstr /V "Up-to-date:" 通过管道将构建输出传递给findstr命令，该命令过滤掉包含"Up-to-date:"的行（即，不显示那些表示没有变化的目标）。

    if %errorlevel% neq 0 goto error_install

    rem 再次检查退出代码。如果构建或安装过程中的任何命令失败（即退出代码不为0），则跳转到标签error_install。

)

rem Build libcarla client
rem
if %BUILD_CLIENT% == true (
    cmake -G %GENERATOR% %PLATFORM%^
      -DCMAKE_BUILD_TYPE=Client^
      -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
      -DCMAKE_INSTALL_PREFIX="%LIBCARLA_CLIENT_INSTALL_PATH:\=/%"^
      "%ROOT_PATH%"
    if %errorlevel% neq 0 goto error_cmake

    cmake --build . --config Release --target install | findstr /V "Up-to-date:"
    if %errorlevel% neq 0 goto error_install
)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    if %BUILD_SERVER% == true echo %FILE_N% LibCarla server has been successfully installed in "%LIBCARLA_SERVER_INSTALL_PATH%"!
    if %BUILD_CLIENT% == true echo %FILE_N% LibCarla client has been successfully installed in "%LIBCARLA_CLIENT_INSTALL_PATH%"!
    goto good_exit

:error_cmake
    echo.
    echo %FILE_N% [ERROR] An error ocurred while executing the cmake.
    echo           [ERROR] Possible causes:
    echo           [ERROR]  - Make sure "CMake" is installed.
    echo           [ERROR]  - Make sure it is available on your Windows "path".
    echo           [ERROR]  - CMake 3.9.0 or higher is required.
    goto bad_exit

:error_install
    echo.
    echo %FILE_N% [ERROR] An error ocurred while installing using %GENERATOR% Win64.
    echo           [ERROR] Possible causes:
    echo           [ERROR]  - Make sure you have Visual Studio installed.
    echo           [ERROR]  - Make sure you have the "x64 Visual C++ Toolset" in your path.
    echo           [ERROR]    For example using the "Visual Studio x64 Native Tools Command Prompt",
    echo           [ERROR]    or the "vcvarsall.bat".
    goto bad_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b %errorlevel%
