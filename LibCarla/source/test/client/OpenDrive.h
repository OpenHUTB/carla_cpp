// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 这是一个预处理指令，用于确保头文件只会被编译一次，避免因多次包含该头文件而产生重复定义等编译错误。

#include <string>
// 引入C++标准库中的<string>头文件，该头文件中定义了std::string类型，用于处理字符串相关操作，方便在后续代码中使用字符串来存储和操作文本信息。
#include <vector>
// 引入C++标准库中的<vector>头文件，<vector>头文件定义了std::vector模板类，它是一个动态大小的数组容器，
// 可以方便地存储和操作多个同类型的元素，在这里可能用于存储一组相关的数据，比如文件名列表等。

namespace util {
// 定义一个名为"util"的命名空间，通常用于将一组相关的工具类、函数等组织在一起，避免与其他代码中的同名标识符发生冲突，
// 使得代码结构更加清晰，这里面的代码都是属于这个工具相关的功能集合。

  /// 加载OpenDrive目录下文件的工具类.
  class OpenDrive {
// 定义一个名为OpenDrive的类，从注释可知它是用于加载OpenDrive目录下文件的工具类，后续会在类中定义相关的成员函数来实现具体的文件加载等功能。
  public:

    static std::vector<std::string> GetAvailableFiles();
// 声明一个名为GetAvailableFiles的静态成员函数，它的返回类型是std::vector<std::string>，也就是一个存储字符串的动态数组容器，
    // 函数功能大概率是获取OpenDrive目录下所有可用文件的文件名列表，由于是静态函数，不需要创建类的实例对象就可以调用该函数来获取文件列表信息。


    static std::string Load(const std::string &filename);
// 声明一个名为Load的静态成员函数，返回类型为std::string，接收一个const std::string &类型的参数filename，
    // 其功能应该是根据传入的文件名（filename）去加载对应的文件内容，并将文件内容以字符串的形式返回，同样作为静态函数可直接调用进行文件加载操作。
  };

} // namespace util
// 结束名为"util"的命名空间的定义，表明这个命名空间内的相关类、函数等定义到此为止，外部代码如果要使用这里面的内容，需要通过正确的命名空间限定来访问。
