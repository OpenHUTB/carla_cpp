@echo off
setlocal enabledelayedexpansion

:: 这是一个批处理脚本，用于编译和导出CARLA项目（carla.org）
:: 请在启用了x64 Visual C++ 工具集的cmd中运行它。
:: 参见：https://wiki.unrealengine.com/How_to_package_your_game_with_commands

set LOCAL_PATH=%~dp0  :: 设置为当前批处理文件所在的目录
set FILE_N=-[%~n0]:  :: 设置文件名（不包含路径和扩展名）作为FILE_N变量，前面带有-[]

:: 打印批处理参数（用于调试）
:: 输出批处理文件名和所有参数
echo %FILE_N% [Batch params]: %*

:: ==============================================================================
:: -- 解析参数 -----------------------------------------------------------------
:: ==============================================================================

:: 设置文档字符串，描述此脚本的功能
set DOC_STRING="Makes a packaged version of CARLA for distribution."

:: 设置用法字符串，描述如何使用此脚本
set USAGE_STRING="Usage: %FILE_N% [-h|--help] [--config={Debug,Development,Shipping}] [--no-packaging] [--no-zip] [--clean] [--clean-intermediate] [--target-archive]"

:: 设置是否进行打包
set DO_PACKAGE=true

:: 设置是否复制文件
set DO_COPY_FILES=true

:: 设置是否创建tarball（归档文件）
set DO_TARBALL=true

:: 设置是否进行清理
set DO_CLEAN=false

:: 设置要打包的项目名称
set PACKAGES=Carla

:: 设置打包配置（默认为Shipping）
set PACKAGE_CONFIG=Shipping

:: 设置是否使用CARSIM（此处未在后续代码中使用）
set USE_CARSIM=false

:: 设置是否创建单个包（此处未在后续代码中使用）
set SINGLE_PACKAGE=false

:arg-parse
if not "%1"=="" (
    if "%1"=="--clean" (
        :: 设置进行清理
        set DO_CLEAN=true
        :: 不创建tarball
        set DO_TARBALL=false
        :: 不进行打包
        set DO_PACKAGE=false
        :: 不复制文件
        set DO_COPY_FILES=false
    )

    if "%1"=="--config" (
        :: 设置打包配置为第二个参数的值
        set PACKAGE_CONFIG=%2
        shift
    )

    if "%1"=="--clean-intermediate" (
        :: 设置进行中间文件清理
        set DO_CLEAN=true
    )

    :: 其他参数解析逻辑可以继续添加在这里...

    shift
    goto :arg-parse
)

:: 接下来的代码将是实际的编译、打包和清理逻辑...

    if "%1"=="--no-zip" (
        :: 如果指定了--no-zip参数，则不创建tarball（归档文件）
        set DO_TARBALL=false
    )

    if "%1"=="--no-packaging" (
        :: 如果指定了--no-packaging参数，则不进行打包
        set DO_PACKAGE=false
    )

    if "%1"=="--packages" (
        :: 如果指定了--packages参数，则不进行打包和文件复制，并设置要打包的项目名称为剩余的所有参数
        set DO_PACKAGE=false
        set DO_COPY_FILES=false
        set PACKAGES=%*
        shift  :: 移除已处理的参数，以便处理下一个参数
    )

    if "%1"=="--target-archive" (
        :: 如果指定了--target-archive参数，则设置创建单个包，并设置目标归档文件的名称为第二个参数的值
        set SINGLE_PACKAGE=true
        set TARGET_ARCHIVE=%2
        shift  :: 移除已处理的参数和值，以便处理下一个参数
    )

    if "%1"=="--carsim" (
        :: 如果指定了--carsim参数，则设置使用CARSIM
        set USE_CARSIM=true
    )

    if "%1"=="-h" (
        :: 如果指定了-h或--help参数，则打印文档字符串和用法字符串，并退出脚本
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
    )

    if "%1"=="--help" (
        :: 同上，处理--help参数
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
    )

    :: 如果没有匹配到任何已知参数，则移除当前参数并继续解析下一个参数
    shift
    goto :arg-parse
)

:: 获取Unreal Engine的根路径
:: 如果UE4_ROOT环境变量未定义，则从注册表中查找
if not defined UE4_ROOT (
    set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine"
    set VALUE_NAME=InstalledDirectory
    for /f "usebackq tokens=1,2,*" %%A in (`reg query !KEY_NAME! /s /reg:64`) do (
        :: 在注册表中查找Unreal Engine的安装目录
        if "%%A" == "!VALUE_NAME!" (
            set UE4_ROOT=%%C
        )
    )
    :: 如果没有找到Unreal Engine的安装目录，则跳转到错误处理标签
    if not defined UE4_ROOT goto error_unreal_no_found
)

:: 设置打包路径
::
:: 从git描述中获取CARLA的版本号
for /f %%i in ('git describe --tags --dirty --always') do set CARLA_VERSION=%%i
:: 如果没有获取到CARLA的版本号，则跳转到错误处理标签
if not defined CARLA_VERSION goto error_carla_version

:: 设置构建文件夹的路径
set BUILD_FOLDER=%INSTALLATION_DIR%UE4Carla/%CARLA_VERSION%/

:: 设置目标归档文件的路径
set DESTINATION_ZIP=%INSTALLATION_DIR%UE4Carla/CARLA_%CARLA_VERSION%.zip
:: 设置源文件夹的路径（注意：这里使用了延迟变量扩展的语法!SOURCE!，但在实际使用中可能需要替换为%SOURCE%，除非在setlocal enabledelayedexpansion之后有对应的set SOURCE=...语句）
:: 注意：由于SOURCE变量在之前的代码中未定义，这里可能是一个错误或者遗漏。如果需要使用，请确保在使用前定义它。
:: 例如：set SOURCE=!BUILD_FOLDER!WindowsNoEditor/ （但请注意，这样使用可能会因为延迟变量扩展的问题而导致路径不正确）
:: 正确的做法可能是直接使用%BUILD_FOLDER%WindowsNoEditor/，或者在使用前确保SOURCE已被正确设置。
:: 因此，下面的行应该被替换为：
set SOURCE=%BUILD_FOLDER%WindowsNoEditor/

:: ============================================================================
:: -- 创建 Carla 包 --------------------------------------------------------
:: ============================================================================

if %DO_PACKAGE%==true (
    :: 如果需要打包Carla，则执行以下操作

    :: 根据是否使用CarSim来配置项目
    if %USE_CARSIM% == true (
        python "%ROOT_PATH%Util/BuildTools/enable_carsim_to_uproject.py" -f="%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject" -e
        echo CarSim ON > "%ROOT_PATH%Unreal/CarlaUE4/Config/CarSimConfig.ini"
    ) else (
        python "%ROOT_PATH%Util/BuildTools/enable_carsim_to_uproject.py" -f="%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"
        echo CarSim OFF > "%ROOT_PATH%Unreal/CarlaUE4/Config/CarSimConfig.ini"
    )

    :: 如果构建文件夹不存在，则创建它
    if not exist "%BUILD_FOLDER%" mkdir "%BUILD_FOLDER%"

    :: 构建CarlaUE4Editor
    call "%UE4_ROOT%\Engine\Build\BatchFiles\Build.bat" ^
        CarlaUE4Editor ^
        Win64 ^
        Development ^
        -WaitMutex ^
        -FromMsBuild ^
        "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"

    if errorlevel 1 goto error_build_editor

    :: 构建CarlaUE4的指定配置（例如Shipping或Development）
    :: 注意：这里%PACKAGE_CONFIG%应该是一个之前已经定义好的变量，表示要构建的配置类型
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

    if errorlevel 1 goto error_build

    :: 使用Unreal Engine的Unreal Automation Tool (UAT)来cook、stage、build和archive Carla项目
    :: 注意：这里同样使用了%PACKAGE_CONFIG%变量来表示客户端配置
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

    if errorlevel 1 goto error_runUAT
)

:: ==============================================================================
:: -- 向包中添加额外文件 -------------------------------------------------------
:: ==============================================================================

if %DO_COPY_FILES%==true (
    echo "%FILE_N% Adding extra files to package..."

    set XCOPY_FROM=%ROOT_PATH:/=\%
    set XCOPY_TO=%SOURCE:/=\%

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

:: ==============================================================================
:: -- 压缩项目 -----------------------------------------------------------------
:: ==============================================================================

if %DO_PACKAGE%==true if %DO_TARBALL%==true (
    set SRC_PATH=%SOURCE:/=\%

    echo %FILE_N% Building package...

    :: 删除存在的Manifest文件
    if exist "!SRC_PATH!Manifest_NonUFSFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_NonUFSFiles_Win64.txt"
    if exist "!SRC_PATH!Manifest_DebugFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_DebugFiles_Win64.txt"
    if exist "!SRC_PATH!Manifest_UFSFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_UFSFiles_Win64.txt"
    :: 删除存在的Saved目录
    if exist "!SRC_PATH!CarlaUE4/Saved" rmdir /S /Q "!SRC_PATH!CarlaUE4/Saved"
    if exist "!SRC_PATH!Engine/Saved" rmdir /S /Q "!SRC_PATH!Engine/Saved"

    set DST_ZIP=%DESTINATION_ZIP:/=\%
    :: 使用7-Zip进行压缩，如果不存在则使用PowerShell
    if exist "%ProgramW6432%/7-Zip/7z.exe" (
        "%ProgramW6432%/7-Zip/7z.exe" a "!DST_ZIP!" "!SRC_PATH!" -tzip -mmt -mx5
    ) else (
        pushd "!SRC_PATH!"
            :: 使用PowerShell的Compress-Archive进行压缩
            powershell -command "& { Compress-Archive -Path * -CompressionLevel Fastest -DestinationPath '!DST_ZIP!' }"
        popd
    )
)

:: ==============================================================================
:: -- 移除中间文件 ---------------------------------------------------------
:: ==============================================================================

if %DO_CLEAN%==true (
    echo %FILE_N% 正在移除中间构建。
    rmdir /S /Q "!BUILD_FOLDER!"
)

:: ==============================================================================
:: -- Cook其他包 -------------------------------------------------------
:: ==============================================================================

:: 设置一些文件位置
set CARLAUE4_ROOT_FOLDER=%ROOT_PATH%Unreal/CarlaUE4
set PACKAGE_PATH_FILE=%CARLAUE4_ROOT_FOLDER%/Content/PackagePath.txt
set MAP_LIST_FILE=%CARLAUE4_ROOT_FOLDER%/Content/MapPaths.txt

:: 从参数字符串中获取要Cook的包（支持多个包）
echo Parsing packages...
if not "%PACKAGES%" == "Carla" (
    :: 这里尝试将PACKAGES中的逗号替换为换行符，但Windows批处理不直接支持。
    :: 下面的代码可能无法按预期工作，需要另一种方法来处理逗号分隔的列表。
    set ARGUMENTS=%PACKAGES:--=!LF!%
    for /f "tokens=*" %%i in ("!ARGUMENTS!") do (
        set a=%%i
        :: 检查参数是否以"packages="或"packages "开头，并提取包名
        if "!a:~0,9!" == "packages=" (
            set RESULT=!a:~9!
        ) else (
            if "!a:~0,9!" == "packages " (
                set RESULT=!a:~9!
            )
        )
    )
) else (
    set RESULT=%PACKAGES%
)

:: 遍历所有要Cook的地图（参数）
:: 注意：下面的代码可能无法正确处理逗号分隔的包名列表。
set PACKAGES=%RESULT:,=!LF!%
for /f "tokens=* delims=" %%i in ("!PACKAGES!") do (
    set PACKAGE_NAME=%%i

    :: 跳过名为"Carla"的包
    if not !PACKAGE_NAME! == Carla (
        echo Cooking package '!PACKAGE_NAME!'...

        set BUILD_FOLDER=%INSTALLATION_DIR%UE4Carla/!PACKAGE_NAME!_%CARLA_VERSION%\
        set PACKAGE_PATH=%CARLAUE4_ROOT_FOLDER%/Content/!PACKAGE_NAME!

        if not exist "!BUILD_FOLDER!" mkdir "!BUILD_FOLDER!"

        echo 正在烹饪包'!PACKAGE_NAME!'...

        pushd "%CARLAUE4_ROOT_FOLDER%"

        echo   - prepare
        :: 准备Cook包
        echo Prepare cooking of package: !PACKAGE_NAME!
        call "%UE4_ROOT%/Engine/Binaries/Win64/UE4Editor.exe" ^
        "%CARLAUE4_ROOT_FOLDER%/CarlaUE4.uproject" ^
        -run=PrepareAssetsForCooking ^
        -PackageName=!PACKAGE_NAME! ^
        -OnlyPrepareMaps=false

        :: 读取包路径和要Cook的地图列表
        set /p PACKAGE_FILE=<%PACKAGE_PATH_FILE%
        set /p MAPS_TO_COOK=<%MAP_LIST_FILE%

        echo   - cook
        for /f "tokens=*" %%a in (%MAP_LIST_FILE%) do (
            :: Cook map
            echo Cooking: %%a
            call "%UE4_ROOT%/Engine/Binaries/Win64/UE4Editor.exe" ^
            "%CARLAUE4_ROOT_FOLDER%/CarlaUE4.uproject" ^
            -run=cook ^
            -map="%%a" ^
            -targetplatform="WindowsNoEditor" ^
            -OutputDir="!BUILD_FOLDER!" ^
            -iterate ^
            -cooksinglepackage ^
            )

        )

       :: 如果存在 props 文件夹，则删除它
       set PROPS_MAP_FOLDER="%PACKAGE_PATH%/Maps/PropsMap"
       if exist "%PROPS_MAP_FOLDER%" (
       rmdir /S /Q "%PROPS_MAP_FOLDER%"
       )

       popd

       echo Copying files to '!PACKAGE_NAME!'...

       pushd "!BUILD_FOLDER!"


       set SUBST_PATH=!BUILD_FOLDER!CarlaUE4
       :: 将包配置文件复制到包中
       set TARGET="!SUBST_PATH!\Content\Carla\Config\"
       mkdir !TARGET:/=\!
       copy "!PACKAGE_FILE:/=\!" !TARGET:/=\!
 

       :: 为每个地图复制一些文件到包中
       :: MAPS_TO_COOK 被读取为一个由 '+' 分隔的标记数组，我们将 '+' 替换为新行
       :: 我们需要这行后面的空行，不要删除它
       set MAPS_TO_COOK=!MAPS_TO_COOK:+=^

       !
       set BASE_CONTENT=%INSTALLATION_DIR:/=\%..\Unreal\CarlaUE4\Content
       for /f "tokens=1 delims=+" %%a in ("!MAPS_TO_COOK!") do (

          :: 获取地图的路径和名称
          for /f %%i in ("%%a") do (
          set MAP_FOLDER=%%~pi
          set MAP_NAME=%%~ni
          :: 移除 '/Game' 字符串
          set MAP_FOLDER=!MAP_FOLDER:~5!
        )

        :: # 复制 OpenDrive 文件
        set SRC=!BASE_CONTENT!!MAP_FOLDER!\OpenDrive\!MAP_NAME!.xodr
        set TRG=!BUILD_FOLDER!\CarlaUE4\Content\!MAP_FOLDER!\OpenDrive\
        if exist "!SRC!" (
            mkdir "!TRG!"
            copy "!SRC!" "!TRG!"
        )

        :: # 复制导航文件
        set SRC=!BASE_CONTENT!!MAP_FOLDER!\Nav\!MAP_NAME!.bin
        set TRG=!BUILD_FOLDER!\CarlaUE4\Content\!MAP_FOLDER!\Nav\
        if exist "!SRC!" (
            kdir "!TRG!"
            opy "!SRC!" "!TRG!"
        )
    
        :: # 复制交通管理器地图文件
        set SRC=!BASE_CONTENT!!MAP_FOLDER!\TM\!MAP_NAME!.bin
        set TRG=!BUILD_FOLDER!\CarlaUE4\Content\!MAP_FOLDER!\TM\
        if exist "!SRC!" (
            mkdir "!TRG!"
            copy "!SRC!" "!TRG!"
        )
    )

    :: 删除不必要的文件夹
    rmdir /S /Q "!BUILD_FOLDER!\CarlaUE4\Metadata"
    rmdir /S /Q "!BUILD_FOLDER!\CarlaUE4\Plugins"
    :: 删除特定地图的 props 文件（这一行已被注释掉，如果需要可以取消注释）
    :: del "!BUILD_FOLDER!\CarlaUE4\Content\!PACKAGE_NAME!/Maps/!PROPS_MAP_NAME!"
    del "!BUILD_FOLDER!\CarlaUE4\AssetRegistry.bin"

    :: 如果需要打包成 tar.gz，则执行以下操作
    if %DO_TARBALL%==true (

        :: 如果是单个包，则设置目标压缩文件名
        if %SINGLE_PACKAGE%==true (
            echo Packaging '%TARGET_ARCHIVE%'...
            set DESTINATION_ZIP=%INSTALLATION_DIR%UE4Carla/%TARGET_ARCHIVE%_%CARLA_VERSION%.zip
        ) else (
            :: 如果是多个包，则设置目标压缩文件名
            echo Packaging '!PACKAGE_NAME!'...
            set DESTINATION_ZIP=%INSTALLATION_DIR%UE4Carla/!PACKAGE_NAME!_%CARLA_VERSION%.zip
        )

        :: 设置源文件夹路径，将路径中的斜杠替换为转义字符
        set SOURCE=!BUILD_FOLDER:/=\!\
        :: 设置目标ZIP文件路径，将路径中的斜杠替换为转义字符
        set DST_ZIP=!DESTINATION_ZIP:/=\!

        :: 切换到源文件夹
        pushd "!SOURCE!"

        :: 检查7-Zip是否安装，如果安装了则使用7-Zip压缩
        if exist "%ProgramW6432%/7-Zip/7z.exe" (
            "%ProgramW6432%/7-Zip/7z.exe" a "!DST_ZIP!" . -tzip -mmt -mx5
        ) else (
            :: 如果没有安装7-Zip，则使用PowerShell进行压缩
            powershell -command "& { Compress-Archive -Update -Path * -CompressionLevel Fastest -DestinationPath '!DST_ZIP!' }"
        )

        :: 返回到原始目录
        popd

        :: 检查上一个命令是否执行成功，如果不成功则跳转到错误处理
        if errorlevel 1 goto bad_exit
        echo ZIP created at !DST_ZIP!
     )

    :: 返回到最外层目录（如果有多个pushd）
    popd

    :: 如果设置了DO_CLEAN为true，则删除中间构建文件夹
    if %DO_CLEAN%==true (
            echo %FILE_N% Removing intermediate build.
            rmdir /S /Q "!BUILD_FOLDER!"
        )
    )
)

:: 成功跳转标签
goto success

:: 错误和消息处理部分

:: 成功标签
:success
    echo.
    if %DO_PACKAGE%==true echo %FILE_N% Carla project successful exported to "%BUILD_FOLDER:/=\%"!
    if %DO_TARBALL%==true echo %FILE_N% Compress carla project exported to "%DESTINATION_ZIP%"!
    goto good_exit

:: Carla版本未设置错误
:error_carla_version
     echo.
    echo %FILE_N% [ERROR] Carla Version is not set
    goto bad_exit

:: Unreal Engine未检测到错误
:error_unreal_no_found
   echo.
    echo %FILE_N% [ERROR] Unreal Engine not detected
    goto bad_exit

:: 构建CarlaUE4Editor错误
:error_build_editor
    echo.
    echo %FILE_N% [ERROR] There was a problem while building the CarlaUE4Editor.
    echo           [ERROR] Please read the screen log for more information.
    goto bad_exit

:: 构建CarlaUE4错误
:error_build
    echo.
    echo %FILE_N% [错误] 构建CarlaUE4时出现问题。
    echo           [错误] 请查看屏幕日志以获取更多信息。
    goto bad_exit

:: 运行UAT错误
:error_runUAT
    echo.
    echo %FILE_N% [ERROR] There was a problem while building the CarlaUE4.
    echo           [ERROR] Please read the screen log for more information.
    goto bad_exit

:: 正常退出标签
:good_exit
    endlocal
    exit /b 0

:: 错误退出标签
:bad_exit
    endlocal
    exit /b 1
