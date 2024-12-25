// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这个头文件保护宏，用于防止头文件被重复包含。
// 当一个头文件在多个源文件中被包含时，通过这个宏可以确保其内容只被编译一次。
#pragma once 

// 前置声明UWorld类，告知编译器后续会用到这个类，但具体定义在别处（一般在其他头文件或者链接阶段能找到其完整定义）
class UWorld; 

// 这里声明了carla和rpc命名空间下的DebugShape类，同样是前置声明，具体定义在对应命名空间的相关代码中
namespace carla { namespace rpc { class DebugShape; }} 

// FDebugShapeDrawer类，从名字推测它可能用于绘制调试形状相关的功能
class FDebugShapeDrawer
{
public:
    // 显式的构造函数，接受一个UWorld类的引用作为参数，用于初始化这个类的成员变量World。
    // 这个构造函数的作用是将传入的游戏世界（UWorld）对象与当前的调试形状绘制器（FDebugShapeDrawer）关联起来，
    // 以便后续在该世界中进行形状绘制等操作。
    explicit FDebugShapeDrawer(UWorld &InWorld) : World(InWorld) {}

    // 用于绘制指定的调试形状（DebugShape）的函数，具体的绘制逻辑应该在这个函数的实现中定义，
    // 它接收一个carla::rpc::DebugShape类型的常量引用参数，表示要绘制的形状对象。
    void Draw(const carla::rpc::DebugShape &Shape);

private:
    // 对UWorld类的引用，用于保存当前与之关联的游戏世界对象，在绘制形状等操作时可能会用到这个世界对象的相关属性和方法。
    UWorld &World;
};
