// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderAnimWalker.h"
#include "CarlaRecorderHelpers.h"

// CarlaRecorderAnimWalker类的相关函数实现，这个类可能用于记录动画行走者（Anim Walker）相关的数据

// 函数用于将CarlaRecorderAnimWalker对象的数据写入到输出流（OutFile）中
// 按照一定的顺序依次写入相关数据成员的值
void CarlaRecorderAnimWalker::Write(std::ostream &OutFile)
{
    // 写入数据库ID，调用WriteValue函数（应该是自定义的用于写入特定类型数据的辅助函数）将this->DatabaseId（类型为uint32_t）写入输出流
    WriteValue<uint32_t>(OutFile, this->DatabaseId);
    // 写入速度值，同样调用WriteValue函数将this->Speed（类型为float）写入输出流
    WriteValue<float>(OutFile, this->Speed);
}

// 函数用于从输入流（InFile）中读取数据并赋值给CarlaRecorderAnimWalker对象的数据成员
void CarlaRecorderAnimWalker::Read(std::istream &InFile)
{
    // 读取数据库ID，调用ReadValue函数（与WriteValue对应，用于读取特定类型数据的辅助函数）从输入流读取数据并赋值给this->DatabaseId
    ReadValue<uint32_t>(InFile, this->DatabaseId);
    // 读取速度值，通过ReadValue函数从输入流读取数据赋值给this->Speed
    ReadValue<float>(InFile, this->Speed);
}

// ---------------------------------------------

// CarlaRecorderAnimWalkers类的相关函数实现，这个类可能用于管理多个CarlaRecorderAnimWalker对象，比如存储、读写一组动画行走者的数据

// 函数用于清空存储动画行走者数据的容器（Walkers），将其内部元素全部清除
void CarlaRecorderAnimWalkers::Clear(void)
{
    Walkers.clear();
}

// 函数用于向存储动画行走者数据的容器（Walkers）中添加一个CarlaRecorderAnimWalker对象
void CarlaRecorderAnimWalkers::Add(const CarlaRecorderAnimWalker &Walker)
{
    Walkers.push_back(Walker);
}

// 函数用于将一组动画行走者（CarlaRecorderAnimWalkers中存储的所有CarlaRecorderAnimWalker对象）的数据写入到输出流（OutFile）中
void CarlaRecorderAnimWalkers::Write(std::ostream &OutFile)
{
    // 写入数据包的ID，将CarlaRecorderPacketId::AnimWalker转换为char类型后写入输出流，
    // 这里CarlaRecorderPacketId::AnimWalker应该是一个用于标识数据包类型的枚举值之类的，通过WriteValue函数进行写入
    WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::AnimWalker));

    // 计算要写入的数据包的总大小，2可能是固定头部之类占用的字节数，
    // Walkers.size() * sizeof(CarlaRecorderAnimWalker)表示所有动画行走者对象数据占用的字节数，
    // 然后将总大小（类型为uint32_t）通过WriteValue函数写入输出流
    uint32_t Total = 2 + Walkers.size() * sizeof(CarlaRecorderAnimWalker);
    WriteValue<uint32_t>(OutFile, Total);

    // 写入总的记录数（也就是动画行走者的数量），先将Walkers容器中的元素数量赋值给Total，再将其转换为uint16_t类型通过WriteValue函数写入输出流
    Total = Walkers.size();
    WriteValue<uint16_t>(OutFile, Total);

    // 如果存在记录（即动画行走者数量大于0），则将所有动画行走者的数据直接写入输出流，
    // 通过OutFile.write函数，将Walkers容器中存储的所有CarlaRecorderAnimWalker对象数据按字节形式写入
    if (Total > 0)
    {
        OutFile.write(reinterpret_cast<const char *>(Walkers.data()),
                      Walkers.size() * sizeof(CarlaRecorderAnimWalker));
    }
}

// 函数用于从输入流（InFile）中读取一组动画行走者的数据，并添加到Walkers容器中
void CarlaRecorderAnimWalkers::Read(std::istream &InFile)
{
    uint16_t i, Total;
    CarlaRecorderAnimWalker Walker;

    // 先从输入流中读取总的动画行走者数量，通过ReadValue函数将读取的值赋给Total变量
    ReadValue<uint16_t>(InFile, Total);
    // 循环读取每个动画行走者的数据，根据Total数量进行循环
    for (i = 0; i < Total; ++i)
    {
        // 调用单个动画行走者对象（Walker）的Read函数从输入流读取数据并解析赋值给自己的数据成员
        Walker.Read(InFile);
        // 将读取并解析好数据的动画行走者对象添加到Walkers容器中，通过调用Add函数实现
        Add(Walker);
    }
}

// 函数用于获取存储动画行走者数据的容器（Walkers）的引用，外部可以通过这个函数获取到内部存储的所有动画行走者数据
const std::vector<CarlaRecorderAnimWalker>& CarlaRecorderAnimWalkers::GetWalkers()
{
    return Walkers;
}
