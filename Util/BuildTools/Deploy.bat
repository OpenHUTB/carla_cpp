@echo off
setlocal enabledelayedexpansion

rem ==============================================================================
rem -- Set up environment --------------------------------------------------------
rem ==============================================================================

set REPLACE_LATEST=true
set AWS_COPY=aws s3 cp
#定义用于执行AWS S3复制操作的命令字符串
rem ==============================================================================
rem -- Parse arguments -----------------------------------------------------------
rem ==============================================================================

set DOC_STRING=Upload latest build to S3
#定义文档字符串，用于描述脚本的主要功能，即上传最新构建到S3
set USAGE_STRING="Usage: $0 [-h|--help] [--replace-latest] [--dry-run]"
#定义使用说明字符串，展示脚本的正确使用方式
:arg-parse
if not "%1"=="" (
    if "%1"=="--replace-latest" (
        set REPLACE_LATEST=true
    )

    if "%1"=="--dry-run" (
      set AWS_COPY=rem aws s3 cp
    )

    if "%1"=="--help" (
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
    )

    shift
    goto :arg-parse
)

rem Get repository version
for /f %%i in ('git describe --tags --dirty --always') do set REPOSITORY_TAG=%%i
if not defined REPOSITORY_TAG goto error_carla_version
echo REPOSITORY_TAG = !REPOSITORY_TAG!
#如果版本号变量未定义，跳转到error_carla_version标签处处理错误情况
rem Last package data
set CARLA_DIST_FOLDER=%~dp0%\Build\UE4Carla
set PACKAGE=CARLA_%REPOSITORY_TAG%.zip
set PACKAGE_PATH=%CARLA_DIST_FOLDER%\%PACKAGE%
set PACKAGE2=AdditionalMaps_%REPOSITORY_TAG%.zip
set PACKAGE_PATH2=%CARLA_DIST_FOLDER%\%PACKAGE2%

set S3_PREFIX=s3://carla-releases/Windows

set LATEST_DEPLOY_URI=!S3_PREFIX!/Dev/CARLA_Latest.zip
set LATEST_DEPLOY_URI2=!S3_PREFIX!/Dev/AdditionalMaps_Latest.zip

rem 检查TAG版本是否符合数字加点的格式（通常是版本号）
echo %REPOSITORY_TAG% | findstr /R /C:"^[0-9]*\.[0-9]*\.[0-9]*.$" 1>nul

rem 如果上一条命令成功执行（即找到了匹配的TAG），则%errorlevel%为0
if %errorlevel% == 0 (
    rem 打印出检测到发布版本的信息
    echo Detected release version with tag %REPOSITORY_TAG%
    
    rem 设置发布包和附加地图包的名字
    set DEPLOY_NAME=CARLA_%REPOSITORY_TAG%.zip
    set DEPLOY_NAME2=AdditionalMaps_%REPOSITORY_TAG%.zip
) else (
    rem 如果没有找到匹配的TAG，则打印出检测到非发布版本的信息
    echo Detected non-release version with tag %REPOSITORY_TAG%
    
    rem 设置S3前缀为开发版本路径
    set S3_PREFIX=!S3_PREFIX!/Dev
    
    rem 获取最近的提交日期和哈希值，并设置部署包的名字
    git log --pretty=format:%%cd_%%h --date=format:%%Y%%m%%d -n 1 > tempo1234
    set /p DEPLOY_NAME= < tempo1234
    del tempo1234
    set DEPLOY_NAME=!DEPLOY_NAME!.zip
    echo deploy name = !DEPLOY_NAME!
    
    rem 获取最近的提交哈希值，并设置附加地图包的名字
    git log --pretty=format:%%h -n 1 > tempo1234
    set /p DEPLOY_NAME2= < tempo1234
    del tempo1234
    set DEPLOY_NAME2=AdditionalMaps_!DEPLOY_NAME2!.zip
    echo deploy name2 = !DEPLOY_NAME2!
)

rem 打印出检测到的版本标签
echo Version detected: %REPOSITORY_TAG%

rem 打印出将要使用的包名
echo Using package %PACKAGE% as %DEPLOY_NAME%

rem 检查包路径是否存在，如果不存在则跳转到错误处理标签（该标签在代码段外定义）
if not exist "%PACKAGE_PATH%" (
    echo Latest package not found, please run 'make package'
    goto :bad_exit
)
rem ==============================================================================
rem -- Upload --------------------------------------------------------------------
rem ==============================================================================

set DEPLOY_URI=!S3_PREFIX!/%DEPLOY_NAME%
%AWS_COPY% %PACKAGE_PATH% %DEPLOY_URI%
echo Latest build uploaded to %DEPLOY_URI%

set DEPLOY_URI2=!S3_PREFIX!/%DEPLOY_NAME2%
%AWS_COPY% %PACKAGE_PATH2% %DEPLOY_URI2%
echo Latest build uploaded to %DEPLOY_URI2%

rem ==============================================================================
rem -- Replace Latest ------------------------------------------------------------
rem ==============================================================================

if %REPLACE_LATEST%==true (
  %AWS_COPY% %DEPLOY_URI% %LATEST_DEPLOY_URI%
  echo Latest build updated as %LATEST_DEPLOY_URI%
  %AWS_COPY% %DEPLOY_URI2% %LATEST_DEPLOY_URI2%
  echo Latest build updated as %LATEST_DEPLOY_URI2%
)

rem ==============================================================================
rem -- ...and we are done --------------------------------------------------------
rem ==============================================================================

echo Success!

:success
    echo.
    goto good_exit
#成功结束的标签，输出空行后跳转到good_exit标签处结束脚本并返回成功码0
:error_carla_version
    echo.
    echo %FILE_N% [ERROR] Carla Version is not set
    goto bad_exit
#处理Carla版本未设置的错误情况，输出错误信息后跳转到bad_exit标签处结束脚本并返回错误码1
:good_exit
    endlocal
    exit /b 0
#正常结束脚本的标签，结束局部变量作用域并以成功码0退出脚本
:bad_exit
    endlocal
    exit /b 1
#错误结束脚本的标签，结束局部变量作用域并以错误码1退出脚本
