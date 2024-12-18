FROM carla-prerequisites:latest#指定基础镜像

ARG GIT_BRANCH

USER carla
WORKDIR /home/carla#切换后续操作用户

RUN cd /home/carla/ && \
  if [ -z ${GIT_BRANCH+x} ]; then git clone --depth 1 https://github.com/carla-simulator/carla.git; \# 检查环境变量GIT_BRANCH是否已设置，如果未设置（即 -z ${GIT_BRANCH+x} 条件成立）
  else git clone --depth 1 --branch $GIT_BRANCH https://github.com/carla-simulator/carla.git; fi && \
  cd /home/carla/carla && \
  ./Update.sh && \
  make CarlaUE4Editor && \
  make PythonAPI && \
  make build.utils && \
  make package && \
  rm -r /home/carla/carla/Dist #设置工作目录为 /home/carla/carla，后续操作如果相对路径的话，会基于此目录进行

WORKDIR /home/carla/carla
