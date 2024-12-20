@echo off

rem ============================================================================
rem -- Set up environment ------------------------------------------------------
rem ============================================================================
rem 获取当前脚本所在目录
set SCRIPT_DIR=%~dp0
rem 设置内容文件夹路径
set CONTENT_FOLDER=%SCRIPT_DIR%Unreal/CarlaUE4/Content/Carla
rem 设置版本文件路径
set VERSION_FILE=%CONTENT_FOLDER%/.version
rem 设置内容版本信息文件路径
set CONTENT_VERSIONS=%SCRIPT_DIR%/Util/ContentVersions.txt

rem ============================================================================
rem -- Get the last version to download ----------------------------------------
rem ============================================================================
rem 如果内容文件夹不存在，则创建该文件夹
if not exist "%CONTENT_FOLDER%" mkdir "%CONTENT_FOLDER%"
rem 读取内容版本信息文件的最后一行并保存
for /F "delims=" %%a in (%CONTENT_VERSIONS%) do set "lastLine=%%a"

rem 从最后一行提取内容的唯一标识符
set CONTENT_ID=%lastLine:~-16,16%

rem 构建内容的下载链接
set CONTENT_LINK=https://carla-assets.s3.us-east-005.backblazeb2.com/%CONTENT_ID%.tar.gz

rem 检查内容标识符是否符合预期格式
if "%CONTENT_ID:~0,2%"=="20" (
    rem 设置下载文件和解压后的tar文件的本地路径
    set CONTENT_FILE=%CONTENT_FOLDER%/%CONTENT_ID%.tar.gz
    set CONTENT_FILE_TAR=%CONTENT_FOLDER%/%CONTENT_ID%.tar
    rem 显示内容标识符和下载链接
    echo %CONTENT_ID%
    echo %CONTENT_LINK%
) else (
    rem 如果内容标识符不符合预期格式，显示错误信息并跳转到错误处理
    echo Error reading the latest version from ContentVersions.txt, check last line of file %CONTENT_VERSIONS%'
    goto error_download
)

rem ============================================================================
rem -- Download the content ----------------------------------------------------
rem ============================================================================

rem 显示下载提示信息
echo Downloading "%CONTENT_LINK%"...

rem 使用PowerShell下载文件
powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%CONTENT_LINK%', '%CONTENT_FILE%')"
rem 如果下载失败，跳转到错误处理
if %errorlevel% neq 0 goto error_download

rem 显示解压提示信息
echo Extracting content from "%CONTENT_FILE%", this can take a while...

rem 判断7-Zip工具是否存在并使用其解压文件
if exist "%ProgramW6432%/7-Zip/7z.exe" (
    "%ProgramW6432%/7-Zip/7z.exe" x "%CONTENT_FILE%" -o"%CONTENT_FOLDER%" -y
    if %errorlevel% neq 0 goto error_download
    del %CONTENT_FILE:/=\%
    "%ProgramW6432%/7-Zip/7z.exe" x "%CONTENT_FILE_TAR%" -o"%CONTENT_FOLDER%" -y
    if %errorlevel% neq 0 goto error_download
    del %CONTENT_FILE_TAR:/=\%
) else (
    rem 如果7-Zip不存在，使用PowerShell解压文件
    powershell -Command "Expand-Archive '%CONTENT_FILE%' -DestinationPath '%CONTENT_FOLDER%'"
    if %errorlevel% neq 0 goto error_download
    del %CONTENT_FILE%
)

rem 跳转到成功处理
goto success

:success
rem 输出成功信息
echo Content has been successfully installed in "%CONTENT_FOLDER%"!
goto good_exit

:error_download
rem 跳转到错误退出处理
goto bad_exit

:good_exit
rem 正常退出
echo Exiting...
endlocal
exit /b 0

:bad_exit
rem 如果内容文件存在，删除内容文件夹
if exist "%CONTENT_FILE%" rd /s /q "%CONTENT_FOLDER%"
rem 错误退出
echo Exiting with error...
endlocal
exit /b %errorlevel%
