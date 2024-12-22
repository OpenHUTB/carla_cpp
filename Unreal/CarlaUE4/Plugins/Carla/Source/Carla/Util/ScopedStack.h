// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这是一个预编译指令，用于确保头文件在多次包含时只被编译一次，避免重复定义等问题
#pragma once

// 包含 <deque> 头文件，用于使用双端队列容器 std::deque，它可以高效地在两端进行元素的插入和删除操作
// 包含 <memory> 头文件，用于使用智能指针相关的功能，比如 std::unique_ptr，方便进行内存管理等操作
#include <deque>
#include <memory>

// 定义一个名为 FScopedStack 的类模板，用于实现一个类似栈的结构，用来跟踪嵌套作用域
// 它私有继承自 std::deque<T>，意味着它可以复用 std::deque 的很多功能，但对外隐藏了这些实现细节
// 模板参数 T 表示栈中存储元素的类型，具体类型在实例化这个模板类时指定
template <typename T>
class FScopedStack : private std::deque<T> {
    // 使用类型别名（typedef 的新语法形式）定义 Super 为 std::deque<T>，方便后续代码中引用父类类型
    using Super = std::deque<T>;
public:

    // 定义一个名为 PushScope 的成员函数模板，用于将一个值压入到这个类似栈的结构中
    // 并且返回一个智能指针，当这个智能指针超出其作用域时，会自动弹出刚刚压入的值，实现了一种基于作用域的自动资源管理机制
    // 函数模板参数 V 是一个通用的类型参数，用于接收各种类型的实参（通过转发引用的形式 std::forward<V>(Value) 来完美转发传入的值）
    template <typename V>
    auto PushScope(V &&Value)
    {
        // 使用父类（即 std::deque<T>）的 emplace_back 函数，将传入的参数值以完美转发的方式构造并插入到双端队列的末尾，相当于压入栈顶
        Super::emplace_back(std::forward<V>(Value));
        // 获取刚刚压入元素（也就是栈顶元素）的指针，后续通过这个指针来操作该元素（比如在智能指针析构时找到对应的元素进行弹出操作）
        T *Pointer = &Super::back();
        // 创建一个匿名的 lambda 表达式作为自定义删除器（deleter），它接收一个指向 T 类型的常量指针（这里虽然参数未实际使用，但符合 std::unique_ptr 的删除器接口要求）
        // 当这个 std::unique_ptr 被销毁时（比如超出作用域），就会调用这个 lambda 表达式，在表达式内部通过调用父类的 pop_back 函数来弹出栈顶元素，实现自动清理
        auto Deleter = [this](const T *) { Super::pop_back(); };
        // 创建并返回一个 std::unique_ptr 类型的智能指针，它指向刚刚压入的元素（通过 Pointer 指定），并关联了自定义的删除器 Deleter
        // 这样当这个智能指针生命周期结束时，就能自动执行删除器中的逻辑，也就是弹出对应的元素
        return std::unique_ptr<T, decltype(Deleter)>(Pointer, Deleter);
    }

    // 将父类（std::deque<T>）的 empty 函数引入到当前类的公有接口中，方便外部代码可以直接调用判断这个栈是否为空
    using Super::empty;
    // 将父类（std::deque<T>）的 size 函数引入到当前类的公有接口中，方便外部代码可以直接调用获取这个栈的元素个数
    using Super::size;
    // 将父类（std::deque<T>）的 begin 函数引入到当前类的公有接口中，用于获取指向栈中第一个元素的迭代器，方便外部代码进行遍历等操作
    using Super::begin;
    // 将父类（std::deque<T>）的 end 函数引入到当前类的公有接口中，用于获取指向栈中最后一个元素之后位置的迭代器，与 begin 配合实现遍历等操作
    using Super::end;
};
