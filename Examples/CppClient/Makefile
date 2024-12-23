# 定义 CARLA 的根目录，使用 $(CURDIR) 表示当前目录的上级目录的上级目录
CARLADIR=$(CURDIR)/../..

# 定义构建目录，用于存放构建过程中生成的文件
BUILDDIR=$(CURDIR)/build

# 定义二进制文件目录，用于存放最终编译生成的可执行文件
BINDIR=$(CURDIR)/bin

# 定义安装目录，用于存放编译安装后的库文件和头文件
INSTALLDIR=$(CURDIR)/libcarla-install

# 定义工具链文件，该文件包含编译器和编译选项的配置
TOOLCHAIN=$(CURDIR)/ToolChain.cmake

# 定义 C 编译器路径
CC=/usr/bin/gcc-7

# 定义 C++ 编译器路径
CXX=/usr/bin/g++-7

# 定义 C++ 编译选项，包括使用 C++14 标准、启用多线程、生成位置无关代码等
CXXFLAGS=-std=c++14 -pthread -fPIC -O3 -DNDEBUG -Werror -Wall -Wextra

# 定义一个函数 log，用于输出带有颜色的日志信息
define log
    @echo "\033[1;35m$(1)\033[0m"
endef

# 默认目标，执行时会触发 build 目标
default: build

# 清理目标，执行时会删除构建目录、安装目录和工具链文件
clean:
    @rm -rf $(BUILDDIR) $(INSTALLDIR)
    @rm -f ToolChain.cmake

# 运行目标，先执行 build 目标，然后运行 C++ 客户端程序
run: build
    $(call log,Running C++ Client...)
    @$(BINDIR)/cpp_client $(ARGS)

# 只运行目标，不依赖 build 目标，直接运行 C++ 客户端程序
run.only:
    $(call log,Running C++ Client...)
    @$(BINDIR)/cpp_client $(ARGS)

# 构建目标，依赖于 $(BINDIR)/cpp_client 的存在
build: $(BINDIR)/cpp_client

# 生成 $(BINDIR)/cpp_client 可执行文件的目标，依赖于 build_libcarla 目标的完成
$(BINDIR)/cpp_client: | build_libcarla
    $(call log,Compiling C++ Client...)
    @mkdir -p $(BINDIR)
    @$(CXX) $(CXXFLAGS) -I$(INSTALLDIR)/include -isystem $(INSTALLDIR)/include/system -L$(INSTALLDIR)/lib \
        -o $(BINDIR)/cpp_client main.cpp \
        -Wl,-Bstatic -lcarla_client -lrpc -lboost_filesystem -Wl,-Bdynamic \
        -lpng -ltiff -ljpeg -lRecast -lDetour -lDetourCrowd

# 构建 libcarla 库的目标，依赖于工具链文件的生成
build_libcarla: $(TOOLCHAIN)
    @cd $(CARLADIR); make setup
    @mkdir -p $(BUILDDIR)
    $(call log,Compiling LibCarla.client...)
    @{ \
    # 在构建目录中执行一系列命令
    cd $(BUILDDIR); \
    # 如果 build.ninja 文件不存在，则执行 CMake 配置
    if [ ! -f "build.ninja" ]; then \
    cmake \
        -G "Ninja" \
        -DCMAKE_BUILD_TYPE=Client \
        -DLIBCARLA_BUILD_RELEASE=ON \
        -DLIBCARLA_BUILD_DEBUG=OFF \
        -DLIBCARLA_BUILD_TEST=OFF \
        -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
        -DCMAKE_INSTALL_PREFIX=$(INSTALLDIR) \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
        $(CARLADIR); \
    fi; \
    # 执行 Ninja 构建
    ninja; \
    # 执行 Ninja 安装，并过滤掉 "Up-to-date:" 信息
    ninja install | grep -v "Up-to-date:"; \
    }

# 生成工具链文件的目标
$(TOOLCHAIN):
    # 在工具链文件中设置 C 编译器路径
    @echo "set(CMAKE_C_COMPILER $(CC))" > $(TOOLCHAIN)
    # 在工具链文件中设置 C++ 编译器路径
    @echo "set(CMAKE_CXX_COMPILER $(CXX))" >> $(TOOLCHAIN)
    # 在工具链文件中设置 C++ 编译选项，并强制更新 CMAKE_CXX_FLAGS
    @echo "set(CMAKE_CXX_FLAGS \"\$${CMAKE_CXX_FLAGS} $(CXXFLAGS)\" CACHE STRING \"\" FORCE)" >> $(TOOLCHAIN)
