// 版权所有 (c) 2017 巴塞罗那自治大学 (UAB) 的计算机视觉中心 (CVC)。
//
// 本作品依据 MIT 许可证的条款进行授权。
// 如需许可证副本，请访问 <https://opensource.org/licenses/MIT>。

#include "carla/Exception.h"  // 引入Carla异常处理头文件
#include <boost/assert/source_location.hpp>  // 引入Boost的源位置头文件

// =============================================================================
// -- 定义boost::throw_exception --------------------------------------------
// =============================================================================

#ifdef BOOST_NO_EXCEPTIONS  // 如果禁用异常

namespace boost {   // 在boost命名空间中定义函数，boost是一个广泛使用的C++库命名空间
 
  // 定义一个函数，用于抛出给定的异常对象。
  // 这个函数是boost命名空间中的一个封装，它实际上调用了另一个（可能是Carla项目中的）抛出异常函数。
  void throw_exception(const std::exception &e) {  // 定义抛出异常函数
    // 调用Carla项目中定义的抛出异常函数，将接收到的异常对象传递给它。
    carla::throw_exception(e);  // 调用Carla的抛出异常函数 
  }

  // 重载上面的throw_exception函数，以便能够同时传递异常对象和源位置信息。
  // 源位置信息有助于开发者在调试时定位异常发生的具体位置。
  void throw_exception(  // 重载的抛出异常函数，带有源位置信息
      const std::exception &e,
      boost::source_location const & loc) {  // 接受异常和源位置
    // 调用上面定义的、不带源位置信息的throw_exception函数，只传递异常对象。
    // 这里没有直接利用源位置信息，但可以在被调用的函数中进一步处理或记录。
    throw_exception(e);  // 调用上面定义的抛出异常函数
  }
  
} // 结束boost命名空间的定义

#endif // BOOST_NO_EXCEPTIONS

// =============================================================================
// -- 处理与rpclib捆绑的Boost.Asio的临时解决方案----------------------------
// =============================================================================

#ifdef ASIO_NO_EXCEPTIONS // 检查是否定义了ASIO_NO_EXCEPTIONS宏

#include <exception>  // 引入标准异常类的头文件
#include <system_error>  // 引入系统错误相关类的头文件
#include <typeinfo>  // 引入运行时类型信息相关的头文件

namespace clmdep_asio {  // 在clmdep_asio命名空间中定义函数，clmdep_asio可能是Asio库的某个封装或别名
namespace detail {  // 在detail子命名空间中定义实现细节，通常用于隐藏内部实现

  // 定义一个模板函数，用于抛出给定的异常对象
  // 这个函数是Asio库（或其封装）中的一个封装，它实际上调用了另一个（可能是Carla项目中的）抛出异常函数
  template <typename Exception>  // 模板参数Exception表示任意异常类型
  void throw_exception(const Exception& e) {  // 函数接受一个异常对象的引用作为参数
    // 调用Carla项目中定义的抛出异常函数，将接收到的异常对象传递给它
    carla::throw_exception(e);  // 调用Carla的抛出异常函数
  }

  template void throw_exception<std::bad_cast>(const std::bad_cast &); //为std::bad_cast类型的异常提供特定的处理逻辑
  template void throw_exception<std::exception>(const std::exception &); //为std::exception及其派生类型的异常提供特定的处理逻辑
  template void throw_exception<std::system_error>(const std::system_error &); //为std::system_error类型的异常提供特定的处理逻辑

} // detail命名空间的结束
} // clmdep_asio命名空间的结束
#endif // 检查ASIO_NO_EXCEPTIONS宏是否定义的结束，用于条件编译
