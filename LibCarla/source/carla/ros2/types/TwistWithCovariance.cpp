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
 * @file TwistWithCovariance.cpp  
 * 此源文件包含在 IDL 文件中描述的类型的定义。  
 *  
 * 此文件是由工具 gen 生成的。  
 */

#ifdef _WIN32
// 在 Visual Studio 中删除链接器警告 LNK4221
namespace {
char dummy;// 创建一个 dummy 变量以避免链接器警告
}  // namespace
#endif  // _WIN32

#include "TwistWithCovariance.h"// 包含头文件，定义了 TwistWithCovariance 类
#include <fastcdr/Cdr.h>// 包含 Fast CDR 序列化和反序列化功能

#include <fastcdr/exceptions/BadParamException.h>// 包含异常处理
using namespace eprosima::fastcdr::exception;// 使用 Fast CDR 的异常命名空间

#include <utility>// 包含标准库的实用工具，比如 std::move
// 定义最大 CDR 类型大小
#define geometry_msgs_msg_TwistWithCovariance_max_cdr_typesize 336ULL;
#define geometry_msgs_msg_Vector3_max_cdr_typesize 24ULL;
#define geometry_msgs_msg_Twist_max_cdr_typesize 48ULL;
#define geometry_msgs_msg_TwistWithCovariance_max_key_cdr_typesize 0ULL;
#define geometry_msgs_msg_Vector3_max_key_cdr_typesize 0ULL;
#define geometry_msgs_msg_Twist_max_key_cdr_typesize 0ULL;
// 默认构造函数
geometry_msgs::msg::TwistWithCovariance::TwistWithCovariance()
{
    // 初始化 m_covariance 为 0
    memset(&m_covariance, 0, (36) * 8);// 将协方差数组的每个元素初始化为 0
}
// 默认析构函数
geometry_msgs::msg::TwistWithCovariance::~TwistWithCovariance()
{
}
// 复制构造函数
geometry_msgs::msg::TwistWithCovariance::TwistWithCovariance(
        const TwistWithCovariance& x)
{
    m_twist = x.m_twist;// 复制 m_twist
    m_covariance = x.m_covariance;// 复制 m_covariance
}
// 移动构造函数
geometry_msgs::msg::TwistWithCovariance::TwistWithCovariance(
        TwistWithCovariance&& x) noexcept
{
    m_twist = std::move(x.m_twist);// 移动 m_twist
    m_covariance = std::move(x.m_covariance);// 移动 m_covariance
}
// 复制赋值运算符
geometry_msgs::msg::TwistWithCovariance& geometry_msgs::msg::TwistWithCovariance::operator =(
        const TwistWithCovariance& x)
{
    m_twist = x.m_twist;// 复制 m_twist
    m_covariance = x.m_covariance;// 复制 m_covariance

    return *this;// 返回自身的引用
}
// 移动赋值运算符
geometry_msgs::msg::TwistWithCovariance& geometry_msgs::msg::TwistWithCovariance::operator =(
        TwistWithCovariance&& x) noexcept
{
    m_twist = std::move(x.m_twist);// 移动 m_twist
    m_covariance = std::move(x.m_covariance);// 移动 m_covariance

    return *this;// 返回自身的引用
}
// 比较运算符
bool geometry_msgs::msg::TwistWithCovariance::operator ==(
        const TwistWithCovariance& x) const
{
    return (m_twist == x.m_twist && m_covariance == x.m_covariance);// 比较两个对象是否相等
}
// 不等于运算符
bool geometry_msgs::msg::TwistWithCovariance::operator !=(
        const TwistWithCovariance& x) const
{
    return !(*this == x);// 使用 == 运算符检查不等
}
// 获取最大序列化大小
size_t geometry_msgs::msg::TwistWithCovariance::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);// 避免未使用参数的警告
    return geometry_msgs_msg_TwistWithCovariance_max_cdr_typesize;// 返回最大 CDR 序列化大小
}
// 获取序列化大小
size_t geometry_msgs::msg::TwistWithCovariance::getCdrSerializedSize(
        const geometry_msgs::msg::TwistWithCovariance& data,
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;// 保存初始对齐
    current_alignment += geometry_msgs::msg::Twist::getCdrSerializedSize(data.twist(), current_alignment);// 添加 twist 的序列化大小
    current_alignment += ((36) * 8) + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);// 添加协方差数组的序列化大小

    return current_alignment - initial_alignment;// 返回当前对齐与初始对齐的差值
}
// 序列化函数
void geometry_msgs::msg::TwistWithCovariance::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    scdr << m_twist;// 序列化 m_twist
    scdr << m_covariance;// 序列化 m_covariance
}
// 反序列化函数
void geometry_msgs::msg::TwistWithCovariance::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    dcdr >> m_twist;// 反序列化 m_twist
    dcdr >> m_covariance;// 反序列化 m_covariance
}

/*!
 * @brief This function copies the value in member twist
 * @param _twist New value to be copied in member twist
 */
// 设置 twist 成员的值
void geometry_msgs::msg::TwistWithCovariance::twist(
        const geometry_msgs::msg::Twist& _twist)
{
    m_twist = _twist;// 复制传入的 twist 值
}

/*!
 * @brief This function moves the value in member twist
 * @param _twist New value to be moved in member twist
 */
// 移动设置 twist 成员的值
void geometry_msgs::msg::TwistWithCovariance::twist(
        geometry_msgs::msg::Twist&& _twist)
{
    m_twist = std::move(_twist);// 移动传入的 twist 值
}

/*!
 * @brief This function returns a constant reference to member twist
 * @return Constant reference to member twist
 */
// 获取 twist 成员的常量引用
const geometry_msgs::msg::Twist& geometry_msgs::msg::TwistWithCovariance::twist() const
{
    return m_twist;// 返回 m_twist 的常量引用
}

/*!
 * @brief This function returns a reference to member twist
 * @return Reference to member twist
 */
// 获取 twist 成员的引用
geometry_msgs::msg::Twist& geometry_msgs::msg::TwistWithCovariance::twist()
{
    return m_twist;// 返回 m_twist 的引用
}

/*!
 * @brief This function copies the value in member covariance
 * @param _covariance New value to be copied in member covariance
 */
// 设置 covariance 成员的值
void geometry_msgs::msg::TwistWithCovariance::covariance(
        const geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36& _covariance)
{
    m_covariance = _covariance;// 复制传入的 covariance 值
}

/*!
 * @brief This function moves the value in member covariance
 * @param _covariance New value to be moved in member covariance
 */
// 移动设置 covariance 成员的值
void geometry_msgs::msg::TwistWithCovariance::covariance(
        geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36&& _covariance)
{
    m_covariance = std::move(_covariance);// 移动传入的 covariance 值
}

/*!
 * @brief This function returns a constant reference to member covariance
 * @return Constant reference to member covariance
 */
// 获取 covariance 成员的常量引用
const geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36& geometry_msgs::msg::TwistWithCovariance::covariance() const
{
    return m_covariance;// 返回 m_covariance 的常量引用
}

/*!
 * @brief This function returns a reference to member covariance
 * @return Reference to member covariance
 */
// 获取 covariance 成员的引用
geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36& geometry_msgs::msg::TwistWithCovariance::covariance()
{
    return m_covariance;// 返回 m_covariance 的引用
}
// 获取键的最大序列化大小
size_t geometry_msgs::msg::TwistWithCovariance::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);// 避免未使用参数的警告
    return geometry_msgs_msg_TwistWithCovariance_max_key_cdr_typesize;// 返回最大键的 CDR 序列化大小
}
// 检查键是否已定义
bool geometry_msgs::msg::TwistWithCovariance::isKeyDefined()
{
    return false;// 此类型没有定义键
}
// 序列化键
void geometry_msgs::msg::TwistWithCovariance::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;// 当前不需要序列化键
}
