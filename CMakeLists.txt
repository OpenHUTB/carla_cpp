# 要求 CMake 的最低版本为 3.5.1。这确保了使用的 CMake 功能在指定版本及以上可用。
cmake_minimum_required(VERSION 3.5.1)

project(CARLA)
# 设置项目名称为 CARLA。这将在后续的构建过程中用于识别项目相关的目标、源文件和配置。


include("Build/CMakeLists.txt.in")#包含制定路径下的另一个CMake列表文件# 包含指定路径下的另一个 CMake 列表文件，即 "Build/CMakeLists.txt.in"。
# 这样做通常是为了复用一些通用的 CMake 配置代码或者引入额外的构建规则、依赖设置等内容。
# 比如该被包含的文件中可能定义了一些通用的编译选项、源文件分组规则、第三方库的引入方式等，
# 通过包含这个文件，就可以把其中的配置整合到当前的构建体系中来，避免在当前 CMakeLists.txt 文件中重复编写相同的配置代码，
# 使得整个项目的构建配置更加模块化和易于维护。

add_subdirectory("LibCarla/cmake")#将指定目录添加子目录进行处理
