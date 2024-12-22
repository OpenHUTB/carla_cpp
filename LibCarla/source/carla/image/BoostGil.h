// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// Boost Generic Image Library (GIL): https://boostorg.github.io/gil/doc/html/tutorial.html
// 
// 对Boost通用图像库（GIL）的相关说明，并给出了其文档教程的网页链接，方便查看详细的使用文档等信息
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
#pragma once
// 使用 #pragma once 预处理指令，确保该头文件在一个编译单元中只会被包含一次，避免头文件被重复包含造成编译错误等问题

#if defined(__clang__)
// 如果定义了 __clang__ ，一般来说 __clang__ 是用于标识当前使用的编译器是Clang编译器
#  pragma clang diagnostic push
// 将当前的编译诊断状态压入栈中保存起来，以便后续可以恢复，这样就可以在局部范围内修改诊断设置而不影响整体的其他部分
#  pragma clang diagnostic ignored "-Wc++11-narrowing"
// 指示Clang编译器忽略“-Wc++11-narrowing”这个警告，这个警告通常和C++11中可能出现的窄化转换
#  pragma clang diagnostic ignored "-Wunused-parameter"
// 指示Clang编译器忽略“-Wunused-parameter”这个警告，该警告是在函数参数定义了但未被使用时触发
#  pragma clang diagnostic ignored "-Wunused-local-typedef"
// 指示Clang编译器忽略“-Wunused-local-typedef”这个警告，该警告通常在定义了局部的类型别名但未被使用时触发
#endif

#include <boost/gil.hpp>
// 包含Boost通用图像库（GIL）的头文件，这样就可以在当前代码文件中使用该库提供的各种功能、类型、函数等来处理图像相关的操作
#if defined(__clang__)
#  pragma clang diagnostic pop
/ 恢复之前压入栈中的编译诊断状态，使得编译器的诊断设置回到之前保存的状态，即撤销之前在局部所做的忽略某些警告的设置
#endif
