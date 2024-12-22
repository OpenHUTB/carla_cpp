#!/usr/bin/env python

# 版权所有 （c） 2019 巴塞罗那自治大学 （UAB） 计算机视觉中心 （CVC）。
# 本作品根据 MIT 许可证的条款进行许可。
# 有关副本，请参阅 <https://opensource.org/licenses/MIT>。

# 从setuptools库导入setup和Extension类，用于配置和构建Python扩展模块
from setuptools import setup, Extension
 
# 导入fnmatch模块，用于文件名匹配
import fnmatch
# 导入os模块，用于与操作系统交互，如环境变量、文件路径等
import os
# 导入sys模块，用于访问与Python解释器紧密相关的变量和函数
import sys
 
# 定义一个函数，检查是否启用了RSS变体构建
def is_rss_variant_enabled():
    # 检查环境变量BUILD_RSS_VARIANT是否被设置为'true'
    if 'BUILD_RSS_VARIANT' in os.environ and os.environ['BUILD_RSS_VARIANT'] == 'true':
        return True
    return False
 
# 定义一个函数，用于获取libcarla扩展模块的配置信息
def get_libcarla_extensions():
    # 指定头文件搜索路径
    include_dirs = ['dependencies/include']
    
    # 指定库文件搜索路径
    library_dirs = ['dependencies/lib']
    # 初始化库名称列表（这里可能根据条件动态添加）
    libraries = []
 
    # 指定源代码文件
    sources = ['source/libcarla/libcarla.cpp']
 
    # 定义一个生成器函数，用于遍历指定文件夹下的文件，支持文件过滤
    def walk(folder, file_filter='*'):
        for root, _, filenames in os.walk(folder):
            for filename in fnmatch.filter(filenames, file_filter):
                yield os.path.join(root, filename)
 
    # 检查当前操作系统是否为POSIX兼容系统（如Linux, macOS等）
    if os.name == "posix":
        import distro  # 导入distro模块，用于获取Linux发行版信息
        supported_dists = ["ubuntu", "debian", "deepin"]  # 支持的Linux发行版列表
        
        # 获取当前Linux发行版的ID并转换为小写
        linux_distro = distro.id().lower()
        # 如果当前发行版在支持的列表中
        if linux_distro in supported_dists:
            pwd = os.path.dirname(os.path.realpath(__file__))  # 获取当前脚本所在目录的绝对路径
            # 根据Python版本构建Boost Python库的名称
            pylib = "libboost_python%d%d.a" % (sys.version_info.major, sys.version_info.minor)
            # 检查是否启用了RSS变体构建
            if is_rss_variant_enabled():
                print('Building AD RSS variant.')
                # 如果是，则添加RSS相关的库文件到链接参数中
                extra_link_args = [ os.path.join(pwd, 'dependencies/lib/libcarla_client_rss.a') ]
            else:
                # 否则，添加普通的Carla客户端库文件到链接参数中
                extra_link_args = [ os.path.join(pwd, 'dependencies/lib/libcarla_client.a') ]
                
            extra_link_args += [#构建列表参数
                os.path.join(pwd, 'dependencies/lib/librpc.a'),#将路径的各个组成部分组合成完整的路径
                os.path.join(pwd, 'dependencies/lib/libboost_filesystem.a'),
                os.path.join(pwd, 'dependencies/lib/libRecast.a'),
                os.path.join(pwd, 'dependencies/lib/libDetour.a'),
                os.path.join(pwd, 'dependencies/lib/libDetourCrowd.a'),
                os.path.join(pwd, 'dependencies/lib/libosm2odr.a'),
                os.path.join(pwd, 'dependencies/lib/libxerces-c.a')]
            extra_link_args += ['-lz']#编译参数列表
            extra_compile_args = [
                '-isystem', os.path.join(pwd, 'dependencies/include/system'), '-fPIC', '-std=c++14',#指定额外的系统文件搜索路径
                '-Werror',#将警告当作错误处理
                '-Wall', #开启大多数常见的警告
                '-Wextra', #开启额外的警告
                '-Wpedantic', #阐述更多关于不符合标志的警告
                '-Wno-self-assign-overloaded',
                '-Wdeprecated', '-Wno-shadow', '-Wuninitialized', '-Wunreachable-code',
                '-Wpessimizing-move', '-Wold-style-cast', '-Wnull-dereference',
                '-Wduplicate-enum', '-Wnon-virtual-dtor', '-Wheader-hygiene',
                '-Wconversion', '-Wfloat-overflow-conversion',
                '-DBOOST_ERROR_CODE_HEADER_ONLY', '-DLIBCARLA_WITH_PYTHON_SUPPORT',
                '-stdlib=libstdc++'
            ]
            if is_rss_variant_enabled():
                extra_compile_args += ['-DLIBCARLA_RSS_ENABLED']
                extra_compile_args += ['-DLIBCARLA_PYTHON_MAJOR_' +  str(sys.version_info.major)]
               # 检查 libad_rss_map_integration_python 相关库文件是否存在
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_rss_map_integration_python' +  str(sys.version_info.major) + str(sys.version_info.minor) + '.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_rss_map_integration.a')]
               # 检查 libad_rss_map_integration 库文件是否存在
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_map_access_python' +  str(sys.version_info.major) + str(sys.version_info.minor) + '.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_map_access.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_rss_python' +  str(sys.version_info.major) + str(sys.version_info.minor) + '.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_rss.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_physics_python' +  str(sys.version_info.major) + str(sys.version_info.minor) + '.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_physics.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_map_opendrive_reader.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libboost_program_options.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libodrSpiral.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libspdlog.a')]
                extra_link_args += ['-lrt']
                extra_link_args += ['-ltbb']

            # rss_variant还需要 libproj、libsqlite 和 python 库，因此将它们放在rss_variant链接库之后
            extra_link_args += [os.path.join(pwd, 'dependencies/lib/libproj.a'),
                                os.path.join(pwd, 'dependencies/lib/libsqlite3.a'),
                                os.path.join(pwd, 'dependencies/lib', pylib)]

            if 'TRAVIS' in os.environ and os.environ['TRAVIS'] == 'true':
                print('Travis CI build detected: disabling PNG support.')
                extra_link_args += ['-ljpeg', '-ltiff']
                extra_compile_args += ['-DLIBCARLA_IMAGE_WITH_PNG_SUPPORT=false']
            else:
                extra_link_args += ['-lpng', '-ljpeg', '-ltiff']
                extra_compile_args += ['-DLIBCARLA_IMAGE_WITH_PNG_SUPPORT=true']
            # @todo 我们为什么需要这个？
            # include_dirs += ['/usr/lib/gcc/x86_64-linux-gnu/7/include']
            # library_dirs += ['/usr/lib/gcc/x86_64-linux-gnu/7']
            # extra_link_args += ['/usr/lib/gcc/x86_64-linux-gnu/7/libstdc++.a']
            extra_link_args += ['-lstdc++']
        else: 
            raise NotImplementedError(linux_distro + " not in supported posix platforms: " + str(supported_dists))
    elif os.name == "nt":
        pwd = os.path.dirname(os.path.realpath(__file__))
        pylib = 'libboost_python%d%d' % (
            sys.version_info.major,
            sys.version_info.minor)
        # 初始化额外的链接参数列表，添加一些在 Windows 下链接时需要的库名称，这些库都是 Windows 系统中常用的系统库或者项目依赖的相关库，
        # 比如 shlwapi.lib 提供了一些 Windows Shell 相关的实用函数，Advapi32.lib 包含了很多高级 Windows API 函数等，后续链接阶段会根据这些名称去查找并链接对应的库文件。
        extra_link_args = ['shlwapi.lib', 'Advapi32.lib', 'ole32.lib', 'shell32.lib']
          # 定义一个列表，记录了在 Windows 环境下项目需要链接的一些必要的库名称，包括前面构造的 Boost.Python 相关库、文件系统操作相关的库、
          # 以及项目中其他自定义或者依赖的各种库（如 carla_client.lib、libpng.lib 等），这些库在后续链接阶段都是必须要正确链接上才能保证项目正常编译和运行的。
        required_libs = [
            pylib, 'libboost_filesystem',
            'rpc.lib', 'carla_client.lib',
            'libpng.lib', 'zlib.lib',
            'Recast.lib', 'Detour.lib', 'DetourCrowd.lib',
            'xerces-c_3.lib', 'sqlite3.lib',
            'proj.lib', 'osm2odr.lib']

        # 在 'PythonAPIcarladependencieslib' 中搜索文件名中包含 required_libs 中列出的名称的文件
        libs = [x for x in os.listdir('dependencies/lib') if any(d in x for d in required_libs)]

        for lib in libs:
            extra_link_args.append(os.path.join(pwd, 'dependencies/lib', lib))

        # https://docs.microsoft.com/es-es/cpp/porting/modifying-winver-and-win32-winnt
        extra_compile_args = [
            '/experimental:external', '/external:W0', '/external:I', 'dependencies/include/system',
            '/DBOOST_ALL_NO_LIB', '/DBOOST_PYTHON_STATIC_LIB',
            '/DBOOST_ERROR_CODE_HEADER_ONLY', '/D_WIN32_WINNT=0x0600', '/DHAVE_SNPRINTF',
            '/DLIBCARLA_WITH_PYTHON_SUPPORT', '-DLIBCARLA_IMAGE_WITH_PNG_SUPPORT=true', '/MD']
    else:
        raise NotImplementedError

    depends = [x for x in walk('source/libcarla')]
    depends += [x for x in walk('dependencies')]

    def make_extension(name, sources):

        return Extension(
            name,             # 模块名：carla.libcarla
            sources=sources,  # 源代码：PythonAPI/carla/source/libcarla/libcarla.cpp
            include_dirs=include_dirs,  # 头文件目录：dependencies/include
            library_dirs=library_dirs,  # 库文件目录：dependencies/lib
            libraries=libraries,        # 标准的库搜索路径中需要链接的库：暂时为空
            extra_compile_args=extra_compile_args,  # 额外的编译参数
            extra_link_args=extra_link_args,        # 额外的链接参数
            language='c++14',  # 使用的语言为C++14
            depends=depends)   # depends选项是扩展所依赖的文件列表（例如头文件）。

    print('compiling:\n  - %s' % '\n  - '.join(sources))

 # 返回一个列表，列表元素是由'make_extension'函数处理'carla.libcarla'和'sources'参数得到的结果
    return [make_extension('carla.libcarla', sources)]

# 定义获取许可证的函数
def get_license():
    if is_rss_variant_enabled():
        return 'LGPL-v2.1-only License'
    return 'MIT License'

with open("README.md") as f:
 # 读取文件内容并赋值给long_description
    long_description = f.read()

# 调用setup函数进行项目配置
setup(
    name='carla', # 项目名称
    version='0.9.15', # 项目版本
    package_dir={'': 'source'}, # 包目录，这里设置为项目根目录下的'source'文件夹
    packages=['carla'], # 要包含的包名列表
    ext_modules=get_libcarla_extensions(), # 获取扩展模块
    license=get_license(), # 获取许可证
    description='Python API for communicating with the CARLA server.', # 项目简短描述
    long_description=long_description,  # 项目详细描述，与之前读取的README.md内容相同
    long_description_content_type='text/markdown', # 详细描述的内容类型为markdown文本
    url='https://github.com/carla-simulator/carla', # 项目的URL
    author='The CARLA team',   # 项目作者
    author_email='carla.simulator@gmail.com', # 作者邮箱
    include_package_data=True) # 是否包含包数据，这里设置为True
