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
 * @file CarlaLineInvasion.cpp
 * 此源文件包含了在 IDL 文件中所描述类型的定义。
 * 该文件由工具 gen 生成。
 */

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
// 通过定义一个匿名命名空间及一个未使用的变量 dummy，用于消除 Visual Studio 中的链接器警告 LNK4221
namespace {
char dummy;
}  // namespace
#endif  // _WIN32
// 引入自定义的 CarlaLineInvasion 头文件，可能包含了 LaneInvasionEvent 类的前置声明等相关内容
#include "CarlaLineInvasion.h"
// 引入 FastCDR 库中用于处理 Cdr（Common Data Representation，一种数据表示格式）相关操作的头文件
#include <fastcdr/Cdr.h>
// 引入 FastCDR 库中用于处理异常的头文件，这里导入了整个异常命名空间，方便后续抛出相关异常类型
#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;
// 引入 C++ 标准库中的 utility 头文件，可能用于使用 std::move 等相关功能
#include <utility>
// 定义一些宏，用于表示不同消息类型的最大 CDR 序列化类型大小，这里的值可能是预计算或者根据某种规则确定的固定值
#define carla_msgs_msg_std_msgs_msg_Header_max_cdr_typesize 268ULL;
#define carla_msgs_msg_LaneInvasionEvent_max_cdr_typesize 672ULL;
#define carla_msgs_msg_std_msgs_msg_Time_max_cdr_typesize 8ULL;
#define carla_msgs_msg_std_msgs_msg_Header_max_key_cdr_typesize 0ULL;
#define carla_msgs_msg_LaneInvasionEvent_max_key_cdr_typesize 0ULL;
#define carla_msgs_msg_std_msgs_msg_Time_max_key_cdr_typesize 0ULL;
// LaneInvasionEvent 类的默认构造函数，用于创建对象时进行默认的初始化操作，目前函数体为空，可能依赖成员变量的默认初始化行为
carla_msgs::msg::LaneInvasionEvent::LaneInvasionEvent()
{
}
// LaneInvasionEvent 类的默认析构函数，用于在对象生命周期结束时释放相关资源，目前函数体为空，可能意味着没有需要手动释放的资源，依赖编译器自动生成的析构逻辑来处理成员变量的销毁等操作
carla_msgs::msg::LaneInvasionEvent::~LaneInvasionEvent()
carla_msgs::msg::LaneInvasionEvent::~LaneInvasionEvent()
{
}
// LaneInvasionEvent 类的拷贝构造函数，根据传入的另一个 LaneInvasionEvent 对象进行拷贝创建新对象，对成员变量进行逐个拷贝赋值，实现深拷贝效果（具体取决于成员变量自身的拷贝语义）
carla_msgs::msg::LaneInvasionEvent::LaneInvasionEvent(
        const LaneInvasionEvent& x)
{
    m_header = x.m_header;
    m_crossed_lane_markings = x.m_crossed_lane_markings;
}
// LaneInvasionEvent 类的移动构造函数，通过移动语义，高效地将传入的 LaneInvasionEvent 对象资源所有权转移到新创建的对象中，对成员变量使用 std::move 进行移动赋值，避免不必要的拷贝开销
carla_msgs::msg::LaneInvasionEvent::LaneInvasionEvent(
        LaneInvasionEvent&& x) noexcept
{
    m_header = std::move(x.m_header);
    m_crossed_lane_markings = std::move(x.m_crossed_lane_markings);
}
// LaneInvasionEvent 类的拷贝赋值运算符重载，将一个 LaneInvasionEvent 对象的值赋给当前对象，对成员变量进行逐个赋值操作，实现对象间数据的复制
carla_msgs::msg::LaneInvasionEvent& carla_msgs::msg::LaneInvasionEvent::operator =(
        const LaneInvasionEvent& x)
{
    m_header = x.m_header;
    m_crossed_lane_markings = x.m_crossed_lane_markings;

    return *this;
}
// LaneInvasionEvent 类的移动赋值运算符重载，通过移动语义将一个 LaneInvasionEvent 对象的资源所有权转移给当前对象，对成员变量使用 std::move 进行移动赋值，实现高效赋值操作，避免不必要的拷贝
carla_msgs::msg::LaneInvasionEvent& carla_msgs::msg::LaneInvasionEvent::operator =(
        LaneInvasionEvent&& x) noexcept
{
    m_header = std::move(x.m_header);
    m_crossed_lane_markings = std::move(x.m_crossed_lane_markings);

    return *this;
}
// 比较运算符 == 的重载，用于比较当前对象与传入的 LaneInvasionEvent 对象是否相等，通过比较两个对象的成员变量（消息头和越界车道标记）是否相等来确定整个对象是否相等
bool carla_msgs::msg::LaneInvasionEvent::operator ==(
        const LaneInvasionEvent& x) const
{
    return (m_header == x.m_header && m_crossed_lane_markings == x.m_crossed_lane_markings);
}
// 比较运算符!= 的重载，通过调用 == 运算符并取反来判断当前对象与传入的 LaneInvasionEvent 对象是否不相等
bool carla_msgs::msg::LaneInvasionEvent::operator !=(
        const LaneInvasionEvent& x) const
{
    return !(*this == x);
}
// 获取 LaneInvasionEvent 类型对象根据当前缓冲区对齐方式的最大 CDR 序列化大小，这里忽略传入的当前对齐参数，直接返回预定义的该类型最大序列化大小常量值
size_t carla_msgs::msg::LaneInvasionEvent::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return carla_msgs_msg_LaneInvasionEvent_max_cdr_typesize;
}
// 计算给定 LaneInvasionEvent 类型对象根据当前缓冲区对齐方式的 CDR 序列化大小，通过依次计算消息头和越界车道标记向量等成员的序列化大小，并考虑对齐要求，最终返回总的序列化大小
size_t carla_msgs::msg::LaneInvasionEvent::getCdrSerializedSize(
        const carla_msgs::msg::LaneInvasionEvent& data,
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;// 先计算消息头成员的序列化大小，并更新当前对齐位置
    current_alignment += std_msgs::msg::Header::getCdrSerializedSize(data.header(), current_alignment);// 考虑 4 字节对齐，更新当前对齐位置
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    if (data.crossed_lane_markings().size() > 0)// 如果越界车道标记向量不为空，计算其序列化大小（每个元素占 4 字节，并考虑对齐），并更新当前对齐位置
    {
        current_alignment += (data.crossed_lane_markings().size() * 4) + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    }

    return current_alignment - initial_alignment;
}
// 使用 CDR 格式对 LaneInvasionEvent 类型对象进行序列化操作，将对象的成员变量（消息头和越界车道标记向量）依次写入到给定的 Cdr 对象中，以便进行网络传输或存储等
void carla_msgs::msg::LaneInvasionEvent::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    scdr << m_header;
    scdr << m_crossed_lane_markings;
}
// 使用 CDR 格式对 LaneInvasionEvent 类型对象进行反序列化操作，从给定的 Cdr 对象中依次读取数据并赋值给对象的成员变量（消息头和越界车道标记向量），将接收到的序列化数据还原为对象
void carla_msgs::msg::LaneInvasionEvent::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    dcdr >> m_header;
    dcdr >> m_crossed_lane_markings;
}

/*!
 * @brief 此函数用于拷贝成员 header 的值，传入新的 std_msgs::msg::Header 对象进行赋值。
 * @param _header 要拷贝到成员 header 的新值。
 */
void carla_msgs::msg::LaneInvasionEvent::header(
        const std_msgs::msg::Header& _header)
{
    m_header = _header;
}

/*!
 * @brief 此函数用于移动成员 header 的值，传入新的 std_msgs::msg::Header 对象进行资源所有权转移赋值。
 * @param _header 要移动到成员 header 的新值。
 */
void carla_msgs::msg::LaneInvasionEvent::header(
        std_msgs::msg::Header&& _header)
{
    m_header = std::move(_header);
}

/*!
 * @brief 此函数返回成员 header 的常量引用，外部可通过该引用获取 header 的值，但不能修改它。
 * @return 成员 header 的常量引用。
 */
const std_msgs::msg::Header& carla_msgs::msg::LaneInvasionEvent::header() const
{
    return m_header;
}

/*!
 * @brief 此函数返回成员 header 的引用，外部可通过该引用获取和修改 header 的值。
 * @return 成员 header 的引用。
 */
std_msgs::msg::Header& carla_msgs::msg::LaneInvasionEvent::header()
{
    return m_header;
}
/*!
 * @brief 此函数用于拷贝成员 crossed_lane_markings 的值，传入新的 std::vector<int32_t> 对象进行赋值。
 * @param _crossed_lane_markings 要拷贝到成员 crossed_lane_markings 的新值。
 */
void carla_msgs::msg::LaneInvasionEvent::crossed_lane_markings(
        const std::vector<int32_t>& _crossed_lane_markings)
{
    m_crossed_lane_markings = _crossed_lane_markings;
}

/*!
 * @brief 此函数用于移动成员 crossed_lane_markings 的值，传入新的 std::vector<int32_t> 对象进行资源所有权转移赋值。
 * @param _crossed_lane_markings 要移动到成员 crossed_lane_markings 的新值。
 */
void carla_msgs::msg::LaneInvasionEvent::crossed_lane_markings(
        std::vector<int32_t>&& _crossed_lane_markings)
{
    m_crossed_lane_markings = std::move(_crossed_lane_markings);
}

/*!
 * @brief 此函数返回成员 crossed_lane_markings 的常量引用，外部可通过该引用获取其值，但不能修改它。
 *  @return 成员 crossed_lane_markings 的常量引用。
 */
const std::vector<int32_t>& carla_msgs::msg::LaneInvasionEvent::crossed_lane_markings() const
{
    return m_crossed_lane_markings;
}

/*!
 * @brief 此函数返回成员 crossed_lane_markings 的引用，外部可通过该引用获取和修改其值。
 * @return 成员 crossed_lane_markings 的引用。
 */
std::vector<int32_t>& carla_msgs::msg::LaneInvasionEvent::crossed_lane_markings()
{
    return m_crossed_lane_markings;
}
// 获取 LaneInvasionEvent 类型对象的键（Key）根据当前缓冲区对齐方式的最大 CDR 序列化大小，这里忽略传入的当前对齐参数，直接返回预定义的该类型键的最大序列化大小常量值（目前为 0）

size_t carla_msgs::msg::LaneInvasionEvent::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return carla_msgs_msg_LaneInvasionEvent_max_key_cdr_typesize;
}
// 判断 LaneInvasionEvent 类型是否已经定义了键（Key），目前返回 false，表示该类型未定义键
bool carla_msgs::msg::LaneInvasionEvent::isKeyDefined()
{
    return false;
}
// 使用 CDR 格式对 LaneInvasionEvent 类型对象的键（Key）成员进行序列化操作，目前函数体为空（只是将传入的 Cdr 对象忽略，可能表示该类型无键需要序列化或者未实现具体序列化逻辑）
void carla_msgs::msg::LaneInvasionEvent::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
}
