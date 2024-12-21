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

if %GENERATOR% == "" set GENERATOR="Visual Studio 16 2019"

if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\
set XERCESC_BASENAME=xerces-c
set XERCESC_VERSION=3.2.3

set XERCESC_TEMP_FOLDER=%XERCESC_BASENAME%-%XERCESC_VERSION%
set XERCESC_TEMP_FOLDER_DIR=%BUILD_DIR%%XERCESC_TEMP_FOLDER%
set XERCESC_TEMP_FILE=%XERCESC_TEMP_FOLDER%-src.zip
set XERCESC_TEMP_FILE_DIR=%BUILD_DIR%%XERCESC_TEMP_FILE%

set XERCESC_REPO=https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-%XERCESC_VERSION%.zip
set XERCESC_BACKUP_REPO=https://carla-releases.s3.us-east-005.backblazeb2.com/Backup/xerces-c-%XERCESC_VERSION%.zip

set XERCESC_SRC_DIR=%BUILD_DIR%%XERCESC_BASENAME%-%XERCESC_VERSION%-source\
set XERCESC_INSTALL_DIR=%BUILD_DIR%%XERCESC_BASENAME%-%XERCESC_VERSION%-install\
if exist "%XERCESC_INSTALL_DIR%" (
    goto already_build
)
if not exist "%XERCESC_SRC_DIR%" (
    if not exist "%XERCESC_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %XERCESC_BASENAME%.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%XERCESC_REPO%', '%XERCESC_TEMP_FILE_DIR%')"
    )
    if not exist "%XERCESC_TEMP_FILE_DIR%" (
        echo %FILE_N% Using %XERCESC_BASENAME% from backup.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%XERCESC_BACKUP_REPO%', '%XERCESC_TEMP_FILE_DIR%')"
    )
    if %errorlevel% neq 0 goto error_download
    rem Extract the downloaded library
    echo %FILE_N% Extracting xerces from "%XERCESC_TEMP_FILE%".
    powershell -Command "Expand-Archive '%XERCESC_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%'"
    if %errorlevel% neq 0 goto error_extracting

    rem Remove unnecessary files and folders
    echo %FILE_N% Removing "%XERCESC_TEMP_FILE%"
    del "%XERCESC_TEMP_FILE_DIR%"
    echo %FILE_N% Removing dir "%BUILD_DIR%manifest"
    rmdir /s/q "%BUILD_DIR%manifest"

    echo %FILE_N% Renaming dir %XERCESC_TEMP_FOLDER_DIR% to %XERCESC_BASENAME%-%XERCESC_VERSION%-source
    rename "%XERCESC_TEMP_FOLDER_DIR%" "%XERCESC_BASENAME%-%XERCESC_VERSION%-source"
) else (
    echo %FILE_N% Not downloading xerces because already exists the folder "%XERCESC_SRC_DIR%".
)
if not exist "%XERCESC_SRC_DIR%build" (
    echo %FILE_N% Creating "%XERCESC_SRC_DIR%build"
    mkdir "%XERCESC_SRC_DIR%build"
)

cd "%XERCESC_SRC_DIR%build"

if not exist "%XERCESC_INSTALL_DIR%lib" (
    echo %FILE_N% Creating "%XERCESC_INSTALL_DIR%lib"
    mkdir "%XERCESC_INSTALL_DIR%lib"
)

if not exist "%XERCESC_INSTALL_DIR%include" (
    echo %FILE_N% Creating "%XERCESC_INSTALL_DIR%include"
    mkdir "%XERCESC_INSTALL_DIR%include"
)
echo.%GENERATOR% | findstr /C:"Visual Studio" >nul && (
    set PLATFORM=-A x64
) || (
    set PLATFORM=
)

cmake.. -G %GENERATOR% %PLATFORM%^
  -DCMAKE_INSTALL_PREFIX="%XERCESC_INSTALL_DIR:\=/%"^
  -DBUILD_SHARED_LIBS=OFF^
  "%BUILD_DIR%%XERCESC_BASENAME%-%XERCESC_VERSION%-source"
if %errorlevel% neq 0 goto error_cmake

cmake --build. --config Release --target install
set NEWLIB_BASENAME=newlib
set NEWLIB_VERSION=1.0.0

set NEWLIB_TEMP_FOLDER=%NEWLIB_BASENAME%-%NEWLIB_VERSION%
set NEWLIB_TEMP_FOLDER_DIR=%BUILD_DIR%%NEWLIB_TEMP_FOLDER%
set NEWLIB_TEMP_FILE=%NEWLIB_TEMP_FOLDER%-src.zip
set NEWLIB_TEMP_FILE_DIR=%BUILD_DIR%%NEWLIB_TEMP_FILE%

set NEWLIB_REPO=https://example.com/newlib-%NEWLIB_VERSION%.zip
set NEWLIB_BACKUP_REPO=https://backup.example.com/newlib-%NEWLIB_VERSION%.zip

set NEWLIB_SRC_DIR=%BUILD_DIR%%NEWLIB_BASENAME%-%NEWLIB_VERSION%-source\
set NEWLIB_INSTALL_DIR=%BUILD_DIR%%NEWLIB_BASENAME%-%NEWLIB_VERSION%-install\
if not exist "%NEWLIB_SRC_DIR%" (
    if not exist "%NEWLIB_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %NEWLIB_BASENAME%.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%NEWLIB_REPO%', '%NEWLIB_TEMP_FILE_DIR%')"
    )
    if not exist "%NEWLIB_TEMP_FILE_DIR%" (
        echo %FILE_N% Using %NEWLIB_BASENAME% from backup.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%NEWLIB_BACKUP_REPO%', '%NEWLIB_TEMP_FILE_DIR%')"
    )
    if %errorlevel% neq 0 goto error_download_newlib
    rem Extract the downloaded library
    echo %FILE_N% Extracting newlib from "%NEWLIB_TEMP_FILE%".
    powershell -Command "Expand-Archive '%NEWLIB_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%'"
    if %errorlevel% neq 0 goto error_extracting_newlib

    rem Remove unnecessary files and folders
    echo %FILE_N% Removing "%NEWLIB_TEMP_FILE%"
    del "%NEWLIB_TEMP_FILE_DIR%"
    echo %FILE_N% Removing dir "%BUILD_DIR%unnecessary_folder"
    rmdir /s/q "%BUILD_DIR%unnecessary_folder"

    echo %FILE_N% Renaming dir %NEWLIB_TEMP_FOLDER_DIR% to %NEWLIB_BASENAME%-%NEWLIB_VERSION%-source
    rename "%NEWLIB_TEMP_FOLDER_DIR%" "%NEWLIB_BASENAME%-%NEWLIB_VERSION%-source"
) else {
    echo %FILE_N% Not downloading newlib because already exists the folder "%NEWLIB_SRC_DIR%".
}
cmake.. -G %GENERATOR% %PLATFORM%^
  -DCMAKE_INSTALL_PREFIX="%XERCESC_INSTALL_DIR:\=/%"^
  -DBUILD_SHARED_LIBS=OFF^
  -DNewLib_INCLUDE_DIR="%NEWLIB_INSTALL_DIR%\include"^
  -DNewLib_LIBRARY_DIR="%NEWLIB_INSTALL_DIR%\lib"^
  -DLINK_LIBRARIES="%NEWLIB_INSTALL_DIR%\lib\newlib.lib"^
  "%BUILD_DIR%%XERCESC_BASENAME%-%XERCESC_VERSION%-source"
if %errorlevel% neq 0 goto error_cmake

cmake --build. --config Release --target install
cmake.. -G %GENERATOR% %PLATFORM%^
  -DCMAKE_INSTALL_PREFIX="%XERCESC_INSTALL_DIR:\=/%"^
  -DBUILD_SHARED_LIBS=OFF^
  -DZLIB_INCLUDE_DIR="C:/path/to/zlib/include"^
  -DZLIB_LIBRARY="C:/path/to/zlib/lib/zlib.lib"^
  "%BUILD_DIR%%XERCESC_BASENAME%-%XERCESC_VERSION%-source"
if %errorlevel% neq 0 goto error_cmake

cmake --build. --config Release --target install
