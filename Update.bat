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

rem 读取内容版本信息文件的最后一行
for /F "delims=" %%a in (%CONTENT_VERSIONS%) do set "lastLine=%%a"
rem 提取内容的唯一标识
set CONTENT_ID=%lastLine:~-16,16%
rem 构建下载链接
set CONTENT_LINK=https://carla-assets.s3.us-east-005.backblazeb2.com/%CONTENT_ID%.tar.gz

rem 检查内容标识前两位是否为"20"
if "%CONTENT_ID:~0,2%"=="20" (
    rem 设置下载文件的本地路径
    set CONTENT_FILE=%CONTENT_FOLDER%/%CONTENT_ID%.tar.gz
    rem 设置解压后的tar文件的本地路径
    set CONTENT_FILE_TAR=%CONTENT_FOLDER%/%CONTENT_ID%.tar
    rem 输出内容标识和下载链接
    echo %CONTENT_ID%
    echo %CONTENT_LINK%
) else (
    rem 如果内容标识不符合预期格式，输出错误信息
    echo Error reading the latest version from ContentVersions.txt, check last line of file %CONTENT_VERSIONS%'
    goto error_download
)

rem ============================================================================
rem -- Download the content ----------------------------------------------------
rem ============================================================================
rem 输出下载提示信息
echo Downloading "%CONTENT_LINK%"...
rem 使用PowerShell下载文件
powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%CONTENT_LINK%', '%CONTENT_FILE%')"
rem 如果下载失败，跳转到错误处理
if %errorlevel% neq 0 goto error_download

rem 输出解压提示信息
echo Extracting content from "%CONTENT_FILE%", this can take a while...
rem 判断7-Zip工具是否存在
if exist "%ProgramW6432%/7-Zip/7z.exe" (
    rem 使用7-Zip解压文件
    "%ProgramW6432%/7-Zip/7z.exe" x "%CONTENT_FILE%" -o"%CONTENT_FOLDER%" -y
    rem 如果解压失败，跳转到错误处理
    if %errorlevel% neq 0 goto error_download
    rem 删除下载的文件
    del %CONTENT_FILE:/=\%
    rem 解压tar文件
    "%ProgramW6432%/7-Zip/7z.exe" x "%CONTENT_FILE_TAR%" -o"%CONTENT_FOLDER%" -y
    rem 如果解压失败，跳转到错误处理
    if %errorlevel% neq 0 goto error_download
    rem 删除tar文件
    del %CONTENT_FILE_TAR:/=\%
) else (
    rem 如果7-Zip不存在，使用PowerShell解压文件
    powershell -Command "Expand-Archive '%CONTENT_FILE%' -DestinationPath '%CONTENT_FOLDER%'"
    rem 如果解压失败，跳转到错误处理
    if %errorlevel% neq 0 goto error_download
    rem 删除下载的文件
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
