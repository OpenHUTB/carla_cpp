// Copyright (c) 2020 Robotics and Perception Group (GPR)
// University of Zurich and ETH Zurich
// 说明该代码遵循MIT许可协议，可通过对应链接查看协议详情
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入C++标准库中固定宽度整数类型的头文件，后续结构体中的成员变量会用到相关整数类型定义
#include <cstdint>

namespace carla {
namespace sensor {
namespace data {

// 设置结构体的内存对齐方式为1字节对齐，方便精确控制结构体大小，后续会有对应的恢复操作，这样结构体成员会紧密排列
#pragma pack(push, 1)

// 定义DVSEvent结构体，用于表示某种特定事件相关的数据结构，可能与传感器数据等相关（结合所在命名空间推测）
struct DVSEvent {
    // =========================================================================
    // -- 构造函数相关 ----------------------------------------------------------
    // =========================================================================

    // 默认构造函数，使用编译器默认的初始化行为，结构体成员变量会被初始化为其对应类型的默认值（例如数值类型为0，布尔类型为false等）
    /// Default constructor
    DVSEvent() = default;

    // 拷贝构造函数，用于根据已有的DVSEvent对象创建一个新的副本，将传入对象的各个成员变量的值复制给新创建的对象
    /// Copy Constructor
    DVSEvent(const DVSEvent &arg)
      : x(arg.x), y(arg.y), t(arg.t), pol(arg.pol) {}

    // 移动构造函数，用于通过“移动”语义来构造新的DVSEvent对象，它会“窃取”传入的临时对象（右值引用参数）的资源，避免不必要的拷贝操作，提高性能，常用于涉及资源管理的情况
    /// Moving constructor
    DVSEvent(const DVSEvent &&arg)
      : x(std::move(arg.x)),
        y(std::move(arg.y)),
        t(std::move(arg.t)),
        pol(std::move(arg.pol)) {}

    // 带参数的构造函数，用于根据传入的具体参数值初始化DVSEvent结构体对象的各个成员变量，方便按照指定的值来创建事件对象
    /// Constructor
    DVSEvent(std::uint16_t x, std::uint16_t y, std::int64_t t, bool pol)
      : x(x), y(y), t(t), pol(pol) {}

    // =========================================================================
    // -- 赋值运算符相关 --------------------------------------------------------
    // =========================================================================

    // 拷贝赋值运算符重载，用于将一个DVSEvent对象的值复制给另一个已存在的对象，实现对象间数据的赋值操作
    /// Assignement operator
    DVSEvent &operator=(const DVSEvent &other) {
      x = other.x;
      y = other.y;
      t = other.t;
      pol = other.pol;
      return *this;
    }

    // 移动赋值运算符重载，用于将一个临时的（右值引用的）DVSEvent对象的资源“移动”给另一个已存在的对象，同样是为了提高性能，避免不必要的拷贝，实现资源的高效转移赋值
    /// Move Assignement operator
    DVSEvent &operator=(const DVSEvent &&other) {
      x = std::move(other.x);
      y = std::move(other.y);
      t = std::move(other.t);
      pol = std::move(other.pol);
      return *this;
    }

    // =========================================================================
    // -- 比较运算符相关 --------------------------------------------------------
    // =========================================================================

    // 重载==运算符，用于比较两个DVSEvent结构体对象是否相等，通过依次比较各个成员变量（x、y、t、pol）是否分别相等来判断两个对象是否完全相同
    bool operator==(const DVSEvent &rhs) const {
      return (x == rhs.x) && (y == rhs.y) && (t == rhs.t) && (pol == rhs.pol);
    }

    // 重载!=运算符，通过对==运算符的结果取反来判断两个DVSEvent结构体对象是否不相等，提供一种便捷的判断不相等的方式
    bool operator!=(const DVSEvent &rhs) const {
      return !(*this == rhs);
    }

    // 结构体的成员变量，用于存储与该事件相关的具体数据信息

    // 表示事件在某个二维空间中x坐标相关的数据，类型为无符号16位整数，具体含义取决于应用场景（比如可能是图像传感器中像素的横坐标等）
    std::uint16_t x;
    // 表示事件在某个二维空间中y坐标相关的数据，类型为无符号16位整数，类似x坐标，可能对应像素纵坐标等含义
    std::uint16_t y;
    // 表示事件发生的时间相关的数据，类型为有符号64位整数，可用于记录精确的时间戳等信息，方便按时间顺序处理事件等操作
    std::int64_t t;
    // 表示事件的某种极性（或状态等布尔属性）相关的数据，类型为布尔值，具体含义根据具体事件的定义来确定（比如信号的正负、事件的激活与否等）
    bool pol;

  };
// 恢复之前的内存对齐方式，与前面的#pragma pack(push, 1)对应，使后续的内存布局恢复到之前的设置
#pragma pack(pop)
} // namespace data
} // namespace sensor
} // namespace carla
