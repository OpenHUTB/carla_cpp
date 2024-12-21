REM @echo off
setlocal

rem BAT script that downloads and installs a ready to use
rem x64 eigen for CARLA (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

:arg-parse
if not "%1"=="" (
rem 判断第一个命令行参数是否等于"--build-dir"，可能用于指定构建相关的目录路径等操作
    if "%1"=="--build-dir" (
        rem 如果第一个参数是"--build-dir"，则将第二个参数（%~dpn2会获取去掉文件名只保留路径的形式）的值赋给BUILD_DIR变量，用于后续构建目录相关的操作
        set BUILD_DIR=%~dpn2
        rem 使用shift命令，将命令行参数列表整体左移一位，这样原本的第二个参数就变成了第一个参数，方便后续继续判断新的第一个参数情况
        shift
    )
    rem 判断第一个命令行参数是否等于"-h"，通常这是一种简洁的帮助命令选项格式
    if "%1"=="-h" (
        rem 如果参数是"-h"，则跳转到名为help的代码标签处执行相应帮助相关的代码逻辑，一般用于显示使用说明等
        goto help
    )
    rem 判断第一个命令行参数是否等于"--help"，这也是常见的用于请求显示帮助信息的命令行选项格式
    if "%1"=="--help" (
        rem 如果参数是"--help"，同样跳转到help代码标签处去执行帮助相关的操作逻辑
        goto help
    )
    rem 使用shift命令将命令行参数列表整体左移一位，为后续可能的其他参数解析等操作做准备，比如再次循环判断新的第一个参数
    shift
    rem :: 跳转到名为:arg-parse的代码标签处继续执行后续的参数解析或者基于参数的其他处理逻辑
    goto :arg-parse
)

rem If not set set the build dir to the current dir
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

rem ============================================================================
rem -- Get Eigen (CARLA dependency) -------------------------------------------
rem ============================================================================

set EIGEN_VERSION=3.1.0
set EIGEN_REPO=https://gitlab.com/libeigen/eigen/-/archive/%EIGEN_VERSION%/eigen-%EIGEN_VERSION%.zip
set EIGEN_BASENAME=eigen-%EIGEN_VERSION%

set EIGEN_SRC_DIR=%BUILD_DIR%%EIGEN_BASENAME%
set EIGEN_INSTALL_DIR=%BUILD_DIR%eigen-install
set EIGEN_INCLUDE=%EIGEN_INSTALL_DIR%\include
set EIGEN_TEMP_FILE=eigen-%EIGEN_VERSION%.zip
set EIGEN_TEMP_FILE_DIR=%BUILD_DIR%eigen-%EIGEN_VERSION%.zip

if not exist "%EIGEN_SRC_DIR%" (
    rem 在上面的目录不存在的基础上，进一步判断EIGEN_TEMP_FILE_DIR环境变量代表的目录是否也不存在，如果不存在则执行后续下载相关操作
    if not exist "%EIGEN_TEMP_FILE_DIR%" (
        rem 输出提示信息，告知当前正在获取（下载）EIGEN_TEMP_FILE_DIR所代表的文件或目录，这里的%FILE_N%应该是一个自定义的文件名相关变量，用于标识具体操作的对象
        echo %FILE_N% Retrieving %EIGEN_TEMP_FILE_DIR%.
        rem 使用PowerShell命令，通过System.Net.WebClient类来下载文件，将从EIGEN_REPO所指定的地址下载文件到EIGEN_TEMP_FILE_DIR所指定的目标位置
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%EIGEN_REPO%', '%EIGEN_TEMP_FILE_DIR%')"
    )
    rem 判断上一步执行PowerShell下载命令后的错误级别（errorlevel），如果不等于0（意味着出现错误），则跳转到error_download_eigen标签处执行相应的错误处理逻辑
    if %errorlevel% neq 0 goto error_download_eigen
    rem 这里是一个注释，提醒接下来的代码功能是解压已下载的库文件，方便阅读代码时能快速知晓下面代码的大致意图
    rem Extract the downloaded library
    echo %FILE_N% Extracting eigen from "%EIGEN_TEMP_FILE%".
    powershell -Command "Expand-Archive '%EIGEN_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%'"
    if %errorlevel% neq 0 goto error_extracting
    echo %EIGEN_SRC_DIR%

    del %EIGEN_TEMP_FILE_DIR%

)

if not exist "%EIGEN_INSTALL_DIR%" (
    mkdir %EIGEN_INSTALL_DIR%
    mkdir %EIGEN_INCLUDE%
    mkdir %EIGEN_INCLUDE%\unsupported
    mkdir %EIGEN_INCLUDE%\Eigen
)

xcopy /q /Y /S /I /d "%EIGEN_SRC_DIR%\Eigen" "%EIGEN_INCLUDE%\Eigen"
xcopy /q /Y /S /I /d "%EIGEN_SRC_DIR%\unsupported\Eigen" "%EIGEN_INCLUDE%\unsupported\Eigen"
copy "%BUILD_DIR%..\Util\Patches\Eigen3.1.0\Macros.h" "%EIGEN_INCLUDE%\Eigen\src\Core\util\Macros.h"
copy "%BUILD_DIR%..\Util\Patches\Eigen3.1.0\VectorBlock.h" "%EIGEN_INCLUDE%\Eigen\src\Core\VectorBlock.h"

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================
rem 以下是名为:help的代码标签部分，通常用于显示帮助信息，告知用户该脚本的功能及使用方法等相关内容
:help
    rem 输出提示信息，向用户说明这个脚本的主要功能是下载并安装Chrono库（虽然这里文本里写的是Chrono库，但前面代码似乎是关于Eigen库的，可能存在一些不一致，需根据实际情况确认），%FILE_N%应该是一个自定义的文件名相关变量，用于标识具体操作的对象
    echo %FILE_N% Download and install a the Chrono library.
    rem 输出该脚本的使用方法说明，展示了可用的命令行参数选项，比如-h或者--help用于获取帮助，--build-dir和--zlib-install-dir可能是用于指定构建目录、zlib安装目录等相关功能的参数，这里告知用户如何正确使用该脚本
    echo "Usage: %FILE_N% [-h^|--help] [--build-dir] [--zlib-install-dir]"
    rem 跳转到名为:eof的代码标签处，可能在那里有脚本结束相关的处理逻辑，比如清理临时变量、关闭文件等操作（具体需结合:eof标签对应的代码来看）
    goto eof
rem 以下是名为:success的代码标签部分，一般用于在操作成功（这里推测是Eigen库安装成功的情况）时向用户显示相应的成功提示信息
:success
    echo.
    rem 输出成功提示信息，告知用户Eigen库已经成功安装到了EIGEN_INSTALL_DIR所指定的目录下，同样%FILE_N%用于标识操作对象，方便用户明确是哪个脚本执行的操作以及对应的结果
    echo %FILE_N% Eigen has been successfully installed in "%EIGEN_INSTALL_DIR%"!
    rem  跳转到名为:good_exit的代码标签处，可能那里有统一的正常退出脚本的相关处理逻辑，比如释放资源、返回合适的退出码等（具体要结合:good_exit标签对应的代码来确定）
    goto good_exit
rem 以下是名为:already_build的代码标签部分，大概率用于处理已经存在相关安装（这里推测是Eigen库安装）的情况，告知用户相应的信息以及后续如果要强制重新构建需要做的操作
:already_build
    rem 输出提示信息，告知用户已经存在一个Eigen库的安装（这里根据代码文本推测，实际情况需结合更多上下文判断）
    echo %FILE_N% A xerces installation already exists.
    rem     :: 进一步输出提示信息，告诉用户如果想要强制重新构建Eigen库，需要删除EIGEN_INSTALL_DIR所指定的目录，引导用户进行后续操作
    echo %FILE_N% Delete "%EIGEN_INSTALL_DIR%" if you want to force a rebuild.
    rem 跳转到名为:good_exit的代码标签处，和上面类似，去执行统一的正常退出脚本的相关处理逻辑
    goto good_exit

:error_download_eigen
    echo.
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
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the xerces's source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%EIGEN_SRC_DIR%"
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
    rem A return value used for checking for errors
    endlocal & set install_chrono=%CHRONO_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%EIGEN_INSTALL_DIR%" rd /s /q "%EIGEN_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
