REM @echo off
setlocal

rem BAT脚本用于下载并安装适用于CARLA（carla.org）的x64版本的xerces-c构建版本。
rem 需要在启用了x64 Visual C++工具集的命令提示符（cmd）中运行该脚本。
set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem 打印批处理参数（用于调试目的）
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

:arg-parse
if not "%1"=="" (
    if "%1"=="--build-dir" (
        rem 如果参数是--build-dir，将下一个参数（%2）作为构建目录路径赋值给BUILD_DIR变量，并将参数指针向后移动一位（shift）
        set BUILD_DIR=%~dpn2
        shift
    )
    if "%1"=="-h" (
        rem 如果参数是-h，跳转到帮助信息显示部分（help标签处）
        goto help
    )
    if "%1"=="--help" (
        rem 如果参数是--help，同样跳转到帮助信息显示部分（help标签处）
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

rem 如果未设置构建目录，将当前目录设置为构建目录，确保目录路径最后有反斜杠（\）
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

rem ============================================================================
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

rem 设置xerces-c的基础名称
set XERCESC_BASENAME=xerces-c
rem 设置xerces-c的版本号
set XERCESC_VERSION=3.2.3

rem xerces-c-x.x.x，用于临时文件夹名称，例如xerces-c-3.2.3，存储下载解压前的临时文件等
set XERCESC_TEMP_FOLDER=%XERCESC_BASENAME%-%XERCESC_VERSION%
rem../xerces-c-x.x.x，完整的临时文件夹路径，基于构建目录和临时文件夹名称组合
set XERCESC_TEMP_FOLDER_DIR=%BUILD_DIR%%XERCESC_TEMP_FOLDER%
rem xerces-c-x.x.x-src.zip，临时文件名称，即要下载的xerces-c源码压缩包名称，例如xerces-c-3.2.3-src.zip
set XERCESC_TEMP_FILE=%XERCESC_TEMP_FOLDER%-src.zip
rem../xerces-c-x.x.x-src.zip，完整的临时文件路径，基于构建目录和临时文件名称组合
set XERCESC_TEMP_FILE_DIR=%BUILD_DIR%%XERCESC_TEMP_FILE%

rem 设置xerces-c源码的官方下载地址
set XERCESC_REPO=https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-%XERCESC_VERSION%.zip
rem 设置xerces-c源码的备用下载地址
set XERCESC_BACKUP_REPO=https://carla-releases.s3.us-east-005.backblazeb2.com/Backup/xerces-c-%XERCESC_VERSION%.zip

rem../xerces-c-x.x.x-source/，解压后的xerces-c源码所在目录路径
set XERCESC_SRC_DIR=%BUILD_DIR%%XERCESC_BASENAME%-%XERCESC_VERSION%-source\
rem../xerces-c-x.x.x-install/，安装后的xerces-c文件存放目录路径（包含头文件、库文件等最终安装结果）
set XERCESC_INSTALL_DIR=%BUILD_DIR%%XERCESC_BASENAME%-%XERCESC_VERSION%-install\

rem ============================================================================
rem -- Get xerces --------------------------------------------------------------
rem ============================================================================

rem 如果xerces-c已经安装（安装目录存在），则跳转到already_build标签处
if exist "%XERCESC_INSTALL_DIR%" (
    goto already_build
)

rem 如果xerces-c源码目录不存在
if not exist "%XERCESC_SRC_DIR%" (
    rem 如果临时文件（下载的压缩包）也不存在
    if not exist "%XERCESC_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %XERCESC_BASENAME%.
        rem 使用PowerShell命令下载xerces-c源码压缩包到指定的临时文件路径，从官方地址下载
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%XERCESC_REPO%', '%XERCESC_TEMP_FILE_DIR%')"
    )
    rem 如果从官方地址下载失败（临时文件仍不存在），尝试从备用地址下载
    if not exist "%XERCESC_TEMP_FILE_DIR%" (
        echo %FILE_N% Using %XERCESC_BASENAME% from backup.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%XERCESC_BACKUP_REPO%', '%XERCESC_TEMP_FILE_DIR%')"
    )
    rem 如果下载过程出现错误（errorlevel非0），跳转到error_download标签处处理下载错误情况
    if %errorlevel% neq 0 goto error_download
    rem 下载成功后，提取下载的压缩包到构建目录
    echo %FILE_N% Extracting xerces from "%XERCESC_TEMP_FILE%".
    powershell -Command "Expand-Archive '%XERCESC_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%'"
    rem 如果解压过程出现错误（errorlevel非0），跳转到error_extracting标签处处理解压错误情况
    if %errorlevel% neq 0 goto error_extracting

    rem 删除下载的临时压缩包文件
    echo %FILE_N% Removing "%XERCESC_TEMP_FILE%"
    del "%XERCESC_TEMP_FILE_DIR%"
    rem 删除构建目录下的manifest文件夹（可能是解压后不需要的文件或文件夹）
    echo %FILE_N% Removing dir "%BUILD_DIR%manifest"
    rmdir /s/q "%BUILD_DIR%manifest"

    echo %FILE_N% Renaming dir %XERCESC_TEMP_FOLDER_DIR% to %XERCESC_BASENAME%-%XERCESC_VERSION%-source
    rem 将临时文件夹重命名为指定的源码目录名称，使其符合后续处理要求
    rename "%XERCESC_TEMP_FOLDER_DIR%" "%XERCESC_BASENAME%-%XERCESC_VERSION%-source"
) else (
    echo %FILE_N% Not downloading xerces because already exists the folder "%XERCESC_SRC_DIR%".
)

rem ============================================================================
rem -- Compile xerces ----------------------------------------------------------
rem ============================================================================

rem 如果源码目录下的build文件夹不存在，创建该文件夹，用于后续构建过程
if not exist "%XERCESC_SRC_DIR%build" (
    echo %FILE_N% Creating "%XERCESC_SRC_DIR%build"
    mkdir "%XERCESC_SRC_DIR%build"
)

rem 切换到源码目录下的build文件夹，后续的构建操作在此目录下进行
cd "%XERCESC_SRC_DIR%build"

rem 如果安装目录下的lib文件夹不存在，创建该文件夹，用于存放编译生成的库文件
if not exist "%XERCESC_INSTALL_DIR%lib" (
    echo %FILE_N% Creating "%XERCESC_INSTALL_DIR%lib"
    mkdir "%XERCESC_INSTALL_DIR%lib"
)

rem 如果安装目录下的include文件夹不存在，创建该文件夹，用于存放编译所需的头文件等
if not exist "%XERCESC_INSTALL_DIR%include" (
    echo %FILE_N% Creating "%XERCESC_INSTALL_DIR%include"
    mkdir "%XERCESC_INSTALL_DIR%include"
)

rem 判断生成器名称中是否包含"Visual Studio"，如果包含则设置PLATFORM为-A x64，用于指定64位平台构建，否则PLATFORM为空
echo.%GENERATOR% | findstr /C:"Visual Studio" >nul && (
    set PLATFORM=-A x64
) || (
    set PLATFORM=
)

rem 使用cmake配置构建项目，指定生成器、平台、安装前缀、不构建共享库以及源码目录等参数
cmake.. -G %GENERATOR% %PLATFORM%^
  -DCMAKE_INSTALL_PREFIX="%XERCESC_INSTALL_DIR:\=/%"^
  -DBUILD_SHARED_LIBS=OFF^
  "%BUILD_DIR%%XERCESC_BASENAME%-%XERCESC_VERSION%-source"
rem 如果cmake配置过程出现错误（errorlevel非0），跳转到error_cmake标签处处理配置错误情况
if %errorlevel% neq 0 goto error_cmake

rem 使用cmake进行构建并安装，指定构建配置为Release并执行安装目标
cmake --build. --config Release --target install

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    echo %FILE_N% Download and install a xerces.
    echo "Usage: %FILE_N% [-h^|--help] [--build-dir] [--zlib-install-dir]"
    goto eof

:success
    echo.
    echo %FILE_N% xerces has been successfully installed in "%XERCESC_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A xerces installation already exists.
    echo %FILE_N% Delete "%XERCESC_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

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

:error_extracting
    echo.
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the xerces's source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%XERCESC_SRC_DIR%"
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
    rem A return值用于检查错误情况，将安装目录路径设置到环境变量install_xerces中并返回0表示成功退出
    endlocal & set install_xerces=%XERCESC_INSTALL_DIR%
    exit /b 0

:bad_exit
    rem 如果安装目录存在，删除该安装目录（可能是构建安装出现错误，清理残留）
    if exist "%XERCESC_INSTALL_DIR%" rd /s /q "%XERCESC_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
