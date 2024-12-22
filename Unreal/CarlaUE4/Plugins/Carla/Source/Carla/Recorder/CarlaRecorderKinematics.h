// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 头文件保护宏，确保该头文件在被多次包含时只会被编译一次，避免重复定义等问题
#pragma once  

// 引入输入/输出流相关头文件，方便后续进行文件读写等操作，比如从文件读取数据或者向文件写入数据
#include <sstream>  
// 引入向量容器头文件，用于存储一组同类型的元素，在这里可用于存储相关的结构体对象
#include <vector>  

// 使用#pragma pack指令来设置结构体的内存对齐方式，这里将当前的对齐设置压入栈中，并设置对齐值为1字节，
// 目的是为了让结构体成员按照紧凑的方式在内存中存储，节省空间或者满足特定的内存布局需求（常用于和底层数据交互等情况）
#pragma pack(push, 1)  

// 定义CarlaRecorderKinematics结构体，该结构体可能用于记录与运动学相关的数据信息，
// 推测是在Carla项目中用于记录某个对象（比如车辆、角色等）的运动学状态。
struct CarlaRecorderKinematics
{
    // 用于在数据库中唯一标识该运动学数据对应的对象的ID，类型为无符号32位整数
    uint32_t DatabaseId;  
    // 线性速度向量，用于表示对象在三维空间中的直线运动速度，具体类型FVector可能是项目自定义的向量类型，
    // 它包含了X、Y、Z三个方向上的速度分量信息（取决于FVector的定义）
    FVector LinearVelocity;  
    // 角速度向量，用于表示对象绕某个轴旋转的速度，同样是三维向量，其各个分量对应不同轴向的旋转速度情况，
    // 反映了对象的旋转运动状态（具体也依赖于FVector类型的定义）
    FVector AngularVelocity;  

    // 从输入流（例如从文件读取数据的流）中读取该结构体成员变量的值，
    // 具体的读取格式和顺序要根据流中数据的组织形式来确定，可能是按照特定顺序依次读取各个成员的数据
    void Read(std::istream &InFile);  
    // 将该结构体成员变量的值写入到输出流（比如向文件写入数据的流）中，
    // 同样要遵循一定的格式约定，确保写入的数据能被正确解析和使用（和Read函数对应的数据格式一致）
    void Write(std::ostream &OutFile);  
};
// 恢复之前压入栈的内存对齐设置，使得后续的结构体等内存对齐方式回到之前的状态（如果有）
#pragma pack(pop)  

// 定义CarlaRecorderActorsKinematics类，它可能是用于管理一组CarlaRecorderKinematics结构体对象的集合，
// 例如对多个对象的运动学数据进行添加、清除、整体写入等操作，起到组织和操作这些运动学数据的作用。
class CarlaRecorderActorsKinematics
{
public:
    // 向内部存储的运动学数据集合（Kinematics）中添加一个CarlaRecorderKinematics类型的对象，
    // 可用于不断积累需要记录的不同对象的运动学数据信息。
    void Add(const CarlaRecorderKinematics &InObj);  
    // 清空内部存储的所有运动学数据对象，即将Kinematics向量容器中的所有元素移除，
    // 常用于重置状态或者重新开始记录等应用场景。
    void Clear(void);  
    // 将内部存储的所有运动学数据对象依次写入到输出流（比如文件流）中，
    // 会调用每个CarlaRecorderKinematics对象的Write函数来完成具体的写入操作，确保数据能正确保存。
    void Write(std::ostream &OutFile);  

private:
    // 使用vector容器来存储CarlaRecorderKinematics类型的结构体对象，
    // 这个容器就是用来管理所有相关的运动学数据信息的，类中的各个成员函数会对其进行相应的操作，
    // 比如添加、清除等操作都是围绕这个容器进行的。
    std::vector<CarlaRecorderKinematics> Kinematics;  
};
