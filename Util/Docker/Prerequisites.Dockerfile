FROM ubuntu:18.04 #指定基础镜像为 Ubuntu 18.04

USER root #指定当前用户为 root，以便执行后续的安装和配置操作

ARG EPIC_USER=user #定义了两个构建参数，用于 GitHub 认证
ARG EPIC_PASS=pass #定义了两个构建参数，用于 GitHub 认证
ENV DEBIAN_FRONTEND=noninteractive #设置环境变量，避免在安装过程中出现交互式提示
RUN apt-get update ; \#更新 apt 包索引
  apt-get install -y wget software-properties-common && \#安装 wget 和软件源管理工具
  add-apt-repository ppa:ubuntu-toolchain-r/test && \#添加 Ubuntu Toolchain 测试 PPA，用于安装新版本的编译工具。
  wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|apt-key add - && \#添加 LLVM 的 GPG 密钥。
  apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-8 main" && \#添加 LLVM 的软件源。
  apt-get update ; \
  apt-get install -y build-essential \
    clang-8 \
    lld-8 \
    g++-7 \
    cmake \
    ninja-build \
    libvulkan1 \
    python \
    python-pip \
    python-dev \
    python3-dev \
    python3-pip \
    libpng-dev \
    libtiff5-dev \
    libjpeg-dev \
    tzdata \
    sed \
    curl \
    unzip \
    autoconf \
    libtool \
    rsync \
    libxml2-dev \
    git \
    aria2 && \
  pip3 install -Iv setuptools==47.3.1 && \
  pip3 install distro && \
  update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-8/bin/clang++ 180 && \
  update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-8/bin/clang 180

# 在 Docker 容器内执行 `useradd -m carla` 命令，该命令用于创建一个名为 `carla` 的用户，并且通过 `-m` 参数同时创建该用户的家目录（通常是 `/home/carla`）。
# 这一步操作是为后续以该用户身份运行相关操作、进行权限管理等做准备，确保在容器环境中有对应的用户账户存在。
RUN useradd -m carla

# 将当前目录（构建上下文目录，通常是 Dockerfile 所在的目录，包含了要拷贝的文件或文件夹）下的所有内容复制到容器内 `/home/carla` 目录下，
# 同时通过 `--chown=carla:carla` 参数设置复制过去的文件和目录的所有者为 `carla` 用户以及所属组也为 `carla`，以此来保证权限设置符合预期，方便后续操作。
COPY --chown=carla:carla. /home/carla

# 将后续执行的命令的用户身份切换为 `carla` 用户，意味着从这一行之后的命令（如 `RUN`、`WORKDIR` 等相关命令）都会以 `carla` 用户的权限来执行，
# 实现了在容器内模拟以特定用户操作的效果，便于进行权限隔离以及符合特定的应用场景需求（比如某些软件运行要求特定用户权限等）。
USER carla

# 设置容器内的工作目录为 `/home/carla`，后续的命令（如 `RUN` 等命令执行的脚本等操作）如果涉及相对路径，都会基于这个工作目录来进行，
# 相当于指定了一个默认的操作路径起点，让后续操作更方便、更有条理，无需每次都写全绝对路径。
WORKDIR /home/carla

# 设置一个名为 `UE4_ROOT` 的环境变量，其值为 `/home/carla/UE4.26`，这个环境变量通常会在后续的脚本运行或者软件安装等操作中被引用，
# 用于指示某个软件（这里很可能是 Unreal Engine 相关的，从后续代码可推测）在容器内的安装根目录等相关位置信息，方便代码中统一使用该变量来指代这个位置。
ENV UE4_ROOT /home/carla/UE4.26

# 在容器内执行 `git clone` 命令，用于从指定的 GitHub 仓库克隆代码到本地（容器内）。
# 这里克隆的是 `https://${EPIC_USER}:${EPIC_PASS}@github.com/CarlaUnreal/UnrealEngine.git` 这个仓库，
# 其中 `${EPIC_USER}` 和 `${EPIC_PASS}` 应该是预先定义好的环境变量，代表 GitHub 账号的用户名和密码（或访问令牌等认证信息），
# 并通过 `-b carla` 参数指定克隆仓库的特定分支为 `carla` 分支，最后将克隆下来的代码存放到由 `UE4_ROOT` 环境变量指定的目录（即 `/home/carla/UE4.26`）中。
RUN git clone --depth 1 -b carla "https://${EPIC_USER}:${EPIC_PASS}@github.com/CarlaUnreal/UnrealEngine.git" ${UE4_ROOT}

# 在容器内执行一系列的命令，首先通过 `cd $UE4_ROOT` 切换到 `UE4_ROOT` 环境变量所指向的目录（也就是刚才克隆代码存放的目录 `/home/carla/UE4.26`），
# 然后执行 `./Setup.sh` 脚本（这个脚本通常用于配置相关的软件安装前的准备工作，比如安装依赖等），
# 接着执行 `./GenerateProjectFiles.sh` 脚本（可能用于生成项目相关的文件，便于后续编译等操作），
# 最后执行 `make` 命令（用于编译构建相关的项目，这里大概率是构建 Unreal Engine 项目，不过具体构建结果取决于项目本身的结构和配置）。
RUN cd $UE4_ROOT && \
 ./Setup.sh && \
 ./GenerateProjectFiles.sh && \
  make

# 再次设置容器内的工作目录为 `/home/carla`，这可能是为了确保后续其他操作（如果有的话）能在预期的默认工作目录下进行，
# 也有可能是前面的操作改变了工作目录，这里重新设置回来以保证后续操作的一致性和可预测性。
WORKDIR /home/carla
