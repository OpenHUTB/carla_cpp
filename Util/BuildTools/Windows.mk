ARGS=--all
# 定义一个变量ARGS，其值为--all，这个变量后续可能会在多个命令中作为参数使用，用于控制命令执行的范围等情况
default: help
# 将默认目标（当在命令行中只输入 make 时执行的目标）设置为 help，即默认情况下会显示帮助信息
# root of the project (makefile directory)
export ROOT_PATH=$(CURDIR)/
# 定义一个环境变量 ROOT_PATH，其值为当前目录（CURDIR）加上 /，也就是将当前 Makefile 所在的目录设置为项目的根目录，后续其他命令可以基于此目录来定位相关文件和资源
# dependecy install/build directory (rpclib, gtest, boost)
export INSTALLATION_DIR=$(ROOT_PATH)Build/
# 定义一个环境变量 INSTALLATION_DIR，其值为 ROOT_PATH 加上 Build/，用于指定依赖安装或构建的目录
help:
	@type "${CARLA_BUILD_TOOLS_FOLDER}\Windows.mk.help"
# 使用 @ 符号表示在执行命令时不显示命令本身，只显示命令的输出结果。这里尝试查找并执行一个名为 "${CARLA_BUILD_TOOLS_FOLDER}\Windows.mk.help" 的文件
    @type "${CARLA_BUILD_TOOLS_FOLDER}\Windows.mk.help"
# use PHONY to force next line as command and avoid conflict with folders of the same name
.PHONY: import
import: server
	@"${CARLA_BUILD_TOOLS_FOLDER}/Import.py" $(ARGS)
# 使用.PHONY 声明 import 为伪目标，这意味着即使在文件系统中存在与该目标同名的文件或目录，make 命令也会将其当作一个命令来执行，而不会与实际的文件或目录产生混淆。import 目标依赖于 server 目标，执行时会调用 "${CARLA_BUILD_TOOLS_FOLDER}/Import.py" 脚本，并传入 ARGS 变量的值作为参数
CarlaUE4Editor: LibCarla osm2odr
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --build $(ARGS)
# CarlaUE4Editor 目标依赖于 LibCarla 和 osm2odr 目标，执行时会调用 "${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" 脚本，并传入 --build 和 ARGS 变量的值作为参数
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --launch $(ARGS)
# launch 目标依赖于 CarlaUE4Editor 目标，执行时会调用 "${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" 脚本，并传入 --launch 和 ARGS 变量的值作为参数
launch: CarlaUE4Editor
launch-only:
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --launch $(ARGS)
# launch-only 目标，执行时会调用 "${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" 脚本，并传入 --launch 和 ARGS 变量的值作为参数
package: PythonAPI
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --at-least-write-optionalmodules $(ARGS)
	@"${CARLA_BUILD_TOOLS_FOLDER}/Package.bat" --ue-version 4.26 $(ARGS)
# package 目标依赖于 PythonAPI 目标，执行时先调用 "${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" 脚本传入相应参数进行一些构建相关操作，然后调用 "${CARLA_BUILD_TOOLS_FOLDER}/Package.bat" 脚本传入 --ue-version 4.26 和 ARGS 变量的值作为参数，用于将 CARLA 项目打包成适合分发的版本（具体打包逻辑取决于 Package.bat 脚本内容）
.PHONY: docs
docs:
	@doxygen
	@echo "Documentation index at ./Doxygen/html/index.html"
# 使用.PHONY 声明 docs 为伪目标，docs 目标用于生成项目文档。先执行 doxygen 命令（用于从代码注释等信息生成文档的工具），然后输出提示信息，告知文档索引文件所在的位置（./Doxygen/html/index.html），方便用户查看生成的文档内容
PythonAPI.docs:
	python PythonAPI/docs/doc_gen.py
	cd PythonAPI/docs && python bp_doc_gen.py
# PythonAPI.docs 目标，用于生成 PythonAPI 模块相关的文档。先执行 PythonAPI/docs/doc_gen.py 脚本（可能是自定义的用于生成 Python API 文档的 Python 脚本），然后进入 PythonAPI/docs 目录下执行 bp_doc_gen.py 脚本，通过这两个脚本的执行来生成 PythonAPI 相关的详细文档内容
clean:
	@"${CARLA_BUILD_TOOLS_FOLDER}/Package.bat" --clean --ue-version 4.26
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --clean
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.bat" --clean
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --clean
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.bat" --clean
# clean 目标用于清理项目构建过程中产生的中间文件和临时文件等。分别调用多个不同的批处理脚本，并传入相应的清理参数（如 --clean 等），对 Package、BuildCarlaUE4、BuildPythonAPI、BuildLibCarla、BuildOSM2ODR 等相关的构建内容进行清理操作，使项目目录恢复到相对干净的状态，便于重新构建等操作
rebuild: setup
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --rebuild
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --rebuild
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.bat" --rebuild
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.bat" --rebuild
# rebuild 目标依赖于 setup 目标，执行时会依次调用多个批处理脚本（BuildCarlaUE4.bat、BuildLibCarla.bat、BuildOSM2ODR.bat、BuildPythonAPI.bat）并传入 --rebuild 参数，用于先执行 setup 相关的设置操作，然后对整个项目进行重新构建，相当于先清理再完整构建一遍项目内容
check: PythonAPI
	@echo "Not implemented!"
# check 目标依赖于 PythonAPI 目标，但目前只是输出提示信息 "Not implemented!"，说明此功能可能尚未实现，理论上可能是用于对项目进行一些检查（比如代码规范检查、功能完整性检查等），后续需要完善相应的实现逻辑
benchmark: LibCarla
	@echo "Not implemented!"
# benchmark 目标依赖于 LibCarla 目标，同样目前只是输出提示信息 "Not implemented!"，意味着此功能暂未实现，按常理应该是用于对 LibCarla 模块进行性能基准测试（比如测试运行速度、资源占用等性能指标），后续需要补充具体的实现代码
.PHONY: PythonAPI
PythonAPI: LibCarla osm2odr
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.bat" --py3
# 使用.PHONY 声明 PythonAPI 为伪目标，PythonAPI 目标依赖于 LibCarla 和 osm2odr 目标，执行时会调用 "${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.bat" 脚本并传入 --py3 参数，用于构建针对 Python 3 的 Python API 模块，使其能在 Python 3 环境下正确使用
server: setup
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --server --generator "$(GENERATOR)"
# server 目标依赖于 setup 目标，执行时会调用 "${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" 脚本，并传入 --server 和 --generator "$(GENERATOR)" 参数，用于按照服务器端的配置要求构建 LibCarla 库（具体构建逻辑和依赖的生成器等由传入的参数和脚本内部决定），GENERATOR 变量的值应该在外部传入或者在 Makefile 其他地方有定义
client: setup
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --client --generator "$(GENERATOR)"
# client 目标依赖于 setup 目标，执行时会调用 "${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" 脚本，并传入 --client 和 --generator "$(GENERATOR)" 参数，用于按照客户端的配置要求构建 LibCarla 库，与 server 目标相对应
.PHONY: LibCarla
LibCarla: setup
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --server --client --generator "$(GENERATOR)"
# 使用.PHONY 声明 LibCarla 为伪目标，LibCarla 目标依赖于 setup 目标，执行时会调用 "${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" 脚本，并传入 --server、--client 和 --generator "$(GENERATOR)" 参数，用于构建 LibCarla 库
setup: downloadplugin
	@"${CARLA_BUILD_TOOLS_FOLDER}/Setup.bat" --boost-toolset msvc-14.2 --generator "$(GENERATOR)" $(ARGS)

.PHONY: Plugins
plugins:
	@"${CARLA_BUILD_TOOLS_FOLDER}/Plugins.bat" $(ARGS)
# 使用.PHONY 声明 Plugins 为伪目标，plugins 目标执行时会调用 "${CARLA_BUILD_TOOLS_FOLDER}/Plugins.bat" 脚本，并传入 ARGS 变量的值作为参数
deploy:
	@"${CARLA_BUILD_TOOLS_FOLDER}/Deploy.bat" $(ARGS)
# deploy 目标执行时会调用 "${CARLA_BUILD_TOOLS_FOLDER}/Deploy.bat" 脚本，并传入 ARGS 变量的值作为参数
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.bat" --generator "$(GENERATOR)" --build $(ARGS)
# osm2odr 目标执行时会调用 "${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.bat" 脚本，并传入 --generator "$(GENERATOR)" 和 --build $(ARGS) 参数，用于构建与 osm2odr 相关的内容
osmrenderer:
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildOSMRenderer.bat"

downloadplugin:
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.bat" --build $(ARGS)
