# 设置默认参数，这里默认的参数值为 --all，后续的命令可能会使用到这个参数
ARGS=--all

# 将 help 设置为默认目标，当直接运行 make 命令时，如果没有指定其他目标，就会执行 help 对应的规则
default: help

# 定义项目的根目录（也就是 Makefile 所在的目录），通过 $(CURDIR) 获取当前目录路径，并在末尾添加 /
# 同时将其导出为环境变量 ROOT_PATH，方便后续其他规则中引用这个路径变量
export ROOT_PATH=$(CURDIR)/

# 定义依赖项的安装/构建目录（例如 rpclib、gtest、boost 等依赖项相关的构建和安装路径）
# 路径基于项目根目录（ROOT_PATH）下的 Build/ 目录，并将其导出为环境变量 INSTALLATION_DIR，供后续使用
export INSTALLATION_DIR=$(ROOT_PATH)Build/

# help 目标的规则，用于显示帮助信息
# 通过 @type 命令调用 "${CARLA_BUILD_TOOLS_FOLDER}\Windows.mk.help" 文件来展示帮助内容
# @ 符号表示在执行命令时不显示命令本身，只显示命令的输出结果
help:
    @type "${CARLA_BUILD_TOOLS_FOLDER}\Windows.mk.help"

# 使用.PHONY 声明 import 为伪目标，这意味着即使存在与 import 同名的文件或目录，make 也会将其当作一个命令来执行，而不是当作文件依赖去处理
# import 目标依赖于 server 目标，当执行 import 时，会先执行 server 目标对应的规则，然后执行 "${CARLA_BUILD_TOOLS_FOLDER}/Import.py" 脚本，并传入 $(ARGS) 参数
.PHONY: import
import: server
    @"${CARLA_BUILD_TOOLS_FOLDER}/Import.py" $(ARGS)

# CarlaUE4Editor 目标依赖于 LibCarla 和 osm2odr 目标，在执行时会先执行这两个依赖目标对应的规则
# 然后执行 "${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" 脚本，并传入 --build 和 $(ARGS) 参数，可能用于构建 CarlaUE4Editor 相关的内容
CarlaUE4Editor: LibCarla osm2odr
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --build $(ARGS)

# launch 目标依赖于 CarlaUE4Editor 目标，执行时先执行 CarlaUE4Editor 对应的规则，然后执行 "${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" 脚本，传入 --launch 和 $(ARGS) 参数，可能用于启动相关程序或服务
launch: CarlaUE4Editor
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --launch $(ARGS)

# launch-only 目标，直接执行 "${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" 脚本，传入 --launch 和 $(ARGS) 参数，功能上可能和 launch 目标有相似之处，但具体依赖关系不同
launch-only:
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --launch $(ARGS)

# package 目标依赖于 PythonAPI 目标，执行顺序上先执行 PythonAPI 对应的规则
# 然后依次执行 "${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" 脚本（传入特定参数）和 "${CARLA_BUILD_TOOLS_FOLDER}/Package.bat" 脚本（传入 --ue-version 4.26 和 $(ARGS) 参数），可能用于打包相关项目，并且指定了 Unreal Engine 的版本为 4.26
package: PythonAPI
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --at-least-write-optionalmodules $(ARGS)
    @"${CARLA_BUILD_TOOLS_FOLDER}/Package.bat" --ue-version 4.26 $(ARGS)

# 声明 docs 为伪目标，docs 目标的规则先执行 doxygen 命令（可能用于生成文档相关操作），然后输出提示信息，告知文档索引所在的路径
.PHONY: docs
docs:
    @doxygen
    @echo "Documentation index at./Doxygen/html/index.html"

# PythonAPI.docs 目标，先执行 PythonAPI/docs/doc_gen.py 脚本（可能用于生成 PythonAPI 相关文档），然后切换到 PythonAPI/docs 目录下执行 bp_doc_gen.py 脚本，具体功能取决于这两个脚本的实现内容
PythonAPI.docs:
    python PythonAPI/docs/doc_gen.py
    cd PythonAPI/docs && python bp_doc_gen.py

# clean 目标，用于清理相关的构建产物等内容，依次执行多个.bat 脚本，并传入对应的清理相关参数，对不同的项目组件（如 Package、CarlaUE4Editor、PythonAPI、LibCarla、OSM2ODR 等）进行清理操作
clean:
    @"${CARLA_BUILD_TOOLS_FOLDER}/Package.bat" --clean --ue-version 4.26
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --clean
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.bat" --clean
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --clean
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.bat" --clean

# rebuild 目标依赖于 setup 目标，执行顺序上先执行 setup 对应的规则
# 然后依次执行多个.bat 脚本（针对 CarlaUE4Editor、LibCarla、OSM2ODR、PythonAPI 等项目组件）并传入 --rebuild 参数，用于重新构建这些组件
rebuild: setup
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --rebuild
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --rebuild
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.bat" --rebuild
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.bat" --rebuild

# check 目标依赖于 PythonAPI 目标，执行顺序上先执行 PythonAPI 对应的规则，不过目前规则中的命令只是输出提示信息，表示该功能尚未实现
check: PythonAPI
    @echo "Not implemented!"

# benchmark 目标依赖于 LibCarla 目标，执行顺序上先执行 LibCarla 对应的规则，同样目前规则中的命令只是输出提示信息，表示该功能尚未实现
benchmark: LibCarla
    @echo "Not implemented!"

# 声明 PythonAPI 为伪目标，PythonAPI 目标依赖于 LibCarla 和 osm2odr 目标，执行顺序上先执行这两个依赖目标对应的规则
# 然后执行 "${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.bat" 脚本，并传入 --py3 参数，可能用于构建 PythonAPI 相关内容，并且指定了 Python 版本相关的一些配置（这里可能表示 Python 3）
.PHONY: PythonAPI
PythonAPI: LibCarla osm2odr
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.bat" --py3

# server 目标依赖于 setup 目标，执行顺序上先执行 setup 对应的规则，然后执行 "${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" 脚本，传入 --server 和 --generator "$(GENERATOR)" 参数，可能用于构建服务器相关的组件，并指定生成器相关配置
server: setup
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --server --generator "$(GENERATOR)"

# client 目标依赖于 setup 目标，执行顺序上先执行 setup 对应的规则，然后执行 "${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" 脚本，传入 --client 和 --generator "$(GENERATOR)" 参数，可能用于构建客户端相关的组件，并指定生成器相关配置
client: setup
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --client --generator "$(GENERATOR)"

# 声明 LibCarla 为伪目标，LibCarla 目标依赖于 setup 目标，执行顺序上先执行 setup 对应的规则
# 然后执行 "${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" 脚本，传入 --server、--client 和 --generator "$(GENERATOR)" 参数，可能用于构建 LibCarla 相关内容，同时涉及服务器和客户端相关的配置以及指定生成器相关内容
.PHONY: LibCarla
LibCarla: setup
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --server --client --generator "$(GENERATOR)"

# setup 目标依赖于 downloadplugin 目标，执行顺序上先执行 downloadplugin 对应的规则
# 然后执行 "${CARLA_BUILD_TOOLS_FOLDER}/Setup.bat" 脚本，传入 --boost-toolset msvc-14.2、--generator "$(GENERATOR)" 和 $(ARGS) 参数，可能用于项目的初始化设置相关操作，比如配置 boost 工具集、指定生成器以及传入其他通用参数等
setup: downloadplugin
    @"${CARLA_BUILD_TOOLS_FOLDER}/Setup.bat" --boost-toolset msvc-14.2 --generator "$(GENERATOR)" $(ARGS)

# 声明 Plugins 为伪目标，执行 "${CARLA_BUILD_TOOLS_FOLDER}/Plugins.bat" 脚本，并传入 $(ARGS) 参数，可能用于处理插件相关的操作
.PHONY: Plugins
plugins:
    @"${CARLA_BUILD_TOOLS_FOLDER}/Plugins.bat" $(ARGS)

# deploy 目标，执行 "${CARLA_BUILD_TOOLS_FOLDER}/Deploy.bat" 脚本，并传入 $(ARGS) 参数，可能用于项目的部署相关操作
deploy:
    @"${CARLA_BUILD_TOOLS_FOLDER}/Deploy.bat" $(ARGS)

# osm2odr 目标，执行 "${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.bat" 脚本，传入 --generator "$(GENERATOR)" 和 --build $(ARGS) 参数，可能用于构建与 osm2odr 相关的内容，并指定生成器相关配置以及其他通用构建参数
osm2odr:
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildOSM2ODR.bat" --generator "$(GENERATOR)" --build $(ARGS)

# osmrenderer 目标，执行 "${CARLA_BUILD_TOOLS_FOLDER}/BuildOSMRenderer.bat" 脚本，具体功能取决于该脚本的实现内容，可能用于构建与 osm 渲染相关的功能模块
osmrenderer:
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildOSMRenderer.bat"

# downloadplugin 目标，执行 "${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.bat" 脚本，并传入 --build $(ARGS) 参数，可能用于下载插件相关的操作，并传入构建相关的通用参数
downloadplugin:
    @"${CARLA_BUILD_TOOLS_FOLDER}/BuildUE4Plugins.bat" --build $(ARGS)
