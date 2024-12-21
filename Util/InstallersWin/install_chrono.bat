:arg-parse
if not "%1"=="" (
    if "%1"=="--build-dir" (
        set BUILD_DIR=%~dpn2
        shift
    )
    if "%1"=="-h" (
        goto help
    )
    if "%1"=="--help" (
        goto help
    )
    if "%1"=="--generator" (
        set GENERATOR=%2
        shift
    )
    shift
    goto :arg-parse
)

if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\
if %GENERATOR% == "" set GENERATOR="Visual Studio 16 2019"
set EIGEN_VERSION=3.3.7
set EIGEN_REPO=https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.zip
set EIGEN_BASENAME=eigen-%EIGEN_VERSION%

set EIGEN_SRC_DIR=%BUILD_DIR%%EIGEN_BASENAME%
set EIGEN_INSTALL_DIR=%BUILD_DIR%eigen-install
set EIGEN_INCLUDE=%EIGEN_INSTALL_DIR%\include
set EIGEN_TEMP_FILE=eigen-%EIGEN_VERSION%.zip
set EIGEN_TEMP_FILE_DIR=%BUILD_DIR%eigen-%EIGEN_VERSION%.zip
if not exist "%EIGEN_SRC_DIR%" (
    if not exist "%EIGEN_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %EIGEN_TEMP_FILE_DIR%.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%EIGEN_REPO%', '%EIGEN_TEMP_FILE_DIR%')"
    )
    if %errorlevel% neq 0 goto error_download_eigen
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

xcopy /q /Y /S /I "%EIGEN_SRC_DIR%\Eigen" "%EIGEN_INCLUDE%\Eigen"
xcopy /q /Y /S /I "%EIGEN_SRC_DIR%\unsupported\Eigen" "%EIGEN_INCLUDE%\unsupported\Eigen"
set CHRONO_VERSION=6.0.0
@REM set CHRONO_VERSION=develop
set CHRONO_REPO=https://github.com/projectchrono/chrono.git
set CHRONO_BASENAME=chrono

set CHRONO_SRC_DIR=%BUILD_DIR%%CHRONO_BASENAME%-src
set CHRONO_INSTALL_DIR=%BUILD_DIR%chrono-install
set CHRONO_BUILD_DIR=%CHRONO_SRC_DIR%\build
if not exist %CHRONO_INSTALL_DIR% (
    echo %FILE_N% Retrieving Chrono.
    call git clone --depth 1 --branch %CHRONO_VERSION% %CHRONO_REPO% %CHRONO_SRC_DIR%

    mkdir %CHRONO_BUILD_DIR%
    mkdir %CHRONO_INSTALL_DIR%

    cd "%CHRONO_BUILD_DIR%"

    echo.%GENERATOR% | findstr /C:"Visual Studio" >nul && (
        set PLATFORM=-A x64
    ) || (
        set PLATFORM=
    )

    echo %FILE_N% Compiling Chrono.
    cmake -G %GENERATOR% %PLATFORM%^
        -DCMAKE_BUILD_TYPE=Release^
        -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
        -DEIGEN3_INCLUDE_DIR="%EIGEN_INCLUDE%"^
        -DCMAKE_INSTALL_PREFIX="%CHRONO_INSTALL_DIR%"^
        -DENABLE_MODULE_VEHICLE=ON^
        %CHRONO_SRC_DIR%

    echo %FILE_N% Building...
    cmake --build. --config Release --target install

)
set NEWLIB_VERSION=1.0.0
set NEWLIB_REPO=https://github.com/someuser/newlib.git
set NEWLIB_BASENAME=newlib-%NEWLIB_VERSION%

set NEWLIB_SRC_DIR=%BUILD_DIR%%NEWLIB_BASENAME%
set NEWLIB_INSTALL_DIR=%BUILD_DIR%newlib-install
set NEWLIB_INCLUDE=%NEWLIB_INSTALL_DIR%\include
set NEWLIB_TEMP_FILE=newlib-%NEWLIB_VERSION%.zip
set NEWLIB_TEMP_FILE_DIR=%BUILD_DIR%newlib-%NEWLIB_VERSION%.zip
if not exist "%NEWLIB_SRC_DIR%" (
    if not exist "%NEWLIB_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %NEWLIB_TEMP_FILE_DIR%.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%NEWLIB_REPO%', '%NEWLIB_TEMP_FILE_DIR%')"
    )
    if %errorlevel% neq 0 goto error_download_newlib
    rem Extract the downloaded library
    echo %FILE_N% Extracting newlib from "%NEWLIB_TEMP_FILE%".
    powershell -Command "Expand-Archive '%NEWLIB_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%'"
    if %errorlevel% neq 0 goto error_extracting_newlib
    echo %NEWLIB_SRC_DIR%

    del %NEWLIB_TEMP_FILE_DIR%
)
if not exist "%NEWLIB_INSTALL_DIR%" (
    mkdir %NEWLIB_INSTALL_DIR%
    mkdir %NEWLIB_INCLUDE%
    mkdir %NEWLIB_INCLUDE%\subfolder1
    mkdir %NEWLIB_INCLUDE%\subfolder2
)

xcopy /q /Y /S /I "%NEWLIB_SRC_DIR%\include" "%NEWLIB_INCLUDE%"
cmake -G %GENERATOR% %PLATFORM%^
    -DCMAKE_BUILD_TYPE=Release^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DEIGEN3_INCLUDE_DIR="%EIGEN_INCLUDE%"^
    -DNewLib_INCLUDE_DIR="%NEWLIB_INCLUDE%"^
    -DCMAKE_INSTALL_PREFIX="%CHRONO_INSTALL_DIR%"^
    -DENABLE_MODULE_VEHICLE=ON^
    -DLINK_LIBRARIES="%NEWLIB_INSTALL_DIR%\lib\newlib.lib"^
    %CHRONO_SRC_DIR%
cmake -G %GENERATOR% %PLATFORM%^
    -DCMAKE_BUILD_TYPE=Release^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DEIGEN3_INCLUDE_DIR="%EIGEN_INCLUDE%"^
    -DCMAKE_INSTALL_PREFIX="%CHRONO_INSTALL_DIR%"^
    -DENABLE_MODULE_VEHICLE=ON^
    -DZLIB_INCLUDE_DIR="C:/path/to/zlib/include"^
    -DZLIB_LIBRARY="C:/path/to/zlib/lib/zlib.lib"^
    %CHRONO_SRC_DIR%
