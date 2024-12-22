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

RUN useradd -m carla
COPY --chown=carla:carla . /home/carla
USER carla
WORKDIR /home/carla
ENV UE4_ROOT /home/carla/UE4.26

RUN git clone --depth 1 -b carla "https://${EPIC_USER}:${EPIC_PASS}@github.com/CarlaUnreal/UnrealEngine.git" ${UE4_ROOT}

RUN cd $UE4_ROOT && \
  ./Setup.sh && \
  ./GenerateProjectFiles.sh && \
  make

WORKDIR /home/carla/
