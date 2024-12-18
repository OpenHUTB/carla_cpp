@echo off
setlocal enabledelayedexpansion

rem BAT脚本用于下载并安装CARLA（carla.org）可用的Boost构建版本。
rem 以下是对整个脚本功能的简要描述，后续代码按不同功能模块划分并详细注释。

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem 打印批处理参数（用于调试目的）
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

:arg-parse
if not "%1"=="" (
    rem 判断参数是否为 -j，若为 -j，则将其后的参数作为并行作业数量赋值给NUMBER_OF_ASYNC_JOBS变量
    if "%1"=="-j" (
        set NUMBER_OF_ASYNC_JOBS=%~2
        shift
    )
    rem 判断参数是否为 --build-dir，若为该参数，则将其后的参数作为构建目录赋值给BUILD_DIR变量
    if "%1"=="--build-dir" (
        set BUILD_DIR=%~dpn2
        shift
    )
    rem 判断参数是否为 --toolset，若为该参数，则将其后的参数作为工具集赋值给TOOLSET变量
    if "%1"=="--toolset" (
        set TOOLSET=%~2
        shift
    )
    rem 判断参数是否为 --version或 -v，若为这两个参数之一，则将其后的参数作为Boost版本号赋值给BOOST_VERSION变量
    if "%1"=="--version" (
        set BOOST_VERSION=%~2
        shift
    )
    if "%1"=="-v" (
        set BOOST_VERSION=%~2
        shift
    )
    rem 判断参数是否为 -h或 --help，若为这两个参数之一，则跳转到帮助信息显示部分（help标签处）
    if "%1"=="-h" (
        goto help
    )
    if "%1"=="--help" (
        goto help
    )
    shift
    goto :arg-parse
)

rem 如果未指定Boost版本号，则输出错误提示信息并跳转到bad_exit标签处结束脚本执行
if "%BOOST_VERSION%" == "" (
    echo %FILE_N% You must specify a boost version using [-v^|--version]
    goto bad_exit
)

rem 如果未设置构建目录，则将其设置为当前目录（即批处理脚本所在目录），并确保目录路径以反斜杠结尾
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

rem 如果未定义工具集，则默认使用Visual Studio 2019作为工具集（对应msvc-14.2）
if "%TOOLSET%" == "" set TOOLSET=msvc-14.2

rem 如果未设置并行作业数量，则将其设置为CPU线程数量（通过%NUMBER_OF_PROCESSORS%获取）
if "%NUMBER_OF_ASYNC_JOBS%" == "" set NUMBER_OF_ASYNC_JOBS=%NUMBER_OF_PROCESSORS%

rem ============================================================================
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

rem 设置Boost的基础文件名，基于指定的BOOST_VERSION变量构建，用于后续文件路径等相关操作
set BOOST_BASENAME=boost-%BOOST_VERSION%
rem 设置Boost文件的预期SHA256校验和值，用于验证下载文件的完整性
set BOOST_SHA256SUM="e34756f63abe8ac34b35352743f17d061fcc825969a2dd8458264edb38781782"

rem 设置临时文件夹名称，将版本号中的点替换为下划线，用于存放下载的临时文件等
set BOOST_TEMP_FOLDER=boost_%BOOST_VERSION:.=_%
rem 设置临时文件名称，基于临时文件夹名称加上.zip后缀，即下载的Boost压缩包文件名
set BOOST_TEMP_FILE=%BOOST_TEMP_FOLDER%.zip
rem 设置临时文件的完整路径，结合构建目录和临时文件名称
set BOOST_TEMP_FILE_DIR=%BUILD_DIR%%BOOST_TEMP_FILE%

rem 设置Boost的下载仓库地址，根据指定的版本号从Boost官方仓库获取对应版本的源文件压缩包
set BOOST_REPO=https://archives.boost.io/release/%BOOST_VERSION%/source/%BOOST_TEMP_FILE%
rem 设置解压后的Boost源文件目录路径，用于后续构建操作
set BOOST_SRC_DIR=%BUILD_DIR%%BOOST_BASENAME%-source\
rem 设置Boost安装目录路径，构建完成后将文件安装到此目录
set BOOST_INSTALL_DIR=%BUILD_DIR%%BOOST_BASENAME%-install\
rem 设置Boost库文件目录路径，是安装目录下的lib子目录，用于存放编译生成的库文件
set BOOST_LIB_DIR=%BOOST_INSTALL_DIR%lib\

rem ============================================================================
rem -- Get Boost ---------------------------------------------------------------
rem ============================================================================

rem 如果Boost安装目录已存在，则直接跳转到already_build标签处，提示已存在安装并给出相关操作建议
if exist "%BOOST_INSTALL_DIR%" (
    goto already_build
)

set _checksum=""

rem 如果Boost源文件目录不存在，且临时文件也不存在，则执行以下下载操作
if not exist "%BOOST_SRC_DIR%" (
    if not exist "%BOOST_TEMP_FILE_DIR%" (
        rem 输出提示信息，表示正在下载Boost文件
        echo %FILE_N% Retrieving boost.
        rem 使用PowerShell命令下载Boost文件到指定的临时文件路径
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%BOOST_REPO%', '%BOOST_TEMP_FILE_DIR%')"
        rem 调用自定义函数CheckSumEvaluate来验证下载文件的SHA256校验和是否正确，将结果保存在_checksum变量中
        call :CheckSumEvaluate %BOOST_TEMP_FILE_DIR%,%BOOST_SHA256SUM%,_checksum
    )
    rem 如果校验和验证失败（_checksum为1），尝试从备用地址下载Boost备份文件，并再次验证校验和
    if "!_checksum!" == "1" (
        echo %FILE_N% Using Boost backup
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('https://carla-releases.s3.us-east-005.backblazeb2.com/Backup/%BOOST_TEMP_FILE%', '%BOOST_TEMP_FILE_DIR%')"
        call :CheckSumEvaluate %BOOST_TEMP_FILE_DIR%,%BOOST_SHA256SUM%,_checksum
    )
    rem 如果经过多次尝试下载后校验和仍不正确，则跳转到error_download标签处处理下载错误情况
    if "!_checksum!" == "1" goto error_download
    rem 输出提示信息，表示正在从临时文件中解压Boost文件，此过程可能耗时较长
    echo %FILE_N% Extracting boost from "%BOOST_TEMP_FILE%", this can take a while...
    rem 判断系统中是否存在7-Zip解压工具，如果存在则使用7-Zip进行解压，否则使用PowerShell的Expand-Archive命令解压
    if exist "%ProgramW6432%/7-Zip/7z.exe" (
        "%ProgramW6432%/7-Zip/7z.exe" x "%BOOST_TEMP_FILE_DIR%" -o"%BUILD_DIR%" -y
    ) else (
        powershell -Command "Expand-Archive '%BOOST_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%' -Force"
    )
    rem 输出提示信息，表示正在删除临时文件（下载的Boost压缩包）
    echo %FILE_N% Removing "%BOOST_TEMP_FILE%"
    del "%BOOST_TEMP_FILE_DIR%"
    rem 将解压后的临时文件夹重命名为正式的Boost源文件目录名称（带有-source后缀）
    rename "%BUILD_DIR%%BOOST_TEMP_FOLDER%" "%BOOST_BASENAME%-source"
) else (
    rem 如果Boost源文件目录已存在，则输出提示信息，表示不再下载Boost
    echo %FILE_N% Not downloading boost because already exists the folder "%BOOST_SRC_DIR%".
)

rem 进入Boost源文件目录，准备后续的构建操作
cd "%BOOST_SRC_DIR%"
rem 如果源文件目录下不存在b2.exe（Boost构建工具），则执行bootstrap.bat生成构建环境，这里指定使用vc141版本
if not exist "b2.exe" (
    echo %FILE_N% Generating build...
    call bootstrap.bat vc141
)

rem 如果执行bootstrap.bat过程中出现错误（errorlevel非0），则跳转到error_bootstrap标签处处理构建初始化错误情况
if %errorlevel% neq 0 goto error_bootstrap

rem 以下这行代码被注释掉了，原本的功能是用于处理Boost版本小于1.67时安装头文件可能出现的问题，通过b2命令打包头文件
rem echo %FILE_N% Packing headers...
rem b2 headers link=static

rem 输出提示信息，表示开始构建Boost库
echo %FILE_N% Building...
rem 使用b2命令构建Boost库，指定了一系列构建参数，如并行作业数量、包含的模块、构建布局、工具集、编译选项等
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
rem 如果执行b2构建安装过程中出现错误（errorlevel非0），则跳转到error_install标签处处理构建安装错误情况
if %errorlevel% neq 0 goto error_install

rem 遍历Boost安装目录下以boost开头的子目录（通常是安装后的头文件目录），将其重命名为include，以符合常见的头文件存放规范
for /d %%i in ("%BOOST_INSTALL_DIR%boost*") do rename "%%i" include
goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    rem 显示帮助信息，说明脚本的基本功能以及正确的使用参数格式
    echo %FILE_N% Download and install a boost version.
    echo "Usage: %FILE_N% [-h^|--help] [-v^|--version] [--toolset] [--build-dir] [-j]"
    goto eof

:success
    echo.
    rem 输出成功信息，提示Boost已成功安装到指定的安装目录
    echo %FILE_N% Boost has been successfully installed in "%BOOST_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A Boost installation already exists.
    echo %FILE_N% Delete "%BOOST_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_download
    echo.
    rem 输出下载错误相关的提示信息，包括错误提示、可能原因以及解决办法（建议手动下载并解压）
    echo %FILE_N% [GIT ERROR] An error ocurred while downloading boost.
    echo %FILE_N% [GIT ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%BOOST_REPO%"
    echo %FILE_N% [GIT ERROR] Workaround:
    echo %FILE_N%              - Download the source code of boost "%BOOST_VERSION%" and
    echo %FILE_N%                extract the content of "%BOOST_TEMP_FOLDER%" in
    echo %FILE_N%                "%BOOST_SRC_DIR%"
    goto bad_exit

:error_bootstrap
    echo.
    rem 输出构建初始化（执行bootstrap.bat）出现错误的提示信息
    echo %FILE_N% [BOOTSTRAP ERROR] An error ocurred while executing "bootstrap.bat".
    goto bad_exit

:error_install
    echo.
    rem 输出使用b2.exe进行构建安装过程中出现错误的提示信息
    echo %FILE_N% [B2 ERROR] An error ocurred while installing using "b2.exe".
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    endlocal
    rem 设置一个用于检查是否成功安装的返回值变量install_boost为done
    set install_boost=done
    exit /b 0

:bad_exit
    rem 如果安装目录存在，则删除整个安装目录（包括其下所有文件和子目录），用于清理可能存在的错误安装文件
    if exist "%BOOST_INSTALL_DIR%" rd /s /q "%BOOST_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%

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
