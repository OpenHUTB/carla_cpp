# 导入必要的模块
# 从setuptools库导入setup和Extension类，这些类用于配置和构建Python扩展模块
from setuptools import setup, Extension
 
# 导入fnmatch模块，它提供了支持Unix shell风格通配符的文件名匹配功能
import fnmatch
# 导入os模块，它提供了一种方便的使用操作系统功能的方式
import os
# 导入sys模块，它提供了一些变量和函数，用以操纵Python运行时环境
import sys  # 注意：虽然在此脚本中未直接使用sys，但它可能用于其他目的，如检查Python版本等
 
# 定义一个函数，用于检查是否启用了RSS变体构建
def is_rss_variant_enabled():
    # 检查环境变量BUILD_RSS_VARIANT是否被设置为'true'
    # 如果设置了，表示启用了RSS变体构建，函数返回True；否则返回False
    if 'BUILD_RSS_VARIANT' in os.environ and os.environ['BUILD_RSS_VARIANT'] == 'true':
        return True
    return False
 
# 定义一个函数，用于获取libcarla扩展模块的配置信息
def get_libcarla_extensions():
    # 指定头文件（.h或.hpp文件）的搜索路径
    include_dirs = ['dependencies/include']
    
    # 指定库文件（.so、.dylib或.dll文件，取决于操作系统）的搜索路径
    library_dirs = ['dependencies/lib']
    
    # 初始化一个空列表，用于存储要链接的库名称（这些名称将在后续可能根据条件动态添加）
    libraries = []
    
    # 指定libcarla扩展模块的源代码文件
    sources = ['source/libcarla/libcarla.cpp']
    
    # 定义一个内部生成器函数，用于遍历指定文件夹下的文件，并支持文件过滤功能
    def walk(folder, file_filter='*'):
        # 使用os.walk遍历指定文件夹及其子文件夹
        for root, _, filenames in os.walk(folder):
            # 使用fnmatch.filter过滤出符合指定模式的文件名
            for filename in fnmatch.filter(filenames, file_filter):
                # 生成并返回文件的完整路径
                yield os.path.join(root, filename)
    
    # 检查当前操作系统是否为POSIX兼容系统（如Linux, macOS等）
    if os.name == "posix":
        # 导入distro模块，它提供了获取Linux发行版信息的函数
        import distro
        # 定义一个列表，包含支持的Linux发行版ID
        supported_dists = ["ubuntu", "debian", "deepin"]
        
        # 获取当前Linux发行版的ID，并将其转换为小写字母
        linux_distro = distro.id().lower()
        # 注意：此处代码被截断，原本可能根据linux_distro的值执行一些操作
        # 例如，根据发行版的不同，动态添加库名称到libraries列表中
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
             # 定义一个列表，包含要链接的静态库文件的完整路径
libraries = [
    os.path.join(pwd, 'dependencies/lib/librpc.a'),  # 链接RPC库
    os.path.join(pwd, 'dependencies/lib/libboost_filesystem.a'),  # 链接Boost文件系统库
    os.path.join(pwd, 'dependencies/lib/libRecast.a'),  # 链接Recast导航网格生成库
    os.path.join(pwd, 'dependencies/lib/libDetour.a'),  # 链接Detour路径查找库
    os.path.join(pwd, 'dependencies/lib/libDetourCrowd.a'),  # 链接DetourCrowd群体路径查找库
    os.path.join(pwd, 'dependencies/lib/libosm2odr.a'),  # 链接OSM到ODR转换器库
    os.path.join(pwd, 'dependencies/lib/libxerces-c.a')  # 链接Xerces-C++ XML解析库
]

# 定义一个列表，包含额外的链接器参数
extra_link_args = ['-lz']  # 链接zlib库

# 定义一个列表，包含额外的编译器参数
extra_compile_args = [
    '-isystem', os.path.join(pwd, 'dependencies/include/system'),  # 指定额外的系统头文件搜索路径
    '-fPIC',  # 生成与位置无关的代码
    '-std=c++14',  # 使用C++14标准
    '-Werror',  # 将所有警告视为错误
    '-Wall',  # 启用所有常见的警告
    '-Wextra',  # 启用额外的警告
    '-Wpedantic',  # 启用更多关于不符合标准的警告
    '-Wno-self-assign-overloaded',  # 禁用关于重载运算符自我赋值的警告
    '-Wdeprecated',  # 启用关于使用已弃用特性的警告
    '-Wno-shadow',  # 禁用关于局部变量隐藏其他变量或参数的警告
    '-Wuninitialized',  # 启用关于使用未初始化变量的警告
    '-Wunreachable-code',  # 启用关于不可达代码的警告
    '-Wpessimizing-move',  # 启用关于可能导致性能下降的移动的警告
    '-Wold-style-cast',  # 启用关于旧式C风格类型转换的警告
    '-Wnull-dereference',  # 启用关于空指针解引用的警告
    '-Wduplicate-enum',  # 启用关于枚举值重复的警告
    '-Wnon-virtual-dtor',  # 启用关于基类析构函数未声明为虚函数的警告
    '-Wheader-hygiene',  # 启用关于头文件包含问题的警告（非标准GCC警告）
    '-Wconversion',  # 启用关于类型转换的警告
    '-Wfloat-overflow-conversion',  # 启用关于浮点溢出转换的警告
    '-DBOOST_ERROR_CODE_HEADER_ONLY',  # 定义BOOST_ERROR_CODE_HEADER_ONLY宏，用于Boost错误代码库的头文件方式
    '-DLIBCARLA_WITH_PYTHON_SUPPORT',  # 定义LIBCARLA_WITH_PYTHON_SUPPORT宏，表示启用Python支持
    '-stdlib=libstdc++'  # 指定使用libstdc++标准库
]

# 如果启用了RSS变体构建，则添加额外的编译和链接参数
if is_rss_variant_enabled():
    extra_compile_args += [
        '-DLIBCARLA_RSS_ENABLED',  # 定义LIBCARLA_RSS_ENABLED宏，表示启用RSS支持
        '-DLIBCARLA_PYTHON_MAJOR_' + str(sys.version_info.major)  # 定义LIBCARLA_PYTHON_MAJOR_x宏，表示Python主版本号
    ]
    
    # 添加与Python版本相关的RSS和其他库的链接路径
    extra_link_args += [
        os.path.join(pwd, 'dependencies/lib/libad_rss_map_integration_python' + str(sys.version_info.major) + str(sys.version_info.minor) + '.a'),
        os.path.join(pwd, 'dependencies/lib/libad_rss_map_integration.a'),
        os.path.join(pwd, 'dependencies/lib/libad_map_access_python' + str(sys.version_info.major) + str(sys.version_info.minor) + '.a'),
        os.path.join(pwd, 'dependencies/lib/libad_map_access.a'),
        os.path.join(pwd, 'dependencies/lib/libad_rss_python' + str(sys.version_info.major) + str(sys.version_info.minor) + '.a'),
        os.path.join(pwd, 'dependencies/lib/libad_rss.a'),
        os.path.join(pwd, 'dependencies/lib/libad_physics_python' + str(sys.version_info.major) + str(sys.version_info.minor) + '.a'),
        os.path.join(pwd, 'dependencies/lib/libad_physics.a'),
        os.path.join(pwd, 'dependencies/lib/libad_map_opendrive_reader.a'),
        os.path.join(pwd, 'dependencies/lib/libboost_program_options.a'),
        os.path.join(pwd, 'dependencies/lib/libodrSpiral.a'),
        os.path.join(pwd, 'dependencies/lib/libspdlog.a')
    ]
    
    # 添加额外的链接器参数
    extra_link_args += [
        '-lrt',  # 链接实时库（Linux特定）
        '-ltbb'  # 链接Intel Threading Building Blocks库
    ]

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

        extra_link_args = ['shlwapi.lib', 'Advapi32.lib', 'ole32.lib', 'shell32.lib']

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
