# 要求 CMake 的最低版本为 3.5.1。这确保了使用的 CMake 功能在指定版本及以上可用。
# 之所以指定这个最低版本，是因为后续构建项目时可能会使用到该版本及以上的 CMake 所提供的特定功能，
# 只有确保 CMake 版本达到要求，才能保证这些功能正常可用，避免因版本过低而出现不兼容或功能缺失等问题
cmake_minimum_required(VERSION 3.5.1)

project(CARLA)
# 设置项目名称为 CARLA。这将在后续的构建过程中用于识别项目相关的目标、源文件和配置。


include("Build/CMakeLists.txt.in")#包含制定路径下的另一个CMake列表文件

add_subdirectory("LibCarla/cmake")#将指定目录添加子目录进行处理
