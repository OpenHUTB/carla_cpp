@echo off
setlocal

rem 这是一个BAT脚本，用于下载并安装适用于CARLA（carla.org）的x64版本的Chrono库。
rem 需要在启用了x64 Visual C++ Toolset的命令提示符（cmd）中运行此脚本。

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem 打印批处理参数（用于调试目的）
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

:arg-parse
rem 判断第一个参数是否为空，如果不为空则进入参数解析逻辑
if not "%1"=="" (
    rem 如果参数是"--build-dir"，则设置构建目录（BUILD_DIR）变量，取第二个参数的目录和文件名部分
    if "%1"=="--build-dir" (
        set BUILD_DIR=%~dpn2
        shift
    )
    rem 如果参数是"-h"或者"--help"，跳转到帮助信息显示部分（help标签处）
    if "%1"=="-h" (
        goto help
    )
    if "%1"=="--help" (
        goto help
    )
    rem 如果参数是"--generator"，则设置生成器（GENERATOR）变量为第二个参数
    if "%1"=="--generator" (
        set GENERATOR=%2
        shift
    )
    rem 移动参数位置，继续解析下一个参数
    shift
    goto :arg-parse
)

rem 如果构建目录（BUILD_DIR）变量未设置，则将其设置为当前目录
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
rem 如果构建目录最后一个字符不是反斜杠"\",则添加一个反斜杠
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\
rem 如果生成器（GENERATOR）变量为空，则设置默认的生成器值
if %GENERATOR% == "" set GENERATOR="Visual Studio 16 2019"

rem ============================================================================
rem -- Get Eigen (Chrono dependency) -------------------------------------------
rem ============================================================================

rem 设置Eigen的版本号
set EIGEN_VERSION=3.3.7
rem 设置Eigen的仓库下载地址
set EIGEN_REPO=https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.zip
rem 设置Eigen的基础文件名（基于版本号）
set EIGEN_BASENAME=eigen-%EIGEN_VERSION%

rem 设置Eigen源文件目录，基于构建目录和基础文件名
set EIGEN_SRC_DIR=%BUILD_DIR%%EIGEN_BASENAME%
rem 设置Eigen安装目录
set EIGEN_INSTALL_DIR=%BUILD_DIR%eigen-install
rem 设置Eigen包含目录（用于后续编译包含头文件等操作）
set EIGEN_INCLUDE=%EIGEN_INSTALL_DIR%\include
rem 设置临时下载的Eigen压缩文件名
set EIGEN_TEMP_FILE=eigen-%EIGEN_VERSION%.zip
rem 设置临时下载的Eigen压缩文件所在目录
set EIGEN_TEMP_FILE_DIR=%BUILD_DIR%eigen-%EIGEN_VERSION%.zip

rem 判断Eigen源文件目录是否不存在，如果不存在则进一步判断临时文件是否不存在
if not exist "%EIGEN_SRC_DIR%" (
    if not exist "%EIGEN_TEMP_FILE_DIR%" (
        rem 如果临时文件不存在，输出正在下载的提示信息，并使用PowerShell命令下载Eigen压缩文件
        echo %FILE_N% Retrieving %EIGEN_TEMP_FILE_DIR%.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%EIGEN_REPO%', '%EIGEN_TEMP_FILE_DIR%')"
    )
    rem 如果下载出现错误（错误码不为0），跳转到错误处理部分（error_download_eigen标签处）
    if %errorlevel% neq 0 goto error_download_eigen
    rem 输出正在解压Eigen的提示信息，并使用PowerShell命令解压下载的压缩文件到构建目录
    echo %FILE_N% Extracting eigen from "%EIGEN_TEMP_FILE%".
    powershell -Command "Expand-Archive '%EIGEN_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%'"
    if %errorlevel% neq 0 goto error_extracting
    echo %EIGEN_SRC_DIR%

    rem 删除临时下载的压缩文件
    del %EIGEN_TEMP_FILE_DIR%
)

rem 判断Eigen安装目录是否不存在，如果不存在则创建相关目录结构
if not exist "%EIGEN_INSTALL_DIR%" (
    mkdir %EIGEN_INSTALL_DIR%
    mkdir %EIGEN_INCLUDE%
    mkdir %EIGEN_INCLUDE%\unsupported
    mkdir %EIGEN_INCLUDE%\Eigen
)

rem 使用xcopy命令将Eigen源文件目录中的Eigen头文件相关内容复制到安装目录下的对应位置，进行静默复制（不显示复制过程信息）
xcopy /q /Y /S /I "%EIGEN_SRC_DIR%\Eigen" "%EIGEN_INCLUDE%\Eigen"
xcopy /q /Y /S /I "%EIGEN_SRC_DIR%\unsupported\Eigen" "%EIGEN_INCLUDE%\unsupported\Eigen"

rem ============================================================================
rem -- Get Chrono -------------------------------------------
rem ============================================================================

rem 设置Chrono的版本号
set CHRONO_VERSION=6.0.0
@REM set CHRONO_VERSION=develop
rem 设置Chrono的仓库克隆地址
set CHRONO_REPO=https://github.com/projectchrono/chrono.git
rem 设置Chrono的基础文件名
set CHRONO_BASENAME=chrono

rem 设置Chrono源文件目录，基于构建目录和基础文件名并添加-src后缀
set CHRONO_SRC_DIR=%BUILD_DIR%%CHRONO_BASENAME%-src
rem 设置Chrono安装目录
set CHRONO_INSTALL_DIR=%BUILD_DIR%chrono-install
rem 设置Chrono构建目录，位于源文件目录下的build文件夹
set CHRONO_BUILD_DIR=%CHRONO_SRC_DIR%\build

rem 判断Chrono安装目录是否不存在，如果不存在则执行以下下载、构建和安装操作
if not exist %CHRONO_INSTALL_DIR% (
    rem 输出正在获取Chrono的提示信息，并使用git命令克隆Chrono仓库到指定源文件目录，只克隆最新的一次提交（--depth 1）且指定分支（--branch）
    echo %FILE_N% Retrieving Chrono.
    call git clone --depth 1 --branch %CHRONO_VERSION% %CHRONO_REPO% %CHRONO_SRC_DIR%

    rem 创建Chrono构建目录和安装目录
    mkdir %CHRONO_BUILD_DIR%
    mkdir %CHRONO_INSTALL_DIR%

    rem 切换到Chrono构建目录
    cd "%CHRONO_BUILD_DIR%"

    rem 判断生成器（GENERATOR）中是否包含"Visual Studio"字符串，如果包含则设置平台为x64（用于后续cmake配置）
    echo.%GENERATOR% | findstr /C:"Visual Studio" >nul && (
        set PLATFORM=-A x64
    ) || (
        set PLATFORM=
    )

    rem 输出正在编译Chrono的提示信息，并使用cmake进行项目配置，指定生成器、平台、编译类型、各种编译选项、依赖的Eigen包含目录、安装前缀等参数
    echo %FILE_N% Compiling Chrono.
    cmake -G %GENERATOR% %PLATFORM%^
        -DCMAKE_BUILD_TYPE=Release^
        -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
        -DEIGEN3_INCLUDE_DIR="%EIGEN_INCLUDE%"^
        -DCMAKE_INSTALL_PREFIX="%CHRONO_INSTALL_DIR%"^
        -DENABLE_MODULE_VEHICLE=ON^
        %CHRONO_SRC_DIR%

    rem 输出正在构建的提示信息，并使用cmake进行构建，指定配置为Release并以安装为目标进行构建
    echo %FILE_N% Building...
    cmake --build. --config Release --target install

)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    rem 显示帮助信息，说明脚本的功能以及用法示例
    echo %FILE_N% Download and install a the Chrono library.
    echo "Usage: %FILE_N% [-h^|--help] [--build-dir] [--zlib-install-dir]"
    goto eof

:success
    echo.
    rem 输出Chrono成功安装的提示信息以及安装目录位置
    echo %FILE_N% Chrono has been successfully installed in "%EIGEN_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A xerces installation already exists.
    echo %FILE_N% Delete "%EIGEN_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_download_eigen
    echo.
    rem 输出下载Eigen时出现错误的提示信息以及可能的原因和解决办法
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
    rem 输出下载Chrono时出现错误的提示信息以及可能的原因和解决办法（此处原代码中XERCESC_REPO变量未定义，可能有误，推测应该类似Eigen的处理逻辑）
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
    rem 输出解压文件时出现错误的提示信息以及可能的解决办法
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the xerces's source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%EIGEN_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_compiling
    echo.
    rem 输出编译时出现错误的提示信息以及可能的原因，比如缺少Visual Studio或相关工具集没在环境变量路径中
    echo %FILE_N% [COMPILING ERROR] An error ocurred while compiling with cl.exe.
    echo %FILE_N%              Possible causes:
    echo %FILE_N%              - Make sure you have Visual Studio installed.
    echo %FILE_N%              - Make sure you have the "x64 Visual C++ Toolset" in your path.
    echo %FILE_N%                For example, using the "Visual Studio x64 Native Tools Command Prompt",
    echo %FILE_N%                or the "vcvarsall.bat".
    goto bad_exit

:error_generating_lib
    echo.
    rem 输出生成库时出现错误的提示信息（具体错误相关内容可能需要根据实际情况进一步完善）
    echo %FILE_N% [NMAKE ERROR] An error ocurred while compiling and installing using nmake.
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    rem 设置一个环境变量install_chrono为Chrono安装目录的值，并以0作为返回值正常退出脚本，用于外部检查是否执行成功
    endlocal & set install_chrono=%CHRONO_INSTALL_DIR%
    exit /b 0

:bad_exit
    rem 如果Eigen安装目录存在，则删除该目录，然后输出带错误的退出提示信息，以当前错误码退出脚本
    if exist "%EIGEN_INSTALL_DIR%" rd /s /q "%EIGEN_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
