@echo off

rem ============================================================================
rem -- Set up environment ------------------------------------------------------
rem ============================================================================
rem 设置脚本所在目录，%~dp0表示获取当前批处理脚本所在的驱动器和路径
set SCRIPT_DIR=%~dp0
rem 设置内容文件夹路径，指向Unreal/CarlaUE4/Content/Carla这个目录，基于脚本所在目录
set CONTENT_FOLDER=%SCRIPT_DIR%Unreal/CarlaUE4/Content/Carla
rem 设置版本文件的路径，位于内容文件夹下的.version文件
set VERSION_FILE=%CONTENT_FOLDER%/.version
rem 设置内容版本信息文件的路径，用于获取版本相关内容
set CONTENT_VERSIONS=%SCRIPT_DIR%/Util/ContentVersions.txt

rem ============================================================================
rem -- Get the last version to download ----------------------------------------
rem ============================================================================
rem 如果内容文件夹不存在，则创建该文件夹
if not exist "%CONTENT_FOLDER%" mkdir "%CONTENT_FOLDER%"

rem 逐行读取内容版本信息文件（ContentVersions.txt），将每行内容依次赋给%%a变量
for /F "delims=" %%a in (%CONTENT_VERSIONS%) do (
    rem 将当前读取到的行内容保存到lastLine变量中，后续用于提取相关信息
    set "lastLine=%%a"
)
rem 从lastLine变量中提取特定长度（从倒数第16位开始，长度为16位）的字符串作为内容的唯一标识（CONTENT_ID）
set CONTENT_ID=%lastLine:~-16,16%
rem 根据内容唯一标识构建下载链接（CONTENT_LINK），用于从指定的服务器地址下载对应的内容压缩包
set CONTENT_LINK=https://carla-assets.s3.us-east-005.backblazeb2.com/%CONTENT_ID%.tar.gz
rem 判断内容唯一标识的前两位是否为"20"，以此来区分不同的版本格式或者进行相关处理逻辑判断
if "%CONTENT_ID:~0,2%"=="20" (
    rem 设置下载的内容文件在本地保存的完整路径（包含文件名和扩展名）
    set CONTENT_FILE=%CONTENT_FOLDER%/%CONTENT_ID%.tar.gz
    rem 设置解压后的tar文件在本地保存的完整路径（包含文件名和扩展名），后续可能用于进一步解压操作
    set CONTENT_FILE_TAR=%CONTENT_FOLDER%/%CONTENT_ID%.tar
    rem 输出内容唯一标识，可用于调试或者查看当前获取到的版本信息
    echo %CONTENT_ID%
    rem 输出内容下载链接，同样可用于调试或者确认下载地址是否正确
    echo %CONTENT_LINK%
) else (
    rem 如果内容唯一标识不符合预期格式，输出错误提示信息，告知用户检查内容版本信息文件的最后一行内容是否正确
    echo Error reading the latest version from ContentVersions.txt, check last line of file %CONTENT_VERSIONS%'
    rem 跳转到error_download标签处，执行错误处理相关逻辑
    goto error_download
)

rem ============================================================================
rem -- Download the content ----------------------------------------------------
rem ============================================================================
rem 输出提示信息，告知用户即将开始下载文件，显示下载的链接地址
echo Downloading "%CONTENT_LINK%"...
rem 使用PowerShell命令，通过WebClient对象来下载文件，从指定的下载链接（CONTENT_LINK）下载到本地指定路径（CONTENT_FILE）
powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%CONTENT_LINK%', '%CONTENT_FILE%')"
rem 判断上一步下载操作的返回错误码（errorlevel），如果不等于0表示下载出现错误，跳转到error_download标签处进行错误处理
if %errorlevel% neq 0 goto error_download

rem 输出提示信息，告知用户即将开始解压文件，显示要解压的文件路径，提示可能需要花费一些时间
echo %FILE_N% Extracting content from "%CONTENT_FILE%", this can take a while...
rem 判断64位系统下7-Zip解压工具（7z.exe）是否存在，如果存在则使用7-Zip进行解压操作
if exist "%ProgramW6432%/7-Zip/7z.exe" (
    rem 使用7-Zip解压下载的压缩文件（CONTENT_FILE）到指定的内容文件夹（CONTENT_FOLDER），-y表示自动确认覆盖等操作，不进行提示
    "%ProgramW6432%/7-Zip/7z.exe" x "%CONTENT_FILE%" -o"%CONTENT_FOLDER%" -y
    rem 再次判断操作的返回错误码，如果解压过程出现错误，跳转到error_download标签处进行错误处理
    if %errorlevel% neq 0 goto error_download
    rem 输出提示信息，显示即将删除的下载文件路径（将路径中的斜杠替换为反斜杠，以符合Windows文件路径格式要求）
    echo Deleting %CONTENT_FILE:/=\%
    rem 删除下载的文件，注意这里同样对路径格式进行了处理
    del %CONTENT_FILE:/=\%
    rem 使用7-Zip解压之前解压出来的tar文件（CONTENT_FILE_TAR）到指定的内容文件夹（CONTENT_FOLDER）
    "%ProgramW6432%/7-Zip/7z.exe" x "%CONTENT_FILE_TAR%" -o"%CONTENT_FOLDER%" -y
    rem 判断解压tar文件操作的返回错误码，如果出现错误，跳转到error_download标签处进行错误处理
    if %errorlevel% neq 0 goto error_download

    rem 输出提示信息，显示即将删除的tar文件路径（处理路径格式）
    echo Deleting %CONTENT_FILE_TAR:/=\%
    rem 删除tar文件
    del %CONTENT_FILE_TAR:/=\%
) else (
    rem 如果7-Zip工具不存在，则使用PowerShell的Expand-Archive命令来解压文件，将下载的文件解压到指定的内容文件夹
    powershell -Command "Expand-Archive '%CONTENT_FILE%' -DestinationPath '%CONTENT_FOLDER%'"
    rem 判断解压操作的返回错误码，如果出现错误，跳转到error_download标签处进行错误处理
    if %errorlevel% neq 0 goto error_download
    rem 删除下载的文件
    del %CONTENT_FILE%
)

rem 跳转到success标签处，表示文件下载和解压等操作成功完成，执行后续成功相关的处理逻辑
goto success

:success
    rem 输出空行，用于格式排版，使输出信息更清晰
    echo.
    rem 输出成功提示信息，告知用户内容已经成功安装到指定的内容文件夹中
    echo %FILE_N% Content has been successfully installed in "%CONTENT_FOLDER%"!
    rem 跳转到good_exit标签处，执行正常退出相关的操作
    goto good_exit

:error_download
    rem 跳转到bad_exit标签处，执行错误情况下的退出相关操作
    goto bad_exit

:good_exit
    rem 输出提示信息，告知用户即将退出程序
    echo %FILE_N% Exiting...
    rem 结束批处理文件中设置的局部环境变量，释放相关资源
    endlocal
    rem 以返回码0正常退出批处理程序，表示程序执行成功完成
    exit /b 0

:bad_exit
    rem 如果下载的文件（CONTENT_FILE）存在，说明可能出现了错误情况，删除整个内容文件夹及其所有内容（/s表示删除所有子目录和文件，/q表示安静模式，不进行确认提示）
    if exist "%CONTENT_FILE%" rd /s /q "%CONTENT_FOLDER%"
    rem 输出提示信息，告知用户程序将带着错误状态退出
    echo %FILE_N% Exiting with error...
    rem 结束批处理文件中设置的局部环境变量，释放相关资源
    endlocal
    rem 以当前的错误返回码（errorlevel）退出批处理程序，表示程序执行出现错误
    exit /b %errorlevel%
