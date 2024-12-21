cmake_minimum_required(VERSION 3.5.1)
project(libcarla-pytorch)

# Install headers.

file(GLOB libcarla_carla_pytorch_headers "${libcarla_source_path}/carla/pytorch/*.h")
install(FILES ${libcarla_carla_pytorch_headers} DESTINATION include/carla/pytorch)

# carla_pytorch library.

file(GLOB libcarla_pytorch_sources
    "${libcarla_source_path}/carla/pytorch/*.h"
    "${libcarla_source_path}/carla/pytorch/*.cpp")

set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)
find_package(Torch REQUIRED)
find_package(TorchScatter REQUIRED)
find_package(TorchCluster REQUIRED)
find_package(Python3 REQUIRED)

set(PYTORCH_CPP_STD_INCLUDES "/usr/include/c++/7")

# @todo These flags need to be compatible with setup.py compilation.
set(CMAKE_CXX_FLAGS_RELEASE "-DDEBUG -std=c++14 -O2 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -I${PYTORCH_CPP_STD_INCLUDES}" CACHE STRING "" FORCE)
 
# ==============================================================================
# Create targets for debug and release in the same build type.
# ==============================================================================

if (LIBCARLA_BUILD_RELEASE)
# 检查是否为LIBCARLA_BUILD_RELEASE构建配置
  add_library(carla_pytorch STATIC ${libcarla_pytorch_sources})
# 如果是LIBCARLA_BUILD_RELEASE配置，则创建一个名为carla_pytorch的静态库
    # 该库将包含所有在变量libcarla_pytorch_sources中列出的源文件
  target_include_directories(carla_pytorch SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")
//为目标carla_pytorch添加包含目录
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TORCH_INCLUDE_DIRS}")
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchScatter_INCLUDE_DIR}")
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchCluster_INCLUDE_DIR}")
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchSparse_INCLUDE_DIR}")
  target_include_directories(carla_pytorch PRIVATE SYSTEM "${Python3_INCLUDE_DIRS}")
  target_link_libraries(carla_pytorch "${TORCH_LIBRARIES}")

  install(TARGETS carla_pytorch DESTINATION lib OPTIONAL)
# 安装carla_pytorch目标到lib目录，可选安装
  set_target_properties(carla_pytorch PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
# 设置carla_pytorch目标的编译标志为CMAKE_CXX_FLAGS_RELEASE
endif()
# 结束LIBCARLA_BUILD_RELEASE配置的代码块
if (LIBCARLA_BUILD_DEBUG)
# 如果是LIBCARLA_BUILD_DEBUG配置
  add_library(carla_pytorch_debug STATIC ${libcarla_pytorch_sources})
# 创建carla_pytorch_debug静态
  target_include_directories(carla_pytorch_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")
# 为目标carla_pytorch_debug添加包含目录
  install(TARGETS carla_pytorch_debug DESTINATION lib OPTIONAL)
# 安装carla_pytorch_debug目标到lib目录，可选安装
  set_target_properties(carla_pytorch_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
# 设置carla_pytorch_debug目标的编译标志为CMAKE_CXX_FLAGS_DEBUG
  target_compile_definitions(carla_pytorch_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)
# 为目标添加编译定义，启用Boost ASIO的调试功能
endif()
# 结束LIBCARLA_BUILD_DEBUG配置的代码块
