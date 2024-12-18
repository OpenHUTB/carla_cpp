// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 定义全局宏，指定使用 C++11 ABI 版本为 0，可能影响 C++ 标准库的行为，如动态链接库中符号的解析方式等
#define _GLIBCXX_USE_CXX11_ABI 0
// 引入 C++ 标准字符串库，用于处理字符串相关操作
#include <string>

namespace carla {
namespace ros2 {  
// CarlaPublisher 类定义，作为发布者的基类，为具体类型发布者提供基础框架和通用接口
  class CarlaPublisher {
    public:  // 获取帧 ID 的函数，返回 const 引用，避免不必要的拷贝，且不允许外部修改返回值
      const std::string& frame_id() const { return _frame_id; }    // 获取名称的函数，返回 const 引用，避免拷贝，且不允许外部修改返回值
      const std::string& name() const { return _name; }   // 获取父级名称的函数，返回 const 引用，避免拷贝，且不允许外部修改返回值
      const std::string& parent() const { return _parent; }
      // 设置帧 ID 的函数，通过右值引用接受参数，使用 std::move 高效转移资源所有权，避免拷贝
      void frame_id(std::string&& frame_id) { _frame_id = std::move(frame_id); }  // 设置名称的函数，通过右值引用接受参数，使用 std::move 高效转移资源所有权，避免拷贝
      void name(std::string&& name) { _name = std::move(name); }   // 设置父级名称的函数，通过右值引用接受参数，使用 std::move 高效转移资源所有权，避免拷贝
      void parent(std::string&& parent) { _parent = std::move(parent); }  
    // 纯虚函数，用于获取发布者发布的数据类型，具体类型发布者必须实现此函数
      virtual const char* type() const = 0;

    public:  // 默认构造函数，使用编译器生成的默认构造函数实现
      CarlaPublisher() = default;    // 析构函数，使用编译器生成的默认析构函数实现，析构函数在子类中可能会被重写以释放特定资源
      virtual ~CarlaPublisher() = default;

    protected:  // 存储帧 ID 的字符串成员变量，初始化为空字符串
      std::string _frame_id = "";//存储名称的字符串成员变量，初始化为空字符串
      std::string _name = "";   // 存储父级名称的字符串成员变量，初始化为空字符串
      std::string _parent = "";
  };
}
}
