# 定义 CARLA 目录，使用 $(CURDIR) 表示当前目录的上级目录的上级目录
CARLADIR=$(CURDIR)/../..
# 定义构建目录
BUILDDIR=$(CURDIR)/build
# 定义二进制文件目录
BINDIR=$(CURDIR)/bin
# 定义安装目录
INSTALLDIR=$(CURDIR)/libcarla-install
# 定义工具链文件
TOOLCHAIN=$(CURDIR)/ToolChain.cmake

# 定义 C 编译器路径
CC=/usr/bin/gcc-7
# 定义 C++ 编译器路径
CXX=/usr/bin/g++-7
# 定义 C++ 编译选项，包括使用 C++14 标准、启用多线程、生成位置无关代码、优化级别为 O3、禁用调试、开启所有警告和额外警告
CXXFLAGS=-std=c++14 -pthread -fPIC -O3 -DNDEBUG -Werror -Wall -Wextra

# 定义一个函数 log，用于输出带有颜色的日志信息，\033[1;35m 是颜色代码，\033[0m 用于恢复默认颜色
define log
    @echo "\033[1;35m$(1)\033[0m"
endef

# 默认目标，执行 build 目标
default: build

# 清理目标，删除构建目录、安装目录和工具链文件
clean:
    @rm -rf $(BUILDDIR) $(INSTALLDIR)
    @rm -f ToolChain.cmake

# 运行目标，先执行 build 目标，然后运行 C++ 客户端程序
run: build
    # 输出运行 C++ 客户端的日志信息
    $(call log,Running C++ Client...)
    @$(BINDIR)/cpp_client $(ARGS)

# 只运行目标，直接运行 C++ 客户端程序
run.only:
    # 输出运行 C++ 客户端的日志信息
    $(call log,Running C++ Client...)
    @$(BINDIR)/cpp_client $(ARGS)

# 构建目标，依赖于 $(BINDIR)/cpp_client
build: $(BINDIR)/cpp_client

# 生成 $(BINDIR)/cpp_client 可执行文件，依赖于 build_libcarla 目标
$(BINDIR)/cpp_client: | build_libcarla
    # 输出编译 C++ 客户端的日志信息
    $(call log,Compiling C++ Client...)
    @mkdir -p $(BINDIR)
    @$(CXX) $(CXXFLAGS) -I$(INSTALLDIR)/include -isystem $(INSTALLDIR)/include/system -L$(INSTALLDIR)/lib \
        -o $(BINDIR)/cpp_client main.cpp \
        # 链接静态库 libcarla_client、rpc、boost_filesystem，链接动态库 png、tiff、jpeg、Recast、Detour、DetourCrowd
        -Wl,-Bstatic -lcarla_client -lrpc -lboost_filesystem -Wl,-Bdynamic \
        -lpng -ltiff -ljpeg -lRecast -lDetour -lDetourCrowd

# 构建 libcarla 目标，依赖于 $(TOOLCHAIN)
build_libcarla: $(TOOLCHAIN)
    # 进入 CARLADIR 目录并执行 make setup
    @cd $(CARLADIR); make setup
    @mkdir -p $(BUILDDIR)
    # 输出编译 LibCarla.client 的日志信息
    $(call log,Compiling LibCarla.client...)
    @{ \
        # 进入构建目录
        cd $(BUILDDIR); \
        # 如果 build.ninja 文件不存在，则执行 CMake 配置
        if [! -f "build.ninja" ]; then \
        cmake \
            # 使用 Ninja 作为构建工具
            -G "Ninja" \
            # 构建类型为 Client
            -DCMAKE_BUILD_TYPE=Client \
            # 开启 LibCarla 的 Release 构建
            -DLIBCARLA_BUILD_RELEASE=ON \
            # 关闭 LibCarla 的 Debug 构建
            -DLIBCARLA_BUILD_DEBUG=OFF \
            # 关闭 LibCarla 的测试构建
            -DLIBCARLA_BUILD_TEST=OFF \
            # 使用自定义的工具链文件
            -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
            # 设置安装前缀
            -DCMAKE_INSTALL_PREFIX=$(INSTALLDIR) \
            # 导出编译命令
            -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
            # CMake 的源目录为 CARLADIR
            $(CARLADIR); \
      fi; \
        # 执行 Ninja 构建
        ninja; \
        # 执行 Ninja 安装，并过滤掉 "Up-to-date:" 信息
        ninja install | grep -v "Up-to-date:"; \
    }

# 生成工具链文件
$(TOOLCHAIN):
    # 生成工具链文件，设置 C 编译器
    @echo "set(CMAKE_C_COMPILER $(CC))" > $(TOOLCHAIN)
    # 生成工具链文件，设置 C++ 编译器
    @echo "set(CMAKE_CXX_COMPILER $(CXX))" >> $(TOOLCHAIN)
    # 生成工具链文件，设置 C++ 编译选项并强制更新 CACHE 中的 CMAKE_CXX_FLAGS
    @echo "set(CMAKE_CXX_FLAGS \"\$${CMAKE_CXX_FLAGS} $(CXXFLAGS)\" CACHE STRING \"\" FORCE)" >> $(TOOLCHAIN)
