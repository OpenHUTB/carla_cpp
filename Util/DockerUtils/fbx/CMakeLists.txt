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
target_link_libraries(FBX2OBJ fbxsdk)#这表明目标 FBX20BJ 需要与 fbxsdk 库进行链接。fbxsdk 库可能包含与处理FBX（一种3D数据交换格式）相关的功能，比如读取、写入FBX文件，处理其中的3D模型、动画等数据。
target_link_libraries(FBX2OBJ dl)#将其与 FBX20BJ 链接，可能是因为目标程序在运行时需要动态加载其他模块或者库。
target_link_libraries(FBX2OBJ z)#z 库是与压缩相关的库。如果 FBX20BJ 在处理数据时涉及到压缩或者解压缩操作，比如对FBX文件中的某些数据进行压缩存储或者传输，就需要链接 z 库。

target_link_libraries(FBX2OBJ xml2)#将 xml2 库与 FBX20BJ 链接，意味着 FBX20BJ 可能需要使用 xml2 库来解析或者生成XML内容，以处理与FBX文件相关的XML数据部分。
