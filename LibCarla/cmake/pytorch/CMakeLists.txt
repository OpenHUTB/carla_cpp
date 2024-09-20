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
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TORCH_INCLUDE_DIRS}")
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchScatter_INCLUDE_DIR}")
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchCluster_INCLUDE_DIR}")
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchSparse_INCLUDE_DIR}")
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${Python3_INCLUDE_DIRS}")
  target_link_libraries(carla_pytorch "${TORCH_LIBRARIES}") # 链接PyTorch库

  install(TARGETS carla_pytorch DESTINATION lib OPTIONAL) # 安装carla_pytorch库

  set_target_properties(carla_pytorch PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")# 设置carla_pytorch的编译标志

endif()

# 检查是否定义了LIBCARLA_BUILD_DEBUG变量，并且其值为真（通常是通过在CMake命令行中设置-DLIBCARLA_BUILD_DEBUG=ON或者在CMakeLists.txt中提前定义）
if (LIBCARLA_BUILD_DEBUG)

# 添加一个静态库目标，名为carla_pytorch_debug，它由变量libcarla_pytorch_sources指定的源文件构建而成  
# 这些源文件可能包含与PyTorch集成的CARLA相关代码
  add_library(carla_pytorch_debug STATIC ${libcarla_pytorch_sources})
  
  # 为carla_pytorch_debug目标设置包含目录。这些目录是编译时查找头文件所需的  
  # BOOST_INCLUDE_PATH和RPCLIB_INCLUDE_PATH分别包含了Boost和RpcLib库的头文件路径  
  # SYSTEM关键字表示这些路径中的文件被视为系统文件，这可能会影响编译器的警告和错误消息  
  # PRIVATE表示这些包含目录仅对carla_pytorch_debug目标本身有效，不会传递给依赖该目标的其他目标
  target_include_directories(carla_pytorch_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")
  # 安装carla_pytorch_debug库到指定的目录（lib），OPTIONAL表示这个安装步骤是可选的，即如果安装过程中出现错误，不会停止整个CMake配置过程
  install(TARGETS carla_pytorch_debug DESTINATION lib OPTIONAL)

   # 设置carla_pytorch_debug目标的编译标志，这里使用CMake的变量CMAKE_CXX_FLAGS_DEBUG，该变量通常包含了在调试模式下编译C++代码所需的标志
  set_target_properties(carla_pytorch_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  
  # 为carla_pytorch_debug目标添加编译定义。这里添加了-DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING，这是一个预处理器定义  
  # 它可能会使Boost.Asio库在调试时输出更多的调试信息或启用额外的检查  
  # PUBLIC表示这个编译定义不仅会影响carla_pytorch_debug目标本身，还会传递给依赖该目标的其他目标  
  target_compile_definitions(carla_pytorch_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

endif()
