// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 表明该代码遵循MIT许可协议，可通过对应链接查看具体协议内容
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入Carla项目中rpc相关的Color和FloatColor头文件，用于后续颜色相关类型转换等操作
#include "carla/rpc/Color.h"
#include "carla/rpc/FloatColor.h"

// 引入C++标准库中固定宽度整数类型的头文件，可能在更广泛的代码上下文中涉及相关整数操作
#include <cstdint>

namespace carla {
namespace sensor {
namespace data {

// 设置结构体的内存对齐方式为1字节对齐，方便精确控制结构体大小，后续会有对应的恢复操作
#pragma pack(push, 1)

// 结构体Color用于表示一个32位的BGRA颜色格式的数据结构
  /// A 32-bit BGRA color.
  struct Color {
    // 默认构造函数，使用编译器默认的初始化方式，将成员变量初始化为默认值（此处成员变量默认为0）
    Color() = default;
    // 默认的拷贝构造函数，允许以默认方式进行对象的拷贝构造操作
    Color(const Color &) = default;

    // 带参数的构造函数，用于根据传入的红（r）、绿（g）、蓝（b）以及可选的透明度（a，默认值为255）值初始化Color结构体对象
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255u)
      : b(b), g(g), r(r), a(a) {}

    // 默认的赋值运算符重载，使用编译器默认的赋值行为，实现对象之间简单的赋值操作
    Color &operator=(const Color &) = default;

    // 重载==运算符，用于比较两个Color结构体对象是否相等，通过比较红、绿、蓝三个颜色分量是否分别相等来判断（此处未考虑透明度在相等比较中的作用）
    bool operator==(const Color &rhs) const  {
      return (r == rhs.r) && (g == rhs.g) && (b == rhs.b);
    }

    // 重载!=运算符，通过对==运算符的结果取反来判断两个Color结构体对象是否不相等
    bool operator!=(const Color &rhs) const  {
      return !(*this == rhs);
    }

    // 类型转换运算符重载，将当前Color结构体对象隐式转换为rpc::Color类型，返回仅包含红、绿、蓝分量的rpc::Color对象（忽略透明度）
    operator rpc::Color() const {
      return {r, g, b};
    }

    // 类型转换运算符重载，将当前Color结构体对象隐式转换为rpc::FloatColor类型，将红、绿、蓝、透明度分量都转换为0到1之间的浮点数（通过除以255.f实现），用于需要浮点数表示颜色的场景
    operator rpc::FloatColor() const {
      return {r/255.f, g/255.f, b/255.f, a/255.f};
    }

    // 表示蓝色分量的成员变量，类型为无符号8位整数，初始值为0
    uint8_t b = 0u;
    // 表示绿色分量的成员变量，类型为无符号8位整数，初始值为0
    uint8_t g = 0u;
    // 表示红色分量的成员变量，类型为无符号8位整数，初始值为0
    uint8_t r = 0u;
    // 表示透明度分量的成员变量，类型为无符号8位整数，初始值为0
    uint8_t a = 0u;
    // 用于和msgpack序列化库配合，定义如何将结构体的成员数组（按顺序为r、g、b、a）进行序列化操作，便于数据存储和传输等需求
    MSGPACK_DEFINE_ARRAY(r, g, b, a);
  };
// 恢复之前的内存对齐方式，与前面的#pragma pack(push, 1)对应
#pragma pack(pop)

// 静态断言，在编译时检查Color结构体的大小是否等于uint32_t类型的大小（即4字节），若不相等则报错并显示指定错误信息
static_assert(sizeof(Color) == sizeof(uint32_t), "Invalid color size!");

// 设置结构体的内存对齐方式为1字节对齐，方便精确控制结构体大小，后续会有对应的恢复操作
#pragma pack(push, 1)

// 结构体OpticalFlowPixel用于表示光流像素格式的数据，包含两个通道的浮点数数据（可能代表光流的水平和垂直分量等含义）
  /// Optical flow pixel format. 2 channel float data.
  struct OpticalFlowPixel {
    // 默认构造函数，使用编译器默认的初始化方式，将成员变量初始化为默认值（此处成员变量默认为0）
    OpticalFlowPixel() = default;
    // 默认的拷贝构造函数，允许以默认方式进行对象的拷贝构造操作
    OpticalFlowPixel(const OpticalFlowPixel &) = default;

    // 带参数的构造函数，用于根据传入的x和y坐标值（可能代表光流像素在某个坐标空间下的位移等相关含义）初始化OpticalFlowPixel结构体对象
    OpticalFlowPixel(float x, float y)
      : x(x), y(y) {}

    // 默认的赋值运算符重载，使用编译器默认的赋值行为，实现对象之间简单的赋值操作
    OpticalFlowPixel &operator=(const OpticalFlowPixel &) = default;

    // 重载==运算符，用于比较两个OpticalFlowPixel结构体对象是否相等，通过比较x和y两个坐标分量是否分别相等来判断
    bool operator==(const OpticalFlowPixel &rhs) const  {
      return (x == rhs.x) && (y == rhs.y);
    }

    // 重载!=运算符，通过对==运算符的结果取反来判断两个OpticalFlowPixel结构体对象是否不相等
    bool operator!=(const OpticalFlowPixel &rhs) const  {
      return !(*this == rhs);
    }

    // 表示光流像素相关的x坐标（可能是水平方向分量等含义）的成员变量，类型为浮点数，初始值为0
    float x = 0;
    // 表示光流像素相关的y坐标（可能是垂直方向分量等含义）的成员变量，类型为浮点数，初始值为0
    float y = 0;
    // 用于和msgpack序列化库配合，定义如何将结构体的成员数组（按顺序为x、y）进行序列化操作，便于数据存储和传输等需求
    MSGPACK_DEFINE_ARRAY(x, y);
  };
// 恢复之前的内存对齐方式，与前面的#pragma pack(push, 1)对应
#pragma pack(pop)

} // namespace data
} // namespace sensor
} // namespace carla
