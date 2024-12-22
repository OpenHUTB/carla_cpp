// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <rpc/config.h> // 包含RPC配置的头文件，可能包含一些预处理器定义或配置选项。
#include <rpc/rpc_error.h> // 包含RPC错误处理相关的类和函数的头文件。
 
#include <sstream> // 包含std::stringstream，用于构建字符串流。
 
// 定义一个函数，用于将rpc::rpc_error转换为Python异常。
void translator(const rpc::rpc_error &e) {
  std::stringstream ss; // 创建一个字符串流对象ss。
  ss << e.what() << " in function " << e.get_function_name(); // 将错误的描述信息和发生错误的函数名添加到字符串流中。
  /// @todo Supposedly we can extract the error string here as provided by the
  /// server with e.get_error().as<std::string>(), but it gives the wrong
  /// string.
  // TODO：理论上，我们可以从服务器提供的错误信息中提取错误字符串，使用e.get_error().as<std::string>()，
  // 但这给出的字符串是错误的。
  PyErr_SetString(PyExc_RuntimeError, ss.str().c_str()); // 使用Python的C API设置运行时错误异常，异常信息为ss中构建的字符串。
}
 
// 定义一个函数，用于注册RPC错误的异常转换器。
void export_exception() {
  using namespace boost::python; // 使用Boost.Python命名空间，简化代码。
  namespace cc = carla::client; // 定义一个别名cc，代表carla::client命名空间，假设这与CARLA仿真框架相关。
 
  // 注册rpc::rpc_error类型的异常转换器，当这种异常在C++代码中抛出时，将调用translator函数来处理它。
  register_exception_translator<rpc::rpc_error>(translator); 
}
