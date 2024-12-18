# 定义 CARLA 目录，通过 $(CURDIR)（当前 Makefile 所在目录）的相对路径表示，指向上级目录的上级目录，方便适配不同项目结构找到 CARLA 相关位置。
CARLADIR=$(CURDIR)/../..
# 构建目录，存放构建过程的中间文件，如目标文件等，保持项目结构清晰，便于管理构建产物。
BUILDDIR=$(CURDIR)/build
# 二进制文件目录，用于存放最终生成的可执行文件，便于查找和部署应用。
BINDIR=$(CURDIR)/bin
# 安装目录，构建完成后安装的库、头文件等资源放置于此，遵循常见软件安装目录规范。
INSTALLDIR=$(CURDIR)/libcarla-install
# 工具链文件，包含编译器相关配置，用于定制构建环境，对项目构建起关键作用。
TOOLCHAIN=$(CURDIR)/ToolChain.cmake

# C 编译器路径，明确指定使用 /usr/bin/gcc-7 版本的 C 编译器，确保编译版本符合要求。
CC=/usr/bin/gcc-7
# C++ 编译器路径，指定使用 /usr/bin/g++-7 版本的 C++ 编译器，保证构建按指定版本执行编译操作。
CXX=/usr/bin/g++-7
# C++ 编译选项：
# -std=c++14：按 C++14 标准编译代码，保证兼容性与利用新特性。
# -pthread：启用多线程支持，编译时链接相应线程库，用于多线程编程。
# -fPIC：生成位置无关代码，便于构建动态链接库及灵活内存布局场景使用。
# -O3：较高优化级别，提升程序运行效率，但可能增加编译时间和二进制文件大小，常用于发布版本。
# -DNDEBUG：定义此宏，预处理阶段排除调试相关代码，减少运行开销，适用于发布版本。
# -Werror：将编译器警告视为错误，强制解决潜在问题，保障代码质量。
# -Wall：开启常见警告信息，助于发现代码逻辑或规范问题。
# -Wextra：进一步开启额外警告，加强代码质量把控。
CXXFLAGS=-std=c++14 -pthread -fPIC -O3 -DNDEBUG -Werror -Wall -Wextra

# 定义 log 函数，用于输出带颜色（紫色加粗）的日志信息，方便在终端突出显示提示内容，结束后恢复默认颜色。
define log
    @echo "\033[1;35m$(1)\033[0m"
endef

# 默认目标，执行 make 无参数时默认执行 build 目标，遵循常见构建流程习惯，方便快速启动构建。
default: build

# 清理目标，删除构建目录、安装目录及工具链文件，@ 让命令执行时不显示命令本身，只显示结果，rm -rf 递归删除指定目录及内容，实现彻底清理构建相关文件。
clean:
    @rm -rf $(BUILDDIR) $(INSTALLDIR)
    @rm -f ToolChain.cmake

# 运行目标，依赖 build 目标，先完成项目构建再运行 C++ 客户端程序，模拟先构建后运行的常规操作流程。
run: build
    # 输出运行客户端程序的日志信息，使用 log 函数以特定颜色显示提示，便于用户知晓当前操作。
    $(call log,Running C++ Client...)
    @$(BINDIR)/cpp_client $(ARGS)

# 只运行目标，直接运行 C++ 客户端程序，与 run 不同，不先执行 build，适用于确认可执行文件存在且无需重新构建的情况，提供更灵活运行方式。
run.only:
    $(call log,Running C++ Client...)
    @$(BINDIR)/cpp_client $(ARGS)

# 构建目标，依赖 $(BINDIR)/cpp_client，整个构建围绕此展开，确保该可执行文件能正确生成。
build: $(BINDIR)/cpp_client

# 生成 $(BINDIR)/cpp_client 可执行文件，依赖 build_libcarla 目标，先完成 libcarla 构建保证依赖就绪，有序组织编译和链接流程。
$(BINDIR)/cpp_client: | build_libcarla
    # 输出编译客户端程序的日志信息，提示用户当前编译阶段，方便跟踪进度与排查问题。
    $(call log,Compiling C++ Client...)
    @mkdir -p $(BINDIR)
    @$(CXX) $(CXXFLAGS) -I$(INSTALLDIR)/include -isystem $(INSTALLDIR)/include/system -L$(INSTALLDIR)/lib \
        -o $(BINDIR)/cpp_client main.cpp \
        -Wl,-Bstatic -lcarla_client -lrpc -lboost_filesystem -Wl,-Bdynamic \
        -lpng -ltiff -ljpeg -lRecast -lDetour -lDetourCrowd

# 构建 libcarla 目标，依赖 $(TOOLCHAIN)，工具链文件含构建关键配置，需先确保其存在才能正确构建 libcarla。
build_libcarla: $(TOOLCHAIN)
    # 进入 CARLADIR 目录执行 make setup，这是 CARLA 项目根目录下的准备工作（具体功能由其内部定义），为后续构建做前置准备。
    @cd $(CARLADIR); make setup
    @mkdir -p $(BUILDDIR)
    $(call log,Compiling LibCarla.client...)
    @{ \
        # 进入构建目录，后续 CMake 配置、构建及安装操作在此进行，集中管理构建文件，避免原目录结构混乱。
        cd $(BUILDDIR); \
        # 若 build.ninja 文件不存在则执行 CMake 配置，避免重复配置，提高构建效率。
        if [! -f "build.ninja" ]; then \
        cmake \
            # 使用 Ninja 构建工具，它在大型项目构建时有更快速度和更好并行构建能力。
            -G "Ninja" \
            # 构建类型为 Client，项目自定义的构建类型标识，区分不同构建情况，此处针对客户端相关代码构建。
            -DCMAKE_BUILD_TYPE=Client \
            # 开启 LibCarla 的 Release 构建，注重生成高效的发布版本二进制文件，进行优化并去除调试信息。
            -DLIBCARLA_BUILD_RELEASE=ON \
            # 关闭 Debug 构建，不生成含调试信息的文件，只专注发布版本构建。
            -DLIBCARLA_BUILD_DEBUG=OFF \
            # 关闭测试构建，不生成测试相关目标文件等，提高构建效率，专注实际可用资源生成。
            -DLIBCARLA_BUILD_TEST=OFF \
            # 使用自定义工具链文件，让 CMake 按此配置进行构建，满足项目特定需求。
            -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
            # 设置安装前缀，指定安装文件存放目录，与 INSTALLDIR 对应，方便后续管理和引用已安装资源。
            -DCMAKE_INSTALL_PREFIX=$(INSTALLDIR) \
            # 导出编译命令，生成 compile_commands.json 文件（若支持），方便编辑器或工具分析编译依赖等情况。
            -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
            $(CARLADIR); \
      fi; \
        # 执行 Ninja 构建，根据 CMake 配置生成构建产物。
        ninja; \
        # 执行 Ninja 安装，过滤掉 "Up-to-date:" 信息，使终端输出更简洁，将构建好的文件安装到指定目录。
        ninja install | grep -v "Up-to-date:"; \
    }

# 生成工具链文件，在其中设置 C 编译器、C++ 编译器路径以及 C++ 编译选项，并强制更新 CACHE 中的 CMAKE_CXX_FLAGS。
$(TOOLCHAIN):
    @echo "set(CMAKE_C_COMPILER $(CC))" > $(TOOLCHAIN)
    @echo "set(CMAKE_CXX_COMPILER $(CXX))" >> $(TOOLCHAIN)
    @echo "set(CMAKE_CXX_FLAGS \"\$${CMAKE_CXX_FLAGS} $(CXXFLAGS)\" CACHE STRING \"\" FORCE)" >> $(TOOLCHAIN)
