# 设置CMake的最低版本要求为3.5.1
cmake_minimum_required(VERSION 3.5.1)

# 定义项目名称为libcarla-pytorch
project(libcarla-pytorch)

# 使用file(GLOB)命令查找所有.h头文件，并将它们安装到include/carla/pytorch目录下
file(GLOB libcarla_carla_pytorch_headers "${libcarla_source_path}/carla/pytorch/*.h")
install(FILES ${libcarla_carla_pytorch_headers} DESTINATION include/carla/pytorch)

# 定义libcarla_pytorch库的源文件，包括.h和.cpp文件
file(GLOB libcarla_pytorch_sources "${libcarla_source_path}/carla/pytorch/*.h" "${libcarla_source_path}/carla/pytorch/*.cpp")

# 设置CUDA编译器的路径为/usr/local/cuda/bin/nvcc
set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)

# 查找并要求Torch、TorchScatter、TorchCluster和Python3包
find_package(Torch REQUIRED)
find_package(TorchScatter REQUIRED)
find_package(TorchCluster REQUIRED)
find_package(Python3 REQUIRED)

# 设置PyTorch C++ API的标准库包含路径为/usr/include/c++/7
# 注意：这个路径可能需要根据你的系统环境进行调整
set(PYTORCH_CPP_STD_INCLUDES "/usr/include/c++/7")

# 设置CMAKE_CXX_FLAGS_RELEASE变量，包含编译选项，用于发布版本的构建
# 注意：这里同时包含了-DDEBUG，这在发布版本中可能是不合适的，应该去除
set(CMAKE_CXX_FLAGS_RELEASE "-DDEBUG -std=c++14 -O2 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -I${PYTORCH_CPP_STD_INCLUDES}" CACHE STRING "" FORCE)

# 根据是否定义了LIBCARLA_BUILD_RELEASE或LIBCARLA_BUILD_DEBUG来构建不同的目标
if (LIBCARLA_BUILD_RELEASE)
    # 添加静态库carla_pytorch
    add_library(carla_pytorch STATIC ${libcarla_pytorch_sources})
    
    # 为carla_pytorch库添加包含目录
    target_include_directories(carla_pytorch SYSTEM PRIVATE "${BOOST_INCLUDE_PATH}" "${RPCLIB_INCLUDE_PATH}")
    
    # 添加PyTorch及其依赖库的包含目录
    target_include_directories(carla_pytorch PRIVATE SYSTEM "${TORCH_INCLUDE_DIRS}")
    target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchScatter_INCLUDE_DIR}")
    target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchCluster_INCLUDE_DIR}")
    # 注意：TorchSparse的包含目录被添加，但脚本中未显示查找TorchSparse包
    target_include_directories(carla_pytorch PRIVATE SYSTEM "${TorchSparse_INCLUDE_DIR}")
    target_include_directories(carla_pytorch PRIVATE SYSTEM "${Python3_INCLUDE_DIRS}")
    
    # 链接PyTorch库到carla_pytorch目标
    target_link_libraries(carla_pytorch "${TORCH_LIBRARIES}")
    
    # 安装carla_pytorch库到lib目录（可选）
    install(TARGETS carla_pytorch DESTINATION lib OPTIONAL)
    
    # 设置carla_pytorch的编译标志为发布版本的编译选项
    set_target_properties(carla_pytorch PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
endif()

if (LIBCARLA_BUILD_DEBUG)
    # 添加静态库carla_pytorch_debug，并指定其源文件
    add_library(carla_pytorch_debug STATIC ${libcarla_pytorch_sources})
    
    # 为carla_pytorch_debug库添加包含目录
    target_include_directories(carla_pytorch_debug SYSTEM PRIVATE "${BOOST_INCLUDE_PATH}" "${RPCLIB_INCLUDE_PATH}")
    
    # 安装carla_pytorch_debug库到lib目录（可选）
    install(TARGETS carla_pytorch_debug DESTINATION lib OPTIONAL)
    
    # 设置carla_pytorch_debug的编译属性，但CMAKE_CXX_FLAGS_DEBUG未在脚本中定义，需要添加
    # 假设这里应该使用一组适合调试的编译标志
    # set_target_properties(carla_pytorch_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
    
    # 为目标carla_pytorch_debug添加编译定义，启用Boost.Asio的调试功能
    target_compile_definitions(carla_pytorch_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)
endif()

# 注意：
# 1. 脚本中引用了BOOST_INCLUDE_PATH和RPCLIB_INCLUDE_PATH变量，但未显示如何设置这些变量。
#    通常，这些变量应该在CMake脚本的某处被设置，或者通过命令行参数传递给CMake。
# 2. TorchSparse的find_package命令未在脚本中显示，但包含了其包含目录。
#    如果TorchSparse是必需的，那么应该添加相应的find_package命令。
# 3. CMAKE_CXX_FLAGS_DEBUG未在脚本中定义，如果需要使用调试版本的编译标志，应该添加相应的设置。
