# use: ./build/cmake .. -A x64
# Download from: https://www.autodesk.com/content/dam/autodesk/www/adn/fbx/2020-0-1/fbx202001_fbxsdk_linux.tar.gz
# Dependencies:
#	sudo apt-get install libxml2-dev
# 指定 CMake 的最低版本要求为 2.8.9
cmake_minimum_required(VERSION 2.8.9)
# 定义项目名称为 FBX2OBJ
project(FBX2OBJ)
# 设置生成器平台为 x64
set(CMAKE_GENERATOR_PLATFORM x64)
# 找到 LibXml2 包，此行被注释掉了，如果需要可以取消注释
#find_package(LibXml2 REQUIRED)

# 添加 include 文件夹
include_directories(src)
include_directories(dependencies/include)

# 添加库文件夹
link_directories(dependencies/lib/gcc/x64/release)

# 收集 src 目录下的所有 cpp 文件作为源文件
file(GLOB SOURCES "src/*.cpp")
# 添加一个名为 FBX2OBJ 的可执行文件，源文件为上面收集到的 SOURCES
add_executable(FBX2OBJ ${SOURCES})

# 安装目标，将 FBX2OBJ 安装到 ${CMAKE_CURRENT_SOURCE_DIR}/../dist 目录下
install(TARGETS FBX2OBJ DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../dist")

# 链接需要的库文件
target_link_libraries(FBX2OBJ fbxsdk)
target_link_libraries(FBX2OBJ dl)
target_link_libraries(FBX2OBJ z)
target_link_libraries(FBX2OBJ xml2)
