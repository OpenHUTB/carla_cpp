// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file Point32.cpp
 * 此源文件包含了在 IDL 文件中所描述类型的定义。
 * 该文件由工具 gen 生成。
 */

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
// 在 Windows 平台（_WIN32 宏定义存在时）下，通过定义一个匿名命名空间及一个未使用的变量 dummy，消除 Visual Studio 的链接器警告 LNK4221
namespace {
char dummy;
}  // namespace
#endif  // _WIN32

// 引入自定义的 Point32 头文件，其中应该包含了 Point32 类的前置声明以及相关依赖类型的定义等内容
#include "Point32.h"
// 引入 FastCDR 库中用于处理 Cdr（Common Data Representation，一种数据表示格式）相关操作的头文件
#include <fastcdr/Cdr.h>

// 引入 FastCDR 库中用于处理异常的头文件，并使用整个异常命名空间，方便后续抛出对应的异常类型
#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

// 引入 C++ 标准库中的 utility 头文件，用于支持 std::move 等实用功能，在移动语义相关操作中会用到
#include <utility>

// 定义一个宏，表示 Point32 类型的最大 CDR 序列化类型大小，这里设置为 12ULL（可能根据其内部数据结构及序列化规则确定，由于包含三个 float 类型成员，每个 float 占 4 字节，共 12 字节）
#define geometry_msgs_msg_Point32_max_cdr_typesize 12ULL;
// 定义一个宏，表示 Point32 类型的键（Key）的最大 CDR 序列化类型大小，设置为 0ULL，意味着目前暂未定义有效的键或者该类型在当前设计下不需要键相关功能（后续可按需扩展）
#define geometry_msgs_msg_Point32_max_key_cdr_typesize 0ULL;

// Point32 类的默认构造函数，用于初始化该类对象的各个成员变量，将表示三维坐标的成员变量 m_x、m_y、m_z 都初始化为 0.0，为对象提供一个默认的初始状态
geometry_msgs::msg::Point32::Point32()
{
    // 初始化成员变量 m_x 为 0.0，它代表三维空间中点的 x 坐标，初始化为 0 作为默认值
    m_x = 0.0;
    // 初始化成员变量 m_y 为 0.0，代表 y 坐标，同样初始化为 0
    m_y = 0.0;
    // 初始化成员变量 m_z 为 0.0，代表 z 坐标，初始化为 0
    m_z = 0.0;
}

// Point32 类的默认析构函数，目前函数体为空，说明没有需要手动释放的资源（如动态分配的内存等），编译器会自动生成默认的析构逻辑来处理成员变量的销毁等操作，例如调用成员对象的析构函数（如果有）
geometry_msgs::msg::Point32::~Point32()
{
}

// Point32 类的拷贝构造函数，根据传入的另一个 Point32 对象进行拷贝创建新对象，对成员变量 m_x、m_y、m_z 进行逐个拷贝赋值，实现深拷贝效果，确保新对象和原对象的数据完全独立，具体的拷贝行为取决于 float 类型自身的拷贝语义（通常就是简单的值拷贝）
geometry_msgs::msg::Point32::Point32(
        const Point32& x)
{
    m_x = x.m_x;
    m_y = x.m_y;
    m_z = x.m_z;
}

// Point32 类的移动构造函数，虽然这里对成员变量的赋值看起来和拷贝构造函数类似，但实际上通过 std::move 语义（尽管此处成员变量为简单的 float 类型，移动语义效果和拷贝基本相同，但遵循统一的编程规范），将传入对象的资源所有权转移到新创建的对象中，避免不必要的拷贝开销，适用于更复杂的对象或者涉及资源管理的场景
geometry_msgs::msg::Point32::Point32(
        Point32&& x) noexcept
{
    m_x = x.m_x;
    m_y = x.m_y;
    m_z = x.m_z;
}

// Point32 类的拷贝赋值运算符重载，将一个 Point32 对象的值赋给当前对象，对成员变量 m_x、m_y、m_z 进行逐个赋值操作，实现对象间数据的复制，遵循拷贝语义进行赋值，使得当前对象能获取到传入对象的数据副本
geometry_msgs::msg::Point32& geometry_msgs::msg::Point32::operator =(
        const Point32& x)
{
    m_x = x.m_x;
    m_y = x.m_y;
    m_z = x.m_z;

    return *this;
}

// Point32 类的移动赋值运算符重载，通过移动语义将一个 Point32 对象的资源所有权转移给当前对象，对成员变量使用 std::move 进行移动赋值（同样对于简单 float 类型效果类似拷贝，但符合编程规范），实现高效赋值操作，避免不必要的拷贝，提升性能（在复杂对象场景下更明显）
geometry_msgs::msg::Point32& geometry_msgs::msg::Point32::operator =(
        Point32&& x) noexcept
{
    m_x = x.m_x;
    m_y = x.m_y;
    m_z = x.m_z;

    return *this;
}

// 比较运算符 == 的重载，用于比较当前对象与传入的 Point32 对象是否相等，通过比较两个对象的成员变量 m_x、m_y、m_z 是否分别相等来确定整个对象是否相等，提供了一种方便的方式来判断两个 Point32 对象的内容一致性
bool geometry_msgs::msg::Point32::operator ==(
        const Point32& x) const
{
    return (m_x == x.m_x && m_y == x.m_y && m_z == x.m_z);
}

// 比较运算符!= 的重载，通过调用 == 运算符并取反来判断当前对象与传入的 Point32 对象是否不相等，这是基于 == 运算符实现的一种便捷的相反判断逻辑，方便在不同场景下使用
bool geometry_msgs::msg::Point32::operator!=(
        const Point32& x) const
{
    return!(*this == x);
}

// 获取 Point32 类型对象根据当前缓冲区对齐方式的最大 CDR 序列化大小，这里忽略传入的当前对齐参数，直接返回预定义的该类型最大序列化大小常量值（即前面定义的 12ULL），此函数可用于在一些场景下（如分配缓冲区空间等）提前知晓该类型对象序列化后的最大可能长度
size_t geometry_msgs::msg::Point32::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return geometry_msgs_msg_Point32_max_cdr_typesize;
}

// 计算给定 Point32 类型对象根据当前缓冲区对齐方式的 CDR 序列化大小，虽然传入了表示对象的数据参数，但这里并没有实际使用它（通过 `(void)data` 语句体现），只是按照固定的计算方式，考虑每个成员变量（都是 float 类型，占 4 字节，并考虑对齐要求）的序列化大小，最终返回总的序列化大小，该函数在实际进行序列化操作前，可准确计算出特定对象的序列化长度，有助于精准分配内存空间等操作
size_t geometry_msgs::msg::Point32::getCdrSerializedSize(
        const geometry_msgs::msg::Point32& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;
    // 考虑 4 字节对齐，更新当前对齐位置，然后加上成员变量 m_x 的序列化大小（float 类型占 4 字节）
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    // 同样考虑 4 字节对齐，更新当前对齐位置，然后加上成员变量 m_y 的序列化大小
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    // 再次考虑 4 字节对齐，更新当前对齐位置，然后加上成员变量 m_z 的序列化大小
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    return current_alignment - initial_alignment;
}

// 使用 CDR 格式对 Point32 类型对象进行序列化操作，将对象的成员变量 m_x、m_y、m_z 依次写入到给定的 Cdr 对象中，以便进行网络传输或存储等，这是将对象转换为可传输或存储格式的关键操作，按照特定的顺序和格式将数据写入
void geometry_msgs::msg::Point32::serialize(
        eprosima::fastcdr::Cdr& scdr)
{
    scdr << m_x;
    scdr << m_y;
    scdr << m_z;
}

// 使用 CDR 格式对 Point32 类型对象进行反序列化操作，从给定的 Cdr 对象中依次读取数据并赋值给对象的成员变量 m_x、m_y、m_z，将接收到的序列化数据还原为对象，与 serialize 函数相对应，实现从序列化数据恢复出原始对象的功能，确保数据在传输或存储后能正确还原
void geometry_msgs::msg::Point32::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    dcdr >> m_x;
    dcdr >> m_y;
    dcdr >> m_z;
}

/*!
 * @brief 此函数用于设置成员变量 x（即 m_x）的值，传入新的 float 类型值进行赋值，提供了一种外部修改 Point32 对象中 x 坐标值的方式
 * @param _x 要设置给成员变量 x 的新值。
 */
void geometry_msgs::msg::Point32::x(
        float _x)
{
    m_x = _x;
}

/*!
 * @brief 此函数返回成员变量 x（即 m_x）的值，提供了一种只读的方式来获取 x 坐标的值，保障了数据的安全性，避免外部误修改
 * @return 成员变量 x 的值。
 */
float geometry_msgs::msg::Point32::x() const
{
    return m_x;
}

/*!
 * @brief 此函数返回成员变量 x（即 m_x）的引用，外部可通过该引用获取和修改 x 坐标的值，提供了一种可读写的方式来访问 x 坐标，方便外部根据需要修改其值
 * @return 成员变量 x 的引用。
 */
float& geometry_msgs::msg::Point32::x()
{
    return m_x;
}

/*!
 * @brief 此函数用于设置成员变量 y（即 m_y）的值，传入新的 float 类型值进行赋值，方便外部更新 y 坐标的值
 * @param _y 要设置给成员变量 y 的新值。
 */
void geometry_msgs::msg::Point32::y(
        float _y)
{
    m_y = _y;
}

/*!
 * @brief 此函数返回成员变量 y（即 m_y）的值，提供了一种只读的方式来获取 y 坐标的值，用于外部获取该坐标信息但不修改
 * @return 成员变量 y 的值。
 */
float geometry_msgs::msg::Point32::y() const
{
    return m_y;
}

/*!
 * @brief 此函数返回成员变量 y（即 m_y）的引用，外部可通过该引用获取和修改 y 坐标的值，便于外部按需改变该坐标值
 * @return 成员变量 y 的引用。
 */
float& geometry_msgs::msg::Point32::y()
{
    return m_y;
}

/*!
 * @brief 此函数用于设置成员变量 z（即 m_z）的值，传入新的 float 类型值进行赋值，用于更新 z 坐标的值
 * @param _z 要设置给成员变量 z 的新值。
 */
void geometry_msgs::msg::Point32::z(
        float _z)
{
    m_z = _z;
}

/*!
 * @brief 此函数返回成员变量 z（即 m_z）的值，提供了一种只读的方式来获取 z 坐标的值，供外部获取该坐标信息
 * @return 成员变量 z 的值。
 */
float geometry_msgs::msg::Point32::y() const
{
    return m_z;
}

/*!
 * @brief 此函数返回成员变量 z（即 m_z）的引用，外部可通过该引用获取和修改 z 坐标的值，方便外部操作该坐标
 * @return 成员变量 z 的引用。
 */
float& geometry_msgs::msg::Point32::z()
{
    return m_z;
}

// 获取 Point32 类型对象的键（Key）根据当前缓冲区对齐方式的最大 CDR 序列化大小，这里忽略传入的当前对齐参数，直接返回预定义的该类型键的最大序列化大小常量值（即 0ULL，表示无有效键定义）
size_t geometry_msgs::msg::Point32::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return geometry_msgs_msg_Point32_max_key_cdr_typesize;
}

// 判断 Point32 类型是否已经定义了键（Key），目前返回 false，表示该类型暂未定义有效的键，后续若需要可根据具体需求扩展键相关功能
bool geometry_msgs::msg::Point32::isKeyDefined()
{
    return false;
}

// 使用 CDR 格式对 Point32 类型对象的键（Key）成员进行序列化操作，目前函数体为空（只是将传入的 Cdr 对象忽略，意味着该类型当前无键需要序列化或者未实现具体序列化逻辑）
void geometry_msgs::msg::Point32::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
}
