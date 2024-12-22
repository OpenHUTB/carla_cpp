@echo off
setlocal enabledelayedexpansion

rem 这里设置了一个换行符，用于后续在需要换行的地方使用，注意不要删除下面这两行空行
set LF=^


rem 本Bat脚本用于编译并导出CARLA项目（carla.org）
rem 需要在启用了x64 Visual C++工具集的命令提示符（cmd）中运行
rem 参考链接：https://wiki.unrealengine.com/How_to_package_your_game_with_commands

rem 设置当前脚本所在的路径，方便后续操作中基于此路径进行文件和目录相关操作
set LOCAL_PATH=%~dp0
rem 设置一个用于在打印信息中标识当前脚本的字符串格式，方便调试查看输出来源
set FILE_N=-[%~n0]:

rem 打印批处理脚本接收到的命令行参数（用于调试目的）
echo %FILE_N% [Batch params]: %*

rem ==============================================================================
rem -- 解析命令行参数部分 --
rem ==============================================================================
rem 设置文档说明字符串，描述脚本的主要功能
set DOC_STRING="Makes a packaged version of CARLA for distribution."
rem 设置用法说明字符串，展示脚本的正确使用方式和可用参数选项
set USAGE_STRING="Usage: %FILE_N% [-h|--help] [--config={Debug,Development,Shipping}] [--no-packaging] [--no-zip] [--clean] [--clean-intermediate] [--target-archive]"

rem 初始化一些用于控制脚本执行流程的变量，默认值表示是否进行相应操作，例如是否打包、是否复制文件、是否创建压缩包、是否清理等
set DO_PACKAGE=true
set DO_COPY_FILES=true
set DO_TARBALL=true
set DO_CLEAN=false
set PACKAGES=Carla
set PACKAGE_CONFIG=Shipping
set USE_CARSIM=false
set SINGLE_PACKAGE=false

rem 定义代码标签 :arg-parse，用于后续跳转到此处进行参数解析循环处理
:arg-parse
rem 判断第一个命令行参数是否不为空字符串，若不为空则意味着有传入的参数需要解析
if not "%1"=="" (
    rem 判断参数是否为“--clean”，如果是，则设置一系列相关变量，表示进行清理操作，不进行打包、创建压缩包以及复制文件等操作
    if "%1"=="--clean" (
        set DO_CLEAN=true
        set DO_TARBALL=false
        set DO_PACKAGE=false
        set DO_COPY_FILES=false
    )
    rem 判断参数是否为“--config”，如果是，则将第二个参数的值赋给PACKAGE_CONFIG变量，用于指定打包配置（如Debug、Development、Shipping等）
    if "%1"=="--config" (
        set PACKAGE_CONFIG=%2
        shift
    )
    rem 判断参数是否为“--clean-intermediate”，如果是，则设置DO_CLEAN为true，表示进行清理中间文件操作
    if "%1"=="--clean-intermediate" (
        set DO_CLEAN=true
    )
    rem 判断参数是否为“--no-zip”，如果是，则设置DO_TARBALL为false，表示不创建压缩包
    if "%1"=="--no-zip" (
        set DO_TARBALL=false
    )
    rem 判断参数是否为“--no-packaging”，如果是，则设置DO_PACKAGE为false，表示不进行打包操作
    if "%1"=="--no-packaging" (
        set DO_PACKAGE=false
    )
    rem 判断参数是否为“--packages”，如果是，则表示要处理多个包相关的情况，后续需进一步解析参数获取具体的包信息，同时设置一些相关变量表示不进行常规的打包和复制文件操作
    if "%1"=="--packages" (
        set DO_PACKAGE=false
        set DO_COPY_FILES=false
        set PACKAGES=%*
        shift
    )
    rem 判断参数是否为“--target-archive”，如果是，则设置SINGLE_PACKAGE为true，并将第二个参数的值赋给TARGET_ARCHIVE变量，可能用于指定目标归档文件相关信息
    if "%1"=="--target-archive" (
        set SINGLE_PACKAGE=true
        set TARGET_ARCHIVE=%2
        shift
    )
    rem 判断参数是否为“--carsim”，如果是，则设置USE_CARSIM为true，可能用于启用与CarSim相关的功能（具体需看脚本后续相关操作）
    if "%1"=="--carsim" (
        set USE_CARSIM=true
    )
    rem 判断参数是否为“-h”或“--help”，如果是，则输出文档说明和用法说明字符串，然后跳转到脚本末尾结束执行，用于显示帮助信息
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
    rem 左移命令行参数列表，准备解析下一轮的参数
    shift
    rem 跳回 :arg-parse 标签处，继续循环解析剩余参数
    goto :arg-parse
)

rem 获取Unreal Engine根路径，如果未定义UE4_ROOT环境变量，则尝试从注册表中查询相关键值来获取
if not defined UE4_ROOT (
    set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine"
    set VALUE_NAME=InstalledDirectory
    for /f "usebackq tokens=1,2,*" %%A in (`reg query!KEY_NAME! /s /reg:64`) do (
        if "%%A" == "!VALUE_NAME!" (
            set UE4_ROOT=%%C
        )
    )
    rem 如果未获取到UE4_ROOT路径，则跳转到error_unreal_no_found标签处处理错误，提示未检测到Unreal Engine
    if not defined UE4_ROOT goto error_unreal_no_found
)

rem 设置项目相关的打包路径等信息，首先尝试获取CARLA项目的版本号（通过git命令描述标签等方式），如果获取失败则跳转到error_carla_version标签处处理错误
for /f %%i in ('git describe --tags --dirty --always') do set CARLA_VERSION=%%i
if not defined CARLA_VERSION goto error_carla_version

set BUILD_FOLDER=%INSTALLATION_DIR%UE4Carla/%CARLA_VERSION%/
set DESTINATION_ZIP=%INSTALLATION_DIR%UE4Carla/CARLA_%CARLA_VERSION%.zip
set SOURCE=!BUILD_FOLDER!WindowsNoEditor/

rem ============================================================================
rem -- 创建Carla包部分 --
rem ============================================================================
rem 根据DO_PACKAGE变量的值判断是否进行打包操作，如果为true则执行以下打包相关流程
if %DO_PACKAGE%==true (
    rem 根据USE_CARSIM变量的值，通过Python脚本启用或禁用CarSim相关功能，并设置对应的配置文件内容
    if %USE_CARSIM% == true (
        python %ROOT_PATH%Util/BuildTools/enable_carsim_to_uproject.py -f="%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject" -e
        echo CarSim ON > "%ROOT_PATH%Unreal/CarlaUE4/Config/CarSimConfig.ini"
    ) else (
        python %ROOT_PATH%Util/BuildTools/enable_carsim_to_uproject.py -f="%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"
        echo CarSim OFF > "%ROOT_PATH%Unreal/CarlaUE4/Config/CarSimConfig.ini"
    )
    rem 如果打包的目标文件夹不存在，则创建该文件夹
    if not exist "!BUILD_FOLDER!" mkdir "!BUILD_FOLDER!"
    rem 调用Unreal Engine的Build.bat脚本构建CarlaUE4Editor，传递相关参数，如果构建过程出现错误（errorlevel为1），则跳转到error_build_editor标签处处理错误
    call "%UE4_ROOT%\Engine\Build\BatchFiles\Build.bat"^
        CarlaUE4Editor^
        Win64^
        Development^
        -WaitMutex^
        -FromMsBuild^
        "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"
    if errorlevel 1 goto error_build_editor
    rem 调用Unreal Engine的Build.bat脚本构建CarlaUE4，传递相关参数（包括之前设置的PACKAGE_CONFIG指定的配置等）
    echo "%UE4_ROOT%\Engine\Build\BatchFiles\Build.bat"^
        CarlaUE4^
        Win64^
        %PACKAGE_CONFIG%^
        -WaitMutex^
        -FromMsBuild^
        "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"
    call "%UE4_ROOT%\Engine\Build\BatchFiles\Build.bat"^
        CarlaUE4^
        Win64^
        %PACKAGE_CONFIG%^
        -WaitMutex^
        -FromMsBuild^
        "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"
    rem 如果构建CarlaUE4过程出现错误（errorlevel为1），则跳转到error_build标签处处理错误
    if errorlevel 1 goto error_build
    rem 调用Unreal Engine的RunUAT.bat脚本执行一系列操作，如构建、烹饪、打包等，传递相关参数（包括指定项目路径、平台、配置等）
    echo "%UE4_ROOT%\Engine\Build\BatchFiles\RunUAT.bat"^
        BuildCookRun^
        -nocompileeditor^
        -TargetPlatform=Win64^
        -Platform=Win64^
        -installed^
        -nop4^
        -project="%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"^
        -cook^
        -stage^
        -build^
        -archive^
        -archivedirectory="!BUILD_FOLDER!"^
        -package^
        -clientconfig=%PACKAGE_CONFIG%
    call "%UE4_ROOT%\Engine\Build\BatchFiles\RunUAT.bat"^
        BuildCookRun^
        -nocompileeditor^
        -TargetPlatform=Win64^
        -Platform=Win64^
        -installed^
        -nop4^
        -project="%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"^
        -cook^
        -stage^
        -build^
        -archive^
        -archivedirectory="!BUILD_FOLDER!"^
        -package^
        -clientconfig=%PACKAGE_CONFIG%
    rem 如果执行RunUAT.bat过程出现错误（errorlevel为1），则跳转到error_runUAT标签处处理错误
    if errorlevel 1 goto error_runUAT
)

rem ==============================================================================
rem -- 向包中添加额外文件部分 --
rem ==============================================================================
rem 根据DO_COPY_FILES变量的值判断是否进行复制额外文件到包中的操作，如果为true则执行以下文件复制相关流程
if %DO_COPY_FILES%==true (
    echo "%FILE_N% Adding extra files to package..."
    rem 设置源文件路径和目标文件路径变量，将路径中的斜杠替换为反斜杠（Windows下路径格式要求）
    set XCOPY_FROM=%ROOT_PATH:/=\%
    set XCOPY_TO=%SOURCE:/=\%
    rem 使用xcopy命令复制各种文件和文件夹到目标路径，如许可证文件、变更日志文件、不同目录下的Python相关文件、地图文件等，不同的复制操作根据文件类型（文件或文件夹）设置了不同的参数（如/y表示覆盖时不提示等）
    echo f | xcopy /y "!XCOPY_FROM!LICENSE"                                         "!XCOPY_TO!LICENSE"
    echo f | xcopy /y "!XCOPY_FROM!CHANGELOG.md"                                    "!XCOPY_TO!CHANGELOG"
    echo f | xcopy /y "!XCOPY_FROM!Docs\release_readme.md"                          "!XCOPY_TO!README"
    echo f | xcopy /y "!XCOPY_FROM!Util\Docker\Release.Dockerfile"                  "!XCOPY_TO!Dockerfile"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\carla\dist\*.egg"                      "!XCOPY_TO!PythonAPI\carla\dist\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\carla\dist\*.whl"                      "!XCOPY_TO!PythonAPI\carla\dist\"
    echo d | xcopy /y /s "!XCOPY_FROM!Co-Simulation"                                "!XCOPY_TO!Co-Simulation"
    echo d | xcopy /y /s "!XCOPY_FROM!PythonAPI\carla\agents"                       "!XCOPY_TO!PythonAPI\carla\agents"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\carla\scene_layout.py"                 "!XCOPY_TO!PythonAPI\carla\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\carla\requirements.txt"                "!XCOPY_TO!PythonAPI\carla\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\examples\*.py"                         "!XCOPY_TO!PythonAPI\examples\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\examples\requirements.txt"             "!XCOPY_TO!PythonAPI\examples\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\util\*.py"                             "!XCOPY_TO!PythonAPI\util\"
    echo d | xcopy /y /s "!XCOPY_FROM!PythonAPI\util\opendrive"                     "!XCOPY_TO!PythonAPI\util\opendrive"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\util\requirements.txt"                 "!XCOPY_TO!PythonAPI\util\"
    echo f | xcopy /y "!XCOPY_FROM!Unreal\CarlaUE4\Content\Carla\HDMaps\*.pcd"      "!XCOPY_TO!HDMaps\"
    echo f | xcopy /y "!XCOPY_FROM!Unreal\CarlaUE4\Content\Carla\HDMaps\Readme.md"  "!XCOPY_TO!HDMaps\README"
    if exist "!XCOPY_FROM!Plugins" (
        echo d | xcopy /y /s "!XCOPY_FROM!Plugins"                                  "!XCOPY_TO!Plugins"
    )
)

rem ==============================================================================
rem -- 压缩项目部分 --
rem ==============================================================================
rem 根据DO_PACKAGE和DO_TARBALL变量的值判断是否进行压缩项目操作，如果两者都为true则执行以下压缩相关流程
if %DO_PACKAGE%==true if %DO_TARBALL%==true (
    set SRC_PATH=%SOURCE:/=\%
    echo %FILE_N% Building package...
    rem 删除一些可能不需要的文件和文件夹（如各种清单文件、保存相关的文件夹等），以清理打包目录
    if exist "!SRC_PATH!Manifest_NonUFSFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_NonUFSFiles_Win64.txt"
    if exist "!SRC_PATH!Manifest_DebugFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_DebugFiles_Win64.txt"
    if exist "!SRC_PATH!Manifest_UFSFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_UFSFiles_Win64.txt"
    if exist "!SRC_PATH!CarlaUE4/Saved" rmdir /S /Q "!SRC_PATH!CarlaUE4/Saved"
    if exist "!SRC_PATH!Engine/Saved" rmdir /S /Q "!SRC_PATH!Engine/Saved"
    set DST_ZIP=%DESTINATION_ZIP:/=\%
    rem 判断系统中是否存在7-Zip压缩工具，如果存在则使用7-Zip进行压缩，设置相关参数（如压缩格式为zip、启用多线程、设置压缩级别等），否则使用PowerShell的Compress-Archive命令进行压缩（指定压缩级别等参数）
    if exist "%ProgramW6432%/7-Zip/7z.exe" (
        "%ProgramW6432%/7-Zip/7z.exe" a "!DST_ZIP!" "!SRC_PATH!" -tzip -mmt -mx5
    ) else {
        pushd "!SRC_PATH!"
            rem https://docs.microsoft.com/en-us/powershell/module/microsoft.powershell.archive/compress-archive?view=powershell-6
            powershell -command "& { Compress-Archive -Path * -CompressionLevel Fastest -DestinationPath '!DST_ZIP!' }"
        popd
    }
)

rem ==============================================================================
rem -- 移除中间文件部分 --
rem ==============================================================================
rem 根据DO_CLEAN变量的值判断是否进行移除中间文件操作，如果为true则执行以下清理操作，删除构建文件夹及其内容
if %DO_CLEAN%==true (
    echo %FILE_N% Removing intermediate build.
    rmdir /S /Q "!BUILD_FOLDER!"
)

rem ==============================================================================
rem -- 处理其他包（多包相关情况）部分 --
rem ==============================================================================
rem 设置一些与包处理相关的文件路径变量，用于后续操作中定位不同的配置文件、地图文件等信息
set CARLAUE4_ROOT_FOLDER=%ROOT_PATH%Unreal/CarlaUE4
