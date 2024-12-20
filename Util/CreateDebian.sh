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
# 判断是否传入了第一个参数，如果参数为空（即没有传入CARLA版本号）
if [[ -z $1 ]];
then
 # 打印提示信息，显示当前时间以及缺少CARLA版本号这个必需参数的错误提示
  echo "$(date) - Missing mandatory arguments: CARLA version. "
   # 打印脚本的使用方式提示信息
  echo "$(date) - Usage: ./CreateDebian.sh [version]. "
   # 以状态码1退出脚本，表示参数错误导致执行失败
  exit 1
fi
# 将传入的第一个参数（CARLA版本号）赋值给变量CARLA_VERSION，用于后续操作中表示版本相关信息
CARLA_VERSION=$1
# 构建CARLA目录名称，格式为carla-simulator-加上传入的版本号，用于后续操作中表示CARLA相关文件的存放目录等情况
CARLA_DIR=carla-simulator-${CARLA_VERSION}
# 定义CARLA发布版本的下载链接，根据传入的版本号拼接出对应的.tar.gz文件下载地址，用于下载CARLA主程序文件
CARLA_RELEASE_URL=https://carla-releases.s3.us-east-005.backblazeb2.com/Linux/CARLA_${CARLA_VERSION}.tar.gz
# 定义额外地图的下载链接，同样根据版本号拼接出对应的.tar.gz文件下载地址，用于下载CARLA相关的额外地图文件
ADDITIONAL_MAPS_URL=https://carla-releases.s3.us-east-005.backblazeb2.com/Linux/AdditionalMaps_${CARLA_VERSION}.tar.gz
# 添加维护者名称，设置环境变量DEBFULLNAME，用于后续在Debian软件包相关配置文件中显示维护者信息，这里将维护者名称设置为Carla Simulator Team，注意名称中的空格使用\进行转义
# Adding maintainer name.
DEBFULLNAME=Carla\ Simulator\ Team
export DEBFULLNAME

# ==================================================================================================
# -- Dependencies ----------------------------------------------------------------------------------
# ==================================================================================================
# Installing required dependencies.
sudo apt-get install build-essential dh-make

# ==================================================================================================
# -- Download --------------------------------------------------------------------------------------
# ==================================================================================================
# 创建carla-debian目录以及其下对应的CARLA版本目录，如果目录已存在则不会重复创建，用于后续存放下载和解压后的CARLA相关文件
mkdir -p carla-debian/"${CARLA_DIR}"
# 切换到刚创建的CARLA版本目录下，后续操作都在此目录及其子目录中进行
cd carla-debian/"${CARLA_DIR}"
# 定义一个变量FILE，其值为当前目录下的ImportAssets.sh文件的路径，用于后续判断该文件是否存在，以确定是否已经下载过相关文件
FILE=$(pwd)/ImportAssets.sh
# 判断ImportAssets.sh文件是否存在，如果存在则表示相关文件可能已经下载过，打印提示信息
if [ -f "$FILE" ]; then
  echo "Package already downloaded!"
else
# 如果文件不存在，使用curl命令从CARLA_RELEASE_URL指定的链接下载CARLA主程序文件，并通过管道传递给tar命令进行解压（xz格式解压）
  curl "${CARLA_RELEASE_URL}" | tar xz
# 使用wget命令从ADDITIONAL_MAPS_URL指定的链接下载额外地图文件
  wget "${ADDITIONAL_MAPS_URL}"
   # 将下载的额外地图文件（文件名带有版本号）移动到Import目录下，可能是为了后续统一处理相关资源
  mv AdditionalMaps_"${CARLA_VERSION}".tar.gz Import/
fi
# 执行ImportAssets.sh脚本，可能用于导入新的地图资源或者进行一些与资产相关的初始化操作（具体功能取决于该脚本内部实现）
# Importing new maps.
./ImportAssets.sh
# 删除CarlaUE4/Binaries/Linux目录下的CarlaUE4-Linux-Shipping.debug文件，可能是因为该文件在最终的软件包中不需要或者是调试相关的临时文件等原因
# Removing unnecessary files
rm CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping.debug
# 删除CarlaUE4/Binaries/Linux目录下的CarlaUE4-Linux-Shipping.sym文件，同理可能是不需要的符号文件之类的情况
rm CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping.sym

# ==================================================================================================
# -- Debian package --------------------------------------------------------------------------------
# ==================================================================================================
# 删除当前目录下的CarlaUE4.sh脚本文件，可能是要重新生成或更新该脚本内容
# Updating CarlaUE4.sh script
rm CarlaUE4.sh
# 使用cat命令向CarlaUE4.sh文件追加内容，这里定义了一个简单的shell脚本逻辑，用于执行位于/opt/carla-simulator/CarlaUE4/Binaries/Linux/目录下的CarlaUE4-Linux-Shipping可执行文件，并传递后续传入的所有参数（$@表示所有参数）
cat >> CarlaUE4.sh <<EOF
#!/bin/sh
"/opt/carla-simulator/CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping" CarlaUE4 \$@
EOF
# 删除当前目录下的Makefile文件，准备重新生成用于构建Debian软件包的Makefile内容
# Making debian package to install Carla in /opt folder
rm Makefile
# 使用cat命令向Makefile文件追加内容，定义了Makefile中的两个目标（target）：
# - binary目标：注释说明不会进行实际的构建操作（可能在这个脚本场景下不需要重新编译代码等构建行为）
# - install目标：用于将相关文件安装到指定的目录结构中，创建/opt/carla-simulator/bin目录，然后将CarlaUE4.sh、ImportAssets.sh以及CarlaUE4、Engine、Import、PythonAPI、Co-Simulation、Tools等目录及其内容复制到/opt/carla-simulator/目录下，以便后续安装软件包时能正确部署这些文件到相应位置
cat >> Makefile <<EOF
binary:
	# we are not going to build anything

install:
	mkdir -p \$(DESTDIR)/opt/carla-simulator/bin
	cp CarlaUE4.sh \$(DESTDIR)/opt/carla-simulator/bin
	cp ImportAssets.sh \$(DESTDIR)/opt/carla-simulator
	cp -r CarlaUE4 \$(DESTDIR)/opt/carla-simulator
	cp -r Engine \$(DESTDIR)/opt/carla-simulator
	cp -r Import \$(DESTDIR)/opt/carla-simulator
	cp -r PythonAPI \$(DESTDIR)/opt/carla-simulator
	cp -r Co-Simulation \$(DESTDIR)/opt/carla-simulator
	cp -r Tools \$(DESTDIR)/opt/carla-simulator
EOF
# 创建Debian软件包所需的必要文件结构，并使用dh_make工具进行初始化，指定了维护者的邮箱地址、独立模式（--indep）、创建原始文件（--createorig）以及自动回答yes（-y），超时时间设置为10秒，若超时则发送SIGINT信号中断操作
# Create necessary file structure for debian packaging
# Create necessary file structure for debian packaging
timeout --signal=SIGINT 10 dh_make -e carla.simulator@gmail.com --indep --createorig -y
# 切换到刚创建的debian目录下，后续对Debian软件包相关配置文件的操作都在此目录中进行
cd debian/
# 删除当前目录下所有以.ex和.EX为后缀的文件，可能是一些不需要的示例文件或者临时文件等
# Removing unnecessary files
rm ./*.ex
rm ./*.EX
# 删除当前目录下的control文件，准备重新生成用于定义软件包依赖关系、维护者信息、描述等内容的配置文件
# Adding package dependencies(Package will install them itself) and description
rm control
# 使用cat命令向control文件追加内容，定义了软件包的源名称（Source）、所属分类（Section）、优先级（Priority）、维护者信息（包括姓名和邮箱）、构建依赖（Build-Depends）、标准版本（Standards-Version）、主页（Homepage）以及软件包名称（Package）、架构（Architecture）、依赖的其他软件包（Depends）和详细的软件包描述（Description），详细说明了CARLA软件包的相关属性以及其功能特点，例如用于自动驾驶研究的开源模拟器，支持传感器套件配置、环境条件设置、地图生成等功能
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
# 删除当前目录下的postinst文件，准备重新生成用于在软件包安装后执行的脚本内容，主要用于添加CARLA库路径到Python的site-packages目录中
# Adding Carla library path (carla.pth) to site-packages, during post installation.
rm postinst
# 使用cat命令向postinst文件追加内容，定义了一个shell脚本逻辑：
# - 首先通过python3和python2分别获取用户目录下的site-packages路径（即Python的模块搜索路径），并创建对应的目录（如果不存在）。
# - 然后在CARLA软件包的PythonAPI/carla/dist目录下查找对应的Python 3和Python 2版本的egg文件（Python的一种打包格式），将其路径以及CARLA的PythonAPI/carla/目录路径写入到对应的site-packages目录下的carla.pth文件中，这样Python解释器就能找到CARLA相关的Python模块了。
# - 最后给/opt/carla-simulator/bin/CarlaUE4.sh文件添加可执行权限，并处理了一些不同参数情况下的逻辑，正常情况下执行完相关操作后以状态码0退出脚本，表示安装后脚本执行成功。
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
# 删除当前目录下的prerm文件，准备重新生成用于在软件包移除等操作前执行的脚本内容，主要用于移除之前添加到Python的site-packages目录中的CARLA库路径相关文件
# Removing Carla library from site-packages
rm prerm
# 使用cat命令向prerm文件追加内容，定义了一个shell脚本逻辑：
# - 通过python3和python2分别获取用户目录下的site-packages路径，然后删除对应的carla.pth文件，这样在软件包移除等操作时就能清理相关的Python模块路径配置了。
# - 同样处理了一些不同参数情况下的逻辑，正常情况下执行完相关操作后以状态码0退出脚本，表示移除前脚本执行成功。
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
# 删除当前目录下的copyright文件，将上级目录（../）的LICENSE文件复制过来作为新的版权文件，用于更新Debian软件包的版权信息
# Updating copyright.
rm copyright
cp ../LICENSE ./copyright
# 更新debian目录下的changelog文件，将文件中的UNRELEASED替换为stable，以及将unstable替换为stable，可能是用于将版本状态标记为稳定版本相关的处理（具体取决于软件包发布管理的需求）
# Updating debian/Changelog
awk '{sub(/UNRELEASED/,"stable")}1' changelog > tmp && mv tmp changelog
awk '{sub(/unstable/,"stable")}1' changelog > tmp && mv tmp changelog
# 切换回上级目录，回到之前构建Debian软件包的主目录下
cd ..
# 使用dpkg-buildpackage命令构建Debian软件包，-uc选项表示不生成和包含原始的源文件包（.orig.tar.gz等），-us选项表示不进行签名操作，-b选项表示构建二进制软件包，即生成最终可用于安装的.deb文件
# Building debian package.
dpkg-buildpackage -uc -us -b

