FROM ubuntu:18.04
# 使用Ubuntu 18.04作为基础镜像来构建容器环境
USER root
# 将后续操作的用户切换为root用户，root用户拥有最高权限，方便执行安装软件包等系统级操作
ARG EPIC_USER=user
ARG EPIC_PASS=pass
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update ; \
  apt-get install -y wget software-properties-common && \
  add-apt-repository ppa:ubuntu-toolchain-r/test && \
  wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|apt-key add - && \
  apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-8 main" && \
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

RUN useradd -m carla
# 创建一个名为carla的用户，并且同时创建该用户的家目录（-m选项的作用），后续操作可能会切换到这个用户下进行，以增强安全性和权限管理
COPY --chown=carla:carla . /home/carla
# 将构建上下文（也就是Dockerfile所在目录及其子目录下的所有文件）复制到容器内的/home/carla目录下，并设置文件的所有者为carla用户，所属组也为carla组，确保文件权限的正确设置
USER carla
# 切换到carla用户，此后在容器内执行的操作将以carla用户的身份进行，避免使用root用户权限带来的潜在安全风险，同时也符合最小权限原则
WORKDIR /home/carla
# 设置工作目录为/home/carla，后续在容器内执行的命令如果使用相对路径，将会基于这个目录来进行操作，方便文件查找和操作定位
ENV UE4_ROOT /home/carla/UE4.26
# 设置环境变量UE4_ROOT，其值为/home/carla/UE4.26，用于指定Unreal Engine 4.26的安装目录路径，后续相关操作会围绕这个目录展开（比如克隆Unreal Engine的代码仓库到这个目录等）
RUN git clone --depth 1 -b carla "https://${EPIC_USER}:${EPIC_PASS}@github.com/CarlaUnreal/UnrealEngine.git" ${UE4_ROOT}
# 使用git命令克隆UnrealEngine仓库，克隆的是指定分支（carla分支），并且通过指定的用户名（EPIC_USER）和密码（EPIC_PASS）进行认证（如果仓库需要认证访问的话），克隆的深度设置为1，即只获取最新的一次提交记录等信息，减少克隆的数据量，将仓库克隆到UE4_ROOT指定的目录下
RUN cd $UE4_ROOT && \
  ./Setup.sh && \
  ./GenerateProjectFiles.sh && \
  make
# 进入到UE4_ROOT指定的UnrealEngine目录下，依次执行以下几个脚本：
# - Setup.sh脚本通常用于配置Unreal Engine项目的一些依赖项、环境变量等相关设置，确保项目能在当前环境下正确编译和运行
# - GenerateProjectFiles.sh脚本用于生成项目的编译相关文件，比如在不同的操作系统下会生成对应的项目文件（如在Windows下生成Visual Studio项目文件，在Linux下生成Makefile等），方便后续进行编译构建操作
# - make命令用于根据前面生成的编译相关文件来实际编译构建Unreal Engine项目，前提是系统已经安装好了所需的编译工具链以及相关依赖都配置正确
WORKDIR /home/carla/
# 将工作目录切换回/home/carla，方便后续进行其他相关操作（如果有的话），确保操作的目录上下文符合预期
