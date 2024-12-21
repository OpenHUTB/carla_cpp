# Here CURDIR is assumed to be the root folder of the project.
# 以下假设CURDIR为项目的根目录，基于此来定义一系列与项目各部分相关的目录路径变量。

# 定义CARLA项目根目录的路径变量，其值等同于CURDIR，代表整个CARLA项目所在的根目录位置。
CARLA_ROOT_FOLDER=${CURDIR}

# 定义CARLA项目构建文件夹的路径变量，位于项目根目录下的Build文件夹，通常用于存放构建过程中生成的各类文件等。
CARLA_BUILD_FOLDER=${CURDIR}/Build

# 定义CARLA项目分发文件夹的路径变量，位于项目根目录下的Dist文件夹，可能用于存放最终可供分发的项目文件、安装包等内容。
CARLA_DIST_FOLDER=${CURDIR}/Dist

# 定义CARLA项目工具文件夹的路径变量，位于项目根目录下的Util文件夹，用于存放项目相关的各种实用工具相关的文件等。
CARLA_UTIL_FOLDER=${CURDIR}/Util

# 定义CARLA项目中Docker工具相关的文件夹路径变量，位于CARLA_UTIL_FOLDER下的DockerUtils文件夹，存放与Docker相关的工具脚本、配置等内容，用于和Docker相关的操作（如构建镜像等可能会用到）。
CARLA_DOCKER_UTILS_FOLDER=${CARLA_UTIL_FOLDER}/DockerUtils

# 定义CARLA项目构建工具相关的文件夹路径变量，位于CARLA_UTIL_FOLDER下的BuildTools文件夹，存放用于项目构建的特定工具、脚本等，辅助项目的编译构建过程。
CARLA_BUILD_TOOLS_FOLDER=${CARLA_UTIL_FOLDER}/BuildTools

# 定义CARLA项目测试结果存放文件夹的路径变量，位于CARLA_BUILD_FOLDER下的test-results文件夹，构建过程中运行测试后产生的结果文件（如测试报告等）会存放在此目录下。
CARLA_TEST_RESULTS_FOLDER=${CARLA_BUILD_FOLDER}/test-results

# 定义CarlaUE4项目根目录的路径变量，位于项目根目录（CURDIR）下的Unreal/CarlaUE4文件夹，是CarlaUE4相关文件、资源等的根目录位置，比如UE4项目的配置、源文件等在此目录下有相关内容。
CARLAUE4_ROOT_FOLDER=${CURDIR}/Unreal/CarlaUE4

# 定义CarlaUE4插件根目录的路径变量，位于项目根目录（CURDIR）下的Unreal/CarlaUE4/Plugins/Carla文件夹，是Carla插件相关的文件、资源等所在的根目录，比如插件的代码、配置、依赖库等相关内容存放在这里。
CARLAUE4_PLUGIN_ROOT_FOLDER=${CURDIR}/Unreal/CarlaUE4/Plugins/Carla

# 定义CARLA Python API项目根目录的路径变量，位于项目根目录（CURDIR）下的PythonAPI文件夹，存放Python API相关的代码文件、模块等内容，用于实现Python与CARLA交互的接口功能。
CARLA_PYTHONAPI_ROOT_FOLDER=${CURDIR}/PythonAPI

# 定义CARLA Python API源代码文件夹的路径变量，位于CARLA_PYTHONAPI_ROOT_FOLDER下的carla文件夹，是Python API具体代码实现所在的文件夹，里面包含具体的Python模块、类、函数等定义。
CARLA_PYTHONAPI_SOURCE_FOLDER=${CARLA_PYTHONAPI_ROOT_FOLDER}/carla

# 定义LibCarla项目根目录的路径变量，位于项目根目录（CURDIR）下的LibCarla文件夹，存放LibCarla相关的代码文件、资源等，它是CARLA中很重要的一个库部分，可能用于实现核心功能等。
LIBCARLA_ROOT_FOLDER=${CURDIR}/LibCarla

# 定义LibCarla服务器端构建文件夹的路径变量，位于CARLA_BUILD_FOLDER下的libcarla-server-build文件夹，专门用于存放构建LibCarla服务器端相关过程中生成的文件，比如编译产生的中间文件、目标文件等。
LIBCARLA_BUILD_SERVER_FOLDER=${CARLA_BUILD_FOLDER}/libcarla-server-build

# 定义LibCarla与PyTorch相关构建文件夹的路径变量，位于CARLA_BUILD_FOLDER下的libcarla-pytorch-build文件夹，用于存放和LibCarla结合PyTorch使用时构建过程产生的相关文件，可能涉及到两者集成相关的编译、配置等操作的中间产物存放。
LIBCARLA_BUILD_PYTORCH_FOLDER=${CARLA_BUILD_FOLDER}/libcarla-pytorch-build

# 定义LibCarla与FastDDS相关的文件夹路径变量，位于CARLA_BUILD_FOLDER下的libcarla-fastdds-install文件夹，也许用于存放LibCarla集成FastDDS库相关的安装文件、配置文件等内容，和其使用FastDDS实现某些通信等功能相关。
LIBCARLA_FASTDDS_FOLDER=${CARLA_BUILD_FOLDER}/libcarla-fastdds-install

# 定义LibCarla客户端构建文件夹的路径变量，位于CARLA_BUILD_FOLDER下的libcarla-client-build文件夹，用于存放构建LibCarla客户端部分过程中生成的文件，类似服务器端构建文件夹，不过针对客户端相关构建产物存放。
LIBCARLA_BUILD_CLIENT_FOLDER=${CARLA_BUILD_FOLDER}/libcarla-client-build

# 定义LibCarla服务器端安装文件夹的路径变量，位于CARLAUE4_PLUGIN_ROOT_FOLDER下的CarlaDependencies文件夹，这里存放的是LibCarla服务器端在CarlaUE4插件环境下安装所需要的依赖文件、库等内容，确保服务器端在插件中能正常运行。
LIBCARLA_INSTALL_SERVER_FOLDER=${CARLAUE4_PLUGIN_ROOT_FOLDER}/CarlaDependencies

# 定义LibCarla客户端安装文件夹的路径变量，位于CARLA_PYTHONAPI_SOURCE_FOLDER下的dependencies文件夹，用于存放LibCarla客户端在Python API环境下能正常运行所需要的依赖文件、库等内容，保障客户端能与Python API协同工作。
LIBCARLA_INSTALL_CLIENT_FOLDER=${CARLA_PYTHONAPI_SOURCE_FOLDER}/dependencies

# 定义OSM2ODR项目构建文件夹的路径变量，位于CARLA_BUILD_FOLDER下的libosm2dr-build文件夹，用于存放OSM2ODR项目构建过程产生的相关文件，OSM2ODR可能是和地图数据转换等相关的一个项目模块（具体取决于CARLA项目本身的功能设定）。
OSM2ODR_BUILD_FOLDER=${CARLA_BUILD_FOLDER}/libosm2dr-build

# 定义OSM2ODR服务器端构建文件夹的路径变量，位于CARLA_BUILD_FOLDER下的libosm2dr-build-server文件夹，专门针对OSM2ODR项目服务器端构建过程中生成的文件存放，与普通构建文件夹区分开来可能是因为服务器端有特定的构建需求和配置。
OSM2ODR_SERVER_BUILD_FOLDER=${CARLA_BUILD_FOLDER}/libosm2dr-build-server

# 定义OSM2ODR项目源代码文件夹的路径变量，位于CARLA_BUILD_FOLDER下的libosm2dr-source文件夹，存放OSM2ODR项目具体的源代码文件，开发人员会在这里进行代码的编写、修改等操作。
OSM2ODR_SOURCE_FOLDER=${CARLA_BUILD_FOLDER}/libosm2dr-source

# 定义CarlaUE4插件依赖文件夹的路径变量，等同于CARLAUE4_PLUGIN_ROOT_FOLDER下的CarlaDependencies文件夹，再次强调此文件夹存放CarlaUE4插件所依赖的各种文件、库等内容，方便统一管理插件的依赖关系。
CARLAUE4_PLUGIN_DEPS_FOLDER=${CARLAUE4_PLUGIN_ROOT_FOLDER}/CarlaDependencies

# 定义用于LibStdCpp相关的工具链文件的路径变量，位于CARLA_BUILD_FOLDER下的LibStdCppToolChain.cmake文件，这个文件可能定义了使用LibStdCpp时的编译、链接等构建相关的配置信息，供构建系统（如CMake等）读取使用。
LIBSTDCPP_TOOLCHAIN_FILE=${CARLA_BUILD_FOLDER}/LibStdCppToolChain.cmake

# 定义用于LibCpp相关的工具链文件的路径变量，位于CARLA_BUILD_FOLDER下的LibCppToolChain.cmake文件，类似LibStdCpp的工具链文件，它定义了LibCpp相关的构建配置信息，用于指导项目在编译构建过程中如何处理LibCpp相关的部分。
LIBCPP_TOOLCHAIN_FILE=${CARLA_BUILD_FOLDER}/LibCppToolChain.cmake

# 定义CMake配置文件的路径变量，位于CARLA_BUILD_FOLDER下的CMakeLists.txt.in文件，这是一个CMake的配置模板文件（通常以.in后缀表示模板文件），可能会根据项目的实际情况生成最终的CMakeLists.txt配置文件来指导整个项目的构建过程。
CMAKE_CONFIG_FILE=${CARLA_BUILD_FOLDER}/CMakeLists.txt.in

# 定义LibCarla测试内容相关的文件夹路径变量，位于CARLA_BUILD_FOLDER下的test-content文件夹，存放LibCarla进行各种测试时所需要用到的测试数据、配置等内容，辅助完成测试流程并确保测试的准确性和完整性。
LIBCARLA_TEST_CONTENT_FOLDER=${CARLA_BUILD_FOLDER}/test-content

# 定义CARLA项目示例文件夹的路径变量，位于项目根目录（CURDIR）下的Examples文件夹，存放一些示例代码、演示项目等内容，方便开发人员或者使用者参考学习如何使用CARLA的相关功能，比如展示如何调用API实现特定功能等。
CARLA_EXAMPLES_FOLDER=${CURDIR}/Examples

# 定义CarlaUE4添加插件相关的文件夹路径变量，位于项目根目录（CURDIR）下的Unreal/CarlaUE4/Plugins文件夹，是在CarlaUE4项目中添加各种插件的相关操作（如安装、配置插件等）所涉及的文件夹，里面存放插件相关的文件以及和UE4项目集成插件的相关配置等内容。
CARLAUE4_ADDPLUGINS_FOLDER=${CURDIR}/Unreal/CarlaUE4/Plugins

# 定义CarlaUE4街道地图相关的文件夹路径变量，位于CARLAUE4_ADDPLUGINS_FOLDER下的Streetmap文件夹，可能存放与CarlaUE4中街道地图相关的资源文件、配置文件等内容，用于实现地图相关的功能展示、交互等在UE4环境下的呈现。
CARLAUE4_STREETMAP_FOLDER=${CARLAUE4_ADDPLUGINS_FOLDER}/Streetmap
