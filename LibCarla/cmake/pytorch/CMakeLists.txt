# ==============================================================================
# Carla PyTorch 扩展构建配置
# 
# 该CMake文件配置了Carla的PyTorch扩展模块，包括：
# - 构建静态库
# - 配置依赖项
# - 设置编译选项
# ==============================================================================

cmake_minimum_required(VERSION 3.5.1)
project(libcarla-pytorch)

# ==============================================================================
# 源文件和头文件配置
# ==============================================================================

# 定位所有相关源文件
file(GLOB libcarla_pytorch_sources
    "${libcarla_source_path}/carla/pytorch/*.h"
    "${libcarla_source_path}/carla/pytorch/*.cpp"
)

# 配置头文件安装
file(GLOB libcarla_carla_pytorch_headers 
    "${libcarla_source_path}/carla/pytorch/*.h"
)
install(FILES ${libcarla_carla_pytorch_headers} 
    DESTINATION include/carla/pytorch
)

# ==============================================================================
# 依赖项配置
# ==============================================================================

# 设置CUDA编译器
set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)

# 查找必要的依赖包
find_package(Torch REQUIRED)       # PyTorch核心库
find_package(TorchScatter REQUIRED)# 图操作扩展
find_package(TorchCluster REQUIRED)# 聚类算法支持
find_package(Python3 REQUIRED)     # Python支持

# 设置PyTorch C++ API标准库路径
set(PYTORCH_CPP_STD_INCLUDES "/usr/include/c++/7")

# ==============================================================================
# 编译选项配置
# ==============================================================================

# 设置发布版本的编译标志
# @note 这些标志需要与setup.py的编译选项兼容
set(CMAKE_CXX_FLAGS_RELEASE 
    "-DDEBUG -std=c++14 -O2 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -I${PYTORCH_CPP_STD_INCLUDES}" 
    CACHE STRING "" FORCE
)

# ==============================================================================
# 目标构建配置
# ==============================================================================

#------------------------------------------------------------------------------
# 发布版本构建
#------------------------------------------------------------------------------
if (LIBCARLA_BUILD_RELEASE)
    # 创建静态库
    add_library(carla_pytorch STATIC ${libcarla_pytorch_sources})
    
    # 配置包含目录
    target_include_directories(carla_pytorch SYSTEM PRIVATE
        # 基础依赖
        "${BOOST_INCLUDE_PATH}"
        "${RPCLIB_INCLUDE_PATH}"
        
        # PyTorch相关依赖
        "${TORCH_INCLUDE_DIRS}"        # PyTorch核心
        "${TorchScatter_INCLUDE_DIR}"  # 图操作支持
        "${TorchCluster_INCLUDE_DIR}"  # 聚类算法
        "${TorchSparse_INCLUDE_DIR}"   # 稀疏张量支持
        "${Python3_INCLUDE_DIRS}"      # Python接口
    )
    
    # 链接PyTorch库
    target_link_libraries(carla_pytorch "${TORCH_LIBRARIES}")
    
    # 设置编译选项
    set_target_properties(carla_pytorch PROPERTIES 
        COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}"
    )
    
    # 配置安装规则
    install(TARGETS carla_pytorch 
        DESTINATION lib 
        OPTIONAL
    )
endif()

#------------------------------------------------------------------------------
# 调试版本构建
#------------------------------------------------------------------------------
if (LIBCARLA_BUILD_DEBUG)
    # 创建调试版本静态库
    add_library(carla_pytorch_debug STATIC ${libcarla_pytorch_sources})
    
    # 配置包含目录
    target_include_directories(carla_pytorch_debug SYSTEM PRIVATE
        "${BOOST_INCLUDE_PATH}"
        "${RPCLIB_INCLUDE_PATH}"
    )
    
    # 设置调试选项
    set_target_properties(carla_pytorch_debug PROPERTIES 
        COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}"
    )
    
    # 启用Boost.Asio缓冲区调试
    target_compile_definitions(carla_pytorch_debug PUBLIC 
        -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING
    )
    
    # 配置安装规则
    install(TARGETS carla_pytorch_debug 
        DESTINATION lib 
        OPTIONAL
    )
endif()
