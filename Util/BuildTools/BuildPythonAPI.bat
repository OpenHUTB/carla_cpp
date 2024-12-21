@echo off
setlocal

rem 这是一个BAT脚本，用于创建LibCarla（carla.org）的客户端Python API。
rem 请在启用了x64 Visual C++ 工具集的cmd中运行它。

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0]:

rem 打印批处理参数（调试目的）
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- 解析参数 ---------------------------------------------------------------
rem ============================================================================

set DOC_STRING=Build and package CARLA Python API.
set "USAGE_STRING=Usage: %FILE_N% [-h^|--help] [--rebuild]  [--clean]"

set REMOVE_INTERMEDIATE=false
set BUILD_FOR_PYTHON2=false
set BUILD_FOR_PYTHON3=false

:arg-parse
if not "%1"=="" (
    if "%1"=="--rebuild" (
        set REMOVE_INTERMEDIATE=true
        rem 我们目前不支持py2
        set BUILD_FOR_PYTHON2=false
        set BUILD_FOR_PYTHON3=true
    )

    if "%1"=="--py2" (
        set BUILD_FOR_PYTHON2=true
    )

    if "%1"=="--py3" (
        set BUILD_FOR_PYTHON3=true
    )

    if "%1"=="--clean" (
        set REMOVE_INTERMEDIATE=true
    )

    if "%1"=="-h" (
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
    )

    if "%1"=="--help" (
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
    )

    shift
    goto :arg-parse
)

set PYTHON_LIB_PATH=%ROOT_PATH:/=\%PythonAPI\carla\

rem 如果没有设置删除中间文件，且没有选择为Python3或Python2构建，则输出没有选择任何操作并退出。
if %REMOVE_INTERMEDIATE% == false (
    if %BUILD_FOR_PYTHON3% == false (
        if %BUILD_FOR_PYTHON2% == false (
          echo 没有选择任何要执行的操作。
          goto :eof
        )
    )
)

if %REMOVE_INTERMEDIATE% == true (
    rem 删除目录
    for %%G in (
        "%PYTHON_LIB_PATH%build",
        "%PYTHON_LIB_PATH%dist",
        "%PYTHON_LIB_PATH%source\carla.egg-info"
    ) do (
        if exist %%G (
           echo %FILE_N% Cleaning %%G
            rmdir /s/q %%G
        )
    )
    
    rem 如果没有为Python3或Python2构建，则跳转到良好退出
    if %BUILD_FOR_PYTHON3% == false (
        if %BUILD_FOR_PYTHON2% == false (
            goto good_exit
        )
    )
)

cd "%PYTHON_LIB_PATH%"
rem 如果已存在"%PYTHON_LIB_PATH%dist"，则跳转到已安装（此部分被注释掉了，不会执行）

rem ============================================================================
rem -- 检查py命令 ------------------------------------------------------------
rem ============================================================================

where py 1>nul
if %errorlevel% neq 0 goto error_py

rem 为Python 2构建（但此部分被下面的条件直接跳转到不支持Python 2的标签）
rem
if %BUILD_FOR_PYTHON2%==true (
    goto py2_not_supported
)

rem 为Python 3构建
rem
if %BUILD_FOR_PYTHON3%==true (
    echo Building Python API for Python 3.
    python setup.py bdist_egg bdist_wheel
    if %errorlevel% neq 0 goto error_build_wheel
    
    rem 构建成功后，可以添加一些后续操作或检查的注释
    rem 例如：检查生成的wheel文件是否存在于"%PYTHON_LIB_PATH%dist"
)

goto success

rem ============================================================================
rem -- 消息和错误 -------------------------------------------------------------
rem ============================================================================

:success
    echo.
     if %BUILD_FOR_PYTHON3%==true echo %FILE_N% Carla lib for python has been successfully installed in "%PYTHON_LIB_PATH%dist"!
    goto good_exit

:already_installed （此标签对应的代码被注释掉了，不会执行）
    echo.
    echo %FILE_N% [ERROR] Already installed in "%PYTHON_LIB_PATH%dist"
    goto good_exit

:py2_not_supported
    echo.
   echo %FILE_N% [ERROR] Python 2 is not currently suported in Windows.
    goto bad_exit

:error_py
    echo.
    echo %FILE_N% [ERROR] 执行py时出错。
    echo %FILE_N% [ERROR] 可能的原因：
    echo %FILE_N% [ERROR]  - 确保已安装"py"。
    echo %FILE_N% [ERROR]  - py是Python启动器。此实用程序随Python安装一起提供，但不是默认安装的。
    echo %FILE_N% [ERROR]  - 确保在Windows上可用"py"。
    goto bad_exit

:error_build_wheel
     echo.
    echo %FILE_N% [ERROR] An error occurred while building the wheel file.
    goto bad_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b %errorlevel%
