// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// 确保这个头文件在编译过程中只被包含一次。
#define _GLIBCXX_USE_CXX11_ABI 0// 定义宏以指定使用旧的C++ ABI（应用二进制接口），这通常用于与旧代码的兼容性。

#include <string>// 包含标准库中的字符串头文件，以便可以使用std::string类。

namespace carla {
namespace ros2 {// 定义嵌套的命名空间carla和ros2，用于组织代码。

  class CarlaSubscriber {
    public:
      const std::string& frame_id() const { return _frame_id; }// 提供一个常量方法来获取_frame_id成员变量的引用。
      const std::string& name() const { return _name; }// 提供一个常量方法来获取_name成员变量的引用。
      const std::string& parent() const { return _parent; }// 提供一个常量方法来获取_parent成员变量的引用。

      void frame_id(std::string&& frame_id) { _frame_id = std::move(frame_id); }// 提供一个方法来设置_frame_id成员变量的值，使用std::move来转移所有权。
      void name(std::string&& name) { _name = std::move(name); }// 提供一个方法来设置_name成员变量的值，使用std::move来转移所有权。
      void parent(std::string&& parent) { _parent = std::move(parent); }// 提供一个方法来设置_parent成员变量的值，使用std::move来转移所有权。


      virtual const char* type() const = 0;// 声明一个纯虚函数type，它应该由派生类实现，返回一个C风格字符串。


    public:
      CarlaSubscriber() = default;// 默认构造函数。
      virtual ~CarlaSubscriber() = default;// 虚析构函数，确保派生类的析构函数被正确调用。

    protected:
      std::string _frame_id = "";// 用于存储frame_id的成员变量，默认为空字符串。
      std::string _name = "";// 用于存储name的成员变量，默认为空字符串。
      std::string _parent = "";// 用于存储parent的成员变量，默认为空字符串。
  };
}
}
