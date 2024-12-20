
# run the docker container as:
#
# sudo -E docker run --rm --gpus all -it --net=host carla:latest /bin/bash

# 使用NVIDIA的Vulkan和CUDA 10.1的Docker镜像，基于Ubuntu 18.04操作系统
FROM nvidia/vulkan:1.1.121-cuda-10.1--ubuntu18.04
 
# 添加NVIDIA CUDA的APT密钥，以便可以从NVIDIA的官方Ubuntu仓库安装软件包
RUN apt-key adv --fetch-keys "https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/3bf863cc.pub"
 
# 更新APT包索引，并安装指定的软件包，包括SDL2库、X服务器、Vulkan库和OpenMP库
# 使用DEBIAN_FRONTEND=noninteractive避免在安装过程中弹出交互式对话框
# --no-install-recommends 选项表示不安装推荐的额外软件包
RUN packages='libsdl2-2.0 xserver-xorg libvulkan1 libomp5' && apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y $packages --no-install-recommends
 
# 创建一个新用户名为carla的用户，并为其创建一个主目录
RUN useradd -m carla
 
# 将当前上下文（Dockerfile所在的目录）中的所有文件复制到容器内的/home/carla目录
# --chown=carla:carla 参数确保这些文件属于carla用户
COPY --chown=carla:carla . /home/carla
 
# 设置后续命令（RUN、CMD、ENTRYPOINT等）运行的用户为carla
USER carla
 
# 设置工作目录为/home/carla，这意味着容器启动时，会在这个目录下运行命令
WORKDIR /home/carla

# you can also run CARLA in offscreen mode with -RenderOffScreen
# CMD /bin/bash CarlaUE4.sh -RenderOffScreen
CMD /bin/bash CarlaUE4.sh
