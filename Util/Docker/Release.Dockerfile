
# run the docker container as:
#
# sudo -E docker run --rm --gpus all -it --net=host carla:latest /bin/bash

FROM nvidia/vulkan:1.1.121-cuda-10.1--ubuntu18.04
# 基于 nvidia/vulkan:1.1.121-cuda-10.1--ubuntu18.04 这个基础镜像来构建当前镜像
RUN apt-key adv --fetch-keys "https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/3bf863cc.pub"
# 使用 apt-key 命令添加 NVIDIA CUDA 软件源的公钥，用于后续能从该软件源正确下载和安装相关软件包，
# 通过指定的 URL 获取公钥信息
RUN packages='libsdl2-2.0 xserver-xorg libvulkan1 libomp5' && apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y $packages --no-install-recommends
# 定义要安装的软件包列表，这里包含了 libsdl2-2.0（用于多媒体等相关功能的库）、
# xserver-xorg（X 窗口系统相关的服务器组件）、libvulkan1（Vulkan 图形 API 相关库）、
# libomp5（OpenMP 相关库，用于并行计算支持等），然后执行 apt-get update 更新软件包索引，
# 接着使用 DEBIAN_FRONTEND=noninteractive 设置非交互模式（避免安装过程中的一些交互提示），
# 通过 apt-get install -y 命令安装前面定义的软件包列表，--no-install-recommends 表示只安装明确指定的软件包，不安装推荐的额外软件包
RUN useradd -m carla
# 创建一个名为 carla 的用户，-m 选项表示同时为该用户创建家目录
COPY --chown=carla:carla . /home/carla
# 将当前目录（构建上下文目录，一般是 Dockerfile 所在目录）下的所有文件和目录复制到容器内 /home/carla 目录下，
# 并将复制后的文件和目录的所有者设置为 carla 用户和 carla 用户组，保证权限正确归属
USER carla
# 切换到 carla 用户，后续的操作都将以该用户身份在容器内执行
WORKDIR /home/carla
# 设置容器内的工作目录为 /home/carla，后续在容器内执行的命令如果是相对路径，将基于此目录来解析
# you can also run CARLA in offscreen mode with -RenderOffScreen
# CMD /bin/bash CarlaUE4.sh -RenderOffScreen
CMD /bin/bash CarlaUE4.sh
