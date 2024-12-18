// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// Boost Generic Image Library (GIL): https://boostorg.github.io/gil/doc/html/tutorial.html
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
//使用 #pragma once 预处理指令，确保该头文件在一个编译单元中只会被包含一次

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wc++11-narrowing"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif

#include <boost/gil.hpp>

#if defined(__clang__)
#  pragma clang diagnostic pop
#endif
