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
          echo Nothing selected to be done.

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
    echo.  ; 打印一个空行
    if %BUILD_FOR_PYTHON3%==true echo %FILE_N% Carla lib for python has been successfully installed in "%PYTHON_LIB_PATH%dist"!
    ; 如果环境变量BUILD_FOR_PYTHON3为true，则打印安装成功的消息
    goto good_exit
    ; 跳转到标签good_exit

:already_installed （此标签对应的代码被注释掉了，不会执行）
    ; 此部分代码已被注释，不执行任何操作
    ; 如果已安装，则打印错误消息并跳转到good_exit

:py2_not_supported
    echo.  ; 打印一个空行
    echo %FILE_N% [ERROR] Python 2 is not currently supported in Windows.
    ; 打印不支持Python 2的错误消息
    goto bad_exit
    ; 跳转到标签bad_exit

:error_py
    echo.  ; 打印一个空行
    echo %FILE_N% [ERROR] An error occurred while executing the py.
    ; 打印执行py时发生错误的消息
    echo %FILE_N% [ERROR] Possible causes:
    ; 打印可能原因的开始
    echo %FILE_N% [ERROR]  - Make sure "py" is installed.
    ; 确保已安装py（Python启动器）
    echo %FILE_N% [ERROR]  - py = python launcher. This utility is bundled with Python installation but not installed by default.
    ; py是Python启动器，随Python安装但不默认安装
    echo %FILE_N% [ERROR]  - Make sure it is available on your Windows "py".
    ; 确保在Windows上可用
    goto bad_exit
    ; 跳转到标签bad_exit

:error_build_wheel
    echo.  ; 打印一个空行
    echo %FILE_N% [ERROR] An error occurred while building the wheel file.
    ; 打印构建wheel文件时发生错误的消息
    goto bad_exit
    ; 跳转到标签bad_exit

:good_exit
    endlocal  ; 结束本地环境变量的更改
    exit /b 0  ; 以状态码0退出，表示成功

:bad_exit
    endlocal  ; 结束本地环境变量的更改
    exit /b %errorlevel%  ; 以当前错误级别退出，表示失败
