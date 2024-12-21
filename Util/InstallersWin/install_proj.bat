rem -- 假设添加zlib依赖相关变量定义 --
set ZLIB_BASE_NAME=zlib-1.2.11  ;根据实际下载的版本调整名称
set ZLIB_ZIP=%ZLIB_BASE_NAME%.zip
set ZLIB_ZIP_DIR=%BUILD_DIR%%ZLIB_ZIP%
set ZLIB_REPO=https://zlib.net/%ZLIB_ZIP%

set ZLIB_SRC_DIR=%BUILD_DIR%zlib-src
set ZLIB_BUILD_DIR=%ZLIB_SRC_DIR%\build
set ZLIB_INSTALL_DIR=%BUILD_DIR%zlib-install

set ZLIB_INCLUDE_DIR=%ZLIB_INSTALL_DIR%\include
set ZLIB_LIB_DIR=%ZLIB_INSTALL_DIR%\lib
rem -- 检查并下载zlib文件（如果不存在）--
if not exist "%ZLIB_INSTALL_DIR%" (
    echo %FILE_N% Retrieving %ZLIB_BASE_NAME%.
    powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%ZLIB_REPO%', '%ZLIB_ZIP_DIR%')"
    if %errorlevel% neq 0 goto error_download_zlib

    cd %BUILD_DIR%
    echo %FILE_N% Extracting ZLIB from "%ZLIB_ZIP%".
    call tar -xzf %ZLIB_ZIP%
    move %BUILD_DIR%%ZLIB_BASE_NAME% %ZLIB_SRC_DIR%
)

rem -- 错误处理标签（下载zlib出错）--
:error_download_zlib
    echo.
    echo %FILE_N% [DOWNLOAD ERROR] An error ocurred while downloading ZLIB.
    echo %FILE_N% [DOWNLOAD ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%ZLIB_REPO%"
    echo %FILE_N% [DOWNLOAD ERROR] Workaround:
    echo %FILE_N%              - Download the ZLIB's source code and
    echo %FILE_N%                extract the content in
    echo %FILE_N%                "%ZLIB_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit
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
    -DZLIB_INCLUDE_DIR=%ZLIB_INCLUDE_DIR% -DZLIB_LIBRARY=%ZLIB_LIB_DIR%\zlib.lib  ;添加对zlib头文件目录和库文件的配置
if %errorlevel% neq 0 goto error_cmake
