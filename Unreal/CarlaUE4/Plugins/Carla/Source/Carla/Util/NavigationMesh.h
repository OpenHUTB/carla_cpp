// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 预处理指令，作用是确保该头文件在整个编译过程中只会被包含一次，避免因重复包含此头文件而出现编译错误，这是C++ 中防止头文件重复包含的常用手段。

class FNavigationMesh
{
public:
 // 类的公有部分，这里声明的成员（函数、变量等）可以被类外部的代码访问。

  /// Return the Navigation Mesh Binary associated to @a MapName, or empty if the such
  /// file wasn't serialized.
// 这是一段文档注释（通常遵循 Doxygen 等代码文档生成工具的规范），用于简要说明下面所声明的静态函数的功能。
    // 它表示该函数会返回与给定的地图名称（@a MapName 参数所指代的）相关联的导航网格二进制数据，如果对应的文件没有被序列化，那么就返回空的结果。

  static TArray<uint8> Load(FString MapName);
// 声明一个静态函数 Load，它接受一个 FString 类型的参数 MapName（用于指定地图名称），
    // 函数的返回值类型是 TArray<uint8>，即一个存储无符号 8 位整数（字节）的数组，这个函数的功能应该是根据传入的地图名称去加载对应的导航网格二进制数据，若加载失败等情况则返回空数组。
};
