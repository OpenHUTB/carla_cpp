@echo off
setlocal

rem 本BAT脚本用于下载并安装适用于CARLA（carla.org）的x64位zlib构建版本
rem 需要在启用了x64 Visual C++工具集的命令提示符（cmd）中运行

rem 清除MAKEFLAGS变量的值（如果有设置的话），后续可按需重新设置
set MAKEFLAGS=
rem 获取当前批处理脚本所在的路径
set LOCAL_PATH=%~dp0
rem 设置一个用于在打印信息中标识当前脚本的字符串格式，方便调试查看输出来源
set FILE_N=    -[%~n0]:

rem 打印批处理脚本接收到的命令行参数（用于调试目的）
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- 解析命令行参数部分 --
rem ============================================================================
rem 定义代码标签 :arg-parse，用于后续跳转到此处进行参数解析循环处理
:arg-parse
rem 判断第一个命令行参数是否不为空字符串，若不为空则意味着有传入的参数需要解析
if not "%1"=="" (
    rem 判断第一个参数是否为"--build-dir"，该参数通常用于指定构建目录路径
    if "%1"=="--build-dir" (
        rem 将第二个参数（去除文件名后的路径形式）赋值给BUILD_DIR变量，作为构建目录路径
        set BUILD_DIR=%~dpn2
        rem 左移命令行参数列表，使下一轮解析时原来的第二个参数变为第一个参数
        shift
    )
    rem 判断第一个参数是否为"--toolset"，用于指定使用的工具集相关信息
    if "%1"=="--toolset" (
        rem 将第二个参数的值赋给TOOLSET变量，以便后续依据此变量确定具体工具集
        set TOOLSET=%~2
        shift
    )
    rem 判断第一个参数是否为"-h"，常见的请求显示帮助信息的命令行选项
    if "%1"=="-h" (
        rem 跳转到help代码标签处显示帮助信息
        goto help
    )
    rem 判断第一个参数是否为"--help"，同样是请求帮助的命令行选项
    if "%1"=="--help" (
        goto help
    )
    rem 左移命令行参数列表，准备解析下一轮的参数
    shift
    rem 跳回 :arg-parse 标签处，继续循环解析剩余参数
    goto :arg-parse
)

rem 如果未设置BUILD_DIR变量，则将其设置为当前目录
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
rem 如果BUILD_DIR路径末尾不是反斜杠"\", 则添加一个反斜杠，确保路径格式正确
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

rem 如果未定义TOOLSET变量，则默认使用空字符串（后续可能对应默认的工具集情况，此处示例代码中未详细体现默认工具集逻辑）
if "%TOOLSET%" == "" set TOOLSET=""

rem ============================================================================
rem -- 定义本地变量部分 --
rem ============================================================================
rem 设置zlib的基础名称，例如文件名相关的主体部分
set ZLIB_BASENAME=zlib
rem 设置zlib的版本号
set ZLIB_VERSION=1.2.13

rem 设置临时文件夹名称，格式为：zlib-版本号
set ZLIB_TEMP_FOLDER=%ZLIB_BASENAME%-%ZLIB_VERSION%
rem 设置临时文件名称，格式为：临时文件夹名称.zip
set ZLIB_TEMP_FILE=%ZLIB_TEMP_FOLDER%.zip
rem 设置临时文件的完整路径，包含构建目录（BUILD_DIR）和文件名
set ZLIB_TEMP_FILE_DIR=%BUILD_DIR%%ZLIB_TEMP_FILE%

rem 设置zlib的官方仓库下载地址，根据版本号拼接而成
set ZLIB_REPO=https://github.com/madler/zlib/archive/refs/tags/v%ZLIB_VERSION%.zip
rem 设置zlib的备用仓库下载地址（可能用于官方地址下载失败等情况）
set ZLIB_BACKUP_REPO=https://carla-releases.s3.us-east-005.backblazeb2.com/Backup/zlib%ZLIB_VERSION:.=%.zip
rem 设置zlib源代码解压后的目录路径
set ZLIB_SRC_DIR=%BUILD_DIR%%ZLIB_BASENAME%-source\
rem 设置zlib安装后的目录路径
set ZLIB_INSTALL_DIR=%BUILD_DIR%%ZLIB_BASENAME%-install\

rem ============================================================================
rem -- 获取zlib相关操作部分 --
rem ============================================================================
rem 如果zlib已经安装（安装目录已存在），则跳转到already_build标签处处理
if exist "%ZLIB_INSTALL_DIR%" (
    goto already_build
)

rem 如果zlib源代码目录不存在
if not exist "%ZLIB_SRC_DIR%" (
    rem 如果临时文件也不存在，则尝试从官方仓库下载zlib
    if not exist "%ZLIB_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %ZLIB_BASENAME%.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%ZLIB_REPO%', '%ZLIB_TEMP_FILE_DIR%')"
    )
    rem 如果从官方仓库下载失败，临时文件仍不存在，则尝试从备用仓库下载
    if not exist "%ZLIB_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %ZLIB_BASENAME% from backup.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%ZLIB_BACKUP_REPO%', '%ZLIB_TEMP_FILE_DIR%')"
    )
    rem 如果下载过程出现错误（errorlevel不为0），跳转到error_download标签处处理错误
    if %errorlevel% neq 0 goto error_download
    rem 下载成功后，提取下载的zlib压缩文件到指定构建目录
    echo %FILE_N% Extracting zlib from "%ZLIB_TEMP_FILE%".
    powershell -Command "Expand-Archive '%ZLIB_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%'"
    rem 如果提取过程出现错误，跳转到error_extracting标签处处理
    if %errorlevel% neq 0 goto error_extracting
    rem 删除不再需要的下载的临时文件
    echo %FILE_N% Removing "%ZLIB_TEMP_FILE%"
    del "%ZLIB_TEMP_FILE_DIR%"
    rem 重命名解压后的文件夹为指定的源代码目录名称格式
    rename "%BUILD_DIR%%ZLIB_TEMP_FOLDER%" "%ZLIB_BASENAME%-source"
) else (
    echo %FILE_N% Not downloading zlib because already exists the folder "%ZLIB_SRC_DIR%".
)

rem 如果源代码目录下的build子目录不存在，则创建该子目录
if not exist "%ZLIB_SRC_DIR%build" (
    echo %FILE_N% Creating "%ZLIB_SRC_DIR%build"
    mkdir "%ZLIB_SRC_DIR%build"
)

rem 切换工作目录到源代码目录下的build子目录
cd "%ZLIB_SRC_DIR%build"

rem 使用CMake配置构建项目，指定生成NMake Makefiles，设置安装前缀以及构建类型为Release，并指定源代码目录
cmake -G "NMake Makefiles"^
 -DCMAKE_INSTALL_PREFIX="%ZLIB_INSTALL_DIR:\=/%"^
 -DCMAKE_BUILD_TYPE=Release^
 "%ZLIB_SRC_DIR%"
rem 如果CMake执行过程出现错误，跳转到error_cmake标签处处理
if %errorlevel% neq 0 goto error_cmake

rem 设置编译选项，利用多核进行编译（参考Stack Overflow相关做法）
set CL=/MP

rem 使用nmake进行安装
nmake install
rem 如果nmake安装过程出现错误，跳转到error_install标签处处理
if %errorlevel% neq 0 goto error_install

rem 安装成功后跳转到success标签处
goto success

rem ============================================================================
rem -- 消息提示与错误处理部分 --
rem ============================================================================
rem 帮助信息显示标签，用于输出脚本的使用说明和参数介绍等帮助内容
:help
    echo %FILE_N% Download and install a zlib.
    echo "Usage: %FILE_N% [-h^|--help] [--toolset] [--build-dir]"
    goto eof

rem 安装成功后的处理标签，输出成功信息并进行一些后续收尾操作
:success
    echo.
    echo %FILE_N% zlib has been successfully installed in "%ZLIB_INSTALL_DIR%"!
    goto good_exit

rem 如果zlib已经构建过的处理标签，提示已存在安装且告知如何强制重新构建
:already_build
    echo %FILE_N% A zlib installation already exists.
    echo %FILE_N% Delete "%ZLIB_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

rem 下载zlib出现错误时的处理标签，输出错误信息以及可能的原因和解决办法
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

rem 提取zlib压缩文件出现错误时的处理标签，输出错误信息及解决办法
:error_extracting
    echo.
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the libpng's source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%ZLIB_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

rem CMake执行出现错误时的处理标签，输出错误信息以及可能的原因提示
:error_cmake
    echo.
    echo %FILE_N% [CMAKE ERROR] An error ocurred while executing cmake command.
    echo %FILE_N% [CMAKE ERROR] Possible causes:
    echo %FILE_N%                - Make sure "CMake" is installed.
    echo %FILE_N%                - Make sure it is available on your Windows "path".
    echo %FILE_N%                - Make sure you have cmake 3.12.4 or higher installed.
    goto bad_exit

rem nmake安装出现错误时的处理标签，输出错误信息
:error_install
    echo.
    echo %FILE_N% [NMAKE ERROR] An error ocurred while compiling and installing using nmake.
    goto bad_exit

rem 正常退出处理标签，输出退出信息，进行一些必要的文件复制等收尾操作，并以0状态码退出脚本，表示执行成功
:good_exit
    echo %FILE_N% Exiting...
    rem 将安装目录下的zlibstatic.lib文件复制到CARLA_DEPENDENCIES_FOLDER\lib目录（此处CARLA_DEPENDENCIES_FOLDER变量未在前面定义，需根据实际情况确定其值）
    copy %ZLIB_INSTALL_DIR%\lib\zlibstatic.lib %CARLA_DEPENDENCIES_FOLDER%\lib
    endlocal & set install_zlib=%ZLIB_INSTALL_DIR%
    exit /b 0

rem 错误退出处理标签，删除已存在的安装目录（如果有），输出错误退出信息，结束局部变量设置，并以错误状态码退出脚本
:bad_exit
    if exist "%ZLIB_INSTALL_DIR%" rd /s /q "%ZLIB_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
