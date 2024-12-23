
# run the docker container as:
#
# sudo -E docker run --rm --gpus all -it --net=host carla:latest /bin/bash
# 基于 nvidia/vulkan:1.1.121-cuda-10.1--ubuntu18.04 镜像，该镜像包含 Vulkan 和 CUDA 10.1，以及 Ubuntu 18.04 系统。
FROM nvidia/vulkan:1.1.121-cuda-10.1--ubuntu18.04
# 添加 NVIDIA CUDA 仓库的公钥，以便安装 NVIDIA 的驱动和库。
RUN apt-key adv --fetch-keys "https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/3bf863cc.pub"
# 安装所需的依赖包：libsdl2-2.0、xserver-xorg、libvulkan1 和 libomp5。
# 这些包分别用于 SDL2 库、X 服务器、Vulkan 驱动和 OpenMP 库。
RUN packages='libsdl2-2.0 xserver-xorg libvulkan1 libomp5' && apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y $packages --no-install-recommends
# 创建一个名为 carla 的新用户
RUN useradd -m carla
# 将当前目录下的文件复制到容器的 /home/carla 目录下，并更改文件所有者为 carla 用户。
COPY --chown=carla:carla . /home/carla
# 切换到 carla 用户。
USER carla
# 设置容器内的工作目录为 /home/carla。
WORKDIR /home/carla

# 容器启动时默认执行的命令。这里设置为运行 CarlaUE4.sh 脚本启动 CARLA。
# 你可以通过设置 -RenderOffScreen 参数来让 CARLA 在无头模式下运行。
CMD /bin/bash CarlaUE4.sh
