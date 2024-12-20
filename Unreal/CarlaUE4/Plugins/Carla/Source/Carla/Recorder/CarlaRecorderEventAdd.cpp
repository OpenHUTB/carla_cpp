// 版权所有 （c） 2017 巴塞罗那自治大学 （UAB） 计算机视觉中心 （CVC）。
//
// 本作品根据 MIT 许可证的条款进行许可。
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#include "CarlaRecorder.h"
#include "CarlaRecorderEventAdd.h"
#include "CarlaRecorderHelpers.h"

// CarlaRecorderEventAdd类的Write函数，用于将该类相关的数据写入到输出流（OutFile）中
// 通常用于将对象的状态信息持久化存储等操作，比如记录到文件中
void CarlaRecorderEventAdd::Write(std::ostream &OutFile) const
{
    // 将数据库的唯一标识符（DatabaseId）写入输出流，WriteValue应该是一个自定义的用于按特定格式写入数据的函数，这里写入的是32位无符号整数类型的数据
    WriteValue<uint32_t>(OutFile, this->DatabaseId);
    // 将类型（Type）信息写入输出流，此处写入的是8位无符号整数类型的数据
    WriteValue<uint8_t>(OutFile, this->Type);

    // 写入位置信息（Location）到输出流，WriteFVector应该是用于写入向量类型数据的自定义函数，用于记录对象在空间中的位置
    WriteFVector(OutFile, this->Location);
    // 写入旋转信息（Rotation）到输出流，同样使用WriteFVector函数，用于记录对象的旋转状态
    WriteFVector(OutFile, this->Rotation);

    // 写入描述类型相关的唯一标识符（UId）到输出流，它是32位无符号整数类型
    WriteValue<uint32_t>(OutFile, this->Description.UId);
    // 写入描述类型的具体标识（Id）到输出流，WriteFString应该是用于写入字符串类型数据的自定义函数
    WriteFString(OutFile, this->Description.Id);

    // 开始处理属性（Attributes）相关的数据写入
    // 获取属性的总数量
    uint16_t Total = this->Description.Attributes.size();
    // 将属性的总数量写入输出流，同样使用WriteValue函数，这里写入的是16位无符号整数类型的数据
    WriteValue<uint16_t>(OutFile, Total);
    // 遍历所有的属性
    for (uint16_t i = 0; i < Total; ++i)
    {
        // 写入属性的类型（Type）信息到输出流，8位无符号整数类型
        WriteValue<uint8_t>(OutFile, this->Description.Attributes[i].Type);
        // 写入属性的标识（Id）字符串到输出流
        WriteFString(OutFile, this->Description.Attributes[i].Id);
        // 写入属性的值（Value）字符串到输出流
        WriteFString(OutFile, this->Description.Attributes[i].Value);
    }
}

// CarlaRecorderEventAdd类的Read函数，用于从输入流（InFile）中读取数据并恢复该类对象的状态
// 通常用于从文件等存储介质中读取之前保存的对象信息来重建对象
void CarlaRecorderEventAdd::Read(std::istream &InFile)
{
    // 从输入流中读取数据库的唯一标识符（DatabaseId）并赋值给当前对象的对应成员变量，ReadValue是自定义的按特定格式读取数据的函数
    ReadValue<uint32_t>(InFile, this->DatabaseId);

    // 从输入流中读取数据库的类型（Type）信息并赋值给当前对象的对应成员变量
    ReadValue<uint8_t>(InFile, this->Type);

    // 从输入流中读取位置信息（Location）并赋值给当前对象的对应成员变量，ReadFVector是用于读取向量类型数据的自定义函数
    ReadFVector(InFile, this->Location);
    // 从输入流中读取旋转信息（Rotation）并赋值给当前对象的对应成员变量
    ReadFVector(InFile, this->Rotation);

    // 从输入流中读取描述类型相关的唯一标识符（UId）并赋值给当前对象的对应成员变量
    ReadValue<uint32_t>(InFile, this->Description.UId);
    // 从输入流中读取描述类型的具体标识（Id）并赋值给当前对象的对应成员变量，ReadFString用于读取字符串类型数据
    ReadFString(InFile, this->Description.Id);

    // 开始处理属性（Attributes）相关的数据读取
    uint16_t Total;
    // 从输入流中读取属性的总数量
    ReadValue<uint16_t>(InFile, Total);
    // 先清空当前对象的属性容器，准备重新填充数据
    this->Description.Attributes.clear();
    // 预留足够的空间来存储即将读取的属性数据，避免后续频繁的内存分配操作，提高效率
    this->Description.Attributes.reserve(Total);
    // 遍历所有要读取的属性
    for (uint16_t i = 0; i < Total; ++i)
    {
        // 创建一个临时的属性对象（Att），用于存储从输入流中读取的单个属性信息
        CarlaRecorderActorAttribute Att;
        // 从输入流中读取属性的类型（Type）信息并赋值给临时属性对象的对应成员变量
        ReadValue<uint8_t>(InFile, Att.Type);
        // 从输入流中读取属性的标识（Id）字符串并赋值给临时属性对象的对应成员变量
        ReadFString(InFile, Att.Id);
        // 从输入流中读取属性的值（Value）字符串并赋值给临时属性对象的对应成员变量
        ReadFString(InFile, Att.Value);
        // 将填充好的临时属性对象添加到当前对象的属性容器中
        this->Description.Attributes.push_back(std::move(Att));
    }
}

//---------------------------------------------

// CarlaRecorderEventsAdd类的Clear函数，用于清空存储的事件列表（Events）
// 一般用于重置对象的状态，释放相关内存等操作
void CarlaRecorderEventsAdd::Clear(void)
{
    Events.clear();
}

// CarlaRecorderEventsAdd类的Add函数，用于向事件列表（Events）中添加一个CarlaRecorderEventAdd类型的事件
// 通过移动语义（std::move）来高效地传递参数，避免不必要的拷贝操作
void CarlaRecorderEventsAdd::Add(const CarlaRecorderEventAdd &Event)
{
    Events.push_back(std::move(Event));
}

// CarlaRecorderEventsAdd类的Write函数，用于将该类所管理的所有事件信息写入到输出流（OutFile）中
// 可能用于将一系列事件数据保存到文件等操作，包含了一些计算和写入包大小等逻辑
void CarlaRecorderEventsAdd::Write(std::ostream &OutFile)
{
    // 写入数据包的标识符（CarlaRecorderPacketId::EventAdd）到输出流，这里将其转换为字符类型（char）后写入，用于标识数据包的类型
    WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::EventAdd));

    // 获取当前输出流的写入位置，用于后续计算数据包大小等操作，PosStart记录了开始写入数据包主体内容前的位置
    std::streampos PosStart = OutFile.tellp();

    // 先写入一个虚拟的数据包大小（初始设为0），后续会重新计算并更新这个值，这里先占位，方便后续定位和修改
    uint32_t Total = 0;
    WriteValue<uint32_t>(OutFile, Total);

    // 写入事件的总数量到输出流，这里的Total获取了存储在Events容器中的事件个数，并转换为16位无符号整数类型写入
    Total = Events.size();
    WriteValue<uint16_t>(OutFile, Total);

    // 遍历所有的事件，调用每个事件对象自身的Write函数将其详细信息写入输出流
    for (uint16_t i = 0; i < Total; ++i)
        Events[i].Write(OutFile);

    // 获取当前输出流的写入位置，PosEnd记录了写完所有事件数据后的位置，通过它与PosStart的差值来计算实际的数据包大小（减去之前写入的虚拟大小占位的4个字节，即sizeof(uint32_t)）
    std::streampos PosEnd = OutFile.tellp();
    Total = PosEnd - PosStart - sizeof(uint32_t);
    // 将输出流的写入位置移动回之前记录的PosStart位置，以便更新之前写入的虚拟数据包大小的值
    OutFile.seekp(PosStart, std::ios::beg);
    // 写入实际计算得到的数据包大小
    WriteValue<uint32_t>(OutFile, Total);
    // 再将输出流的写入位置移动回PosEnd，以便后续可以继续写入其他可能的数据（如果有的话）
    OutFile.seekp(PosEnd, std::ios::beg);
}

// CarlaRecorderEventsAdd类的Read函数，用于从输入流（InFile）中读取事件数据并添加到当前对象管理的事件列表（Events）中
// 通过循环读取每个事件的数据，调用相应的Read函数来恢复事件对象，并添加到列表中
void CarlaRecorderEventsAdd::Read(std::istream &InFile)
{
    CarlaRecorderEventAdd EventAdd;
    uint16_t i, Total;
    // 从输入流中读取事件的总数量
    ReadValue<uint16_t>(InFile, Total);
    // 遍历所有要读取的事件
    for (i = 0; i < Total; ++i)
    {
        // 调用单个事件对象（EventAdd）的Read函数从输入流中读取该事件的数据并恢复其状态
        EventAdd.Read(InFile);
        // 将恢复好的事件对象添加到当前对象管理的事件列表（Events）中
        Add(EventAdd);
    }
}

// CarlaRecorderEventsAdd类的GetEvents函数，用于获取当前对象所管理的事件列表（Events）的引用
// 外部代码可以通过这个函数来访问和操作这些事件数据，但不能通过返回的引用去改变存储事件的容器本身（比如重新赋值整个容器等操作是不行的）
const std::vector<CarlaRecorderEventAdd>& CarlaRecorderEventsAdd::GetEvents()
{
    return Events;
}
