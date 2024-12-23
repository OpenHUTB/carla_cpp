// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// 这一行是版权声明，表明这段代码（或者相关的软件作品、文档等，具体取决于其所在的完整内容主体）的版权归属方。
// 这里指出版权归属于巴塞罗那自治大学（Universitat Autonoma de Barcelona，简称 UAB）的计算机视觉中心（Computer Vision Center，简称 CVC），
// 时间限定为 2017 年，意味着该版权从 2017 年开始受到相应的法律保护，未经授权不得随意使用、复制、修改等。
//
// This work is licensed under the terms of the MIT license.
// 此句说明这份代码（或相关作品）所采用的授权许可协议类型，即遵循 MIT 许可证（MIT license）的条款来进行授权使用。
// MIT 许可证是一种比较宽松的开源许可证，允许使用者在满足一定条件（如保留版权声明、包含许可证副本等）的基础上，自由地使用、修改、分发相关的代码或作品。
// For a copy, see <https://opensource.org/licenses/MIT>.
// 这一行提供了获取 MIT 许可证副本的具体途径，通过给出的网址（https://opensource.org/licenses/MIT），
// 使用者可以访问该网址查看 MIT 许可证详细的条款内容，便于使用者准确了解在使用这份受 MIT 许可证授权的代码（或作品）时，需要遵循哪些具体的规则和要求。

#include <rpc/config.h> // 包含RPC配置的头文件，可能包含一些预处理器定义或配置选项。
#include <rpc/rpc_error.h> // 包含RPC错误处理相关的类和函数的头文件。
 
#include <sstream> // 包含std::stringstream，用于构建字符串流。
 
// 定义一个函数，用于将rpc::rpc_error转换为Python异常。
void translator(const rpc::rpc_error &e) {
  std::stringstream ss; // 创建一个字符串流对象ss。
  ss << e.what() << " in function " << e.get_function_name(); // 将错误的描述信息和发生错误的函数名添加到字符串流中。
  /// @todo Supposedly we can extract the error string here as provided by the
 // 这里使用了 `@todo` 标记，表明这是一个待办事项的注释说明，意味着此处的代码逻辑存在需要后续进一步完善或者修改的地方。
  /// server with e.get_error().as<std::string>(), but it gives the wrong
 // 接着进一步说明了原本打算提取错误字符串的具体方式，即通过调用 `e.get_error().as<std::string>()` 这个表达式来获取。
  // 这里的 `e` 应该是某个代表错误相关信息的对象（可能是一个异常对象或者包含错误详情的自定义结构体等，具体要结合代码上下文确定），
  // 先通过 `get_error()` 方法获取到错误相关的基础数据，再尝试使用 `as<std::string>()` 方法将其转换为 `std::string` 类型的字符串，
  // 也就是期望以此来得到能够直观展示给用户或者用于日志记录等用途的错误提示字符串内容。
  /// string.
  // 最后指出了当前存在的问题，“but it gives the wrong string”表明按照上述提到的获取错误字符串的方式，实际得到的字符串是不正确的。
  // 这意味着后续需要去排查为什么会得到错误的字符串，可能是 `e` 对象内部的实现问题、`as<std::string>()` 转换方法的逻辑问题，
  // 或者是服务器返回的错误数据格式与预期不符等多种原因导致，需要进一步分析并修正此处代码以正确获取期望的错误字符串。
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
