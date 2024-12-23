@echo off
setlocal enabledelayedexpansion

rem 本BAT脚本用于下载并安装适用于CARLA（carla.org）的Boost库构建版本

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
    rem 判断第一个参数是否为“-j”，可能用于指定异步任务数量（比如并行编译的任务数量等）
    if "%1"=="-j" (
        rem 将第二个参数的值赋给NUMBER_OF_ASYNC_JOBS变量，用于后续控制相关操作的并行度
        set NUMBER_OF_ASYNC_JOBS=%~2
        rem 左移命令行参数列表，使下一轮解析时原来的第二个参数变为第一个参数
        shift
    )
    rem 判断第一个参数是否为“--build-dir”，该参数通常用于指定构建目录路径
    if "%1"=="--build-dir" (
        rem 将第二个参数（去除文件名后的路径形式）赋值给BUILD_DIR变量，作为构建目录路径
        set BUILD_DIR=%~dpn2
        shift
    )
    rem 判断第一个参数是否为“--toolset”，用于指定使用的工具集相关信息（例如特定的编译工具版本等）
    if "%1"=="--toolset" (
        rem 将第二个参数的值赋给TOOLSET变量，以便后续依据此变量确定具体工具集
        set TOOLSET=%~2
        shift
    )
    rem 判断第一个参数是否为“--version”，用于指定Boost库的版本号
    if "%1"=="--version" (
        rem 将第二个参数的值赋给BOOST_VERSION变量，后续操作会依据此版本号进行下载等相关处理
        set BOOST_VERSION=%~2
        shift
    )
    rem 判断第一个参数是否为“-v”，同样用于指定Boost库的版本号，功能和“--version”类似
    if "%1"=="-v" (
        set BOOST_VERSION=%~2
        shift
    )
    rem 判断第一个参数是否为“-h”，常见的请求显示帮助信息的命令行选项
    if "%1"=="-h" (
        rem 跳转到help代码标签处显示帮助信息
        goto help
    )
    rem 判断第一个参数是否为“--help”，同样是请求帮助的命令行选项
    if "%1"=="--help" (
        goto help
    )
    rem 左移命令行参数列表，准备解析下一轮的参数
    shift
    rem 跳回 :arg-parse 标签处，继续循环解析剩余参数
    goto :arg-parse
)

rem 如果未指定Boost版本号（BOOST_VERSION变量为空），则输出提示信息并跳转到bad_exit标签处结束脚本执行，因为版本号是后续操作必需的参数
if "%BOOST_VERSION%" == "" (
    echo %FILE_N% You must specify a boost version using [-v^|--version]
    goto bad_exit
)

rem 如果未设置BUILD_DIR变量，则将其设置为当前目录
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
rem 如果BUILD_DIR路径末尾不是反斜杠“\”，则添加一个反斜杠，确保路径格式正确
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

rem 如果未定义TOOLSET变量，则默认使用“msvc-14.2”（可能对应一种默认的编译工具集配置情况）
if "%TOOLSET%" == "" set TOOLSET=msvc-14.2

rem 如果未设置NUMBER_OF_ASYNC_JOBS变量，则将其设置为系统的CPU线程数（可能用于控制并行编译等任务的并行度，利用系统已有资源）
if "%NUMBER_OF_ASYNC_JOBS%" == "" set NUMBER_OF_ASYNC_JOBS=%NUMBER_OF_PROCESSORS%

rem ============================================================================
rem -- 定义本地变量部分 --
rem ============================================================================
rem 设置Boost库的基础名称，包含版本号，例如文件名相关的主体部分
set BOOST_BASENAME=boost-%BOOST_VERSION%
rem 设置Boost库文件的预期SHA256校验和值，用于后续验证下载文件的完整性
set BOOST_SHA256SUM="e34756f63abe8ac34b35352743f17d061fcc825969a2dd8458264edb38781782"

rem 设置临时文件夹名称，将版本号中的点替换为下划线（可能是为了符合某些命名规范或避免文件名冲突等）
set BOOST_TEMP_FOLDER=boost_%BOOST_VERSION:.=_%
rem 设置临时文件名称，格式为：临时文件夹名称.zip
set BOOST_TEMP_FILE=%BOOST_TEMP_FOLDER%.zip
rem 设置临时文件的完整路径，包含构建目录（BUILD_DIR）和文件名
set BOOST_TEMP_FILE_DIR=%BUILD_DIR%%BOOST_TEMP_FILE%

rem 设置Boost库的官方仓库下载地址，根据版本号拼接而成
set BOOST_REPO=https://archives.boost.io/release/%BOOST_VERSION%/source/%BOOST_TEMP_FILE%
rem 设置Boost库源代码解压后的目录路径
set BOOST_SRC_DIR=%BUILD_DIR%%BOOST_BASENAME%-source\
rem 设置Boost库安装后的目录路径
set BOOST_INSTALL_DIR=%BUILD_DIR%%BOOST_BASENAME%-install\
rem 设置Boost库安装目录下的库文件子目录路径
set BOOST_LIB_DIR=%BOOST_INSTALL_DIR%lib\

rem ============================================================================
rem -- 获取Boost库相关操作部分 --
rem ============================================================================
rem 如果Boost库已经安装（安装目录已存在），则跳转到already_build标签处处理
if exist "%BOOST_INSTALL_DIR%" (
    goto already_build
)

rem 初始化一个变量，用于后续存储文件校验和的检查结果
set _checksum=""

rem 如果Boost库源代码目录不存在
if not exist "%BOOST_SRC_DIR%" (
    rem 如果临时文件也不存在，则尝试从官方仓库下载Boost库
    if not exist "%BOOST_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving boost.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%BOOST_REPO%', '%BOOST_TEMP_FILE_DIR%')"
        rem 调用自定义函数CheckSumEvaluate来检查下载文件的校验和是否正确，传入临时文件路径、预期校验和值以及用于存储结果的变量名
        call :CheckSumEvaluate %BOOST_TEMP_FILE_DIR%,%BOOST_SHA256SUM%,_checksum
    )
    rem 如果文件校验和检查结果表示校验失败（_checksum值为“1”），则尝试从备用仓库下载
    if "!_checksum!" == "1" (
        echo %FILE_N% Using Boost backup
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('https://carla-releases.s3.us-east-005.backblazeb2.com/Backup/%BOOST_TEMP_FILE%', '%BOOST_TEMP_FILE_DIR%')"
        call :CheckSumEvaluate %BOOST_TEMP_FILE_DIR%,%BOOST_SHA256SUM%,_checksum
    )
    rem 如果从备用仓库下载后校验和仍失败，则跳转到error_download标签处处理错误
    if "!_checksum!" == "1" goto error_download
    echo %FILE_N% Extracting boost from "%BOOST_TEMP_FILE%", this can take a while...
    rem 判断系统中是否存在7-Zip解压工具，如果存在则使用7-Zip进行解压，否则使用PowerShell的Expand-Archive命令解压
    if exist "%ProgramW6432%/7-Zip/7z.exe" (
        "%ProgramW6432%/7-Zip/7z.exe" x "%BOOST_TEMP_FILE_DIR%" -o"%BUILD_DIR%" -y
    ) else (
        powershell -Command "Expand-Archive '%BOOST_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%' -Force"
    )
    echo %FILE_N% Removing "%BOOST_TEMP_FILE%"
    del "%BOOST_TEMP_FILE_DIR%"
    rem 重命名解压后的文件夹为指定的源代码目录名称格式
    rename "%BUILD_DIR%%BOOST_TEMP_FOLDER%" "%BOOST_BASENAME%-source"
) else (
    echo %FILE_N% Not downloading boost because already exists the folder "%BOOST_SRC_DIR%".
)

rem 切换工作目录到Boost库源代码目录
cd "%BOOST_SRC_DIR%"
rem 如果b2.exe文件不存在（可能是构建相关的必要文件），则执行bootstrap.bat来生成构建相关配置等内容
if not exist "b2.exe" (
    echo %FILE_N% Generating build...
    call bootstrap.bat vc141
)
rem 如果执行bootstrap.bat过程出现错误，跳转到error_bootstrap标签处处理
if %errorlevel% neq 0 goto error_bootstrap

rem 以下这部分代码被注释掉了，原作用可能是处理Boost库版本小于1.67时安装头文件的某种问题，如需启用可根据实际情况调整
rem echo %FILE_N% Packing headers...
rem b2 headers link=static

echo %FILE_N% Building...
rem 使用b2命令进行Boost库的构建和安装操作，指定了一系列构建参数，如并行任务数量、包含的模块、架构、工具集、编译变体、链接方式等以及安装相关的目录配置
b2 -j%NUMBER_OF_ASYNC_JOBS%^
    headers^
    --layout=versioned^
    --build-dir=.\build^
    --with-system^
    --with-filesystem^
    --with-python^
    --with-date_time^
    architecture=x86^
    address-model=64^
    toolset=%TOOLSET%^
    variant=release^
    link=static^
    runtime-link=shared^
    threading=multi^
    --prefix="%BOOST_INSTALL_DIR:~0,-1%"^
    --libdir="%BOOST_LIB_DIR:~0,-1%"^
    --includedir="%BOOST_INSTALL_DIR:~0,-1%"^
    install
rem 如果b2执行过程出现错误，跳转到error_install标签处处理
if %errorlevel% neq 0 goto error_install

rem 对Boost库安装目录下以“boost”开头的文件夹重命名为“include”，可能是为了统一头文件目录结构等
for /d %%i in ("%BOOST_INSTALL_DIR%boost*") do rename "%%i" include
rem 构建安装成功后跳转到success标签处
goto success

rem ============================================================================
rem -- 消息提示与错误处理部分 --
rem ============================================================================
rem 帮助信息显示标签，用于输出脚本的使用说明和参数介绍等帮助内容
:help
    echo %FILE_N% Download and install a boost version.
    echo "Usage: %FILE_N% [-h^|--help] [-v^|--version] [--toolset] [--build-dir] [-j]"
    goto eof

rem 安装成功后的处理标签，输出成功信息并进行一些后续收尾操作
:success
    echo.
    echo %FILE_N% Boost has been successfully installed in "%BOOST_INSTALL_DIR%"!
    goto good_exit

rem 如果Boost库已经构建过的处理标签，提示已存在安装且告知如何强制重新构建
:already_build
    echo %FILE_N% A Boost installation already exists.
    echo %FILE_N% Delete "%BOOST_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

rem 下载Boost库出现错误时的处理标签，输出错误信息以及可能的原因和解决办法
:error_download
    echo.
    echo %FILE_N% [GIT ERROR] An error ocurred while downloading boost.
    echo %FILE_N% [GIT ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%BOOST_REPO%"
    echo %FILE_N% [GIT ERROR] Workaround:
    echo %FILE_N%              - Download the source code of boost "%BOOST_VERSION%" and
    echo %FILE_N%                extract the content of "%BOOST_TEMP_FOLDER%" in
    echo %FILE_N%                "%BOOST_SRC_DIR%"
    goto bad_exit

rem 执行bootstrap.bat出现错误时的处理标签，输出错误信息
:error_bootstrap
    echo.
    echo %FILE_N% [BOOTSTRAP ERROR] An error ocurred while executing "bootstrap.bat".
    goto bad_exit

rem 使用b2.exe进行安装出现错误时的处理标签，输出错误信息
:error_install
    echo.
    echo %FILE_N% [B2 ERROR] An error ocurred while installing using "b2.exe".
    goto bad_exit

rem 正常退出处理标签，输出退出信息，进行一些必要的变量设置等收尾操作，并以0状态码退出脚本，表示执行成功
:good_exit
    echo %FILE_N% Exiting...
    endlocal
    rem 设置一个用于后续检查安装情况的变量install_boost并赋值，表示安装完成
    set install_boost=done
    exit /b 0

rem 错误退出处理标签，删除已存在的安装目录（如果有），输出错误退出信息，结束局部变量设置，并以错误状态码退出脚本
:bad_exit
    if exist "%BOOST_INSTALL_DIR%" rd /s /q "%BOOST_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%

rem 自定义函数，用于检查文件的SHA256校验和是否与预期值匹配
:CheckSumEvaluate
set filepath=%1
set checksum=%2

echo %FILE_N% calculating %filepath% checksum...

set PsCommand="(Get-FileHash %filepath%).Hash -eq '%checksum%'"

for /f %%F in ('Powershell -C %PsCommand%') do (
    set filechecksum=%%F
)

if %filechecksum% == True (
    echo %FILE_N% %filepath% checksum OK
    set "%~3=0"
    exit /b 0
) else (
    echo %FILE_N% %filepath% BAD SHA256 checksum
    set "%~3=1"
    exit /b 1
)
