default: help
#这是一个Makefile（从格式推测）中的默认目标定义。当没有指定具体目标时，将执行help目标相关的操作。
help:#这是help目标的定义。

	@less${CARLA_BUILD_TOOLS_FOLDER}/Linux.mk.help：这行命令使用less命令查看${CARLA_BUILD_TOOLS_FOLDER}/Linux.mk.help文件的内容。@符号在Makefile中表示在执行命令时不显示命令本身，只显示命令的输出结果。
	@less ${CARLA_BUILD_TOOLS_FOLDER}/Linux.mk.help

launch: LibCarla.server.release osm2odr downloadplugins#这是launch目标的定义，并且它依赖于LibCarla.server.release、osm2odr和downloadplugins这几个目标（这里假设这几个也是在脚本中其他地方定义的目标，如果不是目标而是文件或者其他实体，那就是表示依赖关系的不同含义）。
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.sh --build $(ARGS)
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --build --launch $(ARGS)

launch-only:#这是launch - only目标的定义。

	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --launch$(ARGS)：执行${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh脚本，并传递--launch参数和变量$(ARGS)。
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --launch $(ARGS)

import: CarlaUE4Editor PythonAPI#这是import目标的定义，依赖于CarlaUE4Editor和PythonAPI（同样假设是目标或者相关实体）。
	@${CARLA_BUILD_TOOLS_FOLDER}/Import.sh $(ARGS)

package: CarlaUE4Editor PythonAPI#这是package目标的定义，依赖于CarlaUE4Editor和PythonAPI。
	@${CARLA_BUILD_TOOLS_FOLDER}/Package.sh $(ARGS)

package.rss: CarlaUE4Editor PythonAPI.rss.rebuild#这是package.rss目标的定义，依赖于CarlaUE4Editor和PythonAPI.rss.rebuild。
	@${CARLA_BUILD_TOOLS_FOLDER}/Package.sh $(ARGS)

docs:#来是一个目标名称，可能用于生成文档
	@doxygen
	@echo "Documentation index at ./Doxygen/html/index.html"
#一个简单的命令，用于显示文档索引的位置
clean.LibCarla:# 清理LibCarla相关内容的目标
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --clean
clean.PythonAPI:# 清理PythonAPI相关内容的目标
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --clean
clean.CarlaUE4Editor:# 清理CarlaUE4Editor相关内容的目标
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.sh --clean $(ARGS)
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --clean
clean.osm2odr:# 清理osm2odr相关内容的目标
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.sh --clean
clean: clean.CarlaUE4Editor clean.PythonAPI clean.LibCarla clean.osm2odr

rebuild: setup# 重建相关的目标定义
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --rebuild
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.sh --rebuild
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --rebuild $(ARGS)
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.sh --rebuild $(ARGS)
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --rebuild $(ARGS)

hard-clean:# 'hard - clean'目标的操作
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.sh --clean $(ARGS)
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --hard-clean
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.sh --clean
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --clean
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --clean
	@echo "To force recompiling dependencies run: rm -Rf ${CARLA_BUILD_FOLDER}"

check: LibCarla PythonAPI# 'check'目标依赖于'LibCarla'和'PythonAPI'，可能用于检查这两个部分相关的内容
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --all $(ARGS)

check.LibCarla: LibCarla # 'check.LibCarla'目标依赖于'LibCarla'，可能是对'LibCarla'部分进行单独检查
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --libcarla-debug --libcarla-release $(ARGS)

check.LibCarla.debug: LibCarla.debug# 'check.LibCarla.debug'目标依赖于'LibCarla.debug'，专门针对'LibCarla.debug'部分进行检查
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --libcarla-debug $(ARGS)

check.LibCarla.release: LibCarla.release# 'check.LibCarla.release'目标依赖于'LibCarla.release'，针对'LibCarla.release'进行检查
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --libcarla-release $(ARGS)

check.PythonAPI: PythonAPI# 'check.PythonAPI'目标依赖于'PythonAPI'，用于对'PythonAPI'进行检查
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --python-api $(ARGS)

check.PythonAPI.2: PythonAPI.2# 'check.PythonAPI.2'目标依赖于'PythonAPI.2'，可能是对特定版本（版本2相关的'PythonAPI'）的检查
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --python-api --python-version=2 $(ARGS)

check.PythonAPI.3: PythonAPI.3 # 'check.PythonAPI.3'目标依赖于'PythonAPI.3'，可能是对版本3相关的'PythonAPI'进行检查
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --python-api --python-version=3 $(ARGS)

benchmark: LibCarla.release# 'benchmark'目标依赖于'LibCarla.release'，可能用于对'LibCarla.release'进行性能测试之类的操作
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --benchmark $(ARGS)
	@cat profiler.csv

smoke_tests:# 'smoke_tests'目标，可能用于进行一些初步的、快速的测试
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --smoke $(ARGS)

examples:# 'examples'目标，可能用于构建示例相关的操作
	@for D in ${CARLA_EXAMPLES_FOLDER}/*; do [ -d "$${D}" ] && make -C $${D} build; done

run-examples:    # 这是一个循环，用于遍历${CARLA_EXAMPLES_FOLDER}下的所有文件（或目录）
	@for D in ${CARLA_EXAMPLES_FOLDER}/*; do # 这部分看起来像是和CarlaUE4Editor相关的构建操作
[ -d "$${D}" ] && make -C $${D} run.only; done
        # 这部分看起来像是和CarlaUE4Editor相关的构建操作
CarlaUE4Editor: LibCarla.server.release osm2odr downloadplugins    # 执行BuildUE4Plugins.sh脚本，--build参数的值由$(ARGS)变量提供
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.sh --build $(ARGS)    # 执行BuildCarlaUE4.sh脚本，--build参数的值由$(ARGS)变量提供
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --build $(ARGS)# 定义一个伪目标（.PHONY），这在Makefile中表示这个目标不是一个真正的文件

.PHONY: PythonAPI# "PythonAPI"是一个目标，依赖于LibCarla.client.release和osm2odr
PythonAPI: LibCarla.client.release osm2odr
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh $(ARGS)
    # 执行BuildPythonAPI.sh脚本，参数由$(ARGS)变量提供
PythonAPI.2: LibCarla.client.release osm2odr# "PythonAPI.2"是一个目标，依赖于LibCarla.client.release和osm2odr
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --python-version=2
        
PythonAPI.3: LibCarla.client.release osm2odr    # 执行BuildPythonAPI.sh脚本，设置python - version = 3
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --python-version=3
# "PythonAPI.rebuild"是一个目标，依赖于LibCarla.client.release和osm2odr
PythonAPI.rebuild: LibCarla.client.release osm2odr
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --rebuild $(ARGS)
# "PythonAPI.rebuild"是一个目标，依赖于LibCarla.client.release和osm2odr
PythonAPI.rss: LibCarla.client.rss.release osm2odr
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --rss $(ARGS)
# "PythonAPI.rss"是一个目标，依赖于LibCarla.client.rss.release和osm2odr
PythonAPI.rss.rebuild: LibCarla.client.rss.release osm2odr
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --rebuild --rss $(ARGS)
# "PythonAPI.rss.rebuild"是一个目标，依赖于LibCarla.client.rss.release和osm2odr
PythonAPI.docs:    # 执行PythonAPI/docs/doc_gen.py脚本，可能用于生成文档相关的操作
	@python PythonAPI/docs/doc_gen.py
	@cd PythonAPI/docs && python3 bp_doc_gen.py
    # 进入PythonAPI/docs目录并执行python3 bp_doc_gen.py脚本，可能也是文档生成相关操作
.PHONY: LibCarla# 定义一个伪目标（.PHONY），名为LibCarla
LibCarla: LibCarla.release LibCarla.debug

LibCarla.debug: LibCarla.server.debug LibCarla.client.debug
LibCarla.release: LibCarla.server.release LibCarla.client.release

LibCarla.server: LibCarla.server.debug LibCarla.server.release
LibCarla.server.debug: setup
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --server --debug $(ARGS)
LibCarla.server.release: setup
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --server --release $(ARGS)

LibCarla.client: LibCarla.client.debug LibCarla.client.release
LibCarla.client.debug: setup
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --client --debug $(ARGS)
LibCarla.client.release: setup
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --client --release $(ARGS)

LibCarla.client.rss: LibCarla.client.rss.debug LibCarla.client.rss.release
LibCarla.client.rss.debug: setup ad-rss
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --client --debug --rss
LibCarla.client.rss.release: setup ad-rss
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --client --release --rss

.PHONY: Plugins
plugins:
	@${CARLA_BUILD_TOOLS_FOLDER}/Plugins.sh $(ARGS)

setup: downloadplugins
	@${CARLA_BUILD_TOOLS_FOLDER}/Setup.sh $(ARGS)

ad-rss:
	@${CARLA_BUILD_TOOLS_FOLDER}/Ad-rss.sh $(ARGS)

deploy:
	@${CARLA_BUILD_TOOLS_FOLDER}/Deploy.sh $(ARGS)

pretty:
	@${CARLA_BUILD_TOOLS_FOLDER}/Prettify.sh $(ARGS)

build.utils: PythonAPI
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildUtilsDocker.sh

osm2odr:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.sh --build $(ARGS)

osmrenderer:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildOSMRenderer.sh

downloadplugins:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.sh --build $(ARGS)
