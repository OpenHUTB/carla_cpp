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
 * @file Twist.cpp  
 * 此源文件包含在 IDL 文件中描述的类型的定义。  
 *  
 * 此文件是由工具 gen 生成的。  
 */

#ifdef _WIN32
// 在 Visual Studio 中删除链接器警告 LNK4221
namespace {
char dummy;// 定义一个 dummy 变量以防止链接器警告
}  // namespace
#endif  // _WIN32

#include "Twist.h"// 包含 Twist.h 头文件
#include <fastcdr/Cdr.h>// 包含 Fast CDR 序列化库

#include <fastcdr/exceptions/BadParamException.h>// 包含异常处理
using namespace eprosima::fastcdr::exception;// 使用 Fast CDR 的异常命名空间

#include <utility>// 包含实用工具头文件
// 定义几何消息类型 Twist 和 Vector3 的最大 CDR 序列化大小
#define geometry_msgs_msg_Vector3_max_cdr_typesize 24ULL;// Vector3 的最大 CDR 序列化大小为 24 字节
#define geometry_msgs_msg_Twist_max_cdr_typesize 48ULL;// Twist 的最大 CDR 序列化大小为 48 字节
#define geometry_msgs_msg_Vector3_max_key_cdr_typesize 0ULL;// Vector3 的最大键 CDR 序列化大小为 0（无键）
#define geometry_msgs_msg_Twist_max_key_cdr_typesize 0ULL;// Twist 的最大键 CDR 序列化大小为 0（无键）
// Twist 类的构造函数
geometry_msgs::msg::Twist::Twist()
{
}
// Twist 类的析构函数
geometry_msgs::msg::Twist::~Twist()
{
}
// 拷贝构造函数，复制另一个 Twist 对象的成员
geometry_msgs::msg::Twist::Twist(
        const Twist& x)
{
    m_linear = x.m_linear;// 复制线性速度
    m_angular = x.m_angular;// 复制角速度
}
// 移动构造函数，移动另一个 Twist 对象的成员
geometry_msgs::msg::Twist::Twist(
        Twist&& x) noexcept
{
    m_linear = std::move(x.m_linear);// 移动线性速度
    m_angular = std::move(x.m_angular);// 移动角速度
}
// 拷贝赋值运算符，复制另一个 Twist 对象的成员
geometry_msgs::msg::Twist& geometry_msgs::msg::Twist::operator =(
        const Twist& x)
{
    m_linear = x.m_linear;// 复制线性速度
    m_angular = x.m_angular;// 复制角速度

    return *this;// 返回当前对象的引用
}
// 移动赋值运算符，移动另一个 Twist 对象的成员
geometry_msgs::msg::Twist& geometry_msgs::msg::Twist::operator =(
        Twist&& x) noexcept
{
    m_linear = std::move(x.m_linear);// 移动线性速度
    m_angular = std::move(x.m_angular);// 移动角速度

    return *this;// 返回当前对象的引用
}
// 判断两个 Twist 对象是否相等
bool geometry_msgs::msg::Twist::operator ==(
        const Twist& x) const
{
    return (m_linear == x.m_linear && m_angular == x.m_angular);// 比较线性速度和角速度
}
// 判断两个 Twist 对象是否不相等
bool geometry_msgs::msg::Twist::operator !=(
        const Twist& x) const
{
    return !(*this == x);// 通过相等运算符的结果判断
}
// 获取 Twist 的最大 CDR 序列化大小
size_t geometry_msgs::msg::Twist::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);// 防止未使用变量的警告
    return geometry_msgs_msg_Twist_max_cdr_typesize;// 返回最大序列化大小
}
// 获取 Twist 的 CDR 序列化大小
size_t geometry_msgs::msg::Twist::getCdrSerializedSize(
        const geometry_msgs::msg::Twist& data,
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;// 保存初始对齐
    current_alignment += geometry_msgs::msg::Vector3::getCdrSerializedSize(data.linear(), current_alignment);// 计算线性速度的序列化大小
    current_alignment += geometry_msgs::msg::Vector3::getCdrSerializedSize(data.angular(), current_alignment);// 计算角速度的序列化大小

    return current_alignment - initial_alignment;// 返回序列化大小
}
// 序列化 Twist 对象
void geometry_msgs::msg::Twist::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    scdr << m_linear;// 序列化线性速度
    scdr << m_angular;// 序列化角速度
}
// 反序列化 Twist 对象
void geometry_msgs::msg::Twist::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    dcdr >> m_linear;// 反序列化线性速度
    dcdr >> m_angular;// 反序列化角速度
}

*!  
 * @brief 此函数复制成员 linear 的值  
 * @param _linear 要复制到成员 linear 的新值  
 */
void geometry_msgs::msg::Twist::linear(
        const geometry_msgs::msg::Vector3& _linear)
{
    m_linear = _linear;// 复制线性速度
}

/*!  
 * @brief 此函数移动成员 linear 的值  
 * @param _linear 要移动到成员 linear 的新值  
 */
void geometry_msgs::msg::Twist::linear(
        geometry_msgs::msg::Vector3&& _linear)
{
    m_linear = std::move(_linear);// 移动线性速度
}

/*!  
 * @brief 此函数返回成员 linear 的常量引用  
 * @return 成员 linear 的常量引用  
 */
const geometry_msgs::msg::Vector3& geometry_msgs::msg::Twist::linear() const
{
    return m_linear;// 返回线性速度的常量引用
}

/*!  
 * @brief 此函数返回成员 linear 的引用  
 * @return 成员 linear 的引用  
 */
geometry_msgs::msg::Vector3& geometry_msgs::msg::Twist::linear()
{
    return m_linear;// 返回线性速度的引用
}

/*!  
 * @brief 此函数复制成员 angular 的值  
 * @param _angular 要复制到成员 angular 的新值  
 */
void geometry_msgs::msg::Twist::angular(
        const geometry_msgs::msg::Vector3& _angular)
{
    m_angular = _angular;// 复制角速度
}

/*!  
 * @brief 此函数移动成员 angular 的值  
 * @param _angular 要移动到成员 angular 的新值  
 */
void geometry_msgs::msg::Twist::angular(
        geometry_msgs::msg::Vector3&& _angular)
{
    m_angular = std::move(_angular);// 移动角速度
}

/*!  
 * @brief 此函数返回成员 angular 的常量引用  
 * @return 成员 angular 的常量引用  
 */
const geometry_msgs::msg::Vector3& geometry_msgs::msg::Twist::angular() const
{
    return m_angular;// 返回角速度的常量引用
}

/*!  
 * @brief 此函数返回成员 angular 的引用  
 * @return 成员 angular 的引用  
 */
geometry_msgs::msg::Vector3& geometry_msgs::msg::Twist::angular()
{
    return m_angular;// 返回角速度的引用
}

// 获取 Twist 的最大键 CDR 序列化大小
size_t geometry_msgs::msg::Twist::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);// 防止未使用变量的警告
    return geometry_msgs_msg_Twist_max_key_cdr_typesize;// 返回最大键序列化大小
}
// 判断是否定义了键
bool geometry_msgs::msg::Twist::isKeyDefined()
{
    return false;// Twist 类型没有键
}
// 序列化键（无操作）
void geometry_msgs::msg::Twist::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;// 防止未使用变量的警告
}
