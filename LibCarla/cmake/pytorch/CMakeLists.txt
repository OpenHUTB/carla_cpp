# 设置CMake的最低版本要求
cmake_minimum_required(VERSION 3.5.1)

# 定义项目名称为libcarla-pytorch
project(libcarla-pytorch)

# 使用GLOB命令获取carla/pytorch目录下的所有头文件，存储在变量libcarla_carla_pytorch_headers中
file(GLOB libcarla_carla_pytorch_headers "${libcarla_source_pathcarla/pytorch/*.h")

# 安装获取的头文件到指定的目标目录include/carla/pytorch
install(FILES ${libcarla_carla_pytorch_headers DESTINATION include/carla/pytorch)

# 使用GLOB命令获取carla/pytorch目录下的所有源文件（包括.h和.cpp），存储在变量libcarla_pytorch_sources中
file(GLOB libcarla_pytorch_sources
    "${libcarla_source_pathcarla/pytorch/*.h"
    "${libcarla_source_pathcarla/pytorch/*.cpp")

# 设置CUDA编译器的路径
set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)

# 查找并引入PyTorch库，这些库是构建本项目所必需的
find_package(Torch REQUIRED)

# 查找并引入TorchScatter库，这个库扩展了PyTorch的功能，提供了散射操作
find_package(TorchScatter REQUIRED)

# 查找并引入TorchCluster库，提供了图神经网络的聚类方法
find_package(TorchCluster REQUIRED)

# 查找并引入Python3库，以便在项目中使用Python功能和绑定
find_package(Python3 REQUIRED)

set(PYTORCH_CPP_STD_INCLUDES "/usr/include/c++/7")

set(CMAKE_CXX_FLAGS_RELEASE "-DDEBUG -std=c++14 -O2 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -I${PYTORCH_CPP_STD_INCLUDES}" CACHE STRING "" FORCE)
 

# 如果定义了LIBCARLA_BUILD_RELEASE（表示正在进行释放构建），则执行以下代码
if (LIBCARLA_BUILD_RELEASE)
  # 创建一个静态库carla_pytorch，源文件为libcarla_pytorch_sources中定义的文件
  add_library(carla_pytorch STATIC ${libcarla_pytorch_sources  # 添加包含目录，SYSTEM标识告诉编译器这些路径包含的是系统头文件，使用PRIVATE表示这些目录仅在该库中可见  
  target_include_directories(carla_pytorch SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH  # Boost库的包含目录
      "${RPCLIB_INCLUDE_PATH # RPCLIB库的包含目录

  # 继续添加私有包含目录，使用系统标识
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TORCH_INCLUDE_DIRS 
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchScatter_INCLUDE_DIR 
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchCluster_INCLUDE_DIR 
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchSparse_INCLUDE_DIR 
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${Python3_INCLUDE_DIRS 

  # 链接目标库carla_pytorch与PyTorch库，确保使用PyTorch的功能
  target_link_libraries(carla_pytorch "${TORCH_LIBRARIES 

  # 安装carla_pytorch库到lib目录，OPTIONAL表示如果安装失败也不会终止整个构建
  install(TARGETS carla_pytorch DESTINATION lib OPTIONAL) 

  # 设置目标属性，定义在释放模式下的编译标志
  set_target_properties(carla_pytorch PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASEendif()
if (LIBCARLA_BUILD_DEBUG)
  # 如果定义了LIBCARLA_BUILD_DEBUG（表示正在进行调试构建），则执行以下代码

  # 创建一个静态库carla_pytorch_debug，源文件为libcarla_pytorch_sources中定义的文件
  add_library(carla_pytorch_debug STATIC ${libcarla_pytorch_sources  # 添加包含目录，SYSTEM标识告诉编译器这些路径包含的是系统头文件，使用PRIVATE表示这些目录仅在该库中可见 
  target_include_directories(carla_pytorch_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH  # Boost库的包含目录
      "${RPCLIB_INCLUDE_PATH # RPCLIB库的包含目录

  # 安装目标carla_pytorch_debug库到lib目录，OPTIONAL表示如果安装失败也不会终止整个构建
  install(TARGETS carla_pytorch_debug DESTINATION lib OPTIONAL)

  # 设置carla_pytorch_debug库的编译属性，定义在调试模式下的编译标志
  set_target_properties(carla_pytorch_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG  # 为目标carla_pytorch_debug添加编译定义，启用Boost.Asio的缓冲区调试功能
  target_compile_definitions(carla_pytorch_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUG)

endif()
