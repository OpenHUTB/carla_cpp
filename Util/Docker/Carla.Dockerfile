FROM carla-prerequisites:latest#指定基础镜像

ARG GIT_BRANCH

USER carla
WORKDIR /home/carla#切换后续操作用户

RUN cd /home/carla/ && \
  if [ -z ${GIT_BRANCH+x} ]; then git clone --depth 1 https://github.com/carla-simulator/carla.git; \
  else git clone --depth 1 --branch $GIT_BRANCH https://github.com/carla-simulator/carla.git; fi && \
  cd /home/carla/carla && \
  ./Update.sh && \
  make CarlaUE4Editor && \
  make PythonAPI && \  #编译生成build.utils，可能是一些构建相关的工具模块
  make build.utils && \ #进行打包操作，具体打包内容取决于项目配置
  make package && \
  rm -r /home/carla/carla/Dist

WORKDIR /home/carla/carla
