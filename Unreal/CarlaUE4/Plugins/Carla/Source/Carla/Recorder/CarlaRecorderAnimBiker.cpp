// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderAnimBiker.h"
#include "CarlaRecorderHelpers.h"

// CarlaRecorderAnimBiker类的Write函数，用于将该类对象的相关数据成员写入到给定的输出流OutFile中，以便进行数据持久化、传输等操作，此函数被声明为const，意味着不会修改类的非mutable成员变量。
void CarlaRecorderAnimBiker::Write(std::ostream &OutFile) const
{
    // 调用WriteValue函数（推测是自定义的用于向输出流写入特定类型数据的辅助函数），将当前对象的DatabaseId（类型为uint32_t，可能是数据库中对应记录的唯一标识之类的用途）写入到输出流OutFile中。
    // 这样，在后续从相应的输入流读取数据时，可以按照相同的顺序和格式恢复这个值，确保数据的一致性和完整性。
    WriteValue<uint32_t>(OutFile, DatabaseId);
    // 同样调用WriteValue函数，把ForwardSpeed（类型为float，大概率表示该对象向前行进的速度值）这个成员变量的值写入到输出流OutFile中，方便后续对速度相关数据的记录和使用。
    WriteValue<float>(OutFile, ForwardSpeed);
    // 再次调用WriteValue函数，将EngineRotation（类型为float，可能代表引擎的旋转相关参数，比如转速之类的）成员变量的值写入到输出流OutFile中，完成该类对象主要数据成员向输出流的写入操作。
    WriteValue<float>(OutFile, EngineRotation);
}

// CarlaRecorderAnimBiker类的Read函数，作用是从给定的输入流InFile中读取数据，并将读取到的值赋给类的相应成员变量，实现从外部数据源恢复对象状态的功能。
void CarlaRecorderAnimBiker::Read(std::istream &InFile)
{
    // 调用ReadValue函数（与WriteValue相对应，应是自定义的用于从输入流读取特定类型数据的辅助函数），从输入流InFile中读取一个uint32_t类型的数据，
    // 并将其赋值给当前对象的DatabaseId成员变量，确保能正确恢复这个在写入时保存的唯一标识数据。
    ReadValue<uint32_t>(InFile, DatabaseId);
    // 类似地，调用ReadValue函数从输入流InFile中读取一个float类型的数据，然后将该数据赋值给ForwardSpeed成员变量，以恢复对象的速度相关信息。
    ReadValue<float>(InFile, ForwardSpeed);
    // 再次调用ReadValue函数，从输入流InFile中读取一个float类型的数据，并将其赋值给EngineRotation成员变量，从而完整地从输入流中读取数据并更新类中对应成员变量的值，恢复对象的状态。
    ReadValue<float>(InFile, EngineRotation);
}

// ---------------------------------------------

// CarlaRecorderAnimBikers类的Clear函数，用于清空该类内部存储的所有CarlaRecorderAnimBiker类型对象的容器，释放相关内存资源或者重置数据状态。
void CarlaRecorderAnimBikers::Clear(void)
{
    // 调用std::vector容器的clear函数，它会销毁Bikers容器中存储的所有元素（如果元素是动态分配内存的对象，会释放相应内存），并将容器的大小设置为0，
    // 使得Bikers容器变为空容器，后续可以重新添加新的元素进行数据存储等操作。
    Bikers.clear();
}

// CarlaRecorderAnimBikers类的Add函数，功能是向类内部的Bikers容器中添加一个CarlaRecorderAnimBiker类型的对象，方便收集和管理多个此类对象。
void CarlaRecorderAnimBikers::Add(const CarlaRecorderAnimBiker &Biker)
{
    // 利用std::vector容器的push_back函数，将传入的Biker对象（以常量引用的形式传入，避免不必要的拷贝开销，同时保证原对象不被修改）添加到Bikers容器的末尾，
    // 这会使Bikers容器中的元素数量增加1，实现了向容器内插入新元素的操作，便于后续对多个对象进行批量处理（比如批量写入、读取等操作）。
    Bikers.push_back(Biker);
}

// CarlaRecorderAnimBikers类的Write函数，目的是将该类所管理的多个CarlaRecorderAnimBiker对象的数据整体写入到输出流OutFile中，包括一些必要的标识信息和各个对象的数据内容，便于后续进行数据存储、传输以及读取恢复操作。
void CarlaRecorderAnimBikers::Write(std::ostream &OutFile) const
{
    // 调用WriteValue函数，写入一个char类型的数据，这里将CarlaRecorderPacketId::AnimBiker（可能是一个枚举值，用于标识当前数据包对应的是与动画骑手相关的类型）转换为char类型后写入输出流OutFile中，
    // 这样在读取数据时，可以先通过这个标识来判断数据包的类型，进而按照相应的规则进行后续数据的解析，起到区分不同类型数据的作用。
    WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::AnimBiker));

    // 计算要写入的数据包的总大小，数据包总大小由两部分组成：一部分是固定的头部大小（这里设置为2字节，可能用于存放一些协议相关的控制信息或者后续扩展的标识等），
    // 另一部分是所有存储在Bikers容器中的CarlaRecorderAnimBiker对象所占用的空间大小（通过Bikers容器中元素个数乘以单个对象的大小，即sizeof(CarlaRecorderAnimBiker)来计算），
    // 将计算得到的总大小存储在Total变量中，后续会把这个总大小信息也写入到输出流中，方便接收方准确知晓整个数据包的长度，以便正确地读取数据。
    uint32_t Total = 2 + Bikers.size() * sizeof(CarlaRecorderAnimBiker);
    // 调用WriteValue函数，将前面计算好的数据包总大小（Total变量的值，类型为uint32_t）写入到输出流OutFile中，使得接收方能够获取这个关键的数据包长度信息，确保数据读取的准确性。
    WriteValue<uint32_t>(OutFile, Total);

    // 将Bikers容器中存储的CarlaRecorderAnimBiker对象的个数赋值给Total变量（重新赋值，覆盖之前用于计算数据包大小的用法），目的是将这个记录个数信息也写入到输出流中，
    // 接收方在读取数据时，可以先获取这个记录个数，从而知道后续需要读取多少个CarlaRecorderAnimBiker对象的数据，便于通过循环等方式准确地解析出每个对象的数据内容。
    Total = Bikers.size();
    // 调用WriteValue函数，将记录个数（Total变量的值，类型为uint16_t）写入到输出流OutFile中，完成数据记录个数信息的写入操作，为后续正确解析数据提供必要的前置信息。
    WriteValue<uint16_t>(OutFile, Total);

    // 判断记录个数（Total）是否大于0，即确认是否有实际的数据记录需要写入输出流。如果有数据记录（Total > 0），则执行以下操作，将所有CarlaRecorderAnimBiker对象的数据写入到输出流中。
    if (Total > 0)
    {
        // 使用std::ostream的write函数，将Bikers容器中存储的所有CarlaRecorderAnimBiker对象的数据以字节流的形式直接写入到输出流OutFile中。
        // 通过reinterpret_cast将Bikers.data()（它返回指向Bikers容器中存储数据的首地址的指针，原本类型为const char *，符合write函数要求的参数类型）进行类型转换，
        // 并按照Bikers容器中元素个数乘以单个对象大小（Bikers.size() * sizeof(CarlaRecorderAnimBiker)）确定要写入的字节长度，实现批量数据的写入操作，将所有对象的数据完整地发送到输出流中。
        OutFile.write(reinterpret_cast<const char *>(Bikers.data()),
            Bikers.size() * sizeof(CarlaRecorderAnimBiker));
    }
}

// CarlaRecorderAnimBikers类的Read函数，用于从输入流InFile中读取数据，并解析恢复出多个CarlaRecorderAnimBiker对象，将它们存储到类内部的Bikers容器中，实现数据的读取和对象集合的重建操作。
void CarlaRecorderAnimBikers::Read(std::istream &InFile)
{
    // 定义两个变量，i（类型为uint16_t）用于循环计数，Total（类型为uint16_t）用于存储从输入流中读取到的CarlaRecorderAnimBiker对象的总个数，初始化这两个变量，为后续的循环操作和数据读取控制做好准备。
    uint16_t i, Total;
    // 创建一个CarlaRecorderAnimBiker类型的临时对象Biker，在后续的循环读取过程中，每次会利用这个临时对象从输入流中读取一个对象的数据，然后通过调用其Read函数进行解析恢复，最后添加到Bikers容器中。
    CarlaRecorderAnimBiker Biker;

    // 调用ReadValue函数，从输入流InFile中读取一个uint16_t类型的数据，这个数据应该是表示后续要读取的CarlaRecorderAnimBiker对象的总个数，将读取到的值赋给Total变量，
    // 以此来确定后续循环读取数据的次数，确保能够正确地解析出所有需要的对象数据，避免多读或者少读数据的情况发生。
    ReadValue<uint16_t>(InFile, Total);
    // 开始循环，只要i小于Total（即还没有读取完所有的对象数据），就持续执行循环体中的操作，每次循环完成一个CarlaRecorderAnimBiker对象的数据读取、解析和添加到Bikers容器的操作。
    for (i = 0; i < Total; ++i)
    {
        // 调用临时创建的Biker对象的Read函数，传入输入流InFile，让Biker对象从输入流中按照其自身定义的读取格式（在CarlaRecorderAnimBiker::Read函数中实现）读取自身相关的数据，
        // 比如之前通过Write函数写入的DatabaseId、ForwardSpeed、EngineRotation等成员变量的数据，完成单个对象数据的解析恢复操作，使其恢复到写入之前的状态。
        Biker.Read(InFile);
        // 调用该类的Add函数，将已经读取并解析好数据的Biker对象添加到Bikers容器中，实现将从输入流中读取到的对象数据存储到类中管理的数据容器里，方便后续对这些对象进行统一的管理和使用，比如再次写入、查询等操作。
        Add(Biker);
    }
}

// CarlaRecorderAnimBikers类的GetBikers函数，用于获取类内部存储CarlaRecorderAnimBiker对象的向量容器（Bikers）的常量引用，方便外部代码访问容器内的数据，但不允许修改容器本身。
const std::vector<CarlaRecorderAnimBiker>& CarlaRecorderAnimBikers::GetBikers()
{
    return Bikers;
}
