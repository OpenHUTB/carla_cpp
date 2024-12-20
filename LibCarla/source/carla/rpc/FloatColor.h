// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>
// 判断是否是从 UE4 中包含进来的这个头文件（通过定义的宏 LIBCARLA_INCLUDED_FROM_UE4 来判断），如果是从 UE4 引入的话，就包含下面相关的 UE4 特定的头文件并进行一些宏相关的启用和禁用操作，以适配在 UE4 环境下的编译和使用情况
#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Math/Color.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4
// 定义 carla 命名空间及其子命名空间 rpc，通过这样的命名空间结构来组织代码，避免不同模块间的命名冲突，清晰地划分功能所属的范围，当前定义的 FloatColor 结构体就在 rpc 这个子命名空间下
namespace carla {
namespace rpc {
    // 使用 #pragma pack(push, 1) 指令来设置结构体的内存对齐方式为 1 字节对齐。这样做可以精确控制结构体在内存中的布局，避免编译器自动进行的默认对齐优化（默认对齐方式可能会根据平台和编译器设置导致结构体占用更多内存空间），在一些需要精确控制数据存储格式（比如与特定协议、文件格式等交互时）的场景中很有用。后续与之对应的 #pragma pack(pop) 用于恢复之前的内存对齐设置
#pragma pack(push, 1)
    // 定义一个名为 FloatColor 的结构体，从名字来看它用于表示一个包含浮点数类型的颜色数据结构，通常可以用于更精确地表示颜色信息（相比于整数类型表示颜色，浮点数能表示更细腻的色彩过渡等情况）
  struct FloatColor {
  public:
        // 定义一个浮点数类型的成员变量 r，用于表示红色通道的值，范围通常是 0.0f（表示没有红色成分）到 1.0f（表示最强的红色），初始值设为 0.f
    float r = 0.f;
        // 定义一个浮点数类型的成员变量 g，用于表示绿色通道的值，同理范围是 0.0f - 1.0f，初始值设为 0.f
    float g = 0.f;
        // 定义一个浮点数类型的成员变量 b，用于表示蓝色通道的值，范围也是 0.0f - 1.0f，初始值设为 0.f
    float b = 0.f;
        // 定义一个浮点数类型的成员变量 a，用于表示透明度（Alpha）通道的值，范围 0.0f（完全透明）到 1.0f（完全不透明），默认初始值设为 1.f，表示不透明
    float a = 1.f;
        // 默认构造函数，使用 = default 语法让编译器自动生成默认的构造函数实现，这个构造函数会按照成员变量的初始化列表进行初始化（这里就是将各颜色通道初始化为上面设定的默认值），方便创建一个默认颜色值的 FloatColor 对象
    FloatColor() = default;
        // 拷贝构造函数，同样使用 = default 让编译器自动生成默认的拷贝构造函数实现，用于在以一个已存在的 FloatColor 对象初始化另一个对象时进行成员变量的逐位拷贝操作，保证对象的正确复制
    FloatColor(const FloatColor &) = default;
        // 自定义的构造函数，接受红、绿、蓝、透明度四个浮点数参数（透明度有默认值 1.f），用于创建一个具有指定颜色值的 FloatColor 对象，通过初始化列表将传入的参数赋值给对应的成员变量
    FloatColor(float r, float g, float b, float a = 1.f)
      : r(r), g(g), b(b), a(a) {}
        // 重载的赋值运算符（=），使用 = default 让编译器自动生成默认的赋值运算符实现，用于将一个 FloatColor 对象的值赋给另一个对象时进行成员变量的逐位赋值操作，实现对象之间的赋值功能
    FloatColor &operator=(const FloatColor &) = default;
        // 重载的相等运算符（==），用于比较两个 FloatColor 对象是否相等。比较逻辑是依次对比红色、绿色、蓝色、透明度这四个成员变量的值是否分别相等，如果都相等则返回 true，表示两个对象颜色值完全相同；只要有一个成员变量的值不同，就返回 false，表示两个对象不相等
    bool operator==(const FloatColor &rhs) const  {
      return (r == rhs.r) && (g == rhs.g) && (b == rhs.b) && (a == rhs.a);
    }
        // 重载的不等运算符（!=），通过调用上面定义的相等运算符（==）并取反来实现不等判断逻辑。即如果两个对象通过 == 运算符判断为相等，则返回 false；如果 == 运算符返回 false（意味着有成员变量的值不同），则这里返回 true，表示两个对象不相等
    bool operator!=(const FloatColor &rhs) const  {
      return !(*this == rhs);
    }
        // 判断是否是从 UE4 中包含进来的这个头文件，如果是，则定义下面这些与 UE4 中颜色类型相互转换的函数和构造函数，用于在 carla 代码与 UE4 相关代码交互时方便地进行颜色数据的转换操作
#ifdef LIBCARLA_INCLUDED_FROM_UE4
            // 定义一个构造函数，接受一个 UE4 中的 FColor 类型的参数，用于将 UE4 的颜色类型转换为当前的 FloatColor 类型。它通过将 FColor 类型中每个通道的值（范围是 0 - 255）转换为浮点数（除以 255.f）来初始化当前 FloatColor 对象的对应通道值，实现从 UE4 颜色格式到自定义浮点数颜色格式的转换
    FloatColor(const FColor &color)
      : FloatColor(color.R / 255.f, color.G / 255.f, color.B / 255.f, color.A / 255.f) {}
            // 定义一个构造函数，接受一个 UE4 中的 FLinearColor 类型的参数，用于将 UE4 的线性颜色类型直接转换为当前的 FloatColor 类型，直接将 FLinearColor 类型中各通道的值赋值给当前 FloatColor 对象的对应通道，因为两者都是基于浮点数表示颜色通道的，只是可能在具体颜色空间等方面有差异，但这里简单进行了数值的传递转换
    FloatColor(const FLinearColor &color)
      : FloatColor(color.R, color.G, color.B, color.A) {}
            // 定义一个成员函数 ToFColor，用于将当前的 FloatColor 对象转换为 UE4 中的 FColor 类型。它通过将当前对象各颜色通道的浮点数乘以 255 并转换为 uint8 类型（使用 static_cast 进行类型转换），然后构造并返回一个 FColor 对象，实现从自定义浮点数颜色格式到 UE4 颜色格式的转换，方便在 UE4 环境中使用这个颜色数据

    FColor ToFColor() const {
      return FColor{
        static_cast<uint8>(r * 255),
        static_cast<uint8>(g * 255),
        static_cast<uint8>(b * 255),
        static_cast<uint8>(a * 255)};
    }
            // 定义一个类型转换运算符（operator），用于将当前的 FloatColor 对象隐式转换为 UE4 中的 FLinearColor 类型。当在代码中需要将 FloatColor 对象当作 FLinearColor 类型使用时（比如作为参数传递给接受 FLinearColor 类型的函数等情况），编译器会自动调用这个转换运算符进行转换，返回一个具有相同颜色通道值的 FLinearColor 对象，方便在不同颜色类型间的兼容使用
    operator FLinearColor() const {
      return FLinearColor{ r, g, b, a };
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(r, g, b, a);
  };
    // 使用 #pragma pack(pop) 指令恢复之前的内存对齐设置，结束之前通过 #pragma pack(push, 1) 设置的 1 字节对齐方式，避免影响后续其他代码中结构体的默认内存对齐行为（如果有的话）
#pragma pack(pop)

} // namespace rpc
} // namespace carla
