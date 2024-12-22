# 假设 CURDIR 是项目的根文件夹，以下变量基于此根目录来定义项目中各部分对应的文件夹路径

# CARLA项目根目录相关的基础文件夹路径定义
# 将项目根目录赋值给 CARLA_ROOT_FOLDER，方便后续以其为基准构建其他相对路径
CARLA_ROOT_FOLDER=${CURDIR}
# 用于存放项目构建过程中产生的各种临时文件、中间文件以及最终构建产物等的文件夹路径
CARLA_BUILD_FOLDER=${CURDIR}/Build
# 项目最终要发布、分发的文件存放的文件夹路径，例如打包好的可执行文件、库以及配置文件等会放在这里
CARLA_DIST_FOLDER=${CURDIR}/Dist
# 存放项目中各种实用工具脚本、辅助程序的文件夹路径，这些工具可用于项目的构建、测试、部署等操作
CARLA_UTIL_FOLDER=${CURDIR}/Util
# 存放与Docker相关的工具脚本或配置文件等的文件夹路径，可能用于辅助项目在Docker环境中的部署、运行等操作
CARLA_DOCKER_UTILS_FOLDER=${CARLA_UTIL_FOLDER}/DockerUtils
# 存放用于构建项目的各种工具脚本的文件夹路径，例如可能有编译脚本、配置生成脚本等
CARLA_BUILD_TOOLS_FOLDER=${CARLA_UTIL_FOLDER}/BuildTools
# 用于存放项目测试结果相关文件的文件夹路径，比如测试报告、测试数据记录等
CARLA_TEST_RESULTS_FOLDER=${CARLA_BUILD_FOLDER}/test-results

# CARLA与Unreal Engine集成部分相关的文件夹路径定义
# CARLA在Unreal Engine中的项目根目录路径，Unreal Engine相关的项目文件、资源文件、插件配置等存放在此目录及其子目录下
CARLAUE4_ROOT_FOLDER=${CURDIR}/Unreal/CarlaUE4
# CARLA在Unreal Engine中的插件根目录路径，插件相关的代码、资源以及依赖库等存放在这里
CARLAUE4_PLUGIN_ROOT_FOLDER=${CURDIR}/Unreal/CarlaUE4/Plugins/Carla
# CARLA项目Python API的根目录路径，外部开发者可通过此API与CARLA进行交互，实现自定义功能开发
CARLA_PYTHONAPI_ROOT_FOLDER=${CURDIR}/PythonAPI
# 进一步细化到CARLA项目Python API的源代码目录路径，具体的Python接口实现代码存放在此处
CARLA_PYTHONAPI_SOURCE_FOLDER=${CARLA_PYTHONAPI_ROOT_FOLDER}/carla

# LibCarla模块相关的文件夹路径定义
# LibCarla模块的根目录路径，可能包含LibCarla相关的源代码、头文件等基础文件
LIBCARLA_ROOT_FOLDER=${CURDIR}/LibCarla
# 用于存放LibCarla服务器端构建相关文件的文件夹路径，例如编译中间文件、生成的服务器可执行文件等
LIBCARLA_BUILD_SERVER_FOLDER=${CARLA_BUILD_FOLDER}/libcarla-server-build
# 推测是与LibCarla和PyTorch集成构建相关的文件夹路径，涉及两者结合编译等操作的中间文件和最终产物存放
LIBCARLA_BUILD_PYTORCH_FOLDER=${CARLA_BUILD_FOLDER}/libcarla-pytorch-build
# 用于存放LibCarla与FastDDS（一种数据分发服务中间件）集成安装相关文件的文件夹路径，如FastDDS的库文件、配置文件等
LIBCARLA_FASTDDS_FOLDER=${CARLA_BUILD_FOLDER}/libcarla-fastdds-install
# 对应LibCarla客户端构建相关文件存放的文件夹路径，例如客户端编译中间文件及最终生成的客户端可执行文件等
LIBCARLA_BUILD_CLIENT_FOLDER=${CARLA_BUILD_FOLDER}/libcarla-client-build
# 指明LibCarla服务器端在CARLA的Unreal Engine插件中安装依赖文件的存放位置，确保插件运行时能找到所需服务器端依赖
LIBCARLA_INSTALL_SERVER_FOLDER=${CARLAUE4_PLUGIN_ROOT_FOLDER}/CarlaDependencies
# 确定LibCarla客户端在Python API源代码目录下依赖文件的存放处，保障Python API调用客户端功能时能正确加载依赖
LIBCARLA_INSTALL_CLIENT_FOLDER=${CARLA_PYTHONAPI_SOURCE_FOLDER}/dependencies

# OSM2ODR（可能与地图数据转换相关）模块相关的文件夹路径定义
# 用于存放OSM2ODR构建过程中文件的文件夹路径，比如编译生成的中间文件、最终可执行文件等
OSM2ODR_BUILD_FOLDER=${CARLA_BUILD_FOLDER}/libosm2dr-build
# 更具体地指向OSM2ODR服务器端构建相关文件存放的文件夹路径，与服务器端构建操作紧密相关
OSM2ODR_SERVER_BUILD_FOLDER=${CARLA_BUILD_FOLDER}/libosm2dr-build-server
# 用于存放OSM2ODR源代码文件的文件夹路径，开发人员可在此基础上进行修改、编译等操作
OSM2ODR_SOURCE_FOLDER=${CARLA_BUILD_FOLDER}/libosm2dr-source

# CARLA在Unreal Engine插件中的依赖文件存放位置相关路径，便于准确获取插件所需的各种依赖资源
CARLAUE4_PLUGIN_DEPS_FOLDER=${CARLAUE4_PLUGIN_ROOT_FOLDER}/CarlaDependencies

# 与C++标准库相关的工具链配置文件路径，在使用CMake进行项目构建时，用于配置C++标准库相关的编译、链接等选项
LIBSTDCPP_TOOLCHAIN_FILE=${CARLA_BUILD_FOLDER}/LibStdCppToolChain.cmake
# 类似地，也是用于CMake构建过程的工具链配置文件路径，可能侧重于特定C++库或编译环境相关配置
LIBCPP_TOOLCHAIN_FILE=${CARLA_BUILD_FOLDER}/LibCppToolChain.cmake
# 指向一个CMake配置文件模板路径，实际构建中可能基于此模板生成具体的 CMakeLists.txt 文件，用于配置项目构建规则等
CMAKE_CONFIG_FILE=${CARLA_BUILD_FOLDER}/CMakeLists.txt.in

# 用于存放项目测试过程中所需的各种测试数据、测试用例相关文件的文件夹路径，确保测试的完整性和准确性
CARLA_TEST_CONTENT_FOLDER=${CARLA_BUILD_FOLDER}/test-content
# 存放项目相关示例代码文件的文件夹路径，方便开发者参考示例来了解如何使用项目功能、接口等
CARLA_EXAMPLES_FOLDER=${CURDIR}/Examples

# CARLA在Unreal Engine中添加插件的根目录路径，除CARLA本身插件外，其他扩展插件可放在此目录管理
CARLAUE4_ADDPLUGINS_FOLDER=${CURDIR}/Unreal/CarlaUE4/Plugins
# 更具体地指向与街道地图相关插件（如果有）的文件夹路径，用于存放该插件相关的代码、资源等内容
CARLAUE4_STREETMAP_FOLDER=${CARLAUE4_ADDPLUGINS_FOLDER}/Streetmap
