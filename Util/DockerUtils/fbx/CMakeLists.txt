# use: ./build/cmake .. -A x64
# Download from: https://www.autodesk.com/content/dam/autodesk/www/adn/fbx/2020-0-1/fbx202001_fbxsdk_linux.tar.gz
# Dependencies:
#	sudo apt-get install libxml2-dev

cmake_minimum_required(VERSION 2.8.9)//设置 CMake 最低版本要求为 2.8.9
project(FBX2OBJ)// 定义项目名称为 FBX2OBJ

set(CMAKE_GENERATOR_PLATFORM x64)

#find_package(LibXml2 REQUIRED)

# include folders
include_directories(src)//包含 src 目录中的头文件
include_directories(dependencies/include)

# library folders
link_directories(dependencies/lib/gcc/x64/release)

# sources
file(GLOB SOURCES "src/*.cpp")//使用文件模式查找 src 目录下所有的 .cpp 文件，并将其保存为 SOURCES 变量
add_executable(FBX2OBJ ${SOURCES})//生成一个名为 FBX2OBJ 的可执行文件，源文件为 ${SOURCES}

# install location
install(TARGETS FBX2OBJ DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../dist")//安装可执行文件到当前源代码目录的上一级目录中的 dist 文件夹

# libraries to link
target_link_libraries(FBX2OBJ fbxsdk)//链接 fbxsdk 库
target_link_libraries(FBX2OBJ dl)//链接 dl 库
target_link_libraries(FBX2OBJ z)//链接 z 库
target_link_libraries(FBX2OBJ xml2)//链接 xml2 库
