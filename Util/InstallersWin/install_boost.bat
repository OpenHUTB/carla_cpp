@echo off
setlocal enabledelayedexpansion

rem BAT脚本用于下载并安装CARLA（carla.org）可用的boost构建版本
rem 以下部分添加了其他编译依赖文件（以mylib为例）相关的操作逻辑

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem 打印批处理脚本接收到的命令行参数（用于调试目的）
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

:arg-parse
if not "%1"=="" (
    if "%1"=="-j" (
        set NUMBER_OF_ASYNC_JOBS=%~2
        shift
    )
    if "%1"=="--build-dir" (
        set BUILD_DIR=%~dpn2
        shift
    )
    if "%1"=="--toolset" (
        set TOOLSET=%~2
        shift
    )
    if "%1"=="--version" (
        set BOOST_VERSION=%~2
        shift
    )
    if "%1"=="-v" (
        set BOOST_VERSION=%~2
        shift
    )
    if "%1"=="-h" (
        goto help
    )
    if "%1"=="--help" (
        goto help
    )
    shift
    goto :arg-parse
)

if "%BOOST_VERSION%" == "" (
    echo %FILE_N% You must specify a boost version using [-v|--version]
    goto bad_exit
)

rem 如果未设置构建目录，则设为当前目录
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

rem 如果未定义，使用Visual Studio 2019作为工具集
if "%TOOLSET%" == "" set TOOLSET=msvc-14.2

rem 如果未设置，将并行任务数量设置为CPU线程数
if "%NUMBER_OF_ASYNC_JOBS%" == "" set NUMBER_OF_ASYNC_JOBS=%NUMBER_OF_PROCESSORS%

rem ============================================================================
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

set BOOST_BASENAME=boost-%BOOST_VERSION%
set BOOST_SHA256SUM="e34756f63abe8ac34b35352743f17d061fcc825969a2dd8458264edb38781782"

set BOOST_TEMP_FOLDER=boost_%BOOST_VERSION:.=_%
set BOOST_TEMP_FILE=%BOOST_TEMP_FOLDER%.zip
set BOOST_TEMP_FILE_DIR=%BUILD_DIR%%BOOST_TEMP_FILE%

set BOOST_REPO=https://archives.boost.io/release/%BOOST_VERSION%/source/%BOOST_TEMP_FILE%
set BOOST_SRC_DIR=%BUILD_DIR%%BOOST_BASENAME%-source\
set BOOST_INSTALL_DIR=%BUILD_DIR%%BOOST_BASENAME%-install\
set BOOST_LIB_DIR=%BOOST_INSTALL_DIR%lib\

rem 以下是新增的mylib相关变量定义，根据实际库情况调整名称、版本、下载地址等信息
set MYLIB_NAME=mylib
set MYLIB_VERSION=1.0.0
set MYLIB_SHA256SUM="这里填写对应版本mylib的正确SHA256校验和"

set MYLIB_TEMP_FOLDER=mylib_%MYLIB_VERSION:.=_%
set MYLIB_TEMP_FILE=%MYLIB_TEMP_FOLDER%.zip
set MYLIB_TEMP_FILE_DIR=%BUILD_DIR%%MYLIB_TEMP_FILE%

set MYLIB_REPO="这里填写mylib的实际下载地址，例如https://example.com/mylib-%MYLIB_VERSION%.zip"
set MYLIB_SRC_DIR=%BUILD_DIR%%MYLIB_NAME%-%MYLIB_VERSION%-source\
set MYLIB_INSTALL_DIR=%BUILD_DIR%%MYLIB_NAME%-%MYLIB_VERSION%-install\
set MYLIB_LIB_DIR=%MYLIB_INSTALL_DIR%lib\

rem ============================================================================
rem -- Get Boost ---------------------------------------------------------------
rem ============================================================================

if exist "%BOOST_INSTALL_DIR%" (
    goto already_build
)

set _checksum=""

if not exist "%BOOST_SRC_DIR%" (
    if not exist "%BOOST_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving boost.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%BOOST_REPO%', '%BOOST_TEMP_FILE_DIR%')"
        call :CheckSumEvaluate %BOOST_TEMP_FILE_DIR%,%BOOST_SHA256SUM%,_checksum
    )
    if "!_checksum!" == "1" (
        echo %FILE_N% Using Boost backup
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('https://carla-releases.s3.us-east-005.backblazeb2.com/Backup/%BOOST_TEMP_FILE%', '%BOOST_TEMP_FILE_DIR%')"
        call :CheckSumEvaluate %BOOST_TEMP_FILE_DIR%,%BOOST_SHA256SUM%,_checksum
    )
    if "!_checksum!" == "1" goto error_download
    echo %FILE_N% Extracting boost from "%BOOST_TEMP_FILE%", this can take a while...
    if exist "%ProgramW6432%/7-Zip/7z.exe" (
        "%ProgramW6432%/7-Zip/7z.exe" x "%BOOST_TEMP_FILE_DIR%" -o"%BUILD_DIR%" -y
    ) else (
        powershell -Command "Expand-Archive '%BOOST_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%' -Force"
    )
    echo %FILE_N% Removing "%BOOST_TEMP_FILE%"
    del "%BOOST_TEMP_FILE_DIR%"
    rename "%BUILD_DIR%%BOOST_TEMP_FOLDER%" "%BOOST_BASENAME%-source"
) else (
    echo %FILE_N% Not downloading boost because already exists the folder "%BOOST_SRC_DIR%".
)

cd "%BOOST_SRC_DIR%"
if not exist "b2.exe" (
    echo %FILE_N% Generating build...
    call bootstrap.bat vc141
)

if %errorlevel% neq 0 goto error_bootstrap

rem This fix some kind of issue installing headers of boost < 1.67, not installing correctly
rem echo %FILE_N% Packing headers...
rem b2 headers link=static

echo %FILE_N% Building...
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
if %errorlevel% neq 0 goto error_install

for /d %%i in ("%BOOST_INSTALL_DIR%boost*") do rename "%%i" include
goto success

rem ============================================================================
rem -- 添加mylib的下载、安装及配置相关操作开始 ------------------------------------
rem 以下代码逻辑与Boost的处理类似，但需根据mylib实际情况调整，比如构建、安装命令等
rem ============================================================================

rem -- Get mylib ---------------------------------------------------------------
if exist "%MYLIB_INSTALL_DIR%" (
    goto mylib_already_build
)

set _mylib_checksum=""

if not exist "%MYLIB_SRC_DIR%" (
    if not exist "%MYLIB_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %MYLIB_NAME%.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%MYLIB_REPO%', '%MYLIB_TEMP_FILE_DIR%')"
        call :CheckSumEvaluate %MYLIB_TEMP_FILE_DIR%,%MYLIB_SHA256SUM%,_mylib_checksum
    )
    if "!_mylib_checksum!" == "1" goto mylib_error_download
    echo %FILE_N% Extracting %MYLIB_NAME% from "%MYLIB_TEMP_FILE%", this can take a while...
    if exist "%ProgramW6432%/7-Zip/7z.exe" (
        "%ProgramW6432%/7-Zip/7z.exe" x "%MYLIB_TEMP_FILE_DIR%" -o"%BUILD_DIR%" -y
    ) else (
        powershell -Command "Expand-Archive '%MYLIB_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%' -Force"
    )
    echo %FILE_N% Removing "%MYLIB_TEMP_FILE%"
    del "%MYLIB_TEMP_FILE_DIR%"
    rename "%BUILD_DIR%%MYLIB_TEMP_FOLDER%" "%MYLIB_NAME%-%MYLIB_VERSION%-source"
) else {
    echo %FILE_N% Not downloading %MYLIB_NAME% because already exists the folder "%MYLIB_SRC_DIR%".
}

rem 进入mylib源文件目录，以下假设mylib有类似的构建脚本（如build.bat等，需根据实际情况调整）
cd "%MYLIB_SRC_DIR%"
if not exist "build.bat" (
    echo %FILE_N% Generating build for %MYLIB_NAME%...
    rem 这里需替换为实际生成构建的命令，比如调用相应的配置脚本等，以下只是示例占位
    call configure.bat
)

if %errorlevel% neq 0 goto mylib_error_bootstrap

echo %FILE_N% Building %MYLIB_NAME%...
rem 以下构建命令需根据mylib实际的构建系统（如Makefile、CMake等）和要求调整
build.bat -j%NUMBER_OF_ASYNC_JOBS%^
    --install-dir="%MYLIB_INSTALL_DIR:~0,-1%"^
    install
if %errorlevel% neq 0 goto mylib_error_install

rem 假设安装后需要对mylib的文件结构做一些整理（比如移动头文件、库文件到合适位置等，根据实际情况调整）
rem 以下只是示例，比如将头文件移动到统一的include目录下
mkdir "%MYLIB_INSTALL_DIR%include"
xcopy /Y /S /I "%MYLIB_SRC_DIR%include\*" "%MYLIB_INSTALL_DIR%include\*" > NUL

goto mylib_success

rem ============================================================================
rem -- Messages and Errors（包含mylib相关的错误处理和提示）-----------------------------------------------------
rem ============================================================================

:help
    echo %FILE_N% Download and install a boost version.
    echo "Usage: %FILE_N% [-h|--help] [-v|--version] [--toolset] [--build-dir] [-j]"
    goto eof

:success
    echo.
    echo %FILE_N% Boost has been successfully installed in "%BOOST_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A Boost installation already exists.
    echo %FILE_N% Delete "%BOOST_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

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

:error_bootstrap
    echo.
    echo %FILE_N% [BOOTSTRAP ERROR] An error ocurred while executing "bootstrap.bat".
    goto bad_exit

:error_install
    echo.
    echo %FILE_N% [B2 ERROR] An error ocurred while installing using "b2.exe".
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    endlocal
    rem A return value used for checking for errors
    set install_boost=done
    exit /b 0

:bad_exit
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

:mylib_already_build
    echo %FILE_N% A %MYLIB_NAME% installation already exists.
    echo %FILE_N% Delete "%MYLIB_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:mylib_error_download
    echo.
    echo %FILE_N% [GIT ERROR] An error ocurred while downloading %MYLIB_NAME%.
    echo %FILE_N% [GIT ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%MYLIB_REPO%"
    echo %FILE_N% [GIT ERROR] Workaround:
    echo %FILE_N%              - Download the source code of %MYLIB_NAME% "%MYLIB_VERSION%" and
    echo %FILE_N%                extract the content of "%MYLIB_TEMP_FOLDER%" in
    echo %FILE_N%                "%MYLIB_SRC_DIR%"
    goto bad_exit

:mylib_error_bootstrap
    echo.
    echo %FILE_N% [BOOTSTRAP ERROR] An error ocurred while executing the build generation for %MYLIB_NAME%.
    goto bad_exit

:mylib_error_install
    echo.
    echo %FILE_N% [BUILD ERROR] An error ocurred while installing using the build commands for %MYLIB_NAME%.
    goto bad_exit

:mylib_success
    echo.
    echo %FILE_N% %MYLIB_NAME% has been successfully installed in "%MYLIB_INSTALL_DIR%"!
    goto good_exit
:eof
endlocal
exit /b 0
