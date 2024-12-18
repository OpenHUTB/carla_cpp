FROM carla-prerequisites:latest#指定基础镜像

ARG GIT_BRANCH

USER carla
WORKDIR /home/carla#切换后续操作用户

RUN cd /home/carla/ && \
  if [ -z ${GIT_BRANCH+x} ]; then git clone --depth 1 https://github.com/carla-simulator/carla.git; \
  else git clone --depth 1 --branch $GIT_BRANCH https://github.com/carla-simulator/carla.git; fi && \
  cd /home/carla/carla && \ #执行项目目录下的Update.sh脚本，通常可能用于更新项目相关的一些配置、依赖等内容
  ./Update.sh && \
  make CarlaUE4Editor && \
  make PythonAPI && \
  make build.utils && \
  make package && \
  rm -r /home/carla/carla/Dist

WORKDIR /home/carla/carla
