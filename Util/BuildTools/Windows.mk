ARGS=--all

default: help

# 定义一个环境变量ROOT_PATH
export ROOT_PATH=$(CURDIR)/

# 定义一个环境变量INSTALLATION_DIR，用于表示依赖项安装/构建的目录位置
export INSTALLATION_DIR=$(ROOT_PATH)Build/

help:
	@type "${CARLA_BUILD_TOOLS_FOLDER}\Windows.mk.help"

# 使用.PHONY声明import为伪目标，即使有与import同名的文件夹存在也能确保make命令将其当作命令来执行
.PHONY: import
import: server
	@"${CARLA_BUILD_TOOLS_FOLDER}/Import.py" $(ARGS)

CarlaUE4Editor: LibCarla osm2odr
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --build $(ARGS)

launch: CarlaUE4Editor
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --launch $(ARGS)

launch-only:
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --launch $(ARGS)

package: PythonAPI
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --at-least-write-optionalmodules $(ARGS)
	@"${CARLA_BUILD_TOOLS_FOLDER}/Package.bat" --ue-version 4.26 $(ARGS)

.PHONY: docs
docs:
	@doxygen
	@echo "Documentation index at ./Doxygen/html/index.html"

PythonAPI.docs:
	python PythonAPI/docs/doc_gen.py
	cd PythonAPI/docs && python bp_doc_gen.py

clean:
	@"${CARLA_BUILD_TOOLS_FOLDER}/Package.bat" --clean --ue-version 4.26
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --clean
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.bat" --clean
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --clean
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.bat" --clean

rebuild: setup
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --rebuild
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --rebuild
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.bat" --rebuild
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.bat" --rebuild

check: PythonAPI
	@echo "Not implemented!"

benchmark: LibCarla
	@echo "Not implemented!"

.PHONY: PythonAPI
PythonAPI: LibCarla osm2odr
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.bat" --py3

server: setup
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --server --generator "$(GENERATOR)"

client: setup
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --client --generator "$(GENERATOR)"

.PHONY: LibCarla
LibCarla: setup
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --server --client --generator "$(GENERATOR)"

setup: downloadplugin
	@"${CARLA_BUILD_TOOLS_FOLDER}/Setup.bat" --boost-toolset msvc-14.2 --generator "$(GENERATOR)" $(ARGS)


.PHONY: Plugins
plugins:
	@"${CARLA_BUILD_TOOLS_FOLDER}/Plugins.bat" $(ARGS)

deploy:
	@"${CARLA_BUILD_TOOLS_FOLDER}/Deploy.bat" $(ARGS)

osm2odr:
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.bat" --generator "$(GENERATOR)" --build $(ARGS)

osmrenderer:
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildOSMRenderer.bat"

downloadplugin:
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.bat" --build $(ARGS)
