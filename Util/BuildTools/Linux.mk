default: help

help:
	@less ${CARLA_BUILD_TOOLS_FOLDER}/Linux.mk.help

launch: LibCarla.server.release osm2odr downloadplugins
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.sh --build $(ARGS)
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --build --launch $(ARGS)

launch-only:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --launch $(ARGS)

import: CarlaUE4Editor PythonAPI
	@${CARLA_BUILD_TOOLS_FOLDER}/Import.sh $(ARGS)

package: CarlaUE4Editor PythonAPI
	@${CARLA_BUILD_TOOLS_FOLDER}/Package.sh $(ARGS)

package.rss: CarlaUE4Editor PythonAPI.rss.rebuild
	@${CARLA_BUILD_TOOLS_FOLDER}/Package.sh $(ARGS)

docs:
	@doxygen
	@echo "Documentation index at ./Doxygen/html/index.html"

clean.LibCarla:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --clean
clean.PythonAPI:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --clean
clean.CarlaUE4Editor:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.sh --clean $(ARGS)
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --clean
clean.osm2odr:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.sh --clean
clean: clean.CarlaUE4Editor clean.PythonAPI clean.LibCarla clean.osm2odr

rebuild: setup
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --rebuild
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.sh --rebuild
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --rebuild $(ARGS)
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.sh --rebuild $(ARGS)
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --rebuild $(ARGS)

hard-clean:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.sh --clean $(ARGS)
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --hard-clean
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.sh --clean
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --clean
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --clean
	@echo "To force recompiling dependencies run: rm -Rf ${CARLA_BUILD_FOLDER}"

check: LibCarla PythonAPI
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --all $(ARGS)

check.LibCarla: LibCarla
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --libcarla-debug --libcarla-release $(ARGS)

check.LibCarla.debug: LibCarla.debug
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --libcarla-debug $(ARGS)

check.LibCarla.release: LibCarla.release
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --libcarla-release $(ARGS)

check.PythonAPI: PythonAPI
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --python-api $(ARGS)

check.PythonAPI.2: PythonAPI.2
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --python-api --python-version=2 $(ARGS)

check.PythonAPI.3: PythonAPI.3
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --python-api --python-version=3 $(ARGS)

benchmark: LibCarla.release
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --benchmark $(ARGS)
	@cat profiler.csv

smoke_tests:
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --smoke $(ARGS)

examples:
	@for D in ${CARLA_EXAMPLES_FOLDER}/*; do [ -d "$${D}" ] && make -C $${D} build; done

run-examples:
	@for D in ${CARLA_EXAMPLES_FOLDER}/*; do [ -d "$${D}" ] && make -C $${D} run.only; done

CarlaUE4Editor: LibCarla.server.release osm2odr downloadplugins
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.sh --build $(ARGS)
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --build $(ARGS)

.PHONY: PythonAPI
PythonAPI: LibCarla.client.release osm2odr
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh $(ARGS)

PythonAPI.2: LibCarla.client.release osm2odr
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --python-version=2

PythonAPI.3: LibCarla.client.release osm2odr
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --python-version=3

PythonAPI.rebuild: LibCarla.client.release osm2odr
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --rebuild $(ARGS)

PythonAPI.rss: LibCarla.client.rss.release osm2odr
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --rss $(ARGS)

PythonAPI.rss.rebuild: LibCarla.client.rss.release osm2odr
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --rebuild --rss $(ARGS)

PythonAPI.docs:
	@python PythonAPI/docs/doc_gen.py
	@cd PythonAPI/docs && python3 bp_doc_gen.py

.PHONY: LibCarla
LibCarla: LibCarla.release LibCarla.debug

LibCarla.debug: LibCarla.server.debug LibCarla.client.debug
LibCarla.release: LibCarla.server.release LibCarla.client.release

LibCarla.server: LibCarla.server.debug LibCarla.server.release#定义了目标LibCarla.server，它依赖于LibCarla.server.debug和LibCarla.server.release。这意味着要构建LibCarla.server，必须先构建这两个依赖目标。
LibCarla.server.debug: setup#定义了LibCarla.server.debug目标的构建规则。
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --server --debug $(ARGS)
LibCarla.server.release: setup# 定义LibCarla.server.release目标的构建规则中的设置部分（setup）
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --server --release $(ARGS)
LibCarla.client: LibCarla.client.debug LibCarla.client.release
# 定义目标LibCarla.client，它有两个子目标LibCarla.client.debug和LibCarla.client.release
LibCarla.client.debug: setup#LibCarla.client目标依赖于LibCarla.client.debug和LibCarla.client.release
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --client --debug $(ARGS)
LibCarla.client.release: setup
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --client --release $(ARGS)
# 定义目标LibCarla.client.rss，它有两个子目标LibCarla.client.rss.debug和LibCarla.client.rss.release
LibCarla.client.rss: LibCarla.client.rss.debug LibCarla.client.rss.release
LibCarla.client.rss.debug: setup ad-rss# 定义LibCarla.client.rss.debug目标的构建规则中的设置部分（setup），这里有个ad - rss不太明确具体含义，可能是特定设置
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --client --debug --rss
LibCarla.client.rss.release: setup ad-rss
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --client --release --rss

.PHONY: #PHONY目标Plugins。PHONY目标在Makefile中有特殊意义。
plugins:    # 使用@符号使得执行下面这行命令时，不会在终端显示命令本身，只显示命令输出的结果   
# 调用${CARLA_BUILD_TOOLS_FOLDER}路径下的Plugins.sh脚本，并且传递变量$(ARGS)作为参数给该脚本
    # 这里的${CARLA_BUILD_TOOLS_FOLDER}大概率是一个环境变量，指向存放构建工具相关脚本的文件夹，而$(ARGS)具体值应是在Makefile其他地方定义或者外部传入的一些配置参数等
	@${CARLA_BUILD_TOOLS_FOLDER}/Plugins.sh $(ARGS)

setup: downloadplugins# setup目标定义，它依赖于downloadplugins目标，意味着执行setup前会先执行downloadplugins相关操作
	@${CARLA_BUILD_TOOLS_FOLDER}/Setup.sh $(ARGS)

ad-rss:# ad - rss目标定义，其具体功能需看Ad - rss.sh脚本的实现，但从名字推测可能和某种与rss相关的自适应（adaptive）功能有关
	@${CARLA_BUILD_TOOLS_FOLDER}/Ad-rss.sh $(ARGS)

deploy:# deploy目标定义，用于执行部署相关操作
	@${CARLA_BUILD_TOOLS_FOLDER}/Deploy.sh $(ARGS)

pretty:# pretty目标定义，或许和代码美化、格式化等使项目相关内容更“美观”易读的操作有关
	@${CARLA_BUILD_TOOLS_FOLDER}/Prettify.sh $(ARGS)

build.utils: PythonAPI# build.utils目标定义，与PythonAPI存在某种关联，不过具体关联还得结合更多代码判断
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildUtilsDocker.sh

osm2odr:# osm2odr目标定义，可能涉及将OpenStreetMap（OSM）数据转换为OpenDRIVE（ODR）格式相关构建操作
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.sh --build $(ARGS)

osmrenderer:# osm2odr目标定义，可能涉及将OpenStreetMap（OSM）数据转换为OpenDRIVE（ODR）格式相关构建操作
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildOSMRenderer.sh

downloadplugins:# osm2odr目标定义，可能涉及将OpenStreetMap（OSM）数据转换为OpenDRIVE（ODR）格式相关构建操作
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.sh --build $(ARGS)
