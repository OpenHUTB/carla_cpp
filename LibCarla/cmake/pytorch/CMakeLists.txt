cmake_minimum_required(VERSION 3.5.1)# 设置CMake的最低版本要求
project(libcarla-pytorch)# 定义项目名称 

# 安装头文件  
# 使用file(GLOB)命令查找所有.h头文件，并安装到include/carla/pytorch目录下

file(GLOB libcarla_carla_pytorch_headers "${libcarla_source_path}/carla/pytorch/*.h")
install(FILES ${libcarla_carla_pytorch_headers} DESTINATION include/carla/pytorch)

# 定义libcarla_pytorch库的源文件  
# 包括.h和.cpp文件
file(GLOB libcarla_pytorch_sources
    "${libcarla_source_path}/carla/pytorch/*.h"
    "${libcarla_source_path}/carla/pytorch/*.cpp")

set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)# 设置CUDA编译器路径
# 查找并要求Torch、TorchScatter、TorchCluster和Python3包
find_package(Torch REQUIRED)
find_package(TorchScatter REQUIRED)
find_package(TorchCluster REQUIRED)
find_package(Python3 REQUIRED)

set(PYTORCH_CPP_STD_INCLUDES "/usr/include/c++/7")# 设置PyTorch C++ API的标准库包含路径

# @todo These flags need to be compatible with setup.py compilation.
set(CMAKE_CXX_FLAGS_RELEASE "-DDEBUG -std=c++14 -O2 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -I${PYTORCH_CPP_STD_INCLUDES}" CACHE STRING "" FORCE)
 
# ==============================================================================
# Create targets for debug and release in the same build type.
# ==============================================================================
# 根据是否定义了LIBCARLA_BUILD_RELEASE或LIBCARLA_BUILD_DEBUG来构建不同的目标
if (LIBCARLA_BUILD_RELEASE)
 # 添加静态库carla_pytorch
  add_library(carla_pytorch STATIC ${libcarla_pytorch_sources})
  # 为carla_pytorch库添加包含目录
  target_include_directories(carla_pytorch SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")
# 添加PyTorch及其依赖库的包含目录
# 添加PyTorch库的头文件路径
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TORCH_INCLUDE_DIRS}")
  # 添加TorchScatter库的头文件路径，TorchScatter是用于高效图操作的扩展库
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchScatter_INCLUDE_DIR}")
  # 添加TorchCluster库的头文件路径，TorchCluster专注于聚类算法的实现
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchCluster_INCLUDE_DIR}")
  # 添加TorchSparse库的头文件路径，TorchSparse提供了稀疏张量的支持
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchSparse_INCLUDE_DIR}")
  # 添加Python3的头文件路径，以便于与Python进行交互和使用Python API
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${Python3_INCLUDE_DIRS}")
  # 链接PyTorch库到carla_pytorch目标
# 这里假设${TORCH_LIBRARIES}变量包含了所有需要链接的PyTorch相关库
# 通过这种方式，确保在编译和链接时能够找到并正确引用PyTorch库
  target_link_libraries(carla_pytorch "${TORCH_LIBRARIES}") # 链接PyTorch库

  install(TARGETS carla_pytorch DESTINATION lib OPTIONAL) # 安装carla_pytorch库

  set_target_properties(carla_pytorch PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")# 设置carla_pytorch的编译标志
#链接库设置
endif()

if (LIBCARLA_BUILD_DEBUG)

  add_library(carla_pytorch_debug STATIC ${libcarla_pytorch_sources})
#命令来添加调试版本的源文件和包含目录
  target_include_directories(carla_pytorch_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  install(TARGETS carla_pytorch_debug DESTINATION lib OPTIONAL)#命令来指定调试版本库的安装目标
#命令来设置调试版本库的编译属性
  set_target_properties(carla_pytorch_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  target_compile_definitions(carla_pytorch_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)# 为目标 carla_pytorch_debug 添加编译定义


endif()
