REM @echo off
setlocal

rem 这是一个批处理脚本（BAT script），用于下载并安装适用于CARLA（carla.org）的x64版本的SQLite3。
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
    shift
    goto :arg-parse
)

rem 如果BUILD_DIR变量未被设置，将其设置为当前目录，并且确保目录路径最后有反斜杠（\）
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

rem ============================================================================
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

rem 设置SQLite的基础名称，这里包含了版本号等信息
set SQLITE_BASE_NAME=sqlite-amalgamation-3340100
rem 设置SQLite压缩包的文件名，基于基础名称加上.zip后缀
set SQLITE_ZIP=%SQLITE_BASE_NAME%.zip
rem 设置SQLite压缩包的完整路径，基于构建目录和压缩包文件名组合
set SQLITE_ZIP_DIR=%BUILD_DIR%%SQLITE_ZIP%
rem 设置SQLite源码的下载地址，根据基础名称构建出对应的网址
set SQLITE_REPO=https://www.sqlite.org/2021/%SQLITE_ZIP%

rem 设置SQLite源码解压后的目录路径
set SQLITE_SRC_DIR=%BUILD_DIR%sqlite3-src
rem 设置SQLite最终的安装目录路径
set SQLITE_INSTALL_DIR=%BUILD_DIR%sqlite3-install

rem 设置SQLite安装目录下的头文件目录路径
set SQLITE_INCLUDE_DIR=%SQLITE_INSTALL_DIR%\include
rem 设置SQLite安装目录下的可执行文件目录路径
set SQLITE_BIN_DIR=%SQLITE_INSTALL_DIR%\bin
rem 设置SQLite安装目录下的库文件目录路径
set SQLITE_LIB_DIR=%SQLITE_INSTALL_DIR%\lib

rem 如果SQLite已经安装（安装目录存在），则跳转到already_build标签处
if exist "%SQLITE_INSTALL_DIR%" (
    goto already_build
)

rem 输出提示信息，表示正在下载SQLite
echo %FILE_N% Retrieving %SQLITE_BASE_NAME%.
rem 使用PowerShell命令下载SQLite的压缩包到指定的路径（SQLITE_ZIP_DIR）
powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%SQLITE_REPO%', '%SQLITE_ZIP_DIR%')"
rem 如果下载过程出现错误（errorlevel非0），跳转到error_download标签处处理下载错误情况
if %errorlevel% neq 0 goto error_download

rem 输出提示信息，表示正在解压SQLite压缩包
echo %FILE_N% Extracting Sqlite3 from "%SQLITE_ZIP%".
rem 使用PowerShell命令解压下载的压缩包到构建目录下
powershell -Command "Expand-Archive '%SQLITE_ZIP_DIR%' -DestinationPath '%BUILD_DIR%'"
rem 将解压后的文件夹重命名并移动到指定的源码目录（SQLITE_SRC_DIR）
move %BUILD_DIR%%SQLITE_BASE_NAME% %SQLITE_SRC_DIR%

rem 切换到SQLite源码目录，后续的编译等操作在此目录下进行
cd %SQLITE_SRC_DIR%

rem 使用cl编译器编译sqlite3.c和shell.c文件，生成可执行文件sqlite.exe，设置编译选项（/MD表示使用多线程DLL运行时库，/MP表示启用多处理器编译）
call cl /MD /MP sqlite3.c shell.c -Fesqlite.exe
rem 使用cl编译器编译sqlite3.c文件，生成目标文件sqlite3.obj，设置编译选项（/MD、/MP、/c表示只编译不链接，/EHsc表示启用C++异常处理）
call cl /MD /MP /c /EHsc sqlite3.c
rem 使用lib工具将sqlite3.obj文件打包成静态库sqlite3.lib
call lib sqlite3.obj

rem 创建SQLite安装目录下的头文件目录
mkdir %SQLITE_INCLUDE_DIR%
rem 创建SQLite安装目录下的可执行文件目录
mkdir %SQLITE_BIN_DIR%
rem 创建SQLite安装目录下的库文件目录
mkdir %SQLITE_LIB_DIR%

rem 将生成的可执行文件sqlite.exe移动到可执行文件目录下
move sqlite.exe %SQLITE_BIN_DIR%
rem 将头文件sqlite3.h移动到头文件目录下
move sqlite3.h %SQLITE_INCLUDE_DIR%
rem 将静态库文件sqlite3.lib移动到库文件目录下
move sqlite3.lib %SQLITE_LIB_DIR%

rem 删除下载的SQLite压缩包文件
del %SQLITE_ZIP_DIR%

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    echo %FILE_N% Download and install a Sqlite3.
    echo "Usage: %FILE_N% [-h^|--help] [--build-dir] [--zlib-install-dir]"
    goto eof

:success
    echo.
    echo %FILE_N% Sqlite3 has been successfully installed in "%SQLITE_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A Sqlite3 installation already exists.
    echo %FILE_N% Delete "%SQLITE_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_download
    echo.
    echo %FILE_N% [DOWNLOAD ERROR] An error ocurred while downloading Sqlite3.
    echo %FILE_N% [DOWNLOAD ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%SQLITE_REPO%"
    echo %FILE_N% [DOWNLOAD ERROR] Workaround:
    echo %FILE_N%              - Download the Sqlite3's source code and
    echo %FILE_N%                extract the content in
    echo %FILE_N%                "%SQLITE_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_extracting
    echo.
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the Sqlite3's source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%SQLITE_SRC_DIR%"
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
    rem A return值用于检查错误情况，将安装目录路径设置到环境变量install_Sqlite3中并返回0表示成功退出
    endlocal & set install_Sqlite3=%SQLITE_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%SQLITE_INSTALL_DIR%" rd /s /q "%SQLITE_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
