#!/bin/bash

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# This script builds a debian package for CARLA.
#
# Usage:
#     $ ./CreateDebian.sh <CARLA-VERSION>
#
# Tested with Ubuntu 14.04, 16.04, 18.04 and 19.10.
#
# IMPORTANT: Add/remove the appropriate folders in Makefile at line 81.

# ==================================================================================================
# -- Variables -------------------------------------------------------------------------------------
# ==================================================================================================

# 检查脚本的第一个参数（即CARLA的版本号）是否为空
if [[ -z $1 ]]; then
  # 如果第一个参数为空，则打印当前日期和时间，并提示缺少必要的参数：CARLA的版本号
  echo "$(date) - Missing mandatory arguments: CARLA version. "
  # 打印当前日期和时间，以及脚本的正确使用方法
  echo "$(date) - Usage: ./CreateDebian.sh [version]. "
  # 退出脚本，并返回状态码1，表示发生错误
  exit 1
fi

# 将第一个参数（即传入的CARLA版本号）赋值给变量CARLA_VERSION
CARLA_VERSION=$1

# 根据CARLA版本号构建CARLA模拟器的目录名
CARLA_DIR=carla-simulator-${CARLA_VERSION}

# 设置CARLA的发布版本下载URL，其中包含版本号
CARLA_RELEASE_URL=https://carla-releases.s3.us-east-005.backblazeb2.com/Linux/CARLA_${CARLA_VERSION}.tar.gz

# 设置附加地图的下载URL，其中也包含版本号
ADDITIONAL_MAPS_URL=https://carla-releases.s3.us-east-005.backblazeb2.com/Linux/AdditionalMaps_${CARLA_VERSION}.tar.gz
# 添加维护者名称
DEBFULLNAME=Carla\ Simulator\ Team
export DEBFULLNAME

# ==================================================================================================
# -- Dependencies ----------------------------------------------------------------------------------
# ==================================================================================================
# 安装所需的依赖项
sudo apt-get install build-essential dh-make

# ==================================================================================================
# -- Download --------------------------------------------------------------------------------------
# ==================================================================================================
mkdir -p carla-debian/"${CARLA_DIR}"
cd carla-debian/"${CARLA_DIR}"

# 定义变量FILE，存储当前目录下ImportAssets.sh脚本的路径
FILE=$(pwd)/ImportAssets.sh

# 检查FILE变量所指向的文件是否存在
if [ -f "$FILE" ]; then
  # 如果文件存在，输出"Package already downloaded!"表示包已经下载好了
  echo "Package already downloaded!"
else
  # 如果文件不存在，则执行以下下载和解压操作
  
  # 使用curl命令从CARLA_RELEASE_URL下载CARLA的发布包，并通过管道传递给tar命令进行解压
  curl "${CARLA_RELEASE_URL}" | tar xz
  
  # 使用wget命令从ADDITIONAL_MAPS_URL下载额外的地图包
  wget "${ADDITIONAL_MAPS_URL}"
  
  # 将下载下来的地图包（假设文件名中包含CARLA_VERSION变量指定的版本号）移动到Import/目录下
  mv AdditionalMaps_"${CARLA_VERSION}".tar.gz Import/
fi

# Importing new maps.
./ImportAssets.sh

#删除不必要的文件
rm CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping.debug
rm CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping.sym

# ==================================================================================================
# -- Debian package --------------------------------------------------------------------------------
# ==================================================================================================
# 更新CarlaUE4.sh脚本
rm CarlaUE4.sh
# 使用 cat 命令和 EOF 标记创建或追加内容到 CarlaUE4.sh 文件中
# 该脚本设置为可执行，并调用 CarlaUE4 的 Linux 发行版二进制文件，传递所有命令行参数给它
cat >> CarlaUE4.sh <<EOF
#!/bin/sh
"/opt/carla-simulator/CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping" CarlaUE4 \$@
EOF

# Making debian package to install Carla in /opt folder
rm Makefile

cat >> Makefile <<EOF
binary:
	# we are not going to build anything

install:
# 创建目标安装路径，确保即使目录不存在也会被创建
	mkdir -p \$(DESTDIR)/opt/carla-simulator/bin
 # 将 CarlaUE4.sh 复制到目标安装路径下的 bin 文件夹中
	cp CarlaUE4.sh \$(DESTDIR)/opt/carla-simulator/bin
	cp ImportAssets.sh \$(DESTDIR)/opt/carla-simulator
	cp -r CarlaUE4 \$(DESTDIR)/opt/carla-simulator
	cp -r Engine \$(DESTDIR)/opt/carla-simulator
	cp -r Import \$(DESTDIR)/opt/carla-simulator
	cp -r PythonAPI \$(DESTDIR)/opt/carla-simulator
	cp -r Co-Simulation \$(DESTDIR)/opt/carla-simulator
	cp -r Tools \$(DESTDIR)/opt/carla-simulator
EOF

# Create necessary file structure for debian packaging
timeout --signal=SIGINT 10 dh_make -e carla.simulator@gmail.com --indep --createorig -y

cd debian/

# 删除不必要文件
rm ./*.ex
rm ./*.EX

# Adding package dependencies(Package will install them itself) and description
rm control

cat >> control <<EOF
Source: carla-simulator
Section: simulator
Priority: optional
Maintainer: Carla Simulator Team <carla.simulator@gmail.com>
Build-Depends: debhelper (>= 9)
Standards-Version: ${CARLA_VERSION}
Homepage: http://carla.org/

Package: carla-simulator
Architecture: any
Depends: python,
  python-numpy,
  python-pygame,
  libpng16-16,
  libjpeg8,
  libtiff5
Description: Open-source simulator for autonomous driving research
 CARLA has been developed from the ground up to support development, training, and validation
 of autonomous driving systems. In addition to open-source code and protocols, CARLA provides
 open digital assets (urban layouts, buildings, vehicles) that were created for this purpose
 and can be used freely. The simulation platform supports flexible specification of sensor suites,
 environmental conditions, full control of all static and dynamic actors, maps generation and much more.
EOF

# Adding Carla library path (carla.pth) to site-packages, during post installation.
rm postinst

cat>> postinst << EOF
#!/bin/sh

SITEDIR=\$(python3 -c 'import site; site._script()' --user-site)
mkdir -p "\$SITEDIR"
PYTHON3_EGG=\$(ls /opt/carla-simulator/PythonAPI/carla/dist | grep py3.)
echo "/opt/carla-simulator/PythonAPI/carla/dist/\$PYTHON3_EGG\n/opt/carla-simulator/PythonAPI/carla/" > "\$SITEDIR/carla.pth"

SITEDIR=\$(python2 -c 'import site; site._script()' --user-site)
mkdir -p "\$SITEDIR"
PYTHON2_EGG=\$(ls /opt/carla-simulator/PythonAPI/carla/dist | grep py2.)
echo "/opt/carla-simulator/PythonAPI/carla/dist/\$PYTHON2_EGG\n/opt/carla-simulator/PythonAPI/carla/" > "\$SITEDIR/carla.pth"

chmod +x /opt/carla-simulator/bin/CarlaUE4.sh

set -e

case "\$1" in
    configure)
    ;;

    abort-upgrade|abort-remove|abort-deconfigure)
    ;;

    *)
        echo "postinst called with unknown argument \\\`\$1'" >&2
        exit 1
    ;;
esac

exit 0
EOF

#从网站软件包中删除Carla库
rm prerm
cat>> prerm << EOF
#!/bin/sh

SITEDIR=\$(python3 -c 'import site; site._script()' --user-site)
rm "\$SITEDIR/carla.pth"

SITEDIR=\$(python2 -c 'import site; site._script()' --user-site)
rm "\$SITEDIR/carla.pth"

set -e

case "\$1" in
    remove|upgrade|deconfigure)
    ;;

    failed-upgrade)
    ;;

    *)
        echo "prerm called with unknown argument \\\`\$1'" >&2
        exit 1
    ;;
esac

exit 0
EOF

#更新版权
rm copyright
cp ../LICENSE ./copyright

# Updating debian/Changelog
awk '{sub(/UNRELEASED/,"stable")}1' changelog > tmp && mv tmp changelog
awk '{sub(/unstable/,"stable")}1' changelog > tmp && mv tmp changelog
cd ..

# Building debian package.
dpkg-buildpackage -uc -us -b

