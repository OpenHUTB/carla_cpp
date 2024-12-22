// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这个头文件保护宏，用于防止头文件被重复包含
#pragma once  

// 包含输入/输出流相关的头文件，用于后续的文件读写操作等
#include <sstream>  
// 包含向量容器的头文件，用于存储一组相关的数据元素
#include <vector>  

// 定义CarlaRecorderEventParent结构体，它可能用于记录某个事件相关的信息，
// 这些信息可能与Carla记录器相关（推测，具体取决于整个项目上下文）
struct CarlaRecorderEventParent
{
    // 用于在数据库中唯一标识该事件的ID，类型为无符号32位整数
    uint32_t DatabaseId;  
    // 可能用于标识该事件父级相关的ID（同样具体含义取决于项目业务逻辑），类型为无符号32位整数
    uint32_t DatabaseIdParent;  

    // 从输入流（比如文件流等）中读取该结构体相关的数据成员的值，
    // 具体的读取格式和内容需要根据实际的流中数据格式来确定
    void Read(std::istream &InFile);  
    // 将该结构体相关的数据成员的值写入到输出流（例如文件流等）中，
    // 写入的格式也需要按照项目约定的格式进行
    void Write(std::ostream &OutFile) const;  
};

// 定义CarlaRecorderEventsParent类，它可能是用于管理一组CarlaRecorderEventParent结构体对象的集合，
// 比如对这些事件相关信息进行添加、清除、读写等操作
class CarlaRecorderEventsParent
{
public:
    // 向内部存储的事件集合（Events）中添加一个CarlaRecorderEventParent类型的事件对象，
    // 这样可以不断积累需要管理的事件信息
    void Add(const CarlaRecorderEventParent &Event);  
    // 清空内部存储的所有事件信息，即将事件集合（Events）中的元素全部移除，
    // 可用于重置状态或者重新开始记录等场景
    void Clear(void);  
    // 将内部存储的所有事件信息写入到输出流（例如文件流）中，
    // 会依次调用每个事件对象的Write函数来完成具体的写入操作
    void Write(std::ostream &OutFile);  
    // 从输入流（例如文件流）中读取事件信息，并填充到内部的事件集合（Events）中，
    // 会根据流中的数据格式依次解析并创建对应的事件对象
    void Read(std::istream &InFile);  
    // 获取内部存储的所有事件对象组成的常引用向量，
    // 通过这个函数外部代码可以获取到当前管理的所有事件信息，但不能修改这些事件对象本身（因为是常引用）
    const std::vector<CarlaRecorderEventParent>& GetEvents();  

private:
    // 使用vector容器来存储CarlaRecorderEventParent类型的事件对象，
    // 这个容器用于管理所有相关的事件信息，在类的各个成员函数中会对其进行操作
    std::vector<CarlaRecorderEventParent> Events;  
};
