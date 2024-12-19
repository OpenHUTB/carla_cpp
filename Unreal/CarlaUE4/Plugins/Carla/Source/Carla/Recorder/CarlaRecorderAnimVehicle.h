// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这是一个头文件保护指令，防止头文件被重复包含。如果该头文件已经被包含过，
// 则编译器会跳过该头文件后续的内容，避免重复定义等编译错误。
#pragma once 

#include <sstream>
#include <vector>

// 这是一个编译指令，用于设置结构体或类的内存对齐方式。
// push表示将当前的对齐方式压入栈中保存，1表示按照1字节对齐。
// 这样做可以精确控制结构体的内存布局，在一些涉及到数据存储和读取的场景中很有用，
// 比如在与特定格式的数据文件交互时，确保数据按预期的字节顺序和大小排列。
#pragma pack(push, 1) 

// 定义一个名为CarlaRecorderAnimVehicle的结构体，用于记录车辆相关的动画信息（可能是用于回放等功能）。
struct CarlaRecorderAnimVehicle
{
    // 车辆在某个数据库中的唯一标识符，通常用于区分不同的车辆对象。
    uint32_t DatabaseId; 
    // 车辆的转向值，范围可能根据具体的模拟环境设定，一般用来表示方向盘转动角度等相关信息。
    float Steering; 
    // 油门踏板的开度，取值范围通常在0到1之间，表示油门踩踏的程度，控制车辆的加速。
    float Throttle; 
    // 刹车踏板的开度，取值范围通常在0到1之间，表示刹车踩踏的程度，控制车辆的减速。
    float Brake; 
    // 表示手刹是否拉起的布尔值，true表示手刹拉起，false表示手刹未拉起。
    bool bHandbrake; 
    // 车辆当前所处的挡位，是一个有符号的32位整数，不同的值对应不同的挡位（如 -1可能表示倒车挡等，具体依实际定义）。
    int32_t Gear; 

    // 成员函数，用于从输入流（比如文件输入流）中读取车辆的动画信息并填充到该结构体的成员变量中。
    void Read(std::istream &InFile); 

    // 成员函数，用于将该结构体中记录的车辆动画信息写入到输出流（比如文件输出流）中。
    void Write(std::ostream &OutFile); 
};
// 恢复之前保存的内存对齐方式，与上面的#pragma pack(push, 1) 对应，确保后续代码的内存对齐不受影响。
#pragma pack(pop) 

// 定义一个名为CarlaRecorderAnimVehicles的类，用于管理多个CarlaRecorderAnimVehicle结构体，
// 可以看作是对一组车辆动画信息的集合进行操作的类。
class CarlaRecorderAnimVehicles
{
public:
    // 成员函数，用于向该类管理的车辆动画信息集合中添加一个车辆的动画信息对象。
    void Add(const CarlaRecorderAnimVehicle &InObj); 

    // 成员函数，用于清空该类管理的所有车辆动画信息，即将内部存储车辆动画信息的容器清空。
    void Clear(void); 

    // 成员函数，用于将该类中管理的所有车辆动画信息写入到输出流（例如文件输出流）中，
    // 可能是用于保存这些信息到文件以便后续回放等操作。
    void Write(std::ostream &OutFile); 

    // 成员函数，用于从输入流（例如文件输入流）中读取一组车辆动画信息并填充到内部管理的容器中，
    // 实现从外部数据源加载车辆动画数据的功能。
    void Read(std::istream &InFile); 

    // 成员函数，用于获取该类内部管理的所有车辆动画信息的只读引用，
    // 返回的是一个存储CarlaRecorderAnimVehicle结构体的向量容器的引用，
    // 外部可以通过这个引用来访问但不能修改内部的车辆动画信息集合。
    const std::vector<CarlaRecorderAnimVehicle>& GetVehicles();

private:
    // 一个私有成员变量，使用std::vector容器来存储多个CarlaRecorderAnimVehicle结构体，
    // 也就是实际保存所有车辆动画信息的地方。
    std::vector<CarlaRecorderAnimVehicle> Vehicles; 
};
