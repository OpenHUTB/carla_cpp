@echo off
setlocal

rem 本BAT脚本用于下载并安装适用于CARLA（carla.org）的x64位xerces-c构建版本
rem 需要在启用了x64 Visual C++工具集的命令提示符（cmd）中运行

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
    rem 判断第一个参数是否为"-h"，常见的请求显示帮助信息的命令行选项
    if "%1"=="-h" (
        rem 跳转到help代码标签处显示帮助信息
        goto help
    )
    rem 判断第一个参数是否为"--help"，同样是请求帮助的命令行选项
    if "%1"=="--help" (
        goto help
    )
    rem 判断第一个参数是否为"--generator"，此参数可能用于指定生成器相关信息（比如特定的编译生成配置等）
    if "%1"=="--generator" (
        rem 将第二个参数的值赋给GENERATOR变量，以便后续依据此变量确定具体的生成配置
        set GENERATOR=%2
        rem 左移命令行参数列表，准备解析下一轮的参数
        shift
    )
    rem 左移命令行参数列表，准备解析下一轮的参数
    shift
    rem 跳回 :arg-parse 标签处，继续循环解析剩余参数
    goto :arg-parse
)

rem 如果GENERATOR变量未设置，则默认设置为"Visual Studio 16 2019"（可能对应一种默认的编译生成配置情况）
if %GENERATOR% == "" set GENERATOR="Visual Studio 16 2019"

rem 如果未设置BUILD_DIR变量，则将其设置为当前目录
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
rem 如果BUILD_DIR路径末尾不是反斜杠"\", 则添加一个反斜杠，确保路径格式正确
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

rem ============================================================================
rem -- 定义本地变量部分 --
rem ============================================================================
rem 设置xerces-c的基础名称，例如文件名相关的主体部分
set XERCESC_BASENAME=xerces-c
rem 设置xerces-c的版本号
set XERCESC_VERSION=3.2.3

rem 设置临时文件夹名称，格式为：xerces-c-版本号
set XERCESC_TEMP_FOLDER=%XERCESC_BASENAME%-%XERCESC_VERSION%
rem 设置临时文件夹的完整路径，包含构建目录（BUILD_DIR）
set XERCESC_TEMP_FOLDER_DIR=%BUILD_DIR%%XERCESC_TEMP_FOLDER%
rem 设置临时文件名称，格式为：xerces-c-版本号-src.zip
set XERCESC_TEMP_FILE=%XERCESC_TEMP_FOLDER%-src.zip
rem 设置临时文件的完整路径，包含构建目录（BUILD_DIR）和文件名
set XERCESC_TEMP_FILE_DIR=%BUILD_DIR%%XERCESC_TEMP_FILE%

rem 设置xerces-c的官方仓库下载地址，根据版本号拼接而成
set XERCESC_REPO=https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-%XERCESC_VERSION%.zip
rem 设置xerces-c的备用仓库下载地址（可能用于官方地址下载失败等情况）
set XERCESC_BACKUP_REPO=https://carla-releases.s3.us-east-005.backblazeb2.com/Backup/xerces-c-%XERCESC_VERSION%.zip

rem 设置xerces-c源代码解压后的目录路径
set XERCESC_SRC_DIR=%BUILD_DIR%%XERCESC_BASENAME%-%XERCESC_VERSION%-source\
rem 设置xerces-c安装后的目录路径
set XERCESC_INSTALL_DIR=%BUILD_DIR%%XERCESC_BASENAME%-%XERCESC_VERSION%-install\

rem ============================================================================
rem -- 获取xerces-c相关操作部分 --
rem ============================================================================
rem 如果xerces-c已经安装（安装目录已存在），则跳转到already_build标签处处理
if exist "%XERCESC_INSTALL_DIR%" (
    goto already_build
)

rem 如果xerces-c源代码目录不存在
if not exist "%XERCESC_SRC_DIR%" (
    rem 如果临时文件也不存在，则尝试从官方仓库下载xerces-c
    if not exist "%XERCESC_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %XERCESC_BASENAME%.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%XERCESC_REPO%', '%XERCESC_TEMP_FILE_DIR%')"
    )
    rem 如果从官方仓库下载失败，临时文件仍不存在，则尝试从备用仓库下载
    if not exist "%XERCESC_TEMP_FILE_DIR%" (
        echo %FILE_N% Using %XERCESC_BASENAME% from backup.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%XERCESC_BACKUP_REPO%', '%XERCESC_TEMP_FILE_DIR%')"
    )
    rem 如果下载过程出现错误（errorlevel不为0），跳转到error_download标签处处理错误
    if %errorlevel% neq 0 goto error_download
    rem 下载成功后，提取下载的xerces-c压缩文件到指定构建目录
    echo %FILE_N% Extracting xerces from "%XERCESC_TEMP_FILE%".
    powershell -Command "Expand-Archive '%XERCESC_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%'"
    rem 如果提取过程出现错误，跳转到error_extracting标签处处理
    if %errorlevel% neq 0 goto error_extracting

    rem 删除不再需要的临时文件
    echo %FILE_N% Removing "%XERCESC_TEMP_FILE%"
    del "%XERCESC_TEMP_FILE_DIR%"
    rem 删除构建目录下的manifest文件夹（可能是下载解压附带的不需要的内容）
    echo %FILE_N% Removing dir "%BUILD_DIR%manifest"
    rmdir /s/q "%BUILD_DIR%manifest"

    rem 重命名解压后的文件夹为指定的源代码目录名称格式
    echo %FILE_N% Renaming dir %XERCESC_TEMP_FOLDER_DIR% to %XERCESC_BASENAME%-%XERCESC_VERSION%-source
    rename "%XERCESC_TEMP_FOLDER_DIR%" "%XERCESC_BASENAME%-%XERCESC_VERSION%-source"
) else (
    echo %FILE_N% Not downloading xerces because already exists the folder "%XERCESC_SRC_DIR%".
)

rem ============================================================================
rem -- 编译xerces-c部分 --
rem ============================================================================
rem 如果源代码目录下的build子目录不存在，则创建该子目录
if not exist "%XERCESC_SRC_DIR%build" (
    echo %FILE_N% Creating "%XERCESC_SRC_DIR%build"
    mkdir "%XERCESC_SRC_DIR%build"
)

rem 切换工作目录到源代码目录下的build子目录
cd "%XERCESC_SRC_DIR%build"

rem 如果安装目录下的lib子目录不存在，则创建该子目录，用于存放编译生成的库文件等
if not exist "%XERCESC_INSTALL_DIR%lib" (
    echo %FILE_N% Creating "%XERCESC_INSTALL_DIR%lib"
    mkdir "%XERCESC_INSTALL_DIR%lib"
)

rem 如果安装目录下的include子目录不存在，则创建该子目录，用于存放头文件等
if not exist "%XERCESC_INSTALL_DIR%include" (
    echo %FILE_N% Creating "%XERCESC_INSTALL_DIR%include"
    mkdir "%XERCESC_INSTALL_DIR%include"
)

rem 根据GENERATOR变量的值判断是否包含"Visual Studio"，如果包含则设置PLATFORM变量为 -A x64（可能用于指定64位平台相关编译配置），否则PLATFORM为空
echo.%GENERATOR% | findstr /C:"Visual Studio" >nul && (
    set PLATFORM=-A x64
) || (
    set PLATFORM=
)

rem 使用CMake配置构建项目，指定生成器（GENERATOR）、平台（PLATFORM）、安装前缀以及设置不构建共享库等参数，并指定源代码目录
cmake.. -G %GENERATOR% %PLATFORM%^
  -DCMAKE_INSTALL_PREFIX="%XERCESC_INSTALL_DIR:\=/%"^
  -DBUILD_SHARED_LIBS=OFF^
  "%BUILD_DIR%%XERCESC_BASENAME%-%XERCESC_VERSION%-source"
rem 如果CMake执行过程出现错误，跳转到error_cmake标签处处理
if %errorlevel% neq 0 goto error_cmake

rem 使用CMake进行构建并安装，指定配置为Release，目标为安装（即将编译好的文件安装到指定的安装目录）
cmake --build. --config Release --target install

rem 编译安装成功后跳转到success标签处
goto success

rem ============================================================================
rem -- 消息提示与错误处理部分 --
rem ============================================================================
rem 帮助信息显示标签，用于输出脚本的使用说明和参数介绍等帮助内容
:help
    echo %FILE_N% Download and install a xerces.
    echo "Usage: %FILE_N% [-h^|--help] [--build-dir] [--zlib-install-dir]"
    goto eof

rem 安装成功后的处理标签，输出成功信息并进行一些后续收尾操作
:success
    echo.
    echo %FILE_N% xerces has been successfully installed in "%XERCESC_INSTALL_DIR%"!
    goto good_exit

rem 如果xerces-c已经构建过的处理标签，提示已存在安装且告知如何强制重新构建
:already_build
    echo %FILE_N% A xerces installation already exists.
    echo %FILE_N% Delete "%XERCESC_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

rem 下载xerces-c出现错误时的处理标签，输出错误信息以及可能的原因和解决办法
:error_download
    echo.
    echo %FILE_N% [DOWNLOAD ERROR] An error ocurred while downloading xerces.
    echo %FILE_N% [DOWNLOAD ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%XERCESC_REPO%"
    echo %FILE_N% [DOWNLOAD ERROR] Workaround:
    echo %FILE_N%              - Download the xerces's source code and
    echo %FILE_N%                extract the content in
    echo %FILE_N%                "%XERCESC_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

rem 提取xerces-c压缩文件出现错误时的处理标签，输出错误信息及解决办法
:error_extracting
    echo.
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the xerces's source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%XERCESC_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

rem 编译xerces-c出现错误时的处理标签，输出错误信息以及可能的原因提示（比如需要安装Visual Studio及相关工具集等）
:error_compiling
    echo.
    echo %FILE_N% [COMPILING ERROR] An error ocurred while compiling with cl.exe.
    echo %FILE_N%              Possible causes:
    echo %FILE_N%              - Make sure you have Visual Studio installed.
    echo %FILE_N%              - Make sure you have the "x64 Visual C++ Toolset" in your path.
    echo %FILE_N%                For example, using the "Visual Studio x64 Native Tools Command Prompt",
    echo %FILE_N%                or the "vcvarsall.bat".
    goto bad_exit

rem 在生成库文件等过程中出现错误（原脚本中可能对应nmake相关错误，此处沿用原错误标签名，但注释更清晰）时的处理标签，输出错误信息
:error_generating_lib
    echo.
    echo %FILE_N% [NMAKE ERROR] An error ocurred while compiling and installing using nmake.
    goto bad_exit

rem 正常退出处理标签，输出退出信息，进行一些必要的变量设置等收尾操作，并以0状态码退出脚本，表示执行成功
:good_exit
    echo %FILE_N% Exiting...
    rem 设置一个用于后续检查安装情况的变量install_xerces，并赋值为xerces-c的安装目录路径
    endlocal & set install_xerces=%XERCESC_INSTALL_DIR%
    exit /b 0

rem 错误退出处理标签，删除已存在的安装目录（如果有），输出错误退出信息，结束局部变量设置，并以错误状态码退出脚本
:bad_exit
    if exist "%XERCESC_INSTALL_DIR%" rd /s /q "%XERCESC_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
