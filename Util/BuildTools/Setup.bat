@echo off
setlocal enabledelayedexpansion

rem BAT脚本用于下载并生成CARLA（carla.org）所需的rpclib、gtest和boost等库
rem 需要在启用了x64 Visual C++工具集的命令提示符（cmd）中运行

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0]:

rem 打印批处理脚本接收到的命令行参数（用于调试目的）
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- 检查编译器是否存在 ------------------------------------------------------
rem ============================================================================

where cl 1>nul
if %errorlevel% neq 0 goto error_cl

rem TODO: 检查是否为x64，而不是x86或x64_x86

rem ============================================================================
rem -- 解析命令行参数 ---------------------------------------------------------
rem ============================================================================

set BOOST_VERSION=1.80.0
set INSTALLERS_DIR=%ROOT_PATH:/=\%Util\InstallersWin\
set VERSION_FILE=%ROOT_PATH:/=\%Util\ContentVersions.txt
set CONTENT_DIR=%ROOT_PATH:/=\%Unreal\CarlaUE4\Content\Carla\
set CARLA_DEPENDENCIES_FOLDER=%ROOT_PATH:/=\%Unreal\CarlaUE4\Plugins\Carla\CarlaDependencies\
set CARLA_BINARIES_FOLDER=%ROOT_PATH:/=\%Unreal\CarlaUE4\Plugins\Carla\Binaries\Win64
set CARLA_PYTHON_DEPENDENCIES=%ROOT_PATH:/=\%PythonAPI\carla\dependencies\
set USE_CHRONO=false
set USE_ROS2=false

:arg-parse
if not "%1"=="" (
    if "%1"=="-j" (
        set NUMBER_OF_ASYNC_JOBS=%2
    )
    if "%1"=="--boost-toolset" (
        set TOOLSET=%2
    )
    if "%1"=="--chrono" (
        set USE_CHRONO=true
    )
    if "%1"=="--ros2" (
        set USE_ROS2=true
    )
    if "%1" == "--generator" (
        set GENERATOR=%2
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

rem 如果未定义，使用Visual Studio 2019作为工具集
if "%TOOLSET%" == "" set TOOLSET=msvc-14.2
if %GENERATOR% == "" set GENERATOR="Visual Studio 16 2019"

rem 如果未设置，将并行任务数量设置为CPU线程数
if "%NUMBER_OF_ASYNC_JOBS%" == "" set NUMBER_OF_ASYNC_JOBS=%NUMBER_OF_PROCESSORS%

rem ============================================================================
rem -- 基础信息与设置 ----------------------------------------------------
rem ============================================================================

set INSTALLATION_DIR=%INSTALLATION_DIR:/=\%

echo %FILE_N% Asynchronous jobs:  %NUMBER_OF_ASYNC_JOBS%
echo %FILE_N% Boost toolset:      %TOOLSET%
echo %FILE_N% Generator:          %GENERATOR%
echo %FILE_N% Install directory:  "%INSTALLATION_DIR%"

if not exist "%CONTENT_DIR%" (
    echo %FILE_N% Creating "%CONTENT_DIR%" folder...
    mkdir "%CONTENT_DIR%"
)

if not exist "%INSTALLATION_DIR%" (
    echo %FILE_N% Creating "%INSTALLATION_DIR%" folder...
    mkdir "%INSTALLATION_DIR%"
)

rem ============================================================================
rem -- 下载并安装zlib ---------------------------------------------------------------
rem ============================================================================

echo %FILE_N% Installing zlib...
call "%INSTALLERS_DIR%install_zlib.bat"^
 --build-dir "%INSTALLATION_DIR%"

if %errorlevel% neq 0 goto failed

if not defined install_zlib (
    echo %FILE_N% Failed while installing zlib.
    goto failed
) else (
    set ZLIB_INSTALL_DIR=%install_zlib%
)

rem ============================================================================
rem -- 下载并安装libpng -------------------------------------------------------------
rem ============================================================================

echo %FILE_N% Installing libpng...
call "%INSTALLERS_DIR%install_libpng.bat"^
 --build-dir "%INSTALLATION_DIR%"^
 --zlib-install-dir "%ZLIB_INSTALL_DIR%"

if %errorlevel% neq 0 goto failed

if not defined install_libpng (
    echo %FILE_N% Failed while installing libpng.
    goto failed
) else (
    set LIBPNG_INSTALL_DIR=%install_libpng%
)

rem ============================================================================
rem -- 下载并安装rpclib -------------------------------------------------------------
rem ============================================================================

echo %FILE_N% Installing rpclib...
call "%INSTALLERS_DIR%install_rpclib.bat"^
 --build-dir "%INSTALLATION_DIR%"^
 --generator %GENERATOR%

if %errorlevel% neq 0 goto failed

if not defined install_rpclib (
    echo %FILE_N% Failed while installing rpclib.
    goto failed
)

rem ============================================================================
rem -- 下载并安装Google Test --------------------------------------------------------
rem ============================================================================

echo %FILE_N% Installing Google Test...
call "%INSTALLERS_DIR%install_gtest.bat"^
 --build-dir "%INSTALLATION_DIR%"^
 --generator %GENERATOR%

if %errorlevel% neq 0 goto failed

if not defined install_gtest (
    echo %FILE_N% Failed while installing Google Test.
    goto failed
)

rem ============================================================================
rem -- 下载并安装Recast & Detour ----------------------------------------------------
rem ============================================================================

echo %FILE_N% Installing "Recast & Detour"...
call "%INSTALLERS_DIR%install_recast.bat"^
 --build-dir "%INSTALLATION_DIR%"^
 --generator %GENERATOR%

if %errorlevel% neq 0 goto failed

if not defined install_recast (
    echo %FILE_N% Failed while installing "Recast & Detour".
    goto failed
) else (
    set RECAST_INSTALL_DIR=%install_recast:\=/%
)

rem ============================================================================
rem -- 下载并安装Fast-DDS（用于ROS2）---------------------------------
rem ============================================================================

if %USE_ROS2% == true (
    echo %FILE_N% Installing "Fast-DDS"...
    call "%INSTALLERS_DIR%install_fastDDS.bat"^
    --build-dir "%INSTALLATION_DIR%"

    if %errorlevel% neq 0 goto failed

    if not defined install_dds (
        echo %FILE_N% Failed while installing "Fast-DDS".
        goto failed
    ) else (
        set FASTDDS_INSTALL_DIR=%install_dds:\=/%
    )
)

rem ============================================================================
rem -- 下载并安装Boost --------------------------------------------------------------
rem ============================================================================

echo %FILE_N% Installing Boost...
call "%INSTALLERS_DIR%install_boost.bat"^
 --build-dir "%INSTALLATION_DIR%"^
 --toolset %TOOLSET%^
 --version %BOOST_VERSION%^
 -j %NUMBER_OF_ASYNC_JOBS%

if %errorlevel% neq 0 goto failed

if not defined install_boost (
    echo %FILE_N% Failed while installing Boost.
    goto failed
)

rem ============================================================================
rem -- 下载并安装Xercesc ------------------------------------------------------------
rem ============================================================================

echo %FILE_N% Installing Xercesc...
call "%INSTALLERS_DIR%install_xercesc.bat"^
 --build-dir "%INSTALLATION_DIR%"^
 --generator %GENERATOR%
copy %INSTALLATION_DIR%\xerces-c-3.2.3-install\lib\xerces-c_3.lib %CARLA_PYTHON_DEPENDENCIES%\lib
copy %INSTALLATION_DIR%\xerces-c-3.2.3-install\lib\xerces-c_3.lib %CARLA_DEPENDENCIES_FOLDER%\lib

rem ============================================================================
rem -- 下载并安装Sqlite3 ------------------------------------------------------------
rem ============================================================================
echo %FILE_N% Installing Sqlite3
call "%INSTALLERS_DIR%install_sqlite3.bat"^
 --build-dir "%INSTALLATION_DIR%"
copy %INSTALLATION_DIR%\sqlite3-install\lib\sqlite3.lib %CARLA_PYTHON_DEPENDENCIES%\lib
copy %INSTALLATION_DIR%\sqlite3-install\lib\sqlite3.lib %CARLA_DEPENDENCIES_FOLDER%\lib

rem ============================================================================
rem -- 下载并安装PROJ ------------------------------------------------------------
rem ============================================================================

echo %FILE_N% Installing PROJ
call "%INSTALLERS_DIR%install_proj.bat"^
 --build-dir "%INSTALLATION_DIR%"^
 --generator %GENERATOR%
copy %INSTALLATION_DIR%\proj-install\lib\proj.lib %CARLA_PYTHON_DEPENDENCIES%\lib
copy %INSTALLATION_DIR%\proj-install\lib\proj.lib %CARLA_DEPENDENCIES_FOLDER%\lib

rem ============================================================================
rem -- 下载并安装Eigen --------------------------------------------------------------
rem ============================================================================

echo %FILE_N% Installing Eigen
call "%INSTALLERS_DIR%install_eigen.bat"^
 --build-dir "%INSTALLATION_DIR%"
xcopy /Y /S /I "%INSTALLATION_DIR%eigen-install\include\*" "%CARLA_DEPENDENCIES_FOLDER%include\*" > NUL

rem ============================================================================
rem -- 下载并安装Chrono --------------------------------------------------------------
rem ============================================================================

if %USE_CHRONO% == true (
    echo %FILE_N% Installing Chrono...
    call "%INSTALLERS_DIR%install_chrono.bat"^
     --build-dir "%INSTALLATION_DIR%" ^
     --generator %GENERATOR%

    if not exist "%CARLA_DEPENDENCIES_FOLDER%" (
        mkdir "%CARLA_DEPENDENCIES_FOLDER%"
    )
    if not exist "%CARLA_DEPENDENCIES_FOLDER%include" (
        mkdir "%CARLA_DEPENDENCIES_FOLDER%include"
    )
    if not exist "%CARLA_DEPENDENCIES_FOLDER%lib" (
        mkdir "%CARLA_DEPENDENCIES_FOLDER%lib"
    )
    if not exist "%CARLA_DEPENDENCIES_FOLDER%dll" (
        mkdir "%CARLA_DEPENDENCIES_FOLDER%dll"
    )
    echo "%INSTALLATION_DIR%chrono-install\include\*" "%CARLA_DEPENDENCIES_FOLDER%include\*" > NUL
    xcopy /Y /S /I "%INSTALLATION_DIR%chrono-install\include\*" "%CARLA_DEPENDENCIES_FOLDER%include\*" > NUL
    copy "%INSTALLATION_DIR%chrono-install\lib\*.lib" "%CARLA_DEPENDENCIES_FOLDER%lib\*.lib" > NUL
    copy "%INSTALLATION_DIR%chrono-install\bin\*.dll" "%CARLA_DEPENDENCIES_FOLDER%dll\*.dll" > NUL
    xcopy /Y /S /I "%INSTALLATION_DIR%eigen-install\include\*" "%CARLA_DEPENDENCIES_FOLDER%include\*" > NUL
)

REM ==============================================================================
REM -- 下载Fast DDS及相关依赖 --------------------------------------------------------
REM ==============================================================================

SET FASTDDS_BASENAME=fast-dds
SET FASTDDS_INSTALL_DIR=%CD%\%FASTDDS_BASENAME%-install
SET FASTDDS_INCLUDE=%FASTDDS_INSTALL_DIR%\include
SET FASTDDS_LIB=%FASTDDS_INSTALL_DIR%\lib
IF "%USE_ROS2%"=="true" (

  :build_fastdds_extension
  SET LIB_SOURCE=%1
  SET LIB_REPO=%2
  SET CMAKE_FLAGS=%3

  IF NOT EXIST "%LIB_SOURCE%" (
    mkdir "%LIB_SOURCE%"
    echo %LIB_REPO%
    git clone %LIB_REPO% %LIB_SOURCE%
    mkdir "%LIB_SOURCE%\build"
  )

  IF NOT EXIST "%FASTDDS_INSTALL_DIR%" (
    mkdir "%FASTDDS_INSTALL_DIR%"
    echo Build foonathan memory vendor
    SET FOONATHAN_MEMORY_VENDOR_BASENAME=foonathan-memory-vendor
    SET FOONATHAN_MEMORY_VENDOR_SOURCE_DIR=%CD%\%FOONATHAN_MEMORY_VENDOR_BASENAME%-source
    SET FOONATHAN_MEMORY_VENDOR_REPO="https://github.com/eProsima/foonathan_memory_vendor.git"
    SET FOONATHAN_MEMORY_VENDOR_CMAKE_FLAGS=-DBUILD_SHARED_LIBS=ON
    CALL :build_fastdds_extension "%FOONATHAN_MEMORY_VENDOR_SOURCE_DIR%" "%FOONATHAN_MEMORY_VENDOR_REPO%"
    pushd "%FOONATHAN_MEMORY_VENDOR_SOURCE_DIR%\build" >nul
    cmake -G "Ninja" ^
      -DCMAKE_INSTALL_PREFIX="%FASTDDS_INSTALL_DIR%" ^
      -DBUILD_SHARED_LIBS=ON ^
      -DCMAKE_CXX_FLAGS_RELEASE="-D_GLIBCXX_USE_CXX11_ABI=0" ^
      -DFOONATHAN_MEMORY_FORCE_VENDORED_BUILD=ON ^
     ..
    ninja
    ninja install
    popd >nul
    rmdir /s /q "%FOONATHAN_MEMORY_VENDOR_SOURCE_DIR%"

    echo Build fast cdr
    SET FAST_CDR_BASENAME=fast-cdr
    SET FAST_CDR_SOURCE_DIR=%CD%\%FAST_CDR_BASENAME%-source
    SET FAST_CDR_REPO="https://github.com/eProsima/Fast-CDR.git"
    CALL :build_fastdds_extension "%FAST_CDR_SOURCE_DIR%" "%FAST_CDR_REPO%"
    pushd "%FAST_CDR_SOURCE_DIR%\build" >nul
    cmake -G "Ninja" ^
      -DCMAKE_INSTALL_PREFIX="%FASTDDS_INSTALL_DIR%" ^
      -DCMAKE_CXX_FLAGS_RELEASE="-D_GLIBCXX_USE_CXX11_ABI=0" ^
     ..
    ninja
    ninja install
    popd >nul
    rmdir /s /q "%FAST_CDR_SOURCE_DIR%"

    echo Build fast dds
    SET FAST_DDS_LIB_BASENAME=fast-dds-lib
    SET FAST_DDS_LIB_SOURCE_DIR=%CD%\%FAST_DDS_LIB_BASENAME%-source
    SET FAST_DDS_LIB_REPO="https://github.com/eProsima/Fast-DDS.git"
    CALL :build_fastdds_extension "%FAST_DDS_LIB_SOURCE_DIR%" "%FAST_DDS_LIB_REPO%"
    pushd "%FAST_DDS_LIB_SOURCE_DIR%\build" >nul
    cmake -G "Ninja" ^
      -DCMAKE_INSTALL_PREFIX="%FASTDDS_INSTALL_DIR%" ^
      -DCMAKE_CXX_FLAGS=-latomic ^
      -DCMAKE_CXX_FLAGS_RELEASE="-D_GLIBCXX_USE_CXX11_ABI=0" ^
     ..
    ninja
    ninja install
    popd >nul
    rmdir /
  )
)

rem ============================================================================
rem -- 添加新的编译依赖：mylib相关操作开始 -----------------------------------------------------
rem 以下是假设添加名为mylib的新库依赖，此处需根据实际库的获取方式进行调整，比如从官网下载、git克隆等
rem 示例中先创建存放该库的文件夹
rem ============================================================================

if not exist "%INSTALLATION_DIR%mylib" (
    echo %FILE_N% Creating "%INSTALLATION_DIR%mylib" folder...
    mkdir "%INSTALLATION_DIR%mylib"
)

rem 假设mylib库有对应的下载脚本install_mylib.bat，调用该脚本来下载并安装库（实际中需替换为真实有效的下载安装方式）
echo %FILE_N% Installing mylib...
call "%INSTALLERS_DIR%install_mylib.bat"^
 --build-dir "%INSTALLATION_DIR%mylib"

if %errorlevel% neq 0 goto failed

rem 获取安装后的mylib库相关路径（这里假设安装后有include和lib等目录，需根据实际库的结构调整）
set MYLIB_INCLUDE_DIR=%INSTALLATION_DIR%mylib\include
set MYLIB_LIB_DIR=%INSTALLATION_DIR%mylib\lib

rem 将mylib的头文件复制到项目依赖的头文件目录（这里假设是CARLA_DEPENDENCIES_FOLDER下的include目录，需根据实际项目结构调整）
xcopy /Y /S /I "%MYLIB_INCLUDE_DIR%\*" "%CARLA_DEPENDENCIES_FOLDER%include\*" > NUL

rem 将mylib的库文件复制到项目依赖的库文件目录（这里假设是CARLA_DEPENDENCIES_FOLDER下的lib目录，需根据实际项目结构调整）
copy "%MYLIB_LIB_DIR%\*.lib" "%CARLA_DEPENDENCIES_FOLDER%lib\*.lib" > NUL

rem 在生成的CMake配置文件中添加mylib相关的配置信息（以下是示例，需根据实际情况和项目构建要求调整）
rem ============================================================================
rem -- 生成CMake相关配置，添加mylib配置信息部分 ------------------------------------------
rem ============================================================================

for /f %%i in ('git describe --tags --dirty --always') do set carla_version=%%i
