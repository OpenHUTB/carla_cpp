// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 预处理指令，确保该头文件在整个编译过程中只会被包含一次，避免重复包含引发的编译错误


template<typename IT>
// 定义一个模板，模板参数为 IT，它可以是任意类型，后续类的定义会基于这个类型参数进行泛型编程
class ListView
{
public:
// 类的公有部分，外部可访问

  using iterator = IT;
// 使用 using 关键字为类型 IT 创建一个别名 iterator，方便后续代码中使用更具语义的名称来指代这个类型，使代码可读性更好

  explicit ListView(iterator begin, iterator end) : Begin(begin), End(end) {}
 // 定义一个显式的构造函数，接受两个类型为 iterator（也就是 IT 类型）的参数，分别表示范围的起始迭代器和结束迭代器，
    // 并使用成员初始化列表来初始化类中的私有成员变量 Begin 和 End

  template <typename STL_CONTAINER>
 // 定义一个模板函数，模板参数为 STL_CONTAINER，它可以是任意符合要求的标准模板库（STL）容器类型
  explicit ListView(STL_CONTAINER &StlContainer) :
    Begin(iterator(StlContainer.begin())),
    End(iterator(StlContainer.end())) {}
 // 另一个显式构造函数，它接受一个 STL_CONTAINER 类型的引用作为参数，通过调用该容器的 begin() 和 end() 成员函数来获取其迭代器范围，
    // 并将获取到的起始迭代器和结束迭代器分别转换为 IT 类型（通过 iterator 别名对应的类型进行转换）后，使用成员初始化列表初始化类中的私有成员变量 Begin 和 End

  ListView(const ListView &) = default;
// 使用 = default 显式指示编译器自动生成默认的拷贝构造函数，即按成员逐一进行拷贝初始化，行为和编译器默认生成的一致
  ListView &operator=(const ListView &) = delete;
// 使用 = delete 显式指示编译器禁止生成默认的赋值运算符重载函数，意味着该类的对象不允许进行赋值操作


  iterator begin() const {
    return Begin;
  }
// 定义成员函数 begin()，它返回表示范围起始位置的迭代器，const 修饰表示该函数不会修改类的成员变量，符合 STL 中容器相关的迭代器获取规范

  iterator end() const {
    return End;
  }
 // 定义成员函数 end()，它返回表示范围结束位置的迭代器（注意，这个迭代器指向的是范围末尾的下一个位置，符合 STL 迭代器区间的习惯表示），const 修饰表示不会修改成员变量

  bool empty() const {
    return Begin == End;
  }
// 定义成员函数 empty()，用于判断当前表示的范围是否为空，通过比较起始迭代器和结束迭代器是否相等来判断，如果相等则表示范围为空，返回 true，否则返回 false，const 修饰表示不会修改成员变量

private:
 // 类的私有部分，外部不可访问

  const iterator Begin;
// 声明一个私有常量成员变量 Begin，类型为 iterator（也就是 IT 类型），用于记录所表示范围的起始迭代器

  const iterator End;
// 声明一个私有常量成员变量 End，类型为 iterator（也就是 IT 类型），用于记录所表示范围的结束迭代器
};
