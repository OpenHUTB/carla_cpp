// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <string>
// 定义 carla 命名空间，可能用于组织与 "Carla" 相关的一系列类、函数等代码结构，使其有独立的作用域，避免命名冲突
namespace carla {
// 在 carla 命名空间内再定义 ros2 命名空间
namespace ros2 {
    // 定义 CarlaSubscriber 类，从名字推测它可能是用于在 ROS2 环境下订阅 Carla 相关消息的基类
  class CarlaSubscriber {
    public:
        // 获取 frame_id（帧 ID）字符串的常引用，通常在 ROS 中帧 ID 用于标识数据所属的参考坐标系等信息，外部可通过此函数获取该成员变量的值，但不能修改它
      const std::string& frame_id() const { return _frame_id; }
      // 获取 name（名称）字符串的常引用，这个名称可能用于标识该订阅者的特定名称，方便区分不同的订阅对象等
      const std::string& name() const { return _name; }
      // 获取 parent（父对象相关）字符串的常引用，可能用于表示该订阅者所属的父级组件或者对象等相关信息
      const std::string& parent() const { return _parent; }
      // 设置 frame_id 成员变量，通过右值引用接收一个新的字符串，并使用 std::move 将资源所有权转移给成员变量，用于更新帧 ID 相关信息
      void frame_id(std::string&& frame_id) { _frame_id = std::move(frame_id); }
      // 设置 name 成员变量，原理同 frame_id 的设置函数，用于更新订阅者名称信息
      void name(std::string&& name) { _name = std::move(name); }
      // 设置 parent 成员变量，用于更新父对象相关信息
      void parent(std::string&& parent) { _parent = std::move(parent); }

      virtual const char* type() const = 0;

    public:
      CarlaSubscriber() = default;
      // 虚析构函数，默认实现，用于在派生类对象销毁时正确地进行内存清理等析构相关操作
      virtual ~CarlaSubscriber() = default;

    protected:
        // 存储帧 ID 的字符串成员变量，初始化为空字符串，用于保存与该订阅者相关的数据的参考坐标系等标识信息
      std::string _frame_id = "";
      // 存储订阅者名称的字符串成员变量，初始化为空字符串，用于区分不同的订阅者
      std::string _name = "";
      // 存储父对象相关信息的字符串成员变量，初始化为空字符串，可能用于表示所属的上级组件等情况
      std::string _parent = "";
  };
}
}
