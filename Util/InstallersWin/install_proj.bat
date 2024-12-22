REM @echo off
setlocal

rem 这是一个批处理脚本（BAT script），用于下载并安装适用于CARLA（carla.org）的x64版本的PROJ库。
rem 需要在启用了x64 Visual C++工具集的命令提示符（cmd）中运行该脚本。

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem 打印批处理参数（用于调试目的），%*表示所有传入的命令行参数
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

:arg-parse
if not "%1"=="" (
    if "%1"=="--build-dir" (
        rem 如果命令行参数是--build-dir，将下一个参数（%2）作为构建目录路径赋值给BUILD_DIR变量，并将参数指针向后移动一位（shift）
        set BUILD_DIR=%~dpn2
        shift
    )
    if "%1"=="-h" (
        rem 如果参数是 -h，跳转到帮助信息显示部分（help标签处）
        goto help
    )
    if "%1"=="--help" (
        rem 如果参数是 --help，同样跳转到帮助信息显示部分（help标签处）
        goto help
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

rem ============================================================================
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

rem 设置PROJ库的基础名称，包含版本号信息
set PROJ_BASE_NAME=proj-7.2.1
rem 设置PROJ库的压缩包文件名，基于基础名称加上.tar.gz后缀
set PROJ_ZIP=%PROJ_BASE_NAME%.tar.gz
rem 设置PROJ库压缩包的完整路径，基于构建目录和压缩包文件名组合
set PROJ_ZIP_DIR=%BUILD_DIR%%PROJ_ZIP%
rem 设置PROJ库源码的下载地址，根据基础名称构建出对应的网址
set PROJ_REPO=https://download.osgeo.org/proj/%PROJ_ZIP%

rem 设置SQLite库的安装目录路径（可能PROJ库构建会依赖SQLite库，此处先定义相关路径方便后续配置使用）
set SQLITE_INSTALL_DIR=%BUILD_DIR%sqlite3-install
rem 设置SQLite库安装目录下的头文件目录路径
set SQLITE_INCLUDE_DIR=%SQLITE_INSTALL_DIR%\include
rem 设置SQLite库可执行文件的完整路径
set SQLITE_BIN=%SQLITE_INSTALL_DIR%\bin\sqlite.exe
rem 设置SQLite库的库文件完整路径
set SQLITE_LIB=%SQLITE_INSTALL_DIR%\lib\sqlite3.lib

rem 设置PROJ库源码解压后的目录路径
set PROJ_SRC_DIR=%BUILD_DIR%proj-src
rem 设置PROJ库构建目录路径，位于源码目录下
set PROJ_BUILD_DIR=%PROJ_SRC_DIR%\build
rem 设置PROJ库最终的安装目录路径
set PROJ_INSTALL_DIR=%BUILD_DIR%proj-install

rem 设置PROJ库安装目录下的头文件目录路径
set PROJ_INCLUDE_DIR=%PROJ_INSTALL_DIR%\include
rem 设置PROJ库安装目录下的可执行文件目录路径
set PROJ_BIN_DIR=%PROJ_INSTALL_DIR%\bin
rem 设置PROJ库安装目录下的库文件目录路径
set PROJ_LIB_DIR=%PROJ_INSTALL_DIR%\lib

rem 如果PROJ已经安装（安装目录存在），则跳转到already_build标签处
if exist "%PROJ_INSTALL_DIR%" (
    goto already_build
)

rem 输出提示信息，表示正在下载PROJ库
echo %FILE_N% Retrieving %PROJ_BASE_NAME%.
rem 使用PowerShell命令下载PROJ库的压缩包到指定的路径（PROJ_ZIP_DIR）
powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%PROJ_REPO%', '%PROJ_ZIP_DIR%')"
rem 如果下载过程出现错误（errorlevel非0），跳转到error_download标签处处理下载错误情况
if %errorlevel% neq 0 goto error_download

rem 切换到构建目录下（此处先切换到构建目录所在的上级目录，即BUILD_DIR）
cd %BUILD_DIR%
rem 输出提示信息，表示正在解压PROJ压缩包
echo %FILE_N% Extracting PROJ from "%PROJ_ZIP%".
rem 使用tar命令解压下载的压缩包（假设系统中已配置好能识别tar命令来解压.tar.gz文件）
call tar -xzf %PROJ_ZIP%
rem 将解压后的文件夹重命名并移动到指定的源码目录（PROJ_SRC_DIR）
move %BUILD_DIR%%PROJ_BASE_NAME% %PROJ_SRC_DIR%

rem 创建PROJ库的构建目录
mkdir %PROJ_BUILD_DIR%
rem 切换到PROJ库的构建目录，后续的配置、编译等操作在此目录下进行
cd %PROJ_BUILD_DIR%

rem 判断生成器名称中是否包含"Visual Studio"，如果包含则设置PLATFORM为-A x64，用于指定64位平台构建，否则PLATFORM为空
echo.%GENERATOR% | findstr /C:"Visual Studio" >nul && (
    set PLATFORM=-A x64
) || (
    set PLATFORM=
)

rem 使用cmake配置构建项目，指定生成器、平台、C++编译Release模式的一些标志、SQLite相关的头文件和库文件路径、可执行文件路径，
rem 以及关闭一些PROJ库相关的功能构建选项（如TIFF、CURL支持、共享库构建、各种工具构建、测试构建等），同时指定构建类型为Release和安装前缀等参数
cmake.. -G %GENERATOR% %PLATFORM%^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DCMAKE_CXX_FLAGS="/MD /MP"^
    -DSQLITE3_INCLUDE_DIR=%SQLITE_INCLUDE_DIR% -DSQLITE3_LIBRARY=%SQLITE_LIB%^
    -DEXE_SQLITE3=%SQLITE_BIN%^
    -DENABLE_TIFF=OFF -DENABLE_CURL=OFF -DBUILD_SHARED_LIBS=OFF -DBUILD_PROJSYNC=OFF^
    -DCMAKE_BUILD_TYPE=Release -DBUILD_PROJINFO=OFF^
    -DBUILD_CCT=OFF -DBUILD_CS2CS=OFF -DBUILD_GEOD=OFF -DBUILD_GIE=OFF^
    -DBUILD_PROJ=OFF -DBUILD_TESTING=OFF^
    -DCMAKE_INSTALL_PREFIX=%PROJ_INSTALL_DIR%
rem 如果cmake配置过程出现错误（errorlevel非0），跳转到error_cmake标签处处理配置错误情况
if %errorlevel% neq 0 goto error_cmake

rem 使用cmake进行构建并安装，指定构建配置为Release并执行安装目标
cmake --build. --config Release --target install 

rem 删除下载的PROJ库压缩包文件，节省磁盘空间，因为已经解压并完成构建安装，不再需要压缩包了
del %PROJ_ZIP_DIR%

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    echo %FILE_N% Download and install a PROJ.
    echo "Usage: %FILE_N% [-h^|--help] [--build-dir] [--zlib-install-dir]"
    goto eof

:success
    echo.
    echo %FILE_N% PROJ has been successfully installed in "%PROJ_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A PROJ installation already exists.
    echo %FILE_N% Delete "%PROJ_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_download
    echo.
    echo %FILE_N% [DOWNLOAD ERROR] An error ocurred while downloading PROJ.
    echo %FILE_N% [DOWNLOAD ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%PROJ_REPO%"
    echo %FILE_N% [DOWNLOAD ERROR] Workaround:
    echo %FILE_N%              - Download the PROJ's source code and
    echo %FILE_N%                extract the content in
    echo %FILE_N%                "%PROJ_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_extracting
    echo.
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the PROJ's source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%PROJ_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_compiling
    echo.
    echo %FILE_N% [COMPILING ERROR] An error ocurred while compiling with cl.exe.
    echo %FILE_N%              Possible causes:
    echo %FILE_N%              - Make sure you have Visual Studio installed.
    echo %FILE_N%              - Make sure you have the "x64 Visual C++ Toolset" in your path.
    echo %FILE_N%                For example, using the "Visual Studio x64 Native Tools Command Prompt",
    echo %FILE_N%                or the "vcvarsall.bat".
    goto bad_exit

:error_generating_lib
    echo.
    echo %FILE_N% [NMAKE ERROR] An error ocurred while compiling and installing using nmake.
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    rem A return值用于检查错误情况，将安装目录路径设置到环境变量install_proj中并返回0表示成功退出
    endlocal & set install_proj=%PROJ_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%PROJ_INSTALL_DIR%" rd /s /q "%PROJ_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
