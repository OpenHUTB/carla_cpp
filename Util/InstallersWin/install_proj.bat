REM @echo off
setlocal

rem 此批处理脚本用于下载并安装适用于CARLA（carla.org）的x64版本的PROJ库。
rem 需要在启用了x64 Visual C++工具集的命令提示符（cmd）环境中运行该脚本。

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem 打印批处理传入的参数（用于调试目的），%*表示所有传入的命令行参数，通过这种方式可以查看实际运行脚本时传入了哪些参数。
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

:arg-parse
if not "%1"=="" (
    if "%1"=="--build-dir" (
        rem 如果传入的第一个参数是 --build-dir，意味着要指定构建目录。
        rem 将下一个参数（%2）作为构建目录的完整路径（包含文件名部分）赋值给BUILD_DIR变量，
        rem 然后使用 shift 命令将参数指针向后移动一位，以便处理后续参数。
        set BUILD_DIR=%~dpn2
        shift
    )
    if "%1"=="-h" (
        rem 如果参数是 -h，跳转到 help 标签处，用于显示帮助信息。
        goto help
    )
    if "%1"=="--help" (
        rem 同理，若参数是 --help，也跳转到 help 标签处显示帮助信息。
        goto help
    )
    if "%1"=="--generator" (
        rem 当参数为 --generator 时，将下一个参数（%2）作为生成器名称赋值给GENERATOR变量，
        rem 例如可能是指定使用的编译工具链等相关生成器信息，之后移动参数指针。
        set GENERATOR=%2
        shift
    )
    shift
    goto :arg-parse
)

rem 如果GENERATOR变量为空（即未通过命令行参数指定生成器），则设置默认的生成器为"Visual Studio 16 2019"。
if %GENERATOR% == "" set GENERATOR="Visual Studio 16 2019"

rem 如果BUILD_DIR变量未被设置（即未通过命令行指定构建目录），将其设置为当前目录，
rem 并且通过判断路径最后一位字符是否为反斜杠（\），若不是则添加反斜杠，确保路径格式规范。
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

rem ============================================================================
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

rem 设置PROJ库的基础名称，包含了版本号信息，后续用于构建相关的文件名、路径等。
set PROJ_BASE_NAME=proj-7.2.1
rem 根据基础名称设置PROJ库的压缩包文件名，格式为基础名称加上.tar.gz后缀，代表这是一个tar压缩包文件。
set PROJ_ZIP=%PROJ_BASE_NAME%.tar.gz
rem 构建PROJ库压缩包在本地的完整路径，由构建目录（BUILD_DIR）和压缩包文件名（PROJ_ZIP）组合而成。
set PROJ_ZIP_DIR=%BUILD_DIR%%PROJ_ZIP%
rem 设置PROJ库源码的下载地址，通过将基础名称嵌入到固定的网址模板中，确定从何处下载PROJ库的源码压缩包。
set PROJ_REPO=https://download.osgeo.org/proj/%PROJ_ZIP%

rem 设置SQLite库的安装目录路径，这可能是因为PROJ库的构建过程中会依赖SQLite库，提前定义好其相关路径方便后续配置引用。
set SQLITE_INSTALL_DIR=%BUILD_DIR%sqlite3-install
rem 设置SQLite库安装目录下的头文件目录路径，以便后续配置PROJ库编译时能正确找到SQLite的头文件。
set SQLITE_INCLUDE_DIR=%SQLITE_INSTALL_DIR%\include
rem 设置SQLite库可执行文件的完整路径，明确其所在位置，可能在与PROJ库交互等场景中会用到。
set SQLITE_BIN=%SQLITE_INSTALL_DIR%\bin\sqlite.exe
rem 设置SQLite库的库文件完整路径，同样是为了让PROJ库在链接等阶段能找到对应的库文件。
set SQLITE_LIB=%SQLITE_INSTALL_DIR%\lib\sqlite3.lib

rem 设置PROJ库源码解压后的目录路径，后续的编译等操作将基于此目录展开。
set PROJ_SRC_DIR=%BUILD_DIR%proj-src
rem 设置PROJ库的构建目录路径，位于源码目录下，是实际执行编译构建命令的工作目录。
set PROJ_BUILD_DIR=%PROJ_SRC_DIR%\build
rem 设置PROJ库最终的安装目录路径，编译生成的文件将安装到此目录下，方便后续项目使用。
set PROJ_INSTALL_DIR=%BUILD_DIR%proj-install

rem 设置PROJ库安装目录下的头文件目录路径，用于存放编译生成的头文件以及拷贝相关依赖的头文件等。
set PROJ_INCLUDE_DIR=%PROJ_INSTALL_DIR%\include
rem 设置PROJ库安装目录下的可执行文件目录路径，生成的可执行程序将放置在此处。
set PROJ_BIN_DIR=%PROJ_INSTALL_DIR%\bin
rem 设置PROJ库安装目录下的库文件目录路径，编译生成的静态库或动态库文件会存放在这里。

rem 如果PROJ库已经安装（即安装目录存在），则跳转到 already_build 标签处，执行相应的已安装处理逻辑。
if exist "%PROJ_INSTALL_DIR%" (
    goto already_build
)

rem 输出提示信息，表示正在从指定网址下载PROJ库，方便用户在运行脚本时知晓当前操作进度。
echo %FILE_N% Retrieving %PROJ_BASE_NAME%.
rem 使用PowerShell命令下载PROJ库的压缩包到指定的本地路径（PROJ_ZIP_DIR），
rem 通过System.Net.WebClient类来实现网络文件下载功能。
powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%PROJ_REPO%', '%PROJ_ZIP_DIR%')"
rem 如果下载过程出现错误（通过判断errorlevel是否不等于0来确定，errorlevel在命令执行出错时会被设置为非0值），
rem 则跳转到 error_download 标签处，执行相应的下载错误处理逻辑。
if %errorlevel% neq 0 goto error_download

rem 切换到构建目录所在的上级目录（即BUILD_DIR），为后续解压操作做准备，确保解压后的文件能放置在正确的位置。
cd %BUILD_DIR%
rem 输出提示信息，表示正在解压PROJ压缩包，让用户了解脚本执行情况。
echo %FILE_N% Extracting PROJ from "%PROJ_ZIP%".
rem 使用tar命令解压下载的.tar.gz压缩包，-xzf 参数分别表示解压（x）、使用gzip格式（z）以及指定压缩包文件名（f）。
call tar -xzf %PROJ_ZIP%
rem 将解压后的文件夹（名称为PROJ_BASE_NAME所定义的名称）移动到指定的源码目录（PROJ_SRC_DIR）下，使其目录结构符合后续操作要求。
move %BUILD_DIR%%PROJ_BASE_NAME% %PROJ_SRC_DIR%

rem 创建PROJ库的构建目录，为后续在该目录下执行cmake等构建操作创建必要的工作空间。
mkdir %PROJ_BUILD_DIR%
rem 切换到PROJ库的构建目录，后续的配置、编译等操作都将在此目录下进行。
cd %PROJ_BUILD_DIR%

rem 通过判断生成器名称（GENERATOR）中是否包含"Visual Studio"字符串，来确定是否需要设置平台相关的参数。
rem 如果包含，则设置PLATFORM为 -A x64，表示构建64位平台的项目；否则将PLATFORM设置为空，可能对应其他默认平台配置或后续根据实际情况处理。
echo.%GENERATOR% | findstr /C:"Visual Studio" >nul && (
    set PLATFORM=-A x64
) || (
    set PLATFORM=
)

rem 使用cmake配置构建项目，指定一系列的参数：
rem -G %GENERATOR%：指定使用的生成器，也就是编译工具链相关的配置，例如前面提到的"Visual Studio 16 2019"。
rem %PLATFORM%：平台相关参数，如前面判断设置的 -A x64 等，用于确定构建的目标平台架构。
rem -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"：设置C++编译Release模式下的一些编译标志，/MD表示使用多线程DLL运行时库，/MP表示启用多处理器编译，以优化编译过程。
rem -DCMAKE_CXX_FLAGS="/MD /MP"：设置通用的C++编译标志，同样包含上述的运行时库和多处理器编译相关设置。
rem -DSQLITE3_INCLUDE_DIR=%SQLITE_INCLUDE_DIR% -DSQLITE3_LIBRARY=%SQLITE_LIB%：指定SQLite库的头文件目录和库文件路径，以便PROJ库在编译时能正确链接和使用SQLite。
rem -DEXE_SQLITE3=%SQLITE_BIN%：指定SQLite可执行文件的路径，可能在PROJ库构建过程中涉及到与该可执行文件的交互等情况。
rem -DENABLE_TIFF=OFF -DENABLE_CURL=OFF 等一系列 -DXXX=OFF 参数：用于关闭PROJ库相关的一些功能模块的构建，比如TIFF支持、CURL支持、共享库构建、各种工具（如PROJSYNC、PROJINFO等）构建以及测试构建等，可能是根据项目实际需求只需要部分功能，或者为了减少构建时间、避免不必要的依赖等原因。
rem -DCMAKE_BUILD_TYPE=Release：明确构建类型为Release版本，通常表示生成优化后的、可用于生产环境的二进制文件。
rem -DCMAKE_INSTALL_PREFIX=%PROJ_INSTALL_DIR%：指定项目最终的安装目录，编译生成的文件将按照此目录结构进行安装部署。
if %errorlevel% neq 0 goto error_cmake

rem 使用cmake进行构建并安装，指定构建配置为Release，并执行安装目标，将编译生成的各种文件（头文件、库文件、可执行文件等）安装到指定的安装前缀目录（PROJ_INSTALL_DIR）下。
cmake --build. --config Release --target install 

rem 删除下载的PROJ库压缩包文件，因为已经解压并完成了构建安装，该压缩包文件不再需要，删除它可以节省磁盘空间。
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
    rem A return值用于检查错误情况，将安装目录路径设置到环境变量install_proj中并返回0表示成功退出，方便外部脚本获取安装结果信息。
    endlocal & set install_proj=%PROJ_INSTALL_DIR%
    exit /b 0

:bad_exit
    rem 如果安装目录存在，删除该安装目录（可能是构建安装出现错误，清理残留的部分安装文件等情况）。
    if exist "%PROJ_INSTALL_DIR%" rd /s /q "%PROJ_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
