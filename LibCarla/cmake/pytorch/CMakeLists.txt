cmake_minimum_required(VERSION 3.5.1)# ����CMake����Ͱ汾Ҫ��
project(libcarla-pytorch)# ������Ŀ���� 

# ��װͷ�ļ�  
# ʹ��file(GLOB)�����������.hͷ�ļ�������װ��include/carla/pytorchĿ¼��

file(GLOB libcarla_carla_pytorch_headers "${libcarla_source_path}/carla/pytorch/*.h")
install(FILES ${libcarla_carla_pytorch_headers} DESTINATION include/carla/pytorch)

# ����libcarla_pytorch���Դ�ļ�  
# ����.h��.cpp�ļ�
file(GLOB libcarla_pytorch_sources
    "${libcarla_source_path}/carla/pytorch/*.h"
    "${libcarla_source_path}/carla/pytorch/*.cpp")

set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)# ����CUDA������·��
# ���Ҳ�Ҫ��Torch��TorchScatter��TorchCluster��Python3��
find_package(Torch REQUIRED)
find_package(TorchScatter REQUIRED)
find_package(TorchCluster REQUIRED)
find_package(Python3 REQUIRED)

set(PYTORCH_CPP_STD_INCLUDES "/usr/include/c++/7")# ����PyTorch C++ API�ı�׼�����·��

# @todo These flags need to be compatible with setup.py compilation.
set(CMAKE_CXX_FLAGS_RELEASE "-DDEBUG -std=c++14 -O2 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -I${PYTORCH_CPP_STD_INCLUDES}" CACHE STRING "" FORCE)
 
# ==============================================================================
# Create targets for debug and release in the same build type.
# ==============================================================================
# �����Ƿ�����LIBCARLA_BUILD_RELEASE��LIBCARLA_BUILD_DEBUG��������ͬ��Ŀ��
if (LIBCARLA_BUILD_RELEASE)
 # ���Ӿ�̬��carla_pytorch
  add_library(carla_pytorch STATIC ${libcarla_pytorch_sources})
  # Ϊcarla_pytorch�����Ӱ���Ŀ¼
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

  install(TARGETS carla_pytorch DESTINATION lib OPTIONAL) # ��װcarla_pytorch��

  set_target_properties(carla_pytorch PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")# ����carla_pytorch�ı����־

endif()

if (LIBCARLA_BUILD_DEBUG)

  add_library(carla_pytorch_debug STATIC ${libcarla_pytorch_sources})

  target_include_directories(carla_pytorch_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  install(TARGETS carla_pytorch_debug DESTINATION lib OPTIONAL)

  set_target_properties(carla_pytorch_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  target_compile_definitions(carla_pytorch_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

endif()
