cmake_minimum_required(VERSION 3.5.1)
project(CARLA)

include("Build/CMakeLists.txt.in")#包含制定路径下的另一个CMake列表文件

add_subdirectory("LibCarla/cmake")#将指定目录添加子目录进行处理
