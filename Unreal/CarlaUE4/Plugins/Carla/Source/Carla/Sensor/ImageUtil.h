// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 版权声明部分，说明这段代码的版权所有者是位于巴塞罗那自治大学（Universitat Autonoma de Barcelona）的计算机视觉中心（Computer Vision Center），
// 并且此代码遵循 MIT 许可证进行开源授权，若想要查看该许可证的详细内容，可以通过给定的网址进行访问。
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 预处理指令，用于确保该头文件在同一个编译单元中只会被包含一次，避免出现重复定义等编译问题，这是 C/C++ 代码组织中常用的一种方式，以保证代码的正确编译。
#pragma once

// 包含 "CoreMinimal.h" 头文件，在虚幻引擎项目中，这是一个基础的头文件，它包含了许多项目所需的最基本的类型定义、宏、函数等内容，比如一些常用的基础数据类型、基础的内存管理相关宏等，为整个代码库提供基础的支持。
#include "CoreMinimal.h"

// 前置声明一个类 FRHIGPUTextureReadback，这只是告诉编译器存在这样一个类，使得在当前代码中可以使用这个类的指针或者引用类型，而不需要完整地包含该类的定义所在的头文件（通常在只涉及到类指针或引用的使用场景下，这样做可以加快编译速度、避免不必要的头文件包含带来的编译复杂性增加等问题）。
class FRHIGPUTextureReadback;

// 定义一个名为 ImageUtil 的命名空间，通常用于将相关功能的函数、类型等组织在一起，避免全局命名冲突，使得代码结构更加清晰，便于代码的维护和管理。
namespace ImageUtil
{
    // 函数 DecodePixelsByFormat 的第一个重载版本声明，它的功能主要是根据指定的像素格式（Format）等参数，对输入的像素数据（PixelData）进行解码操作，并将解码后的结果输出到 Out 参数所指定的 FColor 类型的数组视图（TArrayView）中。
    // 参数说明：
    // - PixelData：指向像素数据的指针，它是需要进行解码处理的原始像素数据的起始地址。
    // - SourcePitch：表示源像素数据的间距，也就是每行像素数据所占用的字节数，这个参数对于正确解析像素数据的布局很重要。
    // - SourceExtent：是一个 FIntPoint 类型的参数，用于表示源像素数据在二维平面上的范围（例如宽度和高度）。
    // - DestinationExtent：同样是 FIntPoint 类型，用于表示解码后像素数据期望的目标范围，可能涉及到缩放等操作时需要指定目标尺寸。
    // - Format：指定了像素数据的格式，例如 RGBA8 等，不同的格式决定了如何对输入的像素数据进行解析和解码。
    // - Flags：是 FReadSurfaceDataFlags 类型，可能包含了一些读取表面数据时的标志位，用于控制读取和解码的具体行为，比如是否进行特定的转换等。
    // - Out：是一个 TArrayView<FColor> 类型，作为输出参数，用于接收解码后的像素数据，以 FColor 类型的数组形式呈现，FColor 常用于表示颜色信息（例如包含红、绿、蓝、透明度等分量）。
    void DecodePixelsByFormat(
        void* PixelData,
        int32 SourcePitch,
        FIntPoint SourceExtent,
        FIntPoint DestinationExtent,
        EPixelFormat Format,
        FReadSurfaceDataFlags Flags,
        TArrayView<FColor> Out);

    // 函数 DecodePixelsByFormat 的第二个重载版本声明，和上面的函数功能类似，也是对输入像素数据进行解码操作，但不同的是，它将解码后的结果输出到 Out 参数所指定的 FLinearColor 类型的数组视图（TArrayView）中。
    // FLinearColor 和 FColor 的区别在于颜色表示方式，FLinearColor 通常采用线性空间表示颜色，更适合在一些涉及颜色计算、光照计算等需要精确颜色处理的场景中使用，而 FColor 一般是在常规的 sRGB 等非线性空间表示颜色。
    // 此函数各参数的含义与上面的重载版本一致，只是输出结果的类型不同，用于满足不同的使用场景对解码后像素颜色数据表示形式的需求。
    void DecodePixelsByFormat(
        void* PixelData,
        int32 SourcePitch,
        FIntPoint SourceExtent,
        FIntPoint DestinationExtent,
        EPixelFormat Format,
        FReadSurfaceDataFlags Flags,
        TArrayView<FLinearColor> Out);
}
