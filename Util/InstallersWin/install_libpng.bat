@echo off
setlocal
// @echo off 命令用于关闭命令行窗口中命令执行时的回显功能，也就是执行脚本时，不会把每条命令本身显示在屏幕上，使输出更加简洁，只显示我们想要展示的结果信息。
// setlocal 命令用于设置局部环境，确保在这个批处理脚本中定义的变量只在该脚本运行的环境内有效，不会影响到外部原有的系统环境变量设置，这是一种良好的变量作用域管理方式，避免变量冲突。

rem BAT script that downloads and installs a ready to use
rem x64 libpng build for CARLA (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
// %~dp0 是一个批处理脚本中的特殊变量扩展语法，它表示获取当前批处理脚本所在的驱动器号和完整路径（例如：D:\scripts\ 这样的格式），并将其赋值给 LOCAL_PATH 变量，方便后续基于这个基础路径进行相关文件操作等的路径拼接。
set FILE_N=    -[%~n0]:
// %~n0 同样是特殊变量扩展语法，它获取的是当前批处理脚本的文件名（不含扩展名），比如脚本叫 myscript.bat，那这里获取到的就是 "myscript"，然后定义 FILE_N 变量并按照指定格式设置，这个变量后续主要用于在输出提示信息等内容时添加一个有辨识度的前缀，便于查看信息来源和区分不同阶段的输出内容。

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*
// 输出当前批处理脚本运行时接收到的所有命令行参数，%* 代表所有的命令行参数，通过这种方式可以在调试阶段方便地查看传入了哪些参数，有助于排查参数传递是否正确以及脚本对参数处理是否符合预期等问题。

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

:arg-parse
if not "%1"=="" (
     // 判断第一个命令行参数是否为空字符串，如果不为空，才进入下面对具体参数的判断和处理逻辑，这是一个常见的循环处理命令行参数的开头条件判断方式。
    rem 判断第一个参数是否为 --build-dir
    if "%1"=="--build-dir" (
        // 判断第一个参数是不是 "--build-dir"，这通常是一种命令行参数的约定格式，用于指定某个特定功能相关的路径等信息，在这里大概率是和构建相关的目录路径设置。
        rem 如果是 --build-dir，设置 BUILD_DIR 变量为第二个参数代表的路径相关内容（含盘符、路径、文件名）
        set BUILD_DIR=%~dpn2
         // %~dpn2 也是特殊的变量扩展语法，它获取的是第二个命令行参数所代表的包含驱动器号、完整路径以及文件名（如果有的话）的完整信息，将其赋值给 BUILD_DIR 变量，这样后续就可以基于这个变量所代表的路径去进行一些和构建相关的操作，比如存放构建生成的文件等。
        rem 将命令行参数向左移动一位，为处理后续参数做准备
        shift
    )
    rem 判断第一个参数是否为 --zlib-install-dir
    if "%1"=="--zlib-install-dir" (
        rem 如果是 --zlib-install-dir，设置 ZLIB_INST_DIR 变量为第二个参数代表的路径相关内容（含盘符、路径、文件名）
        set ZLIB_INST_DIR=%~dpn2
        rem 将命令行参数向左移动一位，为处理后续参数做准备
        shift
    )
    rem 判断第一个参数是否为 -h（短格式帮助参数）
    if "%1"=="-h" (
        rem 如果是 -h，跳转到 help 标签处显示帮助信息
        goto help
    )
    rem 判断第一个参数是否为 --help（长格式帮助参数）
    if "%1"=="--help" (
        rem 如果是 --help，跳转到 help 标签处显示帮助信息
        goto help
    )
    rem 将命令行参数向左移动一位，为处理后续参数做准备
    shift
    rem 跳转到 :arg-parse 标签处，继续解析下一组参数
    goto :arg-parse
)

if "%ZLIB_INST_DIR%" == "" (
    echo %FILE_N% You must specify a zlib install directory using [--zlib-install-dir]
    goto bad_exit
)
if not "%ZLIB_INST_DIR:~-1%"=="\" set ZLIB_INST_DIR=%ZLIB_INST_DIR%\
// 这部分首先判断 ZLIB_INST_DIR 变量是否为空，如果为空就输出提示信息，告知用户必须使用 "--zlib-install-dir" 参数来指定 zlib 的安装目录，然后跳转到 bad_exit 标签处（虽然此处没展示 bad_exit 具体内容，但通常用于错误退出的相关处理，比如清理临时文件、返回错误码等操作）。
// 接着判断 ZLIB_INST_DIR 变量所代表的路径字符串的最后一个字符是不是反斜杠 "\ "，如果不是，就给它补上，这是为了保证路径格式的规范性，在后续进行文件路径拼接等操作时不容易出现错误，很多文件和目录操作函数都要求路径结尾以反斜杠结束（如果是目录路径的话），符合 Windows 操作系统下路径处理的常规习惯。

rem If not set set the build dir to the current dir
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\
// 类似地，对于 BUILD_DIR 变量，如果它为空，就将其设置为当前批处理脚本所在的路径（通过 %~dp0 获取），同样也检查其末尾字符是否为反斜杠，不是的话就补上，确保后续使用该路径时格式正确，能准确地定位到相关的构建目录等。

rem ============================================================================
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

set LIBPNG_BASENAME=libpng
set LIBPNG_VERSION=1.2.37
// 这里定义了两个基础变量，LIBPNG_BASENAME 表示要操作的库（libpng）的基本名称，LIBPNG_VERSION 明确了该库的版本号，这是一种常见的对软件资源进行标识和管理的方式，后续很多和该库相关的文件名、目录名等都会基于这两个变量来生成，方便在不同版本升级或者更换库等情况下快速修改相关的配置。

rem libpng-x.x.x
set LIBPNG_TEMP_FOLDER=%LIBPNG_BASENAME%-%LIBPNG_VERSION%
// 根据前面定义的基本名称和版本号，组合生成一个临时文件夹的名称，形式为 "libpng-1.2.37"，这个临时文件夹可能会用于存放下载的库文件压缩包解压后的源码等内容，方便在安装过程中进行临时的文件管理和操作。
rem libpng-x.x.x-src.zip
set LIBPNG_TEMP_FILE=%LIBPNG_TEMP_FOLDER%-src.zip
// 基于临时文件夹名称进一步生成临时文件的名称，即 "libpng-1.2.37-src.zip"，可以推测这个文件就是要从网络上下载的 libpng 库的源码压缩包文件名，后续会通过这个文件名去进行下载、保存等操作。

rem ../libpng-x.x.x-src.zip
set LIBPNG_TEMP_FILE_DIR=%BUILD_DIR%%LIBPNG_TEMP_FILE%
// 将前面生成的临时文件名称和之前处理得到的构建目录（BUILD_DIR）拼接起来，形成临时文件在本地磁盘上完整的保存路径，例如如果 BUILD_DIR 是 "D:\projects\build\"，那这里 LIBPNG_TEMP_FILE_DIR 就会是 "D:\projects\build\libpng-1.2.37-src.zip"，方便后续准确地找到该文件进行读写操作等。
set LIBPNG_REPO=http://downloads.sourceforge.net/gnuwin32/libpng-%LIBPNG_VERSION%-src.zip

rem ../libpng-x.x.x-source/
set LIBPNG_SRC_DIR=%BUILD_DIR%%LIBPNG_BASENAME%-%LIBPNG_VERSION%-source\
rem ../libpng-x.x.x-install/
set LIBPNG_INSTALL_DIR=%BUILD_DIR%%LIBPNG_BASENAME%-%LIBPNG_VERSION%-install\

rem ============================================================================
rem -- Get libpng --------------------------------------------------------------
rem ============================================================================

if exist "%LIBPNG_INSTALL_DIR%" (
    goto already_build
)

if not exist "%LIBPNG_SRC_DIR%" (
    if not exist "%LIBPNG_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %LIBPNG_BASENAME%.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%LIBPNG_REPO%', '%LIBPNG_TEMP_FILE_DIR%')"
    )
    if not exist "%LIBPNG_TEMP_FILE_DIR%" (
        echo %FILE_N% Using %LIBPNG_BASENAME% from backup.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('https://carla-releases.s3.us-east-005.backblazeb2.com/Backup/libpng-%LIBPNG_VERSION%-src.zip', '%LIBPNG_TEMP_FILE_DIR%')"
    )
    if %errorlevel% neq 0 goto error_download
    rem Extract the downloaded library
    echo %FILE_N% Extracting libpng from "%LIBPNG_TEMP_FILE%".
    powershell -Command "Expand-Archive '%LIBPNG_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%'"
    if %errorlevel% neq 0 goto error_extracting

    rem Remove unnecessary files and folders
    echo %FILE_N% Removing "%LIBPNG_TEMP_FILE%"
    del "%LIBPNG_TEMP_FILE_DIR%"
    echo %FILE_N% Removing dir "%BUILD_DIR%manifest"
    rmdir /s/q "%BUILD_DIR%manifest"

    rename "%BUILD_DIR%src" "%LIBPNG_BASENAME%-%LIBPNG_VERSION%-source"
) else (
    echo %FILE_N% Not downloading libpng because already exists the folder "%LIBPNG_SRC_DIR%".
)

rem ============================================================================
rem -- Compile libpng ----------------------------------------------------------
rem ============================================================================

set LIBPNG_SOURCE_DIR=%LIBPNG_SRC_DIR%libpng\%LIBPNG_VERSION%\libpng-%LIBPNG_VERSION%-src\

if not exist "%LIBPNG_SRC_DIR%build" (
    echo %FILE_N% Creating "%LIBPNG_SRC_DIR%build"
    mkdir "%LIBPNG_SRC_DIR%build"
)

cd "%LIBPNG_SRC_DIR%build"

cl /nologo /c /O2 /MD /Z7 /EHsc /MP /W2 /TP /GR /Gm-^
 -DWIN32 -DNDEBUG -D_CRT_SECURE_NO_WARNINGS -DPNG_NO_MMX_CODE^
 /I"%ZLIB_INST_DIR%include"^
 "%LIBPNG_SOURCE_DIR%*.c"

if not exist "%LIBPNG_INSTALL_DIR%lib" (
    echo %FILE_N% Creating "%LIBPNG_INSTALL_DIR%lib"
    mkdir "%LIBPNG_INSTALL_DIR%lib"
)

if not exist "%LIBPNG_INSTALL_DIR%include" (
    echo %FILE_N% Creating "%LIBPNG_INSTALL_DIR%include"
    mkdir "%LIBPNG_INSTALL_DIR%include"
)

lib /nologo /MACHINE:X64 /LTCG /OUT:"%LIBPNG_INSTALL_DIR%lib\libpng.lib"^
 /LIBPATH:"%ZLIB_INST_DIR%lib" "*.obj" "zlibstatic.lib"

copy "%LIBPNG_SOURCE_DIR%png.h" "%LIBPNG_INSTALL_DIR%include\png.h"
copy "%LIBPNG_SOURCE_DIR%pngconf.h" "%LIBPNG_INSTALL_DIR%include\pngconf.h"

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    echo %FILE_N% Download and install a libpng.
    echo "Usage: %FILE_N% [-h^|--help] [--build-dir] [--zlib-install-dir]"
    goto eof

:success
    echo.
    echo %FILE_N% libpng has been successfully installed in "%LIBPNG_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A libpng installation already exists.
    echo %FILE_N% Delete "%LIBPNG_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_download
    echo.
    echo %FILE_N% [DOWNLOAD ERROR] An error ocurred while downloading libpng.
    echo %FILE_N% [DOWNLOAD ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%LIBPNG_REPO%"
    echo %FILE_N% [DOWNLOAD ERROR] Workaround:
    echo %FILE_N%              - Download the libpng's source code and
    echo %FILE_N%                extract the content in
    echo %FILE_N%                "%LIBPNG_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_extracting
    echo.
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the libpng's source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%LIBPNG_SRC_DIR%"
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
    endlocal & set install_libpng=%LIBPNG_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%LIBPNG_INSTALL_DIR%" rd /s /q "%LIBPNG_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
